;******************************************************************************
;* @file    keypad.inc part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************


;---------------------------------------
;LCD.INC
;A serial LCD driver for various pics
;---------------------------------------
#ifndef LCD_INC_INCLUDED
#define LCD_INC_INCLUDED

;#if GSI_LCD_SUPPORT

LCD_LINES				EQU		d'2'
LCD_COLS				EQU		d'32'

;---------------------------------------------
;
;		LCD
;
;---------------------------------------------
;LCD Data==portd 4-7 pins 27,28,29,30

#define	LCD_E			PORTE,0,A
#define LCD_RW			PORTE,1,A
#define LCD_RS			PORTE,2,A
#define LCD_DATA_PORT		PORTD

;LCDControl flags
;Note that these are also bit positions for the signal lines
#define LCD_CTR_PORT		PORTE	
LCD_CTR_E		EQU	0
LCD_CTR_RW		EQU	1
LCD_CTR_RS		EQU	2

LCD_E_STROBE	MACRO
	bsf	LCD_CTR_PORT,LCD_CTR_E,A
	;require max of 450ns or 3 instructions @20MHz
        ;5 instructions at 40MHz
        IF CLOCK == d'10'
        nop
        nop
        ENDIF
        IF CLOCK == d'20'
        nop
        nop
        nop
        ENDIF
        IF CLOCK == d'40'
        nop
        nop
        nop
        nop
        nop
        ENDIF
	bcf	LCD_CTR_PORT,LCD_CTR_E,A	
	ENDM

DELAY_160_NS    MACRO
        IF CLOCK == d'10'
        nop                     ;400ns
        ENDIF
        IF CLOCK == d'20'
        nop                     ;200ns
        ENDIF
        IF CLOCK == d'40'
        nop
        nop                     ;200ns
        ENDIF
        IF (CLOCK != d'10') && (CLOCK != d'20') && (CLOCK != d'40')
        ERROR Invalid clock speed
        ENDIF
        ENDM


DELAY_80_NS    MACRO
        IF CLOCK == d'10'
        nop                     ;400ns
        ENDIF
        IF CLOCK == d'20'
        nop                     ;200ns
        ENDIF
        IF CLOCK == d'40'
        nop                     ;100ns
        ENDIF
        IF (CLOCK != d'10') && (CLOCK != d'20') && (CLOCK != d'40')
        ERROR Invalid clock speed
        ENDIF
        ENDM


;-----------------------------------------------
;	Defaults
;----------------------------------------------

;-----------------------------------------------------
;					LCD Commands
;-----------------------------------------------------

FIRST_LCD_COMMAND		EQU		d'0'

GSI_NUL				EQU		d'0'	;implement, do not place NUL in buffer, a 1 char delay
GSI_BOL				EQU		d'1'	;ctr-a beginning of line
GSI_CURSOR_LEFT			EQU		d'2'	;ctr-b Cursor left
GSI_USR_ABRT			EQU		d'3'	;ctr-c User abort, INIT the LCD/KEYPAD
GSI_DEL_FWD			EQU		d'4'	;ctr-d delete char forward
GSI_EOL				EQU		d'5'	;ctr-e end of line
GSI_CURSOR_RIGHT		EQU		d'6'	;ctr-f 
GSI_R2_REQ_MASTER		EQU		d'7'	;ctr-g Sender Requests being rs232 master
GSI_BS				EQU		d'8'	;ctr-h BS delete
GSI_HTAB			EQU		d'9'	;ctr-i 
GSI_LF				EQU		d'10'	;ctr-j alias for CursorDown
GSI_CTR_K			EQU		d'11'	;ctr-k 
GSI_FF				EQU		d'12'	;ctr-l 
GSI_CLS				EQU		d'12'	;ctr-l 
GSI_CR				EQU		d'13'	;ctr-m 
GSI_CURSOR_DOWN			EQU		d'14'	;ctr-n 
GSI_TOGGLE_ECHO			EQU		d'15'	;ctr-o 
GSI_CURSOR_UPL			EQU		d'16'	;ctr-p
GSI_XON				EQU		d'17'	;ctr-q
GSI_BINARY_MODE			EQU		d'18'	;ctr-r (down = CR)
GSI_XOFF			EQU		d'19'	;ctr-s
GSI_CURSOR_RIGHT_5		EQU		d'20'	;ctr-t 
GSI_ASCII_MODE			EQU		d'21'	;ctr-u 
GSI_CURSOR_LEFT_5		EQU		d'22'	;ctr-v
GSI_CURSOR_LEFT_10		EQU		d'23'	;ctr-w
GSI_KEYPAD_TOGGLE_STATE		EQU		d'24'	;ctr-x
GSI_KEYPAD_TOGGLE_MODE		EQU		d'25'	;ctr-y
GSI_CTR_Z			EQU		d'26'	;ctr-z

LAST_LCD_COMMAND		EQU		d'26'	;

FIRST_ESC_COMMAND		EQU		d'0'
GSI_ESC_VERSION_STRING		EQU		d'1'
GSI_ESC_CAPABILITIES		EQU		d'2'
GSI_ESC_GET_BUFFER		EQU		d'3'
GSI_ESC_SET_BUFFER		EQU		d'4'
GSI_ESC_CLS			EQU		d'5'
GSI_ESC_LCD_INSTRUCTION		EQU		d'6'
GSI_ESC_GET_CUR_TO_EOL		EQU		d'7'
GSI_ESC_GET_LINE		EQU		d'8'
GSI_ESC_SET_LINE		EQU		d'9'
GSI_ESC_HOME			EQU		d'10'
GSI_ESC_FLOW_CONTROL_ON		EQU		d'11'
GSI_ESC_FLOW_CONTROL_OFF	EQU		d'12'
GSI_ESC_INIT			EQU		d'13'
GSI_ESC_GET_BUTTON_STATE	EQU		d'14'
GSI_ESC_CAN_SET_MASTER		EQU		d'15'
GSI_ESC_CAN_SET_SLAVE		EQU		d'16'
LAST_ESC_COMMAND		EQU		d'16'

	IF 0
PACKET:
ID
LEN 
DATA
CRC
	ENDIF
;--------------------------------------------
;		Macros
;--------------------------------------------
PREDKO_CLOCK_STROBE MACRO		;  Strobe the Data Bit
	bsf	 LCD_CLOCK
	nop
	nop
	nop
	nop
	nop
	bcf	 LCD_CLOCK
 ENDM

PREDKO_E_STROBE MACRO			;  Strobe the "E" Bit
	bsf	LCD_DATA
	nop
	nop
	nop
	nop
	nop
	bcf	 LCD_DATA
 ENDM


LCD_LINE2_ADDRESS		EQU	0x40


LCD_CLS	MACRO
	IF (SIMULATOR == FALSE)
	movlw	0x001			;  Clear the Display RAM
	call	GSILCDInstruction
	call	GSIDelay5ms		;  Note, Can take up to 4.1 msecs
	ENDIF
	ENDM

LCD_CURSOR_LEFT	MACRO
	IF (SIMULATOR == FALSE)
	movlw	b'00010000'
	call	GSILCDInstruction
	ENDIF
	ENDM

LCD_CURSOR_RIGHT	MACRO
	IF (SIMULATOR == FALSE)
	movlw	b'00010100'
	call	GSILCDInstruction
	ENDIF
	ENDM

LCD_DDRAM_ADDRESS	MACRO 
	IF(SIMULATOR == FALSE)
	iorlw	0x80
	call	GSILCDInstruction
	ENDIF
	ENDM

LCD_HOME	MACRO
	IF (SIMULATOR == FALSE)
	movlw	2
	call	GSILCDInstruction
	call	GSIDelay1.53ms
	ENDIF
	ENDM

LCD_WRITE_CHAR_L1	MACRO	literal
	IF (SIMULATOR == FALSE)
	movlw	0x0
	LCD_DDRAM_ADDRESS
	movlw	literal
	call	LCDWriteChar
	ENDIF
	ENDM

LCD_WRITE_CHAR_L2	MACRO	literal
	IF (SIMULATOR == FALSE)
	movlw	0x40
	LCD_DDRAM_ADDRESS
	movlw	literal
	call	LCDWriteChar
	ENDIF
	ENDM

LCD_WRITE_CHAR_DEBUG	MACRO
	IF (SIMULATOR == FALSE)
	movlw	0x40
	LCD_DDRAM_ADDRESS
	movf	LCDDebug,W
	call	LCDWriteChar
	ENDIF
	ENDM

LCD_CURSOR_OFF	MACRO

	IF (SIMULATOR == FALSE)
	movlw	b'00001100'
	call	GSILCDInstruction
	call	GSIDelay160us
	ENDIF
	ENDM

LCD_CURSOR_ON	MACRO

	IF (SIMULATOR == FALSE)
	movlw	b'00001110'
	call	GSILCDInstruction
	call	GSIDelay160us
	ENDIF
	ENDM

SLCT_LCD	MACRO	N

	IF 0

	IF LCD_CODE
;WARNING! Do not make this macro more than one line
;as it might be used immediately after btfss functions
;the macro will not work if it is >1 line!
;requires BANK0
	IF N == 1
	bcf	LCD_SLCT_BIT
	ENDIF	;N=1

	IF N ==0
	bsf	LCD_SLCT_BIT
	ENDIF	;N=0
	IF N > 1
	ERROR
	ENDIF	;N>1
	ENDIF ;LCD_CODE
	ENDIF	;IF 0
	ENDM

;#ENDIF ;GSI_LCD_SUPPORT

#endif ;LCD_INC_INCLUDED
