;******************************************************************************
;* @file    TestGSI.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************


;---------------------------------------------------------
;TMR0 used for timed tones
;TMR1 used for ADC
;TMR2 1msec period for task despatcher
;TMR3 used for delays
;ra0-ra3=AN; ra4=unused; ra5=AN
;rb0=unused; rb1=pic->pc rts/cts +-10V; rb2-3=CAN; rb4=MUTE
;rb5=TestPoint; rb6-rb7=ICD2
;rc0-rc2=SPI CS (tone amplitude,tone frequency,white noise amplitude)

;rc3-5=unused; rc6-7=rs232;
;rd0-7=Audio Chan select
;NOTE
;Timed tones can be a little delayed during an adc. The ADC
;interrupt will be serviced first should a timed tone int
;occur simultaneously. After the ADC has been serviced the
;Timed tone will be serviced
;---------------------------------------------------------

#define IN_TESTGSI.ASM


;#include "p18xxx8.inc"

#include "gsi.inc"

;#IF DEVICE == _18F458
;#include "p18xxx8.inc"
	;LIST   P=PIC18F458
;#ELSE
;#INCLUDE "P18F4680.inc"
	;LIST   P=PIC18F4680
;#ENDIF

 
#include "error.inc"
#include "TestGsi.inc"
#include "rs232.inc"

#IF STARTLE
#include "startle.inc"
#ENDIF
	GLOBAL GSIUserMain,GSIUserISRFirst,GSIUserISRLast,GSIUserInitInterrupts
	GLOBAL GSIUserRestart,GSIUserISRHP
#IF DEBUG
	;GLOBAL TestVol
#ENDIF

	EXTERN GlbResources
	
;from rs232.asm
	EXTERN R2RxRecordType,R2RxRecordNode
	EXTERN R2RxRecordLength,R2RxRecordData
	EXTERN R2BTemp ;currently only used for debugging
;from R2Slave
	EXTERN R2SendProcessRecordSuccessFail,R2SendProcessRecordFail

        ERRORLEVEL      -302    ;SUPPRESS BANK SELECTION MESSAGES


;GSI_GLOBAL_TEST_DATA	UDATA_ACS

#IF BOARD == TRAINING_BOARD
	MESSG "Training board version"
#ENDIF

#IF BOARD == ET_BOARD
	MESSG "ET board version"
#ENDIF


#IF STARTLE == FALSE
GSI_TEST_DATA	UDATA
GtBank			RES 0		;placeholder
GtCompOffset		RES	1


GSI_USER_CODE CODE


;-------------------------------------------------------------
;	GSIUserMain
;Called by library at the end of the libraries main polling loop
;where it checks for CAN messages, rs232 activity, keypad  etc.
;--------------------------------------------------------------
GSIUserMain

	banksel	GtBank

	return

;-------------------------------------------------------------
;	GSIUserISRFIRST
;Called by library as it enters the low priority ISR
;18 clocks to arrive here
;BSR=GiBank, safe to change banks
;safe to use FSR2
;latency +19 cycles to arrive here
;--------------------------------------------------------------
 
GSIUserISRFirst
	return
;-------------------------------------------------------------
;	GSIUserISRLast
;Called by library as it is about to exit the low priority ISR
;--------------------------------------------------------------
GSIUserISRLast
	return
	
;---------------------------------------------------------
;	GSIUserInitInterrupts
;called after the GSI has init'd interrupts. User may now
;override.
;All low priority except TMR3 used for CAN SYNC. If no sync
;required or CAN disabled then TMR3 is usable by user
;--------------------------------------------------------
GSIUserInitInterrupts

	return
;-------------------------------------------------------------
;	GSIUserISRHP
;
;High priority interrupt call from library
;latency +18 clocks to arrive here
;------------------------------------------------------------
GSIUserISRHP
	return	
	
;-----------------------------------------------------------
;	GSIUserRestart
;Called by library after a reset
;User code can override the preset allocation of resources
;as long as GSI functions that use the resource are not called
;Should perform initialisation here
;-----------------------------------------------------------
GSIUserRestart

	return

;---------------------------------------------------------
;	InitSPI
;---------------------------------------------------------
InitSPI
	return

#ELSE		;#if STARTLE ==FALSE
;For User232,asm
	GLOBAL	STLRecord0Received	

;See startle.inc
	
STL_TMR2_PRESCALE	=	b'00000001'	;4
STL_TMR2_POSTSCALE	=	b'00100000'	;5
STL_TMR2_VALUE		=	249		;250*4*5=5000*200e-9=1ms

GSI_GLOBAL_TEST_DATA	UDATA_ACS
GlbGtStatus		RES	1
GlbGtDebug		RES	2
GlbGtNumDataPoints	RES	2
GlbGtTFSR0		RES	2	;store FSR0


GSI_TEST_DATA	UDATA
GtBank			RES	0
GtTickCounter		RES	2	;.994ms tick counter
GtISRTemp		RES	1

GtTCL5615TxByte1	RES	1	;msb first
GtTCL5615TxByte0	RES	1	;lsb second
__GtTCL5615TxByte1	RES	1	;msb first
__GtTCL5615TxByte0	RES	1	;lsb second
GtChan			RES	1	;current chan
__GtChan		RES	1	;used internally
__GtTemp		RES	1
GtCompOffset		RES	1
#if STARTLE_V2
GtDDSByte0		RES 1
GtDDSByte1		RES 1
GtDDSByte2		RES 1
GtDDSByte3		RES 1
GtDDSByte4		RES 1
GtDDSBitCounter	RES 1	

GtVolChanByte	RES 1
GtVolDBByte		RES 1
GtVolBitCounter RES	1
#endif	;STARTLE ==FALSE

;CmdListBuffer holds R2RxRecordData[0-2]
;max length of command is STL_MAX_COMMAND_LENGTH (currently 3 bytes)
GtCmdListRCursor	RES	1
GtCmdListWCursor	RES	1
GtCmdListNum		RES	1
GtCmdListBuffer		RES	STL_MAX_COMMAND_LENGTH*STL_MAX_COMMANDS_IN_LIST;length =3

GtExecuteAtTickWCursor	RES	1
GtExecuteAtTickRCursor	RES	1
GtISRExecuteAtTickRCursor RES	1
GtExecuteAtTickBuffer	RES	2*STL_MAX_COMMANDS_IN_LIST

GtExecuteCounter	RES	1
GtISRExecuteNumPending	RES	1	;timer tick matches found by isr
GtISRExecuteNumInBuff	RES	1


GSI_USER_CODE CODE

;-------------------------------------------------------------
;	GSIUserMain
;Called by library at the end of the libraries main polling loop
;where it checks for CAN messages, rs232 activity, keypad  etc.
;--------------------------------------------------------------
GSIUserMain

	banksel	GtBank
	;call	TestVol
gum_1

	#IF 0

	movlw	b'10110100'	;16 bit reads 1:8 prescalar
;1.6us per tick. max=104.856 ms (65535, 0=0 in this implementation)
	movwf	T3CON		;start

	;movf	POSTINC0,W
	;movff	POSTINC0,TMR3H
	;movwf	TMR3H

	;movff	INDF0,TMR3L
	;movf	INDF0
	;movwf	TMR3L
	movlw	0
	movwf	TMR3H
	movlw	1
	movwf	TMR3L

	bsf	T3CON,0
	
__ssd1x
	movf	TMR3L,W		;check for zero
	iorwf	TMR3H
	bnz	__ssd1x

	bra	gum_1


	banksel	R2Bank
	movlw	1
	movwf	R2RxRecordData+1	;flag=IMMEDIATE
	
	movlw	0
	movwf	R2RxRecordData+2	;chan


	lfsr	FSR0,R2RxRecordData+2
	call	STLCommandSetChannel

	bsf	ADCON0,ADON

gum1
	bsf	ADCON0,GO
gum2
	btfsc	ADCON0,GO	
	bra	gum2
	bra	gum1
	#endif	

	#IF 0
	movlw	1
	nop
	nop
	STARTLE_SET_CHAN WREG

	;movlw	0x40
	movlw	0xff
	movwf	GtTCL5615TxByte0
	movwf	GtTCL5615TxByte1

	SPI_CS_AMPLITUDE
	call	WriteTCL5615
	SPI_AMPLITUDE_STROBE

	movlw	0xa0		;b0
	movwf	GtTCL5615TxByte0
	movwf	GtTCL5615TxByte1

	SPI_CS_FREQUENCY
	call	WriteTCL5615
	SPI_FREQUENCY_STROBE

	call	GSIDelay50ms

	#ENDIF

	return

;-------------------------------------------------------------
;	GSIUserISRHP
;
;High priority interrupt call from library
;latency +18 clocks to arrive here
;------------------------------------------------------------
GSIUserISRHP
	#IF STL_HP_INT_ENABLED ==TRUE
;---------TMR1-------------------------------------------
;Check for ADC start 
;--------------------------------------------------------
;check TMR1 int, the ADC Start
	btfss	PIE1,TMR1IE
	bra	next_HP_int1
	btfsc	PIR1,TMR1IF
	bra	guif_tmr1		;TMR1 fired: Start conversion
	#ENDIF
next_HP_int1:	
guif_hp_exit:
	return

;-------------------------------------------------------------
;	GSIUserISRFIRST
;Called by library as it enters the low priority ISR
;18 clocks to arrive here
;BSR=GiBank, safe to change banks
;safe to use FSR2
;latency +19 cycles to arrive here
;--------------------------------------------------------------
 
GSIUserISRFirst
;---------ADIF----------------------------------------------
;check ADC int, ADC complete (started by TMR1)
;NOTE:
;we are using 1.6uSec conversion time. Probably inefficient to
;be using interrupts. Could poll instead? Was there a good reason
;why I used ISR's??
;Looks to be fast enough in any case as we acquire without error
;at 2.5kHz
;-------------------------------------------------------
	btfss	PIE1,ADIE
	bra	next_int1
	btfsc	PIR1,ADIF
	bra	guif_adc
next_int1
	#IF STL_HP_INT_ENABLED == FALSE
;---------TMR1-------------------------------------------
;Check for ADC start Low priority version
;--------------------------------------------------------
;check TMR1 int, the ADC Start
	btfss	PIE1,TMR1IE
	bra	next_int2
	btfsc	PIR1,TMR1IF
	bra	guif_tmr1		;TMR1 fired: Start conversion
	#ENDIF
;----------TMR2----------------------------------------------
;1msec timer
;--------------------------------------------------------
next_int2
	btfss	PIE1,TMR2IE
	bra	next_int3
	btfsc	PIR1,TMR2IF
	bra	guif_tmr2
next_int3
	#if STL_TIMED_TONE_ENABLE
;--------TMR0--------------------------------------------
;Check TIMED_TONE OFF
;--------------------------------------------------------
	btfss	INTCON,TMR0IE
	bra	next_int4
	btfsc	INTCON,TMR0IF
	bra	guif_tmr0
next_int4
	#endif ;STL_TIMED_TONE_ENABLE
;------------------------------------------------------
;Exit ISR
;------------------------------------------------------
guif_exit:
;If acquiring ADC data then return with C flag set to abort any further
;processing by library
	btfss	GlbGtStatus,STL_STATUS_ADC_ACQUIRING
	bra	guif_exit_NC	;allow gsi to further process

	bsf	STATUS,C	;cause an early return from int routine
	return
guif_exit_NC
	bcf	STATUS,C
	return

;---------------------------------------------------------
;	ISR's
;---------------------------------------------------------

	#IF STL_HP_INT_ENABLED ==TRUE
;--------------TMR1 handler----------------------------
; 	HIGH PRIORITY INT version
;------------------------------------------------------
;2.5kHz timer: begin new ADC
;Note: this might be a high priority int
;latency +22cycles to arrive here
;------------------------------------------------------
guif_tmr1:
	bsf	PIE1,ADIE	;re-enable ADC complete int
	bsf	ADCON0,GO	;

	;btg	STL_TEST_POINT
	

#IF STL_DEBUG		
	;Test to see if any collisions between scheduler (1msec isr)
	;and ADC. Result : ok
	;btfsc	GlbGtStatus,STL_STATUS_ADC_DATA_READY
	;btg	STL_TEST_POINT
#ENDIF

;-----------------------------------------------------------
;Set ADC timebase to correct value (2.5kHz is current)
;-----------------------------------------------------------	
	;movlw	HIGH (0xffff-(STL_TMR1_VALUE-STL_TMR1_HP_LATENCY))	;1000 *200ns =5kHz
	movlw	HIGH STL_TMR1_VALUE	;1000 *200ns =5kHz
	movwf	TMR1H						;2000 *200ns =2.5kHz
	;movlw	LOW (0xffff-(STL_TMR1_VALUE-STL_TMR1_HP_LATENCY))
	movlw	LOW STL_TMR1_VALUE
	movwf	TMR1L		;latency=28 cycles
	bcf	PIR1,TMR1IF	;remove flag

	bra	guif_hp_exit

	#ELSE	;STL_HP_INT_ENABLED == TRUE	
;-------------------------------------------------------
;	LOW PRIORITY INT version
;latency +27
;-------------------------------------------------------
guif_tmr1
	bsf	PIE1,ADIE	;re-enable ADC complete int
	bsf	ADCON0,GO	;
;-----------------------------------------------------------
;Set ADC timebase to correct value (2.5kHz is current)
;-----------------------------------------------------------	
	;movlw	HIGH (0xffff-(STL_TMR1_VALUE-STL_TMR1_LATENCY))	;1000 *200ns =5kHz
	movlw	HIGH STL_TMR1_VALUE	;1000 *200ns =5kHz
	movwf	TMR1H						;2000 *200ns =2.5kHz
	;movlw	LOW (0xffff-(STL_TMR1_VALUE-STL_TMR1_LATENCY))
	movlw	LOW STL_TMR1_VALUE
	movwf	TMR1L		;latency 33 cycles
	bcf	PIR1,TMR1IF	;remove flag

	bra	guif_exit

	#ENDIF	;STL_HP_INT_ENABLED ==TRUE	

;----------ADC complete handler----------------------------
;New ADC data is in reg ADRESL:ADRESH
;Set STL_STATUS_ADC_DATA_READY flag for processing loop 
;data will be tx'd down rs232 by processing loop
;----------------------------------------------------------
guif_adc
	bcf	PIR1,ADIF	;No more ints from this byte!
;Check for usart still Txing. Set an error if so
	btfss	PIR1,TXIF			;PIR1 in BANK0
	bsf	GlbGtStatus,STL_STATUS_ERROR_R2_OVERFLOW

	bsf	GlbGtStatus,STL_STATUS_ADC_DATA_READY

	DEC16	GlbGtNumDataPoints	
	bnz	guif_exit
;--------------------------------------------
;ADC Data collection finished
;update flags
;--------------------------------------------
	bcf	PIE1,ADIE		;ADC complete disable
	bcf	PIR1,ADIF		;clr int flag
	bsf	GlbGtStatus,STL_STATUS_ADC_END	;Set status for processing loop

	bcf	T1CON,TMR1ON		;TMR1 off (ADC timebase)
	bcf	PIE1,TMR1IE		;disable TMR1 int
	bcf	PIR1,TMR1IF		;clr int flag
	bra	guif_exit	
;---------TMR0 ISR------------------------------
;Timed tone interrupt handler
;TMR0 increment is inhibited for two cycles after a load
;so for accuracy should -2 from cycle count
;-----------------------------------------------
	#if STL_TIMED_TONE_ENABLE

guif_tmr0
	bcf	INTCON,TMR0IF	;clear int
	STARTLE_TONE_OFF
	bcf	INTCON,TMR0IE	;and disable further ints
	bcf	T0CON,TMR0ON	;and turn off the timer
	bra	guif_exit
	#endif ;STL_TIMED_TONE_ENABLE

;------------TMR2 ISR --------------------------------
;1msec scheduler interrupt handler
;-----------------------------------------------------
guif_tmr2
	bcf	PIR1,TMR2IF	;clear int flag

	;btg	STL_TEST_POINT
;-----------------------------------------------------
;see if next QUEUED command is to be executed
;use GtCmdListRCursor for next item in queue
;-----------------------------------------------------
	banksel	GtBank		;library will swap banks
;We are storing data hbyte lbyte so do the same for the GtTickCounter
	movf	GtISRExecuteNumInBuff
	bz	guif_exit		;no more queued so abort

	incf	GtTickCounter+1,F		;inc low byte
	btfsc	STATUS,C
	incf	GtTickCounter,F		;inc high byte if rollover
;Need to see if we have executed all the stored items
;Note that the lfsr loads the address of GEATBuffer into FSR2
;whereas the movff moves the contents of GEATRCursor into FSR2L
;so we do not lfsr FSR2,Cursor
	lfsr	FSR2,GtExecuteAtTickBuffer	;load FSR2H:FSR2L
	movff	GtISRExecuteAtTickRCursor,FSR2L	;load FSR2L
guif_tmr2_l1	
	movf	POSTINC2,W		;H byte
	subwf	GtTickCounter,W		;H byte
	bnc	guif_exit		;TickCounter>BufferValue
	movf	POSTINC2,W		;L Byte
	subwf	GtTickCounter+1,W	;L byte
	bnc	guif_exit
;match: TickCounter >= ExecuteAtbuffer[]
	incf	GtISRExecuteNumPending	;ISR will execute this many commands
	;update cursor
	movff	FSR2L,GtISRExecuteAtTickRCursor
	;decf	GtISRExecuteNumInBuff
	decfsz	GtISRExecuteNumInBuff
	bra	guif_tmr2_l1

	bra 	guif_exit
;-------------------------------------------------------------
;	GSIUserISRLast
;Called by library as it is about to exit the low priority ISR
;--------------------------------------------------------------
GSIUserISRLast
	return

;---------------------------------------------------------
;	GSIUserInitInterrupts
;called after the GSI has init'd interrupts. User may now
;override.
;All low priority except TMR3 used for CAN SYNC. If no sync
;required or CAN disabled then TMR3 is usable by user
;--------------------------------------------------------
GSIUserInitInterrupts

	bcf	IPR2,TMR3IP	;no tmr3 HP
	bsf	IPR1,TMR1IP	;TMR1 HP	
	return



;-----------------------------------------------------------
;	GSIUserRestart
;Called by library after a reset
;User code can override the preset allocation of resources
;as long as GSI functions that use the resource are not called
;Should perform initialisation here
;-----------------------------------------------------------
GSIUserRestart

	banksel	GtBank
	#IF STL_DEBUG
	clrf	GlbGtDebug
	#endif
	clrf	GtChan
	clrf	GlbGtStatus
	clrf	GtCmdListNum
	movlw	GtCmdListBuffer
	movwf	GtCmdListRCursor	;commands in Queue
	movwf	GtCmdListWCursor

	clrf	GtTickCounter
	clrf	GtTickCounter+1
	clrf	GtExecuteCounter
	clrf	GtISRExecuteNumPending
	movlw	GtExecuteAtTickBuffer
	movwf	GtExecuteAtTickRCursor	;ExecuteAt buffer
	movwf	GtISRExecuteAtTickRCursor	;ExecuteAt buffer
	movwf	GtExecuteAtTickWCursor
#IF STARTLE_V2
#ELSE
	STARTLE_TONE_OFF
#ENDIF
	#IF  STARTLE
;Require 8 adc'c and 8 dio's but no lcd
;See variable GlbResources GlbResources1
;GSI_RESOURCE_LCD_BIT	EQU	0	;clr for no LCD
;GSI_RESOURCE_RS232_BIT	EQU	1	;clr for no rs232
;GSI_RESOURCE_I2C_BIT	EQU	2	;clr for no i2c
;GSI_RESOURCE_KEYPAD_BIT	EQU	3	;clr for no keypad
;GSI_RESOURCE_CAN_BIT	EQU	4	;clr for no CAN
;GSI_RESOURCE_DIO_BIT	EQU	5	;clr for no DIO
;GSI_RESOURCE_INT_BIT	EQU	6	;clr for no RB0,RB1 Interrupts
;GSI_RESOURCE_SPI_BIT	EQU	7	;clr for no SPI

;GSI_RESOURCE_1_DEBUG_BIT	EQU	0	;clr for no DEBUG code
;GSI_RESOURCE_1_ADC_BIT		EQU	1	
;----------------------------------------------------
;GSI_RESOURCE_LCD	EQU	1	;clr for no LCD
;GSI_RESOURCE_RS232	EQU	2	;clr for no rs232
;GSI_RESOURCE_I2C	EQU	4	;clr for no i2c
;GSI_RESOURCE_KEYPAD	EQU	8	;clr for no keypad
;GSI_RESOURCE_CAN	EQU	10h	;clr for no CAN
;GSI_RESOURCE_DIO	EQU	20h	;clr for no DIO
;GSI_RESOURCE_INT	EQU	40h	;clr for no RB0,RB1 Interrupts
;GSI_RESOURCE_SPI	EQU	80	;clr for no SPI

;GSI_RESOURCE_1_DEBUG	EQU	0h	;clr for no DEBUG code
;GSI_RESOURCE_1_ADC	EQU	1	;clr for no ADC's
;----------------------------------------------------

	movlw	GSI_RESOURCE_RS232 | GSI_RESOURCE_RS232 | GSI_RESOURCE_DIO  | GSI_RESOURCE_SPI 
	call	GSISetResources
	movlw	GSI_RESOURCE_1_DEBUG | GSI_RESOURCE_1_ADC |GSI_RESOURCES_1_LP_INT | GSI_RESOURCES_1_HP_INT
	call	GSISetResources1
;----------------------------------------------------------	
	clrf	LATD			;outputs off
	clrf	TRISD			;PORT D all outputs
;----------------------------------------------------------
#IF STARTLE_V2
#ELSE
	bcf	LATB,4			;TONE=OFF (MUTE=ON)
	bcf	TRISB,4			;PORTB,4=output.
	bcf	TRISB,1			;RTS (CTS for the pc)
	bcf	TRISB,5			;Testpoint
	;bcf	TRISB,3			;CAN Rx
	;bcf	TRISB,2			;CAN Tx
	clrf	LATB
	clrf	GtChan
#ENDIF
;--------TMR0---------------------------------------------	
	bcf	INTCON,TMR0IE		;disable TMR0
	bcf	INTCON,TMR0IF		;unlock
	;movlw	HIGH (0xffff-STL_TMR0_VALUE)	;1000 *200ns =5kHz
	;movwf	TMR0H
	;movlw	LOW (0xffff-STL_TMR0_VALUE)
	;movwf	TMR0L
;--------TMR1 ADC timebase----------------------------------	
	bcf	PIE1,TMR1IE		;disable TMR1
	bcf	PIR1,TMR1IF		;unlock
	;movlw	HIGH (0xffff-STL_TMR1_VALUE)	;1000 *200ns =5kHz
	movlw	HIGH STL_TMR1_VALUE	;1000 *200ns =5kHz
	movwf	TMR1H				;2000 *200ns=2.5kHz
	;movlw	LOW (0xffff-STL_TMR1_VALUE)
	movlw	LOW STL_TMR1_VALUE
	movwf	TMR1L

;-------TMR2 default mode= tick command scheduler
;--------TMR2 Acquire after tmr2 int-----------------------
	bcf	PIR1,TMR2IF		;unlock
	movlw	STL_TMR2_POSTSCALE | STL_TMR2_PRESCALE 
	movwf	T2CON
	bcf	T2CON,TMR2ON		;TMR2=ON
	clrf	TMR2
	movlw	STL_TMR2_VALUE
	movwf	PR2			;TMR2 Period 
	bsf	PIE1,TMR2IE		;enable TMR2 interrupts

;-----------------------------------------------------
#IF STARTLE_V2
#ELSE
	call	InitSPI		;SPI not required and inerferes with PORTC used for VOL control
#ENDIF
;init ADC

	call	InitADC
	;Setup TMR0
	movlw	b'00001000'
	movwf	T0CON		;TMR0 disabled but ready

#IF STARTLE_V2
	call	InitDDS		;f=0
	call	InitVol		;vol=max attenuation
#ELSE
;-----Turn off sound----------------
	movlw	0xff			;mosfet ON=sound off
	movwf	GtTCL5615TxByte1
	movwf	GtTCL5615TxByte0

	SPI_CS_AMPLITUDE		;2 clocks
	call	WriteTCL5615		;
	SPI_AMPLITUDE_STROBE		;2 clocks

	SPI_CS_AMPLITUDE_NOISE		;2 clocks
	call	WriteTCL5615		;
	SPI_AMPLITUDE_NOISE_STROBE	;2 clocks

;require a minimum frequency or else the sine generator
;will not start till we have selected a much higher frequency
;probably need to select a larger C value for oscillator??
	movlw	STL_MINIMUM_FREQUENCY_LSB
	movwf	GtTCL5615TxByte0
	movlw	STL_MINIMUM_FREQUENCY_MSB
	movwf	GtTCL5615TxByte1
	SPI_CS_FREQUENCY		;2 clocks
	call	WriteTCL5615		;
	SPI_FREQUENCY_STROBE		;2 clocks

#ENDIF 

	#ENDIF	;STARTLE

	return

;------------------------------------
InitADC
;------------------------------------
InitADC
	movlw	b'00101111'	;TRISA AN0-4
	iorwf	TRISA
	movlw	b'00000111'	;TRISX AN5-7
	iorwf	TRISE
	bcf	TRISC,0		;DAC CS bits LATC0-2
	bcf	TRISC,1
	bcf	TRISC,2
;Right justify; Fosc/32; 7 AN channels
	movlw	b'10000000'
	movwf	ADCON1
	return

;---------------------------------------------------------
;	InitSPI
;---------------------------------------------------------
InitSPI
#IF !STARTLE_V2		;V2 must disable SPI as it uses PORTC 3,4
	bcf	TRISC,5		;output
	bcf	TRISC,3		;output
	SPI_CS_NONE
	;bcf	TRISC,0		;output(CS_0 select tlc5615 F)
	;bcf	TRISC,1		;output	(CS_1 select tlc5615 A)
	bsf	TRISC,4		;input
	movlw	b'01000000'	;output on rising edge
	movwf	SSPSTAT	
	
	;clrf	SSPSTAT		;output on falling edge

	movlw	b'00000001'	;
	movwf	SSPCON1
	nop
	bsf	SSPCON1,SSPEN	;enable the spi
#ENDIF
	return

;------------------------------------------------------
;	WriteTCL5615
;------------------------------------------------------

WriteTCL5615
#IF !STARTLE_V2
	movff	GtTCL5615TxByte0,__GtTCL5615TxByte0
	movff	GtTCL5615TxByte1,__GtTCL5615TxByte1

	bcf	STATUS,C
	;shift left 2 positions
	rlcf	__GtTCL5615TxByte0
	rlcf	__GtTCL5615TxByte1
	bcf	STATUS,C
	rlcf	__GtTCL5615TxByte0
	rlcf	__GtTCL5615TxByte1

	movff	__GtTCL5615TxByte1,WREG
	andlw	b'00001111'	;first nybble must be 0
	movwf	SSPBUF		;write to the chip
wtcLoop
	btfss	SSPSTAT,BF
	bra	wtcLoop
	movf	SSPBUF,W	;retrieve dummy data (device is read only)	

	movff	__GtTCL5615TxByte0,WREG
	andlw	b'11111100'	;ensure lsb has last two bits reset	
	movwf	SSPBUF		;write to the chip
wtcLoop1
	btfss	SSPSTAT,BF
	bra	wtcLoop1
	movf	SSPBUF,W	;retrieve dummy data (device is read only)	
#ENDIF
	return


;-----------------------------------------------------------
;	STLAddRecordToList
;Add a received STL R2 Record to list. No Need to add the
;IMMEDIATE flag as this is implicitly QUEUE
;Example command
;	1=STL_COMMAND_SET_AMPLITUDE (in data[0])
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=msb amplitude
;		data[3]=lsb amplitude
;		data[4]=msb execute at tick
;		data[5]=lsb execute at tick

;R2RxRecordData=data[0]
;FSR0=data[2]
;BSR=R2Bank
;returns C if error

;GtCmdListRCursor	RES	1
;GtCmdListWCursor	RES	1
;GtCmdListNum		RES	1
;GtCmdListBuffer		RES	STL_MAX_COMMAND_LENGTH*STL_MAX_COMMANDS_IN_LIST;length =3

;Add data[0],data[2],data[3]
;So this function only stores 3 bytes per command.
;1) the command itself
;2) one 16 bit data item (chan/frequency/volume etc)
;The at_tick is stored in a separate buffer
;this is to simplify searching for the next record to execute.
;We can just iterate through the at_tick list and execute commands
;if they are <= the current tick
;-----------------------------------------------------------
STLAddRecordToList
	banksel	GtBank
	movlw	STL_MAX_COMMANDS_IN_LIST
	cpfslt	GtCmdListNum				
	bra	sartl_error
	;still space in queue, update num in list
	incf	GtCmdListNum		;keep track of num in list
	;set FSR1 to List write cursor
	movlw	HIGH GtBank	
	movwf	FSR1H
	movff	GtCmdListWCursor,FSR1L
	;-get CMD and store in list
	movff	R2RxRecordData,POSTINC1 ;data[0]->list[]
	;-get data (max of 2 bytes currently) at FSR0
	;Note: we skip over data[1] the QUEUE flag the queue buffer
	;only holds queued commands, don't need the flag
	movff	POSTINC0,POSTINC1	;data[2]->list[]
	movff	POSTINC0,POSTINC1	;data[3]->list[]
	;update write cursor	
	movff	FSR1L,GtCmdListWCursor
;now update ExecuteAtTickBuffer. FSR0=data[4]
	movff	GtExecuteAtTickWCursor,FSR1L
	movff	POSTINC0,POSTINC1	;data[4]->ExecuteAtTickBuffer[x]
	movff	POSTINC0,POSTINC1	;data[5]->ExecuteAtTickBuffer[x]
	;update write cursor	
	movff	FSR1L,GtExecuteAtTickWCursor

	;banksel	R2Bank	
	;call	R2SendProcessRecordSuccessFail
#if STARTLE_V2
;If set frequency in V2 we mustn't send an STL_SEND_SUCCESS_FAIL
;as this is a double record and we will end up sending 2 SUCCESS_FAIL
;pc will only be looking for 1
	movff	R2RxRecordData,WREG			;get record type
	sublw	STL_COMMAND_SET_FREQUENCY
	bz		no_send_success_fail_exit
#endif
	STL_SEND_SUCCESS_FAIL
no_send_success_fail_exit:
	bcf	STATUS,C
	return
sartl_error
	;ERROR_RANGE
	;set length of returned record
	banksel	R2Bank
	movlw	2
	movwf	R2SubfunctionRecordLength
	;-send back a fail
	movlw	GSI_RS232_MESSAGE_FAIL
	movwf	R2SubfunctionRecordData
	;send back the cause of fail
	movlw	STL_ERR_RANGE
	movwf	R2SubfunctionRecordData+1
	;call	R2SendProcessRecordSuccessFail
	STL_SEND_SUCCESS_FAIL
	bsf	STATUS,C
	return

;-------------------------------------------------------
;	Record0Received
;Rs232 received a Record0 a User rs232 binary record
;R2RxRecordType=44 decimal id OF RT_USER_0
;R2RxRecordNode=CAN_NODE_THIS (for this version, no CAN network)
;R2RxRecordLength=0-14 
;R2RxRecordData[]  the data 0-14 bytes
;BSR=R2Bank
;An R2Record SUCCESS/FAIL has been filled out with SUCESS
;These functions can write to the record, adding data if
;required and sending a FAIL if required.
;Remember to update Length byte if additions made!
;
;within R2RxRecordData:
;R2RxRecordData[0]=STL_COMMAND

;data[0]=STL_COMMAND_XXX
;STL_COMMAND_RESET		EQU	0

;STL_COMMAND_SET_AMPLITUDE	EQU	1
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb amplitude
;	data[3]=lsb amplitude
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

;Ver1:
;STL_COMMAND_SET_FREQUENCY	EQU	2
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb frequency
;	data[3]=lsb frequency
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

;Ver2:
;STL_COMMAND_SET_FREQUENCY	EQU	2
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb frequency
;	data[3]=lsb frequency
;	data[4]=msb frequency
;	data[5]=lsb frequency
;	data[6]=msb tick to execute on (if QUEUED)
;	data[7]=lsb tick to execute on (if QUEUED)


;STL_COMMAND_SET_DELAY		EQU	3
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb TICK (TICK=10us??)
;	data[3]=lsb TICK
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

;STL_COMMAND_ACQUIRE		EQU	4
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2] msb number data points
;	data[3] lsb number data points
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

;STL_COMMAND_SET_CHANNEL		EQU	5
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=channel 0-7
;	data[3]=msb tick to execute on (if QUEUED)
;	data[4]=lsb tick to execute on (if QUEUED)

;STL_COMMAND_EXECUTE		EQU	6
;		always IMMEDIATE

;STL_COMMAND_TONE		EQU	7
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=ON/OFF; OFF=0 ON=1
;	data[3]=msb tick to execute on (if QUEUED)
;	data[4]=lsb tick to execute on (if QUEUED)

;STL_COMMAND_TIMED_TONE_ON		EQU	8
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb
;	data[3]=lsb
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)
;
;STL_COMMAND_SET_NOISE_AMPLITUDE	EQU	9
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb amplitude
;	data[3]=lsb amplitude
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)


;ex:
;
;reset	(amplitude 0,frequency 0,channel 0)
;set channel(0)		;chan 0-7
;set amplitude (50)
;set frequency (5000)
;set Delay (5msec)	;5kHz for 1000 ticks (pre-pulse)
;set amplitude (1000)
;set delay(50ms)
;set amplitude(0)
;Execute

;Probably need a 20ms settling time for the relay
;thus channel set is an implicit 20ms delay

;2RxRecordData[1]=STL_DATA_LENGTH
;2RxRecordData[2-13]=data (if any)

;-------------------------------------------------------
STLRecord0Received
	;ignore the Node
;Load fsr0 in case we are an immediate operand
	lfsr	FSR0,R2RxRecordData+2

	movf	R2RxRecordData,W	;get command
	bz	STLCommandReset
	decf	WREG	
	bz	STLCommandSetAmplitude
	decf	WREG
	btfsc	STATUS,Z
	goto	STLCommandSetFrequency
	decf	WREG
	btfsc	STATUS,Z
	goto	STLCommandSetDelay
	decf	WREG
	btfsc	STATUS,Z
	goto	STLCommandAcquire
	decf	WREG
	btfsc	STATUS,Z
	goto	STLCommandSetChannel
	decf	WREG
	btfsc	STATUS,Z
	goto	STLExecute	;always immediate so bra directly
	decf	WREG
	btfsc	STATUS,Z
	goto	STLCommandSetTone
	decf	WREG
	btfsc	STATUS,Z
	goto	STLCommandTimedToneOn
	decf	WREG
	btfsc	STATUS,Z
	goto	STLCommandSetNoiseAmplitude
	decf	WREG
	btfsc	STATUS,Z
	goto	STLCommandGetStatus
	decf	WREG
	btfsc	STATUS,Z
	goto	STLCommandSetFrequencyUpper

	;btfsc	STATUS,Z
	;goto	STLCommandTimedAcquisition

	;ERROR_RANGE
	;set length of returned record
	movlw	2
	movwf	R2SubfunctionRecordLength
	;-send back a fail
	movlw	GSI_RS232_MESSAGE_FAIL
	movwf	R2SubfunctionRecordData
	;send back the cause of fail
	movlw	STL_ERR_RANGE
	movwf	R2SubfunctionRecordData+1
	STL_SEND_SUCCESS_FAIL
	;call	R2SendProcessRecordSuccessFail
	return

;-----------------------------------------------------------
;	STLCommandReset
;Turn off the amp. Reset the SPI etc.
;Was always an immediate command. Doesn't work properly because
;the timed tone and the acquistion is an asynchronous event and
;the caller might inadvertently call before acq or tone has
;completed
;	data[0]=STL_COMMAND_RESET
;	data[1]=QUEUE=0; IMMEDIATE=1
;-----------------------------------------------------------
STLCommandReset
	
	#IF 0
	lfsr	FSR0,R2RxRecordData+2	;ms to execute at field
	movf	R2RxRecordData+1	;QUEUE flag
	bnz	scr_immediate
	;add to list
	call	STLAddRecordToList
	return
scr_immediate
	#ENDIF

	STL_SEND_SUCCESS_FAIL
	;call	STLReset
	call	GSIUserRestart
	return

;-----------------------------------------------------------
;	STLCommandSetAmplitude
;	1=STL_COMMAND_SET_AMPLITUDE (in data[0])
;		data[0]=STL_COMMAND_SET_AMPLITUDE	
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=msb amplitude
;		data[3]=lsb amplitude
;		data[4]=msb execute at tick (if queued)
;		data[5]=lsb execute at tick (if queued)

;-----------------------------------------------------------
STLCommandSetAmplitude

	lfsr	FSR0,R2RxRecordData+2
	movf	R2RxRecordData+1	;
	bnz	scsa_immediate
	;add to list
	call	STLAddRecordToList
	return
scsa_immediate
	;call	R2SendProcessRecordSuccessFail
	STL_SEND_SUCCESS_FAIL
	call	STLSetAmplitude
	return

;-----------------------------------------------------------
;A special 'double' record that contains the 4 bytes for DDS
;frequency control

;		data[0]=STL_COMMAND_SET_FREQUENCY
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=msb frequency (DDS byte1)
;		data[3]=lsb frequency (DDS byte0)
;		data[4]=msb execute at tick (if queued)
;		data[5]=lsb execute at tick (if queued)

;		data[6]=STL_COMMAND_SET_FREQUENCY_UPPER
;		data[7]=QUEUE=0; IMMEDIATE=1
;		data[8]=msb frequency (DDS byte3)
;		data[9]=lsb frequency (DDS byte2)
;		data[10]=msb execute at tick (if queued)
;		data[11]=lsb execute at tick (if queued)

;-----------------------------------------------------------
STLCommandSetFrequency
	lfsr	FSR0,R2RxRecordData+2 ;point to msb param0
	movf	R2RxRecordData+1	;get queued flag
	bnz	scsf_immediate
	;add to list
	call	STLAddRecordToList
;NOTE this Queued code has not been tested! (lines up to return)
;STLAddToList uses hard-coded R2RxRecord address. So must move from data[6]->data[0]
;Only need to move 3 items, the rest will be the same
	lfsr	FSR0,R2RxRecordData+2 ;point to msb param0
	movff	R2RxRecordData+6,R2RxRecordData+0	;SET_FREQUENCY_UPPER
	movff	R2RxRecordData+8,R2RxRecordData+2	;byte3
	movff	R2RxRecordData+9,R2RxRecordData+3	;byte2
	call	STLAddRecordToList
	return
scsf_immediate
#IF DDS_FREQUENCY_RECORD
	;call	R2SendProcessRecordSuccessFail
	STL_SEND_SUCCESS_FAIL
	call	STLSetFrequency
	lfsr	FSR0,R2RxRecordData+8 ;point to msb param0
	call	STLSetFrequencyUpper
	
#ELSE
	STL_SEND_SUCCESS_FAIL
	call	STLSetFrequency
#ENDIF
	return

;-----------------------------------------------------------
;	STLCommandSetDelay
;		data[0]=STL_COMMAND_SET_DELAY
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=msb delay
;		data[3]=lsb delay
;		data[4]=msb execute at tick (if queued)
;		data[5]=lsb execute at tick (if queued)
;-----------------------------------------------------------
STLCommandSetDelay
	lfsr	FSR0,R2RxRecordData+2

	movf	R2RxRecordData+1	;
	bnz	scsd_immediate
	;add to list
	call	STLAddRecordToList
	return
scsd_immediate
	;call	R2SendProcessRecordSuccessFail
	STL_SEND_SUCCESS_FAIL
	return

;-----------------------------------------------------------
;	STLCommandAcquire
;		data[0]=STL_COMMAND_ACQUIRE
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=msb num data points
;		data[3]=lsb num data poins
;		data[4]=msb execute at tick (if queued)
;		data[5]=lsb execute at tick (if queued)

;-----------------------------------------------------------
STLCommandAcquire

	movff	POSTINC0,GlbGtNumDataPoints
	movff	INDF0,GlbGtNumDataPoints+1

	lfsr	FSR0,R2RxRecordData+2

	movf	R2RxRecordData+1	;
	bnz	sca_immediate
	;add to list
	call	STLAddRecordToList
	bc	sca_1
	;an error occurred, don't set flag
	bsf	GlbGtStatus,STL_STATUS_ADC_QUEUED
sca_1

	return
sca_immediate
;VB has sent an ACQUIRE COMMAND.
;we respond with an rs232 record
;VB will set up for fast acquisition
;when ready it sends an RT_ACK and pauses for one timer tick
;to ensure we are ready
;We echo the ACK and start the acquisition
	;call	R2SendProcessRecordSuccessFail
	STL_SEND_SUCCESS_FAIL
;disable Rx int
	bcf	PIE1,RCIE
	bcf	PIR1,RCIF
;look for ACK from pc
scai_1
	btfss	PIR1,RCIF
	bra	scai_1
	movlw	RT_GSI_ACK
	cpfseq	RCREG
	bra	scai_1

	;have received ACK from pc 
scai_2
	btfss	PIR1,TXIF			;PIR1 in BANK0
	bra	scai_2			
	;WREG still holds  ACK
	movwf	TXREG

scai_4
	btfss	PIR1,TXIF			;PIR1 in BANK0
	bra	scai_4			
	;
	call	STLAcquire

	return

;-----------------------------------------------------------
;	STLCommandSetChannel
;In order to keep consistent timing we need to calculate the 
;bit for the channel and place it into the record here, rather
;than calculate at runtime.
;Relay settle time is probably 2-5ms, allow 10ms
;	5=STL_COMMAND_SET_CHANNEL
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=channel 0-7 (255 means all OFF IMMEDIATE only)
;We will add    data[3]=channel bit position (calculated here)
;		data[4]=msb execute at tick (if queued)
;		data[5]=lsb execute at tick (if queued)

;We will add data[3]=channel bit position
;FSR0=R2RxRecordData+2
;BSR=R2Bank
;-----------------------------------------------------------
STLCommandSetChannel
	;increase length as we will add channel bit pos
	movlw	4
	movwf	R2RxRecordLength
	banksel	GtBank
;special case channel of 255 means all channels off
	movlw	255
	cpfslt	INDF0	;skip if f<W
	bra	scsc_all_off
	
;ensure consistency of data. Mask out > chan 7
	movlw	7	;max channel
	andwf	INDF0	;ensure no invalid chans
;-calculate channels bit position
	movff	INDF0,__GtTemp	;store chan value
	movlw	1
	movwf	__GtChan
	movf	__GtTemp,W
	bz	__scsc1
__l1:	
	rlncf	__GtChan
	decfsz	__GtTemp
	bra	__l1	
__scsc1:
	movff	__GtChan,PREINC0	;replace CHAN with bit pos
	banksel	R2Bank
	movf	R2RxRecordData+1,W	;
	bnz	scsc_immediate
	;add to list
	lfsr	FSR0,R2RxRecordData+2
	call	STLAddRecordToList
	return
scsc_immediate
	;call	R2SendProcessRecordSuccessFail
	STL_SEND_SUCCESS_FAIL
	lfsr	FSR0,R2RxRecordData+2		;set explicitly for immediate
	call	STLSetChan		;as it points to +3 after chan bit pos calculation
	return
scsc_all_off
	STL_SEND_SUCCESS_FAIL
	clrf	LATD			;all off
	return
;-----------------------------------------------------------
;	STLCommandExecute
;always an immediate command
;-----------------------------------------------------------
;STLCommandExecute
;	bra	STLExecute	

;-----------------------------------------------------------
;	STLCommandTone
;
;	STL_COMMAND_TONE		
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=ON/OFF; OFF=0 ON=1
;		data[0]=STL_COMMAND_TONE
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=ON/OFF
;		data[3]=dummy
;		data[4]=msb execute at tick (if queued)
;		data[5]=lsb execute at tick (if queued)

;-----------------------------------------------------------
STLCommandSetTone
	lfsr	FSR0,R2RxRecordData+2

	movf	R2RxRecordData+1	;
	bnz	sct_immediate
	;add to list
	call	STLAddRecordToList
	return
sct_immediate
	;call	R2SendProcessRecordSuccessFail
	STL_SEND_SUCCESS_FAIL
	call	STLSetTone
	return


;-----------------------------------------------------------
;	STLCommandTimedToneOn
;
;	STL_COMMAND_TIMED_TONE_ON
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb
;	data[3]=lsb
;	data[4]=prescalar (0-7, 1:2->1:256)
;	data[5]=msb execute at tick (if queued)
;	data[6]=lsb execute at tick (if queued)

;Using max prescalar we have 51.2us ticks
;Must move data[5],data[6] to data[4],data[5]
;-----------------------------------------------------------
STLCommandTimedToneOn

	lfsr	FSR0,R2RxRecordData+2
	#if STL_TIMED_TONE_ENABLE
	movlw	7			;prescaler mask
	andwf	R2RxRecordData+4,W	;ensure no error
	movwf	T0CON			;set prescaler

	movf	R2RxRecordData+1	;
	bnz	sctto_immediate
;place the ExecuteAtTick data in correct position
	movff	R2RxRecordData+5,R2RxRecordData+4
	movff	R2RxRecordData+6,R2RxRecordData+5
	;add to list

	call	STLAddRecordToList
	return
sctto_immediate
	;call	R2SendProcessRecordSuccessFail
	STL_SEND_SUCCESS_FAIL
	call	STLTimedToneOn
	return
	#else	;STL_TIMED_TONE_ENABLE
;-------------------------------------------------
;Timed tone is disabled, so send back error
;-------------------------------------------------
	banksel	R2Bank
	movlw	2
	movwf	R2SubfunctionRecordLength
	;-send back a fail
	movlw	GSI_RS232_MESSAGE_FAIL
	movwf	R2SubfunctionRecordData
	;send back the cause of fail
	movlw	STL_ERR_FUNCTION_UNSUPPORTED
	movwf	R2SubfunctionRecordData+1
	;call	R2SendProcessRecordSuccessFail
	STL_SEND_SUCCESS_FAIL
	bsf	STATUS,C
	return
	#endif	;STL_TIMED_TONE_ENABLE

;-----------------------------------------------------------
;	STLCommandSetNoiseAmplitude
;	1=STL_COMMAND_SET_NOISE_AMPLITUDE (in data[0])
;		data[0]=STL_COMMAND_SET_AMPLITUDE	
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=msb amplitude
;		data[3]=lsb amplitude
;		data[4]=msb execute at tick (if queued)
;		data[5]=lsb execute at tick (if queued)

;-----------------------------------------------------------
STLCommandSetNoiseAmplitude

	lfsr	FSR0,R2RxRecordData+2
	movf	R2RxRecordData+1	;
	bnz	scsna_immediate
	;add to list
	call	STLAddRecordToList
	return
scsna_immediate
	;call	R2SendProcessRecordSuccessFail
	STL_SEND_SUCCESS_FAIL
	call	STLSetNoiseAmplitude
	return

;-----------------------------------------------------------
;	STLCommandSetGetStatus
;		data[0]=STL_COMMAND_GET_STATUS
;Return current state of the pic
;FSR0=R2RxRecordData+2
;BSR=R2Bank
;GtExecuteCounter holds number of commands in execute list
;Sends back 3 bytes
;data[0]=success/fail as usual
;data[1]=GlbGtStatus
;data[2]=TMR0ON flag (not used V2)
;data[3]=GtExecuteCounter (number of queued actions still waiting) 
;-----------------------------------------------------------
STLCommandGetStatus
	movlw	4			;1 byte success/fail
					;2 bytes status
	movwf	R2SubfunctionRecordLength
	;-send back status
	movff	GlbGtStatus,R2SubfunctionRecordData+1

;send back tmr0 on/off bit as bit0 of 2nd status byte
	clrf	WREG
	#IF STL_TIMED_TONE_ENABLE
	btfsc	T0CON,TMR0ON	;is timer 0 on (timed tone is on)
	bsf	WREG,0	
	#ENDIF
	movff	WREG,R2SubfunctionRecordData+2

	movff	GtExecuteCounter,R2SubfunctionRecordData+3

	STL_SEND_SUCCESS_FAIL
	return

#if STARTLE_V2
;-----------------------------------------------------------
;		data[0]=STL_COMMAND_SET_FREQUENCY_UPPER
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=msb frequency	byte3
;		data[3]=lsb frequency	byte2
;		data[4]=msb execute at tick (if queued)
;		data[5]=lsb execute at tick (if queued)

;-----------------------------------------------------------
STLCommandSetFrequencyUpper
	lfsr	FSR0,R2RxRecordData+2

	movf	R2RxRecordData+1	;
	bnz	scsfu_immediate
	;add to list
	call	STLAddRecordToList
	return
scsfu_immediate
	;call	R2SendProcessRecordSuccessFail
	STL_SEND_SUCCESS_FAIL
	call	STLSetFrequencyUpper
	return
#endif

;------------------------------------------------------------

;-----------------------------------------------------------
;		RunTime and Immediate time functions
;perfom the task in FSR0 which points to a R2RxRecordData+2 struct
;Note that we need consistent timing for all these functions
;-----------------------------------------------------------

;---------------------------------------------------------
;	STLReset
;	1=STL_COMMAND_RESET
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=STL_UNCONDITIONAL_RESET=0; STL_ALLOW_COMPLETION_RESET
;		data[3]=

;FSR0=data[2]
;BSR=GtBank or R2Bank
;If we are called from the queue then we must ensure that
;the timed tone has completed *if* STL_ALLOW_COMPLETION_RESET
;if STL_UNCONDITIONAL_RESET then we simple reset

;Have implemented the timed tone finished problem by use of 
;STLStatus. The dll will call STLStatus in a timeout loop
;looking for a completion code. If not forthcoming it will
;report the error.
;---------------------------------------------------------
	#IF 0
STLReset
	movf	INDF0,W		;get the flag
	bz	sr_unconditional
	decf	INDF0,W
	bnz	sr_error
;Allow Completion reset. Wait for ADC and TIMED_TONE to complete

sr_unconditional
	call	GSIUserRestart
	return
sr_error
;?? flag the error so that caller can call STL_LAST_ERROR to find
;out if there were any failures during the execution of queued cmds
;mustn't use STL_SEND_SUCCESS_FAIL during the queue as the dll
;will not be expecting it
	#IF 0
	;ERROR_RANGE
	;set length of returned record
	banksel	R2Bank
	movlw	2
	movwf	R2SubfunctionRecordLength
	;-send back a fail
	movlw	GSI_RS232_MESSAGE_FAIL
	movwf	R2SubfunctionRecordData
	;send back the cause of fail
	movlw	STL_ERR_RANGE
	movwf	R2SubfunctionRecordData+1
	STL_SEND_SUCCESS_FAIL
	#ENDIF

;Although we have an incorrect flag, best to reset in any case
	call	GSIUserRestart

	return
	#ENDIF
;---------------------------------------------------------
;	STLSetAmplitude

;	1=STL_COMMAND_SET_AMPLITUDE
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=msb amplitude
;		data[3=lsb amplitude
;FSR0=data[2]
;BSR=GtBank or R2Bank
;---------------------------------------------------------
STLSetAmplitude
;perfom the task in FSR0 which points to a R2RxRecordData+2 struct	
;NOTE: this can be called by ExecuteFn so might not be R2RxRecord
;will be in GtCmdListBuffer
#if STARTLE_V2
	banksel	GtBank
	movlw	VOL_SINE		
	movwf	GtVolChanByte			;set channel to sine chan
;load both bytes even though we actually only use the lsb for V2
	movff	POSTINC0,GtVolDBByte	;throw this away
	movff	INDF0,GtVolDBByte		;use this
	call	SendVol16
#ELSE
	banksel	GtBank
	movff	POSTINC0,GtTCL5615TxByte1
	movff	INDF0,GtTCL5615TxByte0
	SPI_CS_AMPLITUDE		;2 clocks
	call	WriteTCL5615		;
	SPI_AMPLITUDE_STROBE		;2 clocks
#ENDIF
	banksel	R2Bank	
	return

;---------------------------------------------------------
;	STLSetFrequency

;	2=STL_COMMAND_SET_FREQUENCY
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=msb frequency
;		data[3=lsb frequency
;FSR0=data[2]
;BSR=R2Bank
;Also called from STLExecute jmp table
;----------------------------------------------------------
STLSetFrequency
;perfom the task in FSR0 which points to a R2RxRecordData+2 struct	
	banksel	GtBank
#if STARTLE_V2
	movff	POSTINC0,GtDDSByte1
	movff	INDF0,GtDDSByte0		;do nothing till STLSetFrequencyUpper rec'd
#else
	movff	POSTINC0,GtTCL5615TxByte1
	movff	INDF0,GtTCL5615TxByte0
	SPI_CS_FREQUENCY		;2 clocks
	call	WriteTCL5615		;
	SPI_FREQUENCY_STROBE		;2 clocks
#endif
	banksel	R2Bank
	return

;-----------------------------------------------------------
;	STLSetDelay
;8 clocks + delay

;	3=STL_COMMAND_SET_DELAY
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=msb TICK (TICK=1.6us, max of 65535)
;		data[3]=lsb TICK
;FSR0=data[2]
;BSR=R2Bank
;perfom the task in FSR0 which points to a R2RxRecordData+2 struct
;-----------------------------------------------------------
STLSetDelay
	;Use a software loop checking the TMRx
	;should also take into account the setup time??
	;probably best to allow the caller to do this where
	;the sums are easier

	;write TMR3L then TMR3H=16 bit transfer.
	;read from TMR3L will also read into TMR3H

	movlw	b'10110101'	;8 bit reads 1:8 prescalar
;1.6us per tick. max=104.856 ms (65535, 0=0 in this implementation)
	movwf	T3CON		;start
	movff	POSTINC0,TMR3H
	movff	INDF0,TMR3L

__ssd1
	movf	TMR3L,W		;check for zero
	iorwf	TMR3H
	bnz	__ssd1

	;btg	STL_TEST_POINT			

	return

#if STARTLE_V2
;---------------------------------------------------------
;	STLSetFrequencyUpper

;	2=STL_COMMAND_SET_FREQUENCY
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=msb frequency
;		data[3=lsb frequency
;FSR0=data[2]
;BSR=R2Bank
;Also called from STLExecute jmp table
;----------------------------------------------------------
STLSetFrequencyUpper
;perfom the task in FSR0 which points to a R2RxRecordData+2 struct	
	banksel	GtBank
	movff	POSTINC0,GtDDSByte3
	movff	INDF0,GtDDSByte2		
	call	WriteDDSWord
	banksel	R2Bank
	return
#endif

;-----------------------------------------------------------
;		STLAcquire
;
;	4=STL_COMMAND_ACQUIRE
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2] msb number data points
;		data[3] lsb number data points
;		data[4] msb ExecuteAt tick
;		data[5] lsb ExecuteAtTick
;FSR0=data[2]
;BSR=R2Bank
;Acquire data and send it. Try it unbuffered?  
;send back a FAIL if problem encountered
;GtNumDataPointsH/L/Count holds num to collect

;Important:
;We will attempt to interleave the 1msec timer int with the adc int
;we will tf run the adc clock at a multiple of the 1msec so that they
;interleave. So, run the adc at 2.5kHz BUT do not start the ADC clock
;till we are at 100usec into the 1msec. This way the ADC's will fall
;on 500us,900us,1300us,1700us,2100,2500,2900,3300 etc. So the two
;ints will never collide. We must be very accurate with our instruction
;timings for this to work, else at some point the two ints will coincide
;and cause errors till they diverge again.

	;VB side
;'Receiving ADC data from pic node
;'Send the record and check for error in the returned RS232Record
;'The pic will suspend normal processing, awaiting a second ACK
;'from this code.
;'On receipt of the ACK it will acknowledge and proceed with the
;'acquisition, sending raw binary data to this code, ignoring
;'any echo.
;'The pic code will replace the interrupt handler with the
;'ADC specific code to gain the necessary speed. The second ACK
;'that the pic echoes is handshaking, letting the pc know it
;'is ready to roll.
;'Before sending the second ACK this code needs to set up its
;'own interrupt servicing ready for the data stream.
;'At the end of the data, the pic will send an ACK, this code
;'will return to default behaviour, send an ACK to let the pic
;'know this has been done. The pic will also return to default
;'behaviour
;'Errors will probably give rise to a WDT reset on the pic
;NOTE:
;Also called from STLExecute jmp table
;approx 20 instructions to arrive here from exec loop
;or 4us
;-----------------------------------------------------------
STLAcquire
	;disable Rx
	banksel	GtBank		;required for DespatchLoop

	bcf	PIE1,RCIE

;INTERLEAVE the 1msec timer with this interrupt
;wait till 1msec timer is 100usec into its count before starting ADC
;delay here till TMR1=0xffff-4500(4500*200ns=900uS)
;in 16 bit mode must read TMR1L, which also fills TMR1H
;STL_TMR2_PRESCALE	=	b'00000001'	;4
;STL_TMR2_POSTSCALE	=	b'00100000'	;5
;STL_TMR2_VALUE		=	249		;250*4*5=5000*200e-9=1ms
;100usec will be a value of TMR2H=0, TMR1L=100, 100*5 (postscale)
;Postscaler is the number of Periods that must fire before an int is triggered
;in other words 250*postcaler. Thus if we look at TMR2 it will
;have a period of 250*4*200ns=200uS, thus 125 is 100uS
;Note on arrival here the TMR2 count is within 2uS of 100uS so we
;probably don't need to do this!
	;movlw	0
	;movwf	TMR2

	;btg	STL_TEST_POINT
;interleave_loop
	;movf	TMR2,F	
;	movlw	125	;TMR2_VALUE is a period. TMR2 increments till it reaches this point
;	cpfseq	TMR2
;	bra	interleave_loop

	;btg	STL_TEST_POINT
	;enable ADC module 

	bsf	ADCON0,ADON
	bcf	PIR1,ADIF
	bsf	PIE1,ADIE

;-------Load the ADC timer value--------------------
	movlw	HIGH STL_TMR1_VALUE		;1000 *200ns =5kHz
	movwf	TMR1H				;2000 *200ns =2.5kHz
	movlw	LOW STL_TMR1_VALUE
	movwf	TMR1L
	bsf	T1CON,TMR1ON

;-------Start acquisition with 100uSec on the timer--------
	bsf	GlbGtStatus,STL_STATUS_ADC_ACQUIRING
;Start first conversion	
	bcf	PIR1,TMR1IF	;clear int flag

	bsf	ADCON0,GO	;start first conversion
;-
	bsf	PIE1,TMR1IE	;enable int

sa_1
	clrwdt
	movf	GtISRExecuteNumPending,W
	bz	sa_check_adc
	call	AcquireDespatchLoop	
;---------check for data ready-----------------------
sa_check_adc	
	btfss	GlbGtStatus,STL_STATUS_ADC_DATA_READY
	bra	sa_1
;-----------Data is in the ADC reg's-----------------
	btfss	GlbGtStatus,STL_STATUS_TONE_START	
	bra	sa_2		;No tone started during this acq
;Tone has begun during this acq, so set data to min
	bcf	GlbGtStatus,STL_STATUS_TONE_START
	movlw	0
	movwf	ADRESL
	movwf	ADRESH	
;--------branch point for no-tone-started acquisition----------------
	;send low byte	
sa_2
	btfss	PIR1,TXIF			;PIR1 in BANK0
	bra	sa_2			
	movff	ADRESL,TXREG
	call	STLR2TxDelay		;req'd for VB
	;wait till sent
sa_3
	btfss	PIR1,TXIF			;PIR1 in BANK0
	bra	sa_3			
	;send high byte
	movff	ADRESH,TXREG

	bcf	GlbGtStatus,STL_STATUS_ADC_DATA_READY

	btfss	GlbGtStatus,STL_STATUS_ADC_END
	bra	sa_1		;loop till all sent
;ADC collection completed, so set flag and cleanup
	MESSG ?? Send back FAIL in R2Record if required ??
	bcf	GlbGtStatus,STL_STATUS_ADC_END
	bcf	GlbGtStatus,STL_STATUS_ADC_ACQUIRING
	bcf	GlbGtStatus,STL_STATUS_ADC_QUEUED

;cleanup	
;look for ACK from pc, it has all the data
sa_4
	btfss	PIR1,RCIF
	bra	sa_4
	movlw	RT_GSI_ACK
	cpfseq	RCREG
	bra	sa_4

	nop
	movwf	TXREG		;echo the ACK

	;ok transfer complete. Return to normal

	bcf	PIR1,RCIF	
	bsf	PIE1,RCIE	;re-enable int

	return

AcquireDespatchLoop
;A timer2 triggered event needs handling
;GtISRExecuteNumPending is >0

	decf	GtISRExecuteNumPending	;remove event from queue
	movff	GtCmdListRCursor,FSR0L
	movlw	HIGH GtCmdListRCursor
	movwf	FSR0H
	;-Get CMD
	movf	POSTINC0,W	;point to data
	call	adl_jmp_tbl		;may modify FSR0 and BSR
	banksel	GtBank
	;increment the RCursor(s)
	movlw	STL_MAX_COMMAND_LENGTH
	addwf	GtCmdListRCursor,F
	incf	GtExecuteAtTickRCursor
	incf	GtExecuteAtTickRCursor
	decfsz	GtExecuteCounter
	return
;-list empty, so disable timer
	bcf	T2CON,TMR2ON		;turn off timer
	clrf	GtTickCounter		;clear the counter
	clrf	GtTickCounter+1

;-reset Cursors to point to start of list
	movlw	GtCmdListBuffer		;address of buffer
	movwf	GtCmdListRCursor	;commands in Queue
	movwf	GtCmdListWCursor
;same for ExecuteAt cursors 
	movlw	GtExecuteAtTickBuffer	;address of buffer
	movwf	GtExecuteAtTickRCursor
	movwf	GtExecuteAtTickWCursor
AcquireDespatchLoopExit
	return	
adl_jmp_tbl:
	;adjust for goto in jmp table
	addwf	WREG
	addwf	WREG
	;calculate computed offset
	movwf	GtCompOffset
	movlw 	LOW adl_exec_table 	;get low 8 bits of address
	addwf 	GtCompOffset,F	;do an 8-bit add operation
	movlw 	HIGH adl_exec_table 	;get high 5 bits of address
	btfsc 	STATUS,C,A 	;page crossed?
	addlw 	1 		;yes then increment high address
	movwf	PCLATH,A 		;load high address in latch
	movf	GtCompOffset,W	;load computed offset in w reg
	movwf	PCL
;-----------------------------------------------------------------
;Implement the jump table as a macro as we have two copies of it
;must be sure that both copies are the same!
;Need a 2nd copy in the ADC loop to ensure commands are despatched
;-----------------------------------------------------------------
adl_exec_table
	CMD_EXECUTE_IN_ACQUIRE_TABLE
	#if 0
	goto	DEBUGCauseWDT
	goto	STLSetAmplitude
	goto	STLSetFrequency
	goto	STLSetDelay
	goto	AcquireDespatchLoopExit	;goto	STLAcquire
	goto	STLSetChan
	goto	DEBUGCauseWDT	;STLExecute	;never called from here
	goto	STLSetTone
	goto	STLTimedToneOn
	goto	STLSetNoiseAmplitude
	#endif ;0


;-----------------------------------------------------------
;	STLSetChan
;
;	5=STL_COMMAND_SET_CHANNEL
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=channel 0-7
;FSR0=data[2]
;BSR=R2Bank
;---------------------------------------------------------
;added by this code data[3] contains bit shifted chan
;		data[3]
;-----------------------------------------------------------
STLSetChan
	;btg	STL_TEST_POINT
	movf	INDF0,W			;get 0-7
	addwf	WREG			;and bit shift to
	addwf	WREG			;required position
#if DEVICE == _18F458	
	addwf	WREG			;set bit 7 for, this shift not reqd for 18f4550??
#endif

	bsf	WREG,7			;fosc/32
	;Keep ADC off, fosc/32	
	movwf	ADCON0

#if DEVICE == _18F4680	
	;only need shl,2 for the 18f4860
	movwf	ADCON0			;select channel
	movlw	2				;left justify fosc/32
	movwf	ADCON2			;can ignore ACTQ, should be compatible
#endif

	movff	PREINC0,LATD	;RELAY ON (bit in data[3])
	;set ADC chan 
	return

;-------------------------------------------------------------
;	STLExecute
;perfom the task in FSR0 which points to a R2RxRecordData+2 struct
;Note that we need consistent timing for all these functions
;	6=STL_COMMAND_EXECUTE
;		always IMMEDIATE
;Send back an RS232Record with data informing VB to turn off
;echo, reading the number of data points expected into an array
;BSR=R2Bank or GtBank
;TIMING:
;It takes 20 cycles to go from the start of the exec loop
;to the first instruction of the goto'd function
;4us in total
;ACQUIRE
;Note that if we are to acquire data that the pc will send us
;an STL_ACK telling us it is ready to receive the data.
;We need to ensure that the default handler doesn't pick off
;this ack.
;use GlbGtStatus,STL_STATUS_ADC_QUEUED for control
;
;-------------------------------------------------------------
STLExecute
	banksel GtBank
	movf	GtCmdListNum
	bz	se_no_data_exit		;empty list	
	nop
	nop
	STL_SEND_SUCCESS_FAIL	;MACRO restores Gt bank
;perform any housekeeping task associated with an ACQ
	nop
	nop
	call	CheckForQueuedADC

	movff	GtCmdListNum,GtExecuteCounter
	movff	GtCmdListNum,GtISRExecuteNumInBuff

	clrf	TMR2
	bsf	T2CON,TMR2ON

;-----Main execute loop---------------------------------------
;the ISR will check the ms counter and increment
;GtISRExecuteNumPending when it finds a task
;Any acquisition will happen when the STLAcquire jmp table
;entry is called. The Acquire routine will also check for pending
;events in AcquireDespatchLoop
;--------------------------------------------------------------
se1:
;Wait for tick to indicate something needs doing	
	movf	GtISRExecuteNumPending
	bz	se1
;---GtTickCounter >0 next item in list-------------------
	decf	GtISRExecuteNumPending

	movff	GtCmdListRCursor,FSR0L
	movlw	HIGH GtCmdListRCursor
	movwf	FSR0H

	;increment the RCursor(s), must be done here
	;or else the despatch loop in Acquire performs re-entrant call
	movlw	STL_MAX_COMMAND_LENGTH
	addwf	GtCmdListRCursor,F
	incf	GtExecuteAtTickRCursor
	incf	GtExecuteAtTickRCursor

	;-Get CMD
	movf	POSTINC0,W	;point to data
	rcall	se2		;may modify FSR0 and BSR
	banksel	GtBank
	decfsz	GtExecuteCounter
	bra	se1

se_exit
	bcf	T2CON,TMR2ON		;turn off timer
	clrf	GtTickCounter		;clear the counter
	clrf	GtTickCounter+1

;-reset Cursors to point to start of list
	movlw	GtCmdListBuffer		;address of buffer
	movwf	GtCmdListRCursor	;commands in Queue
	movwf	GtCmdListWCursor
;same for ExecuteAt cursors 
	movlw	GtExecuteAtTickBuffer	;address of buffer
	movwf	GtExecuteAtTickRCursor
	movwf	GtExecuteAtTickWCursor

	return	
	
se_no_data_exit:
	movlw	GSI_ERR_NO_DATA
	call	R2SendProcessRecordFail
	bra	se_exit
			
se2:
	;adjust for goto in jmp table
	addwf	WREG
	addwf	WREG
	;calculate computed offset
	movwf	GtCompOffset
	movlw 	LOW exec_table 	;get low 8 bits of address
	addwf 	GtCompOffset,F	;do an 8-bit add operation
	movlw 	HIGH exec_table 	;get high 5 bits of address
	btfsc 	STATUS,C,A 	;page crossed?
	addlw 	1 		;yes then increment high address
	movwf	PCLATH,A 		;load high address in latch
	movf	GtCompOffset,W	;load computed offset in w reg
	movwf	PCL
;-----------------------------------------------------------------
;Implement the jump table as a macro as we have two copies of it
;WARNING the two copies have one entry that is different (STLAcquire)
;Need a 2nd copy in the ADC loop to ensure commands are despatched
;-----------------------------------------------------------------
exec_table
	CMD_EXECUTE_TABLE
	#if 0
	goto	DEBUGCauseWDT
	goto	STLSetAmplitude
	goto	STLSetFrequency
	goto	STLSetDelay
	goto	STLAcquire
	goto	STLSetChan
	goto	DEBUGCauseWDT	;STLExecute	;never called from here
	goto	STLSetTone
	goto	STLTimedToneOn
	goto	STLSetNoiseAmplitude
	#endif
	
;-------End of STLExecute--------------------------------


;--------------------------------------------------------
;	STLSetTone
;	7=STL_COMMAND_SET_TONE
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=ON/OFF OFF=0 ON=1
;FSR0=data[2]
;BSR=R2Bank
;LATB,4 is attached to the max232 inverter, thus a high
;out of latb,4 sends a -10V to mute and the mute is OFF
; and the TONE is ON
;low to latb,4 sets +10V to mute (through a diode) thus 
;stopping current flow from the mute pin. MUTE is ON and
;the TONE is OFF
;--------------------------------------------------------
STLSetTone
	decf	INDF0,W		;get ON/OFF
	bz	tone_on
	;OFF or error (if >1) so turn off anyway
tone_off
	STARTLE_TONE_OFF
	return
tone_on
	STARTLE_TONE_ON
;Set a flag if we are currently acquiring so that we can set
;ADC data to max or min, so user can see at which point acq commenced
	btfsc	GlbGtStatus,STL_STATUS_ADC_ACQUIRING
	bsf	GlbGtStatus,STL_STATUS_TONE_START	

	return

;--------------------------------------------------------
;	STLTimedToneOn
;	8=STL_COMMAND_TIMED_TONE_ON
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=msb
;		data[3]=lsb
;		data[4]=prescalar
;FSR0=data[2]
;NOTE
;the prescalar value is an immediate command. It is not
;stored in the execute list
;The prescalar value has been set by the STLCommandTimedToneOn
;Turn on the TONE, turn it off when tmr fires
;--------------------------------------------------------
STLTimedToneOn
	;clrf	T0CON	;timer off, 16 bit, 1:2
	movlw	b'00000111'	;and mask to preserve prescalar
	andwf	T0CON

	movff	POSTINC0,TMR0H
	movff	INDF0,TMR0L

	bcf	INTCON, TMR0IF	;clr int flag
	bsf	INTCON, TMR0IE	;enable int
	bsf	T0CON,TMR0ON	;turn on timer
	STARTLE_TONE_ON
;Set a flag if we are currently acquiring so that we can set
;ADC data to max or min, so user can see at which point acq commenced
	btfsc	GlbGtStatus,STL_STATUS_ADC_ACQUIRING
	bsf	GlbGtStatus,STL_STATUS_TONE_START	
	return

;---------------------------------------------------------
;	STLSetNoiseAmplitude

;	1=STL_COMMAND_SET_NOISE_AMPLITUDE
;		data[1]=QUEUE=0; IMMEDIATE=1
;		data[2]=msb amplitude
;		data[3=lsb amplitude
;FSR0=data[2]
;BSR=GtBank or R2Bank
;---------------------------------------------------------
STLSetNoiseAmplitude
;perfom the task in FSR0 which points to a R2RxRecordData+2 struct	
;NOTE: this can be called by ExecuteFn so might not be R2RxRecord
;will be in GtCmdListBuffer
#if STARTLE_V2
	banksel	GtBank
	movlw	VOL_WHITE_NOISE		
	movwf	GtVolChanByte			;set channel to white noise chan
;load both bytes even though we actually only use the lsb for V2
	movff	POSTINC0,GtVolDBByte
	movff	INDF0,GtVolDBByte		
	call	SendVol16
#ELSE
	banksel	GtBank
	movff	POSTINC0,GtTCL5615TxByte1
	movff	INDF0,GtTCL5615TxByte0
	SPI_CS_AMPLITUDE_NOISE		;2 clocks
	call	WriteTCL5615		;
	SPI_AMPLITUDE_NOISE_STROBE		;2 clocks
#endif
	banksel	R2Bank	
	return


;---------------------------------------------------
;	STLR2TxDelay
;Required for VB to process interrupt
;---------------------------------------------------
STLR2TxDelay

	return


;---------------------------------------------------
;	CheckForQueuedADC
;Called from Execute function.
;If an ADC is queued then we need to perform certain
;housekeeping tasks in preparation
;Return Z if no ADC or NZ if Queued
;---------------------------------------------------
CheckForQueuedADC
;disable Rx int
	bsf	STATUS,Z
	btfss	GlbGtStatus,STL_STATUS_ADC_QUEUED
	return
;we have an ADC queued so must do some housekeeping
;If the pc has already responded with an ACK, the ACK will be in
;the buffer, we will miss it!
;pc must either wait a few ms or we must check the buffer!
	
	banksel R2Bank	;check the buffer
	clrwdt
cfqa_look_for_ack:
	call	GSIGetFromRS232RxBuff	;Z if empty
	bz		cfqa_look_for_ack		;empty so keep looking
	movwf	R2BTemp
	sublw	RT_GSI_ACK
	bz		cfqa_2					;found
	call	DEBUGCauseWDT			;not found: serious error
	;WREG holds char 
#IF 0
;look for ACK from pc
cfqa_1
	btfss	PIR1,RCIF
	bra	cfqa_1
	movlw	RT_GSI_ACK
	cpfseq	RCREG
	bra	cfqa_1
#ENDIF
;have received ACK from pc 
cfqa_2
	banksel GtBank
	bcf	PIE1,RCIE	;disable Rx int for the ADC
	bcf	PIR1,RCIF
;ECHO the ACK,WREG still holds  ACK 
	movlw	RT_GSI_ACK
	call	GSISendByte232

;wait till char has been tx'd
cfqa_3
	btfss	PIR1,TXIF
	bra	cfqa_3	
	;		
	bcf	STATUS,Z
	return


#if STARTLE_V2
WriteDDSWord:
;----------------------------------------------------------
;Send the 40 bit data word
;in DDSByte0-4
;----------------------------------------------------------
SendDDS40
;Frequency bits 0-31(MSB)
;bits 32-34 are flags:
;bit 32 is 0==x1 multiplier, 1==x6 multiplier
;bit 33 must always be 0, otherwise we enter test mode
;bit 34 power down; 0==run, 1==power down
;Note that we must load from LSB W4 to MSB W0

	bcf		GtDDSByte4,1			;Always send 0 for bit 33
#IF DDS == 9850
	bcf		GtDDSByte4,0			;if 9850 there is no x6 clock
#ENDIF
	movlw	40
	movwf	GtDDSBitCounter		
	
SendLoop		
	bcf		DDS_DATA			;assume we are sending  0
;isolate the next bit
	bcf		STATUS,C
	rrcf 	GtDDSByte4
	rrcf 	GtDDSByte3
	rrcf 	GtDDSByte2
	rrcf 	GtDDSByte1
	rrcf	GtDDSByte0
#if 0
	rrcf	GtDDSByte0
	rrcf 	GtDDSByte1
	rrcf 	GtDDSByte2
	rrcf 	GtDDSByte3
	rrcf 	GtDDSByte4
#endif
	;40 bit rotation complete
	btfsc	STATUS,C
	bsf		DDS_DATA			;CY, the bit is set, so set DDS_DATA bit
	;DDS_DATA bit holds the value of the next bit of 40 bit word
	call	WCLKStrobe				;clock in the data

	decfsz	GtDDSBitCounter
	bra		SendLoop

;40 bits have been sent, so strobe into the DDS core
	call	FQUDStrobe

	return

WCLKStrobe:
	bsf			W_CLK
	nop
	nop
	bcf			W_CLK
	return

FQUDStrobe:
	bsf			FQ_UD
	nop
	nop
	bcf			FQ_UD
	return

InitDDS:
;On training board we currently use PORTC
;RC0=FQ_UD
;W_CLK=RC1
;DDS_DATA=RC2
;Check startle.inc to make sure this is correct
;Remember that GSI will want to use RC6,7 for RS232 so don't
;change TRISC with a literal
	movlw	b'11111000'			;select bits 0,1,2
	andwf	DDS_TRIS_PORT		;PORTC 0,1,2 as outputs


;To place dds into serial mode must send out a W_CLK pulse
;followed by a FQ_UD pulse
;This works because the dds starts in parallel mode. The hardware
;has hardwired the correct bit pattern on the parallel data lines
;for the 'serial mode' opcode. Thus we only need to 
	call	WCLKStrobe
	call	LittleDelay
	call	LittleDelay
	call	FQUDStrobe
	call	LittleDelay
	call 	LittleDelay
;ok should now be in serial mode. Must now send 40 bits of 0

;ref_osc_3   equ 0x2A              ; Most significant osc byte
;ref_osc_2   equ 0xF3              ; Next byte
;ref_osc_1   equ 0x1D              ; Next byte
;ref_osc_0   equ 0xC4              ; Least significant byte
	clrf	GtDDSByte0
	clrf	GtDDSByte1
	clrf	GtDDSByte2
	clrf	GtDDSByte3
	clrf	GtDDSByte4

	call	SendDDS40
;----send out 50kHz as a test
	;bsf		GtDDSByte2,5	;ad9851 50kHz (DDSByte2,5)
	;call	SendDDS40
	nop

	return

LittleDelay
	nop
	return


InitVol
	clrf	PORTC
	movlw	b'11000111'		;PORTC 3,4,5 used for VOL
	andwf	TRISC
	movlw	0xff			;full attenuation
	movwf	GtVolDBByte		;0=0dB attenuation
	call	SendVol16
	return
;------------------------------------------------
;16 bits
;msb->lsb
;byte1=chan
;byte2=data
;chan1=0
;chan2=1
;chan3=2

;Drop VOL_LOAD for 150nS setup time
;clock in the 16 data bits, minimum 150nS pulse
;leave VOL_LOAD for 150nS minimum after last clock
;VOL_CLK uses portc,3 which is a SPI pin. SPI/I2C must be disabled??

;-----------------------------------------------
SendVol16
	banksel	GtBank
	bcf		VOL_LOAD		;drop load line
	call	LittleDelay
	movlw	16
	movwf	GtVolBitCounter		
	
VolSendLoop		
	bcf		VOL_DATA			;assume we are sending  0
;isolate the next bit
	bcf		STATUS,C
	rlcf 	GtVolDBByte
	rlcf	GtVolChanByte

	;16 bit rotation complete
	btfsc	STATUS,C
	bsf		VOL_DATA			;CY, the bit is set, so set VOL_DATA bit
	;VOL_DATA bit holds the value of the next bit of 16 bit word
	call	VolCLKStrobe				;clock in the data

	decfsz	GtVolBitCounter
	bra		VolSendLoop

;16 bits have been sent, so strobe into the DDS core
;arrive here should have 150nS delay but make sure
	call	LittleDelay
	bsf		VOL_LOAD		;raise VOL_LOAD

	return

VolCLKStrobe
	bsf			VOL_CLK
	nop
	nop
	bcf			VOL_CLK
	return

VOL_LOADStrobe
	bsf			VOL_LOAD
	nop
	nop
	bcf			VOL_LOAD
	return


	return
#endif ;STARTLE_V2

#IF VOL
TestVol
	movlw	b'11000000'
	andwf	TRISC

	banksel	GtBank
	bsf		VOL_LOAD		;starts high
	movlw	VOL_WHITE_NOISE			;Chan 0=pin4 (in) pin 2 (out) SINE
	movwf	GtVolChanByte
	;movlw	b'00001111'		;78dB attenutation
	movlw	0xff			;full attenuation
	movwf	GtVolDBByte		;0=0dB attenuation

	;clrf	GtVolDBByte

	call	SendVol16

	movlw	VOL_SINE			;Chan	WHITE_NOISE
	movwf	GtVolChanByte

	;movlw	b'01111110'		;78dB attenutation
	movlw	b'11111111'		;full attenutation
	;clrf	WREG
	movwf	GtVolDBByte
	call	SendVol16
	banksel	R2Bank
	return
#ENDIF


#ENDIF




  END

