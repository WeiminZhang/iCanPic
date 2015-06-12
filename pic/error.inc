;******************************************************************************
;* @file    error.inc part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;error.inc
#ifndef ERROR_INC_INCLUDED
#define ERROR_INC_INCLUDED

;------------------------------------------------------------
;	ErrStatus
;Control if error messages go to lcd, rs232, both or neither
;-------------------------------------------------------------

#define 	ERR_TO_LCD	0
#define		ERR_TO_RS232	1

#define		ERR_STATUS_FLAGS	(1 << ERR_TO_LCD) | (1<< ERR_TO_RS232)

;-----------------------------------------------------------------
;	GlbErrState
;NOTE:
;these are dual use. They are a bit position for internal error
;reportin. They are also used as simple variable for sending
;back errors in RS232 Messages
;----------------------------------------------------------------
;bit pos errors
#define		GSI_ERR_I2C_NO_ACK_FROM_SLAVE	0
#define		GSI_ERR_I2C_NO_DATA				1
#define		GSI_ERR_I2C_BUS_COLL			2
#define		GSI_ERR_CAN						3
#define 	GSI_ERR_RS232					4
;Errors 5-7 for internal error condition (these are bit positions)
;further generic errors 
;--ERRORS-------------------------------

#define		GSI_ERR_RANGE				8
#define		GSI_ERR_FAIL				9
#define		GSI_ERR_UNSUPPORTED			10
#define 	GSI_ERR_NO_DATA				11

;------------------FATAL ERRORS-------------------------------
;Used in debugging version. 
;WREG is set to this value when a WDT is to be trigged
;ICD2 can then examine this value to determine error location

#define 	FATAL_ERROR_CQ_READ_FRAME	'a'
#define 	FATAL_ERROR_CQ_READ_LENGTH	'b'
#define 	FATAL_ERROR_CQ_WRITE		'c'
#define 	FATAL_ERROR_ISR_NO_INTERRUPT	'd'
#define 	FATAL_ERROR_RS232_OVERRUN	'e'
#define 	FATAL_ERROR_RS232_FRAME		'f'
#define 	FATAL_ERROR_RS232_BINARY_REC	'g'
#define		FATAL_ERROR_CAN_CQ_OVF		'h'
#define 	FATAL_ERROR_1
;--------------------------------------------------------------


#else	;ERROR_INC_INCLUDED

#endif	;ERROR_INC_INCLUDED
