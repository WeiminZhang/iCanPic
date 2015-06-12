;******************************************************************************
;* @file    defs.inc part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;Definitions
#ifndef DEFS_INC_INCLUDED
#define DEFS_INC_INCLUDED

	RADIX	DEC

;#include "equ.inc"
;#include "gsmacros.inc"

P18FXXX		EQU		TRUE

	IFDEF P18FXXX
	
UI_CODE_START			EQU	1800h
	IF SEPARATE_UI_CODE
#define UI_PAGE				PAGE3
	ELSE
#define UI_PAGE				PAGE0
	ENDIF	;SEPARATE_UI_CODE

#define	MAIN_BANK			BANK0
;#define UI_BANK				BANK3
#define GSI_BANK			BANK0
#define CAN_BANK			BANK1

;UI_DATA_BLOCK			EQU	300h
GSI_DATA_BLOCK			EQU	300h
MAIN_DATA_BLOCK			EQU	d'96'	;step over access bank
ACCESS_DATA_BLOCK		EQU	0

	ENDIF  ;P18FXXX


;----------------------------------
;       Global
;---------------------------------

#define NONE		0
BOARD	SET 		NONE

#define TRAINING_BOARD	1
#define ET_BOARD	    2

BOARD	SET 		TRAINING_BOARD



CLOCK				EQU	d'20'		;20MHz
PLL					EQU	FALSE		;means that baud rate gen is a 10MHz clock


#IF PLL == TRUE
BAUD_CLOCK			EQU	CLOCK/4
#ELSE
BAUD_CLOCK			EQU	CLOCK
#ENDIF


VALID_CLOCK			SET FALSE

FOSC				EQU	CLOCK * d'1000000' ;define FOSC
;Period of T state 
PERIOD_INSTRUCTION		EQU	(1/CLOCK)*4	;Instruction period

TMR3_LOAD_LATENCY		EQU	22		;22 instructions before TMR3 loaded
#if CLOCK == 10
	MESSG "Clock=10MHz"
COUNT_100_MSEC			EQU	(d'65536'-d'62500'+ TMR3_LOAD_LATENCY)	;Use prescalar of 4
VALID_CLOCK			SET	TRUE
#endif

#if CLOCK == 20
	MESSG "Clock=20MHz"
COUNT_100_MSEC			EQU	(d'65536'-d'62500'+ TMR3_LOAD_LATENCY)
VALID_CLOCK			SET	TRUE
#endif

#if CLOCK == 40
	MESSG "Clock=40MHz"
COUNT_100_MSEC			EQU	(d'65536'-d'31250'+ TMR3_LOAD_LATENCY)
VALID_CLOCK			SET	TRUE
#endif

#IF VALID_CLOCK ==FALSE
	MESSG No valid clock set!
	ERROR
#ENDIF


;------------------------------------------------
;	Message strs
;------------------------------------------------

VERSION_MSSG			EQU	d'0' 
WDT_RESET_MSSG			EQU	d'1' 		;Reset messages EQU are
BROWNOUT_RESET_MSSG		EQU	d'2' 		;also used for returning
UNKNOWN_RESET_MSSG		EQU	d'3' 		;id to pc.
USER_ABRT_RESET_MSSG		EQU	d'4' 	;On reset we send an RS232
POWER_ON_RESET_MSSG		EQU	d'5' 		;BREAK followed by one of
STACK_OVF_RESET_MSSG		EQU	d'6'	;these id's
STACK_UNF_RESET_MSSG		EQU	d'7'
ERR_I2C_NO_ACK_FROM_SLAVE_MSSG	EQU	d'8'
ERR_I2C_NO_DATA_MSSG		EQU	d'9'
ERR_I2C_BUS_COLL_MSSG		EQU	d'10'
UNKNOWN_ERROR_MSSG		EQU	d'11'
CAN_EWARN_MSSG			EQU	d'12'
CAN_RXWARN_MSSG			EQU	d'13'
CAN_TXWARN_MSSG			EQU	d'14'
CAN_RXBP_MSSG			EQU	d'15'
CAN_TXBP_MSSG			EQU	d'16'
CAN_TXBO_MSSG			EQU	d'17'
CAN_RXB1OVFL_MSSG		EQU	d'18'
CAN_RXB0OVFL_MSSG		EQU	d'19'
CAN_IRXIF_MSSG			EQU	d'20'
CAN_ERROR_INIT_MSSG		EQU	d'21'
UNKNOWN_CAN_ERROR_MSSG		EQU	d'22'
CAN_MSSG_STR			EQU	d'23' ;23 About to write a CAN mssg
CAN_MSSG_ID_STR			EQU	d'24' ;24 mssg Id str
CAN_MSSG_DATA_STR		EQU	d'25' ;25 mssg Data str
CAN_MSSG_LENGTH_STR		EQU	d'26' ;26 mssg Data Length str
CAN_MSSG_FLAG_STR		EQU	d'27' ;27 mssg Flag str
CAN_MSSG_SET_SLAVE_ID_STR	EQU	d'28' ;28 mssg Set id of slave
R2_ERROR_ECHO_TX_STR		EQU	d'29'
CAN_RX_CQ_OVF_MSSG		EQU	d'30'
CAN_NODE_THIS_STR		EQU d'31'

;-----------------------------------
;               LED
;-----------------------------------

#IF BOARD != TRAINING_BOARD
#define LED_PORT		PORTC
#define LED_0			PORTC,0,A
#define LED_1			PORTC,1,A

#ENDIF

;-------------------------------------------------
;
;		Keypad
;
;-------------------------------------------------

;See keypad.inc


;---------------------------------------------
;	Tmr1
;----------------------------------------------
;TMR1IE is bit 0 which collides with RBIF
;Used by IntType to identify interrupt (See IntType)
TMR1TYPE	EQU	7	;IntType, 7 is GIE or EEIF 


#endif	;DEFS_INC_INCLUDED
