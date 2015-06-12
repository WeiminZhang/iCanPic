;******************************************************************************
;* @file    keypad.inc part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;keypad.inc
#ifndef KEYPAD_INC_INCLUDED
#define KEYPAD_INC_INCLUDED

KEY_DEBOUNCE_TICKS		EQU	d'6'	; * 3.2768ms of stability required

KD_MODE_COOKED			EQU	0
KD_MODE_RAW			EQU	1

;KeypadStatus
KEY_DEBOUNCING			EQU	0
KEY_RAW				EQU	1	;1=raw,0=cooked(key up only)
KEY_ENABLED			EQU	2
KEY_KEY_DOWN			EQU	3	;A key is depressed, poll the debounce function

#define	ROW_PORT		PORTD
#define ROW_TRIS		TRISD

#define COL_PORT		PORTA

#define ROW_0			PORTD,0,A	;outputs
#define ROW_1			PORTD,1,A
#define ROW_2			PORTD,2,A
#define ROW_3			PORTD,3,A

#define ROW_0_TRIS		TRISD,0,A	
#define ROW_1_TRIS		TRISD,1,A
#define ROW_2_TRIS		TRISD,2,A
#define ROW_3_TRIS		TRISD,3,A

#define COL_0			PORTA,2,A	;inputs
#define COL_1			PORTA,4,A
#define COL_2			PORTA,5,A

#define COL_0_TRIS		TRISA,2,A
#define COL_1_TRIS		TRISA,4,A
#define COL_2_TRIS		TRISA,5,A



;KeypadDebounce
KEY_ROW_0			EQU	0	
KEY_ROW_1			EQU	1
KEY_ROW_2			EQU	2
KEY_ROW_3			EQU	3
KEY_COL_0			EQU	4
KEY_COL_1			EQU	5
KEY_COL_2			EQU	6
KEY_COL_3			EQU	7

;KeypadKeyDown1

KEY_D1_0			EQU	0
KEY_D1_1			EQU	1
KEY_D1_2			EQU	2
KEY_D1_3			EQU	3
KEY_D1_4			EQU	4
KEY_D1_5			EQU	5
KEY_D1_6			EQU	6
KEY_D1_7			EQU	7

;KeypadKeyDown2		
KEY_D2_8			EQU	0
KEY_D2_9			EQU	1
KEY_D2_HASH			EQU	2
KEY_D2_ASTERISK			EQU	3

	IF PORT_CHANGE_INT
RBIE_FLAG			EQU	8	;8 other valid value==8 if port change ints enabled
	ELSE
RBIE_FLAG			EQU	0	;8 other valid value==8 if port change ints enabled
	ENDIF	;PORT_CHANGE_INT


#endif; KEYPAD_INC_INCLUDED
