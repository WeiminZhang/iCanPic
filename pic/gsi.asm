;******************************************************************************
;* @file    gsi.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************


;gsi.asm

#define IN_GSI.ASM

        ERRORLEVEL      -302    ;SUPPRESS BANK SELECTION MESSAGES


#include "gsi.inc"

;#IF DEVICE == _18F458
;	LIST   P=PIC18F458
;#include "p18xxx8.inc"
;#ELSE
;#INCLUDE "P18F4680.inc"
;	LIST   P=PIC18F680
;#ENDIF

#include "config.inc"
#include "rs232.inc"
#include "keypad.inc"
#include "i2c.inc"
#include "lcd.inc"
#include "can.inc"


;Following data spaces shouldn't be used when using CAN module
;See errata datasheet
GSI_GLOBAL_DONT_USE_DATA UDATA_ACS 51h
GlbDontUseWithCAN		RES 13			;Can use these when no CAN

GSI_GLOBAL_DATA	UDATA_ACS
GlbGSIState		RES 1

GlbDebugTempH  		RES 1
GlbDebugTempL 		RES 1	
GlbCompOffset 		RES 1	
GlbLocalTempL 		RES 1
GlbLocalTempH 		RES 1	
GlbResources		RES 1	;available resources, user can modify
GlbResources1		RES 1	;available resources, user can modify
GlbClock			RES 2	;simple 13msec clock tick

#if DEBUG 
GlbDebugFlag		RES 1
#endif	

GSI_DATA	UDATA
;Must be stored contiguously in this order and of this size
GiBank			RES	0
GiCANTxRecord
GiCANTxNode		RES	1
GiCANTxBuffLength	RES	1
GiCANTxBuff		RES	8
GiCANTxFlags		RES	1
GiDelayCounter 		RES 1
GiDelayCounterH 	RES 1


;GiErrataDontUse		RES	13	;51 to 5dh don't use

CAN_DATA	UDATA
CnThisNode		RES	1

LCD_DATA	UDATA
LdTempWriteChar		RES 1

;------------------------------------------------
;	exported from this module
;------------------------------------------------
	GLOBAL	GlbResources,GlbResources1,GlbClock;,GSICommonReturn
	GLOBAL GSIProcessRxByte_ctr_code,GSIProcessRxByte_esc_code,GSIProcessRxByte_user_code
	GLOBAL CnThisNode
	GLOBAL GSIProcessRxByteRS232Entry,GSIProcessRxByteCnEntry
	Global GlbDontUseWithCAN	;Used to hold data across resets
	
#if DEBUG
	GLOBAL	GlbDebugFlag,GlbDebugTempL,GlbDebugTempH
	GLOBAL Max518Output
	;EXTERN TestVol
#endif
;-----------------------------------
;	definitions in keypad.asm
;----------------------------------

	EXTERN GlbKeypadStatus

;-------------------------------------------------
;	definition in rs232.asm
;-------------------------------------------------

	EXTERN R2RxByte
	EXTERN GlbSIOMode,GlbSIOMode1
	EXTERN GlbSIOStatus

	;R2Slave
	EXTERN BinaryProcessRxByte
;----------------------------------------------------	
;definitions in R2Master.asm	
;----------------------------------------------------	
	EXTERN R2TxRecordType,R2TxRecordNode,R2TxRecordLength,R2TxRecordData ;,R2TxRecord
;-------------------------------------------------
;	definitions in interrupts.asm
;-------------------------------------------------
	EXTERN ISRHigh,ISRLow
	EXTERN GiIntType
;------------------------------------------------
;	definitions in init.asm
;------------------------------------------------
	EXTERN Restart,GiLastResetType
;-----------------------------------
;	definitions in i2c.asm
;----------------------------------

	EXTERN TestI2C

;---------------------------------
;	definitions in lcd.asm
;----------------------------------
	EXTERN GSINull,GSIBeginningOfLine,GSICursorLeft,GSIUsrAbrt,GSIDeleteCharFwd
	EXTERN GSIEndOfLine,GSICursorRight,GSIR2ReqMaster,GSIBS,GSIHTab
	EXTERN GSICursorDown,GSIEnd,GSICls,GSICR,GSICursorDown,GSIEchoToggle	
	EXTERN GSICursorUp,GSIXOn,GSIBinaryMode,GSIXOff,GSICursorRight5,GSIAsciiMode
	EXTERN GSICursorLeft5,GSICursorLeft10,GSICRKeypadToggleState,GSIKeypadToggleMode,GSICtrZ
	EXTERN GSIVersionString,GSICapabilities,GSIGetBuffer,GSISetBuffer
	EXTERN GSICls,GSIInstruction,GSIGetCurToEol,GSIGetLine,GSISetLine
	EXTERN GSIHome,GSIFlowControlOn,GSIFlowControlOff,GSILCDInit
	EXTERN GSIGetButtonState
	;EXTERN LdTempArray

;-----------------------------------
;	definitions in can.asm
;----------------------------------
	#IF GSI_CAN_SUPPORT
	EXTERN CANDebugCode
	EXTERN ProcessCQRxCANMessage,CnCQRxNumInBuff,CnCQRxR2NumInBuff
	#ENDIF

;-----------------------------------
;	definitions in cantx.asm
	#IF GSI_CAN_SUPPORT
	EXTERN GSISendCANPingMssg

	#ENDIF
;----------------------------------

;-----------------------------
;-----------------------------------
;	definitions in rs232.asm
;----------------------------------
	EXTERN	R2DebugCode
;-----------------------------

;	data
;-----------------------------
	GLOBAL	GlbCompOffset
	GLOBAL	PgmMssgTbl
	GLOBAL	GlbLocalTempL,GlbLocalTempH
;--------------------------------------------------
;	CONSTANTS file register variables
;--------------------------------------------------
;16bit variables are stored H byte L byte
	;CBLOCK	MAIN_DATA_BLOCK ;Currently uses top half of bank0 (non access bank)

;--------------------------------------------------------
;			Global variables
;--------------------------------------------------------
;access bank=low 96 bytes of BANK0 and top 160 bytes of bank15
;note this is not all of the sfr, including some CAN
  

RESET_VECTOR	CODE 0
 
 
	goto	Start
HI_INT_VECT	CODE 8		;interrupt vector
	goto	ISRHigh 
LO_INT_VECT	CODE 0x18	;interrupt vector
	goto	ISRLow 
      
GSI_START	CODE
;-----------------------------------------------------------------------------------
; UserAbort Code. User has pressed (or sent Ctr-c down rs232 while in ascii mode)
;-----------------------------------------------------------------------------------
UserAbort
	bsf	GlbGSIState,GSI_STATE_USER_ABORT
	bra	StartUserAbort	
;-----------------------------------------------------------
;	Main program code start 
;-----------------------------------------------------------

Start
;-----------------------------------------------
;Have to store RCON as the clrwdt instruction
;will be used before RCON is read. This will
;remove the WDT reset flag! So must use GiRCON
;to check NOT_TO bit
;-----------------------------------------------
	bcf	GlbGSIState,GSI_STATE_USER_ABORT
StartUserAbort	

	banksel	GiBank
	
#IF INTERRUPTS
    ;bsf	INTCON,GIEH,A  done in GsiInitInterrupts (interrupts.asm
	;bsf	INTCON,GIEL,A/
    ;bsf INTCON, GIE
#ENDIF
    call    Restart
    call    GSISignOn
    
;---------------------------------------------------------------------------------
;disabled the call to SendResetTypeToPc as if the PC is not present, this code
;will wait for an echo of chars (or wait for ACK of a break condition this code sets)
;We will never return.
;However this does seem to work outside of the debugger!
;Frankly I have no idea what is going on at present (18/6/2012) its a few
;years since I left this code at this point
	;call	SendResetTypeToPc
;--------------------------------------------------------------------------------
	call	R2DebugCode

	#IF GSI_CAN_SUPPORT
	;call	CANDebugCode
	#ENDIF ;GSI_CAN_SUPPORT

;4 line display is effectively 2 separate 2 line lcd's	
;cls and leave at top left
	IF TEST_RX_CODE
	IF SIMULATOR
	TEST_ADD_TO_RX	' '
	TEST_ADD_TO_RX	'2'		
	TEST_ADD_TO_RX	'3'
	TEST_ADD_TO_RX	'4'
	TEST_ADD_TO_RX	'5'
	TEST_ADD_TO_RX	'6'
	TEST_ADD_TO_RX	'7'
	TEST_ADD_TO_RX	'8'
	TEST_ADD_TO_RX	'9'
	TEST_ADD_TO_RX	'0'
	TEST_ADD_TO_RX	13

	TEST_ADD_TO_RX	14
	TEST_ADD_TO_RX	13
	TEST_ADD_TO_RX	'9'
	TEST_ADD_TO_RX	25

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
	call	GSIProcessRxByte
	call	GSIProcessRxByte
	call	GSIProcessRxByte

	TEST_ADD_TO_RX	3
	
	ENDIF	;SIMULATOR
	ENDIF	;TEST_RX_CODE
#IF 0
rloop	
	movlw	0xaa
	call	GSISendByte232
	movlw	0x55
	call	GSISendByte232
	bra		rloop	
#ENDIF
	call	DebugCurrentCode
;-------------------------------------------------------------
;	Main program loop
;This is an endless loop unless user CTR-C or WDT
;will poll keypad and service usart Rx interrupts
;-------------------------------------------------------------

L1	clrwdt	       			;reset WDT (nominal 18ms period)

	btfsc	GlbSIOStatus,SIO_USR_ABRT
	goto	UserAbort


        ;movlw	'G'
	;call	GSISendByte232

	
	movf	GiIntType,F
	btfss	STATUS,Z
	call	IdentifyInt
	
	movf	GlbErrState,F
	btfss	STATUS,Z	;state holds pending message info
	call	GSIWriteErrorMessage
	CHECK_BSR MAIN,'M'
	;if a keypad key is down we need to poll the debounce
	;function. Port change int is no good for detecting
	;multiple simultaneous keystrokes: we must poll.
	;First int is picked up but if
	;a subsequent key in the same row is depressed then the
	;rbif is not fired (port hasn't changed!), we miss the key!
	;So don't use port change int
	;This version doesn't use PORTB in any case, so a moot point.
	#IF GSI_KEYPAD_SUPPORT
	btfsc	GlbKeypadStatus,KEY_DEBOUNCING
	call	GSIKeypadDebounce
	#ENDIF

	btfsc	GlbSIOStatus1,SIO_S1_CHAR_AVAILABLE
	call	GSIProcessRxByte	;Will change BANK


	;btfsc	GlbDebugFlag,DEBUG_CALL_DEBUG_CODE
	call	DebugCurrentCode  ;keypad '*' SetCanMaster; keypad '#' SetCanSlave
;------------CAN checking --------------------------	
	#IF GSI_CAN_SUPPORT
_check_for_can_mssg:
	banksel CnBank
	movf	CnCQRxNumInBuff,W	;get the num in buff 
	bz	_no_new_can_mssg	
	call	ProcessCQRxCANMessage	;message is in CnRxMssg 
	bra	_check_for_can_mssg
_no_new_can_mssg:  						;No NEW CAN mssg, though can still be an earlier one pending

;--------------------------------------------------	

	movf	CnCQRxR2NumInBuff,W			;
	banksel	GiBank						;movlb doesn't affect any flags	
	bz		__no_can_pending

	;Next function will send a BREAK to pc if one hasn't been sent (to let it know we want to be R2Master)
	call	GSIR2RecordDataPendingTxToRS232 
	
	btfsc	GlbSIOStatus1,SIO_S1_MASTER		;only Tx if we are the master
	call	RS232SendPendingCANMssg			;will retrieve mssg fromQ
__no_can_pending:	


	banksel	GiBank		
	#ENDIF ;CAN_SUPPORT
;What if we have CAN DATA pending as well? 
	btfss	GlbSIOStatus2,SIO_S2_RS232_DATA_PENDING_TX
	bra	__no_rs232_pending
	call	GSIR2RecordDataPendingTxToRS232 
	btfsc	GlbSIOStatus1,SIO_S1_MASTER
	call	RS232TxRS232Record	;send a single record
	call	GSIRS232SetSlave			;pc will become master after it receives
								;see also R2_BREAK_SEND_MULTIPLE_RECORD
	;bcf		GlbSIOStatus2,SIO_S2_RS232_DATA_PENDING_TX		;currently must be cleared here as this function
	;Now cleared in RS232TxRS232Record
	;is shared by the CAN writing function, it reset the SIO_S1_CAN_DATA_PENDING_TX bit
	;can probably clear both in  
__no_rs232_pending:	

#if SIMULATOR
	bsf	GlbDebugFlag,DEBUG_CALL_DEBUG_CODE
	call	DebugCurrentCode
#endif
	call	GSIUserMain
	banksel	GiBank
	goto	L1	

;-----------------------------------------------------------
;	SendBreakType
;send break sequence to pc. If no pc then ignore any error
;GlbLocalTempL holds GlbSIOMode value before reset.
;use to determine if PC is still in binary mode
;if so, then send binary data, otherwise send the sign-on
;message
;Uses:
;GlbDontUseWithCAN	;holds resetType
;GlbDontUseWithCAN+1 			;holds GlbSIOMode
;NOTE
;If this is a POR we must send the break as the pc might have
;been attached to us in binary mode but we suffered a power failure
;----------------------------------------------------------
SendResetTypeToPc

	movlw	R2_BREAK_RESET_POR
	subwf	GlbDontUseWithCAN,W		;was this a POR
	bz		__srttpc_por

#IF DEBUG		;ICD2 will give us an unknown reset type
	movlw	R2_BREAK_RESET_UNKNOWN
	subwf	GlbDontUseWithCAN,W		;was this a POR
	bz		__srttpc_por
#ENDIF

	btfss	GlbDontUseWithCAN+1,SIO_M_BINARY
	bra		__srttpc_ascii_mode
__srttpc_por
#IF ENABLE_BREAK		
;-Either the pc is in binary mode or this was a POR and we
;don't know what the pc mode is!
	call	GSISendBreak
	movf 	GlbDontUseWithCAN,W		    ;Get type of reset (temporary variable)
	call	GSISendByte232				;send the reset type

	movwf	GlbLocalTempL
	movlw	4				;timeout for n 13msec ticks		
	movwf	GlbLocalTempH
	lfsr	FSR0,GlbLocalTempL
	call	GSILookForR2RxChar	;using timeout
	btfsc	STATUS,C			;C == to or echo error
	bra		__srttpc_exit		;C, error so ignore

;pc will send ACK once it is finished processing and back in correct
;mode etc.
	movlw	RT_GSI_ACK		;look for ACK from pc
	movwf	GlbLocalTempL
	movlw	0					;infinite timeout, will wdt if error
	movwf	GlbLocalTempH
__srttpc1
	call	GSILookForR2RxChar	;using timeout
	bc		__srttpc1	

__srttpc_exit
	banksel GiBank
#ENDIF
	return

__srttpc_ascii_mode
	movf 	GlbDontUseWithCAN,W		    ;Get type of reset
	banksel	R2Bank
	call	GSIWriteMessageRS232
	
	bra	__srttpc_exit


;-------------------------------------------------------
;		String table
;-------------------------------------------------------

PgmMssgTbl	
	DW	VersionStr		;0
	DW	WDTResetStr		;1
	DW	BrownoutResetStr	;2
	DW	UnknownResetStr		;3
	DW	UserAbortStr		;4
	DW	PowerOnResetStr		;5
	DW	StackOvfResetStr	;6
	DW	StackUnfResetStr	;7
	DW	ErrI2CNoAckStr		;8
	DW	ErrI2CNoDataStr		;9
	DW	ErrI2CBusCollStr	;10
	DW	UnknownErrorStr		;11

	DW	CANEWarn		;12 COMSTAT,EWARN	
	DW	CANRxWarn		;13 COMSTAT,RXWARN
	DW	CANTxWarn		;14 COMSTAT,TXWARN
	DW	CANRxBP			;15 COMSTAT,RXBP
	DW	CANTxBP			;16 COMSTAT,TXBP
	DW	CANTXBO			;17 COMSTAT,TXBO
	DW	CANRxB1Ovfl		;18 COMSTAT,RXB1OVFL
	DW	CANRxB0Ovfl		;19 COMSTAT,RXB0OVFL
	DW	CANIRXIF		;20 PIR3,IRXIF
	DW	CANErrorInit		;21
	DW	CANUnknownError		;22

	DW	CANMssgStr		;23 About to write a CAN mssg
	DW	CANMssgIdStr		;24 mssg Id str
	DW	CANMssgDataStr		;25 mssg Data str
	DW	CANMssgLengthStr	;26 mssg Data Length str
	DW	CANMssgFlagStr		;27 mssg Flag str
	DW	CANSlaveIdStr		;28

	DW	ErrEchoStr			;29
	DW	CANRxCQOvfStr		;30
	DW	CANNodeThisStr		;31

StrTable
VersionStr	DATA	"GSI 0.90",0			;0 beta
WDTResetStr	DATA	"WDT Reset",0			;1
BrownoutResetStr DATA	"Brownout Reset!",0		;2
UnknownResetStr	DATA	"Unknown Reset!",0		;3
UserAbortStr	DATA	"User Abort Reset",0		;4
PowerOnResetStr	DATA	"Power On Reset",0		;5
StackOvfResetStr DATA	"Stack overflow!",0		;6
StackUnfResetStr DATA	"Stack underflow!",0		;7
ErrI2CNoAckStr	DATA	"No ACK I2C slave",0		;8
ErrI2CNoDataStr	DATA	"No I2C data!",0		;9
ErrI2CBusCollStr DATA	"I2C Bus coll!",0		;10
UnknownErrorStr	DATA	"Unknown Error!",0		;11
CANEWarn	DATA	"CAN EWARN!",0			;12 COMSTAT,EWARN	
CANRxWarn	DATA	"CAN RXWARN!",0			;13 COMSTAT,RXWARN		
CANTxWarn	DATA	"CAN TXWARN!",0			;14 COMSTAT,TXWARN
CANRxBP		DATA	"CAN RXBP!",0			;15 COMSTAT,RXBP
CANTxBP		DATA	"CAN TXBP!",0			;16 COMSTAT,TXBP
CANTXBO		DATA	"CAN TXBO!",0			;17 COMSTAT,TXBO
CANRxB1Ovfl	DATA	"CAN RXB1Ovfl!",0		;18 COMSTAT,RXB1OVFL
CANRxB0Ovfl	DATA	"CAN RXB0Ovfl!",0		;19 COMSTAT,RXB0OVFL
CANIRXIF	DATA	"CAN Invalid mssg",0		;20 PIR3,IRXIF
CANErrorInit	DATA	"CAN Init Fail!",0		;21 timeout in setting CAN mode
CANUnknownError	DATA	"CAN Unknown Err!",0		;22 Unknown CAN error
CANMssgStr	DATA	"CAN Messg:",0			;23 About to write a CAN mssg
CANMssgIdStr	DATA	"CAN Messg Id",0		;24 mssg Id str
CANMssgDataStr	DATA	"CAN Messg Data",0		;25 mssg Data str
CANMssgLengthStr DATA	"CAN Messg DLength",0		;26 mssg Data Length str
CANMssgFlagStr	DATA	"CAN Messg Flags",0		;27 mssg Flag str
CANSlaveIdStr	DATA	"CAN Set slave Id",0		;28

ErrEchoStr	DATA	"Echo error!",0			;29
CANRxCQOvfStr	DATA	"CAN Rx CQ overflow!",0	 ;30 the Rx CQ has overflowed
CANNodeThisStr	DATA	"CAN node=",0
;-----------------------------------------------------
;		End of string table
;-----------------------------------------------------


;-----------------------------------
;	ProcessReceivedChar
;receive index in W. Max index=64-1
;-----------------------------------
GSIProcessRxByte
	call	GSIGetFromRS232RxBuff
	btfsc	STATUS,Z
	return
	;goto	GSICommonReturn

	btfsc	GlbSIOMode, SIO_M_BINARY
	goto	BinaryProcessRxByte

	;echo all bytes if echo flag is set
	call	GSIEchoByte232
	btfsc	GlbSIOStatus1,SIO_S1_ESC
	goto	esc_command
GSIProcessRxByteRS232Entry:
GSIProcessRxByteCnEntry:
	sublw	LAST_LCD_COMMAND+1
	btfss	STATUS,C,A
	goto	not_control_code
	;-command 
	sublw	LAST_LCD_COMMAND+1	;retreive index
	;Calculate computed goto offset/PCLATH
	;Multiply index by 4. On the earlier pics this was left unmodified
	;however the pc on the 18 bit core is a byte rather than word counter
	;so mult *2. The goto is a 2 word instruction so *4
	;hence a max of 256/4 goto's in the table
	;Don't need to include PCLATU in the computation as on the 458
	;we only have 32k bytes of program mem so all memory is in 
	;bottom 64k and will not use PCLATU
	;NOTE Should recode this for bra instruction
GSIProcessRxByte_ctr_code
	addwf	WREG
	addwf	WREG
	movwf	GlbCompOffset
	movlw 	LOW control_code_table 	;get low 8 bits of address
	addwf 	GlbCompOffset,F	;do an 8-bit add operation
	movlw 	HIGH control_code_table 	;get high 5 bits of address
	btfsc 	STATUS,C,A 	;page crossed?
	addlw 	1 		;yes then increment high address
	movwf	PCLATH,A 		;load high address in latch
	movf	GlbCompOffset,W	;load computed offset in w reg
	banksel	GiBank
	movwf	PCL,A
control_code_table	;gotos are executed with PCLATH == PAGE0, tf must be short gotos
	goto	GSINull		;0 
	goto	GSIBeginningOfLine	;1 ctr-a
	goto	GSICursorLeft		;2 ctr-b
	goto	GSIUsrAbrt		;3 ctr-c
	goto	GSIDeleteCharFwd	;4 ctr-d
	goto	GSIEndOfLine		;5 ctr-e
	goto	GSICursorRight		;6 ctr-f
	goto	GSIR2ReqMaster		;7 ctr-g sender wants to be rs232 maste3r
	goto	GSIBS			;8 ctr-h
	goto	GSIHTab		;9 ctr-i
	goto	GSICursorDown		;10 ctrj
	goto	GSIEnd			;11 ctr-k
	goto	GSICls			;12 ctr-l
	goto	GSICR			;13 ctr-m
	goto	GSICursorDown		;14 ctr-n
	goto	GSIEchoToggle		;15 ctr-o
	goto	GSICursorUp		;16 ctr-p 
	goto	GSIXOn			;17 ctr-q
	goto	GSIBinaryMode		;18 ctr-r
	goto	GSIXOff		;19 ctr-s
	goto	GSICursorRight5	;20 ctr-t
	goto	GSIAsciiMode		;21 ctr-u
	goto	GSICursorLeft5		;22 ctr-v
	goto	GSICursorLeft10	;23 ctr-w
	goto	GSICRKeypadToggleState	;24 ctr-x
	goto	GSIKeypadToggleMode	;25 ctr-y
	goto	GSICtrZ		;26 ctr-z
	;end of control code jump table
esc_command	
	sublw	LAST_ESC_COMMAND+1
	btfss	STATUS,C,A
	goto	invalid_esc_command
	bcf	GlbSIOStatus1,SIO_S1_ESC
	sublw	LAST_ESC_COMMAND+1	;restore W
GSIProcessRxByte_esc_code
	;Calculate computed goto offset/PCLATH
	addwf	WREG
	addwf	WREG
	movwf	GlbCompOffset
	movlw 	LOW esc_code_table 	;get low 8 bits of address
	addwf 	GlbCompOffset,F	;do an 8-bit add operation
	movlw 	HIGH esc_code_table 	;get high 5 bits of address
	btfsc 	STATUS,C,A 	;page crossed?
	addlw 	1 		;yes then increment high address
	movwf	PCLATH,A 		;load high address in latch
	movf	GlbCompOffset,W	;load computed offset in w reg
	banksel	GiBank
	movwf	PCL,A
esc_code_table	;gotos are executed with PCLATH == PAGE0, tf must be short gotos
	goto	GSIVersionString
	goto	GSICapabilities
	goto	GSIGetBuffer
	goto	GSISetBuffer
	goto 	GSICls
	goto	GSIInstruction
	goto	GSIGetCurToEol
	goto	GSIGetLine
	goto	GSISetLine
	goto	GSIHome
	goto	GSIFlowControlOn
	goto	GSIFlowControlOff
	goto	GSILCDInit
	goto	GSIGetButtonState
	#IF GSI_CAN_SUPPORT
	goto	GSICANSetMaster
	goto	GSICANSetSlave
	#ELSE
	;no CAN support but need dummy jmp table entries in case
	;we add anything after these
	goto	__sm_dummy
	goto	__ss_dummy
__sm_dummy
__ss_dummy
	return
	#ENDIF

	;end of jump table
;Following only called from ProcessR2RxRecord (R2Slave.asm)
GSIProcessRxByte_user_code
	;Calculate computed goto offset/PCLATH
	addwf	WREG
	addwf	WREG
	movwf	GlbCompOffset
	movlw 	LOW user_code_table 	;get low 8 bits of address
	addwf 	GlbCompOffset,F	;do an 8-bit add operation
	movlw 	HIGH user_code_table 	;get high 5 bits of address
	btfsc 	STATUS,C,A 	;page crossed?
	addlw 	1 		;yes then increment high address
	movwf	PCLATH,A 		;load high address in latch
	movf	GlbCompOffset,W	;load computed offset in w reg
	banksel	GiBank
	movwf	PCL,A
user_code_table	;gotos are executed with PCLATH == PAGE0, tf must be short gotos
	goto	GSIUserR2Record_0
	goto	GSIUserR2Record_1
	goto	GSIUserR2Record_2
	goto 	GSIUserR2Record_3
	goto	GSIUserR2Record_4
	;end of jump table


not_control_code
	;still in MAIN_BANK
	;must be ESC or an ascii tty write to the LCD
	;if ESC W=f5
	sublw	LAST_LCD_COMMAND+1-d'27'
	btfsc	STATUS,Z,A
	goto	begin_esc_sequence	
	addlw	d'27'	;restore W
	;ok just an ascii char to print, W holds char to print
	;call	GSIEchoByte232
	banksel	GiBank
GSILCDWriteChar:
	#IF GSI_LCD_SUPPORT
	movwf	LdTempWriteChar
	call	LCDWriteChar
	#ENDIF ;GSI_LCD_SUPPORT
	call	GSICursorRight
	movf	LdTempWriteChar,W	
	return
;GSICommonReturn
;	return
;These two can return rather than common as they are
;still in BANK0
invalid_esc_command
	;goto	GSICommonReturn
	return
begin_esc_sequence
	bsf	GlbSIOStatus1,SIO_S1_ESC
	return
	;goto	GSICommonReturn


;--------------------------------------------------
;	Test232
;Assumes nothing
;--------------------------------------------------
Test232
	;sio output
	movlw	'G'
	call	GSISendByte232
	movlw	'l'
	call	GSISendByte232
	movlw	'e'
	call	GSISendByte232
	movlw	'n'
	call	GSISendByte232
	movlw	'n'
	call	GSISendByte232
	movlw	0x0d
	call	GSISendByte232
	movlw	0x0a
	call	GSISendByte232
	return


;----------------------------------------------------------------
;	IdentifyInt
;High level int handlers are called from here.
;Low level ISR handlers can handle the int directly or can perform
;minimum processing and set a bit in the GiIntType variable.
;GiIntType is checked in the main program loop and this function
;is called if any low level handler has deferred processing
;This can allow us to do lengthy interrupt processing while allowing
;other interrupts to be serviced.
;Low level int handlers are in interrupts.asm
;----------------------------------------------------------------

IdentifyInt
;see what int occured and reset its flag in INTCOM
;ints do not nest as there is only a single chip int flag
;Only InIntxx handlers that set GiIntType variable will
;be called by this routine
	
	btfsc	GiIntType,T0IF	;TMR0
	call 	TMR0IntTaken
       ;-
       	;btfsc	GiIntType,RBIF	;PortChange
	;call	GSIKeypadDebounce
ii1
	IF 0
	btfsc	GiIntType,INTF	;RB0\INT	;currently no INT0 enabled
	call	RB0IntTaken
	ENDIF
       ;-
	btfsc	GiIntType,RCIF
	call	RxIntTaken
	;
	IF 0			;TMR1 is a high priority int as it is used for CAN Sync
	btfsc	GiIntType,TMR1TYPE
	call	TMR1IntTaken
	;
	btfsc	GiIntType,TXIF
	call	TxBuffEmptyIntTaken
	ENDIF
	;-

	return

TMR1IntTaken
	bcf	GiIntType,TMR1TYPE
	return



;-----------------------------------------------------
;	TMR0IntTaken
;currently interrupts @13ms intervals @20MHz
;interval =256*256(prescaler) =65536 *200ns =13ms
;
;Only processing at present is to poll the keypad
;-----------------------------------------------------

TMR0IntTaken
	#IF GSI_KEYPAD_SUPPORT
	call	GSIKeypadDebounce
	#ENDIF
	bcf	GiIntType,T0IF
	bsf	INTCON,T0IE,A
	;allow to free run using delay set in InTimerDefault
	return
	

;-------------------------------------------------------------
;	RB0IntTaken
;IntType,INT0IF was set by ISR, requires processing of int in
;process time rather than interrupt time
;--------------------------------------------------------------
RB0IntTaken
	bcf	GiIntType,INT0IF			;clr flag
	return

;-------------------------------------------------------------
; TxBuffEmptyIntTaken
;IntType,TXIF was set by ISR, requires processing of int in
;process time rather than interrupt time
;-------------------------------------------------------------
TxBuffEmptyIntTaken
	bcf	GiIntType,TXIF	;PIR1,TXIF is read only
	return

;--------------------------------------------------------------
;	RxIntTaken
;IntType,RCIF was set by ISR, requires processing of int in
;process time rather than interrupt time
;---------------------------------------------------------------
RxIntTaken
;RxType has bit set indicating the type of char
;it might be data or a command
;This function currently not called as the InRxInt (called from ISR)
;handles the Rx interrupt
	bcf	GiIntType,RCIF		;PIR1, RCIF is read only
	return

	IF 0
rit1	
	call	GSIGetFromRxBuff
	btfsc	STATUS,Z,A
	return
	call	GSISendByte232				;echo the byte
	goto	rit1
	ENDIF ;IF 0


;-------------------------------------------
;		End High level int handlers
;-------------------------------------------

	IF ENABLE_CHECK_BSR
DEBUGCheckBSR
	subwf	BSR,W,A
	bnz	cbsError
	return

cbsError
	movlw	'!'
	call	GSISendByte232
	movf	GlbDebugTempL,W
	call	GSISendByte232	
	return
	ENDIF

;---------------------------------------------
; 	Max518Output
;output WREG to I2C, set a voltage on the dac
;----------------------------------------------
Max518Output
	movwf	GlbLocalTempL	;store value
	#IF GSI_I2C_SUPPORT
	call	GSIAddToI2CTxBuff
	#endif	
	return



;-----------------------------
;	GSIDelay5ms
;Assumes GSI_BANK
;-----------------------------

GSIDelay5ms                           ;  GSIDelay 5 msecs
	IF(SIMULATOR)
	return
	ENDIF
	call	GSIDelay1.53ms
	call	GSIDelay1.53ms
	call	GSIDelay1.53ms
	call	GSIDelay1.53ms
	return

GSIDelay50ms
	call   	GSIDelay5ms                  ;  Wait 20 msecs before Reset
	call   	GSIDelay5ms
	call   	GSIDelay5ms
	call   	GSIDelay5ms
	call	GSIDelay5ms
	call   	GSIDelay5ms                  ;  Wait 20 msecs before Reset
	call   	GSIDelay5ms
	call   	GSIDelay5ms
	call   	GSIDelay5ms
	call	GSIDelay5ms
	return

GSIDelay500ms
	call	GSIDelay50ms
	call	GSIDelay50ms
	call	GSIDelay50ms
	call	GSIDelay50ms
	call	GSIDelay50ms
	call	GSIDelay50ms
	call	GSIDelay50ms
	call	GSIDelay50ms
	call	GSIDelay50ms
	call	GSIDelay50ms
	return


;--------------------------------
;GSIDelay1Sec
;Assumes GSI_BANK
;--------------------------------
GSIDelay1Sec
#if (SIMULATOR==FALSE)
	movlw	d'200'
	movwf	GiDelayCounterH
d1s
	call	GSIDelay5ms
	decf	GiDelayCounterH,F
	btfss	STATUS,Z,A
	goto	d1s
#endif
	return

;-----------------------------
;	GSIDelay160us
;-----------------------------

GSIDelay160us                         ;  GSIDelay 160 usecs
;400 instructions @10MHz
#IF(SIMULATOR ==FALSE)
	call	GSIDelay39us
	call	GSIDelay39us
	call	GSIDelay39us
	call	GSIDelay39us
#endif
	return

;-----------------------------
;GSIDelay43us
;GSIDelay39us
;Assumes GSI_BANK
;-----------------------------

GSIDelay43us
GSIDelay39us
;This is also called by WriteChar which reqLdre 43us
;so give at least 43us (104 instructions)

	clrwdt
	IF(SIMULATOR)
	return
	ENDIF
	;GSIDelay 39us 98 instruction @10 MHz
	;200 @ 20Mhz
	IF (CLOCK == d'10')
	movlw	d'25'
	ENDIF
	IF (CLOCK == d'20')
	movlw	d'50'
	ENDIF

	movwf	GiDelayCounter
d3us1
	decf	GiDelayCounter,F	
	btfss	STATUS,Z,A
	goto	d3us1
	;107 instruction delay
	return


;-----------------------------
;	GSIDelay1.53ms
;Assumes GSI_BANK
;-----------------------------

GSIDelay1.53ms
;3825 instructions @10MHz
;7650 @ 20MHz

	IF(SIMULATOR==FALSE)
	return
	

	clrf	GiDelayCounter
d4
	decf	GiDelayCounter,F
	clrwdt
	nop			;a goto==2 cycles
	goto	$+2
	goto	$+2
	goto	$+2
	goto	$+2
	goto	$+2
	IF (CLOCK == d'20')
	clrwdt
	nop
	goto	$+2
	goto	$+2
	goto	$+2
	goto	$+2
	goto	$+2
	goto	$+2
	ENDIF
	btfss	STATUS,Z,A
	goto	d4
	;4096 instructions @10MHz
    ENDIF   ;SIMULATOR
	return


;----------------------------------------------------------
;	DebugCurrentCode
;----------------------------------------------------------
DebugCurrentCode

	btfsc	GlbDebugFlag,DEBUG_CALL_DEBUG_CODE ;'*'
	bra	dc0
	btfsc	GlbDebugFlag,DEBUG_CALL_DEBUG_CODE_1;'#'
	bra	dc1
	btfsc	GlbDebugFlag,DEBUG_CALL_DEBUG_CODE_2;'0'
	bra	dc2
	banksel	GiBank
	return
dc0 
	#IF GSI_CAN_SUPPORT
	call	GSICANSetMaster		;'*'	
	bcf	GlbDebugFlag,DEBUG_CALL_DEBUG_CODE
	banksel	GiBank
	#ENDIF
	return
dc1
	
	#IF GSI_CAN_SUPPORT
	nop
	call	GSICANSetSlave		;'#'
	bcf	GlbDebugFlag,DEBUG_CALL_DEBUG_CODE_1
	banksel	GiBank
	#ENDIF
	return


dc2
#if 0
	;call	dc2					;generate stack ovf
	;goto	UserAbort
	;call	GSISendBreak
	;call	GSIDelay5ms			;works with this delay, fails without
	;call	GSIDelay1.53ms
	;call	GSIDelay160us                         ;  GSIDelay 160 usecs
	;movlw	UNKNOWN_RESET_MSSG
	;movlw	'U'							;mimic a loss of power
	;banksel	R2Bank
	;call	GSISendByteWithEcho232				;C on error	
	;banksel	GiBank
	
	;call	GSIDelay5ms
	;movlw	UNKNOWN_RESET_MSSG
	;call	GSISendByte232				;C on error	

	;-----Setup for timeout, waiting for an ACK---------------
	;0=infinite timeout (no wdt)
#IF DEBUG	
	movlw	0		;count 20 *13mSec (uncertainty of approx 13msec)
#ELSE
	movlw,20
#ENDIF		
	movwf	GlbLocalTempH		;timeout interval
	movlw	RT_GSI_ACK
	movwf	GlbLocalTempL		;char to find in rx buff
	
	lfsr	FSR0,GlbLocalTempL	;timeout value (in 13msec ticks)
	call	GSILookForR2RxChar	;using timeout
;timeout: C,Z
;fail (char found but not an ACK): C,NZ
;success NC	
	btfsc	STATUS,C
	bra	__gcdpt_exit	;try again later
	nop					;ACK found

	;movlw	UNKNOWN_RESET_MSSG
	;call	GSISendByteWithEcho232				;C on error	

__gcdpt_exit:
	bcf	GlbDebugFlag,DEBUG_CALL_DEBUG_CODE_2


	return
#endif	
	;movlw	CAN_EWARN_MSSG
	;call 	GSIWriteErrorMessageRS232
	
#IF GSI_CAN_SUPPORT	
	#if 0
		movlw	1					;ping the master 0, or slave 1
		call	GSISendCANPingMssg
	#endif	
#ENDIF
#if 1
	clrf	WREG
	;R2TxRecordType		RES 1	;A binary Mssg
	;R2TxRecordNode		RES 1	;the node this data is for
	;R2TxRecordLength	RES 1   ;the number of bytes in record
	;R2TxRecord		RES R2_MAX_TX_RECORD_DATA	;the data 0-8 +4 bytes for CAN_ID +1 byte for RecFlags if a CAN mssg
	movlw	RT_GSI_R2_RX_CAN_MSSG_NP	;simulate a received CAN message
	;movlw	RT_GSI_KEYPAD_MSSG
	movwf	R2TxRecordType 		;simulate a keypad message
	
	movlw	CAN_NODE_THIS		;on this node
	movwf	R2TxRecordNode		
	
	movlw	8					;data bytes, an embedded (compressed) CAN frame
	movwf	R2TxRecordLength

	;simulate CAN ID for a keypad up from node 1. The id will be the id of this node as the slave
	;has sent the message to us, thus it will be an offset to node 0 so :4064 (GSI BASE)+4 (KEYPAD_MSSG)=4068 (0x00000fe4)	
	movlw	0xe4				
	movwf	R2TxRecordData+0
	movlw	0x0f
	movwf	R2TxRecordData+1
	movlw	0
	movwf	R2TxRecordData+2
	movwf	R2TxRecordData+3	

	movlw	254
	movwf	R2TxRecordData+4		;node of sender
	movwf	R2TxRecordData+5		;length of data exluding flag
	
	movlw	'G'						;data, simulate a 'G', impossible on this numeric keypad
	movwf	R2TxRecordData+6
	
	movlw	0x25		;CAN flag	;flag, no error
	movwf	R2TxRecordData+7
		
	;movlw	CAN_KD_MSSG_KEY_UP	;subfunction uses same constant as they support same functions
	;movwf	R2TxRecordData+0
		
	;movlw	'G'					;holding "G" Not using subfunctions, just a debug simulation
	;movwf	R2TxRecordData+1
	bsf		GlbSIOStatus2,SIO_S2_RS232_DATA_PENDING_TX	;main prog loop should send it...
	clrwdt
#if 0
;record sent, now send an rs233 mssg with REQ_SLAVE so that
;pc will become MASTER again
	movlw	RT_GSI_RS232_MSSG
	movwf	R2TxRecordType	;a generic rs232 message
	movlw	GSI_RS232_MESSAGE_REQ_SLAVE
	movwf	R2TxRecordData	;subfunction REQ_SLAVE (this node wants to be slave)		
	movlw	1
	movwf	R2TxRecordLength	;record data is 1 byte only
#if	ENABLE_CALLER_NODE_SENDBACK		;TRUE 8/1/07
	movff	CnThisNode,R2TxRecordNode	;node of sender
#else
	movlw	CAN_NODE_THIS		;let pc know it is this node
	movwf	R2TxRecordNode
#endif
	call	RS232TxRS232Record	;and send
	;on error we will wdt
	call	GSIRS232SetSlave
#endif

#endif	

	banksel	GiBank	
	bcf	    GlbDebugFlag,DEBUG_CALL_DEBUG_CODE_2 
	return

#if 0
	#IF GSI_CAN_SUPPORT
	banksel GiBank			;'0'
	movlw	1			;node number (id =num*4096)
	movwf	GiCANTxNode
	;-
	movlw	3			;length=3 bytes
	movwf	GiCANTxBuffLength
	;-
	movlw	MAX518_BASE_ADDR	;
	rlncf	WREG			;an 8 bit *rotation*, not a shift, be sure bit 7=0
	;no need to bcf WREG,0 as this will be zero. Bit 0 is the write bit
	;call	GSIAddToI2CTxBuff	;add address to buff
	movwf	GiCANTxBuff
	;-
	movlw	MAX518_CMD_SLCT_DAC0
	;call	GSIAddToI2CTxBuff
	movwf	GiCANTxBuff+1
	;-
	movlw	0x40
	;call	GSIAddToI2CTxBuff	;output byte for dac0
	movwf	GiCANTxBuff+2
	;-
	lfsr	FSR0,GiCANTxRecord	;use FSR0 as source		

	call	GSISendCANI2CMssg
	bcf	GlbDebugFlag,DEBUG_CALL_DEBUG_CODE_2
	banksel	LdBank
	#ENDIF	;GSI_CAN_SUPPORT
#endif	
	return

	IF DEBUG
	ENDIF

;-----------------------------------------------------
;	GSIMakeAscii
;Receive byte in WREG
;convert to ascii and write to 4 byte buffer at FSR0
;3 bytes for the data, 1 byte for temp storage
;-----------------------------------------------------
GSIMakeAscii

	;movwf	R2AsciiTemp
	movwf	GlbLocalTempL
	clrf	GlbLocalTempH
	movlw	d'100'

	btfss	GlbLocalTempL,7		;>=128?
	bra		ma_2
	incf	GlbLocalTempH
	subwf	GlbLocalTempL,F	
ma_2
	
ma_hundreds
	subwf	GlbLocalTempL,F
	bn		ma_tens
	incf	GlbLocalTempH,F
	bra		ma_hundreds
ma_tens
	addwf	GlbLocalTempL,F
	movf	GlbLocalTempH,W
	addlw	0x30			;100's
	movwf	POSTINC0		;store and inc
	;call	GSISendByte232
	clrf	GlbLocalTempH
	movlw	d'10'
ma_1
	subwf	GlbLocalTempL,F
	bn		ma_units
	incf	GlbLocalTempH,F
	bra		ma_1
ma_units
	addwf	GlbLocalTempL,F
	movf	GlbLocalTempH,W
	addlw	0x30			;10's
	movwf	POSTINC0
	
	movf	GlbLocalTempL,W
	addlw	0x30			;units
	movwf	POSTDEC0
	
	decf	FSR0L			;set back to start of buff
	
	;movf	R2AsciiTemp,W
	return
;--------------------------------



	END
