;******************************************************************************
;* @file    gsi.inc part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************


;GSIInterface.inc
#ifndef GSI_INC_INCLUDED
#define GSI_INC_INCLUDED

#include "equ.inc"
#include "gsmacros.inc"

#include "defs.inc"

#include "debug.inc"

#IF DEVICE == _18F458		;See debug.inc, perhaps DEBUG should be moved to equ.inc?
#include "p18xxx8.inc"
	LIST   P=PIC18F458
#ELSE
#INCLUDE "P18F4680.inc"
	LIST   P=PIC18F4680
#ENDIF

;------------------------------------------------------
;Enable runtime support for resources, user may change
;------------------------------------------------------
;See variable GlbResources GlbResources1
GSI_RESOURCE_LCD_BIT	EQU	0	;clr for no LCD
GSI_RESOURCE_RS232_BIT	EQU	1	;clr for no rs232
GSI_RESOURCE_I2C_BIT	EQU	2	;clr for no i2c
GSI_RESOURCE_KEYPAD_BIT	EQU	3	;clr for no keypad
GSI_RESOURCE_CAN_BIT	EQU	4	;clr for no CAN
GSI_RESOURCE_DIO_BIT	EQU	5	;clr for no DIO
GSI_RESOURCE_INT_BIT	EQU	6	;clr for no RB0,RB1 Interrupts
GSI_RESOURCE_SPI_BIT	EQU	7	;clr for no SPI

GSI_RESOURCE_1_DEBUG_BIT	EQU	0	;clr for no DEBUG code
GSI_RESOURCE_1_ADC_BIT		EQU	1	
GSI_RESOURCES_1_LP_INT_BIT	EQU	2	;Low priority int
GSI_RESOURCES_1_HP_INT_BIT	EQU	3	;high priority int

;----------------------------------------------------
GSI_RESOURCE_LCD	EQU	1	;clr for no LCD
GSI_RESOURCE_RS232	EQU	2	;clr for no rs232
GSI_RESOURCE_I2C	EQU	4	;clr for no i2c
GSI_RESOURCE_KEYPAD	EQU	8	;clr for no keypad
GSI_RESOURCE_CAN	EQU	10h	;clr for no CAN
GSI_RESOURCE_DIO	EQU	20h	;clr for no DIO
GSI_RESOURCE_INT	EQU	40h	;clr for no RB0,RB1 Interrupts
GSI_RESOURCE_SPI	EQU	80	;clr for no SPI

GSI_RESOURCE_1_DEBUG	EQU	0h	;clr for no DEBUG code
GSI_RESOURCE_1_ADC	EQU	1	;clr for no ADC's
GSI_RESOURCES_1_LP_INT	EQU	2	;clr for no user isr call
GSI_RESOURCES_1_HP_INT	EQU	4	;clr for no user HP isr call
;----------------------------------------------------

;------------------------------------------------------
;Enable\disable support for various items at compile time
;------------------------------------------------------

GSI_LCD_SUPPORT		=		TRUE
GSI_RS232_SUPPORT	=		TRUE
GSI_I2C_SUPPORT		=		TRUE
GSI_KEYPAD_SUPPORT	=		TRUE
GSI_CAN_SUPPORT		=		TRUE
GSI_DIO_SUPPORT		=		TRUE
GSI_INT_SUPPORT		=		FALSE
GSI_DEBUG_SUPPORT	=		TRUE
GSI_USER_LP_INT_SUPPORT	EQU		TRUE
GSI_USER_HP_INT_SUPPORT	EQU		TRUE

	#IF STARTLE
GSI_CAN_SUPPORT		=	TRUE		;required for IOCTL functions
GSI_LCD_SUPPORT		=	FALSE
GSI_KEYPAD_SUPPORT	=	FALSE
GSI_I2C_SUPPORT		=	FALSE
	#ELSE
GSI_LCD_SUPPORT		=	TRUE
GSI_CAN_SUPPORT		=	TRUE
GSI_KEYPAD_SUPPORT	=	TRUE
	#ENDIF

;-------------------------------------------------------
GSI_CAN_DEDICATED_MASTER	EQU	TRUE	;larger CAN buffers etc
;---------------------------------------------
;End of control defines



;--------------------------------------------
;GlbGSIState flags
;---------------------------------------------
GSI_STATE_USER_ABORT	EQU	0
;---------------------------------------------

;End GSIState flags


#ifdef	IN_GSI.ASM

	;data
	GLOBAL	GiBank, GlbGSIState ;,GlbResources
	GLOBAL	GSIDelay1Sec,GSIDelay160us,GSIDelay5ms,GSIDelay50ms
	GLOBAL GSIProcessRxByte,GSILCDWriteChar
	GLOBAL GSIDelay1.53ms,GSIDelay39us,GSIDelay43us,GSIDelay500ms,GSIDelay1Sec
	GLOBAL GSIMakeAscii
	;called from non-user gsi code
	EXTERN GSIUserMain

	#if ENABLE_CHECK_BSR
		GLOBAL DEBUGCheckBSR
		;data
		;GLOBAL	GlbResources 
	#endif	
#else
	EXTERN	GSIDelay1Sec,GSIDelay160us,GSIDelay5ms,GSIDelay50ms
	EXTERN GSIProcessRxByte,GSILCDWriteChar
	EXTERN GSIDelay1.53ms,GSIDelay39us,GSIDelay43us,GSIDelay500ms,GSIDelay1Sec
	EXTERN GSIMakeAscii
	
	EXTERN	GiBank,GlbGSIState		

	#if ENABLE_CHECK_BSR
		EXTERN DEBUGCheckBSR
	#endif	
#endif

#if GSI_DEBUG_SUPPORT
	#ifdef IN_DEBUG.ASM
		GLOBAL GSIDebugInit
		;variables
		GLOBAL DEBUGCauseWDT,DEBUGDelay500,GlbDebugTemp
		;EXTERN GlbResources
	#else
		EXTERN GSIDebugInit
		;variables
		EXTERN DEBUGCauseWDT,DEBUGDelay500,GlbDebugTemp
	#endif	
#endif

#if GSI_LCD_SUPPORT
	#ifdef	IN_LCD.ASM 
		;GLOBAL	LCDInstruction
		GLOBAL	LCDWriteChar,GSICls,GSIClrLine
		GLOBAL	GSIEnd,GSIBeginningOfLine,GSIWriteStr
		GLOBAL	GSISetCursor
		GLOBAL	GSIHome,GSILCDInit,GSISignOn
		GLOBAL	GSILCDInstruction
		
		GLOBAL	GSIDisplayBinary
		GLOBAL	GSIWriteBCD8,GSIWriteBCD8,GSIUserAbort
		GLOBAL	GSILCDWriteAscii
		;DATA
		GLOBAL LdBank
		;EXTERN	GlbResources	;from gsi.asm

	#else ;not IN_LCD_.ASM so declare as EXTERN
	;Not processing the source code so make the interface visible
		;EXTERN	LCDInstruction
		EXTERN	LCDWriteChar,GSICls,GSIClrLine
		EXTERN	GSIEnd,GSIBeginningOfLine,GSIWriteStr
		EXTERN	GSISetCursor
		EXTERN	GSIHome,GSILCDInit,GSISignOn
		EXTERN	GSILCDInstruction
			
		EXTERN	GSIDisplayBinary
		EXTERN	GSIWriteBCD8,GSIWriteBCD8,GSIUserAbort
		EXTERN	GSILCDWriteAscii
		;DATA
		EXTERN LdBank
	#endif ;IN_LCD_.ASM 
#endif  ;GSI_LCD_SUPPORT

#if GSI_RS232_SUPPORT

	#ifdef IN_RS232.ASM
		GLOBAL GSIAddToRS232RxBuff,GSIGetFromRS232RxBuff,GSIPeekRS232RxBuff
		GLOBAL GSISendBreak,GSILookForR2RxChar,GSISendByteWithEcho232
		GLOBAL GSISendByte232,GSIEchoByte232,GSIInitRS232
		GLOBAL GSIWriteMessageRS232,GSISendBinary232,GSISendBinary232ISR
		GLOBAL GSISendAscii232,GSISendBSRAscii
		GLOBAL	GSIFlushR2RxBuffer,GSISetRS232AsciiMode
		GLOBAL	GSIRS232IOCTL,GSIWriteErrorMessageRS232
		IF DEBUG
			GLOBAL	DEBUGGSISendByte232,DEBUGGSISendBinary232,DEBUGGSISendBinary232ISR
		ENDIF
		;-data
		GLOBAL R2Bank
		GLOBAL R2SubfunctionRecord,R2SubfunctionRecordLength
		GLOBAL R2SubfunctionRecordData
		;EXTERN	GlbResources ;from gsi.asm
	#else
		EXTERN GSIAddToRS232RxBuff,GSIGetFromRS232RxBuff,GSIPeekRS232RxBuff
		EXTERN GSISendBreak,GSILookForR2RxChar,GSISendByteWithEcho232
		EXTERN GSISendByte232,GSIEchoByte232,GSIInitRS232
		EXTERN GSIWriteMessageRS232,GSISendBinary232,GSISendBinary232ISR
		EXTERN GSISendAscii232,GSISendBSRAscii
		EXTERN GSIFlushR2RxBuffer,GSISetRS232AsciiMode
		EXTERN GSIRS232IOCTL,GSIWriteErrorMessageRS232
		IF DEBUG
			EXTERN	DEBUGGSISendByte232,DEBUGGSISendBinary232,DEBUGGSISendBinary232ISR
		ENDIF

		;-data
		EXTERN R2Bank
		EXTERN R2SubfunctionRecord,R2SubfunctionRecordLength
		EXTERN R2SubfunctionRecordData

	#endif	

	#ifdef	IN_R2MASTER.ASM
		GLOBAL GSIRS232ReqMaster,RS232TxRS232Record,GSIRS232SendCANMssg
		GLOBAL GSIR2RecordDataPendingTxToRS232
		GLOBAL RS232SendPendingCANMssg
		;data
		;EXTERN	GlbResources ;from gsi.asm

	#else
		EXTERN GSIRS232ReqMaster,RS232TxRS232Record,GSIRS232SendCANMssg
		EXTERN GSIR2RecordDataPendingTxToRS232 ;,GSICANDataPendingTxToRS232
		EXTERN RS232SendPendingCANMssg

	#endif	;IN_R2MASTER.ASM

	#ifdef	IN_R2SLAVE.ASM
		GLOBAL GSIRS232SetSlave
		;data
		;EXTERN	GlbResources ;from gsi.asm

 	#else
		EXTERN GSIRS232SetSlave
	#endif	;IN_R2SLAVE.ASM


	#ifdef	IN_USER232.ASM
		GLOBAL GSIUserR2Record_0,GSIUserR2Record_1,GSIUserR2Record_2
		GLOBAL GSIUserR2Record_3,GSIUserR2Record_4
 	#else
		EXTERN GSIUserR2Record_0,GSIUserR2Record_1,GSIUserR2Record_2
		EXTERN GSIUserR2Record_3,GSIUserR2Record_4
	#endif	;IN_USER232


#endif ;GSI_RS232_SUPPORT


#if GSI_KEYPAD_SUPPORT

	#ifdef IN_KEYPAD.ASM
		GLOBAL GSIInitKeypad,GSIKeypadDebounce
		;data
		GLOBAL KdBank
		;EXTERN	GlbResources ;from gsi.asm
	#else
		EXTERN GSIInitKeypad,GSIKeypadDebounce
		;data
		EXTERN KdBank
	#endif	

#endif ;GSI_KEYPAD_SUPPORT

#ifdef IN_INTERRUPTS.ASM
	#IF GSI_USER_LP_INT_SUPPORT
		EXTERN GSIUserISRFirst,GSIUserISRLast
	#ENDIF
	#IF GSI_USER_LP_INT_SUPPORT | GSI_USER_HP_INT_SUPPORT
		EXTERN GSIUserInitInterrupts
	#ENDIF
	#IF GSI_USER_HP_INT_SUPPORT
		EXTERN GSIUserISRHP
	#ENDIF

	GLOBAL GSIInitInterrupts

	;data
	;EXTERN	GlbResources ;from gsi.asm
	
#else
	EXTERN GSIInitInterrupts
#endif	

#if GSI_I2C_SUPPORT
	#ifdef IN_I2C.ASM
		GLOBAL GSIGetFromI2CTxBuff,GSIAddToI2CTxBuff
		GLOBAL GSIInitI2C,GSIInitI2CVrbls,GSIInitiateI2CTransfer
		;DATA
		GLOBAL GlbI2CTxByte,GlbI2CRxByte,GlbI2CStatus
		GLOBAL I2CBank
		;data
		;EXTERN	GlbResources ;from gsi.asm
	#else
		EXTERN GSIGetFromI2CTxBuff,GSIAddToI2CTxBuff
		EXTERN GSIInitI2C,GSIInitI2CVrbls,GSIInitiateI2CTransfer
		;DATA
		EXTERN GlbI2CTxByte,GlbI2CRxByte,GlbI2CStatus
		EXTERN I2CBank
	#endif	
#endif ;GSI_I2C_SUPPORT

#ifdef IN_ERROR.ASM
	GLOBAL GSIWriteErrorMessage
	GLOBAL GSIInitErrorHandling
	;DATA
	GLOBAL GlbErrState,ErrBank
	;EXTERN	GlbResources ;from gsi.asm
#else
	EXTERN GSIWriteErrorMessage
	EXTERN GSIInitErrorHandling
	;DATA
	EXTERN GlbErrState,ErrBank
#endif	

#if GSI_CAN_SUPPORT

	#ifdef IN_CAN.ASM
		GLOBAL GSITestCAN,GSIInitCAN
		GLOBAL GSICopyCANRxBuffToR2TxRecord
		GLOBAL CnBank
		;data
		;EXTERN	GlbResources ;from gsi.asm
	#else
		EXTERN GSITestCAN,GSIInitCAN
		EXTERN GSICopyCANRxBuffToR2TxRecord
		EXTERN CnBank		
	#endif ;IN_CAN.ASM

	#ifdef IN_CANRX.ASM
		GLOBAL	GSISendCANRxMssg232Ascii,GSICANGetMsgFromCQ
		;data
		;EXTERN	GlbResources ;from gsi.asm
	#else
		EXTERN	GSISendCANRxMssg232Ascii,GSICANGetMsgFromCQ
	#endif	;IN_CANRX.ASM

	#ifdef IN_CANTX.ASM
		GLOBAL GSISendCANRepeat,GSISendCANAck
		GLOBAL GSISendCANReserved_1,GSISendCANReserved_2
		;GLOBAL GSISendCANErrorCode232,GSISendCanFatalError232
		;above are placeholder functions: notimplemented
		GLOBAL GSISendCANRS232Mssg,GSISendCANLCDMssg
		GLOBAL GSISendCANKeypadMssg,GSISendCANSPIMssg,GSISendCANI2CMssg
		GLOBAL GSISendCANDIOMssg,GSISendCANADCMssg
		GLOBAL GSISendCANMssg,GSISendCANRxMssg,GSISendCANError,GSICANSendErrorMssg
		GLOBAL GSISendCANPingMssg
		;GLOBAL SendCANIOCTLMssg,SendCANGSIIOCTLMssg
		;GLOBAL GSISendCANUserMssg

	;entry point for above functions but called from rs232
		;GLOBAL GSISendCANRepeat232,GSISendCANAck232,GSISendCANRS232Mssg232,GSISendCANLCDMssg232
		;GLOBAL GSISendCANKeypadMssg232,GSISendCANSPIMssg232,GSISendCANI2CMssg232
		;GLOBAL GSISendCANDIOMssg232,GSISendCANADCMssg232

		;GLOBAL GSISendCANMssg232,GSISendCANError232

		;GLOBAL GSISendCANIOCTLMssg232,SendCANGSIIOCTLMssg232,GSISendCANMssg232
		;data
		;EXTERN	GlbResources ;from gsi.asm
	#else	
		EXTERN GSISendCANRepeat,GSISendCANAck
		EXTERN GSISendCANReserved_1,GSISendCANReserved_2
		;EXTERN GSISendCANErrorCode232,GSISendCanFatalError232
		;above are placeholder functions: notimplemented
		EXTERN GSISendCANRS232Mssg,GSISendCANLCDMssg
		EXTERN GSISendCANKeypadMssg,GSISendCANSPIMssg,GSISendCANI2CMssg
		EXTERN GSISendCANDIOMssg,GSISendCANADCMssg
		EXTERN GSISendCANRxMssg,GSISendCANError,GSICANSendErrorMssg,GSISendCANMssg
		EXTERN GSISendCANPingMssg
		;entry point for above functions but called from rs232
		;EXTERN GSISendCANRepeat232,GSISendCANAck232,GSISendCANRS232Mssg232,GSISendCANLCDMssg232
		;EXTERN GSISendCANKeypadMssg232,GSISendCANSPIMssg232,GSISendCANI2CMssg232
		;EXTERN GSISendCANDIOMssg232,GSISendCANADCMssg232
		;EXTERN GSISendCANMssg232,GSISendCANError232
		;EXTERN SendCANIOCTLMssg232,SendCANGSIIOCTLMssg232
		;EXTERN GSISendCANUserMssg232
	#endif	;IN_CANTX.ASM

	#ifdef IN_MASTSLAV.ASM
		GLOBAL GSICANSetMaster,GSICANSetSlaveId,GSICANSetSlave,GSICANSetPCMaster
		;GLOBAL GSICANSetSlaveId232,GSICANSetSlave232,GSICANSetPCMaster232
		;GLOBAL GSICANSetMaster232
		;data
		;EXTERN	GlbResources ;from gsi.asm
	#else
		EXTERN GSICANSetMaster,GSICANSetSlaveId,GSICANSetSlave,GSICANSetPCMaster
		;EXTERN GSICANSetSlaveId232,GSICANSetSlave232,GSICANSetPCMaster232
		;EXTERN GSICANSetMaster232
	#endif ;IN_MASTSLAVE.ASM

	#ifdef IN_USERCAN.ASM
		GLOBAL GSIUserCANF0_4,GSIUserCANF0_5,GSIUserCANF0_6
		GLOBAL GSIUserCANF0_7,GSIUserCANF0_8,GSIUserCANF0_9
		GLOBAL GSIUserCANF0_10,GSIUserCANF0_11	
		GLOBAL GSIUserCANF1_0,GSIUserCANF1_1,GSIUserCANF1_2
		GLOBAL GSIUserCANF1_3,GSIUserCANF1_4,GSIUserCANF1_5
		GLOBAL GSIUserCANF1_6,GSIUserCANF1_7,GSIUserCANF1_8
		GLOBAL GSIUserCANF1_9,GSIUserCANF1_10,GSIUserCANF1_11
		GLOBAL GSIUserCANF1_12,GSIUserCANF1_13,GSIUserCANF1_14
		GLOBAL GSIUserCANF1_15
		;data
		;EXTERN	GlbResources ;from gsi.asm
	#else
		EXTERN GSIUserCANF0_4,GSIUserCANF0_5,GSIUserCANF0_6
		EXTERN GSIUserCANF0_7,GSIUserCANF0_8,GSIUserCANF0_9
		EXTERN GSIUserCANF0_10,GSIUserCANF0_11	
		EXTERN GSIUserCANF1_0,GSIUserCANF1_1,GSIUserCANF1_2
		EXTERN GSIUserCANF1_3,GSIUserCANF1_4,GSIUserCANF1_5
		EXTERN GSIUserCANF1_6,GSIUserCANF1_7,GSIUserCANF1_8
		EXTERN GSIUserCANF1_9,GSIUserCANF1_10,GSIUserCANF1_11
		EXTERN GSIUserCANF1_12,GSIUserCANF1_13,GSIUserCANF1_14
		EXTERN GSIUserCANF1_15

	#endif
#endif ;GSI_CAN_SUPPORT


#ifdef IN_INIT.ASM
	GLOBAL GSIReadEEPROM,GSIWriteEEPROM
	GLOBAL GSIGetResources,GSIGetResources1,GSISetResources,GSISetResources1
	EXTERN GSIUserRestart	;User provided function
	;data
	;EXTERN	GlbResources ;from gsi.asm

#else
	EXTERN GSIReadEEPROM,GSIWriteEEPROM
	EXTERN GSIGetResources,GSIGetResources1,GSISetResources,GSISetResources1	
#endif


#endif ;GSI_INC_INCLUDED
