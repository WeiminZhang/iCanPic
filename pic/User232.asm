;******************************************************************************
;* @file    user232.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;User232.asm

#define IN_USER232.ASM

;#include "p18xxx8.inc"

#include	"GSI.inc"

#include "TestGsi.inc"

;from R2Slave.asm	
	EXTERN R2SendProcessRecordSuccessFail

#IF STARTLE	
;For testGsi.asm
	EXTERN STLRecord0Received
#ENDIF	
;-------------------------------------

USER232	CODE

;-------------------------------------------------------------
; R2RxRecord holds the record
;This code has been executed
;movlw	1		;1 data byte sent;
;movwf	R2SubfunctionRecordLength
;movlw	GSI_RS232_MESSAGE_SUCCESS ;| (1<< GSI_RS232_MESSAGE_MASTER_REQ_BIT)
;movwf	R2SubfunctionRecordData ;data[0]
;So The success/fail has been filled with success returning
;1 byte the success code
;User must issue a:
;call	R2SendProcessRecordSuccessFail
;at some point in this code
;Note that it must respond within 1 sec approx or else the 
;caller can assume a timeout.
;User may also increase R2SubfunctionRecordLength up to a maximum
;of 8 and add data to the R2SubfunctionRecordData[0-7] array
;This data will be received by the pc

;BSR=R2Bank

;On return from this function, library will restore BSR
;-------------------------------------------------------------
GSIUserR2Record_0
#IF STARTLE
	call	STLRecord0Received
#ELSE	
	call	R2SendProcessRecordSuccessFail
#ENDIF
	return
GSIUserR2Record_1	
	call	R2SendProcessRecordSuccessFail
	return
GSIUserR2Record_2
	call	R2SendProcessRecordSuccessFail
	return
GSIUserR2Record_3
	call	R2SendProcessRecordSuccessFail
	return
GSIUserR2Record_4
	call	R2SendProcessRecordSuccessFail
	return

	END
