;******************************************************************************
;* @file    error.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************


;error.asm
#define IN_ERROR.ASM

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

#include "i2c.inc"
#include "error.inc"
#if GSI_CAN_SUPPORT
#include "can.inc"
#endif
;------- exported from this module------------

	GLOBAL ErrStatus,InitErrorVrbls

;-------- from rs232.asm ---------------------------
	EXTERN GlbSIOMode1

;-------- from lcd.asm ---------------------------
	#IF GSI_LCD_SUPPORT
	EXTERN LdLCDRow,LdLCDCol
	#ENDIF

#IF GSI_CAN_SUPPORT
	;CAN variables beginning with Cn are in a separate bank
	EXTERN CnErrorState, CnErrorState1
#ENDIF

#IF GSI_RS232_SUPPORT
	;RS232 variables beginning with R2 are in a separate bank
	EXTERN R2ErrorState
#ENDIF


GLOBAL_ERROR_DATA	UDATA_ACS

GlbErrState	RES 1


ERROR_DATA	UDATA
ErrBank		RES 0
ErrStatus	RES 1
ErrLocalTemp	RES 1

ErrCurrentCol	RES 1
ErrCurrentRow	RES 1

ERROR_CODE CODE


;-----------------------------------
;GSIInitErrorHandling
;-----------------------------------
GSIInitErrorHandling
;-----------------------------------------
;	InitErrorVrbls
;called only from init.asm
;-----------------------------------------
InitErrorVrbls
	movlw	ERR_STATUS_FLAGS
	movwf	ErrStatus
	clrf	GlbErrState	
	return

;-------------------------------------
;	GSIWriteErrorMssg
;Use GlbErrState to determine pending message
;Write error mssg to lcd,rs232
;Preserve current cursor 
;-------------------------------------
GSIWriteErrorMessage
	#IF GSI_LCD_SUPPORT
	movff	LdLCDRow,ErrCurrentRow
	movff	LdLCDCol,ErrCurrentCol
	#ENDIF ;GSI_LCD_SUPPORT

	movlw	ERR_I2C_NO_ACK_FROM_SLAVE_MSSG
	btfss	GlbErrState,GSI_ERR_I2C_NO_ACK_FROM_SLAVE
	bra	swem_a
	bcf	GlbErrState,GSI_ERR_I2C_NO_ACK_FROM_SLAVE
	;can also call additional error handling code here
	;as the ErrState get reset, the info is lost
	bra	swem_write
swem_a
	;insert added messages here
	movlw	ERR_I2C_NO_DATA_MSSG
	btfss	GlbErrState,GSI_ERR_I2C_NO_DATA
	bra	swem_b
	bcf	GlbErrState,GSI_ERR_I2C_NO_DATA
	bra	swem_write
swem_b
	movlw	ERR_I2C_BUS_COLL_MSSG
	btfss	GlbErrState,GSI_ERR_I2C_BUS_COLL
	bra	swem_c
	bcf	GlbErrState,GSI_ERR_I2C_BUS_COLL
	bra	swem_write
swem_c
	#IF GSI_CAN_SUPPORT
	btfss	GlbErrState,GSI_ERR_CAN
	bra	swem_d
	call	CANError		;returns error in W
	bcf	GlbErrState,GSI_ERR_CAN
	banksel	ErrCurrentRow		;reset to Err bank
	bra	swem_write
	#ENDIF

swem_d
#IF GSI_RS232_SUPPORT
	btfss	GlbErrState,GSI_ERR_RS232
	bra	swem_e
	call	R2Error			;returns error in W
	bcf	GlbErrState,GSI_ERR_RS232
	banksel	ErrCurrentRow		;reset to Err bank
	bra	swem_write
#endif

swem_e
	;insert added messages here

	movlw	UNKNOWN_ERROR_MSSG
	clrf	GlbErrState	
swem_write
	banksel	ErrLocalTemp		;may be in CAN_DATA bank
	movwf	ErrLocalTemp
	btfss	ErrStatus,ERR_TO_LCD
	goto	lcdNotWritten
	#if GSI_LCD_SUPPORT
	call	GSIEnd			;bottomLine,right-most col
	call	GSIClrLine		;clear current line
	movf	ErrLocalTemp,W	;retrieve the mssg index
	call	GSIWriteStr
;lcd has been written so rs232 might have also
	movf	ErrLocalTemp,W		;retrieve the index
	;btfss	GlbSIOMode1,SIO_M1_ECHO ;if set already echoed!
	btfsc	GlbSIOMode1,SIO_M1_ECHO ;if set already echoed!
	bra		swem2					;mod 4/2/06
	;message not echoed
	
	;call	GSIWriteMessageRS232
	;goto	swem2
	#ENDIF
lcdNotWritten
	btfss	ErrStatus,ERR_TO_RS232	;abort if no rs232 echo
	bra		swem2
	;btfss	GlbSIOMode,SIO_M_BINARY
	call	GSIWriteErrorMessageRS232	;only write to rs232 if ascii mode and ERR_TO_RS232
	
swem2
	#IF GSI_LCD_SUPPORT
	movff	ErrCurrentRow,LdLCDRow	;restore cursor
	movff	ErrCurrentCol,LdLCDCol
	call	GSISetCursor
	#ENDIF
	#IF GSI_CAN_SUPPORT
	;finally reset ERR_CAN if all CAN errors reported
	banksel CnErrorState 
	movf	CnErrorState		;WREG=CnErrorState
	iorwf	CnErrorState1,W		;OR with CnErrorState1
	banksel	ErrCurrentRow
	btfsc	STATUS,Z
	bcf	GlbErrState,GSI_ERR_CAN	;no more CAN errors pending
	#ENDIF
	return

;--------------------------------------------
;	CANError
;Must clr individual bits 7=highest priority
;clr GlbErrState,ERR_CAN when CnErrorState==0
;Cn prefixes are in their own bank 
;---------------------------------------------
	#IF GSI_CAN_SUPPORT

CANError
	banksel	CnErrorState

	movlw	CAN_RXB0OVFL_MSSG
	btfss	CnErrorState,CN_ERROR_RXB0OVFL
	bra	ce1
	bcf	CnErrorState,CN_ERROR_RXB0OVFL
	return
ce1
	movlw	CAN_RXB1OVFL_MSSG	
	btfss	CnErrorState,CN_ERROR_RXB1OVFL
	bra	ce2
	bcf	CnErrorState,CN_ERROR_RXB1OVFL
	return
ce2
	movlw	CAN_TXBO_MSSG
	btfss	CnErrorState,CN_ERROR_TXBO
	bra	ce3
	bcf	CnErrorState,CN_ERROR_TXBO
	return
ce3
	movlw	CAN_TXBP_MSSG
	btfss	CnErrorState,CN_ERROR_TXBP
	bra	ce4
	bcf	CnErrorState,CN_ERROR_TXBP
	return
ce4
	movlw	CAN_RXBP_MSSG
	btfss	CnErrorState,CN_ERROR_RXBP
	bra	ce5
	bcf	CnErrorState,CN_ERROR_RXBP
	return
ce5
	movlw	CAN_TXWARN_MSSG
	btfss	CnErrorState,CN_ERROR_TXWARN
	bra	ce6
	bcf	CnErrorState,CN_ERROR_TXWARN
	return
ce6
	movlw	CAN_RXWARN_MSSG
	btfss	CnErrorState,CN_ERROR_RXWARN
	bra	ce7
	bcf	CnErrorState,CN_ERROR_RXWARN
	return
ce7	;not needed as the TX/RXWARN bits are tested explicitly
	;this flag is a composite of TX/RXWARN
	;movlw	CAN_EWARN_MSSG
	;btfss	CnErrorState,CN_ERROR_EWARN
	;bra	ce8
	;bcf	CnErrorState,CN_ERROR_EWARN
	;return
ce8
	movlw	CAN_IRXIF_MSSG
	btfss	CnErrorState1,CN_ERROR1_IRXIF
	bra	ce9
	bcf	CnErrorState1,CN_ERROR1_IRXIF
	bsf	PIE3,IRXIE		;re-enable int
	return
ce9
	movlw	CAN_ERROR_INIT_MSSG
	btfss	CnErrorState1,CN_ERROR1_INIT
	bra	ce10
	bcf	CnErrorState1,CN_ERROR1_INIT
	return
ce10
	movlw	CAN_RX_CQ_OVF_MSSG
	btfss	CnErrorState1,CN_ERROR1_RX_CQ_OVF
	bra	ce11
	bcf		CnErrorState1,CN_ERROR1_RX_CQ_OVF
ce11	
	;messg	require CN_ERROR1_RX_CQ_OVF error message
	clrf	CnErrorState1
	movlw	UNKNOWN_CAN_ERROR_MSSG
	return
	#ENDIF ;GSI_CAN_SUPPORT


R2Error:

	banksel	R2ErrorState

	movlw	R2_ERROR_ECHO_TX_STR
	btfss	R2ErrorState,SIO_ERR_ECHO_TX
	bra	re1
	bcf	R2ErrorState,SIO_ERR_ECHO_TX
re1:
	return


	END
