;******************************************************************************
;* @file    keypad.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

#define IN_KEYPAD.ASM
;keypad.asm

;#include <G:\prog\MPLAB660\MCHIP_Tools\p18f458.inc>
;#include "p18xxx8.inc"
#include "GSI.inc"

;#IF DEVICE == _18F458
;#include "p18xxx8.inc"
;#ELSE
;#INCLUDE "P18F4680.inc"
;#ENDIF


#include "keypad.inc"
#include "can.inc"
#include "rs232.inc"
	#IF GSI_CAN_SUPPORT
#include "CANPrTx.inc"
	#ENDIF
    GLOBAL GSIInitKeypad
	GLOBAL GlbKeypadStatus,GlbKeypadKeyDown1,GlbKeypadKeyDown2
	;required init.asm to see if CAN id setup is required
	GLOBAL	SetTempButtonState,KdTempKeypadKeyDown1

;--------------------------------------
;	definitions in GSInterface.asm
;--------------------------------------
	EXTERN	GlbLocalTempL,GlbLocalTempH
#if DEBUG
	EXTERN GlbDebugFlag
#endif
;In can.asm
	#IF GSI_CAN_SUPPORT
	EXTERN	CnMode	
	#ENDIF
;-----------------------------------------
;	Definitions in TestGSI.asm
;-----------------------------------------
	EXTERN TestI2C
;-----------------------------------------
;	Definitions in can.asm
;-----------------------------------------
	#IF GSI_CAN_SUPPORT
	EXTERN CnTxBuff,CnThisNode
	#ENDIF
;-----------------------------------------

KEYPAD_DATA	UDATA
KdBank				RES 0
KdPrevKeypadKeyDown1		RES 1
KdPrevKeypadKeyDown2		RES 1
KdTempKeypadKeyDown1		RES 1
KdTempKeypadKeyDown2		RES 1
KdKeypadDebounceCounter		RES 1
KdCurrentKey			RES 1
KdTemp				RES 1

KEYPAD_GLOBAL_DATA	UDATA_ACS

GlbKeypadStatus		RES 1
GlbKeypadKeyDown1	RES 1
GlbKeypadKeyDown2	RES 1

;---------------------------
KEYPAD	CODE
;---------------------------
;	InitKeypad
;Assumes MAIN_BANK
;---------------------------
GSIInitKeypad
	;vrbls
	clrf	GlbKeypadKeyDown1
	clrf	GlbKeypadKeyDown2
	clrf	KdPrevKeypadKeyDown1
	clrf	KdPrevKeypadKeyDown2
	clrf	KdTempKeypadKeyDown1
	clrf	KdTempKeypadKeyDown2
	clrf	GlbKeypadStatus

	bcf	GlbKeypadStatus,KEY_RAW
	bsf	GlbKeypadStatus,KEY_ENABLED
	call	SetTempButtonState
	return

;-----------------------------------------------------------------
;	GSIKeypadDebounce
;If a key is currently down we need to keep polling this
;function because once the debounce interval is over if a
;2nd key in the same row (same portb line) is pressed, we
;will not receive an interrupt as the port hasn't changed
;Thus when any key is down we should poll this function
;Use Key
;Alternative strategy:
;disable rbif
;poll on the timer tick, if state has changed, set
;debounce flag, and poll in main loop till key debounced
;currently this would poll every 3ms
;Note that TMR0 will be 0 when called from InTmr0Int
;This function can be called from the InTmr0Int
;-----------------------------------------------------------------

GSIKeypadDebounce
	call	CompareButtonState
	btfss	STATUS,Z,A		;
	call	InitKeypadDebounce	;changed so re-initialise
	;make current debounce state == newly read state
	movf	KdTempKeypadKeyDown2,W
	movwf	GlbKeypadKeyDown2
	movf	KdTempKeypadKeyDown1,W
	movwf	GlbKeypadKeyDown1
	;Set key down flag if reqd
	bcf	GlbKeypadStatus,KEY_KEY_DOWN
	movf	GlbKeypadKeyDown1,W
	iorwf	GlbKeypadKeyDown2,W
	btfss	STATUS,Z,A
	bsf	GlbKeypadStatus,KEY_KEY_DOWN
	;
	movf	TMR0L,W,A		;read clock (8 bit)
	subwf	KdKeypadDebounceCounter,W	;have we waited > DEBOUNCE_TICKS?
	btfss	STATUS,Z,A		;taken when TMR0 == KDC
	return
	call	SendKeypadByte		;Yes, send the info
	bcf	GlbKeypadStatus,KEY_DEBOUNCING
	return

;------------------------------------------
;	CompareButtonState 
;Return Z if no change NZ if change
;rb4->r0 (6) *,0,#
;rb5->r1 (4) 7,8,9
;rb6->r2 (9) 4,5,6
;rb7->r3 (8) 1,2,3

;ra2->c0 (3) 1,4,7,*
;ra3->c1 (5) 2,5,8,0
;ra4->c2 (7) 3,6,9,#
;------------------------------------------
CompareButtonState

	call SetTempButtonState		;set TempButtonState
	;Test the current with prev button state
	movf	KdTempKeypadKeyDown1,W
	subwf	GlbKeypadKeyDown1,W
	btfss	STATUS,Z,A
	return				;different, still debouncing
	movf	KdTempKeypadKeyDown2,W
	subwf	GlbKeypadKeyDown2,W
	return

;--------------------------------
;SetTempButtonState
;--------------------------------
SetTempButtonState
;KdKeypadDebounceState
;tristate outputs when not in use. clr=output set=input (tristate)
;IMPORTANT:
;remember that when used as inputs any input to any bit on
;the port will latch the current value on the port. In this case
;as we are tied to PORTA 2,4,5 with pullup resistors the inputs will be
;in the high state. We must explicitly set the outputs before
;setting to an output with the TRIS reg
;Does a timer int cause a problem? Could it cause a read of
;ports before we are done?
;#define ROW_0			PORTD,0,A	;outputs
;#define ROW_1			PORTD,1,A
;#define ROW_2			PORTD,2,A
;#define ROW_3			PORTD,3,A

;#define COL_0			PORTA,2,A	;inputs
;#define COL_1			PORTA,4,A
;#define COL_2			PORTA,5,A


	;bcf	INTCON,T0IE,A	;disable TMR0

	clrf	KdTempKeypadKeyDown1
	clrf	KdTempKeypadKeyDown2

	bcf	ROW_0			;zero output
	bcf	ROW_0_TRIS		;enable output
	;Read state of ROW_0. ROW_0 is low (*,0,#)
	btfss	COL_0			;r0,c0 = '*'
	bsf	KdTempKeypadKeyDown2,KEY_D2_ASTERISK
	btfss	COL_1	
	bsf	KdTempKeypadKeyDown1,KEY_D1_0
	btfss	COL_2	
	bsf	KdTempKeypadKeyDown2,KEY_D2_HASH
	;finished ROW_0
	bsf	ROW_0_TRIS		;tristate ROW_0

	;sample ROW_1
	bcf	ROW_1			;ROW_1 Low
	bcf	ROW_1_TRIS		;enable output latch of ROW_1
	;Read state of ROW_1. ROW_1 is low (7,8,9)
	btfss	COL_0			;r1,c0 = '7'
	bsf	KdTempKeypadKeyDown1,KEY_D1_7
	btfss	COL_1			;r1,c1 = '8'
	bsf	KdTempKeypadKeyDown2,KEY_D2_8
	btfss	COL_2			;r1,c2 = '9'
	bsf	KdTempKeypadKeyDown2,KEY_D2_9
	;finished ROW_1
	bsf	ROW_1_TRIS		;tristate ROW_1

	;sample ROW_2
	bcf	ROW_2			;ROW_2 Low
	bcf	ROW_2_TRIS		;enable output latch of ROW_2
	;Read state of ROW_2. ROW_2 is low (4,5,6)
	btfss	COL_0			;r2,c0 = '4'
	bsf	KdTempKeypadKeyDown1,KEY_D1_4
	btfss	COL_1			;r2,c1 = '5'
	bsf	KdTempKeypadKeyDown1,KEY_D1_5
	btfss	COL_2			;r2,c2 = '6'
	bsf	KdTempKeypadKeyDown1,KEY_D1_6
	;finished ROW_2
	bsf	ROW_2_TRIS		;tristate ROW_2

	;sample ROW_3
	bcf	ROW_3			;ROW_3 Low
	bcf	ROW_3_TRIS		;enable output latch of ROW_3
	;Read state of ROW_2. ROW_2 is low (1,2,3)
	btfss	COL_0			;r3,c0 = '1'
	bsf	KdTempKeypadKeyDown1,KEY_D1_1
	btfss	COL_1			;r3,c1 = '2'
	bsf	KdTempKeypadKeyDown1,KEY_D1_2
	btfss	COL_2			;r3,c2 = '3'
	bsf	KdTempKeypadKeyDown1,KEY_D1_3

	;0V -> COLs so we can pick up presses again
	bsf	ROW_3_TRIS		;all tristated

	;bsf	INTCON,T0IE,A	;re-enable tmr0
	return


;-----------------------------------------------------
;	SendKeypadByte
;Rx the key
;echo to lcd if reqd
;If KEYPAD_MODE==RAW then send back all up/down keys
;-----------------------------------------------------
SendKeypadByte

	btfss	GlbKeypadStatus,KEY_RAW
	goto	skb_cooked
	;-send all key changes
	movf	KdPrevKeypadKeyDown1,W
	xorwf	GlbKeypadKeyDown1,W
	;set bits that have changed
	movwf	GlbLocalTempL
	;first 8 keys	
	movlw	'0'
	btfsc	GlbLocalTempL,KEY_D1_0
	call	TxKeypadByte
	movlw	'1'
	btfsc	GlbLocalTempL,KEY_D1_1
	call	TxKeypadByte
	movlw	'2'
	btfsc	GlbLocalTempL,KEY_D1_2
	call	TxKeypadByte
	movlw	'3'
	btfsc	GlbLocalTempL,KEY_D1_3
	call	TxKeypadByte
	movlw	'4'
	btfsc	GlbLocalTempL,KEY_D1_4
	call	TxKeypadByte
	movlw	'5'
	btfsc	GlbLocalTempL,KEY_D1_5
	call	TxKeypadByte
	movlw	'6'
	btfsc	GlbLocalTempL,KEY_D1_6
	call	TxKeypadByte
	movlw	'7'
	btfsc	GlbLocalTempL,KEY_D1_7
	call	TxKeypadByte
	;now last 4 keys
	movf	KdPrevKeypadKeyDown2,W
	xorwf	GlbKeypadKeyDown2,W
	;set bits that have changed
	movwf	GlbLocalTempH
	movlw	'8'
	btfsc	GlbLocalTempH,KEY_D2_8
	call	TxKeypadByte
	movlw	'9'
	btfsc	GlbLocalTempH,KEY_D2_9
	call	TxKeypadByte
	movlw	'#'
	btfsc	GlbLocalTempH,KEY_D2_HASH
	call	TxKeypadByte
	movlw	'*'
	btfsc	GlbLocalTempH,KEY_D2_ASTERISK
	call	TxKeypadByte
skb_exit
	movf	GlbKeypadKeyDown1,W
	movwf	KdPrevKeypadKeyDown1
	movf	GlbKeypadKeyDown2,W
	movwf	KdPrevKeypadKeyDown2
	return

skb_cooked
	movf	KdPrevKeypadKeyDown1,W
	xorwf	GlbKeypadKeyDown1,W
	;set bits that have changed
	movwf	GlbLocalTempL

	btfss	GlbLocalTempL,KEY_D1_0
	goto	skb1
	;'0' has been released
	movlw	'0'
	btfss	GlbKeypadKeyDown1,KEY_D1_0
	call	TxKeypadByte		;key is up so must have been released
	IF DEBUG
		btfsc	GlbKeypadKeyDown1,KEY_D1_0
		bra	_no_clr
		bcf	LATC,0
		bcf	LATC,1
_no_clr 
		
	ENDIF
	#IF GSI_CAN_SUPPORT
		btfss	GlbKeypadKeyDown1,KEY_D1_0
		bsf	GlbDebugFlag,DEBUG_CALL_DEBUG_CODE_2
		;call	TestI2C
	#ENDIF
skb1
	btfss	GlbLocalTempL,KEY_D1_1
	goto	skb2
	;'1' has been released
	movlw	'1'
	btfss	GlbKeypadKeyDown1,KEY_D1_1
	call	TxKeypadByte
	IF DEBUG
		;btfss	GlbKeypadKeyDown1,KEY_D1_1
		;btg	LATC,1
	ENDIF

skb2
	btfss	GlbLocalTempL,KEY_D1_2
	goto	skb3
	;'2' has been released
	movlw	'2'
	btfss	GlbKeypadKeyDown1,KEY_D1_2
	call	TxKeypadByte
skb3

	btfss	GlbLocalTempL,KEY_D1_3
	goto	skb4
	;'3' has been released
	movlw	'3'
	btfss	GlbKeypadKeyDown1,KEY_D1_3
	call	TxKeypadByte
skb4
	
	btfss	GlbLocalTempL,KEY_D1_4
	goto	skb5
	;'4' has been released
	movlw	'4'
	btfss	GlbKeypadKeyDown1,KEY_D1_4
	call	TxKeypadByte
skb5

	btfss	GlbLocalTempL,KEY_D1_5
	goto	skb6
	;'5' has been released
	movlw	'5'
	btfss	GlbKeypadKeyDown1,KEY_D1_5
	call	TxKeypadByte
skb6

	btfss	GlbLocalTempL,KEY_D1_6
	goto	skb7
	;'6' has been released
	movlw	'6'
	btfss	GlbKeypadKeyDown1,KEY_D1_6
	call	TxKeypadByte
skb7

	btfss	GlbLocalTempL,KEY_D1_7
	goto	skb8
	;'7' has been released
	movlw	'7'
	btfss	GlbKeypadKeyDown1,KEY_D1_7
	call	TxKeypadByte
skb8
	movf	KdPrevKeypadKeyDown2,W
	xorwf	GlbKeypadKeyDown2,W
	;set bits that have changed
	movwf	GlbLocalTempH

	btfss	GlbLocalTempH,KEY_D2_8
	goto	skb9
	;'8' has been released
	movlw	'8'
	btfss	GlbKeypadKeyDown2,KEY_D2_8
	call	TxKeypadByte
skb9
	btfss	GlbLocalTempH,KEY_D2_9
	goto	skb10
	;'9' has been released
	IF DEBUG
		;movf	GlbSIOStatus1,W
		;call	GSISendBinary232
	ENDIF
	movlw	'9'
	btfss	GlbKeypadKeyDown2,KEY_D2_9
	
	call	TxKeypadByte
skb10
	btfss	GlbLocalTempH,KEY_D2_ASTERISK
	goto	skb11
	;'*' has been released
	movlw	'*'
	btfss	GlbKeypadKeyDown2,KEY_D2_ASTERISK
	call	TxKeypadByte
	#IF GSI_CAN_SUPPORT
		btfss	GlbKeypadKeyDown2,KEY_D2_ASTERISK
		bsf	GlbDebugFlag,DEBUG_CALL_DEBUG_CODE
		;call	GSITestCAN
	#ENDIF

skb11
	btfss	GlbLocalTempH,KEY_D2_HASH
	goto	skb12
	;'#' has been released
	movlw	'#'
	btfss	GlbKeypadKeyDown2,KEY_D2_HASH
	call	TxKeypadByte

	#IF GSI_CAN_SUPPORT
		btfss	GlbKeypadKeyDown2,KEY_D2_HASH
		bsf	GlbDebugFlag,DEBUG_CALL_DEBUG_CODE_1
		;call	TestI2C
	#ENDIF

skb12
	goto	skb_exit

;send to rs232 and possibly lcd if echoing
;chr is in WREG
TxKeypadByte
;'if on a node, send down the CAN to the master for Tx to a pc
;if one is attached
;GlbKeypadStatus,GlbKeypadKeyDown1,GlbKeypadKeyDown2
; can be sent as data
	movwf	KdCurrentKey		;store the key
	;-
tkb_232_exit:
	btfss	GlbSIOMode,SIO_M_BINARY
	call	GSISendByte232		;in ascii mode so just send

	#IF GSI_CAN_SUPPORT
	;also broadcast it down the can
tkb_CAN_tx_key:
	movff	CnMode,KdTemp		;get the current mode
	btfss	KdTemp,CAN_MODE_RUNNING
	bra	tkb_exit
tkbctk1:
	;-ok CAN is running so tx the mssg
	movff	CnThisNode,CnTxBuff
	movlw	CAN_KD_MSSG_KEY_UP
	movff	WREG,CnTxBuff+1
	movff	KdCurrentKey,CnTxBuff+2	
;----------Note that this mssg will be sent to the MASTER---
;If we are already the master should we still send it??
	mCANPrTxSendMsg CAN_ID_KEYPAD_MSSG,CnTxBuff,3,CAN_TX_XTD_FRAME

        addlw   0x00            ;Check for return value of 0 in W
        bz      tkbctk1         ;Buffer Full, Try again
	#ENDIF ;GSI_CAN_SUPPORT

	banksel	KdBank
tkb_exit:
	return


InitKeypadDebounce
	movf	TMR0L,W,A		;8 bit mode
	addlw	KEY_DEBOUNCE_TICKS
	movwf	KdKeypadDebounceCounter	
	bsf	GlbKeypadStatus,KEY_DEBOUNCING
	return

	END
