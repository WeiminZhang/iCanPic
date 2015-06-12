;******************************************************************************
;* @file    can.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;Copyright Glenn Self 2004
;******************************************************************************

#define IN_CAN.ASM

;#include <p18f458.inc>


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



#include "CANPrTx.inc"


	GLOBAL CANDebugCode
	GLOBAL	CnErrorState,CnErrorState1,CnBank
;for CANRx.asm
	GLOBAL	CnLocalTemp
;for CANPrTx.asm	
#if DISABLE_CN_SYNC_RX
	GLOBAL CnFilter0Mask	
#endif	
	
;ForMastSlav.asm
	GLOBAL	CopyR2RecordToCANTxBuff	
	GLOBAL	ProcessCQRxCANMessage
;from CANRx.asm
	EXTERN	ProcessCANMssg

	EXTERN	GlbCANState,GlbCANState1,GlbDebugTempL,GlbDebugTempH
	
	EXTERN R2RxByte
	EXTERN TestI2C
;from MastSlav.asm
;from Cantx.asm
	IF DEBUG
		EXTERN FillCANTxRecord,R2TxRecordData
	ENDIF
;from CANPrTx.asm
	#IF SIMULATOR
		EXTERN	GSCANReadMsgToCQ
	#ENDIF
;--------------------------------------
;	from rs232.asm
	EXTERN	R2RxRecordType,R2RxRecordNode,R2RxRecordLength,R2RxRecordData
	EXTERN	R2TxRecordType,R2TxRecordNode,R2TxRecordLength

;------------------------------------------
;	from gsi.asm
	EXTERN GSIProcessRxByteRS232Entry
;------------------------------------------
;------------------------------------------
;	from init.asm
	#if  EEPROM_RESTORE_STATE_ENABLED

	EXTERN	EpCANSlaveId,EpCANMasterId
	#endif
;-----------------------------------------
;	from gsi.asm
	EXTERN CnThisNode
;------------------------------------------
;-----------------------------------------
;	from usercan.asm
	EXTERN UserCANProcessCANMssg
;------------------------------------------
	;from lcd.asm
#IF GSI_LCD_SUPPORT	
	EXTERN LdLCDRow
#ENDIF	
;-----------------------------------------
;Variables shared with other CAN modules
	GLOBAL	CnErrorState,CnErrorState1,CnRxMssgId,CnRxBuff,CnRxBuffLength
	GLOBAL	CnRxFlags,CnTxRecord,CnTxMssgId,CnTxBuff,CnTxBuffLength
	GLOBAL	CnTxFlags,CnMode,CnTemp,CnTemp32,CnSyncCounter
	GLOBAL	CnSlaveBase
	GLOBAL CnCQRxNumInBuff,CnCQRxBuff,CnCQRxWCursor ;,CnCQRxBuffEnd	
	GLOBAL	CnRxMssg,CnCQRxBuffEndLbl
	GLOBAL CnCQRxCounter,CnCQRxNIB,CnCQRxRCursor
	
	GLOBAL	CnCQRxR2WCursor,CnCQRxR2NumInBuff,CnCQRxR2Buff
	GLOBAL	CnCQRxR2BuffEndLbl,CnCQRxR2RCursor
	

GSI_CONFIG_DATA		CODE		;EEPROM_DATA

CAN_DATA	UDATA
;CAN_GLOBAL_DATA	UDATA_ACS

CnBank		RES	0		;label used for bank selection
CnMode		RES	01		;Current mode master,slave etc	

CnErrorState	RES	1
CnErrorState1	RES	1		;two error state flags

;CQ for CAN messages. Filled in the ISR
CnCQRxRCursor	RES	1
CnCQRxWCursor	RES	1
CnCQRxNumInBuff	RES	1
CnCQRxBuff		RES	CQ_CAN_RX_BUFFER_SIZE 
CnCQRxBuffEndLbl RES 	0
;CnCQRxBuffEnd	RES	0
CnCQRxCounter	RES	1		;temp counter
CnCQRxNIB	RES	1		;temp NumInBuff

;Above messages are also add to this CQ for Tx down the rs232 line if available
;this Q overwrites if it ovfs'
CnCQRxR2RCursor	RES	1
CnCQRxR2WCursor	RES	1
CnCQRxR2NumInBuff	RES	1
CnCQRxR2Buff		RES	CQ_CAN_RX_BUFFER_SIZE 
CnCQRxR2BuffEndLbl RES 	0
;CnCQRxR2BuffEnd	RES	1
;CnCQRxCounter	RES	1		;temp counter uses CnCQRxCounter, can't receive a 2nd message while processing
;CnCQRxNIB	RES	1		;temp NumInBuff. As above

CnFilter0Mask	RES 1	;ignore the messages defined in can.inc

CnRxMssg		RES	0
CnRxMssgId		RES	4			;the RX mess id
CnRxBuff        RES     08		;The data bytes
CnRxBuffLength  RES     01		;The data length
CnRxFlags       RES     01		;The flags


;These must be this order and must be contiguous as we use FSR to move
CnTxRecord	RES	0
CnTxMssgId	RES	04
CnTxBuff	RES	08
CnTxBuffLength	RES	01
CnTxFlags	RES	01


CnTemp		RES	01
CnLocalTemp	RES	01		;mustn't call outside current procedure
CnTemp32	RES	04		;
CnSyncCounter	RES	01		;for debugging

CnSlaveBase	RES	2		;4096 offsets allow 65535 slaves
;CnThisNode	RES	1		;our node (0=master) Variable now in gsi.asm
;NOTE CnThisNode contains the id for the code running on this node
;it is distinct from the CAN_NODE_THIS constant which is a special
;value indicating that data is directed to the this node. It is
;used by the caller when it doesn't know the node's id that it is
;calling. Usually it will be sent over the rs232 and can be used
;to find the actual id of the node to which the rs232 line is attached.
CAN_CODE           CODE

;----------------------------------------------------------------
;		GSIInitCAN
;----------------------------------------------------------------
GSIInitCAN

;Define TRIS bits to define CANRx pin as i/p and CANTx pin as o/p
	banksel	CnBank


	movlw	CAN_SYNC_POSTSCALER
	movwf	CnSyncCounter	;only send 1 in SYNC_COUNTER SYNCS

	clrf	GlbCANState	;store status info
	clrf	GlbCANState1	;more status info
	clrf	CnErrorState	;error state info
	clrf	CnErrorState1	;more error state info
	clrf	CnMode		;current CAN mode (software)
#if DISABLE_CN_SYNC_RX
	clrf	CnFilter0Mask	;control filter 0 message, a debugging aid
#endif
	;clrf	CnThisNode	;0=master
	movlw	CAN_NODE_NO_NETWORK
	movwf	CnThisNode
	clrf	CnSlaveBase	;a stand alone master uses this to
	clrf	CnSlaveBase+1	;generate SLAVE base addresses
	clrf	CnCQRxNumInBuff	;receive buff counter
	movlw	CnCQRxBuff
	movwf	CnCQRxWCursor
	movwf	CnCQRxRCursor

	;movlw	CnCQRxBuffEnd
	
	clrf	CnCQRxNumInBuff

	clrf	CnCQRxR2NumInBuff	;receive buff counter
	movlw	CnCQRxR2Buff
	movwf	CnCQRxR2WCursor
	movwf	CnCQRxR2RCursor
	;clrf	CnCQRxR2NumInBuff
	messg	Disabled CAN module still requires some CAN code, must fix!
#IF STARTLE		;Startle box code must see CAN enabled but mustn't
				;run it!

	return
#ELSE

	clrf	COMSTAT		;clr any pending ints
	clrf	PIR3		;clr any pending ints
	clrf	PIR2		;do all the peripheral ints
	;clrf	IPR3		;(clrf in Interrupts.asm) All CAN ints low priority
	bcf	PIE2,TMR3IE	;no TMR3 ints, used later for SYNC

	movlw	0xff		;ignore EWARN as TX/RXWARN are checked explicitly
	movwf	PIE3		;mask on the required ints
	;Next bit is used so that we drive the mcp2551 correctly. If not
	;set then the pin is tristated when recessive and presumeably needs a pullup R
	bsf	CIOCON,ENDRHI	;Drive CANTX pin when recessive bit
	bsf	PORTB,2		;Need set for when in CONFIG_MODE
				;otherwise sends a LOW and generates
				;a CAN_INVALID_MSSG
	banksel	RXB0CON
	bsf	RXB0CON,RXB0DBEN	;enable ovf Rx0->Rx1
;Note:
;with RXBODEN set. If an ovf from B0 occurs, the message will instead be
;transferred to B1. An RXB1 interrupt will result (confirmed:gs). RXBnCON will accurately 
;reflect the filter that was used to accept the message.
;Thus B1 can hold B0 or B1 filtered messages. B0 can only hold B0 filtered
;messages
	;sjw,brp,phseg1,phseg2,propseg2,CAN_CONFIG_VALID_XTD_MSG
	
        ;mCANPrTxInit   1, 5, 7, 6, 2,CAN_CONFIG_ALL_VALID_MSG
;Probably need to modify this. For 750k (769.2k)Baud use 1,0,6,5,1 
;for 500k Use: 1,1,5,3,1(exactly 500k)
;These defines are taken from those calculted for the ARM7 ET
;clock on ARM7 is 19.661MHz, so should be ok here at 20MHz
;#define   GS_CANBitrate20k_19MHz        0x001c003b //prescaler=60;sjw=1;tseg1=13;tseg2=2
;#define   GS_CANBitrate500k_19MHz       0x00240003 //prescaler=4;sjw=1;tseg1=5;tseg2=3
;#define   GS_CANBitrate1000k_19MHz      0x00240001 //prescaler=2;sjw=1;tseg1=5;tseg2=3
	;500kHz 1,3,5,3
	;1MHz   1,1,5,3
	mCANPrTxInit   1, 4, 7, 6, 2,CAN_CONFIG_VALID_XTD_MSG
	;PrTxInit will modify BSR
	;banksel	CnMode
;Set Loop-back mode for testing in Stand alone mode
#if CAN_LOOPBACK
	mCANPrTxSetOpMode     CAN_OP_MODE_LOOP        ;Loop back mode
#else
	mCANPrTxSetOpMode     CAN_OP_MODE_NORMAL
#endif ;CAN_LOOPBACK

	MAIN_BANK
	#IF EEPROM_RESTORE_STATE_ENABLED
	call	CANRestoreStateFromEEPROM
	#ENDIF
#ENDIF
	return
;----------------------------------------------------------------

GSITestCAN
	;call	GSICANPCMaster		
	
	return

;------------------------------------------------------------------
;	CopyR2RecordToCANTxBuff
;Copy the rs232 record buffer to the CnTxBuff, ready for CAN Tx
;-----------------------------------------------------------------
CopyR2RecordToCANTxBuff
;R2RxRecordType		RES 1	;A binary Mssg
;R2RxRecordNode		RES 1	;the node this data is for
;R2RxRecordLength		RES 1   ;the number of bytes in record
;R2RxRecord		RES 8	;the data 0-8 bytes
;R2RxRecordCursor	RES 1	;Where we are in assembly of record

	movff	R2RxRecordType+1,CnTxBuff	;The node
	movff	R2RxRecordType+3,CnTxBuff+1	;data 0
	movff	R2RxRecordType+4,CnTxBuff+2	;data 1
	movff	R2RxRecordType+5,CnTxBuff+3	;data 2
	movff	R2RxRecordType+6,CnTxBuff+4	;data 3
	movff	R2RxRecordType+7,CnTxBuff+5	;data 4
	movff	R2RxRecordType+8,CnTxBuff+6	;data 5
	movff	R2RxRecordType+9,CnTxBuff+7	;data 6
	movff	R2RxRecordType+8,CnTxBuff+6	;data 7

	return



;------------------------------------------------------------------
;	GSICnCQToR2Record
;Copy the next mssg in Q to CnRxBuff to rs232 record buffer ready for RS232 Tx
;Requires:
;FSR0 address of RxBuffer (currently only R2TxRecord)
;Assumes:
;BSR Nothing
;Modifies:
;WREG
;R2TxRecordxx structure pointed to by FSR0 filled
;copy CnRxBuff to buffer (according to CnRxBuffLength)
;Thus R2TxRecordLength is minimum of 5 bytes and max of 13 bytes
;This can be a 13 byte data buffer sent up the RS232
;The CAN mssg is in the CQ
;-----------------------------------------------------------------
GSICopyCANRxBuffToR2TxRecord
	;movff	CnTxBuff,R2RecordType
	;movff	CnTxBuff,R2RxRecordType+1	;The node
gccrbtrtr
;CnRxMssgId	RES	4		;the RX mess id
;CnRxBuff        RES     08		;The data bytes
;CnRxBuffLength  RES     01		;The data length
;CnRxFlags       RES     01

;R2TxRecordType		RES 1	;A binary Mssg
;R2TxRecordNode		RES 1	;the node this data is for
;R2TxRecordLength		RES 1   ;the number of bytes in record
;R2TxRecord		RES 12	;the data 0-9 bytes (also req CAN_ID)
;R2TxRecordFlags		RES 1	;any Cn flag to add (when sending a Cn mssg to rs232)

;---------------------------------------------------
	lfsr	FSR0,CnRxMssg		;point to buffer
	;Assumes a valid Cn mssg is in CnRxMssg
	;gsi.asm calls ProcessCQRxCANMessage which does this transfer
	;The functions called by the ProcessCQ... jmp table set
	;the SIO_S1_CAN_DATA_PENDING_TX bit if they wish the Cn mssg to
	;be Tx'd to the pc
	 
	;call	GSICANGetMsgFromCQ	;do the transfer
	;bc	_ccrx_error
	;bz	_ccrx_error
;FSR0=pointer to memory to store the CAN mssg
;circular buffer for CAN msg's
;returns:
;STATUS NC NZ no error data in buffer
;STATUS NC Z no data in buffer
;STATUS C on error (W=error code)
;Modifies BSR->CnBank
;-----------------------------------------------------	
	movlw	RT_GSI_R2_RX_CAN_MSSG_NP	;id of RecordType
	movff	WREG,R2TxRecordType	;fill field

	movff	CnRxMssgId,R2TxRecordData		;mod 6/2/06 was R2TxRecord
	movff	CnRxMssgId+1,R2TxRecordData+1
	movff	CnRxMssgId+2,R2TxRecordData+2
	movff	CnRxMssgId+3,R2TxRecordData+3
;-- Set up to fill the buffer 0-8 bytes
	lfsr	FSR0,CnRxBuff
	lfsr	FSR1,R2TxRecordLength
	movff	CnRxBuffLength,R2TxRecordLength	;length
	;now adjust length for the CanMsgId and CanMsgFlag
	movff	INDF1,WREG	;get record len to WREG
	addlw	5		;adjust for id(4 bytes)+flags(1byte)
	movwf	POSTINC1	;inc the pointer to point to Record
	addlw	-5
	;WREG holds length of CAN data, R2TxRecordLength =Len CAN Data +5 (id+flags)
	;movf	WREG,W			;access WREG to set flags (movff doesn't affect flags)
	bz	ccrb3			;no CAN data?
	lfsr	FSR1,R2TxRecordData+4	;step over Id
ccrb2:
	movff	POSTINC0,POSTINC1	;copy CnData->R2Record+4
	decfsz	WREG
	bra	ccrb2
ccrb3:
	;movff	CnRxMssgId,POSTINC1	;the LSB id of message
	movff	CnRxFlags,INDF1		;flags ->R2 append to data
	;-let top level know we have data pending
	;ignore if we are not attached to a pc
	;btfsc	GlbSIOMode,SIO_M_BINARY
	;bsf GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX		;Will be set in ISR
	IF DEBUG
		;movlw	'P'
		;call	GSILCDWriteChar
	ENDIF
	bcf	STATUS,C
	movlw	0
	return
_ccrx_error	
	bsf	STATUS,C
	movlw	1
	return






#if 0
;------------------------------------------------------------------
;	GSICopyCANRxBuffToR2TxRecord
;Copy the CnRxBuff to rs232 record buffer ready for RS232 Tx
;Requires:
;Assumes:
;BSR Nothing
;Modifies:
;WREG
;R2TxRecordxx variable filled
;Copy CnRxMssgId to buffer
;copy CnRxBuff to buffer (according to CnRxBuffLength)
;copy CnRxFlags to buffer.
;Thus R2TxRecordLength is minimum of 5 bytes and max of 13 bytes
;This can be a 13 byte data buffer sent up the RS232
;The CAN mssg is in the CQ
;-----------------------------------------------------------------
GSICopyCANRxBuffToR2TxRecord
	;movff	CnTxBuff,R2RecordType
	;movff	CnTxBuff,R2RxRecordType+1	;The node
gccrbtrtr
;CnRxMssgId	RES	4		;the RX mess id
;CnRxBuff        RES     08		;The data bytes
;CnRxBuffLength  RES     01		;The data length
;CnRxFlags       RES     01

;R2TxRecordType		RES 1	;A binary Mssg
;R2TxRecordNode		RES 1	;the node this data is for
;R2TxRecordLength		RES 1   ;the number of bytes in record
;R2TxRecord		RES 12	;the data 0-9 bytes (also req CAN_ID)
;R2TxRecordFlags		RES 1	;any Cn flag to add (when sending a Cn mssg to rs232)

;---------------------------------------------------
	lfsr	FSR0,CnRxMssg		;point to buffer
	;Assumes a valid Cn mssg is in CnRxMssg
	;gsi.asm calls ProcessCQRxCANMessage which does this transfer
	;The functions called by the ProcessCQ... jmp table set
	;the SIO_S1_CAN_DATA_PENDING_TX bit if they wish the Cn mssg to
	;be Tx'd to the pc
	 
	;call	GSICANGetMsgFromCQ	;do the transfer
	;bc	_ccrx_error
	;bz	_ccrx_error
;FSR0=pointer to memory to store the CAN mssg
;circular buffer for CAN msg's
;returns:
;STATUS NC NZ no error data in buffer
;STATUS NC Z no data in buffer
;STATUS C on error (W=error code)
;Modifies BSR->CnBank
;-----------------------------------------------------	
	movlw	RT_GSI_R2_RX_CAN_MSSG_NP	;id of RecordType
	movff	WREG,R2TxRecordType	;fill field

	movff	CnRxMssgId,R2TxRecordData		;mod 6/2/06 was R2TxRecord
	movff	CnRxMssgId+1,R2TxRecordData+1
	movff	CnRxMssgId+2,R2TxRecordData+2
	movff	CnRxMssgId+3,R2TxRecordData+3
;-- Set up to fill the buffer 0-8 bytes
	lfsr	FSR0,CnRxBuff
	lfsr	FSR1,R2TxRecordLength
	movff	CnRxBuffLength,R2TxRecordLength	;length
	;now adjust length for the CanMsgId and CanMsgFlag
	movff	INDF1,WREG	;get record len to WREG
	addlw	5		;adjust for id(4 bytes)+flags(1byte)
	movwf	POSTINC1	;inc the pointer to point to Record
	addlw	-5
	;WREG holds length of CAN data, R2TxRecordLength =Len CAN Data +5 (id+flags)
	;movf	WREG,W			;access WREG to set flags (movff doesn't affect flags)
	bz	ccrb3			;no CAN data?
	lfsr	FSR1,R2TxRecordData+4	;step over Id
ccrb2:
	movff	POSTINC0,POSTINC1	;copy CnData->R2Record+4
	decfsz	WREG
	bra	ccrb2
ccrb3:
	;movff	CnRxMssgId,POSTINC1	;the LSB id of message
	movff	CnRxFlags,INDF1		;flags ->R2 append to data
	;-let top level know we have data pending
	;ignore if we are not attached to a pc
	btfsc	GlbSIOMode,SIO_M_BINARY
	bsf GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX
	IF DEBUG
		;movlw	'P'
		;call	GSILCDWriteChar
	ENDIF
	bcf	STATUS,C
	movlw	0
	return
_ccrx_error	
	bsf	STATUS,C
	;retlw	1
	movlw	1
	return	
	
#endif	
;---------------------------------------------------------------
;GSICopyCANTxBuffToCANRxBuffer
;A debugging function
;DEBUG EQU TRUE in debug.inc
;---------------------------------------------------------------
	#IF DEBUG
GSICopyCANTxBuffToCANRxBuffer
	;movff	CnTxBuff,R2RecordType
	;movff	CnTxBuff,R2RxRecordType+1	;The node
cctbtcrb
;CnRxMssgId	RES	4		;the RX mess id
;CnRxBuff        RES     08		;The data bytes
;CnRxBuffLength  RES     01		;The data length
;CnRxFlags       RES     01

;R2TxRecordType		RES 1	;A binary Mssg
;R2TxRecordNode		RES 1	;the node this data is for
;R2TxRecordLength		RES 1   ;the number of bytes in record
;R2TxRecord		RES 12	;the data 0-9 bytes (also req CAN_ID)
;R2TxRecordFlags		RES 1	;any Cn flag to add (when sending a Cn mssg to rs232)

	lfsr	FSR0,CnTxMssgId
	lfsr	FSR1,CnRxMssgId
	movlw	CAN_XFRAME_LENGTH
	call	MemCopy
	return
MemCopy:
;FSR0=Src, FSR1=dest, WREG=length
	
	movf	WREG		;do a zero length test
	bz	_mc_exit
_mc_1
	movff	POSTINC0,POSTINC1
	decfsz	WREG
	bra	_mc_1
_mc_exit
	return

	#ENDIF

;-----------------------------------
;
;
;* Macro:               mCANPrTxReadMsg  msgIDPtr,
;*                                      DataPtr,
;*                                      DataLngth,
;*                                      CAN_RX_MSG_FLAGS type
;*
;* PreCondition:        None
;*
;* Input:               msgIDPtr - Starting address of 32-bit Message ID
;*                                      storage
;*                      DataPtr - Starting address for recd. data storage
;*                      DataLngth - Location to store Data Length info
;*                      Flags- Location to store flags info
;*
;* Output:              If any pending received data is available then data
;*                      and ID values is returned

;CAN_RX_FILTER_BITS:     = B'00000111'   ;Use this to access filter bits
;CAN_RX_FILTER_0:        = B'00000000'
;CAN_RX_FILTER_1:        = B'00000001'
;CAN_RX_FILTER_1_BIT_NO: = 0x00
;CAN_RX_FILTER_2:        = B'00000010'
;CAN_RX_FILTER_3:        = B'00000011'
;CAN_RX_FILTER_4:        = B'00000100'
;CAN_RX_FILTER_5:        = B'00000101'

;CAN_RX_OVERFLOW:        = B'00001000'   ;Set if Overflowed else cleared
;CAN_RX_OVERFLOW_BIT_NO: = 0x03

;CAN_RX_INVALID_MSG:     = B'00010000'   ;Set if invalid else cleared
;CAN_RX_INVALID_MSG_BIT_NO: = 0x04

;CAN_RX_XTD_FRAME:       = B'00100000'   ;Set if XTD message else cleared
;CAN_RX_XTD_FRAME_BIT_NO: = 0x05

;CAN_RX_RTR_FRAME:       = B'01000000'   ;Set if RTR message else cleared
;CAN_RX_RTR_FRAME_BIT_NO: = 0x06         ;RTR message bit

;CAN_RX_DBL_BUFFERED: = B'10000000'      ;Set if this message was hardware
;CAN_RX_DBL_BUFFERED_BIT_NO: = 0x07      ;double-buffered
;

;------------------------------------------------------------------
;		ProcessCQRxCANMessage
;CANISR will add messages to CnCQRxBuff
;Main code will call this routine if it finds the CQ has data in it
;Modifies:
;BSR
;Any CAN mssgs are in the RxCQ

;Now use CQ to capture Rx'd messages. All mssgs are stored in the 
;CQ, there is no CQ for individual filters or Rx buffers, though could
;consider a separate buffer for high priority mssgs which could then
;be serviced before lower priority
;Should copy CnRxBuff+0 to R2TxNode, so that pc know which node this
;has come from without needing extra code. Otherwise it needs to
;check rs232 messages to see if they have an embedded CAN frame
;------------------------------------------------------------------

ProcessCQRxCANMessage
	lfsr	FSR0,CnRxMssg
	call	GSICANGetMsgFromCQ
	movff	CnRxBuff,R2TxRecordNode
	messg Need to call user code first. If user processes, then we ignore
	
;BSR:CnBank	
	bz	gscc_no_mssg	;no message received
	bc	gscc_fatal_error
	;Message in CnCQRxBuff
	call	UserCANProcessCANMssg
	addlw	0			;WREG=0, we process, WREG NZ we ignore
	bnz		gscc_no_mssg
	
	goto	ProcessCANMssg
gscc_no_mssg:
	bsf	STATUS,Z		;pretend there there was mo message
	return
gscc_fatal_error:
	;WREG filled by GetMssgFrom function
	
	call	DEBUGCauseWDT	;use call, then can return using icd2
	goto	gscc_fatal_error
	

;------------------------------------------------
;	CANDebugCode
;------------------------------------------------
CANDebugCode
	;test GSICopyCANRxBuffToR2TxRecord
#if SIMULATOR 
	banksel	CnBank
 	#IF  EEPROM_RESTORE_STATE_ENABLED
		call	CANRestoreStateFromEEPROM
	#ENDIF
	movlw	HIGH (CAN_ID_SYNC+0)
	movwf	CnTxMssgId+1
	movlw	LOW (CAN_ID_SYNC+0)
	movwf	CnTxMssgId

	clrf	CnTxMssgId+2
	clrf	CnTxMssgId+3
	;id is set
	movlw	0		;pretend the caller is node 1
	movwf	CnTxBuff
	movlw	0xaa	
	movwf	CnTxBuff+1	
	movlw	'5'
	movwf	CnTxBuff+2		

	movlw	3
	movwf	CnTxBuffLength
	;movlw	0xfe
	;movwf	CnTxFlags


;Message 1, Data 01,02, ID 20
Msg1Agn:

        ;CANSendMessage  CnMssgId,2,CAN_TX_XTD_FRAME
	mCANPrTxSendMsg_IID_IDL_IF CnTxMssgId,CnTxBuff,CnTxBuffLength,CAN_TX_XTD_FRAME
        addlw   0x00            ;Check for return value of 0 in W
        bz      Msg1Agn         ;Buffer Full, Try again
	banksel	CnBank

	call	GSCANReadMsgToCQ
	call	GSCANReadMsgToCQ
	call	GSCANReadMsgToCQ
	call	GSCANReadMsgToCQ

	lfsr	FSR0,CnRxMssg
	call	GSICANGetMsgFromCQ

	call	GSCANReadMsgToCQ

	lfsr	FSR0,CnRxMssg
	call	GSICANGetMsgFromCQ
	lfsr	FSR0,CnRxMssg
	call	GSICANGetMsgFromCQ
	lfsr	FSR0,CnRxMssg
	call	GSICANGetMsgFromCQ
	lfsr	FSR0,CnRxMssg
	call	GSICANGetMsgFromCQ
	lfsr	FSR0,CnRxMssg
	call	GSICANGetMsgFromCQ

	bra	Msg1Agn
;-------------------------------


;-------------------------------
;Message 2, Data 03,04, ID 30

	;movlw	HIGH (CAN_ID_LCD_MSSG+0)
	;movwf	CnTxMssgId+1
	;movlw	LOW (CAN_ID_LCD_MSSG+0)
	;movwf	CnTxMssgId

	;clrf	CnTxMssgId+2
	;clrf	CnTxMssgId+3
	;id is set
	;movlw	0		;pretend the caller is node 0
	;movwf	CnTxBuff
	;movlw	0x2	
	;movwf	CnTxBuff+1	
	;movlw	'5'
	;movwf	CnTxBuff+2		

	;movlw	3
	;movwf	CnTxBuffLength
;R2RxRecordNode		RES 1	;the node this data is for
;R2RxRecordLength	RES 1   ;the number of bytes in record
;R2RxRecord		RES 8	;the data 0-8 bytes
;R2RxRecordFlags	RES 1	;currently unused
;record.RecordType = RT_GSI_LCD_MSSG
;record.RecordNode = NODE_THIS
;record.RecordData(0) = GSI_LCD_MESSAGE_WRITE_CHAR
;record.RecordData(1) = RT_LCD_CLS
;record.RecordData(2) = &H58
;record.RecordLength = 3

	movlw	RT_GSI_LCD_MSSG
	movff	WREG,R2RxRecordNode

	movlw 1
	movff	WREG,R2RxRecordNode

	movlw	3
	movff	WREG,R2RxRecordLength

	movlw	GSI_LCD_MESSAGE_WRITE_CHAR
	movff	WREG,R2RxRecordData			;modified on 6/2/06 was R2RxRecord 
	
	movlw	0xc		;cls
	movff	WREG,R2RxRecordData+1

	movlw	'X'		
	movff	WREG,R2RxRecordData+2

	lfsr	FSR0,R2RxRecordNode		
	call	FillCANTxRecord
	banksel	CnBank
	movlw	UPPER CAN_ID_LCD_MSSG
	iorwf	CnTxMssgId+2,F
	movlw	HIGH CAN_ID_LCD_MSSG
	iorwf	CnTxMssgId+1,F
	movlw	LOW CAN_ID_LCD_MSSG
	iorwf	CnTxMssgId,F


Msg2Agn:
        ;CANSendMessage  0x30,CANDt1,4,CAN_TX_XTD_FRAME
	mCANPrTxSendMsg_IID_IDL_IF CnTxMssgId,CnTxBuff,CnTxBuffLength,CAN_TX_XTD_FRAME

        addlw   0x00            ;Check for return value of 0 in W
        bz      Msg2Agn         ;Buffer Full, Try again
	banksel	CnBank
;-------------------------------



	call	ProcessCANMssg

	;call	GSIR2TxCANMssg
	call	GSICopyCANRxBuffToR2TxRecord
	call	RS232SendPendingCANMssg
#ENDIF	;IF SIMULATOR
	return

 	#IF  EEPROM_RESTORE_STATE_ENABLED

;---------------------------------------------------------------
;	CANRestoreStateFromEEPROM
;---------------------------------------------------------------
CANRestoreStateFromEEPROM
;EpCANSlaveId	DW	CAN_NODE_THIS	;invalid for a slave
;EpCANMasterId	DW	CAN_NODE_THIS	;invalid for  master

	movlw	EpCANMasterId		;address of variable
;any value other than CAN_NODE_THIS indicates a master
;This allows us to have multiple masters which doesn't
;actually work. GSI is only practical with a single master
;keep this code for a possible future addition
	call	GSIReadEEPROM		;Master id is in WREG
	sublw	CAN_NODE_THIS		;sub  w from literal
	bnz	_crsfe_master

	movlw	EpCANSlaveId		;address of variable
	call	GSIReadEEPROM		;Master id is in WREG
	sublw	CAN_NODE_THIS		;sub  w from literal
	bz	_crsfe_exit
	call	GSICANSetSlave
_crsfe_exit:
	return

_crsfe_master:
;Last time this node was powered up it was a master
	call	GSICANSetMaster
	return
	#ENDIF ;EEPROM_RESTORE_STATE_ENABLED

#endif ;GSI_CAN_SUPPORT 

        END





	#if 0  
	messg StatusChangeCAN needs cleaning up
	messg Should use NumInBuffer for instead of GlbCANState?
	;servicing the first. This is a flag, not a counter.
	btfsc	GlbCANState,CAN_RX_0
	bra	rx_0_int
	btfsc	GlbCANState,CAN_RX_1
	bra	rx_1_int
GSIStatusChangeCANExit
	return

rx_0_int:
	bcf	GlbCANState,CAN_RX_0
	btfss	RXB0CON,RXFUL
	bra	GSIStatusChangeCANExit
	Messg BUG Can overwrite CnRxBuff here
	banksel	CnBank

	;mCANPrTxReadMsg  CnRxMssgId, CnRxBuff, CnRxBuffLength, CnRxFlags
	;movf	WREG
	;bz	_gscc_error

	lfsr	FSR0,CnCQRxBuff
	call	GSICANGetMsgFromCQ
	IF DEBUG
r0i_error
	bc	r0i_error
	ENDIF

	;WREG=0 on failure
	;RXB0CON is in access bank (RXB1CON isn't)
	banksel CnBank
	;bcf	PIR3,RXB0IF	;performed in TxReadMsg; remove the lock
	;bsf	PIE3,RXB0IE	;re-enable int				
	goto	ProcessCANMssg

rx_1_int:
;banksel is req as this label is a loop target and the bank will
;be changed to	banksel RXB1CON. Must be sure we are in Cn bank
	banksel	CnBank
	bcf	GlbCANState,CAN_RX_1
	movff	RXB1CON,WREG		;not in access bank
	btfss	WREG,RXFUL
	bra	GSIStatusChangeCANExit

	;mCANPrTxReadMsg  CnRxMssgId, CnRxBuff, CnRxBuffLength, CnRxFlags
	;WREG=0 on failure
	;RXB0CON is in access bank (RXB1CON isn't)
	;movf	WREG,W	
	;bz	_gscc_error

	lfsr	FSR0,CnCQRxBuff
	call	GSICANGetMsgFromCQ
	IF DEBUG
r1i_error
	bc	r1i_error
	ENDIF


	banksel	CnBank		;restore bank
	;banksel RXB1CON	;errata don't use bank 15
	;movff	RXB1CON,WREG
	;btfsc	WREG,RXFUL
	;bra	rx_1_int		;make sure we read it

	;bcf	PIR3,RXB1IF	;performed in TxReadMsg
	;bsf	PIE3,RXB1IE	;re-enable int				
	goto	ProcessCanMssg
_gscc_error:

	IF DEBUG
		banksel LdBank
		movlw	'!'
		call	GSILCDWriteChar
	ENDIF
	banksel CnBank
	;bcf	PIR3,RXB1IF	;remove the lock
	bsf	PIE3,RXB1IE	;re-enable int				
	return
	#ENDIF ;#if 0

