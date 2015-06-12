;******************************************************************************
;* @file    R2Slave.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;R2Slave.asm
;rs232 "slave" functions.

;rs232
#define IN_R2SLAVE.ASM

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
#include "error.inc"

	#IF GSI_CAN_SUPPORT
#include "CANPrTx.inc"
#include "can.inc"
	#ENDIF


#include "error.inc"


	GLOBAL	BinaryProcessRxByte
	GLOBAL R2SendProcessRecordSuccessFail,R2SendProcessRecordFail
	GLOBAL R2RTRepeatCounter
	EXTERN	GlbCompOffset
	
	EXTERN	GSIProcessRxByte_ctr_code,GSIProcessRxByte_esc_code,GSIProcessRxByte_user_code

	;rs232.asm
	EXTERN	R2RxByte,R2BTemp
	EXTERN	R2RxRecordCursor
	EXTERN	R2RxRecordType,R2RxRecordNode,R2RxRecordLength,R2RxRecordData
	EXTERN	R2TxRecordType,R2TxRecordNode,R2TxRecordLength ;,R2TxRecord
	EXTERN 	R2ErrorState
	EXTERN RS232SendReSyncNulls	
	EXTERN R2SubfunctionRecord,R2SubfunctionRecordLength
	EXTERN R2SubfunctionRecordData,R2CurrentFunctionThisNode
	EXTERN DEBUGGSISendByte232

#IF GSI_CAN_SUPPORT
;From CANTx.asm
	;EXTERN	SendCANRxMssg232
	EXTERN	SendCANRepeat232,SendCANAck232,SendCANErrorCode232
	EXTERN 	SendCANFatalError232

	;Striclty speaking the first four messages are not functions
	;they are handshaking bytes, kept here as placeholders
	;and for possible special use
	;EXTERN GSISendCanReserved_1,GSISendCanReserved_2
	EXTERN SendCANReserved232_1,SendCANReserved232_2
	EXTERN SendCANRS232Mssg232,SendCANRS232BreakMssg232
	EXTERN SendCANLCDMssg232,SendCANKeypadMssg232,SendCANSPIMssg232
	EXTERN SendCANI2CMssg232,SendCANDIOMssg232,SendCANADCMssg232
	EXTERN CANSetPCMaster232,CANSetMaster232,SendCANMssg232
	EXTERN SendCANError232,CANSetSlaveId232,CANSetSlave232
	EXTERN SendCANIOCTLMssg232,SendCANGSIIOCTLMssg232
	EXTERN SendCANPingMssg232,SendCANReserved1_232,SendCANReserved2_232
	EXTERN SendCANReserved3_232,SendCANReserved4_232,SendCANReserved5_232
#ENDIF
	;From can.asm
	EXTERN	CnThisNode

RS232_DATA	UDATA
R2RTRepeatCounter	RES	1
R2DebugTemp		RES	1
R2_SLAVE CODE

;---------------------------------------------------------------------------
;	BinaryProcessRxByte
;The byte has not been echoed
;currently in binary mode so need to assemble
;RECORD_TYPE: 1 BYTE
;RECORD_NODE 1 BYTE
;RECORD_LEN:  1 BYTE
;RECORD:      0-8 BYTES
;R2RxRecordType		RES 1	;A binary Mssg, 0 is special case:ignore
;R2RxRecordNode		RES 1	;A binary Mssg
;R2RxRecordLength		RES 1   ;the number of bytes in record
;R2RxRecordData		RES 8	;the data 0-8 bytes
;R2RxRecordCursor	RES 1	;Where we are in assembly of record
;Receives WREG RxByte
;The length is always +1 because an RT_GSI_ACK will always be transmitted
;once pc has received echo of last byte (or after last char Tx'd if no echo)

;-------------------------------------------------------------------------

BinaryProcessRxByte
	movwf	R2BTemp		;temporary storage
	;Mustn't echo here as when we receive an ACK from the pc we need to respond
	;with an ACK or an ERROR
;If we are R2Master then this is an error?
	IF DEBUG
		btfss	GlbSIOStatus1,SIO_S1_MASTER
		bra	bprb1
		movlw	'!'			;Error to LCD
		#IF GSI_LCD_SUPPORT
		call	GSILCDWriteChar
		#ENDIF
		bsf	R2ErrorState,SIO_ERR_ECHO_TX
		bsf	GlbErrState,GSI_ERR_RS232
bprb1:
	ENDIF
bprb1a:	
	movlw	HIGH  bprb_tbl	        ;fetch upper byte of jump table address
	movwf	PCLATH			;load into upper PC latch
	;WREG=0 to 15
	movf	R2RxRecordCursor,W ;What are we currently processing		
	addwf	WREG,W			;2 byte 18f instructions, byte PC
	;addwf	WREG,W			;2 word instruction, so *4 index
	;WREG holds byte offset to correct goto
	;Now add offset to the jump table
	addlw	LOW  (bprb_tbl)          ;calc state machine jump addr into W		
	btfsc	STATUS,C                ;skip if carry occured
	incf	PCLATH,F		;otherwise add carry
I2CJump	; address were jump table branch occurs, this addr also used in fill 
	movwf	PCL  		        ;index into state machine jump table
	;jump to processing for each state= I2CStateIndex value for each state
bprb_tbl
	bra	bprbRecType	        ;rec type
	bra	bprbRecNode		;node this data is for
	bra	bprbRecLen	 	;rec len
	bra	bprbData0		;Data0
	bra	bprbData1		;Data1
	bra	bprbData2		;Data2
	bra	bprbData3		;Data3
	bra	bprbData4		;Data4
	bra	bprbData5		;Data5
	bra	bprbData6		;Data6
	bra	bprbData7		;Data7
	bra	bprbData8		;Data8
	bra	bprbData9		;Data9
	bra	bprbData10		;Data10
	bra	bprbData11		;Data11
	bra	bprbData12		;Data12
	bra	bprbData13		;Data13
	bra	bprbAck			;ACK
LAST_BPRB_ITEM 	EQU	($-bprb_tbl-2)/2

bprb_exit_data_received
	movf	R2BTemp,W
	call	GSISendByte232		;echo the data byte
	;-
	decf	R2RxRecordCursor,W	;see if this is last data
	decf	WREG
	cpfseq	R2RxRecordLength
	bra	bprb_exit		;still reading data
	;---record is complete
	;movf	R2BTemp,W		;already echoed if flag is set
	;call	GSIEchoByte232	
	;-Now ensure next byte jumps to ACK
bprb_last_item_exit:
	movlw	LAST_BPRB_ITEM
	movwf	R2RxRecordCursor
	return
bprbAck:			;Ack byte
	;check to ensure it is an ACK, if not then send REPEAT?
	;if it is REPEAT rather than ACK then there was a transmission error
	;and we must ignore the record as the pc is going to repeat
	
	clrf	R2RxRecordCursor	;start processing from beginning
	movlw	RT_GSI_ACK
	cpfseq	R2BTemp
	bra	bprbAckError
	;echo the ACK
	call	GSISendByte232	

	call	ProcessR2RxRecord
;Must swap bank as the record could have called user code or library
;code that does anything.
	banksel	R2Bank	
	return
	messg require ACK error message
bprbAckError:
	clrf	R2RxRecordCursor	;ensure start processing at RecordType
	DEBUG_LCD_WRITE_CHAR 'R'
	movf	R2BTemp,W		;should be a REPEAT
	call	GSISendByte232		;echo the PRESUMED repeat
	movlw	RT_GSI_REPEAT		;if it isn't, what to do?
	cpfseq	R2BTemp			;best to ignore it!
	bra	bprbResetToAsciiExit	
bprbSendRepeat:
	;repeat has already been sent
	return

bprb_exit

	incf	R2RxRecordCursor,F	;point to next location
	return

bprbRecType	        ;rec type
	;-
	movf	R2BTemp,W		;get current byte
	call	GSISendByte232		;echo
	movwf	R2RxRecordType		;doesn't set flags!
	movf	R2RxRecordType,W	;set flags
	bnz	bprb_exit
	;null record type so ignore it for re-sync purposes	
	;Also allows the pc code to set a record as type=0 while the record is partially filled
	;or the user has set an erroneous error type. If this code is subsequently called with
	;a 0 as record type then it will be ignored. 
	;Unfortunately the next byte sent, the node the record is for will then be interpreted as
	;record type. We have simply deferred the problem!
bprbResetToAsciiExit:
	movlw	0xff			;set cursor back to first item
	movwf	R2RxRecordCursor	;will be incf'd by exit prologue

	;call	GSISetRS232AsciiMode
	
	bra	bprb_exit
bprbRecNode

	;-
	;echo the byte
	movf	R2BTemp,W
	call	GSISendByte232		
	;-
	movff	R2BTemp,R2RxRecordNode
	bra	bprb_exit
bprbRecLen	 	;rec len

	movf	R2BTemp,W
	;echo the byte
	call	GSISendByte232		
	;-
#IF DEBUG_RECORD_LENGTH_TEST
	movlw	R2_MAX_RX_RECORD_DATA+1
	cpfslt	R2BTemp
	call	DEBUGCauseWDT
	movf	R2BTemp,W
#ENDIF	
	movwf	R2RxRecordLength
	btfss	STATUS,Z
	bra	bprb_exit
	;here if no data with this message so need to look for ack		
	movf	R2BTemp,W		;retrieve Byte and echo
	bra	bprb_last_item_exit

bprbData0		;Data0

	movff	R2BTemp,R2RxRecordData
	bra	bprb_exit_data_received

bprbData1		;Data1
	movff	R2BTemp,R2RxRecordData+1
	bra	bprb_exit_data_received

bprbData2		;Data2
	movff	R2BTemp,R2RxRecordData+2
	bra	bprb_exit_data_received

bprbData3		;Data3
	movff	R2BTemp,R2RxRecordData+3
	bra	bprb_exit_data_received

bprbData4		;Data4
	movff	R2BTemp,R2RxRecordData+4
	bra	bprb_exit_data_received

bprbData5		;Data5
	movff	R2BTemp,R2RxRecordData+5
	bra	bprb_exit_data_received

bprbData6		;Data6
	movff	R2BTemp,R2RxRecordData+6
	bra	bprb_exit_data_received

bprbData7		;Data7
	movff	R2BTemp,R2RxRecordData+7
	bra	bprb_exit_data_received

bprbData8		;Data8

	movff	R2BTemp,R2RxRecordData+8
	bra	bprb_exit_data_received

bprbData9		;Data9

	movff	R2BTemp,R2RxRecordData+9
	bra	bprb_exit_data_received

bprbData10		;Data10
	movff	R2BTemp,R2RxRecordData+10
	bra	bprb_exit_data_received

bprbData11		;Data11
	movff	R2BTemp,R2RxRecordData+11
	bra	bprb_exit_data_received

bprbData12		;Data12
	movff	R2BTemp,R2RxRecordData+12
	bra	bprb_exit_data_received

bprbData13		;Data13
	movff	R2BTemp,R2RxRecordData+13
	bra	bprb_exit_data_received

;-----14 data points max

;----------------------------------------------------------------------------
;	ProcessR2RxRecord
;Received a binary record struct from the pc
;Complete record is in R2RxRecordData struct
;The first 26 records are the LCD Terminal Mssgs
;the next 26 records are the LCD ESC Mssgs
;records 44-49 are User232 records.
;NOTE:
;We have received an RT_GSI_ACK
;letting us know that the record was transferred correctly

;return RS232_MESSAGE subfunction SUCCESS or FAILURE with data...
;bit 7 of data[0] is a MASTER_REQ flag. If set then this
;node wishes to be MASTER.
;pc can respond with an echo then handshake procedure
;----------------------------------------------------------------------------

ProcessR2RxRecord
;The BinaryProcessRxByte code will receive any pending records
;then send
;RT_GSI_RS232_MSSG 
;d0=REQ_MASTER
;It will handle all handshaking issues and once all is ok it will:
;# bsf GlbSIOStatus1,SIO_S1_MASTER
;# bcf GlbSIOStatus1,SIO_S1_REQ_MASTER
;# exit

;---setup for the data in the subfuction SUCCESS/FAIL-------
;assume success
	movlw	1		;1 data byte sent
	movwf	R2SubfunctionRecordLength
	;d0
	movlw	GSI_RS232_MESSAGE_SUCCESS ;| (1<< GSI_RS232_MESSAGE_MASTER_REQ_BIT)
	movwf	R2SubfunctionRecordData
;call to R2SendProcessRecordSuccessFail will send a SUCCESS code
;-----------Check for the record type-----------------------
	movf	R2RxRecordType,W
	sublw	RT_END_CTR_CODES
	btfss	STATUS,C,A
	bra	prr1
;-------Ctr code--------------------------------	
	call	R2SendProcessRecordSuccessFail
	movf	R2RxRecordType,W
	goto	GSIProcessRxByte_ctr_code	;continue processing as an LCD_MSSG
prr1
	movf	R2RxRecordType,W
	sublw	RT_END_ESC_CODES 		;
	btfss	STATUS,C,A
	bra	prr2
	movlw	RT_START_ESC_CODES		;28
	subwf	R2RxRecordType,W			;
	btfss	STATUS,C
	bra	prr2

	call	R2SendProcessRecordSuccessFail
	movf	R2RxRecordType,W
	addlw	-RT_START_ESC_CODES		;addlw is add WREG to literal
	goto	GSIProcessRxByte_esc_code
;----------------------------------------------------
;44-49 decimal: User232 records
;----------------------------------------------------
prr2:
	movf	R2RxRecordType,W
	sublw	RT_END_USER_CODES 		;
	bnc	prr3

	;call	R2SendProcessRecordSuccessFail
	movf	R2RxRecordType,W
	addlw	-RT_START_USER_CODES		;addlw is add WREG to literal
	goto	GSIProcessRxByte_user_code

;------ascii codes in here---------------
prr3:
	movf	R2RxRecordType,W
	sublw	RT_END_GSI_CODES 		;
	btfss	STATUS,C,A
	bra	prr_error_exit
	movlw	RT_START_GSI_CODES		
	subwf	R2RxRecordType,W			;
	btfss	STATUS,C
	bra	prr_error_exit

	#IF 	GSI_CAN_SUPPORT == FALSE
	bra	prr_error_exit
	;can't treat this as an error here as we use CAN code to determine
	;if we are talking to NODE_THIS
	#ELSE
	lfsr	FSR0,R2RxRecordNode	;we will move from R2RxRecordNode->CnRecord in all the goto targets
	;
	movf	R2RxRecordType,W
	addlw	-RT_START_GSI_CODES	;addlw is add WREG to literal
	;Process GSI Codes
	addwf	WREG
	addwf	WREG			;using gotos
	movwf	GlbCompOffset
	movlw 	LOW gsi_code_table 	;get low 8 bits of address
	addwf 	GlbCompOffset,F	;do an 8-bit add operation
	movlw 	HIGH gsi_code_table 	;get high 5 bits of address
	btfsc 	STATUS,C,A 		;page crossed?
	addlw 	1 			;yes then increment high address
	movwf	PCLATH,A		;load high address in latch
	movf	GlbCompOffset,W		;load computed offset in w reg
	banksel	GiBank			;called code expects GSI_BANK
	movwf	PCL,A
gsi_code_table	
;These functions are all bound for the CAN so are all outgoing. They will
;all call the corresponding GSICAN function
;NOTE if the node field = node_this or =CnNode then the function
;is for the code on this node
	goto	SendCANRepeat232	;0 data (53)
	goto	SendCANAck232		;0 data (54)
	goto	SendCANErrorCode232	;0		(55)
	goto	SendCANFatalError232	;0  (56)
	goto	SendCANReserved232_1	;0  (57)
	goto	SendCANReserved232_2	;0  (58)
	;Striclty speaking the first four messages are not functions 
	;they are handshaking bytes, kept here as placeholders
	;and for possible special use
	goto	SendCANRS232Mssg232	;5 data 4=node 1=Mssg (59)
	goto	SendCANRS232BreakMssg232	;Shouldn't receive, this is a placeholder
	goto	SendCANLCDMssg232	;5 data 4=node 1=Mssg (60)
	goto	SendCANKeypadMssg232	;5 data 4=node 1=Mssg (61)
	goto	SendCANSPIMssg232	;6 data 4=node 1=address 1=Mssg/data (62)
	goto	SendCANI2CMssg232	;5 data 4=node 1=Mssg (63)
	goto	SendCANDIOMssg232	;6 data 4=node 1=dio 1=Mssg/data (64)
	goto	SendCANADCMssg232	;7 data 4=node 1=adc 2=Mssg/data (65)
	goto	CANSetPCMaster232	;4 data 4=node (pc master) (66)
	goto	CANSetMaster232	;4 data 4=node (stand alone master:no pc master) (67)
	goto	SendCANMssg232		;0-8 data Put a frame on the CAN (68)
	;goto	SendR2CANRxMssg		;0-8 data, send Rx'd CAN mssg down rs232 to master 
	goto	SendCANError232		;0-8 data	(69)
	goto	CANSetSlaveId232	;5 d0=flag d1-5:SLAVE_BASE (70)
	goto	CANSetSlave232 		;0 data (rs232 is attached to slave)(71)
	goto	SendCANIOCTLMssg232 ;(72)
	goto	SendCANGSIIOCTLMssg232 ;(73)
	goto	SendCANPingMssg232		;(74)
	goto	SendCANReserved1_232	;(75)
	goto	SendCANReserved2_232	;(76)
	goto	SendCANReserved3_232	;(77)
	goto	SendCANReserved4_232	;(78)
	goto	SendCANReserved5_232	;(79)

	;messg should implement a CANReadPicReg,GSICANWritePicReg
	#ENDIF	;IF_GSI_CAN_SUPORT
prr_exit:
	return
prr_error_exit:
	;call	RS232SendProcessRecordFailure
	;movlw	1		;1 data byte sent
	;movwf	R2SubfunctionRecordLength	;slready filled in
	;d0
	incf	R2SubfunctionRecordLength	;send back extra data
	movlw	GSI_RS232_MESSAGE_FAIL
	movwf	R2SubfunctionRecordData
	movlw	GSI_ERR_UNSUPPORTED
	movwf	R2SubfunctionRecordData+1	;send back extra error info
	call	R2SendProcessRecordSuccessFail
	bra	prr_exit

;-------------------------------------------------------------------------
;GSIR2SetSlave
;The pc controls the serial port, it Tx's and we echo
;-------------------------------------------------------------------------
GSIRS232SetSlave
	btfss	GlbSIOStatus1,SIO_S1_MASTER
	return			;this node already the slave
;Relinquish control of the serial port. Clear the input buff
	bcf	GlbSIOStatus1,SIO_S1_MASTER	;make us the slave
	;Any further processing req'd??
	return

;--------------------------------------------------------
;		R2ProcessRecordSuccess/Failure
;Every R2Record received will echo back to the pc.
;After the pc is happy that this node has received the data
;it will send an ACK. (A REPEAT if it is not happy)
;After we have seen the ACK we despatch the command or 
;otherwise process the data.
;The PC will wait for a generic RT_GSI_RS232_MSSG with
;data[0] set to 0 SUCCESS or 1 FAILURE and bit 7 set/reset
;we set bit 7 if we wish to be the MASTER
;bit 7 should probably not be set if the process failed??
;If FAILURE then data[1] to data[n] can contain error info
;If PC can relinquish control it will send an RS232_MSSG
;d0:
;----------------------------------------------------------

;-------------------------------------------------------
;	R2SendProcessRecordSuccess
;Modifies RBTemp
;WREG
;STATUS 
;Send RT_GSI_RS232_MSSG (with echo)
;with data[0] as GSI_RS232_MESSAGE_SUCCESS
;Don't use the TxRecord as this can be filled by the CAN
;RS232SendProcessRecordSuccess
;R2TxRecordType		RES 1	;A binary Mssg
;R2TxRecordNode		RES 1	;the node this data is for
;R2TxRecordLength	RES 1   ;the number of bytes in record
;R2TxRecord		RES R2_MAX_TX_RECORD_DATA	;the data 0-8 +4 bytes for CAN_ID +1 byte for RecFlags if a CAN mssg
;R2TxRecordCursor	RES 1	;Where we are in assembly of record
;R2TxRecordError		RES 1	;0=no error,1=EchoError

;--------------------------------------------------------
R2SendProcessRecordSuccessFail
;Send/receive RT_GSI_RS232_MSSG
;Input:
;R2SubfunctionRecord filled with correct data
;success record holds:
;length 1 byte	0-R2_MAX_TX_RECORD_DATA (0 means no following data)
;data   0-R2_MAX_TX_RECORD_DATA bytes
;Modifies:
;FSR0
;WREG
;BSR

;-------See if we are attached to an rs232 line---------------
	btfss	GlbSIOMode,SIO_M_BINARY
	return
;-------rs232 present----------------------------------------
	banksel	R2Bank
	movlw	4		;make 4-1=3 attempts to REPEAT
	movwf	R2RTRepeatCounter
	bcf	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX
	bcf	GlbSIOStatus1,SIO_S1_ERR_ECHO_ACK_TX	;ACK was echoed incorrectly! (currently ignored)
rsprs_start:
	decf	R2RTRepeatCounter
	IF SIMULATOR
	btfsc	STATUS,Z
	goto	rsprsee_exit
	ELSE
	bz	rsprsee_exit	;Echo error failure after REPEAT's	
	ENDIF
	;-	
	movlw	RT_GSI_RS232_MSSG
      	call	GSISendByte232
	IF SIMULATOR
		movff	WREG,R2RxByte	
		call	GSIAddToRS232RxBuff
	ENDIF	
;------Read the RT_GSI_RS232_MSSG echo ------------------
rsprs1:		
	call	GSIGetFromRS232RxBuff
	bz	rsprs1		;wdt on error
	clrwdt

	sublw	RT_GSI_RS232_MSSG
	btfss	STATUS,Z
	bsf	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX
;--------Send/receive this nodes id ------------------------
	movff	CnThisNode,WREG		;send this Nodes id
	movwf	R2BTemp 
	call	GSISendByte232	
	IF SIMULATOR
		movff	WREG,R2RxByte	
		call	GSIAddToRS232RxBuff
	ENDIF	
rsprs2:		;Read CnThisNode

	call	GSIGetFromRS232RxBuff
	bz	rsprs2		;wdt on error
	clrwdt
	subwf	R2BTemp,W
	btfss	STATUS,Z
	bsf	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX
	#IF DEBUG
	#IF GSI_LCD_SUPPORT
		movlw	'~'
		btfss	STATUS,Z
		call	GSILCDWriteChar
	#ENDIF
	#ENDIF
;-------------------------------------------------
;Send/receive Length of Record (everything past length byte)
;Do a length check
#IF DEBUG_RECORD_LENGTH_TEST
	movff	R2SubfunctionRecordLength,R2BTemp	;R2BTemp holds length
	movlw	R2_MAX_RX_RECORD_DATA+1
	cpfslt	R2BTemp
	call	DEBUGCauseWDT	;can't be 0		
#ENDIF
	lfsr	FSR0,R2SubfunctionRecordLength ;Generic subfunction record	
	movf	POSTINC0,W		;get length and point to data
	movwf	R2BTemp			;use variable
	IF DEBUG
		bnz	__rsprsxxx1
		movlw	FATAL_ERROR_RS232_BINARY_REC
		call	DEBUGCauseWDT	;can't be 0
__rsprsxxx1:
		sublw	GSI_RS232_MAX_SUCCESS_FAIL_DATA
		bnn	__rsprsxxx
		movlw	FATAL_ERROR_RS232_BINARY_REC
		call	DEBUGCauseWDT	;use call, then can return using icd2		
__rsprsxxx:	
		movf	R2BTemp,W		;restore WREG
	ENDIF
	call	GSISendByte232			;Length
	IF SIMULATOR
		movff	WREG,R2RxByte	
		call	GSIAddToRS232RxBuff
	ENDIF	
;----- Read length of record echo ----------------------------
rsprs3:		
	call	GSIGetFromRS232RxBuff
	bz	rsprs3		;wdt on error
	clrwdt
	subwf	R2BTemp,W	;length echo R2BTemp holds length
	btfss	STATUS,Z
	bsf	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX
	#IF DEBUG
	#IF GSI_LCD_SUPPORT
		movlw	'~'
		btfss	STATUS,Z
		call	GSILCDWriteChar
	#ENDIF
	#ENDIF
;----------------------------------------------
	;R2BTemp holds length, FSR0 points to data0
;Send/receive data[0] filled by caller
	;movlw	GSI_RS232_MESSAGE_SUCCESS
	;see if this node wants to be master (has data to send)
	btfsc	GlbSIOStatus1,SIO_S1_REQ_MASTER
	bsf	R2SubfunctionRecordData,GSI_RS232_MESSAGE_MASTER_REQ_BIT
;----- Begin loop to send data ---------------------
rsprs_loop:
	movf	INDF0,W		;get data
	call	GSISendByte232
	IF SIMULATOR
		movff	WREG,R2RxByte	
		call	GSIAddToRS232RxBuff
	ENDIF	
;-----Read the RT_GSI_RS232_MSSG echo -----------------------
rsprs4:		
	call	GSIGetFromRS232RxBuff
	bz	rsprs4		;wdt on error
	clrwdt
	subwf	POSTINC0,W
	btfss	STATUS,Z
	bsf	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX
	#IF DEBUG
	#IF GSI_LCD_SUPPORT
		movlw	'~'
		btfss	STATUS,Z
		call	GSILCDWriteChar
	#ENDIF
	#ENDIF
	decf	R2BTemp		;R2BTemp holds length of record
	bnz	rsprs_loop	
;-------- End data send loop ---------------------
;----------------------------------------------------
;----CHECK SIO_S1_ERR_ECHO_TX and send repeat if reqd
	btfsc	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX	;echo-error sending other byte
;An echo error: handle it by sending a REPEAT
	bra	rsprs_echo_error	
;----------------------------------------------------
;-----record was sent and received ok. Send an ACK. PC will echo --
	movlw	RT_GSI_ACK	
	call	GSISendByte232
	IF SIMULATOR
		movff	WREG,R2RxByte	
		call	GSIAddToRS232RxBuff
	ENDIF	

;----------Read the   ACK. -------------------------------
;If this is a bad echo what do we do?
rsprs6:
	call	GSIGetFromRS232RxBuff
	bz	rsprs6		;wdt on error
	clrwdt
	sublw	RT_GSI_ACK
	bz	rsprs_exit		;Echo-error sending ACK!
;------ACK Read (can also be an echo error!)----------------	
rsprs_exit:
	;bcf	LATC,0

	return
;----------Echo Error -------------------------------
rsprs_echo_error:
;One of the bytes had a bad echo
;Send REPEAT
;if echo fails on REPEAT then Init the USART and send nulls
;to re-sync with pc
	DEBUG_LCD_WRITE_CHAR	'~'

	bcf	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX
;Send/receive REPEAT
	movlw	RT_GSI_REPEAT
	call	GSISendByte232
	IF SIMULATOR
		movff	WREG,R2RxByte	
		call	GSIAddToRS232RxBuff
	ENDIF	
rsprsee:		;Read RT_GSI_REPEAT
	;-num of times to repeat a failed RT_REPEAT
	call	GSIGetFromRS232RxBuff
	bz	rsprsee		;wdt on error
;Check for the exceptional case where the pc +might+ have
;received an ACK in error, though we sent a REPEAT. 
;It also might have received and echoed a REPEAT but we received an ACK
;Whatever the case this is effectively an unrecoverable error
;so Init the USART and attempt to re-sync with the pc.
;On second thoughts if this error occurs we can still send 
;REPEAT as the pc will be looking for the next record. When
;this turns out to be REPEAT it can discard last command
;or warn the user.
	;sublw	RT_GSI_ACK
	;bz	rsprsee1	;found an ACK, major problem!
	;addlw	RT_GSI_ACK	;recover received byte
	;sublw	RT_GSI_REPEAT
	;bz	rsprs_start
	;decf	RBTemp		;Send this many failed RT_REPEAT's
	;bnz	rsprsee_start	;send another REPEAT
;----------------------------------------------
	;Failed to send a REPEAT so Init the USART
;Failing to find an echoed REPEAT means we have had two failures
;in one record, best if we simply fail.

	sublw	RT_GSI_REPEAT
	IF SIMULATOR
	btfsc	STATUS,Z
	goto	rsprs_start
	ELSE
	bz	rsprs_start
	ENDIF
	;if we have a comms problem, let it loop. As it will
	;eventually wdt
rsprsee_exit:
	;bcf	LATC,0

	call	RS232SendReSyncNulls	;will also InitUSART
	return


;-----------------------------------------------------
;		RS232SendProcessRecordFailure

;-----------------------------------------------------
R2SendProcessRecordFail

;fill in this stub!!!
;error code in WREG
;R2SubfunctionRecord filled with correct data
;success record holds:
;length 1 byte	0-R2_MAX_TX_RECORD_DATA (0 means no following data)
;data   0-R2_MAX_TX_RECORD_DATA bytes
;Modifies:
;FSR0
;WREG
;BSR
	movff	WREG,R2SubfunctionRecordData+1
	
	movlw	GSI_RS232_MESSAGE_FAIL
	movff	WREG,R2SubfunctionRecordData
	
	movlw	2
	movff	WREG,R2SubfunctionRecordLength
	
	call	R2SendProcessRecordSuccessFail		
	return


	END

;-------Check to see if recordNode = NODE_THIS or CnThisNode
;if it does then we have a request to execute code on this node
;if it isn't then we have a request for another node.
;For the Terminal (ascii) commands this simply entails
;# filling in a CnRecord (set RecordType to CAN_ID_TERMINAL_COMMAND)
;and send the message
;-----check if this node or for remote node --------

	movlw	TRUE	
	movwf	R2CurrentFunctionThisNode ;Assume for this node
	;-if CAN not running an error will be generated later
	movlw	CAN_NODE_THIS		;test for special node value
	subwf	R2RxRecordNode,W
	bz	_prrr_1
	;now see if the requested node is actually this node
	;explicitly set, rather than using special value
	movff	CnThisNode,WREG		;get our node num
	subwf	R2RxRecordNode,W	;cmp to sent node
	btfss	STATUS,Z
	clrf	R2CurrentFunctionThisNode ;Not for this node
_prrr_1

;-------------------------------------------------------------
;RxRecord ascii terminal functions that must vector to
;this node or be converted to a CnRecord and broadcast
;R2CurrentFunctionThisNode:TRUE , for this node
;R2CurrentFunctionThisNode:FALSE, for remote node
;-------------------------------------------------------------
RxRecordControlCode:
;-An RxRecord was a terminal record (an ascii value) it might be
;for this or a remote node. If this node, call the jmp table directly
;if a remote node, fill out a CnRecord and send the mssg

	movlw	TRUE
	cpfseq	R2CurrentFunctionThisNode
	bra
;-for this node so:	
	call	R2SendProcessRecordSuccessFail
	movf	R2RxRecordType,W
	goto	GSIProcessRxByte_ctr_code	;continue processing as an LCD_MSSG
_rrcc:
;Ctr code but not for this node
