;******************************************************************************
;* @file    userCan.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

#define IN_USERCAN.ASM
	GLOBAL UserCANProcessCANMssg
;----------------------------------------------------------------
;		UserCan.asm
;
;The GSI CAN interface will call these functions on receipt of
;the appropriate CAN message id
;Time Critical messages (Filter0)
;id=0x4 GSIUserCANF0_4
;id=0x5 GSIUserCANF0_5
;id=0x6 GSIUserCANF0_6
;id=0x7 GSIUserCANF0_7
;id=0x8 GSIUserCANF0_8
;id=0x9 GSIUserCANF0_9
;id=0xa GSIUserCANF0_10
;id=0xb GSIUserCANF0_11	

;----------------------------------------------------------------

#include	"GSI.inc"

;#IF DEVICE == _18F458
;#include "p18xxx8.inc"
;	LIST   P=PIC18F458
;#ELSE
;#INCLUDE "P18F4680.inc"
;	LIST   P=PIC18F4680
;#ENDIF


USER_CAN	CODE
;--------------------------------------------------------------------
;	UserCANProcessCANMssg
;A CAN frame has been received
;it is in the set of variables CnRxMssg (see can.asm)
;Typically you will ignore this frame by returning with WREG set to 0
;However you have the option of processing it in this code.
;return WREG=1 if the GSI is to ignore this frame
;NOTE
;User CAN messages are defined for normal usage. 
;This function is for system related tasks
;--------------------------------------------------------------------
UserCANProcessCANMssg

	movlw	0
	return 			;return WREG=0, allow GSI to handle the frame


GSIUserCANF0_4
GSIUserCANF0_5
GSIUserCANF0_6
GSIUserCANF0_7
GSIUserCANF0_8
GSIUserCANF0_9
GSIUserCANF0_10
GSIUserCANF0_11	
	return


;----------------------------------------------------------------
;High priority messages (Filter1)
;id CAN_BASE+0 - CAN_BASE+15
;-----------------------------------------------------------------
GSIUserCANF1_0
GSIUserCANF1_1
GSIUserCANF1_2
GSIUserCANF1_3
GSIUserCANF1_4
GSIUserCANF1_5
GSIUserCANF1_6
GSIUserCANF1_7
GSIUserCANF1_8
GSIUserCANF1_9
GSIUserCANF1_10
GSIUserCANF1_11
GSIUserCANF1_12
GSIUserCANF1_13
GSIUserCANF1_14
GSIUserCANF1_15
	return
	END
