;******************************************************************************
;* @file    debug.inc part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;debug stuff for pi

#ifndef DEBUG_INC_INCLUDED

#define DEBUG_INC_INCLUDED 
#define 			_18F4680	1
#define				_18F458  	2
#define				DEVICE _18F458

DEBUG				EQU	TRUE
ICD2				EQU	TRUE                ;can also be pickit3

#if ICD2
SIMULATOR			EQU	FALSE
#else
SIMULATOR           EQU TRUE
#endif

STARTLE				EQU	FALSE

#IF STARTLE
ENABLE_BREAK		EQU	TRUE
ENABLE_CAN			EQU	FALSE
STARTLE_V2			EQU	TRUE		;version 2 of startle box
#ELSE
ENABLE_BREAK		EQU TRUE
ENABLE_CAN			EQU	FALSE
#ENDIF


TEST_RX_CODE			EQU	FALSE
PC_CONTROL				EQU	FALSE
EEPROM_RESTORE_STATE_ENABLED	EQU	FALSE
SEPARATE_UI_CODE		EQU	FALSE
DEBUG_SOUND				EQU	FALSE
DEBUG_BUTTON_PRESS		EQU	FALSE
DEBUG_MENUS				EQU	FALSE
USART_ENABLED			EQU	TRUE
RX_INTERRUPT			EQU	TRUE
TX_INTERRUPT			EQU	FALSE
INTERRUPTS				EQU	TRUE
TEST_LCD				EQU	FALSE
ENABLE_CHECK_BSR		EQU	FALSE
TRACE					EQU	FALSE
PORT_CHANGE_INT			EQU	FALSE
ENABLE_CAN_FILTER_0_TO_RS232	EQU	FALSE		;ENABLE/DISABLE SYNC, ERROR, etc to rs232 for debugging only
DISABLE_CN_SYNC_RX		EQU		TRUE			;don't receive Syncs (outside of setting slave)
DEBUG_CAN				EQU	FALSE
CAN_LOOPBACK			EQU	FALSE
ENABLE_CALLER_NODE_SENDBACK	EQU TRUE
DEBUG_RECORD_LENGTH_TEST	EQU	TRUE
;simulator menus are usually used together
;simulator is a must with ui code as timing delays are
;enormous in the simulator


	IF DEBUG
		
	ENDIF

	IF SIMULATOR
	IFDEF PRINT_MESSAGES	;defined on command line for 1 source listing only
		MESSG "Simulator version. No rs232, Timing loops reduced etc."	
	ENDIF
	ENDIF

	IF TRACE
		MESSG "TRACE version."	
	ENDIF

	IF PC_CONTROL
		MESSG "PC control version."	
	ENDIF

	IF DEBUG_MENUS	
		
	ENDIF
;GLOBAL_DEBUG_FLAG
DEBUG_CALL_DEBUG_CODE	EQU	0	;when set main calls DebugCurrentCode
DEBUG_CALL_DEBUG_CODE_1	EQU	1
DEBUG_CALL_DEBUG_CODE_2	EQU	2
DEBUG_CALL_DEBUG_CODE_3	EQU	3

FIRST_PC_COMMAND	EQU	'A'
LAST_PC_COMMAND		EQU	'Q'
RX_PC_COMMAND		EQU	1
RX_DATA			EQU	2
RX_AWAITING_DATA	EQU	3

IN_START		EQU	'0'
IN_INITPORTA		EQU	'1'
IN_INITPORTB		EQU	'2'
IN_INITOPTIONREG	EQU	'3'
IN_INITTMR1		EQU	'4'
IN_INITINTS		EQU	'5'
IN_IDENTIFYINTS		EQU	'6'
IN_TMR0INTTAKEN		EQU	'7'
IN_RB0INTTAKEN		EQU	'8'
IN_PORTCHANGEINTTAKEN	EQU	'9'
IN_TXBUFFEMPTYINTTAKEN	EQU	'a'
IN_RXINTTAKEN		EQU	'b'
IN_FRAMEERROR		EQU	'c'
IN_OVERRUNERROR		EQU	'd'
IN_ISR			EQU	'e'
IN_INRB0INT		EQU	'f'
IN_INPORTCHANGEINT	EQU	'g'
IN_INTMR0INT		EQU	'h'
IN_INTMR1INT		EQU	'i'
IN_INTXBUFFEMPTYINT	EQU	'j'
IN_INRXINT		EQU	'k'
IN_INITVRBLS		EQU	'l'
IN_INITUSART		EQU	'm'
IN_SETPULSERATE		EQU	'n'
IN_TX_BINARY232		EQU	'o'
IN_MAIN_LOOP		EQU	'p'
IN_TMR1INTTAKEN		EQU	'q'
IN_WDT			EQU	'r'

;IN_INTIMER1INT		EQU	'r'
;IN_INTIMER1INT		EQU	's'

	IF 0
	IF (DEBUG && SIMULATOR == FALSE && TRACE)
DEBUG_TX_BYTE_LITERAL	MACRO	byte
	LOCAL dtb1
	BANK1
dtb1	
	btfss	TXSTA,TRMT
	goto	dtb1		;loop till TSR is empty
	BANK0
	movlw	byte
	movwf	TXREG
		
	ENDM

	ELSE
DEBUG_TX_BYTE_LITERAL	MACRO byte

	ENDM

	ENDIF

	IF DEBUG 	
DEBUG_TX_BYTE_REG MACRO	reg
	LOCAL dtb1
	BANK1
dtb1	
	btfss	TXSTA,TRMT
	goto	dtb1		;loop till TSR is empty
	BANK0
	movf	reg,W
	movwf	TXREG
		
	ENDM

	ELSE
DEBUG_TX_BYTE_REG MACRO	reg
	ENDM
	ENDIF

	IF DEBUG 	
DEBUG_TX_BYTE_W MACRO	
	LOCAL dtb1
	BANK1
dtb1	
	btfss	TXSTA,TRMT
	goto	dtb1		;loop till TSR is empty
	BANK0
	movwf	TXREG
	ENDM

	ELSE
DEBUG_TX_BYTE_W MACRO	
	ENDM
	ENDIF
	ENDIF

LED_ON	MACRO	led
	IF led >1 
	#error
	ENDIF
	IF led == 0
	bsf	LED_0
	ELSE
	bsf	LED_1
	IF led >1 
	#error
	ENDIF
	IF led == 0
	bsf	LED_0
	ENDIF
	ENDM

LED_OFF	MACRO	led
	IF led >1 
	#error
	ENDIF
	IF led == 0
	bcf	LED_0
	ELSE
	bcf	LED_1
	ENDIF
	ENDM

LED_TOGGLE MACRO led
	IF led >1
	#error
	ENDIF
	movlw	1 << led		;turn bit into hex value
	xorwf	LED_PORT,F,A
	ENDM

TEST_ADD_TO_RX	MACRO	x
	movlw	x
	movwf	MnRxByte
	call	AddToRxBuff
	ENDM

MAIN	EQU	0
UI	EQU	3

CHECK_BSR	MACRO b,loc
	IF ENABLE_CHECK_BSR
	movwf	GlbDebugTempH	;save WREG

	movlw	loc
	movwf	GlbDebugTempL	;location

	movlw	b
	call	DEBUGCheckBSR

	movf	GlbDebugTempH,W

	ENDIF
	ENDM

DEBUG_LCD_WRITE_CHAR MACRO literal
	#IF DEBUG
	#IF GSI_LCD_SUPPORT
		movlw	literal
		call	GSILCDWriteChar
	#ENDIF
	#ENDIF
	ENDM


#endif ;DEBUG_INC_INCLUDED
