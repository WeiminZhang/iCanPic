;******************************************************************************
;* @file    debug.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;debug.asm
#define	IN_DEBUG.ASM

;#include <G:\prog\mplab\MPLAB IDE\MCHIP_Tools\p18f458.inc>
;#include "p18xxx8.inc"
#include "GSI.inc"
;#IF DEVICE == _18F458
;#include "p18xxx8.inc"
;#ELSE
;#INCLUDE "P18F4680.inc"
;#ENDIF


;DEBUG_DATA UDATA
;	DebugCounter1	RES 1

DEBUG_GLOBAL_DATA UDATA_ACS
GlbDebugCounter1	RES 1
GlbDebugTemp		RES 1


DEBUG CODE
;---------------------------------------------------------
;		DEBUGCauseWDT
;--------------------------------------------------------
	IF DEBUG	
DEBUGCauseWDT
	#IF GSI_LCD_SUPPORT
	movwf	GlbDebugTemp		;WREG holds error code
	banksel	LdBank
	movlw	'!'
	#endif
	call	GSISendByte232
	#IF GSI_LCD_SUPPORT
	movf	GlbDebugTemp,W
	call	GSILCDWriteChar
	movf	GlbDebugTemp,W
	#ENDIF

_dcw:
	bra	_dcw	;write ! and wdt
	return		;set PC to here using ICD2 to see how we got here
	ENDIF

DEBUGDelay500


	movlw	0
	movwf	GlbDebugCounter1
	clrf	WREG
_dd5	
	clrwdt
	nop
	nop
	nop
	decfsz	WREG
	bra	_dd5
	
	decfsz	GlbDebugCounter1	
	bra	_dd5		;WREG is 0 at this point

	return


GSIDebugInit

		clrf	GlbDebugTemp
	return
	END
