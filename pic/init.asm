;******************************************************************************
;* @file    init.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************


;init.asm
#define IN_INIT.ASM
;#include <G:\prog\mplab\MPLAB IDE\MCHIP_Tools\p18f458.inc>
;#include "p18xxx8.inc"
;#include <H:\prog\src\pic\include\equ.inc>
;#include <H:\prog\src\pic\include\gsmacros.inc>

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
#include "can.inc"

	GLOBAL	Restart,GiLastResetType
	#IF EEPROM_RESTORE_STATE_ENABLED
	GLOBAL	EpCANSlaveId,EpCANMasterId,GiEEPROMAddress
	#ENDIF
;------------------------------------
;	definitions from rs232.asm
;------------------------------------
	EXTERN GlbSIOMode1
;------------------------------------
;	definitions from keypad.asm
;------------------------------------
	EXTERN	SetTempButtonState
	EXTERN	KdTempKeypadKeyDown1

;------------------------------------
;	definitions from lcd.asm
;------------------------------------

;----------------------------------------
;	definitions from interrupts.asm
;----------------------------------------

;------------------------------------
;	definitions from error.asm
;------------------------------------
	;EXTERN InitErrorVrbls

;------------------------------------
;	definitions from i2c.asm
;------------------------------------

;------------------------------------
;	definitions from can.asm
;------------------------------------
	#IF GSI_CAN_SUPPORT
	EXTERN GlbCANState
	#ENDIF
;-------------------------------------
;	definitions in gsi.asm
;-------------------------------------
	EXTERN GlbDebugTempL,GlbDebugFlag,GlbResources,GlbResources1
	EXTERN GlbClock,GlbLocalTempL,GlbLocalTempH
	EXTERN GlbDontUseWithCAN
;-------------------------------------

;------------------------------------------------------------------
;	EEPROM Data
;------------------------------------------------------------------
GSI_CONFIG_DATA		CODE
	#IF  EEPROM_RESTORE_STATE_ENABLED
EpCopyright	DE	"Copyright Glenn Self 2004"
EpCANSlaveId	DE	CAN_NODE_THIS	;invalid for a slave
EpCANMasterId	DE	CAN_NODE_THIS	;invalid for  master

	#ENDIF
;----------------------------------------------------------------	

INIT_DATA	UDATA
	IF	EEPROM_RESTORE_STATE_ENABLED
GiEEPROMAddress	RES	1	
	ENDIF

GiLastResetType		RES 01		
	
INIT CODE

Restart
	call	DetermineResetType	;WREG=MSSG
	movwf	GlbGSIState		;use GlbGSIState as a temp varb
	;movwf	GiLastResetType	;used to send break to pc. see gsi.asm
;store state info for reset handling code to use
	movwf	GlbDontUseWithCAN	;holds resetType
	movff	GlbSIOMode,GlbDontUseWithCAN+1

	call	InitPortA
	call	InitPortB
	call	InitPortC
	call	InitPortD
	call	InitPortE

	call	InitVrbls

	banksel	GiBank		
	;following must be initialised before can print any messages!
	#IF GSI_LCD_SUPPORT
	call	GSILCDInit		
	#ENDIF
	#IF GSI_RS232_SUPPORT
	call	GSIInitRS232
	
	#endif
	IF DEBUG
		
	ENDIF
	call	GSIInitInterrupts
	banksel	GiBank
	#IF GSI_I2C_SUPPORT
	call	GSIInitI2C
	#ENDIF			;
	call	GSIInitErrorHandling
	#IF GSI_KEYPAD_SUPPORT	
	call	GSIInitKeypad
	#endif
	;Still in GSI_BANK
	
	banksel	GiBank
;-----inform user of reset type---------------
	#IF GSI_LCD_SUPPORT
	call	GSICls
	call	GSIEnd
	call	GSIBeginningOfLine
	movf	GlbGSIState,W	;retrieve the message	
	call	GSIWriteStr	
	call	GSIHome
	#ENDIF
;GlbGSIState holds the reset type
	clrf	GlbGSIState
;clear GlbGSIState here as the vrb holds
;USER_ABORT_FLAG which must be read by DetermineResetState.
;After DRS GLbGSIState holds the MSSG index for the reset type.
;--------set reset flags so we can pick the next reset type-------
	bsf	RCON,NOT_POR,A
	bsf	RCON,NOT_BOR,A
	bsf	RCON,NOT_TO	;read only bit?
	bcf	STKPTR,STKFUL
	bcf	STKPTR,STKUNF
;--------check for special resets-----------------
	call	CheckForSpecialResets	;only CAN_SET_SLAVE_ID at present

	;call	GSISignOn	;display banner

	IF GSI_CAN_SUPPORT
		call	GSIInitCAN	;sets PEIE,GIE
		banksel GiBank
	ENDIF
	call	GSIUserRestart
	banksel	GiBank
	return

;------------------------------
;	DetermineResetType
;------------------------------
DetermineResetType
	;user abort must be first in chain
	btfsc	GlbGSIState,GSI_STATE_USER_ABORT
	retlw	USER_ABRT_RESET_MSSG
	;POR must be second in chain
	btfss	RCON,NOT_POR,A
	retlw	POWER_ON_RESET_MSSG
	btfss	RCON,NOT_BOR,A
	retlw	BROWNOUT_RESET_MSSG
	btfsc	STKPTR,STKFUL
	retlw	STACK_OVF_RESET_MSSG
	btfsc	STKPTR,STKUNF
	retlw	STACK_UNF_RESET_MSSG
	btfss	RCON,NOT_TO	
	retlw	WDT_RESET_MSSG
	retlw	UNKNOWN_RESET_MSSG	;an MCLR reset

;------------------------------------------------
;	CheckForSpecialResets
;At present the only special case is POR + 'COL0'
;which is CAN_SET_SLAVE_ID
;------------------------------------------------
CheckForSpecialResets

;-can't use keypad as it might not be present on a slave
;------ See if COL_0 (1,4,7,*) is currently low---------------
;If no keypad then a switch connected between ROW_3 and COL_0 will
;be pressed
;------ Can use keypad code, single switch is a small keypad ----

	#IF GSI_KEYPAD_SUPPORT
	call	SetTempButtonState
	movf	KdTempKeypadKeyDown1
	IF SIMULATOR == FALSE
	btfsc	KdTempKeypadKeyDown1,KEY_D1_1
	bra	por_can_slave
	ENDIF
	;-other special resets in here
	return
por_can_slave
	bsf	GlbCANState,CAN_SET_SLAVE_ID	
	call	GSIHome
	movlw	CAN_MSSG_SET_SLAVE_ID_STR
	call	GSIWriteStr
	call	GSIHome

	call	GSICANSetSlave		;
	
	banksel	GiBank
	#ENDIF
	return

;-------------------------------------------
;	InitVrbls
;-------------------------------------------
InitVrbls
	;clrf	SlIntType
	;clrf	GlbErrState
	;call	InitRs232Vrbls	;not req'd handled in 
	;call	InitKeypadVrbls
	;call	InitLCDVrbls
	;call	InitErrorVrbls 
	;call	InitInterruptVrbls
	;call	InitI2CVrbls
	clrf	GlbResources
SUPPORT_MACRO MACRO
	LOCAL s
	s=0
	IF GSI_LCD_SUPPORT
		s=s | 	GSI_RESOURCE_LCD_BIT
	ENDIF
	IF GSI_RS232_SUPPORT
		s=s | 	GSI_RESOURCE_RS232_BIT
	ENDIF
	IF GSI_I2C_SUPPORT
		s=s | 	GSI_RESOURCE_I2C_BIT
	ENDIF
	IF GSI_I2C_KEYPAD
		s=s | 	GSI_RESOURCE_KEYPAD_BIT
	ENDIF
	IF GSI_CAN_SUPPORT
		s=s | 	GSI_RESOURCE_CAN_BIT
	ENDIF
	IF GSI_DIO_SUPPORT
		s=s | 	GSI_RESOURCE_DIO_BIT
	ENDIF
	IF GSI_INT_SUPPORT
		s=s | 	GSI_RESOURCE_INT_BIT
	ENDIF
	IF GSI_DEBUG_SUPPORT
		s=s | 	GSI_RESOURCE_DEBUG_BIT
	ENDIF

	ENDM
	clrf	GlbDebugFlag
	
	return

;----------------------
;	InitPortA
;----------------------
InitPortA
	clrf	PORTA
	movlw	0x07		;cmps/adc off
#IFDEF CMCON
	movwf	CMCON		;comparators off
#ENDIF	
#IF DEVICE == _18F458
	movwf	ADCON1		;ADC off (need to change this to enable AN0,1,3)
#ENDIF

#IF DEVICE == _18F4680
	movlw	0x0f		;ADC off for 18f4680
	movwf	ADCON1
#ENDIF
	movlw	b'11111111'	
	movwf	TRISA,A
	return


;----------------------
;	InitPortB
;----------------------
InitPortB   
	;CAN TX output low for initialisation is correct state
	clrf	PORTB		;all output off
#IF GSI_CAN_SUPPORT
	bsf		PORTB,2		;must drive the CAN or else we will generate a CAN RXBP (Passive Bus)
#endif	
	;movlw	b'11111010'	;rx=bit1,canRx=3i,canTx=2o, port change 4-7
	movlw	b'00111011'	;ooiiioii
	;movlw	b'00111111'	;ooiiioii
	movwf	TRISB
	return

;----------------------
;	InitPortC
;----------------------
InitPortC   

	clrf	PORTC		;all output off
	movlw	b'10111100'	;SDA SCL inputs bit3,4 (Hi impedance)
	movwf	TRISC
	return

;----------------------
;	InitPortD
;----------------------
InitPortD   
	clrf	PORTD,A		;all output off

	movlw	07h		;comparator off
	movwf	CMCON
	movlw	b'00001111'	;keep keypad (bits 0-3) all tristate
	movwf	TRISD,A
	return

;----------------------
;	InitPortE
;----------------------
InitPortE   

	clrf	PORTE,A		;all output off
	movlw	b'00000000'	;TRISE controls psp mode etc, only bit 0-2 are direction bits
	movwf	TRISE,A
	return


;------------------------------------------------
;	WriteEEPROM	
;------------------------------------------------

GSIWriteEEPROM	
;receive address of eeprom to write in EEPROMAddress
;receive data to write in W

;This function doesn't verify that the data was written
;EEPROM data has 1e6 life cycle.
;Must check for write interrupted by wdt,bod,mclr

	#IF  EEPROM_RESTORE_STATE_ENABLED
	
	MESSG	"Need to check for EEPROM interrupted write"

	IF 0
PIC18FXX8
5.4 Writing to the Data EEPROM
Memory
To write an EEPROM data location, the address must
first be written to the EEADR register and the data written
to the EEDATA register. Then, the sequence in
Example 5-2 must be followed to initiate the write cycle.
The write will not initiate if the above sequence is not
exactly followed (write 55h to EECON2, write 0AAh to
EECON2, then set WR bit) for each byte. It is strongly
recommended that interrupts be disabled during this
code segment.
Additionally, the WREN bit in EECON1 must be set to
enable writes. This mechanism prevents accidental
writes to data EEPROM due to unexpected code execution
(i.e., runaway programs). The WREN bit should
be kept clear at all times, except when updating the
EEPROM. The WREN bit is not cleared by hardware.
After a write sequence has been initiated, clearing the
WREN bit will not affect the current write cycle. The WR
bit will be inhibited from being set unless the WREN bit
is set. The WREN bit must be set on a previous instruction.
Both WR and WREN cannot be set with the same
instruction.
At the completion of the write cycle, the WR bit is
cleared in hardware and the EEPROM Write Complete
Interrupt Flag bit (EEIF) is set. The user may either
enable this interrupt, or poll this bit. EEIF must be
cleared by software.
	movlw 	DATA_EE_ADDR ;
	movwf 	EEADR ; Data Memory Address to read
	movwf 	DATA_EE_DATA ;
	movwf 	EEDATA ; Data Memory Value to write
	bcf	EECON1, EEPGD ; Point to DATA memory
	bcf	EECON1, CFGS ; Access program FLASH or Data EEPROM memory
	bsf	EECON1, WREN ; Enable writes
	bcf	INTCON, GIEL ; Disable interrupts
	;btfsc	INTCON,GIEL
	;bra	
	movlw	55h ;
	movwf	EECON2 ; Write 55h
	movlw	0AAh ;
	movwf	EECON2 ; Write AAh
	bsf	EECON1, WR ; Set WR bit to begin write
	bsf	INTCON, GIE ; Enable interrupts
. ; user code execution
_we_eecon1_loop:
	btfsc	EECON1,WR	;wait for any write to complete
	bra	_we_eecon1_loop

	bcf	EECON1, WREN ; Disable writes on write complete (EEIF set)
;after WR is cleared clear EEIF as it will be set to generate an int
;if ints are enabled
	ENDIF

	movwf	EEDATA
	movff	GiEEPROMAddress,EEADR
	bcf	EECON1,EEPGD	;point to data memory
	bcf	EECON1,CFGS	; Access program FLASH or Data EEPROM memory
	bsf	EECON1,WREN	;enable write

_we_gie_loop:
	bcf	INTCON,GIEL	;disable ints
	btfsc	INTCON,GIEL
	bra	_we_gie_loop	;loop till GIE clear

	;5 opcode  magic sequence to write. Used to ensure no spurious writes
	movlw	0x55
	movwf	EECON2				
	movlw	0xaa
	movwf	EECON2
	bsf	EECON1,WR	;start write operation
_we_eecon1_loop:
	btfsc	EECON1,WR	;wait for any write to complete
	bra	_we_eecon1_loop
	bcf	EECON1, WREN ; Disable writes on write complete (EEIF set)
;after WR is cleared clear EEIF as it will be set to generate an int
;if ints are enabled
	bcf	PIR2,EEIF

	bsf	INTCON,GIEL	;enable ints
	bcf	EECON1,WREN	;disable writes
	#ENDIF
	return

;----------------------------------------------------
;	ReadEEPROM
;----------------------------------------------------
GSIReadEEPROM
;receive address of eeprom to read in W
;return data in W
	#IF  EEPROM_RESTORE_STATE_ENABLED

	;movlw	DATA_EE_ADDR ;
	movwf	EEADR ;Data Memory Address
	;to read
	bcf	EECON1, EEPGD ;Point to DATA memory
	bcf	EECON1, CFGS  ;Should already be clr
	bsf	EECON1, RD ;EEPROM Read
	movf	EEDATA, W ;W = EEDATA
	#ENDIF	
	return

;-------------------------------------------------------------
;	RestoreStateFromEEPROM
;-------------------------------------------------------------
RestoreStateFromEEPROM
;requires interrupts enabled else on the 2nd call ReadEEPROM
;we hang while awaiting the EEPROM Write completion

	IF EEPROM_RESTORE_STATE_ENABLED == TRUE


	ELSE	;EEPROM_STATE_ENABLED

		call	GSISetDefaultState

	ENDIF

	return
;---------------------------------------------------
;	SetDefaultState
;---------------------------------------------------
GSISetDefaultState

	return

;--------------------------------------------------------------
;	GSISetResources
;Turn on or off global resources
;Receive a resource byte and turn on or off the bit
;in the GlbResources variable.
;Must be sure to disable the function if it is currently running
;GSI_RESOURCE_LCD_BIT	EQU	0	;clr for no LCD
;GSI_RESOURCE_RS232_BIT	EQU	1	;clr for no rs232
;GSI_RESOURCE_I2C_BIT	EQU	2	;clr for no i2c
;GSI_RESOURCE_KEYPAD_BIT EQU	3	;clr for no keypad
;GSI_RESOURCE_CAN_BIT	EQU	4	;clr for no CAN
;GSI_RESOURCE_DIO_BIT	EQU	5	;clr for no DIO
;GSI_RESOURCE_INT_BIT	EQU	6	;clr for no RB0,RB1 Interrupts
;GSI_RESOURCE_SPI_BIT	EQU	7	;clr for no SPI

;GSI_RESOURCE_1_DEBUG_BIT	EQU	0	;clr for no DEBUG code
;GSI_RESOURCES_1_ADC_BIT	EQU	1	
;GSI_RESOURCES_1_LP_INT_BIT	EQU	2	
;GSI_RESOURCES_1_HP_INT_BIT	EQU	3	

;----------------------------------------------------
;GSI_RESOURCE_LCD	EQU	1	;clr for no LCD
;GSI_RESOURCE_RS232	EQU	2	;clr for no rs232
;GSI_RESOURCE_I2C	EQU	4	;clr for no i2c
;GSI_RESOURCE_KEYPAD	EQU	8	;clr for no keypad
;GSI_RESOURCE_CAN	EQU	10h	;clr for no CAN
;GSI_RESOURCE_DIO	EQU	20h	;clr for no DIO
;GSI_RESOURCE_INT	EQU	40h	;clr for no RB0,RB1 Interrupts
;GSI_RESOURCE_SPI	EQU	80	;clr for no SPI

;GSI_RESOURCE_1_DEBUG	EQU	0	;clr for no DEBUG code
;GSI_RESOURCES_1_ADC	EQU	1	
;GSI_RESOURCES_1_LP_INT	EQU	2	
;GSI_RESOURCES_1_HP_INT	EQU	4	

;---------------------------------------------------------------
GSISetResources
	movwf	GlbResources	
	return

GSISetResources1
	movwf	GlbResources1	
	return

;---------------------------------------------------
;	GSIGetResources
;Return GlbResources in WREG
;---------------------------------------------------
	Messg Work to be done here
GSIGetResources
	movf	GlbResources,W
	return
GSIGetResources1
	movf	GlbResources1,W
	return


	END
