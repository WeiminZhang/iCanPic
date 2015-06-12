;******************************************************************************
;* @file    i2c.inc part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;i2c.inc
#ifndef I2C_INC_INCLUDED
#define I2C_INC_INCLUDED


;#define  I2CClock    D'400000'           ;define I2C bite rate,400k
#define  I2CClock    D'100000'           ;define I2C bite rate,400k
#define  I2C_BAUD_RATE_VALUE  (((FOSC/I2CClock)/4) -1) ; 

I2C_SIZE_MASK		EQU		0x0F


;--------------------------------------------------------
;	GlbI2CStatus flags
;These are not fully implemented, there is no need
;as the GlbErrState flags return the important status info
;GlbI2CStatus is 0 if subsystem is IDLE
;---------------------------------------------------------
#define I2C_START_CONDITION		0	;I2C is operating a write
#define I2C_STOP_CONDITION		1	;Executing a STOP
#define I2C_WRITE_BYTE_CONDITION	2
#define I2C_READ_BYTE_CONDITION		3  
;-------------------------------
;		RX
;-------------------------------
I2C_RX_BUFF_SIZE			EQU		d'16'

;-------------------------------
;		TX
;-------------------------------
I2C_TX_BUFF_SIZE			EQU		d'8'


;-------------------------------
;GlbI2CStatus:
;-------------------------------
I2C_RX_BUFF_OVF				EQU		0
I2C_TX_BUFF_OVF				EQU		1
I2C_CHAR_AVAILABLE			EQU		2



;-------------------------------------
;	MAX518	8 bit dual DAC
;-------------------------------------

;MAX518_BASE_ADDR	EQU	0x58	;factory set
MAX518_BASE_ADDR	EQU	0x2c	;factory set
MAX518_CMD_RST		EQU	0x10
MAX518_CMD_PD		EQU	0x8
MAX518_CMD_SLCT_DAC0	EQU	0x0
MAX518_CMD_SLCT_DAC1	EQU	0x1		

GSI_DAC_ADDRESS	EQU 	MAX518_BASE_ADDR+0	;A0,A1 set to 0V

#endif ;I2C_INC_INCLUDED
