;******************************************************************************
;* @file    interrupts.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;interrupts
#define IN_INTERRUPTS.ASM

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
	#IF GSI_CAN_SUPPORT
#include "canPrTx.inc"
	#ENDIF
;-----------------------------------
;	Exported from this module
;-----------------------------------
	GLOBAL	ISRLow,ISRHigh
	GLOBAL	GiIntType
;------------------------------------
;	From rs232.asm
;------------------------------------
	EXTERN	R2RxByte,GlbSIOStatus,GlbSIOMode

;------------------------------------
;	From i2c.asm
;------------------------------------
	;data
	EXTERN I2CStateIndex,InI2CInt
;------------------------------------
;	From can.asm
;------------------------------------
	;data
	#IF GSI_CAN_SUPPORT
	EXTERN CANSendSync
	#ENDIF
;-----------------------------------
;	from gsi.asm
;-----------------------------------
	EXTERN GlbClock		;TMR0 13msec clock (16 bit)
;--------------------------------------------------
;	Interrupt data in bank 1
;--------------------------------------------------	
INT_DATA	UDATA	
IntBank			RES 0
IntStatusTemp 		RES 1
IntFSR2LTemp 		RES 1
IntFSR2HTemp 		RES 1
IntBSRTemp 			RES 1
IntPCLATHTemp 		RES 1
IntPCLATUTemp 		RES 1

IntStatusTempHP		RES 1
IntFSR0LTempHP 		RES 1
IntFSR0HTempHP 		RES 1
IntFSR1LTempHP 		RES 1
IntFSR1HTempHP 		RES 1
IntFSR2LTempHP 		RES 1
IntFSR2HTempHP 		RES 1
IntBSRTempHP 		RES 1
IntPCLATHTempHP		RES 1
IntPCLATUTempHP		RES 1
IntCANCONTempHP		RES 1

;-------------------------------------
;	Interrupt data in AccessRam
;------------------------------------
INT_GLOBAL_DATA	UDATA_ACS
GlbWTemp 		RES 1
GlbWTempHP 		RES 1
;------------------------------------
;	data in GSI_BANK
;------------------------------------
GSI_DATA	UDATA
GiIntType 		RES 1		

INTERRUPT_CODE	CODE	
;----------------------------------------------------------------
;	ISR_HIGH
;INT0 (mandatory)
;TMR3 the CAN SYNC clock @100 msec  are High priority
;2-3 clocks latency+2cycles for goto, thus 3 cycles? to arrive here
;the tmr incs should occur synchronously with instruction cycles,
;so assume 1 cycle latency to interrupt, 2 cycles for goto, arriving
;at this point at +3 cycles
;----------------------------------------------------------------

ISRHigh

	movwf	GlbWTempHP		;save W. Doesn't alter STATUS
	swapf	STATUS,W,A		;swapf doesn't alter STATUS

	;Must save BSR before swapping banks for obvious reason!
	;not so obvious of course...		
	movff	BSR,IntBSRTempHP	;doesn't affect STATUS or WREG
	;
	banksel IntBank			;doesn't affect WREG
	;Now can store STATUS in Banked memory to save AccessRam
	movwf	IntStatusTempHP		;a movf might change Z in STATUS

	;Probably need to store pclath,pclathu as if we have performed 
	;a computed goto and an int occurs we will vector here and pclath/u
	;will be filled with this codes address. When this vector returns
	;the goto will be at an offset near this ISR!	
	movf	PCLATH,W
	movwf	IntPCLATHTempHP		;save PCLATH
	movf	PCLATU,W
	movwf	IntPCLATUTempHP
	;

	movf	FSR0L,W		;used in CANTxMsg
	movwf	IntFSR0LTempHP
	movf	FSR2H,W
	movwf	IntFSR0HTempHP		;latency +16
	
	movf	FSR1L,W		;used in CANTxMsg
	movwf	IntFSR1LTempHP
	movf	FSR1H,W
	movwf	IntFSR1HTempHP		;
	
	movf	FSR2L,W		;used in AddToRxBuff also in GetFromRxBuff
	movwf	IntFSR2LTempHP
	movf	FSR2H,W
	movwf	IntFSR2HTempHP		;


	;Process HP ints
	IF 0
	btfss	INTCON,INTE,A		;is this int enabled?
	goto	I1			;no, check the next
	btfsc	INTCON,INTF,A		;RB0\INT
	goto	InRB0Int
       ;-
	ENDIF


	#IF GSI_CAN_SUPPORT
;check for sync timer
	btfss	PIE2,TMR3IE
	bra	i1_hpxx
	btfss	PIR2,TMR3IF		;22 instructions before load
	bra	i1_hpxx			;not fired,so check next int

;-------Send a CAN_ID_SYNC---------------------------------
;We must preserve CANCON across this call as we might
;be interrupting a low priority int
;Does this call actually change CANCON??
	movff	CANCON,IntCANCONTempHP
;------------------------------------------------------------
	call	CANSendSync
	banksel	IntBank
;---------restore CANCON-------------
	movf	CANCON,W
	andlw	b'11110001'
	iorwf	IntCANCONTempHP,W
	movwf	CANCON
;----------------------------------
	bcf	PIR2,TMR3IF		;clr int

	#ENDIF ;GSI_CAN_SUPPORT
i1_hp

#IF GSI_USER_HP_INT_SUPPORT
	banksel	GiBank
	call	GSIUserISRHP
	banksel	IntBank
#ENDIF
	banksel	IntBank
      ;Restore state
	;restore FSR2
	movf	IntFSR2LTempHP,W
	movwf	FSR2L
	movf	IntFSR2HTempHP,W
	movwf	FSR2H
      ;restore FSR0
   	movf	IntFSR1LTempHP,W
	movwf	FSR1L
	movf	IntFSR1HTempHP,W
	movwf	FSR1H

      ;restore FSR0
   	movf	IntFSR0LTempHP,W
	movwf	FSR0L
	movf	IntFSR0HTempHP,W
	movwf	FSR0H

	;-
	movf	IntPCLATHTempHP,W	;get previous PCLATH
	movwf	PCLATH,A		
	movf	IntPCLATUTempHP,W	;get previous PCLATU
	movwf	PCLATU,A		

	swapf	IntStatusTempHP,W	;restore W and STATUS
	;BSR must be restored after MnStatusTemp, else might be in wrong bank
	;movff doesn't affect WREG
	movff	IntBSRTempHP,BSR	;restore BSR	

	movwf	STATUS			;restore STATUS
	swapf	GlbWTempHP,F
	swapf	GlbWTempHP,W	

	retfie
i1_hpxx
	banksel	R2Bank
	movf	INTCON,W
	call	DEBUGGSISendBinary232ISR
	movf	PIR1,W
	call	DEBUGGSISendBinary232ISR
	movf	PIR2,W
	call	DEBUGGSISendBinary232ISR

	bra	i1_hp

;----------------------------------------------------------------
;	ISR_LOW
;All ints except INT0 and TMR3 are LOW priority
;----------------------------------------------------------------
ISRLow
;Interrupt Service Routine
;If no goto ISR is used then just latency clocks to arrive here
;2-3 cycles for latency
;no ints can be serviced while in the isr so no need to
;test and call. Instead test and goto.
;also removes issue with TxRegEmtpy int. The flag will be set and remain
;set till a new char is placed in the TXREG. If no char to send then an
;isr handler that test and calls all flags will continually detect 
;a TX empty int.
;For this reason TXEmpty int must be placed last in the test ladder (is this correct?)

;IMPORTANT:
;The flags INTF,TOIF,RBIF etc will be set irrespective of the state of their
;enable bit: INTE,TOIE,RBIE etc.
;To service ints we must first check that this int is enabled before testing the
;flag.

;VERY IMPORTANT:
;GlbWTemp must be in shared memory or be at same offset in
;all banks. An isr can occur when any bank is selected. To save the bank 
;info reqUires using W, which reqUires storing W before changing banks!
;Thus we must store W in any arbitrary bank! (The bank where the int occurred).
;So keep GlbWTemp in shared memory accessible regardless of BANK in use

	movwf	GlbWTemp		;save W. Doesn't alter STATUS
	swapf	STATUS,W		;swapf doesn't alter STATUS

	;Must save BSR before swapping banks for obvious reason!
	;not so obvious of course...		
	movff	BSR,IntBSRTemp		;doesn't affect STATUS or WREG
	;
	banksel IntBank			;doesn't affect WREG
	;Now can store STATUS in Banked memory to save AccessRam
	movwf	IntStatusTemp		;a movf might change Z in STATUS

	;Probably need to store pclath,pclathu as if we have performed 
	;a computed goto and an int occurs we will vector here and pclath/u
	;will be filled with this codes address. When this vector returns
	;the goto will be at an offset near this ISR!	
	movf	PCLATH,W
	movwf	IntPCLATHTemp		;save PCLATH
	movf	PCLATU,W
	movwf	IntPCLATUTemp
	;
	movf	FSR2L,W,A		;used in AddToRxBuff also in GetFromRxBuff
	movwf	IntFSR2LTemp
	movf	FSR2H,W,A		;used in AddToRxBuff also in GetFromRxBuff
	movwf	IntFSR2HTemp
      ;
#IF GSI_USER_LP_INT_SUPPORT
	banksel	GiBank
	bcf		STATUS,C		;if no user code at ISRFirst then we need to process
	call	GSIUserISRFirst
	banksel	IntBank
	bc	__user_ISR_exit		;C = early return. User handles all ints
#ENDIF
	IF 0
	btfss	INTCON,INTE,A		;is this int enabled?
	goto	I1			;no, check the next
	btfsc	INTCON,INTF,A		;RB0\INT
	goto	InRB0Int
       ;-
	ENDIF
I1
	btfss	INTCON,T0IE,A
	goto	I2
   	btfsc	INTCON,T0IF,A		;TMR0
	goto	InTMR0Int
       ;-
I2
	btfss	INTCON,RBIE,A
	goto	I3
   	btfsc	INTCON,RBIF,A		;PortChange
	goto	InPortChangeInt
I3

;TEST FOR COMPLETION OF VALID I2C EVENT 
	btfss	PIE1,SSPIE               ;test is interrupt is enabled
	goto	test_buscoll             ;no, so test for Bus Collision Int
	btfss   PIR1,SSPIF               ;test for SSP H/W flag
	goto	test_buscoll             ;no, so test for Bus Collision Int
	bcf	PIR1,SSPIF               ;clear SSP H/W flag

	call    InI2CInt           	;service valid I2C event

; TEST FOR I2C BUS COLLISION EVENT
test_buscoll 
	btfss	PIE2,BCLIE              ;test if interrupt is enabled
	goto	I4		        ;no, so test for Timer1 interrupt
	btfss   PIR2,BCLIF              ;test if Bus Collision occured
	goto    I4			;no, so test for Timer1 interrupt
	bcf	PIR2,BCLIF              ;clear Bus Collision H/W flag
	call	InI2CBusColl       	;service bus collision error
	banksel	IntBank
	;-Now check other peripheral ints
I4
	IF EEPROM_RESTORE_STATE_ENABLED == TRUE
		btfss	PIE2,EEIE		;eeprom write int
		goto	I5
		btfsc	PIR2,EEIF
		goto	InEEPROMWriteInt
	ENDIF
I5
	IF GSI_CAN_SUPPORT
		call	CANPrTxISR	;
		banksel	IntBank
	ENDIF

	
	IF RX_INTERRUPT
	btfss	PIE1,RCIE
	goto	I6
	btfsc	PIR1,RCIF		;a single enable bit for Tx/Rx
	goto	InRxInt
	ENDIF
I6	
	IF	TX_INTERRUPT
		btfss	PIE1,TXIE
		goto	I7
		btfsc	PIR1,TXIF
		goto	InTxBuffEmptyInt
I7
	ENDIF		
	;If we arrive here we have an unknown interrupt, an error
i8	;no longer an error. User code might have serviced an
	;interrupt
	;call	DEBUGCauseWDT
	;-
eoi
	banksel	GiBank
	call	GSIUserISRLast
	banksel	IntBank
__user_ISR_exit:
	movf	IntFSR2LTemp,W
	movwf	FSR2L
	movf	IntFSR2HTemp,W
	movwf	FSR2H

	movf	IntPCLATHTemp,W		;get previous PCLATH
	movwf	PCLATH,A		
	movf	IntPCLATUTemp,W		;get previous PCLATU
	movwf	PCLATU,A		

	swapf	IntStatusTemp,W		;restore W and STATUS
	;BSR must be restored after MnStatusTemp, else might be in wrong bank
	;movff doesn't affect WREG
	movff	IntBSRTemp,BSR		;restore BSR	
		
	movwf	STATUS			;
	swapf	GlbWTemp,F
	swapf	GlbWTemp,W	

	retfie

;**********************************************************************
;				Int Handlers
;Called directly from ISR so must retfie
;**********************************************************************
;*************************
;	InRB0Int
;************************
InRB0Int
	bcf	INTCON,INT0IE,A		;disable further ints
	bcf	INTCON,INT0IF,A		;clr rb0 int

	goto	eoi

;***********************
;	InPortChangeInt
;***********************
InPortChangeInt	
;Port B change Interrupt handler
;Should be unused in this program
	IF TRACE
	movlw	'P'
	call	GSIEchoByte232
	ENDIF
	bcf	INTCON,RBIE,A	;no more RBIF's
	banksel	GiBank
  	bsf	GiIntType,RBIF,A	;flag PortChange
	goto	eoi

;***********************************************************
;	InTMR0Int
;prescaler of 256, count of 256, interval 13msec @20MHz
;***********************************************************
InTMR0Int

;poll on the timer tick, if state has changed, set
;debounce flag, and poll in main loop till key debounced
;currently this would poll every 13 ms (3ms @ 10Mhz)

	bcf	INTCON,TMR0IF,A	;clear current int
	;bcf	INTCON,T0IE,A	;disable tmr0 ints
	incf	GlbClock		;inc low byte
	btfsc	STATUS,Z
	incf	GlbClock+1	;inc high byte if rollover
	
	banksel GiBank
	bsf	GiIntType,TMR0IF	;acivate top level handler
	goto	eoi


;*****************************
;	InTMR1Int
;*****************************
InTMR1Int
;Timer1 interrupt handler

	bcf	PIR1,TMR1IF,A		;reset to allow further ints
	bcf	PIE1,TMR1IE,A		;disallow further TMR1 ints
	goto	eoi


;----------------------------------------
;	InEEPROMWriteInt
;----------------------------------------
InEEPROMWriteInt

	bcf	PIR1,EEIF,A
iewi
	nop
	goto	iewi	
	goto	eoi

;-------------------------
;	InTxBuffEmptyInt
;-------------------------
InTxBuffEmptyInt
;probably need to check TXSTA,TRMT

	;currently just ignore as this is disabled!
	bcf	PIE1,TXIE,A
	bcf	PIR1,TXIF,A
itbei
	nop
	goto	itbei
	goto	eoi


;-----------------------------------------------------------------
;	InRxInt
;handled locally and by GlbSIOMode, GlbSIOStatus flags
;IdentifyInt will be ignored
;Note break detection (see mchip appnote for usart)
;A framing error with a received data of 0 is considered
;sufficient to assume a break has been received.
;For Tx of a break they suggest changing the baud rate to a lower
;speed for 1 byte. This would still work for interrupt driven Tx
;The method I am employing is to leave baud rate alone but hold the
;Tx line high (bcf PORTC,6; inverter follows) for a software 
;generated delay. 
;Using software delay is fine, CAN frames and rs232 data can still 
;be received. If baud rate was lowered, could miss data from pc.
;-----------------------------------------------------------------
InRxInt
	banksel	R2Bank

	btfsc	RCSTA,FERR,A
	goto	FrameError
	;-
	btfsc	RCSTA,OERR,A
	goto	OverrunError

	movf	RCREG,W,A
	movwf	R2RxByte
	;Byte might be a char to echo to lcd or a control char

	;btfsc	GlbSIOStatus,SIO_LITERAL
	;goto	iri1
	;-
	;Ignore GSI_USR_ABRT if we are in binary mode
	btfsc	GlbSIOMode,SIO_M_BINARY
	bra	iri1
	;-
	sublw	GSI_USR_ABRT			;CTR-C
	btfss	STATUS,Z,A
	bra	iri1
	;user abort
	bsf	GlbSIOStatus,SIO_USR_ABRT
	bra	iri_usr_abrt_exit
iri1
	;bcf	GlbSIOMode,SIO_M_LITERAL
	movf	R2RxByte,W
	call	GSIAddToRS232RxBuff
iri_usr_abrt_exit
iri_no_write_char_exit:
	goto	eoi
	;
OverrunError
	bcf	RCSTA,CREN,A		;clear error;
	bsf	RCSTA,CREN,A		;reset back to continuous receive
	;nop
	movlw	FATAL_ERROR_RS232_OVERRUN
	call	DEBUGCauseWDT		;use call so that debugger has a stack frame
	;goto	eoi
FrameError
	;nop
;This can be generated when XP executes CreateFile. A small spike
;appears on the rs232 line, causing a dud character to be seen
;so we must ignore it rather than report it!
;On this pc it occurs on COM6 though not COM2 (only two tested so far)
;Can also be a break. If the data received is a 0 then we can
;assume it is a break
	movf	RCREG,W			;check for a break
	bz		iri_break_received
	movlw	FATAL_ERROR_RS232_FRAME
	;??	
	;movf	RCREG,W		;clear frame error?

	bra	iri_no_write_char_exit
	call	DEBUGCauseWDT
	;goto	eoi
iri_break_received
	call	DEBUGCauseWDT
	bra	iri_break_received	;just crash for now
;----------------------------------------------------------------------
;   *************** I2C Bus Collision  ******************
;On a bus collision the I2C module is placed in the idle state
;----------------------------------------------------------------------
InI2CBusColl
	#IF GSI_I2C_SUPPORT
	banksel	I2CBank	
	clrf	I2CStateIndex		;reset I2C bus state variable
	call	GSIInitI2CVrbls        ;re-initialize variables
	bsf	GlbErrState,GSI_ERR_I2C_BUS_COLL
	#ENDIF
	return                          

;***************************************************************************
;			End of Int handlers
;***************************************************************************
	CODE
;-----------------------------------
;	InitInts
;Makes no BANK assumptions
;------------------------------------
GSIInitInterrupts	
	;RBIE_FLAG can be 0 or 8 (bit 3 set)
;-------INTCON enable ints ----------------	
	movlw  1 << GIEH | 1 << RBIE | 1 << PEIE | 1 << TMR0IE	;peie,t0ie
	movwf	INTCON,A
;-----disable TX empty interrupt-----------	
	bcf	PIE1,TXIE,A		;disable TX empty int
	IF EEPROM_RESTORE_STATE_ENABLED == TRUE
		bsf	PIE2,EEIE,A		;enable EEPROM Write int
	ENDIF	;EEPROM_STATE_ENABLED
;----TMR0----------------	
	clrf	GlbClock
	clrf	GlbClock+1
	movlw	b'11000111'			;1:256 prescaler (13ms max count)
	movwf	T0CON,A		
	bcf	INTCON,TMR0IF,A			;and reset flag
	;bsf	INTCON,TMR0IE			;allow TMR0 ints already done
;--------PORTB int---------	
	movf	PORTB,W			;read port b to clr any port change condition
	IF PORT_CHANGE_INT
		movf	PORTB,W,A			;read portb to initalise port change 
		bcf	INTCON,RBIF,A			;and reset flag
		bcf	INTCON2,RBIP	;LOW priority port change
	ENDIF
;----------High Priority ints ----------------
	bsf	RCON,IPEN	;Interrupt priority enabled
	;vrbls
	banksel GiBank
	clrf	GiIntType
	bcf	INTCON2,TMR0IP	;low priority TMR0
	bcf	INTCON2,RBIP	;lp port change
	clrf	IPR1		;all low priority
	movlw	b'00000010'	;TMR3=High priority
	movwf	IPR2
	clrf	IPR3		;all low priority
#IF GSI_USER_LP_INT_SUPPORT | GSI_USER_HP_INT_SUPPORT
	call	GSIUserInitInterrupts
#ENDIF
	return

	END

