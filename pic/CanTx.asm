;******************************************************************************
;* @file    canTx.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;CANTx.asm
;Can Tx functions, including CANRS232 functions

#define IN_CANTX.ASM

;#include "p18xxx8.inc"
#include "GSI.inc"

;#IF DEVICE == _18F458
;#include "p18xxx8.inc"
;#ELSE
;#INCLUDE <P18F4680.inc>
;#ENDIF
	#IF GSI_CAN_SUPPORT

#include "rs232.inc"
#include "error.inc"
#include "can.inc"
#include "i2c.inc"		;only for debugging
#include "lcd.inc"


#include "CANPrTx.inc"



	GLOBAL SendCANRepeat232,SendCANAck232,SendCANErrorCode232
	GLOBAL SendCANFatalError232

	;Striclty speaking the first four messages are not functions
	;they are handshaking bytes, kept here as placeholders
	;and for possible special use
	GLOBAL  SendCANReserved232_1,SendCANReserved232_2
	GLOBAL  SendCANRS232Mssg232,SendCANRS232BreakMssg232
	GLOBAL SendCANLCDMssg232,SendCANKeypadMssg232,SendCANSPIMssg232
	GLOBAL  SendCANI2CMssg232,SendCANDIOMssg232,SendCANADCMssg232
	GLOBAL  SendCANMssg232
	GLOBAL  SendCANError232
	GLOBAL  SendCANIOCTLMssg232,SendCANGSIIOCTLMssg232
	GLOBAL  SendCANPingMssg232,SendCANReserved1_232,SendCANReserved2_232
	GLOBAL  SendCANReserved3_232,SendCANReserved4_232,SendCANReserved5_232

	GLOBAL	CANSendSync
	IF DEBUG
		GLOBAL FillCANTxRecord
	ENDIF
;from can.asm
	EXTERN	CnThisNode
;from lcd.asm
	EXTERN GSIProcessRxByteRS232Entry
	
;--------- variables all defined in CAN.asm
	EXTERN	CnErrorState,CnErrorState1,CnRxMssgId,CnRxBuff,CnRxBuffLength
	EXTERN	CnRxFlags,CnTxRecord,CnTxMssgId,CnTxBuff,CnTxBuffLength
	EXTERN	CnTxFlags,CnMode,CnTemp,CnTemp32,CnSyncCounter
	EXTERN	CnSlaveBase
	EXTERN	GlbCANState
;from lcd.asm
#IF GSI_LCD_SUPPORT
	EXTERN LCDWriteChar
#ENDIF	
;from rs232.asm
	EXTERN R2RxRecordNode,R2RxRecordData,R2RxRecordLength
	EXTERN R2ErrorState
	EXTERN R2SubfunctionRecord,R2SubfunctionRecordLength
	EXTERN R2SubfunctionRecordData

	IF DEBUG
		EXTERN DEBUGGSISendByte232,GlbDebugFlag
	ENDIF
;from r2Slave.asm
	EXTERN R2SendProcessRecordSuccessFail,R2SendProcessRecordFail
;from keypad.asm
	EXTERN GlbKeypadStatus,GlbKeypadKeyDown1,GlbKeypadKeyDown2
CAN_TX	CODE
;---------------------------------------------------------------
;		CANSendSync:
;Highest priority CAN message 
;must also send the current 16 bit timer to see how many ticks
;late the message is.
;This is currently called from within the high priority 
;ISR
;NOTE:
;This is an interrupt context function so it must preserve FSR0
;This should be performed in the ISR for the timer int. 
;We are now using timer3 as a high priority int, it is not saving
;the FSR's!
;---------------------------------------------------------------
CANSendSync:
	banksel	CnBank
	decfsz	CnSyncCounter
	bra	no_sync
	movlw	CAN_SYNC_POSTSCALER
	movwf	CnSyncCounter
css_1:
	lfsr	FSR0,CnTxBuff
		movf	CnThisNode	;Allows multiple masters?! Dangerous for SYNC
        movwf   POSTINC0	;later will be a timestamp
        movlw   0xaa	
        movwf   POSTINC0
        movlw	0x55
        movwf	POSTINC0
        ;mCANPrTxSendMsg  CAN_ID_SYNC,CnTxBuff,2,CAN_TX_FLAG_XTD_NO_RTR
	mCANPrTxSendMsg  CAN_ID_SYNC,CnTxBuff,3,CAN_TX_FLAG_XTD_NO_RTR
	banksel	CnBank	;restore our bank
        addlw   0x00            ;Check for return value of 0 in W
        bz      css_1         ;Buffer Full, Try again

	btg	LATC,1
	;movlw	'S'
	;call	DEBUGGSISendByte232
no_sync:
	return

;--------------------------------------------------------------
;These functions are called via rs232 from pc
;Or from CAN code, hence two entry points
;if rs232 must copy data from R2Record to CnTxBuff before sending
;any messages
;They are all to be CAN Tx'd or immediately control the pic subsystem
;if CAN_NODE_THIS
;---------------------------------------------------------------

;----------------------------------------------------------------
;	CAN message to send.
;Called with a filled CnRecord in user ram pointed to by FSR0
;232 suffix indicates the CnRecord is in an R2Record, just received
;so must load FSR0 with address of the R2Record
;Messages received down the rs232 line requesting that this node
;send a CAN message
;Copy the R2Record data to the CnBuff and mCANPrTxSendMsg
;Note that if mssg is CAN_NODE_THIS then the message is for the
;subsystem on this node and not for the CAN
;---------------------------------------------------------------

SendCANRepeat232		;0 data
GSISendCANRepeat
	movff	CnThisNode,CnTxBuff
	return
	
SendCANAck232		;0 data
GSISendCANAck
	movff	CnThisNode,CnTxBuff
	return
	
SendCANErrorCode232		;0
SendCANErrorCode		;0
	movff	CnThisNode,CnTxBuff
	return
	
SendCANFatalError232	;0
SendCANFatalError	;0
	movff	CnThisNode,CnTxBuff
	return
	
SendCANReserved232_1	;0
GSISendCANReserved_1	;0
	movff	CnThisNode,CnTxBuff
	return
	
SendCANReserved232_2	;0
GSISendCANReserved_2	;0
	movff	CnThisNode,CnTxBuff
	return

;----------------------------------------------------------------
;		SendCANRS232Mssg232
;This node has received an RT Record from the pc, despatch it.
;CAN_ID_RS232_MSSG received down RS232
;Send it down the CAN or to the rs232 on this node if node=CAN_NODE_THIS
;Assumes:
;FSR0=R2RxRecordNode 
;BSR=R2 Bank
;Changes R2RxRecordLength

;Data[0] subfunction
;GSI_RS232_MESSAGE_SUCCESS	EQU	0	
;GSI_RS232_MESSAGE_FAILURE	EQU	1
;GSI_RS232_MESSAGE_REQ_MASTER	EQU	2

;If receive REQ_SLAVE, then pc has allowed this node to be
;master. If sent, then this node has allowed pc to be master
;GSI_RS232_MESSAGE_REQ_SLAVE	EQU	3	
;GSI_RS232_MESSAGE_SEND_DATA	EQU	4
;GSI_RS232_MESSAGE_GET_DATA	EQU	5
;GSI_RS232_MESSAGE_n1		EQU	3
;GSI_RS232_MESSAGE_n2		EQU	4
;GSI_RS232_MESSAGE_n3		EQU	5

;----------------------------------------------------------------
SendCANRS232Mssg232

	banksel	R2Bank
	IF DEBUG
		;movlw	'm'
		;call	GSILCDWriteChar
	ENDIF
	movlw	CAN_NODE_THIS
	;sublw	R2RxRecordNode
	;btfsc	STATUS,Z
	cpfseq	R2RxRecordNode
	bra	GSISendCANRS232Mssg	;a mssg to the CAN

	IF DEBUG
		;movlw	'M'
		;call	GSILCDWriteChar
	ENDIF

;a mssg to this node, this will be control data of some sort 
;though could be data to send back (?? for debugging??)
	lfsr	FSR0,R2RxRecordData	;point to the data bytes
scrm1:
	movf	POSTINC0,W	;get byte and inc
	;-
	bz	RS232Success	
	decf	WREG,W
	bz	RS232Failure
	decf	WREG,W
	bz	RS232ReqSlave
	decf	WREG,W
	bz	RS232ReqMaster
	decf	WREG,W
	bz	RS232SendData
	decf	WREG,W
	bz	RS232GetData
	return

;------------------------------------------------
;For the RS232 function (effectively ioctl call)
;FSR0 is pointing to the 1st data byte
;------------------------------------------------
;-----------------------------------------
;	RS232Success
;-----------------------------------------
RS232Success
;This node currently master, the pc will send these messages
;after despatching RT records sent by this node.
;If pc wishes to be MASTER it will send
;D0:7 set (GSI_RS232_MESSAGE_MASTER_REQ_BIT)
	return
;----------------------------------------
;	RS232Failure
;---------------------------------------
RS232Failure

	return

;----------------------------------------
;	RS232ReqMaster
;---------------------------------------
RS232ReqMaster
;?? shouldn't be called. The pc will send a MSSG_SUCCESS
;with D0:7 set to indicate it want to be master. The MSSG_SUCCESS
;record is sent by the slave once it has received data from
;the master and despatched the message.

	return

;----------------------------------------
;	RS232ReqSlave
;---------------------------------------
RS232ReqSlave
;pc has sent RT_ReqSlave. This will be in response to a 
;RT_REQ_MASTER that this node has sent earlier
;The pc has relinquished control and we are now master
;record has been echoed etc.
;SIO_S1_MASTER				EQU		0
;SIO_S1_DATA_PENDING_TX			EQU		1
;SIO_S1_REQ_MASTER			EQU		2 ;node wants to be 232 master
;SIO_S1_ESC				EQU		3
;SIO_S1_CHAR_AVAILABLE			EQU		4
;SIO_S1_ERR_ECHO_TX			EQU		5 ;also see R2ErrState variable for general Error handling

	IF DEBUG
		;call	GSIHome
		;movlw	'M'
		;call	GSILCDWriteChar
	ENDIF 
	;clrf	GlbSIOStatus1
	movlw	1 << SIO_S1_CAN_DATA_PENDING_TX 	;set PENDING_TX bit for mask
	andwf	GlbSIOStatus1			;clr flag leaving TX bit unchanged
	;
	clrf	R2ErrorState
	bcf	GlbErrState,GSI_ERR_RS232		;clr any errors
	call	GSIFlushR2RxBuffer
#IF	0
	IF DEBUG
		movlw	0x40+LCD_COLS-1
		LCD_DDRAM_ADDRESS
		movlw	'M'
		call	LCDWriteChar
		call	GSISetCursor
	ENDIF
#ENDIF	
	;-report back the success
	movlw	1		;1 data byte
	movwf	R2SubfunctionRecordLength
	movlw	GSI_RS232_MESSAGE_SUCCESS
	movwf	R2SubfunctionRecordData
	call	R2SendProcessRecordSuccessFail	

	bsf	GlbSIOStatus1,SIO_S1_MASTER	;we are MASTER

	return
;----------------------------------------
;	RS232SendData
;---------------------------------------
RS232SendData
	return

;----------------------------------------
;	RS232GetData
;---------------------------------------
RS232GetData
	return

;----------------------------------------------------------------
;GSICANRS232Mssg
;CAN_ID_RS232_Mssg record to be sent down the CAN, or back down
;This function called by user code, shouldn't ever have a CAN_NODE_THIS
;
;----------------------------------------------------------------
GSISendCANRS232Mssg		;5 data 4=node 1=Mssg
	lfsr	FSR0,R2RxRecordNode 
	
	;movff	CnThisNode,CnTxBuff	;FillCanTx overwrites this
	call	FillCANTxRecord		;fsr0=source
	banksel CnBank
	;now must or in the CAN_ID_I2C_MSSG d'4070' 0x7e6
	;movlw	UPPER CAN_ID_RS232_MSSG
	;iorwf	CnTxMssgId+2
	;movlw	HIGH CAN_ID_RS232_MSSG
	;iorwf	CnTxMssgId+1
	;movlw	LOW CAN_ID_RS232_MSSG
	;iorwf	CnTxMssgId
	movlw	CAN_TX_FLAG_XTD_NO_RTR
	movwf	CnTxFlags
gscrm
	mCANPrTxSendMsg_IID_IDL_IF CnTxMssgId,CnTxBuff,CnTxBuffLength,CnTxFlags
	banksel	CnBank	;above function will change bank
        addlw   0x00            ;Check for return value of 0 in W
        bz      gscrm         ;Buffer Full, Try again

	return

;-----------------------------------------------------
;SendCANRS232BreakMssg232
;A placeholder, this should not be called as the 
;pc shouldn't be sending us these
;------------------------------------------------------	
SendCANRS232BreakMssg232
	return


;----------------------------------------------------------------
;	SendCANLCDMssg232
;Send the record obtained from rs232 to the CAN as an LCD mssg
;or directly to this nodes LCD if CAN_NODE_THIS
;After completion call R2SendProcessRecordSuccess/Failure
;to signify success or failure
;Receives bsr:R2
;----------------------------------------------------------------
SendCANLCDMssg232

;assume success for both local and remote
;set up for the data in the subfuction SUCCESS/FAIL
;assume success
	movlw	1		;1 data byte
	movwf	R2SubfunctionRecordLength
	movlw	GSI_RS232_MESSAGE_SUCCESS
	movwf	R2SubfunctionRecordData
;---------------------------------------------
	;check if this is a record for this or a remote node	
	movlw	CAN_NODE_THIS		;test for special node value
	subwf	R2RxRecordNode,W
	bz	_sclm_this
	;now see if the requested node is actually this node
	;explicitly set, rather than using special value
	movff	CnThisNode,WREG		;get our node num
	subwf	R2RxRecordNode,W	;cmp to sent node
	bnz	GSISendCANLCDMssg	;A CAN messg
;--------direct use of the LCD on this node---------------
_sclm_this
	lfsr	FSR0,R2RxRecordData	;point to the 8 data bytes
	movf	POSTINC0,W	;get byte and inc
;WREG holds type of LCD command
	bz	Rs232LCDInstruction	;0
	decf	WREG
	bz	Rs232LCDWriteStr	;1
	decf	WREG
	bz	Rs232LCDWriteChar	;2
	decf	WREG
	bz	Rs232LCDGetStr		;3
	decf	WREG			
	bz	Rs232LCDCtrJmpTable	;4
	decf	WREG			
	bz	Rs232LCDGetCapability
	movlw	GSI_RS232_MESSAGE_FAIL
	movwf	R2SubfunctionRecordData
	return
;-----------------------------------------------------------------
;Read the LCD instructions from the record and despatch them
;Generic subfunction has been setup for SUCCESS
;-----------------------------------------------------------------
Rs232LCDInstruction
#IF GSI_LCD_SUPPORT
	banksel LdBank
	movf	POSTINC0,W		;get byte and inc
	call	GSILCDInstruction	
	decfsz	R2RxRecordLength,F
	bra	Rs232LCDInstruction

	call	R2SendProcessRecordSuccessFail
#ELSE
	movlw	GSI_ERR_UNSUPPORTED
	call	R2SendProcessRecordFail
#ENDIF	
	return
;-----------------------------------------------------------------
;Read the chars from the record and despatch them
;?error check for length >8?? Best done at record assembly time
;-----------------------------------------------------------------
Rs232LCDWriteStr
Rs232LCDWriteChar
	decf	R2RxRecordLength,F	;step over the subfunction data R2RxRecordData[0]
	bz	r2lwc_exit	
r2lwc:
	movf	POSTINC0,W		;get byte and inc
	call 	GSIProcessRxByteRS232Entry
	decfsz	R2RxRecordLength,F
	bra	r2lwc
r2lwc_exit:
	call	R2SendProcessRecordSuccessFail
	return
Rs232LCDGetStr
	call	R2SendProcessRecordSuccessFail
	return
;-----------------------------------------------------------------
;Read the LCD Ctr codes from the record and despatch them
;Will also write the char if it is not a Ctr code
;-----------------------------------------------------------------
Rs232LCDCtrJmpTable
;Too dangerous to stuff the chars in the RxBuff as this is an IntTime
;function. Could disable RxInt but could then mix up these chars
;with newly Rx'd ones. A recipe for disaster. Much better to call the
;jmp table directly.
;Note this function will also write ascii text
	banksel	R2Bank
	decf	R2RxRecordLength,F	;step over the subfunction data R2RxRecordData[0]
	bz		r2lcjt_exit			;no data with this function
R2ljtNextCommand:
	movf	POSTINC0,W			;get byte and inc
	call	GSIProcessRxByteRS232Entry	;execute the function
	decfsz	R2RxRecordLength,F
	bra		R2ljtNextCommand
r2lcjt_exit:	
	call	R2SendProcessRecordSuccessFail
	return
;------------------------------------------------------------------
;Send back the num rows cols and other relevant data
;send data back in the 232 record if it on this node
;otherwise it will go back as an asynchronous record
;FSR0 points to data[1], this is free for sending the data back
;as the success fail is success
;------------------------------------------------------------------
Rs232LCDGetCapability
	banksel R2Bank
	
	movlw	HIGH LCD_COLS
	movwf	POSTINC0
	movlw	LOW LCD_COLS
	movwf	POSTINC0
	
	movlw	HIGH LCD_LINES
	movwf	POSTINC0
	movlw	LOW LCD_LINES
	movwf	POSTINC0
	
	movlw	0
	movwf	POSTINC0				;zero next byte as we can use as byte of 8 flags, (0=graphics,1=xxx,)

	movlw	5					;5 extra bytes going back	
	addwf	R2SubfunctionRecordData,F
	
	call	R2SendProcessRecordSuccessFail

;--------------------------------------------------------
;	GSISendCANLCDMssg
;Send LCD message to the CAN
;On success/failure call R2SendProcessRecordSuccess/Failure
;to signify success/failure
;data is in R2Record, FSR0 points to R2RecordNode
;--------------------------------------------------------
GSISendCANLCDMssg
	banksel CnBank
;--------Are we running--------------------------
	btfss	CnMode,CAN_MODE_RUNNING
	bra	_sclm_error_exit
;--------ok CAN is running so tx the mssg

	lfsr	FSR0,R2RxRecordNode
	call	FillCANTxRecord	 ;fsr0=src
;Next piece of code is probably redundant as the pc should
;fill in the CAN mssg portion of the R2Record with the
;correct LCD message id shifted for node.
;now must or in the CAN_ID_LCD_MSSG d'4070' 0x7e6
	movlw	UPPER CAN_ID_LCD_MSSG
	iorwf	CnTxMssgId+2,F
	movlw	HIGH CAN_ID_LCD_MSSG
	iorwf	CnTxMssgId+1,F
	movlw	LOW CAN_ID_LCD_MSSG
	iorwf	CnTxMssgId,F
;--------------Set flag to be Xtd data frame-------------
	movlw	CAN_TX_FLAG_XTD_NO_RTR ;| CAN_TX_NO_RTR_FRAME
	movwf	CnTxFlags
;---------------Send the msg----------------------------
_sclm2:
	mCANPrTxSendMsg_IID_IDL_IF CnTxMssgId,CnTxBuff,CnTxBuffLength,CnTxFlags
	banksel	CnBank
        addlw   0x00            ;Check for return value of 0 in W
        bz      _sclm2         ;Buffer Full, Try again
;--------CAN send msg loop. wdt on error-----------------
_sclm_exit
;-------Inform the pc that the message was despatched-----------
	banksel	R2Bank
	call	R2SendProcessRecordSuccessFail
	return
_sclm_error_exit
;-------Inform the pc that the message was NOT despatched-------
	banksel	R2Bank
	movlw	GSI_RS232_MESSAGE_FAIL
	movwf	R2SubfunctionRecordData
	call	R2SendProcessRecordSuccessFail
	return

;------------- End RS232LCD code ---------------------------------


;-------------CAN Keypad code-------------------------------------

SendCANKeypadMssg232		;5 data 4=node 1=Mssg
;message to keypad received from rs232
;can be:
;KD_READ, Read the KeypadStatus flags
;KD_WRITE, currently meaningless as keypad not intelligent
;though can mimic keypress on a node?
;This function must send a GSI_RS232_MESSAGE, SUCCESS/FAILURE
;---------------------------------------------------------
;assume success for both local and remote
;set up for the data in the subfuction SUCCESS/FAIL
;assume success
	movlw	1		;1 data byte
	movwf	R2SubfunctionRecordLength
	movlw	GSI_RS232_MESSAGE_SUCCESS
	movwf	R2SubfunctionRecordData
;-------check if this is a record for this or a remote node----	
	movlw	CAN_NODE_THIS		;test for special node value
	subwf	R2RxRecordNode,W
	bz	_sckm_this
;now see if the requested node is actually this node
;explicitly set, rather than using special value
	movff	CnThisNode,WREG		;get our node num
	subwf	R2RxRecordNode,W	;cmp to sent node
	bnz	GSISendCANKeypadMssg	;A CAN messg
;---------direct use of the KD on this node reqd--------------	
_sckm_this:
	lfsr	FSR0,R2RxRecordData	;point to the 8 data bytes
	movf	POSTINC0,W	;get byte and inc
;WREG holds type of Keypad command
	bz	KdRead			;0, sends back data in success record
	decf	WREG
	bz	KdWrite			;1, receives data in R2RxRecord
;-------failed. Fill the generic subfunction record
;length is already filled in by this function 
	movlw	GSI_RS232_MESSAGE_FAIL
	movff	WREG,R2SubfunctionRecordData
;--Let failure fall through so that it returns the keypad status bytes
sCANKdMssg232_success:		;common exit point for success
;-always send back the keypad status bytes
;first byte (d0) already filled in with SUCCESS or FAILURE
	movlw	4
	movff	WREG,R2SubfunctionRecordLength
	;d1
	movff	GlbKeypadStatus,R2SubfunctionRecordData+1
	;d2
	movff	GlbKeypadKeyDown1,R2SubfunctionRecordData+2
	;d3
	movff	GlbKeypadKeyDown2,R2SubfunctionRecordData+3
	;send record
	call	R2SendProcessRecordSuccessFail	
	return
KdRead:
KdWrite:
	bra	sCANKdMssg232_success
;---------------------------------------------------------------
; A keypad message to be sent to a remote node
;Send R2SendProcessRecordSuccessFail on succesful Tx of CAN mssg
;---------------------------------------------------------------
GSISendCANKeypadMssg
;message to keypad called from user/node code. 
;ie a key has been pressed (raw mode) or released (not raw mode)
;CAN record is pointed to by FSR0, should be in user/gsi memory
;the CAN code will copy to Cn code etc.
;This should perhaps be a RTR_FRAME as we are sending a mssg to 
;keypad code. This will be a request for data. 
;Might keep to data code method
	return

;------------- END Keypad code -------------------------------------

SendCANSPIMssg232		;6 data 4=node 1=address 1=Mssg
GSISendCANSPIMssg
	return

;---------------------------------------------------------------
;	GSISendCANI2CMssg/232
;Send the CAN_I2_Mssg message in CnTxBuff
;If rs232 the data is in the R2Record struct
;First byte is node number (0-250) mcp2551 can drive 112 nodes
;second byte is data length(0-8)
;source in FSR0 byte 0=node (0-255) byte1-9=data
;Assumes:
;FSR0=R2RxRecordNode
;
;---------------------------------------------------------------
SendCANI2CMssg232		;5 data 4=node 1=Mssg
;Assumes FSR0=
GSISendCANI2CMssg
	lfsr	FSR0,R2RxRecordNode
	call	FillCANTxRecord	;fsr0=src
	banksel CnBank
	;now must or in the CAN_ID_I2C_MSSG d'4070' 0x7e6
	movlw	UPPER CAN_ID_I2C_MSSG
	iorwf	CnTxMssgId+2
	movlw	HIGH CAN_ID_I2C_MSSG
	iorwf	CnTxMssgId+1
	movlw	LOW CAN_ID_I2C_MSSG
	iorwf	CnTxMssgId

	movlw	CAN_TX_FLAG_XTD_NO_RTR ;| CAN_TX_NO_RTR_FRAME
	movwf	CnTxFlags
gscim:
	mCANPrTxSendMsg_IID_IDL_IF CnTxMssgId,CnTxBuff,CnTxBuffLength,CnTxFlags
        ;mCANPrTxSendMsg CAN_ID_I2C_Mssg,CnTxBuff,1,CAN_TX_FLAG_XTD_NO_RTR
	banksel	CnBank
        addlw   0x00            ;Check for return value of 0 in W
        bz      gscim         ;Buffer Full, Try again
	return

SendCANDIOMssg232		;6 data 4=node 1=dio 1=Mssg	
GSISendCANDIOMssg
	return
SendCANADCMssg232		;6 data 4=node 1=adc 2=Mssg/data
GSISendCANADCMssg
	return	
 
;GSICANMaster232		;4 data 4=node (stand alone master:no pc master)
;GSICANMaster
;	return
;------------------------------------------------------------
;	Send an arbitrary CAN message.
;PC can use this rather than sending specific CAN messages.
;PC then responsible or filling in the id field to vector to
;correct node
;------------------------------------------------------------
SendCANMssg232		;0-8 data
;RxRecord contains a (hopefully) correctly formatted CAN frame.
;Tx it. 
;We do not fill in the node information as this is not a call to 
;the GSI library, this is a request to place the frame on the CAN
;assume success for both local and remote
;set up for the data in the subfuction SUCCESS/FAIL
;assume success
	movlw	1		;2 data byte
	movff	WREG,R2SubfunctionRecordLength
	movlw	GSI_RS232_MESSAGE_SUCCESS
	movwf	R2SubfunctionRecordData
	
	;-----Send the received frame from RS232 down the CAN
	;Note that for this arbitrary frame Tx the rs232 will send
	;the frame in its entirety, that is, the frame will always be
	;4+8+1+1 bytes long
	;lfsr	FSR0,R2RxRecordNode
;can't use FillCANTxRecord as it will use the node to calculate
;a CAN ID and expects data in R2Record format.
	movlw	CAN_XFRAME_LENGTH		;14 bytes
	lfsr	FSR0,R2RxRecordData		;src
	lfsr	FSR1,CnTxMssgId			;dst
_scm_fill
	movff	POSTINC0,POSTINC1
	decfsz	WREG
	bra	_scm_fill
	
	call	R2SendProcessRecordSuccessFail	;rs232 function complete
	
GSISendCANMssg
	banksel	CnBank
	;movlw	CAN_TX_FLAG_XTD_NO_RTR ;| CAN_TX_NO_RTR_FRAME
	;movwf	CnTxFlags
_scm2:
	mCANPrTxSendMsg_IID_IDL_IF CnTxMssgId,CnTxBuff,CnTxBuffLength,CnTxFlags
        addlw   0x00            ;Check for return value of 0 in W
        bz      _scm2         ;Buffer Full, Try again
	banksel	CnBank

	banksel	R2Bank
	;call	R2SendProcessRecordSuccessFail, user code must complete this

	return



	
SendCANRxMssg232		;0-8 data
GSISendCANRxMssg
	return
SendCANError232		;0-8 data	
GSISendCANError
	return

;-------------CAN IOCTL code-------------------------------------

SendCANIOCTLMssg232		;5 data 4=node 1=Mssg
;Assumes BSR R2Bank
;---------------------------------------------------------
;assume success for both local and remote
;set up for the data in the subfuction SUCCESS/FAIL
;assume success
	movlw	2		;2 data byte
	movff	WREG,R2SubfunctionRecordLength
	movlw	GSI_RS232_MESSAGE_SUCCESS
	movwf	R2SubfunctionRecordData
;-------check if this is a record for this or a remote node----	
	movlw	CAN_NODE_THIS		;test for special node value
	subwf	R2RxRecordNode,W
	bz	_scim_this
;now see if the requested node is actually this node
;explicitly set, rather than using special value
	movff	CnThisNode,WREG		;get our node num
	subwf	R2RxRecordNode,W	;cmp to sent node
	bz	_scim_this
;-----Send the CAN_IOCTL received from RS232 down the CAN
	lfsr	FSR0,R2RxRecordNode
	call	FillCANTxRecord	 ;fsr0=src, changes to CnBank

	call	GSISendCANIOCTLMssg	;A CAN message
;called function will set FAILURE (actually wdt!) if a problem
	bra	_sCANIOCTLMssg232_successFail

;---------direct IOCTL to this node reqd--------------	
_scim_this:
;---point to the data bytes
	lfsr	FSR0,R2RxRecordData	;was +1 (addition 8/7/06)	
	movf	POSTINC0,W	;get byte and inc
;WREG holds type of IOCTL command
	bz	IOCTLGetNode	;0, sends back data in success record
	decf	WREG
	bz	IOCTLGetMode		;1, receives data in R2RxRecord
	decf	WREG
	bz	IOCTLSetMode
;-------failed. Fill the generic subfunction record-----------
;length is already filled (==2) in by this function 
	movlw	GSI_RS232_MESSAGE_FAIL
	movff	WREG,R2SubfunctionRecordData
	movlw	GSI_ERR_RANGE
	movff	WREG,R2SubfunctionRecordData+1
;--Let failure fall through so that it returns the keypad status bytes
_sCANIOCTLMssg232_successFail:		;common exit point for success
;first byte (d0) already filled in with SUCCESS or FAILURE
	call	R2SendProcessRecordSuccessFail	
	return
IOCTLGetNode:
	;movlw	2
	;movff	WREG,R2SubfunctionRecordLength ;set previously
	;d(0)=SUCCESS
;--------Send back CnThisNode in d(1)
	;d1
	movff	CnThisNode,R2SubfunctionRecordData+1
	bra	_sCANIOCTLMssg232_successFail
IOCTLGetMode:
	;movlw	2
	;movff	WREG,R2SubfunctionRecordLength
	;d(0)=SUCCESS
;--------Send back CnMode in d(1)
	;d1
	movlw	CAN_IOCTL_MODE_NORMAL
	movff	WREG,R2SubfunctionRecordData+1
	bra	_sCANIOCTLMssg232_successFail

IOCTLSetMode:
;Send back previous mode (only NORMAL at present)
	;movlw	2
	;movff	WREG,R2SubfunctionRecordLength
	;d(0)=SUCCESS
;--------Send back CnMode in d(1)
	;d1
	movlw	CAN_IOCTL_MODE_NORMAL
	movff	WREG,R2SubfunctionRecordData+1
	;movlw	2
	;movff	WREG,R2SubfunctionLength
	bra	_sCANIOCTLMssg232_successFail

;-------------------------------------------------
;SendCANIOCTLMssg
;SEND an IOCTL message to a remote node called from user/node code. 
;CAN record is pointed to by FSR0, should be in user/gsi memory
;the CAN code will copy to Cn code etc.
;Tx the CAN mssg
;NOTE:
;DO NOT send an rs232 record from this code as it might be called
;directly by user
;fill in the rs232 record and return. If in user code there will
;be no rs232 record sent. If called from rs232 code, the caller
;will despatch the RS23Record
;RS232Record already set with length=1 and SUCCESS
;-----------------------------------------------------
GSISendCANIOCTLMssg
	banksel CnBank
;--------Are we running--------------------------
	btfss	CnMode,CAN_MODE_RUNNING
	bra	_scim_error_exit
;--------ok CAN is running so tx the mssg
	;lfsr	FSR0,R2RxRecordNodeData
	;call	FillCANTxRecord	 ;fsr0=src
;Next piece of code is probably redundant as the pc should
;fill in the CAN mssg portion of the R2Record with the
;correct LCD message id shifted for node.
;now must or in the CAN_ID_LCD_MSSG d'4070' 0x7e6
	movlw	UPPER CAN_ID_IOCTL_MSSG
	iorwf	CnTxMssgId+2,F
	movlw	HIGH CAN_ID_IOCTL_MSSG
	iorwf	CnTxMssgId+1,F
	movlw	LOW CAN_ID_IOCTL_MSSG
	iorwf	CnTxMssgId,F
;--------------Set flag to be Xtd data frame-------------
	movlw	CAN_TX_FLAG_XTD_NO_RTR ;| CAN_TX_NO_RTR_FRAME
	movwf	CnTxFlags
;---------------Send the msg----------------------------
_scim2:
	mCANPrTxSendMsg_IID_IDL_IF CnTxMssgId,CnTxBuff,CnTxBuffLength,CnTxFlags
        addlw   0x00            ;Check for return value of 0 in W
        bz      _scim2         ;Buffer Full, Try again
	banksel	CnBank
;--------CAN send msg loop. wdt on error-----------------

;Don't send rs232 mssg from here. Let caller do it so that this
;code can be called by user/non rs232 code
	return
_scim_error_exit
;-------Inform the pc that the message was NOT despatched-------
	movlw	GSI_RS232_MESSAGE_FAIL
	movff	WREG,R2SubfunctionRecordData
	return

;------------- END CANIOCTL code -------------------------------------


;-------------CAN GSIIOCTL code-------------------------------------
;IOCTL to the GSI via the CAN and or RS232

SendCANGSIIOCTLMssg232		;5 data 4=node 1=Mssg
;Assumes BSR R2Bank
;---------------------------------------------------------
;assume success for both local and remote
;set up for the data in the subfuction SUCCESS/FAIL
;assume success
	IF DEBUG
	#IF GSI_LCD_SUPPORT
		banksel	LdBank
		movlw	'a'
		call	GSILCDWriteChar
		banksel R2Bank
	#ENDIF			
	ENDIF

	movlw	2		;2 data byte
	movff	WREG,R2SubfunctionRecordLength
	movlw	GSI_RS232_MESSAGE_SUCCESS
	movwf	R2SubfunctionRecordData
;-------check if this is a record for this or a remote node----	
	movlw	CAN_NODE_THIS		;test for special node value
	subwf	R2RxRecordNode,W
	bz	_scgim_this
;now see if the requested node is actually this node
;explicitly set, rather than using special value
	movff	CnThisNode,WREG		;get our node num
	subwf	R2RxRecordNode,W	;cmp to sent node
	bz	_scgim_this
;-----Send the CAN_IOCTL received from RS232 down the CAN
	lfsr	FSR0,R2RxRecordNode
	call	FillCANTxRecord	 ;fsr0=src, changes to CnBank

	call	SendCANGSIIOCTLMssg	;A CAN message
;called function will set FAILURE (actually wdt!) if a problem
	bra	_sCANGSIIOCTLMssg232_successFail

;---------direct IOCTL to this node reqd--------------	
_scgim_this:
#IF GSI_LCD_SUPPORT
	IF DEBUG
		banksel	LdBank
		movlw	'b'
		call	GSILCDWriteChar
		banksel R2Bank
	ENDIF
#ENDIF	
;--First data byte is node of caller, 2nd is subfunction--
	lfsr	FSR0,R2RxRecordData+1	;point to the 8 data bytes+1
	movf	POSTINC0,W	;get byte and inc
;WREG holds type of IOCTL command
	bz	_GSIIOCTLGetReg	;0, sends back data in success record
	decf	WREG
	bz	_GSIIOCTLSetReg		;1, receives data in R2RxRecord
	decf	WREG
	bz	_GSIIOCTL_1		;
	decf	WREG
	bz	_GSIIOCTL_2		;
	decf	WREG
	bz	_GSIIOCTL_3		
;-------failed. Fill the generic subfunction record-----------
;length is already filled (==2) in by this function 
	movlw	GSI_RS232_MESSAGE_FAIL
	movff	WREG,R2SubfunctionRecordData
	movlw	GSI_ERR_RANGE
	movff	WREG,R2SubfunctionRecordData+1
;--Let failure fall through so that it returns the keypad status bytes
_sCANGSIIOCTLMssg232_successFail:		;common exit point for success
;first byte (d0) already filled in with SUCCESS or FAILURE
	call	R2SendProcessRecordSuccessFail	
	return
_GSIIOCTLGetReg:
	;movlw	2
	;movff	WREG,R2SubfunctionRecordLength ;set previously
	;d(0)=SUCCESS
;--------Send back 6 contiguous Reg values in d(1)
	;d1
	IF DEBUG
		movlw	0xaa
		movwf	GlbDebugFlag
	ENDIF

	IF DEBUG
	#if GSI_LCD_SUPPORT	
		banksel	LdBank
		movlw	'c'
		call	GSILCDWriteChar
		banksel R2Bank
	#ENDIF			
	ENDIF

	movff	POSTINC0,FSR1H	;
	movff	POSTINC0,FSR1L	;get address of variable
;-----Point to R2Record to send back data---------	
	lfsr	FSR0,R2SubfunctionRecordData+1
	movlw	6
_gigr1
	movff	POSTINC1,POSTINC0
	decfsz	WREG
	bra	_gigr1
	movlw	7
	movff	WREG,R2SubfunctionRecordLength
	bra	_sCANGSIIOCTLMssg232_successFail
_GSIIOCTLSetReg:
	movff	POSTINC0,FSR1L	;
	movff	POSTINC0,FSR1H	;get address of variable
	lfsr	FSR0,R2SubfunctionRecordData+1
;-------loop to here
_gisr1
	movff	POSTINC1,POSTINC0
;--need to loop here if length of message suggests we do so
	movlw	2
	movwf	R2SubfunctionRecordLength
	bra	_sCANGSIIOCTLMssg232_successFail

	;movlw	2
	;movff	WREG,R2SubfunctionRecordLength
	;d(0)=SUCCESS
;--------Send back CnMode in d(1)
	;d1
	;movlw	CAN_IOCTL_MODE_NORMAL
	;movff	WREG,R2SubfunctionRecordData+1
	;bra	_sCANGSIIOCTLMssg232_successFail

_GSIIOCTL_1:
_GSIIOCTL_2:
_GSIIOCTL_3:
	bra	_sCANGSIIOCTLMssg232_successFail

;-------------------------------------------------
;SendCANGSIIOCTLMssg
;IOCTL of the GSI library 
;SEND an IOCTL message to a remote node called from user/node code. 
;CAN record is pointed to by FSR0, should be in user/gsi memory
;the CAN code will copy to Cn code etc.
;Tx the CAN mssg
;NOTE:
;DO NOT send an rs232 record from this code as it might be called
;directly by user
;fill in the rs232 record and return. If in user code there will
;be no rs232 record sent. If called from rs232 code, the caller
;will despatch the RS23Record
;RS232Record already set with length=1 and SUCCESS
;-----------------------------------------------------
SendCANGSIIOCTLMssg
	banksel CnBank
;--------Are we running--------------------------
	btfss	CnMode,CAN_MODE_RUNNING
	bra	_scgim_error_exit
;--------ok CAN is running so tx the mssg
	;lfsr	FSR0,R2RxRecordNode
	;call	FillCANTxRecord	 ;fsr0=src
;Next piece of code is probably redundant as the pc should
;fill in the CAN mssg portion of the R2Record with the
;correct IOCTL message id shifted for node.
;now must or in the CAN_ID_LCD_MSSG d'4070' 0x7e6
	movlw	UPPER CAN_ID_GSI_IOCTL_MSSG
	iorwf	CnTxMssgId+2,F
	movlw	HIGH CAN_ID_GSI_IOCTL_MSSG
	iorwf	CnTxMssgId+1,F
	movlw	LOW CAN_ID_GSI_IOCTL_MSSG
	iorwf	CnTxMssgId,F
;--------------Set flag to be Xtd data frame-------------
	movlw	CAN_TX_FLAG_XTD_NO_RTR ;| CAN_TX_NO_RTR_FRAME
	movwf	CnTxFlags
;---------------Send the msg----------------------------
_scgim2:
	mCANPrTxSendMsg_IID_IDL_IF CnTxMssgId,CnTxBuff,CnTxBuffLength,CnTxFlags
        addlw   0x00            ;Check for return value of 0 in W
        bz      _scgim2         ;Buffer Full, Try again
	banksel	CnBank
;--------CAN send msg loop. wdt on error-----------------

;Don't send rs232 mssg from here. Let caller do it so that this
;code can be called by user/non rs232 code
	return
_scgim_error_exit
;-------Inform the pc that the message was NOT despatched-------
	movlw	GSI_RS232_MESSAGE_FAIL
	movff	WREG,R2SubfunctionRecordData
	return

;------------- END CANGSIIOCTL code -------------------------------------

;------------------------------------------------------------
;	Send a CAN PING message.
;------------------------------------------------------------
SendCANPingMssg232		;0-8 data
;set up for the data in the subfuction SUCCESS/FAIL
;assume success
	movlw	1		;1 data byte
	movff	WREG,R2SubfunctionRecordLength
	movlw	GSI_RS232_MESSAGE_SUCCESS
	movff	WREG,R2SubfunctionRecordData
	banksel CnBank
	
	lfsr	FSR0,R2RxRecordNode
	call	FillCANTxRecord	 ;fsr0=src	sets CnBank
	;set:
	;data[0]=CnThisNode ;the recipient will ping back this node
	;data[1]=CAN_PING_SEND	;we are the originator of a PING
	
	;When the recipient collects the message it resonds with a PING
	;with:
	;data[0]=its node, so sender knows which node has responded in case of multiple pings
	;data[1]=CAN_PING_RECEIVED
	
;---------------Send the msg----------------------------
	movff	R2RxRecordData,WREG		;node to ping in WREG
	call	GSISendCANPingMssg		;returns WREG=0 success, WREG=1 fail
    addlw   0x00
    bz	_scpm_no_error	
   ;error	 
   	movlw	GSI_RS232_MESSAGE_FAIL
	movff	WREG,R2SubfunctionRecordData
_scpm_no_error:	
	call	R2SendProcessRecordSuccessFail	;
	return
;----------------------------------------------------	
;	GSISendCANPingMssg
;Entry point for user code (non rs232)	
;WREG=node to send the ping
;----------------------------------------------------
GSISendCANPingMssg
;Next piece of code is probably redundant as the pc should
;fill in the CAN mssg portion of the R2Record with the
;correct LCD message id shifted for node.
;now must or in the CAN_ID_PING_MSSG 
	banksel	CnBank	
	movwf	CnTxMssgId+1	;8bit shift of the node
	
	movlw	250
	cpfslt	CnThisNode		;will be 255 if no network, 0 if master etc
	bra		_gscpm_error	;CAN_THIS_NODE should never be set here, it is only
							;used in RS232 comms with the pc to refer to the local node
	;Now shift the node another 4 bits left for message_id. Already shifted
;by 8 from the initial load, so total of 12 bit shift 
;-----------performa 24 bit rol,4-------------------- 
	movlw	4
	clrf	CnTxMssgId
	clrf	CnTxMssgId+2
	clrf	CnTxMssgId+3
	bcf		STATUS,C			;mustn't forget to clear the carry flag!
_gscpm2
	rlcf	CnTxMssgId+1
	rlcf	CnTxMssgId+2
	rlcf	CnTxMssgId+3
	decfsz	WREG
	bra	_gscpm2
;CnTxMssgId hold the node of recipient correctly shifted

	movlw	UPPER CAN_ID_PING_MSSG
	iorwf	CnTxMssgId+2,F
	movlw	HIGH CAN_ID_PING_MSSG
	iorwf	CnTxMssgId+1,F
	movlw	LOW CAN_ID_PING_MSSG
	iorwf	CnTxMssgId,F
;--------------Set flag to be Xtd data frame-------------
	movlw	CAN_TX_FLAG_XTD_NO_RTR ;| CAN_TX_NO_RTR_FRAME
	movwf	CnTxFlags
;-----------------------------------------	
	movff	CnThisNode,CnTxBuff		;data[0]
	movlw	CAN_PING_SEND			
	movwf	CnTxBuff+1				;data[1]
;------Set length--------------------------
	movlw	2						;currently always 2
	movwf	CnTxBuffLength
;------------------------------------------		
;Receiver will reply asap with data[1]=CAN_PING_RECEIVED
_scpm1:
#if 1
	mCANPrTxSendMsg_IID_IDL_IF CnTxMssgId,CnTxBuff,CnTxBuffLength,CnTxFlags
	banksel	CnBank
    addlw   0x00            ;Check for return value of 0 in W
    bz      _scpm1         ;Buffer Full, Try again
#endif    
    movlw	'P'
	call DEBUGGSISendByte232
	;call	R2SendProcessRecordSuccessFail	;caller will send
	movlw	0
	return 
_gscpm_error:	
	movlw 1
	return  
;----End  SendCANPingMssg------------------------------


;-----------------------------------------------------------------
;		FillCANTxRecord	
;FSR0=address of R2RxRecordNode buffer to move to CnTxBuff
;WREG=length of data
;Byte 0 of source(1st byte of data)is always the node number (to be shifted left 12)	
;-----------------------------------------------------------------

;CnTxRecord
;CnTxMssgId	RES	04	;29 bit node id
;CnTxBuff	RES	08	;the data
;CnTxBuffLength	RES	01	;length of data
;CnTxFlags	RES	01

;Rs232
;R2RxRecordNode		RES 1	;the node this data is for
;R2RxRecordLength	RES 1   ;the number of bytes in record
;R2RxRecordData		RES 8	;the data 0-8 bytes
;R2RxRecordFlags	RES 1	;currently unused

FillCANTxRecord

;If arriving through an Rs232 record, FSR0 points to R2RxRecordNode
	banksel	CnBank
	
	lfsr	FSR1,CnTxMssgId+1	;destination (shift left 8 bits)
	movff	POSTINC0,INDF1		;node number fsr0++
	
	movff	POSTINC0,CnTxBuffLength	;fsr0=record length fsr++=data buff

	lfsr	FSR1,CnTxBuff
	;now copy the buff
	movlw	8			;always copy all data
cftb1	
        movff   POSTINC0,POSTINC1 	;Copy LH byte
	decfsz	WREG					;doesn't effect any flags, including Z!
	bra	cftb1
	;fsr0=flags fsr1=length, so preinc1
	movff	INDF0,PREINC1	;PREINC steps over length to point to flags

;finally shift the node 12 bits left for message_id. Already shifted
;by 8 from the initial load
;must do a 24 bit rol,4. 
	movlw	4
	clrf	CnTxMssgId
	clrf	CnTxMssgId+2
	clrf	CnTxMssgId+3
	bcf		STATUS,C			;mustn't forget to clear the carry flag!
cftb2
	rlcf	CnTxMssgId+1
	rlcf	CnTxMssgId+2
	rlcf	CnTxMssgId+3
	decfsz	WREG
	bra	cftb2
;------CnTxMssgId is now rotated left 12 bits (x 4096) ---------------	

;-- Now check for valid length. If > 8 set to 8 (the max CAN message data length)
	movf	CnTxBuffLength,W
	sublw	8
	movlw	8
	btfss	STATUS,C
	movwf	CnTxBuffLength
;- ignore user flags and insert the TX_XTD_FRAME flag
	movlw	CAN_TX_FLAG_XTD_NO_RTR
	movwf	CnTxFlags
	return
;---------------------------------------------------------------
;	CanSendErrorMssg
;WREG=ERROR  type
;Send message to MASTER
;---------------------------------------------------------------
GSICANSendErrorMssg
	banksel	CnBank
	movff	CnThisNode,CnTxBuff	;let master know it was this node that has a problem
	movwf	CnTxBuff+1			;TxBuff
;--------Are we running--------------------------
	btfss	CnMode,CAN_MODE_RUNNING
	bra	_csem_exit
;--------ok CAN is running so tx the mssg
	;-
	clrf	CnTxMssgId+3
	movlw	UPPER CAN_ID_CRITICAL_ERROR
	movwf	CnTxMssgId+2
	movlw	HIGH CAN_ID_CRITICAL_ERROR
	movwf	CnTxMssgId+1
	movlw	LOW CAN_ID_CRITICAL_ERROR
	movwf	CnTxMssgId
;--- Fill length byte---------------------------------------------------
	movlw	2
	movwf	CnTxBuffLength	
;--------------Set flag to be Xtd data frame-------------
	movlw	CAN_TX_FLAG_XTD_NO_RTR ;| CAN_TX_NO_RTR_FRAME
	movwf	CnTxFlags
_csem:
	mCANPrTxSendMsg_IID_IDL_IF CnTxMssgId,CnTxBuff,CnTxBuffLength,CnTxFlags
	banksel	CnBank
        addlw   0x00            ;Check for return value of 0 in W
        bz      _csem         ;Buffer Full, Try again
_csem_exit:
	return

#if 0
;---------------------------------------------------------------
;	CanSendPingMssg
;WREG=ERROR  type
;Send message to any node
;node to direct message to is in WREG
;---------------------------------------------------------------
GSICANSendPingMssg
	banksel	CnBank
	movwf	CnTxBuff
;--------Are we running--------------------------
	btfss	CnMode,CAN_MODE_RUNNING
	bra	_cspm_exit
;--------ok CAN is running so tx the mssg
	;-
	clrf	CnTxMssgId+3
	movlw	UPPER CAN_ID_CRITICAL_ERROR
	movwf	CnTxMssgId+2
	movlw	HIGH CAN_ID_CRITICAL_ERROR
	movwf	CnTxMssgId+1
	movlw	LOW CAN_ID_CRITICAL_ERROR
	movwf	CnTxMssgId
;--- Fill length byte---------------------------------------------------
	movlw	2				;The node and the type of ping (SEND/RECEIVED)
	movwf	CnTxBuffLength	

;--------------Set flag to be Xtd data frame-------------
	movlw	CAN_TX_FLAG_XTD_NO_RTR ;| CAN_TX_NO_RTR_FRAME
	movwf	CnTxFlags
_cspm:
	mCANPrTxSendMsg_IID_IDL_IF CnTxMssgId,CnTxBuff,CnTxBuffLength,CnTxFlags
	banksel	CnBank
        addlw   0x00            ;Check for return value of 0 in W
        bz      _cspm         ;Buffer Full, Try again
_cspm_exit:
	return
#endif

;Placeholders for CAN232 functions being added
SendCANReserved1_232
SendCANReserved2_232
SendCANReserved3_232
SendCANReserved4_232
SendCANReserved5_232

	return
	
	#endif ;GSI_CAN_SUPPORT
	END
