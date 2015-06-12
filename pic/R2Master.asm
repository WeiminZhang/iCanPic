;******************************************************************************
;* @file    R2Master.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;R2Master.asm
;rs232 "master" functions.

#define IN_R2MASTER.ASM

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

	GLOBAL	R2TxRecordType,R2TxRecordNode,R2TxRecordLength,R2TxRecordData	
	GLOBAL	R2TxBuff,R2TxWCursor,R2TxRCursor,R2TxBuffEnd,R2TxNumInBuff

	EXTERN	GlbCompOffset,GlbLocalTempL
	
	EXTERN	R2RxByte,R2BTemp,R2RTRepeatCounter
	EXTERN	R2RxRecordCursor,R2RxNumInBuff
	EXTERN	R2RxRecordType,R2RxRecordNode,R2RxRecordLength,R2RxRecordData

	#IF GSI_CAN_SUPPORT
		EXTERN	CnRxBuffLength,CnThisNode,CnRxMssg,GSICANGetMsgFromR2CQ
		#if	ENABLE_CALLER_NODE_SENDBACK
			EXTERN CnRxBuff
		#endif	
	#ENDIF
;-------------------------	
;from gsi.am
;-------------------------	
	EXTERN GlbLocalTempL,GlbLocalTempH

RS232_DATA	UDATA
R2TxByte 			RES 1

;Is a Tx buffer reqd? We can simply use R2TxBuff (GSIAddToTxBuff)
;However as we haven't implemented Tx buffering then we can't do this!
;Mustn't change relative address!
R2TxRecordStructure	RES 0	;to remind us that this is a struct, will be accessed indirectly
R2TxRecordType		RES 1	;A binary Mssg
R2TxRecordNode		RES 1	;the node this data is *from* found in CAN buffer[0]
R2TxRecordLength	RES 1   ;the number of bytes in record
R2TxRecordData		RES R2_MAX_TX_RECORD_DATA	;the data 0-8 +4 bytes for CAN_ID +1 byte for RecFlags if a CAN mssg
;R2TxRecordCursor	RES 1	;Where we are in assembly of record
R2TxRecordError		RES 1	;0=no error,1=EchoError
;R2_MAX_RECORD_LEN =16 see rs232.inc


;These fields should not change order as we might refer to them
;using indirect addressing
R2TxCQStructure		RES	0	;just to let us know this is a structure
R2TxNumInBuff 		RES 1	;GSXXX 13/4/07 changed order of numInBuff and BuffEnd 
R2TxBuffEnd 		RES 1
R2TxWCursor 		RES 1
R2TxRCursor 		RES 1
R2TxBuff 			RES TX_BUFF_SIZE				



R2_MASTER CODE
;-----------------------------------------------------------------
;	RS232SendPendingCANMssg
;called from main loop when SIO_S1_MASTER and CAN_DATA_PENDING_TX
;are both set
;Check to see if any CAN mssgs need to be sent down the rs232
;This function called from main loop.
;The SIO_S1_MASTER flag is set
;S1_CAN_DATA_PENDING_TX is set.
;Note the PENDING flag is set +after+ the CAN mssg has been copied
;to the R2TxBuffer
;------------------------------------------------------------------
RS232SendPendingCANMssg
	;here!
	;banksel	R2TxRecordType
	
#IF GSI_CAN_SUPPORT

	lfsr	FSR0,CnRxMssg
	call	GSICANGetMsgFromR2CQ			;get the mssg from the rs232 CQ. Changes to CnBank
	;now in CnBank
	call	GSICopyCANRxBuffToR2TxRecord	;copy it to the buffer, filling in R2Record fields (no bank assumptions)
	;										;and send down the com port
	banksel	R2Bank	
	call	RS232TxRS232Record				;Assumes R2Bank
	;bcf	GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX
	;now must become slave
	;pc is still slave so must send a RS232_SET_SLAVE 

	clrwdt
#IF 0		;pc will automatically resume as master after 1 record
	movlw	RT_GSI_RS232_MSSG
	movwf	R2TxRecordType	;a generic rs232 message
	movlw	GSI_RS232_MESSAGE_REQ_SLAVE
	movwf	R2TxRecordData	;subfunction REQ_SLAVE (this node wants to be slave)		
	movlw	1
	movwf	R2TxRecordLength	;record data is 1 byte only
#if	ENABLE_CALLER_NODE_SENDBACK
	movff	CnRxBuff,R2TxRecordNode	;node of sender
#else
	movlw	CAN_NODE_THIS		;let pc know it is this node
	movwf	R2TxRecordNode
#endif		;IF 0
	call	RS232TxRS232Record	;and send
	;on error we will wdt
	call	GSIRS232SetSlave
#ENDIF		;ENABLE_CALLER_NODE_SENDBACK
#ENDIF		;GSI_CAN_SUPPORT
	call	GSIRS232SetSlave
	return
;-------------------------------------------------------------------
;	RS232TxRS232Record
;Send the RS232/CAN message in
;CnRxMssgId	RES	4		;the RX mess id
;CnRxBuff        RES     08		;The data bytes
;CnRxBuffLength  RES     01		;The data length
;CnRxFlags       RES     01
;Assumes node is RS232 master. That is it can Tx data and expect
;to see data echoed.
;This function should be called within main program loop

;on receipt of complete echoed record, Tx an ACK, and read its Echo
;
;R2TxRecordType		RES 1	;A binary Mssg
;R2TxRecordNode		RES 1	;the node this data is for
;R2TxRecordLength		RES 1   ;the number of bytes in record
;R2TxRecord		RES 10	;the data 0-10 bytes (0 as we need to send the CAN_ID and CAN_FLAGS)
;NOT USED R2TxRecordId		RES 1	;the CAN_ID if any
;NOT USED R2TxRecordFlags	RES 1	;any Cn flag to add (when sending a Cn mssg to rs232)
;The R2RecordType must be set for RT_?? by this function
;R2RecordNode will be the node the CAN mssg was vectored to (usually THIS)
;however might be nice to set a filter to capture ALL mssg's. The
;RecordNode can hold any node's mssg, the GSICopyCANRxBuffToR2TxRecord
;performs the necessary 12 bit rotation
;The data R2RxRecordData will hold 0-8 CAN data bytes +2 other bytes 
;being the CAN_ID and the CAN_FLAGS. These bytes will follow the
;CAN data. The R2Len will reflect the extra two bytes

;The caller must have performed BREAK sequence. It may then have
;informed pc that this is a single record, multiple records, or data
;------------------------------------------------------------------
RS232TxRS232Record
;Need to put a repeat loop counter in here
	movlw	4		;make 4-1=3 attempts to REPEAT
	movwf	R2RTRepeatCounter
	bcf	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX
	bcf	GlbSIOStatus1,SIO_S1_ERR_ECHO_ACK_TX	;ACK was echoed incorrectly! (currently ignored)

rtrr_start:
	decf	R2RTRepeatCounter
	bz	rtrr_error	;Echo error failure after REPEAT's	
	call	GSIFlushR2RxBuffer
	;Rx Buffer is empty
	bcf	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX ;clr error flag

	movf	R2TxRecordType,W	;get Rec type (filled in by CopyCANRxTo...)
	call	GSISendByte232		;send RecordType
rtrr2:
	;bra	grtcm2
	IF SIMULATOR
	ELSE
		call	GSIGetFromRS232RxBuff	;look for echo
		bz	rtrr2			;will wdt if failure
	ENDIF
	cpfseq	R2TxRecordType
	;bra	grtcm_echo_failed	;failed to receive RT_GSI_CAN_RX_MSSG
	bsf	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX

;--sent+read the recordType

;--now send the record Node, ?? shouldn't this be the node that sent the msg?
;Or do we send this as part of the CAN message if it is reqd
;Sending the node of this code is redundant information.
;Should send the node of sender (if there is one) CnThisNode otherwise
	;movff	CnThisNode,R2TxRecordNode
	movf	R2TxRecordNode,W
	call	GSISendByte232		;send RecordNode
rtrr3:	;check RecordNode echoed from pc

	IF SIMULATOR
	ELSE
		call	GSIGetFromRS232RxBuff	
		bz	rtrr3			;will wdt if failure
	ENDIF
	cpfseq	R2TxRecordNode				;check echo
	;bra	grtcm_echo_failed	;failed to receive RecordNode
	bsf	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX
;--sent+read the RecordNode

;--Now send+read the Record Length, add 4 bytes for CnRxMsgId and 1 byte CnRxFlags
	;movff	CnRxBuffLength,WREG
	;addlw	5			;CnRxMsgId+CnRxFlags
	;movwf	GlbLocalTempL		;used as a counter
	;movwf	R2TxRecordLength
	movf	R2TxRecordLength,W
	call	GSISendByte232		;send RecordLength
rtrr4:	;check for echo from pc
	IF SIMULATOR
	ELSE
		call	GSIGetFromRS232RxBuff	
		bz	rtrr4			;will wdt if failure
	ENDIF
	cpfseq	R2TxRecordLength		;check echo
	;bra	grtcm_echo_failed	;failed to receive RecordLength
	bsf	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX
 
;Send CnRxMsgId;CnRxMsgBuff;CnRxMsgFlags
	lfsr	FSR0,R2TxRecordData
	movff	R2TxRecordLength,GlbLocalTempL
	clrwdt
rtrr5:
	movf	INDF0,W
	call	GSISendByte232		
rtrr6:
	IF SIMULATOR
	ELSE
;BUG (fixed)
;While in this loop we can generate CAN interrupts. These modify FSR0!
;must find out where these access problems are. 
;ICD2 has good file reg access breakpoints	
		call	GSIGetFromRS232RxBuff	
		bz	rtrr6			;will wdt if failure
	ENDIF
	cpfseq	INDF0
	;bra	grtcm_echo_failed	;failed to receive correct data
	bsf	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX
	incf	FSR0L			;point to next data

	decfsz	GlbLocalTempL
	bra	rtrr5

;Now must send an ack if no error or REPEAT if an echo error
	btfss	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX
	bra	rtrr_send_ack
;-echo error. Send a REPEAT
;actually we need to send MAX_R2_RECORD_LEN REPEATS, as the
;error might have been in the length field. In this case the pc will
;set it to  MAX_R2_RECORD_LEN, we must send enough data for the pc
;to fill the record, then an extra REPEAT so it re-syncs
rtrr_sync_repeats:
	bcf	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX
	movf	R2TxRecordLength,W
	sublw	(R2_MAX_TX_RECORD_DATA+1)	;see how many repeats to send
	movwf	GlbLocalTempL
rtrr_rpt_loop:
	movlw	RT_GSI_REPEAT
	call	GSISendByte232
rtrr_7:
	IF SIMULATOR
		movlw	RT_GSI_REPEAT		;simulate success from pc
	ELSE
		call	GSIGetFromRS232RxBuff	
		bz	rtrr_7		;will wdt if failure
	ENDIF
	decfsz	GlbLocalTempL
	bra	rtrr_rpt_loop
	;only check the last REPEAT echo
	;If it isn't REPEAT we are stuffed!
	sublw	RT_GSI_REPEAT
	btfss	STATUS,Z		
	bra	rtrr_fatal_error	;failed to receive REPEAT
	bra	rtrr_start		;restart send loop

;--receive the ACK/REPEAT
rtrr_send_ack:

	movlw	RT_GSI_ACK
	call	GSISendByte232
rtrr_8
	IF SIMULATOR
		movlw	RT_GSI_ACK		;simulate success from pc
	ELSE
		call	GSIGetFromRS232RxBuff	
		bz	rtrr_8		;will wdt if failure
	ENDIF
	sublw	RT_GSI_ACK
	btfss	STATUS,Z		
	bra	rtrr_sync_repeats		;failed to receive ACK
;failure to receive ack. +might+ have been received ok. Ignore the
;error. The pc will either know there was an error (it didn't receive the ack)
;or it thinks all is ok (it rec'd an ack, but we didn't). This is ok
;and we can afford to ignore
;Should we perhaps keep sending REPEAT's?? Another special meaning single byte??
rtrr_exit:
	bcf		GlbSIOStatus2,SIO_S2_RS232_DATA_PENDING_TX	;clear pending rs232 record flag
	;if this was a CAN message the caller will be the CAN Tx function which will clear
	;the CANPending flag 
	return

rtrr_error:
;Error. 
;Send R2_MAX_TX_RECORD_LEN RT_ERROR codes (another special byte)
;Note that this is also an implicit repeat of the record
rtrr_sync_error
	bcf	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX
	movf	R2TxRecordLength,W
	sublw	(R2_MAX_TX_RECORD_DATA+1)	;see how many repeats to send
	movwf	GlbLocalTempL
rtrr_err_loop
	movlw	RT_GSI_ERROR
	call	GSISendByte232
rtrr_9:
	IF SIMULATOR
		movlw	RT_GSI_ERROR		;simulate success from pc
	ELSE
		call	GSIGetFromRS232RxBuff	
		bz	rtrr_9			;will wdt if failure
	ENDIF
	decfsz	GlbLocalTempL
	bra	rtrr_err_loop
	;only check the last REPEAT echo
	;If it isn't REPEAT we are stuffed!
	sublw	RT_GSI_ERROR
	btfss	STATUS,Z		
	bra	rtrr_fatal_error	;failed to receive ERROR
	bra	rtrr_start		;restart send loop
	return

rtrr_fatal_error:
	bcf	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX
	movf	R2TxRecordLength,W
	sublw	(R2_MAX_TX_RECORD_DATA+1)	;see how many repeats to send
	movwf	GlbLocalTempL
rtrr_fatal_err_loop
	movlw	RT_GSI_ERROR
	call	GSISendByte232
rtrr_10:
	IF SIMULATOR
		movlw	RT_GSI_ERROR		;simulate success from pc
	ELSE
		call	GSIGetFromRS232RxBuff	
		bz	rtrr_10	;will wdt if failure
	ENDIF
	decfsz	GlbLocalTempL
	bra	rtrr_fatal_err_loop
rtrr_wdt:
	bra	rtrr_wdt		;resart, hopefully pc has got the message!			
	return

;------------------------------------------------------------------------
;GSIRS232ReqMaster
;This node wishes to be the rs232 master as it has data to send.
;node controls the serial port, we Tx and the PC echoes
;Any code requiring to send data (currently only the CAN) will
;# bsf GlbSioStatus,SIO_S1_REQ_MASTER
;# Fill the TXRecord (GSICopyCANRxBuffToR2TxRecord for a CAN mssg)
;# bsf GlbSIOStatu1s,SIO_S1_TX_DATA_PENDING
;# return
;
;The BinaryProcessRxByte code will receive any pending records, then send
;RT_GSI_RS232_MSSG 
;d0=REQ_MASTER
;It will handle all handshaking issues and once all is ok it will:
;# bsf GlbSIOStatus1,SIO_S1_MASTER
;# bcf GlbSIOStatus1,SIO_S1_REQ_MASTER
;# exit

;The top level loop will check for SIO_S1_MASTER and call Current232Master
;Current232Master will perform any pending tasks, including Txing data

;-------------------------------------------------------------------------
GSIRS232ReqMaster
	btfsc	GlbSIOStatus1,SIO_S1_MASTER	;already master
	return
;ascii mode:
;#
;Binary mode
;#Wait till any pending records are sent/complete. 
;#Send a RT_RS232_SET_MASTER
;#Wait for an ACK followed by an Echo of RT_RS232_REQ_MASTER
;NOTE. There will be no record pending as the receipt of this request occurs
;synchronously. Does a CAN message fill the RS232Record? Does it matter? As we
;can ignore it.
;Currently can see no need for special handling of changing from ascii mode

	;btfss	GlbSIOMode,SIO_M_BINARY
	;bra	grmAscii
	;in binary mode and must set this node to be the master
	call	GSIFlushR2RxBuffer	;flush the Rx Buff
	;Rx Buffer is empty
	bcf	GlbSIOStatus1,SIO_S1_ESC	;ignore any ESC character we might have received
	;bcf	GlbSIOStatus1,SIO_S1_LITERAL	;ignore any LITERAL character we might have received
	bsf	GlbSIOMode,SIO_M_BINARY
	bsf	GlbSIOStatus1,SIO_S1_MASTER			
	bcf	GlbSIOStatus1,SIO_S1_ERR_ECHO_TX ;clr error flag
	return
;-----------------------------------------------------------
;Need to reset any flags that might be concerned with ESC 
;grmAscii:
;	bcf	GlbSIOStatus1,SIO_S1_ESC	;ignore any ESC character we might have received
;	bsf	GlbSIOMode,SIO_M_BINARY
;gr2m2:
;	goto	gr2m_exit
;-----------------------------------------------------------


;----------------------------------------------------------------------
;	RS232MakeThisNodeMaster
;Called from BinaryProcessRxByte after it has processed the R2Record
;This function handles the handshaking and sets us up as a master
;----------------------------------------------------------------------

RS232MakeThisNodeMaster
;The BinaryProcessRxByte code will receive any pending records
;then send
;RT_GSI_RS232_MSSG 
;d0=REQ_MASTER
;It will handle all handshaking issues and once all is ok it will:
;# bsf GlbSIOStatus1,SIO_S1_MASTER
;# bcf GlbSIOStatus1,SIO_S1_REQ_MASTER
;# exit

	return

;-----------------------------------------------------------------
;GSIRS232SendCanMssg

;Send a CAN message currently in the CAN R2 buffer down the rs232
;line. Make this node the master if required.
;Note that the main program loop will call RS2322TxCANMssg to
;actually send the data
;if already RS232Master then set SIO_S1_DATA_PENDING_TX and exit
;Note that the PENDING_TX flag should only be set after CAN data
;has been copied to R2RxBuffer.
;This is done in GSICopyCANRxBuffToR2TxRecord
;-----------------------------------------------------------------
GSIRS232SendCANMssg

	;bsf	GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX	;data pending

	;btfsc	GlbSIOStatus1,SIO_S1_MASTER
	;return	;already master so let main loop despatch

	;bsf	GlbSIOStatus1,SIO_S1_REQ_MASTER
	;next data record received, we will send a request to the pc
	;PC must be continually sending data for this to work.
	;?? let pc send nulls when it is otherwise idle.
	;this node responds with a null if doesn't want to be master
	;or an rs232Record if it does.
	;Indeed on receipt of a null, we can send the record in question
	;or an rs232Record REQ_MASTER
	

	return

;----------------------------------------------------------------
;	GSICANDataPendingTxToRS232
;Called fom main loop when SIO_S1_CAN_DATA_PENDING_TX is set
;or GlbSIOStatus2,SIO_S2_RS232_DATA_PENDING_TX is set
;
;Data is in the CnTxRecord structure
;If we are in Slave mode then we need to become temporary master
;Do this by generating a BREAK condition followed by a BREAK type
;pc will echo the type and then send an ACK to let us know that it
;is now the comms slave
;If we receive a bad echo or no ACK then we abort from this function
;leaving the flag set so that this function will be called again
;at a later time. This is because a bad echo indicates that the
;pc had just begun to send us a new record which we need to process
;first
;If we are not in a binary mode then bcf the SIO_S1_CAN_DATA_PENDING_TX bit
;-------------------------------------------------------------------------

GSIR2RecordDataPendingTxToRS232
	btfss	GlbSIOMode,SIO_M_BINARY
	bra	_gcdpttr_no_rs232
	;check to see there is no rs232 activity. Any Chars pending?
	movf	R2RxRecordCursor,W	;See if we are processing a record
	bnz	__gcdpt_exit	
	;ok see if there is another char pending for any reason
	btfsc	GlbSIOStatus1,SIO_S1_CHAR_AVAILABLE	
	bra	__gcdpt_exit 	
	;finally check to see if the rs232 input queue is empty
	movf	R2RxNumInBuff,W
	bnz	__gcdpt_exit
;ok doesn't appear to be any rs232 activity so send a BREAK
;which tells pc we have something to say. Also used for mc resets
	call	GSISendBreak
	movlw 	R2_BREAK_SEND_RECORD		;tell pc we want to send a record
	call	GSISendByteWithEcho232
;timeout: C,Z
;fail (char found but not correct): C,NZ
;success NC	
	btfsc	STATUS,C
	bra	__gcdpt_exit	;try again later
;--------------------------------------------------
;PC will respond with an ACK once it has processed
;PC will then become master and we must become slave
;If we wish to send more than one record can use the
;R2_BREAK_SEND_MULTIPLE_RECORDS
;must terminate MULTIPLE with an RS232_MSSG REQ_SLAVE
;--------------------------------------------------
	movlw	RT_GSI_ACK		;look for ACK from pc
	movwf	GlbLocalTempL
	movlw	0					;infinite timeout, will wdt if error
	movwf	GlbLocalTempH
_gcdpttr_look_for_ack:
	call	GSILookForR2RxChar	;using timeout
	bc		_gcdpttr_look_for_ack	
;ACK found, will WDT if a problem
	
	bsf	GlbSIOStatus1,SIO_S1_MASTER	;we are MASTER
	
	;ok, ACK was received (has been removed)
	;pc is Slave till we send RS232 Messg data[0] REQ_SLAVE
;-------------------------------------------------------------
;Send an RS232Mssg data[0]=subFunction ReqSlave
;pc responds with ACK if successful
;we then SetMaster and Tx
;-------------------------------------------------------------
	;movlw	RT_GSI_RS232_MSSG
	;movwf	R2TxRecordType	;a generic rs232 message
	;movlw	GSI_RS232_MESSAGE_REQ_MASTER
	;movwf	R2TxRecordData	;subfunction REQ_SLAVE (this node wants to be slave)		
	;movlw	1
	;movwf	R2TxRecordLength	;record data is 1 byte only

__gcdpt_exit
	return
_gcdpttr_no_rs232:
	;bcf	GlbSIOStatus1,SIO_S1_RECORD_DATA_PENDING_TX
	bcf	GlbSIOStatus2,SIO_S2_RS232_DATA_PENDING_TX		;also used by r232 record pending code
	return
;Need code to detect if pc is present. If so then use this function
;if not then never attempt to send the CAN data to the pc.


#IF 0
;*******************************************************
; GSIRS232RecordAwaitingTx
;********************************************************
GSIRS232RecordAwaitingTx
	btfss	GlbSIOMode,SIO_M_BINARY
	bra	_rat_no_rs232
	;check to see there is no rs232 activity. Any Chars pending
	;Probably need to do some form of timeout on this NULL_SENT. If the
	;master doesn't respond after n seconds, didn't we ought to send
	;another one?
	btfsc	GlbSIOStatus1,SIO_S1_NULL_SENT
	bra	__rat_exit			
	movf	R2RxRecordCursor,W	;See if we are processing a record
	bnz	__rat_exit	
	;ok see if there is another char pending for any reason
	btfsc	GlbSIOStatus1,SIO_S1_CHAR_AVAILABLE	
	bra	__rat_exit 	
	;finally check to see if the rs232 input queue is empty
	movf	R2RxNumInBuff,W
	bnz	__ratt_exit
;ok doesn't appear to be any rs232 activity so send a null
;NULL tells pc we want to be master
	clrf	WREG
	call	GSISendByte232
;Should wait for an echo for handshaking purposes but defer processing for main loop
;in case the pc is not ready to relinquish control	
	bsf	GlbSIOStatus1,SIO_S1_NULL_SENT		
__rat_exit
	#IF ENABLE_NULL_RECEIVED 
;NULL has been sent. Now should loop till we see SIO_S2_NULL_RECEIVED. WDT if fail
	btfss	GlbSIOStatus2,SIO_S2_NULL_RECEIVED
	bra		__gcdpt_exit			;wdt if we don't receive the byte
	bcf		GlbSIOStatus2,SIO_S2_NULL_RECEIVED	;clear the flag
	#ENDIF ;ENABLE_NULL_RECEIVED
	return
_rat_no_rs232:
	bcf	GlbSIOStatus2,SIO_S2_RS232_DATA_PENDING_TX
	return
#ENDIF ;IF 0



;------------------------------------------------------------
;		GSIAddToR2TxAsyncCQ
;
;An asynchronous record needs to be sent to the pc down the rs232
;Received CAN mssg's are already placed in their own CQ.
;This Q is for messages that this node wishes to send. In particular
;it will be sent when a node changes id from NoNetwork to Master
;Also will be sent when a master node registers a new node on the CAN
;it will send the new nodes id to the pc.
;Must use a CQ as the nature of async events dictates that we don't
;know when they will arrive. Might get a number very quickly
;Change the Q's numInBuff variable last so that an int will not
;If record will not fit (ovf) then must remove first record in Q
;and re-perform the test and removal if still insufficient room 
;Should also flag the ovf, preferably by sending an error record
;to the pc
;Receives:
;FSR0 : address of CQ structure
;R2TxCQStructure	RES	0
;R2TxNumInBuff 		RES 1	;GSXXX 13/4/07 changed order of numInBuff and BuffEnd 
;R2TxBuffEnd 		RES 1
;R2TxWCursor 		RES 1
;R2TxRCursor 		RES 1
;R2TxBuff 			RES TX_BUFF_SIZE				

;FSR1: address of R2RecordStructure
;-------------------------------------------------------------

GSIAddToR2TxAsyncCQ
	DW	0xffff 
#if 0
	btfss	GlbSIOMode,SIO_M_CAN_RX_TO_SERIAL
	bra	atrcq_no_error_exit

	btfss	GlbSIOMode,SIO_M_BINARY			;not binary, then can't tx so ignore
	bra atrcq_no_error_exit								;Z exit, no error
#IF 0
;Ignore ovf. Overwrite the message
	movf	_CnDataLength,W
	addwf	CnCQRxR2NumInBuff,W		;add to num bytes currently in buffer
	sublw	(CQ_CAN_RX_R2_BUFFER_SIZE-CQ_CAN_MSSG_FIXED_MSSG_SIZE)
	bnc	atrcqOvf
;ok mssg will fit in buffer.
#ENDIF

;indirectly address the id
        lfsr	FSR0, _CnvReg1_O
;now add the rest of the id. id and length bytes are contiguous in memory so loop 
;counter can include it.
	movlw	CAN_MSSG_MAX_SIZEOF_ID+CAN_MSSG_SIZEOF_LENGTH 
	movwf	_CnAddToCQCounter		;counter
;Set up buffer insertion point
	movf	CnCQRxR2WCursor,W			;Ptr to buff insertion point
	movwf	FSR1L,A				;Indirect access insertion point
	movlw	HIGH CnCQRxR2Buff	
    movwf   FSR1H
	;
gcatrc1:
;FSR1 points to correct insertion point, FSR0 points to id:length bytes
	movff	POSTINC0,POSTINC1		
;Buffer management
	movlw	CnCQRxR2BuffEndLbl		;address of end of buff. Keep in W for quicker access
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxR2Buff			;start pos of buffer	
	decfsz	_CnAddToCQCounter
	bra	gcatrc1
;id and length safely in buffer, so set up for data transfer
;move it straight out of the pic's buffer
	movf	_CnDataLength,W
	bz	gcatrc_add_flag
	movwf	_CnAddToCQCounter	;keep the length for the counter
	lfsr	FSR0,RXB0D0		;WINCON will select RXB1 if reqd (set by caller)
gcatrc2:
;FSR1 points to correct insertion point, FSR0 points to id:length bytes
	movff	POSTINC0,POSTINC1		
;Buffer management
	movlw	CnCQRxR2BuffEndLbl		;address of end of buff.
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxR2Buff			;start pos of buffer	

	decfsz	_CnAddToCQCounter
	bra	gcatrc2
gcatrc_add_flag:
	movff	_CnRxFlags,POSTINC1
	movlw	CnCQRxR2BuffEndLbl		;address of end of buff. Keep in W for quicker access
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxR2Buff			;start pos of buffer	
;mssg+flag now in CQ
	;update the WCursor
	movff	FSR1L,CnCQRxR2WCursor
	;now adjust num in buffer
	movlw	CQ_CAN_MSSG_FIXED_MSSG_SIZE	;includes flag	
	addwf	_CnDataLength,W
	addwf	CnCQRxR2NumInBuff	
	bsf		GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX ;set for individ
atrcq_no_error_exit
	bsf	STATUS,Z			;no error
	return
atrcqOvf
	bcf	STATUS,Z			;send error
#ENDIF
	return
	DW	0xffff





#IF 0
GSCANAddToR2CQ
	DW	0xffff 
	btfss	GlbSIOMode,SIO_M_CAN_RX_TO_SERIAL
	bra	atrcq_no_error_exit
#if	ENABLE_CAN_FILTER_0_TO_RS232 == FALSE
	movf	_CnRxFlags,W	;see which filter accepted this message
	andlw	CAN_RX_FILTER_BITS	;mask out non-filter bits		
	bz	atrcq_no_error_exit			;filter 0 message so ignore. Allows us to debug without SYNC id's always appearing
#endif

	btfss	GlbSIOMode,SIO_M_BINARY			;not binary, then can't tx so ignore
	bra atrcq_no_error_exit								;Z exit, no error
#IF 0
;Ignore ovf. Overwrite the message
	movf	_CnDataLength,W
	addwf	CnCQRxR2NumInBuff,W		;add to num bytes currently in buffer
	sublw	(CQ_CAN_RX_R2_BUFFER_SIZE-CQ_CAN_MSSG_FIXED_MSSG_SIZE)
	bnc	atrcqOvf
;ok mssg will fit in buffer.
#ENDIF

;indirectly address the id
        lfsr	FSR0, _CnvReg1_O
;now add the rest of the id. id and length bytes are contiguous in memory so loop 
;counter can include it.
	movlw	CAN_MSSG_MAX_SIZEOF_ID+CAN_MSSG_SIZEOF_LENGTH 
	movwf	_CnAddToCQCounter		;counter
;Set up buffer insertion point
	movf	CnCQRxR2WCursor,W			;Ptr to buff insertion point
	movwf	FSR1L,A				;Indirect access insertion point
	movlw	HIGH CnCQRxR2Buff	
    movwf   FSR1H
	;
gcatrc1:
;FSR1 points to correct insertion point, FSR0 points to id:length bytes
	movff	POSTINC0,POSTINC1		
;Buffer management
	movlw	CnCQRxR2BuffEndLbl		;address of end of buff. Keep in W for quicker access
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxR2Buff			;start pos of buffer	
	decfsz	_CnAddToCQCounter
	bra	gcatrc1
;id and length safely in buffer, so set up for data transfer
;move it straight out of the pic's buffer
	movf	_CnDataLength,W
	bz	gcatrc_add_flag
	movwf	_CnAddToCQCounter	;keep the length for the counter
	lfsr	FSR0,RXB0D0		;WINCON will select RXB1 if reqd (set by caller)
gcatrc2:
;FSR1 points to correct insertion point, FSR0 points to id:length bytes
	movff	POSTINC0,POSTINC1		
;Buffer management
	movlw	CnCQRxR2BuffEndLbl		;address of end of buff.
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxR2Buff			;start pos of buffer	

	decfsz	_CnAddToCQCounter
	bra	gcatrc2
gcatrc_add_flag:
	movff	_CnRxFlags,POSTINC1
	movlw	CnCQRxR2BuffEndLbl		;address of end of buff. Keep in W for quicker access
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxR2Buff			;start pos of buffer	
;mssg+flag now in CQ
	;update the WCursor
	movff	FSR1L,CnCQRxR2WCursor
	;now adjust num in buffer
	movlw	CQ_CAN_MSSG_FIXED_MSSG_SIZE	;includes flag	
	addwf	_CnDataLength,W
	addwf	CnCQRxR2NumInBuff	
	bsf		GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX ;set for individ
atrcq_no_error_exit
	bsf	STATUS,Z			;no error
	return
atrcqOvf
	bcf	STATUS,Z			;send error
	return
	DW	0xffff
dsfd
#ENDIF ;#IF 0

	END
