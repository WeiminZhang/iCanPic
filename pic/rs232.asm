;******************************************************************************
;* @file    rs232.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************


;rs232
#define IN_RS232.ASM

;#include <G:\prog\mplab\MPLAB IDE\MCHIP_Tools\p18f458.inc>
;#include "p18xxx8.inc"
#include "GSI.inc"

;#IF DEVICE == _18F458
;#include "p18xxx8.inc"
;#ELSE
;#INCLUDE "P18F4680.inc"
;#ENDIF

 
#include "lcd.inc"
#include "keypad.inc"
#include "rs232.inc"
#include "can.inc"
	#IF GSI_CAN_SUPPORT
#include "CANPrTx.inc"
	#ENDIF
;from MastSlav.asm
	
;CAN_SYNC_PERIOD_USEC	EQU	100000	;100,000us,100msec, 10 SYNCS/SEC
;CAN_SYNC_COUNTER	EQU	CAN_SYNC_PERIOD_USEC/1000000; /PERIOD_INSTRUCTION


;---------------------------------------------------------------------

	GLOBAL	R2RxByte,R2BTemp
	GLOBAL R2DebugCode
	GLOBAL	R2RxRecordCursor,R2RxNumInBuff
	GLOBAL	R2RxRecordType,R2RxRecordNode,R2RxRecordLength,R2RxRecordData
	GLOBAL	R2ErrorState
	GLOBAL RS232SendReSyncNulls
	IF DEBUG
	GLOBAL	DEBUGGSISendByte232,DEBUGGSISendBinary232ISR
	GLOBAL DEBUGGSISendBinary232
	GLOBAL R2CurrentFunctionThisNode

	ENDIF
;---------------------------------
;	definition in	gsi.asm
;----------------------------------
	EXTERN	GlbCompOffset,PgmMssgTbl,GlbClock

;---------------------------------------------
;	definitions in	lcd.asm
;---------------------------------------------
	EXTERN GSIProcessRxByte_esc_code,GSIProcessRxByte_ctr_code
;--------------------
;From TestGSI.asm
;--------------------
	EXTERN	GlbLocalTempL,GlbLocalTempH
;-------------------------------------------------
;	from can.asm
#IF GSI_CAN_SUPPORT
	EXTERN CnMode,CnThisNode,CnTxBuff
#endif	
;--------------------------------------------------
;-------------------------------------------------
;	from R2Master.asm
;--------------------------------------------------
	EXTERN R2TxBuff,R2TxWCursor,R2TxRCursor,R2TxBuffEnd,R2TxNumInBuff
;--------------------------------------------------------
;	From debug.asm
;--------------------------------------------------------

RS232_GLOBAL_DATA	UDATA_ACS

GlbSIOMode		RES 1
GlbSIOMode1		RES 1
GlbSIOStatus		RES 1	  
GlbSIOStatus1		RES 1	  
GlbSIOStatus2		RES 1

RS232_DATA	UDATA
R2Bank			RES 0
R2BTemp			RES 1
R2BTemp1		RES 1
R2BCounter		RES 1

R2RxByte		RES 1

;These fields should not change order as we might refer to them
;using indirect addressing
R2RxNumInBuff 		RES 1	;GSXXX 13/4/07 changed order of numInBuff and BuffEnd 
R2RxBuffEnd 		RES 1
R2RxWCursor 		RES 1
R2RxRCursor 		RES 1
R2RxBuff 		RES RX_BUFF_SIZE				


R2BSR			RES 1
R2BTempISR		RES 1
R2BTemp1ISR		RES 1
R2BCounterISR		RES 1
;---------------------------------------------------------------
;R2RxRecord		RES 0	;label for this structure
R2RxRecordType		RES 1	;A binary Mssg
R2RxRecordNode		RES 1	;the node this data is for
R2RxRecordLength	RES 1   ;the number of bytes in record
R2RxRecordData		RES R2_MAX_RX_RECORD_DATA ;the data 0-14 bytes
;--The above are Tx'd Rx'd down the rs232.
;final variable is used in binary record reception
R2RxRecordCursor	RES 1	;Where we are in assembly of record
;---------------------------------------------------------------
R2AsciiTemp		RES 1	
R2ErrorState		RES 1 

;--------------------------------------------------------
;d0:length
;d1=GSI_RS232_MESSAGE_SUCCESS/GSI_RS232_MESSAGE_FAILURE
;d2=other data. Error codes or returned data
;call R2SendProcessRecordSuccessFail function
R2SubfunctionRecord		RES 0	;no memory reserved here
R2SubfunctionRecordLength 	RES 1
R2SubfunctionRecordData		RES GSI_RS232_MAX_SUCCESS_FAIL_DATA ;currently 8 bytes max
R2CurrentFunctionThisNode	RES 1	;TRUE call is for this node
					;FALSE call is for remote node

;-------------------------------------------------------

RS232	CODE

;----------------------------------------------
;	GSIRS232IOCTL
;an rs232 record 
;----------------------------------------------
GSIRS232IOCTL

	return

;------------------------------------------
;	GSISetRS232AsciiMode
;------------------------------------------
GSISetRS232AsciiMode
	#IF GSI_LCD_SUPPORT

	movlw	0x40+LCD_COLS-1
	LCD_DDRAM_ADDRESS
	movlw	'A'
	call	LCDWriteChar
	#ENDIF ;GSI_LCD_SUPPORT

	bcf	GlbSIOMode,SIO_M_BINARY
	;allow fall through to FlushBuffer
	bra	GSIFlushR2RxBuffer
	;don't allow fall through for safety sake, in case
	;code is accidentally inserted here!
;-----------------------------------------------------
;GSIFlushR2RxBuffer
;Empty the Rx buffer
;------------------------------------------------------
GSIFlushR2RxBuffer
	banksel	R2RxBuff
	;need to disable Rx ints!
frrb_1
	bcf	INTCON,GIEL	;Rx int is low priority
	btfsc	INTCON,GIEL
	bra	frrb_1

	movlw	R2RxBuff
	movwf	R2RxWCursor
	movwf	R2RxRCursor
	clrf	R2RxNumInBuff

	bsf	INTCON,GIEL	;enable usart rx int

	return
;--------------------------------------------------------
;	RS232SendReSyncNulls
;Modifies R2BTemp
;if comms problem we will send R2_MAX_RECORD_LEN*2+2 nulls
;so that pc can re-synchronise to us. 
;It should ensure that at least one null record is read 
;by the pc which it recognises as a re-sync
;Should also do whatever is required to place rs232 into
;a known state. 
;Perhaps should do an init??
;--------------------------------------------------------
RS232SendReSyncNulls
	banksel	R2BTemp	
	call	GSIInitRS232		;init everything

	movlw	(R2_MAX_RECORD_LEN*2)+2
	movwf	R2BTemp
	clrf	WREG
rsrsn:
	call	GSISendByte232	
	decf	R2BTemp,F
	bnz	rsrsn
	
	movlw	RT_GSI_ACK		;send an ACK when we are through
	call	GSISendByte232
rsrsn1:		
	call	GSIGetFromRS232RxBuff	;wait for an echo
	bz	rsrsn1		;wdt on error
	clrwdt
	
	call	GSIFlushR2RxBuffer
	return

;------------------------------------------------------------
;	GSIEchoByte232
;Non-interrupt rs232 TX
;Check TXIF (TSR empty), if set then can write the byte
;if clr then must wait to be set
;Receive byte to send in W
;leaves W unchanged
;NOTE: ilegal to test TXIF immediately after write to TXREG
;must place minimum of a single nop between.
;------------------------------------------------------------
	IF DEBUG
DEBUGGSISendByte232
	btfss	GlbSIOMode,SIO_M_BINARY
	bra GSISendByte232	;only execute if not binary mode
	return
	ENDIF
GSIEchoByte232
	btfss	GlbSIOMode1,SIO_M1_ECHO
	return
GSISendByte232
	IF USART_ENABLED
	IF SIMULATOR == FALSE
		messg wdt removal for debugging!!
		clrwdt
		btfss	PIR1,TXIF			;PIR1 in BANK0
		bra	GSISendByte232
	ENDIF	;SIMULATOR
	movwf	TXREG	
	ENDIF	;USART_ENABLED
	return


;----------------------------------------------------------
;GSIPeekRS232RxBuff
;return the next char in the r2buff that will be read
;return
;empty Z
;char NZ with char in WREG
;----------------------------------------------------------
GSIPeekRS232RxBuff
;If we read NumInBuff, then a char arrives. NumInBuff will inc
;but we will have old value. Won't matter as we will quit.
;the ISR will not modify RCursor (Read Cursor)
	movff	R2RxNumInBuff,WREG	;should be ISR safe
	movf	WREG				;want to set flags
	bz		_gprb_exit
	movff	R2RxRCursor,FSR2L
	;movwf	FSR2L				;Indirect access tail
	movlw	HIGH R2RxBuff
	movwf	FSR2H
	movf	INDF2,W			;Peek from buffer, leave it there
	bcf		STATUS,Z
_gprb_exit:
	return
	
;-----------------------------------------------------------------
;	GSIGetFromRxBuffer
;Assumes MAIN_BANK
;Resources
;Mustn't use GlbLocalTempL
;Return Z if empty, NZ otherwise, data in WREG
;While in this routine an rs232 Rx int can/will occur
;Do we need to cli during this function
;Only NumInBuff can affect us??
;----------------------------------------------------------------
GSIGetFromRS232RxBuff

;Appear to fail to return from this routine if a CAN RX1 int occurs
;so does the PtRxISR change something that this routine uses?
;obvious candidate is fsr2
	;DW	0xffff

	movf	R2RxNumInBuff,W
	bz	gfrb_exit
	;btfsc	STATUS,Z
	;bra	gfrb_exit
	;return					;buffer empty
	IF DEBUG
		;movlw	't'
		;cpfseq GlbDebugTemp
		;bra	__1			;signal from r2slave
		;movlw	'g'			;Error to LCD
		;call	GSILCDWriteChar
;__1	
	ENDIF
	movf	R2RxRCursor,W
	movwf	FSR2L				;Indirect access tail
;added 15/11/04
	movlw	HIGH R2RxBuff
	movwf	FSR2H
	;Buffer management
	incf	R2RxRCursor,F			;increment write cursor
	movf	R2RxRCursor,W			;Load cursor to W
	subwf	R2RxBuffEnd,W			;compare to buff end
	movf	R2RxRCursor,W			;Cursor pos for no ovf
	btfss	STATUS,C			;nc = ovf
	movlw	R2RxBuff			;ovf, get start pos of buffer
	;W holds next valid Insertion point
	movwf	R2RxRCursor			;update cursor
;--------------------------------------------------------
;If a char is received after we decf, then, before we btfsc
;NumInBuff increases by 1. Thus we can clear the CHAR_AVAIL bit
;even though 1 is actually available
;Can either, Use the R2RxNumInBuff variable directly in mainline code
;or can disable ints while we do the read/check
	IF DEBUG
	btfsc	INTCON,GIEL
	bra	_dIntsLoop
	;ints are currently disabled
	decf	R2RxNumInBuff,F			;keep count
	btfsc	STATUS,Z,A
	bcf	GlbSIOStatus1,SIO_S1_CHAR_AVAILABLE	;signal buff empty
 	bra	_dInts_exit
	ENDIF
_dIntsLoop:
	bcf	INTCON,GIEL	;disable ints
	btfsc	INTCON,GIEL
	bra	_dIntsLoop	;loop till GIE clear
;-------- Interrupts disabled-----------------
	decf	R2RxNumInBuff,F			;keep count
	btfsc	STATUS,Z,A
	bcf	GlbSIOStatus1,SIO_S1_CHAR_AVAILABLE	;signal buff empty
	;must re-enable ints here!
	bsf	INTCON,GIEL	;enable ints
;-------- Interrupts enabled ----------------
_dInts_exit:
	IF DEBUG
		;movlw	't'
		;cpfseq GlbDebugTemp
		;bra	__2			;signal from r2slave
		;movlw	'h'			;Error to LCD
		;call	GSILCDWriteChar
;__2	
	ENDIF
 
	;XOn reqd?
	btfss	GlbSIOMode,SIO_M_BINARY		;ignore XON/XOFF in binary mode
	bra	gfrb_no_error_exit
	;-see if XOn needs to be sent
	btfss	GlbSIOStatus,SIO_XOFF_SENT
	bra	gfrb_no_error_exit
	;-XOff has been sent, do we need to XON?
	movf	R2RxNumInBuff,W			;check for buff full
	sublw	RX_BUFF_SIZE/2
	btfsc	STATUS,C,A		;Buffer < half full?
	call	SendXOn			;Yes so start receiving again
gfrb_no_error_exit
	movf	INDF2,W,A		;Get from buffer
	IF DEBUG	
		;movlw	'c'
		;call	DEBUGGSISendByte232
	ENDIF

	bcf	STATUS,Z,A
gfrb_exit	;Z=empty
	;bcf	LATC,1
	return
	DW	0xffff		

;--------------------------------------------
;	AddToRxBuff
;circular buffer for USART Rx characters
;Assumes R2Bank
;returns Z no error
;returns NZ error
;--------------------------------------------
GSIAddToRS232RxBuff
;R2RxByte holds incoming data
;can be called at int time
	DW	0xffff

	bsf	GlbSIOStatus1,SIO_S1_CHAR_AVAILABLE
	movlw	RX_BUFF_SIZE			;check for buffer ovf
	subwf	R2RxNumInBuff,W		
	btfsc	STATUS,Z
	goto	atrbOvf

	bcf	GlbSIOStatus,SIO_TX_BUFF_OVF	;reset flag if it is set

	movf	R2RxWCursor,W			;Ptr to buff insertion point
	movwf	FSR2L,A				;Indirect access insertion point
	movf	R2RxByte,W			;Get the incoming byte
	movwf	INDF2,A				;put it in the buffer
	;Buffer management
	incf	R2RxWCursor,F			;increment write cursor
	movf	R2RxWCursor,W			;Load cursor to W
	subwf	R2RxBuffEnd,W			;compare to buff end
	movf	R2RxWCursor,W			;Cursor pos for no ovf
	btfss	STATUS,C,A			;nc = ovf
	movlw	R2RxBuff				;start pos of buffer
	;W holds next valid Insertion point
	movwf	R2RxWCursor			;update cursor
	incf	R2RxNumInBuff,F			;keep count

	btfss	GlbSIOMode,SIO_M_BINARY		;ignore XON/XOFF in binary mode
	bra	atrb_exit
	;XOff reqd?
	movlw	RX_BUFF_SIZE-2			;check for buff full
	subwf	R2RxNumInBuff,W
	btfsc	STATUS,Z,A			;Buffer almost full?
	call	SendXOff			;2 bytes before ovf
atrb_exit
	IF DEBUG	
		;movlw	'C'
		;call	DEBUGGSISendByte232
		;movlw	'a'			;Error to LCD
		;call	GSILCDWriteChar

	ENDIF
	bsf	STATUS,Z			;no error
	return
atrbOvf
	call	SendOverflow	
	bcf	STATUS,Z			;send error
	return
	DW	0xffff
;--------------------------
;	SendOverflow
;--------------------------
SendOverflow
	bsf	GlbSIOStatus,SIO_RX_BUFF_OVF
	;now must place an error code in the TX queue
	;If fails then must record a double error

	return

;--------------------------
;	SendXOn
;--------------------------
SendXOn
	bsf	GlbSIOStatus,SIO_XON_SENT
	bcf	GlbSIOStatus,SIO_XOFF_SENT
	return
;--------------------------
;	SendXOff
;--------------------------

SendXOff
	bcf	GlbSIOStatus,SIO_XON_SENT
	bsf	GlbSIOStatus,SIO_XOFF_SENT
	return

;------------------------------------------
;	GSISendBinary232
;send binary in W down rs232
;Assumes MAIN_BANK
;------------------------------------------
DEBUGGSISendBinary232
	IF DEBUG
	btfss	GlbSIOMode,SIO_M_BINARY
	bra GSISendBinary232	;only execute if not binary mode
	return
	ENDIF
GSISendBinary232
	IF(SIMULATOR == FALSE)
	movwf	R2BTemp		;store original

	;movlw	' '
	;call	GSISendByte232

	movlw	8
	movwf	R2BCounter
txb:
	clrf	R2BTemp1
	bcf	STATUS,C,A
	rlcf	R2BTemp,F
	rlcf	R2BTemp1,F
	movlw	30h
	addwf	R2BTemp1,W
	call	GSISendByte232	;assumes BANK0
	decfsz	R2BCounter,F	
	goto	txb
	movlw	' '
	call	GSISendByte232
	ENDIF
	return

;--------------------------------------------
;	GSIInitRS232
;--------------------------------------------

GSIInitRS232
	call	InitUSART
	;vrbls
	movlw	R2RxBuff
	movwf	R2RxWCursor
	movwf	R2RxRCursor
	addlw	RX_BUFF_SIZE-1
	movwf	R2RxBuffEnd

	movlw	R2TxBuff
	movwf	R2TxWCursor
	movwf	R2TxRCursor
	addlw	TX_BUFF_SIZE-1
	movwf	R2TxBuffEnd
	clrf	R2TxNumInBuff
	clrf	R2RxNumInBuff
	clrf	GlbSIOStatus
	clrf	GlbSIOStatus1
	clrf	GlbSIOStatus2

	clrf	GlbSIOMode
	clrf	GlbSIOMode1
	clrf	R2RxRecordCursor
	clrf	R2RxRecordLength
	movlw	SIO_M1_DEFAULT_FLAGS		;echo to rs232
	movwf	GlbSIOMode1

	return
;---------------------------------------------------
;InitUSART
;---------------------------------------------------
InitUSART
;init SPBRG for 57k baud rate counter
;Note that an inverter is reqUired for Tx+Rx otherwise
;framing errors/wrong data.
;1488 ->pic is used for Tx
;74hct04 -> pic is used for Rx

	IF USART_ENABLED
	movlw	BAUD_RATE
	;movlw	BAUD_RATE_115200
	;movlw	BAUD_RATE_57600
	;movlw	BAUD_RATE_416666
	movwf	SPBRG
	bcf	TXSTA,SYNC,A		;async mode
	bsf	TXSTA,BRGH,A		;High speed baud rate
	bcf	TXSTA,TXEN,A		;disable tx, re-enable only when needed
	bcf	TXSTA,TX9,A		;disable 9 bit (should be default)
	IF TX_INTERRUPT
		bsf	PIE1,TXIE,A	;enable tx ints
	ELSE	;TX_INTERRUPT
		bcf	PIE1,TXIE,A	;disable tx ints
	ENDIF	;TX_INTERRUPT

	IF RX_INTERRUPT
		bsf	PIE1,RCIE,A	;enable usart rx int
	ELSE	;RX_INTERRUPT
		bcf	PIE1,RCIE,A	;disable usart rx int
	ENDIF	;RX_INTERRUPT
	;to tx, load data to TXREG
	;set up Rx

	bcf	RCSTA,RX9,A		;disable 9 bit (should be default)
	bsf	RCSTA,CREN,A		;enable continuous rx
	bsf	RCSTA,SPEN,A		;serial port enable
	;PIIE1:RCIF will be set	on rx of byte
	;read RCREG to get byte.  On error RCSTA :FERR | OERR
	;for FERR read RCREG to clear error and read next byte ie
	;ignore the error
	;for OERR (overrun) bcf RCSTA,CREN. ?? then bsf it again??

	bsf	TXSTA,TXEN,A		;enable tx, re-enable only when needed

	ENDIF	;USART_ENABLED

	return

;------------------------------------------------------------------
;		GSIWriteErrorMessageRS232
;receive WREG=mssg number
;modifies: BSR,WREG
;Checks for a running can node, sends down the CAN if we are running
;uses R2BTemp
;------------------------------------------------------------------
GSIWriteErrorMessageRS232
#IF GSI_CAN_SUPPORT
	banksel	R2Bank
	movwf	R2BTemp1		;store error code
	movff	CnMode,R2BTemp		;get the current mode
	btfss	R2BTemp,CAN_MODE_SLAVE	;are we a slave? If so then the CAN is running (or was)
	bra	gwemr2_no_can
gwemr21:
	;-ok CAN is running so tx the mssg
	movff	CnThisNode,CnTxBuff	;our node
	;movlw					;CAN_KD_MSSG_KEY_UP
	movff	R2BTemp1,CnTxBuff+1	;The message id (defs.inc Message Strs)
	;movff	KdCurrentKey,CnTxBuff+2	
;----------Note that this mssg will be sent to the MASTER---
;If we are already the master should we still send it??
	mCANPrTxSendMsg CAN_ID_ERROR_MSSG,CnTxBuff,2,CAN_TX_XTD_FRAME

        addlw   0x00            ;Check for return value of 0 in W
        bz      gwemr21         ;Buffer Full, Try again
        movff	R2BTemp,WREG	;restore error code
gwemr2_no_can        
#ENDIF ;GSI_CAN_SUPPORT

    banksel	R2Bank
	;WREG holds error number
	banksel	R2Bank
	btfss	GlbSIOMode,SIO_M_BINARY
	bra		GSIWriteMessageRS232
	;binary mode so send an rs232 message, subfunction error
	;If we are a slave node then we need to send this to the master as a CAN mssg
	;messg	send to rs232
	
	return	

;-----------------------------------------------------------
;	GSIWriteMessageRS232
;uses R2Btemp as this will not intefere with Binary routine
;-----------------------------------------------------------
GSIWriteMessageRS232
;Receive address of message in W
;we are in GSI_BANK
	
	addwf	WREG			;Word ptr
	movwf	TBLPTRL			;index to str

	movlw	LOW PgmMssgTbl		;add the offset
	addwf	TBLPTRL

	clrf	TBLPTRH
	movlw	HIGH PgmMssgTbl		;ripple any Cy
	addwfc	TBLPTRH

	clrf	TBLPTRU
	movlw	UPPER PgmMssgTbl	;through H and U
	addwfc	TBLPTRU
	;TBLPTR holds address of string table anywhere in memory

	TBLRD*+				 
	movf	TABLAT,W		;low byte of str address
	movwf	R2BTemp			;use Binary temp

	TBLRD*
	movf	TABLAT,W		;High byte of str address
	movwf	TBLPTRH
	;Don't fill upper as it will be modified if req
	;by the str pointer code
	movff	R2BTemp,TBLPTRL
lwm1	
	TBLRD*+
	movf	TABLAT,W
	;Test first so that we can ignore a null string
	tstfsz	WREG	
	goto	lwm2
	return
lwm2
	call	GSISendByte232	;unconditional write
	goto	lwm1

;------------------------------------------
;	GSISendBinary232ISR
;A version called from the ISR so no bank assumptions
;send binary in W down rs232
;Assumes MAIN_BANK
;------------------------------------------
DEBUGGSISendBinary232ISR
	IF DEBUG
	btfss	GlbSIOMode,SIO_M_BINARY
	bra GSISendBinary232ISR	;only execute if not binary mode
	return
	ENDIF
GSISendBinary232ISR
	IF(SIMULATOR == FALSE)

	movff	BSR,R2BSR		;preserve current bank
	banksel	R2BTempISR

	movwf	R2BTempISR		;store original

	movlw	'i'
	call	GSISendByte232

	movlw	8
	movwf	R2BCounterISR
txbi:
	clrf	R2BTemp1ISR
	bcf	STATUS,C,A
	rlcf	R2BTempISR,F
	rlcf	R2BTemp1ISR,F
	movlw	30h
	addwf	R2BTemp1ISR,W
	call	GSISendByte232	
	decfsz	R2BCounterISR,F	
	goto	txbi

	movlw	'i'
	call	GSISendByte232

	movlw	' '
	call	GSISendByte232	;this is ISR safe, makes no bank assumptions

	movff	R2BSR,BSR		;restore current bank

	ENDIF	;SIMULATOR


	return

;------------------------------------------------
;	GSISendAscii232
;Receive byte in WREG
;convert to ascii and send down rs232
;------------------------------------------------
GSISendAscii232

	movwf	R2AsciiTemp
	movwf	GlbLocalTempL
	clrf	GlbLocalTempH
	movlw	d'100'

	btfss	GlbLocalTempL,7		;>=128?
	bra	ssai_2
	incf	GlbLocalTempH
	subwf	GlbLocalTempL,F	
ssai_2
	
ssai_hundreds
	subwf	GlbLocalTempL,F
	bn	ssai_tens
	incf	GlbLocalTempH,F
	bra	ssai_hundreds
ssai_tens
	addwf	GlbLocalTempL,F
	movf	GlbLocalTempH,W
	addlw	0x30	
	call	GSISendByte232
	clrf	GlbLocalTempH
	movlw	d'10'
ssai_1
	subwf	GlbLocalTempL,F
	bn	ssai_units
	incf	GlbLocalTempH,F
	bra	ssai_1
ssai_units
	addwf	GlbLocalTempL,F
	movf	GlbLocalTempH,W
	addlw	0x30
	call	GSISendByte232
	movf	GlbLocalTempL,W
	addlw	0x30
	call	GSISendByte232
	SPACE_RS232
	movf	R2AsciiTemp,W
	return



GSISendBSRAscii
	NEWLINE_RS232
	movlw	'B'
	call	GSISendByte232
	movlw	'S'
	call	GSISendByte232
	movlw	'R'
	call	GSISendByte232
	SPACE_RS232
	movf	BSR,W
	goto	GSISendAscii232
	
R2DebugCode
	;call	GSICANPCMaster		
	;movlw	RT_ESC_CAN_SET_SLAVE		;record type==CAN_SET_SLAVE
	;movlw	RT_GSI_REPEAT
	;movlw	RT_END_GSI_CODES+1

	return

#if SIMULATOR

	movlw	RT_GSI_ACK
	movwf	GlbLocalTempL
	movlw	1
	movwf	GlbLocalTempH
	lfsr	FSR0,GlbLocalTempL
	
	call	GSILookForR2RxChar	
	bnc		_rdc_found
	return
_rdc_found
	movlw	1
	return	
	
	

	;bsf	GlbSIOMode,SIO_M_BINARY
	;As we begin in ascii mode this is a single byte 
	;command rather than a record
	bsf	GlbSIOStatus1,SIO_S1_REQ_MASTER
	movlw	RT_BINARY_MODE_ASC		;binary mode
	movwf	R2RxByte
	call	GSIAddToRS232RxBuff
	;-Now in Binary record mode
	movlw	RT_GSI_LCD_MSSG		;RecType
	movwf	R2RxByte
	call	GSIAddToRS232RxBuff
	;-
	movlw	CAN_NODE_THIS		;recNode
	movwf	R2RxByte
	call	GSIAddToRS232RxBuff
	;-
	movlw	2		;record len	
	movwf	R2RxByte
	call	GSIAddToRS232RxBuff
	;-
	movlw	d'2'		;record data b0	(writeChar)
	movwf	R2RxByte
	call	GSIAddToRS232RxBuff
	;-
	movlw	'Z'		;record data b1	
	movwf	R2RxByte
	call	GSIAddToRS232RxBuff

	;
	movlw	RT_GSI_ACK	;Pretend we received all echoes and RT_ACK
	movwf	R2RxByte
	call	GSIAddToRS232RxBuff
	;Can't just send another, must first read the
	;RS232_MSG sent by this node (it simulates an echo
	;by adding the sent data to the input queue)
	#if 0
;---------------------------------------------------------
;	Return to ascii mode
;---------------------------------------------------------
	movlw	RT_ASCII_MODE_ASC	;ascii mode
	movwf	R2RxByte		
	call	GSIAddToRS232RxBuff	;RecordType
	;-
	movlw	CAN_NODE_THIS		;recNode
	movwf	R2RxByte
	call	GSIAddToRS232RxBuff
	;-
	movlw	0		;record len no data
	movwf	R2RxByte
	call	GSIAddToRS232RxBuff
	;-
	movlw	RT_GSI_ACK	;Pretend we received all echoes and RT_ACK
	movwf	R2RxByte
	call	GSIAddToRS232RxBuff
	;----------------------------------
	endif ;if 0
	;-
	call	GSIProcessRxByte	
	call	GSIProcessRxByte	
	call	GSIProcessRxByte	
	call	GSIProcessRxByte	
	call	GSIProcessRxByte
	call	GSIProcessRxByte	
	call	GSIProcessRxByte
	;here after processing the lcd write
	
	call	GSIProcessRxByte	
	call	GSIProcessRxByte	
	call	GSIProcessRxByte
	call	GSIProcessRxByte	
	call	GSIProcessRxByte	
	call	GSIProcessRxByte	
	call	GSIProcessRxByte	
	call	GSIProcessRxByte
	call	GSIProcessRxByte	
	call	GSIProcessRxByte	
	call	GSIProcessRxByte	
	call	GSIProcessRxByte
	call	GSIProcessRxByte	
	call	GSIProcessRxByte	
	call	GSIProcessRxByte

	ENDIF	;SIMULATOR
	return

;-----------------------------------------------
;GSISendBreak
;Hold Tx line high for > 1 char
;----------------------------------------------
GSISendBreak

;need to disable rs232 int so that we don't attempt to echo any char
;though in binary mode this will not happen as we do not echo in the
;ISR but in a call from the main program loop.
;However the pc could send us a char at the same instant that we
;send the BREAK. Under this condition we must honour the sent char
;by echoing it, ignoring the break we sent.
;On receipt of a break, the pc will look for a Break code, informing
;it of the type of break

;TMR0 has a 50uSec period, can use it to timeout
;Returns:
;timeout: C,Z
;fail (char found but not an ACK): C,NZ
;success NC
;------------------------------------------------------------

;first ensure any bytes currently Txing are sent
	btfss	PIR1,TXIF			;PIR1 in BANK0
	bra	GSISendBreak

	bcf	RCSTA,SPEN,A		;must disable port to toggle PORTC,6
	;bcf	TXSTA,TXEN		;disable Tx
	
	bcf	PORTC,6
	banksel	GiBank
	call	GSIDelay160us	
	bsf	PORTC,6
	
	;bsf	TXSTA,TXEN		;enable Tx
	
	bsf	RCSTA,SPEN,A
;Ater break, delay at least one char, 100 uS for 115200 baud
;NOTE
	call	GSIDelay160us                         ;  GSIDelay 160 usecs
#IF BAUD_RATE == BAUD_RATE_57600
	call	GSIDelay160us
#ENDIF

;-------break has been sent	---------

	return
	
	
;----------------------------------------------------------------	
;GSILookForR2RxChar		
;FSR0 points to 2 bytes
;fsr+0=char to look for in buffer
;fsr+1=timeout in 13 msec ticks

;Returns
;timeout: C,Z
;fail (char found but not an ACK): C,NZ
;success NC
;If found, the char is removed from the CQ
;If timeout of not the correct char, CQ is left unchanged

;R2Master.asm calls this function after a Break has been sent
;it uses GlbLocalTempL/H for variables (FSR0 points to L)
;------------------------------------------------------------------
GSILookForR2RxChar	
;timeout probably needs to be about 50ms. If pc is engaged
;in lengthy tasks. It might not respond for several time slices
;If we are here, there is currently no RS232 activity, so nothing to
;process from the pc. Interrupts are still able to receive CAN frames
;and they are dependent on this function succeeding. So, we need a 
;long timeout interval.
;TMR0 is set for prescaler of 256 and count of 256, so 13ms per tick
;should probably count 20 of these ticks, 260ms. 
;GlbClock is 13msec counter. Use this.
;
	incf	FSR0L			;point to timeout value
	movf	INDF0,F			
	bz		_gsb_infinite_timeout

	movff	GlbClock,WREG	;get the current clock tick
	incf	WREG			;add one in case it is about to tick over
	addwf	INDF0			;add to the current timeout
_to_loop
	clrwdt
	movf	INDF0,W			;get timeout
	subwf	GlbClock,W
	bz		_gsb_timeout
;--
	call	GSIPeekRS232RxBuff	;WREG==next char (uses FSR2)
	bz		_to_loop			;empty
_gsb_check:
	decf	FSR0L				;point to char
	cpfseq	INDF0	 			;is it the char requested
	bra		_gsb_fail
;success, pc has echoed char	
	call	GSIGetFromRS232RxBuff	;take the byte out of the buffer
	bcf		STATUS,C			;NC
	return
_gsb_timeout
	bsf		STATUS,Z			;Z,C
_gsb_fail						;NZ,C	
	bsf		STATUS,C
	clrf	WREG
	return 
_gsb_infinite_timeout:
	clrwdt
	call	GSIPeekRS232RxBuff	;WREG==next char (uses FSR2)
	bz		_gsb_infinite_timeout			;empty
	bra		_gsb_check

;--------------------------------------------------------------------------	
;GSISendByteWithEcho232	
;Send byte in WREG to rs232
;look for an echo
;use default timeout 
;timeout: C,Z
;fail (char found but not correct one): C,NZ
;success NC
;-------------------------------------------------------------------------
GSISendByteWithEcho232
	movwf	GlbLocalTempL
	call	GSISendByte232
	movlw	R2_DEFAULT_ECHO_TIMEOUT
	movwf	GlbLocalTempH		;timeout in 13msec ticks
	;clrf	GlbLocalTempH		;XXX GS infinite timeout for debugging
	lfsr	FSR0,GlbLocalTempL

	call 	GSILookForR2RxChar	
	btfss	STATUS,C			;C==error
	return	
	call	GSIFlushR2RxBuffer
	bsf		STATUS,C
	return
	END
