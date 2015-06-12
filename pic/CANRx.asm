#define IN_CANRX.ASM
;******************************************************************************
;* @file    canrx.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;Copyright Glenn Self 2004

;Handle and despatch can frames for gsi
;******************************************************************************
;CANRx
;CAN Rx stuff

;#include <G:\prog\mplab\MPLAB IDE\MCHIP_Tools\p18f458.inc>
;#include "p18xxx8.inc"
#include "GSI.inc"
;#IF DEVICE == _18F458
;#include "p18xxx8.inc"
;#ELSE
;#INCLUDE "P18F4680.inc"
;#ENDIF


	#IF GSI_CAN_SUPPORT
	
#include "rs232.inc"
#include "error.inc"
#include "can.inc"
#include "i2c.inc"		;only for debugging
#include "lcd.inc"



#include "CANPrTx.inc"

;for can.asm
	GLOBAL	ProcessCANMssg,GSISendCANRxMssg232Ascii,GSICANGetMsgFromR2CQ

;-------defined in gsi.asm
	EXTERN GlbDebugTempH,GlbDebugTempL

;--------defined in lcd.asm
	EXTERN GSIProcessRxByteCnEntry

;--------- variables all defined in CAN.asm
	EXTERN	CnErrorState,CnErrorState1,CnRxMssgId,CnRxBuff,CnRxBuffLength
	EXTERN	CnRxFlags,CnTxRecord,CnTxMssgId,CnTxBuff,CnTxBuffLength
	EXTERN	CnTxFlags,CnMode,CnTemp,CnTemp32,CnSyncCounter
	EXTERN	CnSlaveBase,CnThisNode
	
	EXTERN	CnCQRxNumInBuff,CnCQRxBuff,CnCQRxBuffEndLbl
	EXTERN	CnCQRxCounter,CnCQRxNIB,CnCQRxRCursor,CnLocalTemp

	EXTERN	CnCQRxR2NumInBuff,CnCQRxR2Buff,CnCQRxR2BuffEndLbl,CnCQRxR2RCursor
;from rs232.asm
	EXTERN R2RxRecordType,R2RxRecordNode
	;EXTERN R2RxRecord
	EXTERN R2TxRecordType,R2TxRecordNode
	;EXTERN R2TxRecord
	EXTERN DEBUGGSISendByte232

CAN_CODE	CODE
;---------------------------------------------------------------
;	processCANMssg
;main loop will check for receipt of CAN message, extract it from
;the Q and call this function.
;If any of the called functions using the jmp table need to alert the pc
;to the CAN message then they need to:
;bsf	GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX
;This will send the data to the pc
;No longer true?? : Arrived here through a goto from GSIStatusChangeCAN (can.asm)
;will return to main loop code
;Assumes:
;BSR:nothing
;Modifes:
;BSR, WREG
;See which filter caused the message to be accepted and process
;accordingly
;mask0 value for f0/f1
;f0/f1=11111111 11111111 11111111 11110000 	
;Accept 0-15 irrespective of filter value (GSI/User time critical)
;f0   =00000000 00000000 00000000 00010000 (0-15 in filter 0)
;f1   =00000000 00000000 00000000 00100000 (CAN_BASE+0-31 in filter 1)

;mask1 value for f2-f5:accept 0-31 (ignored as mask0 does the same)
;f2/f5=11111111 11111111 11111111 11100000
;Accept 0-31
;f2=00000000 00000000 00001111 100 00000 (accept 32 messages CAN_BASE+3968-3999)   
;f3=00000000 00000000 00001111 101 00000 (accept 32 messages CAN_BASE+4000-4031)
;f4=00000000 00000000 00001111 110 00000 (accept 32 messages CAN_BASE+4032-4063)
;f5=00000000 00000000 00001111 111 00000 (accept 32 messages CAN_BASE+4064-4095)

;---------------------------------------------------------------
ProcessCANMssg
	;message is in	CnRxxxx variables

	banksel	CnBank
	movf	CnRxFlags,W	;see which filter accepted this message
	andlw	CAN_RX_FILTER_BITS	;mask out non-filter bits		
	;bz	pcmF0			;Filter 0
	bnz	_pcmf1?
	goto	pcmF0
_pcmf1?:
	decf	WREG,W
	bz	pcmF1			;Filter 1
	decf	WREG,W
	bz	pcmF2			;Filter 2
	decf	WREG,W
	bz	pcmF3			;Filter 3
	decf	WREG,W
	bz	pcmF4			;Filter 4
	decf	WREG,W
	bz	pcmF5			;Filter 4
	
	IF DEBUG
	#IF GSI_LCD_SUPPORT
		banksel LdBank
		movlw	'!'
		call	GSILCDWriteChar
		banksel CnBank
	#ENDIF			
	ENDIF

	bsf	STATUS,C
	return
pcmF5:
	movlw	HIGH  f5_tbl	        ;fetch upper byte of jump table address
	movwf	PCLATH			;load into upper PC latch

	movlw	LOW CAN_GSI_BASE 	;d'224'	e0h ;CAN id LSB will be d'224' for GSI_BASE
	subwf	CnRxMssgId,W		;calc offset to the messages
	
	addwf	WREG,W			;2 byte 18f instructions, byte PC
	addwf	WREG,W			;Using goto's
	;WREG holds byte offset to correct goto
	;Now add offset to the jump table
	addlw	LOW  (f5_tbl)          ;calc state machine jump addr into W		
	btfsc	STATUS,C                ;skip if carry occured
	incf	PCLATH,F		;otherwise add carry
	movwf	PCL  		        ;index into state machine jump table
f5_tbl	;--- GSI Mssgs
	goto	CANRxGetSetSlaveId        ;Message type
	goto	CANRxTerminalMssg	;these all need to return
	goto	CANRxRS232Mssg
	goto	CANRxLCDMssg
	goto	CANRxKeypadMssg
	goto	CANRxSPIMssg
	goto	CANRxI2CMssg
	goto	CANRxDIOMssg
	goto	CANRxADCMssg
	goto	CANRxIOCTL
	goto	CANRxGSIIOCTL
	goto	CANRxErrorMssg
	goto	CANRxPingMssg
;----------------------------------------------------------------
;		Filter 4 CAN Messages
;Called from GSIStatusChangeCAN         
;----------------------------------------------------------------
pcmF4
;----------------------------------------------------------------
;		Filter 3 CAN Messages
;Called from GSIStatusChangeCAN         

	return
;----------------------------------------------------------------

pcmF3
;----------------------------------------------------------------
;		Filter 2 CAN Messages
;Called from GSIStatusChangeCAN         
;----------------------------------------------------------------
	return
pcmF2

	return


;----------------------------------------------------------------------
;	Filter 1 2nd highest priority
;f1   =00000000 00000000 00000000 00100000 (CAN_BASE+0-31 in filter 1)
;Messages CAN_BASE+0 to CAN_BASE+31
;CAN_BASE will be assigned by a master if this is a slave or
;assigned by a pc if this is a pc master (could be multi-master)
;or assigned by this code if this is a standalone master (not implemented)
;id's CAN_BASE+0 - CAN_BASE+15 User	
;id's CAN_BASE+16 - CAN_BASE+31 System	
;Called from GSIStatusChangeCAN         
;-----------------------------------------------------------------------
pcmF1

	movlw	HIGH  f1_tbl	        ;fetch upper byte of jump table address
	movwf	PCLATH			;load into upper PC latch

	movf	CnRxMssgId,W 		;Get mssg id (guaranteed to be 0-31)
	addwf	WREG,W			;2 byte 18f instructions, byte PC
	;WREG holds byte offset to correct goto
	;Now add offset to the jump table
	addlw	LOW  (f1_tbl)          ;calc state machine jump addr into W		
	btfsc	STATUS,C                ;skip if carry occured
	incf	PCLATH,F		;otherwise add carry
	movwf	PCL  		        ;index into state machine jump table
	;jump to processing for each state= I2CStateIndex value for each state
f1_tbl
	bra	RxIdF1_0	        ;Message type
	bra	RxIdF1_1	        ;Message type
	bra	RxIdF1_2	        ;Message type
	bra	RxIdF1_3	        ;Message type
	bra	RxIdF1_4	        ;Message type
	bra	RxIdF1_5	        ;Message type
	bra	RxIdF1_6	        ;Message type
	bra	RxIdF1_7	        ;Message type
	bra	RxIdF1_8	        ;Message type
	bra	RxIdF1_9	        ;Message type
	bra	RxIdF1_1	        ;Message type
	bra	RxIdF1_11	        ;Message type
	bra	RxIdF1_12	        ;Message type
	bra	RxIdF1_13	        ;Message type
	bra	RxIdF1_14	        ;Message type
	bra	RxIdF1_15	        ;Message type
	;GSI REserved 
	bra	RxIdF1_16	        ;Message type
	bra	RxIdF1_17	        ;Message type
	bra	RxIdF1_18	        ;Message type
	bra	RxIdF1_19	        ;Message type
	bra	RxIdF1_20	        ;Message type
	bra	RxIdF1_21	        ;Message type
	bra	RxIdF1_22	        ;Message type
	bra	RxIdF1_23	        ;Message type
	bra	RxIdF1_24	        ;Message type
	bra	RxIdF1_25	        ;Message type
	bra	RxIdF1_26	        ;Message type
	bra	RxIdF1_27	        ;Message type
	bra	RxIdF1_28	        ;Message type
	bra	RxIdF1_29	        ;Message type
	bra	RxIdF1_30	        ;Message type
	bra	RxIdF1_31	        ;Message type

;----- Filter 1 CAN messages --------
;Called from GSIStatusChangeCAN         
RxIdF1_0	        ;Message type
	goto	GSIUserCANF1_0
RxIdF1_1	        ;Message type
	goto	GSIUserCANF1_1
RxIdF1_2	        ;Message type
	goto	GSIUserCANF1_2
RxIdF1_3	        ;Message type
	goto	GSIUserCANF1_3
RxIdF1_4	       ;Message type
	goto	GSIUserCANF1_4
RxIdF1_5	        ;Message type
	goto	GSIUserCANF1_5
RxIdF1_6	        ;Message type
	goto	GSIUserCANF1_6
RxIdF1_7	        ;Message type
	goto	GSIUserCANF1_7
RxIdF1_8	        ;Message type
	goto	GSIUserCANF1_8
RxIdF1_9	        ;Message type
	goto	GSIUserCANF1_9
RxIdF1_10	        ;Message type
	goto	GSIUserCANF1_10
RxIdF1_11	        ;Message type
	goto	GSIUserCANF1_11
RxIdF1_12	        ;Message type
	goto	GSIUserCANF1_12
RxIdF1_13	        ;Message type
	goto	GSIUserCANF1_13
RxIdF1_14	        ;Message type
	goto	GSIUserCANF1_14
RxIdF1_15	        ;Message type
	goto	GSIUserCANF1_15
;GSI REserved 
RxIdF1_16	        ;Message type
RxIdF1_17	        ;Message type
RxIdF1_18	        ;Message type
RxIdF1_19	        ;Message type
RxIdF1_20	        ;Message type
RxIdF1_21	        ;Message type
RxIdF1_22	        ;Message type
RxIdF1_23	        ;Message type
RxIdF1_24	        ;Message type
RxIdF1_25	        ;Message type
RxIdF1_26	        ;Message type
RxIdF1_27	        ;Message type
RxIdF1_28	        ;Message type
RxIdF1_29	        ;Message type
RxIdF1_30	        ;Message type
RxIdF1_31	        ;Message type
	return
;---------------------------------------------------------------
	;Filter 0 messages
;----------------------------------------------------------------
;Time critical priority message received.
;CAN_ID 0-16 sytem level mssg's
;CAN_ID_SYNC			EQU	0
;CAN_ID_CRITICAL_ERROR		EQU	1
;CAN_ID_TIMESTAMP		EQU	2
;CAN_ID_SYSTEM_			EQU	3
;CAN_ID_4-11			EQU	4-11  USER ID's
;CAN_ID_SYSTEM_12-15		EQU	12-15 SYSTEM_ID's
;Called from GSIStatusChangeCAN         
;--------------------------------------------------------------
pcmF0	 
	movlw	HIGH  f0_tbl	        ;fetch upper byte of jump table address
	movwf	PCLATH			;load into upper PC latch

	movf	CnRxMssgId,W 		;Get mssg id (guaranteed to be 0-15)
	addwf	WREG,W			;2 byte 18f instructions, byte PC
	;WREG holds byte offset to correct goto
	;Now add offset to the jump table
	addlw	LOW  (f0_tbl)          ;calc state machine jump addr into W		
	btfsc	STATUS,C                ;skip if carry occured
	incf	PCLATH,F		;otherwise add carry
	movwf	PCL  		        ;index into state machine jump table
	;jump to processing for each state= I2CStateIndex value for each state
f0_tbl
	bra	rxIdSYNC	        ;Message type
	bra	rxIdCritcalError
	bra	rxIdTimestamp
	bra	rxIdReservedTC3		;TC3=TimeCritical3		
	bra	rxIdTC4			;User code
	bra	rxIdTC5			;User code
	bra	rxIdTC6			;User code	
	bra	rxIdTC7			;User code
	bra	rxIdTC8			;User code
	bra	rxIdTC9			;User code
	bra	rxIdTC10		;User code	
	bra	rxIdTC11		;User code
	bra	rxIdTC12		;System code
	bra	rxIdTC13		;System code
	bra	rxIdTC14		;System code
	bra	rxIdTC15		;System code

;----------------------------------------------------------------
;	Filter 0 CAN messages
;Called from GSIStatusChangeCAN         
;This node has received the message down the CAN
;----------------------------------------------------------------
;-------------------------------------------------
;	rxIdSYNC
;Received a CAN SYNC message
;-------------------------------------------------
rxIdSYNC	        ;Message type
	;call	GSICopyCANRxBuffToR2TxRecord
	#if ENABLE_CAN_FILTER_0_TO_RS232	;send/don't send sync messages. Off is useful for debugging
	;bsf	GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX
	#endif
	btg	LATC,1
	;movlw	's'
	;call	DEBUGGSISendByte232

	return
rxIdCritcalError
	;call	GSICopyCANRxBuffToR2TxRecord
	;bsf	GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX
	return
rxIdTimestamp
	;call	GSICopyCANRxBuffToR2TxRecord
	;bsf	GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX
	return
rxIdReservedTC3		;TC3=TimeCritical3 (reserved)		
	;call	GSICopyCANRxBuffToR2TxRecord
	bsf	GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX
	return
rxIdTC4	
		;User code
	goto	GSIUserCANF0_4
rxIdTC5			;User code
	goto	GSIUserCANF0_5
rxIdTC6			;User code	
	goto	GSIUserCANF0_6
rxIdTC7			;User code
	goto	GSIUserCANF0_7
rxIdTC8			;User code
	goto	GSIUserCANF0_8
rxIdTC9			;User code
	goto	GSIUserCANF0_9
rxIdTC10		;User code	
	goto	GSIUserCANF0_10
rxIdTC11		;User code
	goto	GSIUserCANF0_11	
rxIdTC12		;System code (reserved)
	return
rxIdTC13		;System code (reserved)
	return
rxIdTC14		;System code (reserved)
	return
rxIdTC15		;System code (reserved)
	return

;------------------------------------------------------------
;	GSISendCANRxMssg232Ascii
;
;Send the Message in CAN BANK to rs232
;------------------------------------------------------------
GSISendCANRxMssg232Ascii
;CAN_MSSG_STR			EQU	d'23' ;23 About to write a CAN mssg
;CAN_MSSG_ID_STR			EQU	d'24' ;24 mssg Id str
;CAN_MSSG_DATA_STR		EQU	d'25' ;25 mssg Data str
;CAN_MSSG_LENGTH_STR		EQU	d'26' ;26 mssg Data Length str
;CAN_MSSG_FLAG_STR		EQU	d'27' ;27 mssg Flag str

	btfss	GlbSIOMode,SIO_M_BINARY
	return

	NEWLINE_RS232

	movlw	CAN_MSSG_ID_STR
	banksel	R2Bank
	call	GSIWriteMessageRS232
	SPACE_RS232
	banksel	CnBank
	
	movf	CnRxMssgId,W
	call	GSISendAscii232

	movf	CnRxMssgId+1,W
	call	GSISendAscii232

	movf	CnRxMssgId+2,W
	call	GSISendAscii232

	movf	CnRxMssgId+3,W
	call	GSISendAscii232

	CHECK_BSR 1,'Y'

	NEWLINE_RS232
	movlw	CAN_MSSG_DATA_STR
	banksel	R2Bank
	call	GSIWriteMessageRS232	
	;NEWLINE_RS232
	SPACE_RS232

	banksel	CnBank
	movf	CnRxBuff,W
	call	GSISendAscii232	

	movf	CnRxBuff+1,W
	call	GSISendAscii232	

	movf	CnRxBuff+2,W
	call	GSISendAscii232	

	movf	CnRxBuff+3,W
	call	GSISendAscii232	

	movf	CnRxBuff+4,W
	call	GSISendAscii232	

	movf	CnRxBuff+5,W
	call	GSISendAscii232	

	movf	CnRxBuff+6,W
	call	GSISendAscii232	

	movf	CnRxBuff+7,W
	call	GSISendAscii232	

	NEWLINE_RS232
	movlw	CAN_MSSG_LENGTH_STR
	banksel	R2Bank
	call	GSIWriteMessageRS232	
	;NEWLINE_RS232
	SPACE_RS232

	banksel	CnBank
	movf	CnRxBuffLength,W
	call	GSISendAscii232	

	NEWLINE_RS232
	movlw	CAN_MSSG_FLAG_STR
	banksel	R2Bank
	call	GSIWriteMessageRS232	
	;NEWLINE_RS232
	SPACE_RS232

	banksel	CnBank
	movf	CnRxFlags,W
	call	GSISendAscii232	
	NEWLINE_RS232

	return
;---------------------------------------------------------------
;CAN Rx Mssg's are by definition for CAN_NODE_THIS, since they
;have been specifically sent to this nodes id
;---------------------------------------------------------------

;----------------------------------------------------------------
;		Received CAN  Messages
;----------------------------------------------------------------
;		CANRxGetSetSlaveId
;id=CAN_GSI_BASE
;d0:b0 CAN_ID_SET
;d0:b1 CAN_PC_PRESENT
;----------------------------------------------------------------
CANRxGetSetSlaveId        ;Message type
	btfsc	CnRxBuff,CAN_ID_SET
	bra	cgssi
;----- this code must be on a master -----
	;movlw	'C'
	;call	DEBUGGSISendByte232

	call	GSICANSetSlaveId	;GetId A slave wants a SLAVE_BASE
;The slave called with GET, we call SET
	IF DEBUG
		;movlw	'x'		
		;call	DEBUGGSISendByte232
	ENDIF
	;movlw	'I'
	;call	DEBUGGSISendByte232

	return				
;----------------------------------------------------------------
cgssi:
	;Set id, this code must be running on a slave
	call	GSICANSetSlave	;The received Slave id from the master
	IF DEBUG
		;movlw	'y'
		;call	DEBUGGSISendByte232
	ENDIF
	;movlw	'i'
	;call	DEBUGGSISendByte232

	return

CANRxTerminalMssg
	return

;---------------------------------------------------------------
;	CANRxRs232Mssg
;Received a CAN_ID_RS232_Mssg
;message is in CnRxBuff, length is in CnRxBuffLength
;Send the data down the rs232 line
;---------------------------------------------------------------
CANRxRS232Mssg
	banksel	CnBank
z1	bra	z1
	bra	z1
	;WDT here
	;call	SendMssgToRS232
	IF 0
	movf	CnRxBuffLength,W
	lfsr	FSR0,CnRxBuff
crsc1
	movf	POSTINC0,W	;get the CnBuffer item
	;call	GSISendByte232	;and send it down the rs232 line
	decfsz	CnRxBuffLength,F
	bra	crsc1
	ENDIF
	return

;-------------------------------------------------
;	CANRxLCDMssg
;Received a CAN_ID_LCD_MSSG
;CnRx
;--------------------------------------------------
CANRxLCDMssg
	;bra	CANRxLCDMssg	
	lfsr	FSR0,CnRxBuff	;point to the 8 data bytes
	movf	POSTINC0,W	;get byte and inc
;WREG holds type of LCD command
	bz	CnLCDInstruction	;0
	decf	WREG
	bz	CnLCDWriteStr	;1
	decf	WREG
	bz	CnLCDWriteChar	;2
	decf	WREG
	bz	CnLCDGetStr		;3
	decf	WREG			
	bz	CnLCDCtrJmpTable	;4
	decf	WREG
	bz	CnLCDGetCapabilities ;5
	return
;-----------------------------------------------------------------
;Read the LCD instructions from the record and despatch them
;Generic subfunction has been setup for SUCCESS
;-----------------------------------------------------------------
CnLCDInstruction
	;bra	CnLCDInstruction
#if GSI_LCD_SUPPORT
	movf	POSTINC0,W		;get byte and inc
	banksel LdBank
	call	GSILCDInstruction
	banksel	LdBank
	decfsz	CnRxBuffLength,F
	bra	CnLCDInstruction
#endif	
	return
;-----------------------------------------------------------------
;Read the chars from the record and despatch them
;?error check for length >8?? Best done at record assembly time
;-----------------------------------------------------------------
CnLCDWriteStr
CnLCDWriteChar

	decf	CnRxBuffLength,F	;step over the subfunction data R2RxRecordData[0]
	bz	r2lwc_exit	
r2lwc:
	movf	POSTINC0,W		;get byte and inc
	banksel	R2Bank
	call 	GSIProcessRxByteCnEntry
	banksel	CnBank
	decfsz	CnRxBuffLength,F
	bra	r2lwc
r2lwc_exit:
	return
CnLCDGetStr
	return
;-----------------------------------------------------------------
;Read the LCD Ctr codes from the record and despatch them
;Will also write the char if it is not a Ctr code
;-----------------------------------------------------------------
CnLCDCtrJmpTable
;Too dangerous to stuff the chars in the RxBuff as this is an IntTime
;function. Could disable RxInt but could then mix up these chars
;with newly Rx'd ones. A recipe for disaster. Much better to call the
;jmp table directly.
;Note this function will also write ascii text
	decf	CnRxBuffLength,F	;step over the subfunction data R2RxRecordData[0]
	bz		clcjt_exit			;no data with this function
ClJtNextCommand:
	movf	POSTINC0,W			;get byte and inc
	banksel	R2Bank
	call	GSIProcessRxByteCnEntry	;execute the function
	banksel	CnBank
	decfsz	CnRxBuffLength,F
	bra		ClJtNextCommand
clcjt_exit:	
	return

;------------------------------------------------------------------
;		CnLCDGetCapabilities
;Send back the num rows cols and other relevant data
;send data back in the 232 record if it on this node
;otherwise it will go back as an asynchronous record
;FSR0 points to data[1], this is free for sending the data back
;as the success fail is success
;We are in CnBank
;CAN must be running or we would not have received this message
;------------------------------------------------------------------

CnLCDGetCapabilities
		
	movlw	GSI_LCD_MESSAGE_GET_CAPABILITIES
	movwf	CnTxBuff
			
	movlw	HIGH LCD_COLS
	movwf	CnTxBuff+1
	movlw	LOW LCD_COLS
	movwf	CnTxBuff+2
	
	movlw	HIGH LCD_LINES
	movwf	CnTxBuff+3
	movlw	LOW LCD_LINES
	movwf	CnTxBuff+4
	
	movlw	0
	movwf	CnTxBuff+5				;zero next byte as we can use as byte of 8 flags, (0=graphics,1=xxx,)

	movlw	6					;5 bytes going back	
	movwf	CnTxBuffLength
	
	clrf	CnTxMssgId+3
	movlw	UPPER CAN_ID_LCD_MSSG
	movwf	CnTxMssgId+2
	movlw	HIGH CAN_ID_LCD_MSSG
	movwf	CnTxMssgId+1
	movlw	LOW CAN_ID_LCD_MSSG
	movwf	CnTxMssgId
;--------------Set flag to be Xtd data frame-------------
	movlw	CAN_TX_XTD_FRAME ;| CAN_TX_NO_RTR_FRAME
	movwf	CnTxFlags
;--------------------------------------------------------		
	clrwdt
_clgc:
	mCANPrTxSendMsg_IID_IDL_IF CnTxMssgId,CnTxBuff,CnTxBuffLength,CnTxFlags
        addlw   0x00		;Check for return value of 0 in W
        bz      _clgc        	;Buffer Full, Try again
	banksel	CnBank
	
	
	return	



;----------------------------------------------------------------
;	CANRxKeypadMssg
;Received a keypad message. 
;CAN_KD_MSSG_KEY_DN	EQU	0
;CAN_KD_MSSG_KEY_UP	EQU	1
;CAN_KD_MSSG_SET_MODE	EQU	2
;CAN_KD_MSSG_GET_MODE	EQU	3
;We are in Cn bank
;d0=subfunction
;d1=data
;d2=data...
;----------------------------------------------------------------
CANRxKeypadMssg
	;call	GSICopyCANRxBuffToR2TxRecord	;will rs232 Tx 
	;bsf	GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX
	banksel	CnBank

	movf	CnRxBuffLength,W
	bz	crk_exit
	
	lfsr	FSR0,CnRxBuff+1		;+0=CnNode of sender
	movf	CnRxBuff+1,W		;Subfunction
	bz	kd_down
	decf	WREG
	bz	kd_up
	decf	WREG
	bz	kd_set_mode
kd_getMode:
	;requires a can mssg sent back to the node that sent it	
	;thus we need to know the node of the caller
	;node is in CnRxBuff+1	
	return	
kd_down:
	;
	return	
kd_up:
	return	
kd_set_mode:
	return	
		
crk1
	movf	POSTINC0,W		;get the CnBuffer item

	decfsz	CnRxBuffLength,F
	bra	crk1
crk_exit:

	return


CANRxSPIMssg
	;bra CANRxSPIMssg
	return
;---------------------------------------------------------------
;	CANRxI2CMssg
;Received a CAN_ID_I2C_MESSAGE
;message is in CnRxBuff, length is in CnRxBuffLength
;add the contents of the CnRxBuff to the I2C buffer,then
;call the InI2C function to initiate the I2c interrupt sequence
;We are in Cn data bank
;---------------------------------------------------------------
CANRxI2CMssg
	;bra	CANRxI2CMssg
	;banksel	CnRxBuff
	IF GSI_I2C_SUPPORT
	movf	CnRxBuffLength,W
	bz	cric_exit

	;call	GSISendCANRxMssg232Ascii

	lfsr	FSR0,CnRxBuff
cric1
	movf	POSTINC0,W	;get the CnBuffer item
	call	GSIAddToI2CTxBuff	;will change to I2C bank	
	banksel	CnBank
	decfsz	CnRxBuffLength,F
	bra	cric1
	call	GSIInitiateI2CTransfer
	ENDIF	;GSI_I2C_support
	banksel	CnBank
cric_exit:
	return

CANRxDIOMssg
	return
CANRxADCMssg

	return

;----------------------------------------------------------------
;		CANRxIOCTL
;Received a CAN IOCTL message
;Need to configure the CAN module on this node
;or send back CAN module data from this node
;----------------------------------------------------------------

;CAN_IOCTL_GET_NODE		EQU 0
;CAN_IOCTL_GET_MODE		EQU 1
;CAN_IOCTL_SET_MODE		EQU 2
;CAN_IOCTL_GET_NODE_N2M		EQU 3
;CAN_IOCTL_GET_MODE_N2M		EQU 4
;CAN_IOCTL_SET_MODE_N2M		EQU 5

;CAN_IOCTL_MODE_NORMAL		EQU 0
;CAN_IOCTL_MODE_DISABLE		EQU 1
;CAN_IOCTL_MODE_LOOPBACK		EQU 2
;CAN_IOCTL_MODE_LISTEN		EQU 3
;CAN_IOCTL_MODE_CONFIGURATION	EQU 4
;CAN_IOCTL_MODE_ERROR_RECOGNITION EQU 5

;We are in Cn bank
;d0=subfunction
;d1=data
;d2=data...
;----------------------------------------------------------------
CANRxIOCTL
	banksel	CnBank

	movf	CnRxBuffLength,W
	bz	crk_exit
	
	;lfsr	FSR0,CnRxBuff+1		;+0=CnNode of sender
	movf	CnRxBuff+1,W		;Subfunction
	bz	ioctlGetNode
	decf	WREG
	bz	ioctlGetMode
	decf	WREG
	bz	ioctlSetMode
	decf	WREG
	bz	ioctlNewNode			;found a new node
	;---node->master returns
	decf	WREG
	bz	ioctlGetNodeN2M		;returning remote Node
	decf	WREG
	bz	ioctlGetModeN2M		;returning remote Mode
	decf	WREG
	bz	ioctlSetModeN2M		;returning remote Mode
	decf	WREG
	bz	ioctlNewNodeN2M			;found a new node

#IF GSI_LCD_SUPPORT
	IF DEBUG
		banksel	LdBank
		movlw	'!'
		call	GSILCDWriteChar
		banksel CnBank
	ENDIF
#ENDIF	
	;GS error, should we send a CAN ERROR
	movlw	GSI_ERR_RANGE
	call	GSICANSendErrorMssg
	bra	_cri_exit
ioctlGetNode:

	movff	CnThisNode,CnTxBuff	;node in buff+0
	movlw	CAN_IOCTL_GET_MODE_N2M
	movwf	CnTxBuff+1		;subfunction in buff+1
	movlw	2		;length
	bra	ioctlSendMssg
	return	
ioctlGetMode:

	;send back CAN Mode (microchip CAN mode)
	movff	CnThisNode,CnTxBuff
	movlw	CAN_IOCTL_GET_MODE_N2M
	movwf	CnTxBuff+1

	movlw	CAN_IOCTL_MODE_NORMAL
	movwf	CnTxBuff+2
	;-also send back the GSI Mode
	movf	CnMode,W
	movwf	CnTxBuff+3

	;-length
	movlw	4		;node+MChip Mode+GSIMode
	bra	ioctlSendMssg

ioctlNewNode:
;Master has detected a new node, we can do something or ignore it
;send back CAN Mode (microchip CAN mode)
;No need to send a message back
	return


ioctlSendMssg:
;WREG=length
;--------Are we running--------------------------
	btfss	CnMode,CAN_MODE_RUNNING
	bra	_cri_exit
;--------ok CAN is running so tx the mssg
	;-
	movwf	CnTxBuffLength
	clrf	CnTxMssgId+3
	movlw	UPPER CAN_ID_IOCTL_MSSG
	movwf	CnTxMssgId+2
	movlw	HIGH CAN_ID_IOCTL_MSSG
	movwf	CnTxMssgId+1
	movlw	LOW CAN_ID_IOCTL_MSSG
	movwf	CnTxMssgId
;--------------Set flag to be Xtd data frame-------------
	movlw	CAN_TX_XTD_FRAME ;| CAN_TX_NO_RTR_FRAME
	movwf	CnTxFlags
;--------------------------------------------------------		
	;bsf	GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX
;send it down this nodes rs232 if it is attached	
	clrwdt
_cri:
	mCANPrTxSendMsg_IID_IDL_IF CnTxMssgId,CnTxBuff,CnTxBuffLength,CnTxFlags
        addlw   0x00		;Check for return value of 0 in W
        bz      _cri        	;Buffer Full, Try again
	banksel	CnBank

_cri_exit:
	return	;will return to main loop (possibly user code?)
ioctlSetMode:
;unsupported function
	movlw	GSI_ERR_UNSUPPORTED
	call	GSICANSendErrorMssg
	return	
;-----------------------------------------------------------------------
;data returned from a node (uses same CAN_ID with different subfunction)
;N2M refers to Node to Master. A node has responded to an ioctl from
;the master.
;-----------------------------------------------------------------------
ioctlGetNodeN2M
	;call	GSICopyCANRxBuffToR2TxRecord	;will rs232 Tx 
	;bsf	GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX
	return
;------------------return same data (mode or previous mode)
ioctlGetModeN2M
ioctlSetModeN2M

	;IF DEBUG
		;banksel	LdBank
		;movlw	'M'
		;call	GSILCDWriteChar
		;banksel CnBank
	;ENDIF
	;call	GSICopyCANRxBuffToR2TxRecord	;will rs232 Tx 
	;bsf	GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX

	return
ioctlNewNodeN2M:
;a slave node (if we are here then this is the master) has detected
;a new node.
;currently, only the master allocated nodes, so it always has full
;knowledge of all nodes on the network
;Though for robust behaviour, a node should probably send one of these
;after it comes on-line. It should tell us what type of reset was responsible?
;NOTE
;For orderly access to the CAN, a device must look for the SYNC from the master
;before doing anything, thus all transactions will be through the master.

	return
;--------end of CANRxIOCTL---------------------------------



;----------------------------------------------------------------
;		CANRxGSIIOCTL
;Received a GSI IOCTL message
;Need to depatch the command on this node
;or send the rx'd data to the pc through the rs232
;----------------------------------------------------------------
;CAN_GSI_IOCTL_GET_REG		EQU 0
;CAN_GSI_IOCTL_SET_REG		EQU 1
;CAN_GSI_IOCTL_1			EQU 2
;CAN_GSI_IOCTL_2			EQU 3
;CAN_GSI_IOCTL_3			EQU 4
;CAN_GSI_IOCTL_4			EQU 5
;CAN_GSI_IOCTL_5			EQU 6
;CAN_GSI_IOCTL_6			EQU 7
;CAN_GSI_IOCTL_7			EQU 8
;CAN_GSI_IOCTL_LAST		EQU 8

;We are in Cn bank
;d0=subfunction
;d1=data
;d2=data...
;----------------------------------------------------------------
CANRxGSIIOCTL
	banksel	CnBank

	movf	CnRxBuffLength,W
	bz	crk_exit
	
	movf	CnRxBuff+1,W		;Subfunction
	bz	ioctlGSIGetReg
	decf	WREG
	bz	ioctlGSISetReg
	decf	WREG
	bz	ioctlGSI_1
	;---node->master returns
	decf	WREG
	bz	ioctlGSIGetRegN2M		;returning remote Node
	decf	WREG
	bz	ioctlGSISetRegN2M		;returning remote Mode
	decf	WREG
	bz	ioctlGSI_1N2M		;returning remote Mode
	IF DEBUG
	#IF GSI_LCD_SUPPORT
		banksel	LdBank
		movlw	'!'
		call	GSILCDWriteChar
		banksel CnBank
	#ENDIF		
	ENDIF
	;GS error, should we send a CAN ERROR
	movlw	GSI_ERR_RANGE
	call	GSICANSendErrorMssg
	bra	_igsm_exit
ioctlGSIGetReg:
	;12 bit address is in CnRxBuff+2,+3
;Set the fsr0,1 reg, then do an indf.
;actually, can always send back 6 regs

	movff	CnThisNode,CnTxBuff	;node in buff+0
	movlw	CAN_GSI_IOCTL_GET_REG_N2M
	movwf	CnTxBuff+1		;subfunction in buff+1

	movf	CnRxBuff+2,W		;get the HIGH byte ofaddress to read 
	movwf	FSR0H
	movf	CnRxBuff+3,W
	movwf	FSR0L
	movlw	6
	lfsr	FSR1,CnTxBuff+2		;destination
_iggr_1
	movff	POSTINC0,POSTINC1
	decf	WREG
	bnz	_iggr_1

	movlw	8		;length
	bra	ioctlSendMssg
	return	
ioctlGSISetReg:

	;IF DEBUG
		;banksel	LdBank
		;movlw	'm'
		;call	GSILCDWriteChar
		;banksel CnBank
	;ENDIF

	;send back CAN Mode (microchip CAN mode)
	;movff	CnThisNode,CnTxBuff
	movlw	CAN_GSI_IOCTL_SET_REG_N2M
	movwf	CnTxBuff+1

	;movlw	CAN_IOCTL_MODE_NORMAL
	;movwf	CnTxBuff+2
	;-also send back the GSI Mode
	;movf	CnMode,W
	;movwf	CnTxBuff+3

	;-length
	movlw	2		;currently no data
	bra	ioctlSendMssg
ioctlGSI_1
	return

ioctlGSISendMssg:
;WREG=length
;--------Are we running--------------------------
	btfss	CnMode,CAN_MODE_RUNNING
	bra	_igsm_exit
;--------ok CAN is running so tx the mssg
	;-
	movwf	CnTxBuffLength
	clrf	CnTxMssgId+3
	movlw	UPPER CAN_ID_GSI_IOCTL_MSSG
	movwf	CnTxMssgId+2
	movlw	HIGH CAN_ID_GSI_IOCTL_MSSG
	movwf	CnTxMssgId+1
	movlw	LOW CAN_ID_GSI_IOCTL_MSSG
	movwf	CnTxMssgId
;--------------Set flag to be Xtd data frame-------------
	movlw	CAN_TX_XTD_FRAME ;| CAN_TX_NO_RTR_FRAME
	movwf	CnTxFlags
;--------------------------------------------------------		
	;bsf	GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX
;send it down this nodes rs232 if it is attached	
	clrwdt
_igsm1:
	mCANPrTxSendMsg_IID_IDL_IF CnTxMssgId,CnTxBuff,CnTxBuffLength,CnTxFlags
        addlw   0x00		;Check for return value of 0 in W
        bz      _igsm1        	;Buffer Full, Try again
	banksel	CnBank

_igsm_exit:
	return	;will return to main loop (possibly user code?)
;-----------------------------------------------------------------------
;data returned from a node (uses same CAN_ID with different subfunction)
;-----------------------------------------------------------------------
ioctlGSIGetRegN2M
	;call	GSICopyCANRxBuffToR2TxRecord	;will rs232 Tx 
	;bsf	GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX
	return
;------------------return same data (mode or previous mode)
ioctlGSISetRegN2M
ioctlGSI_1N2M

	
	;IF DEBUG
		;banksel	LdBank
		;movlw	'M'
		;call	GSILCDWriteChar
		;banksel CnBank
	;ENDIF
	;call	GSICopyCANRxBuffToR2TxRecord	;will rs232 Tx 
	;bsf	GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX

	return

;--------end of CANRxIOCTL---------------------------------


;----------------------------------------------------------
;	CANRxErrorMssg
;----------------------------------------------------------
CANRxErrorMssg
	;banksel	CnBank

	;movf	CnRxBuffLength,W
	;bz	crem_exit
	
	;Currently don't need to do anything as the message will be sent to the rs232
	;port if we are an rs232 master
;crem_exit


	return	

;-----End CANRxErrorMssg-----------------------------------

;----------------------------------------------------------
;	CANRxPingMssg
;another node wants to know if we are listening to this base
;address
;data[0]=node of sender
;data[1]=CAN_PING_SEND or CAN_PING_RECEIVED
;If data[1]==CAN_PING_SEND then we must respond with a PING of
;our own, placing our node in data[0] and CAN_PING_RECEIVED in data[1]
;If data[1]=CAN_PING_RECEIVED, then this is a return ping. Another part
;of this code has Pinged the sender. We ignore the mssg as it will be sent
;to rs232 port.
;This prevents an endless loop of PING's on the CAN!
;To respond we just need to extract the sender node, and send
;back the contents of the data buffer
;node of sender is data[0]
;----------------------------------------------------------
CANRxPingMssg
	banksel	CnBank
	movlw	CAN_PING_RECEIVED	;See if this is a ping 'echo'
	subwf	CnRxBuff+1			;data[1]
	bz		_crpm_ignore
;-----place the node in CnTxMssgID+1 (an 8 bit shift)----
	movff	CnRxBuff,CnTxMssgId+1	;8 bit shift of senders node->Id
;Now shift the node another 4 bits left for message_id. Already shifted
;by 8 from the initial load, so total of 12 bit shift 
;-----------performa 24 bit rol,4-------------------- 
	movlw	4
	clrf	CnTxMssgId
	clrf	CnTxMssgId+2
	clrf	CnTxMssgId+3
	bcf		STATUS,C			;mustn't forget to clear the carry flag!
_crpm2
	rlcf	CnTxMssgId+1
	rlcf	CnTxMssgId+2
	rlcf	CnTxMssgId+3
	decfsz	WREG
	bra	_crpm2
;CnTxMssgId hold the node of recipient correctly shifted
;------Or in the PING function---------
	movlw	UPPER CAN_ID_PING_MSSG
	iorwf	CnTxMssgId+2,F
	movlw	HIGH CAN_ID_PING_MSSG
	iorwf	CnTxMssgId+1,F
	movlw	LOW CAN_ID_PING_MSSG
	iorwf	CnTxMssgId,F
;---send back this node as data[0]-------------
	movff	CnThisNode,CnTxBuff	
;---send back CAN_PING_RECEIVED to indicate this is an echo---
	movlw	CAN_PING_RECEIVED
	movwf	CnTxBuff+1	
;---Set length of frame----------
	movlw	2				;currently always 2 	
	movwf	CnTxBuffLength
;--------------Set flag to be Xtd data frame-------------
	movlw	CAN_TX_XTD_FRAME ;| CAN_TX_NO_RTR_FRAME
	movwf	CnTxFlags
;--------------------------------------------------------		
	clrwdt
_crpm1:
	mCANPrTxSendMsg_IID_IDL_IF CnTxMssgId,CnTxBuff,CnTxBuffLength,CnTxFlags
        addlw   0x00		;Check for return value of 0 in W
        bz      _crpm1        	;Buffer Full, Try again
_crpm_ignore        
	banksel	CnBank

    movlw	'p'
	call DEBUGGSISendByte232

	return	

;-----End CANRxErrorMssg-----------------------------------



;--------------------------------------------------------------
;SendMssgToRS232
;We received a CAN mssg requesting the data be sent to the rs232 
;line on this node.
;Rx'd CAN mssg is in CnRxxx variables
;Requires: GlbSIOMOde:SIO_M_BINARY
;Assumes:
;BSR=NOTHING
;--------------------------------------------------------------
SendMssgToRS232
;R2TxRecordType		RES 1	;A binary Mssg
;R2TxRecordNode		RES 1	;the node this data is for
;R2TxRecordLength		RES 1   ;the number of bytes in record
;R2TxRecord		RES 8	;the data 0-8 bytes
;R2TxRecordFlags	RES 1	;Where we are in assembly of record

;CnRxMssgId	RES	4		;the RX mess id
;CnRxBuff        RES     08		;The data bytes
;CnRxBuffLength  RES     01		;The data length
;CnRxFlags       RES     01

	btfss	GlbSIOMode,SIO_M_BINARY
	return			;must be in binary mode

	;call	GSICopyCANRxBuffToR2TxRecord
	;bsf	GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX
	;
	
	movlw	RT_GSI_R2_RX_CAN_MSSG_NP	;RecordType
	movff	WREG,R2TxRecordType
	;This is potentially a 9 byte buffer as flag appended to buff
	
	btfss	GlbSIOStatus1,SIO_S1_MASTER
	banksel	R2Bank
	;call	GSIRS232ReqMaster		;We need to be 232 master

	return


;------------------------------------------------------------------------
;	GSCANGetFromCQ
;FSR0=pointer to memory to store the CAN mssg
;circular buffer for CAN msg's
;Assumes CnBank
;returns:
;STATUS NC NZ no error data in buffer
;STATUS NC Z no data in buffer
;STATUS C on error (W=error code)
;
;NOTE:
;The following variables can be changed in the CAN Rx interrupt handler
;If they need to be used then low priority ints must be disabled or a local copy
;must be used. This function should not change any of these variables except 
;CnCQRxNumInBuff. This must only be done after the mssg has been fully copied and 
;for safety should be performed in a single instruction
;_CnDataLength
;CnCQRxNumInBuff
;_CnvReg1_O
;CnCQRxWCursor
;_CnAddToCQCounter
;total length of mssg is id(4)+length(1)+data(0-8)+flags(1)
;fixed length=6 bytes
;Stored in this format:
;id:4 bytes
;length:1 byte
;data:0-8 bytes
;flags:1 byte

;Returns in this format
;CnRxMssgId	RES	4		;the RX mess id
;CnRxBuff        RES     08		;The data bytes
;CnRxBuffLength  RES     01		;The data length
;CnRxFlags       RES     01		;The flags

;CnCQRxCounter
;CnCQRxNIB
;------------------------------------------------------------------------

GSICANGetMsgFromCQ
	DW	0xffff
	banksel	CnBank
	movf	CnCQRxNumInBuff,W
	bz	gcgmfc_empty_exit	;no data	
	movwf	CnCQRxNIB		;local num in buffer
	IF DEBUG
;do a check to ensure we have at least a 0 data sized mssg here
	movlw	CQ_CAN_MSSG_FIXED_MSSG_SIZE
	subwf	CnCQRxNIB,F		;subtract fixed size
	movf	CnCQRxNIB,W		;load wreg
	bnc	gcgfc_frame_error	;mchip interesting C flag usage
	ENDIF
;ok we have at least CQ_CAN_MSSG_FIXED_MSSG_SIZE in the Q
	;Set up buffer removal point
	movff	CnCQRxRCursor,FSR1L
	movlw	HIGH CnCQRxBuff	
        movwf   FSR1H

	movlw	CAN_MSSG_MAX_SIZEOF_ID		;4 bytes 	
	movwf	CnCQRxCounter
gcgfc1:
;read id
	movff	 POSTINC1,POSTINC0
	
	movlw	CnCQRxBuffEndLbl		;address of end of buff. Keep in W for quicker access
	;movlw	CnCQRxBuffEnd			;end of buff
	
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxBuff			;start pos of buffer	
	decfsz	CnCQRxCounter
	bra	gcgfc1
;FSR1 points to length, FSR0 points to data buff
;NIB has already had the fixed length subtracted from it
	movff	POSTINC1,CnCQRxCounter	;Hopefully the length byte!
	;was a bug here. Forgot to check for a buffer wrap around!
	movlw	CnCQRxBuffEndLbl
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxBuff			;start pos of buffer	

;if length is > NIB or length >8 then we have an error. If this occurs
;then we should reset the CAN? Must have a foolproof way of
;letting the pc know. The rs232 CQ has never failed in testing. This one
;shouldn't either. Any transmission errors should be picked up by the
;CAN module
	IF DEBUG
	movlw	CAN_MSSG_MAX_DATA_SIZE+1
	subwf	CnCQRxCounter,W		;length byte is > max
	bc	gcgfc_frame_error

	movf	CnCQRxCounter,W		;reload length byte
	subwf	CnCQRxNIB,F		;NIB already has fixed length subtracted
	bnc	gcgfc_frame_error_length
	ENDIF
;id has been transfered, length is in CnCQRxCounter, it appears valid
	movlw	CAN_MSSG_MAX_DATA_SIZE
	addwf	FSR0L,F			;point to length
	movff	CnCQRxCounter,INDF0	;store the length
	;WREG still holds DATA_SIZE
	subwf	FSR0L,F			;restore pointer to data buffer
	
;Now ready to move the data (if any)
;FSR0= dst data buffer, FSR1=src data buffer
	movf	CnCQRxCounter,W		;counter	
	bz	gcgfc_no_data
	movwf	CnLocalTemp		;save length byte
gcgfc2:
	movff	POSTINC1,POSTINC0

	movlw	CnCQRxBuffEndLbl		;address of end of buff. Keep in W for quicker access
	;movlw	CnCQRxBuffEnd
	
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxBuff			;start pos of buffer	

	decfsz	CnCQRxCounter
	bra	gcgfc2	
gcgfc_no_data
;flag remains to be be moved
;FSR0=data buffer(0-8 or flag) , FSR1=src:flag	
;need to explicitly set FSR0 to flag
;WREG holds counter, so subtract from MAX_DATA_SIZE, and add to FSR0
	;subfwb
	movf	CnLocalTemp,W
	movwf	CnCQRxCounter	;store original length
	negf	WREG		;8=-8 1=-1
	addlw	CAN_MSSG_MAX_DATA_SIZE+1	;+1 to jump over length byte
	addwf	FSR0L
	movff	POSTINC1,INDF0
;adjust RCursor
	movlw	CnCQRxBuffEndLbl		;address of end of buff. Keep in W for quicker access
	;movlw	CnCQRxBuffEnd
	
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxBuff			;start pos of buffer	
;-update RCursor
	movff	FSR1L,CnCQRxRCursor
;Now adjust NIB
	movf	CnCQRxCounter,W			;retrieve data length
	addlw	CQ_CAN_MSSG_FIXED_MSSG_SIZE	;total length
	;-finally update the number in buffer variable
	subwf	CnCQRxNumInBuff
;gfcq_exit
	bcf	STATUS,Z			;not empty error
	bcf	STATUS,C			;no error
gcgmfc_empty_exit:				;Z set on empty error
	retlw 0
	DW	0xffff

gcgfc_frame_error:
	bsf	STATUS,C
	bcf	STATUS,Z
	retlw	FATAL_ERROR_CQ_READ_FRAME
gcgfc_frame_error_length:
	bsf	STATUS,C
	bcf	STATUS,Z
	retlw	FATAL_ERROR_CQ_READ_LENGTH

;****************************************************
;	GSICANGetMsgFromR2CQ
;Currently a duplicate of above CQ retrieval function
;Need to write a full indirect version, taking the address
;of the CQ in a FSR
;****************************************************

GSICANGetMsgFromR2CQ
	DW	0xffff
	banksel	CnBank
	movf	CnCQRxR2NumInBuff,W
	bz	gcgmfrc_empty_exit	;no data	
	movwf	CnCQRxNIB		;local num in buffer, can share with all 
	IF DEBUG
;do a check to ensure we have at least a 0 data sized mssg here
	movlw	CQ_CAN_MSSG_FIXED_MSSG_SIZE
	subwf	CnCQRxNIB,F		;subtract fixed size
	movf	CnCQRxNIB,W		;load wreg
	bnc	gcgfrc_frame_error	;mchip interesting C flag usage
	ENDIF
;ok we have at least CQ_CAN_MSSG_FIXED_MSSG_SIZE in the Q
	;Set up buffer removal point
	movff	CnCQRxR2RCursor,FSR1L
	movlw	HIGH CnCQRxR2Buff	
        movwf   FSR1H

	movlw	CAN_MSSG_MAX_SIZEOF_ID		;4 bytes 	
	movwf	CnCQRxCounter
gcgfrc1:
;read id
	movff	 POSTINC1,POSTINC0
	
	movlw	CnCQRxR2BuffEndLbl		;address of end of buff. Keep in W for quicker access
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxR2Buff			;start pos of buffer	
	decfsz	CnCQRxCounter
	bra	gcgfrc1
;FSR1 points to length, FSR0 points to data buff
;NIB has already had the fixed length subtracted from it
	movff	POSTINC1,CnCQRxCounter	;Hopefully the length byte!
		;was a bug here. Forgot to check for a buffer wrap around!
	movlw	CnCQRxR2BuffEndLbl
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxR2Buff			;start pos of buffer	

;if length is > NIB or length >8 then we have an error. If this occurs
;then we should reset the CAN? Must have a foolproof way of
;letting the pc know. The rs232 CQ has never failed in testing. This one
;shouldn't either. Any transmission errors should be picked up by the
;CAN module
	IF DEBUG
	movlw	CAN_MSSG_MAX_DATA_SIZE+1
	subwf	CnCQRxCounter,W		;length byte is > max
	bc	gcgfrc_frame_error

	movf	CnCQRxCounter,W		;reload length byte
	subwf	CnCQRxNIB,F		;NIB already has fixed length subtracted
	bnc	gcgfrc_frame_error_length
	ENDIF
;id has been transfered, length is in CnCQRxCounter, it appears valid
	movlw	CAN_MSSG_MAX_DATA_SIZE
	addwf	FSR0L,F			;point to length
	movff	CnCQRxCounter,INDF0	;store the length
	;WREG still holds DATA_SIZE
	subwf	FSR0L,F			;restore pointer to data buffer
	
;Now ready to move the data (if any)
;FSR0= dst data buffer, FSR1=src data buffer
	movf	CnCQRxCounter,W		;counter	
	bz	gcgfrc_no_data
	movwf	CnLocalTemp		;save length byte
gcgfrc2:
	movff	POSTINC1,POSTINC0

	movlw	CnCQRxR2BuffEndLbl		;address of end of buff. Keep in W for quicker access
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxR2Buff			;start pos of buffer	

	decfsz	CnCQRxCounter
	bra	gcgfrc2	
gcgfrc_no_data
;flag remains to be be moved
;FSR0=data buffer(0-8 or flag) , FSR1=src:flag	
;need to explicitly set FSR0 to flag
;WREG holds counter, so subtract from MAX_DATA_SIZE, and add to FSR0
	;subfwb
	movf	CnLocalTemp,W
	movwf	CnCQRxCounter	;store original length
	negf	WREG		;8=-8 1=-1
	addlw	CAN_MSSG_MAX_DATA_SIZE+1	;+1 to jump over length byte
	addwf	FSR0L
	movff	POSTINC1,INDF0
;adjust RCursor
	movlw	CnCQRxR2BuffEndLbl		;address of end of buff. Keep in W for quicker access
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxR2Buff			;start pos of buffer	
;-update RCursor
	movff	FSR1L,CnCQRxR2RCursor
;Now adjust NIB
	movf	CnCQRxCounter,W			;retrieve data length
	addlw	CQ_CAN_MSSG_FIXED_MSSG_SIZE	;total length
	;-finally update the number in buffer variable
	subwf	CnCQRxR2NumInBuff
;gfcq_exit
	bcf	STATUS,Z			;not empty error
	bcf	STATUS,C			;no error
gcgmfrc_empty_exit:				;Z set on empty error
	retlw 0
	DW	0xffff

gcgfrc_frame_error:
	bsf	STATUS,C
	bcf	STATUS,Z
	retlw	FATAL_ERROR_CQ_READ_FRAME
gcgfrc_frame_error_length:
	bsf	STATUS,C
	bcf	STATUS,Z
	retlw	FATAL_ERROR_CQ_READ_LENGTH



	#ENDIF ;GSI_CAN_SUPPORT
	END

