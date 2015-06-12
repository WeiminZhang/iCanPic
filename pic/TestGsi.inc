;******************************************************************************
;* @file    testGsi.inc part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;TestGsi.inc
#IF STARTLE == FALSE
STL_SAVE_FSR0        macro
        movff   FSR0L,GlbGtTFSR0
        movff   FSR0H,GlbGtTFSR0+1
        endm

STL_RESTORE_FSR0     macro
        movff   GlbGtTFSR0,FSR0L
        movff   GlbGtTFSR0+1,FSR0H
        endm

STL_SEND_SUCCESS_FAIL	macro
	STL_SAVE_FSR0
	call	R2SendProcessRecordSuccessFail
	banksel GtBank
	STL_RESTORE_FSR0
	endm



#IF	0
pic resource usage

-------PORTA----------------------------
RA0-3	Analogue input
RA4	Unused open drain output
Ra5	Analogue input
----------------------------------------
------PORTB-----------------------------
RB0	Unused

RB1	CTS/RTS (doesn't work, VB fails to recognise)

RB2	CAN
RB3	CAN	CAN currently not used

RB4	Mute

RB5	Test point (digital output for cro)

RB6	ICD2
RB7	ICD2
-------------------------------------------
----------PORTC	---------------------------

RC0	spi CS Amplitude
RC1	spi CS Frequency

RC2	?Taken out with analogue channels?

RC3-5	Unused

RC6	RS232
RC7	RS232
-------------------------------------------
-------PORTD-------------------------------
RD0-RD7	Audio channel select

-----------------------------------------
--------PORTE----------------------------
RE0-2	Analogue input
-----------------------------------------

TMR0 timed tones
TMR1 ADC timer
TMR2 1msec period for task despatcher
TMR3 used for non-interrupt timing loops


Calibration data

Sine 	Rheostat on 3

SPL(dB)		Volts across Inductor in startle box

65		60.8mV
70		154mV
80		376mV
90		1.06V
95		1.94V
100		3.44V
105		6.16V
110		11.1V
115		20.6V
120		30.2V



White noise

SPL		Volts
65		220mV		most commonly used
80		1.3V
90		4.2V
100		10.2V
#endif

#ELSE ;STARTLE

	#IFDEF IN_TESTGSI.ASM
	MESSG STARTLE box code compiled!		
	#ENDIF

#if 0  ;Placed in startle.inc
#define STL_TEST_POINT	LATB,5
#define STL_RS232_RTS	LATB,1		;an output; CTS as seen from pc

STL_DEBUG				EQU	TRUE
#IF STARTLE_V2
STL_TIMED_TONE_ENABLE			EQU	FALSE
#ELSE
STL_TIMED_TONE_ENABLE			EQU	TRUE
#ENDIF

STL_HP_INT_ENABLED			EQU	TRUE

STL_TEST_POINT_BIT			EQU	5
STL_RS232_RTS_BIT			EQU	1 ;an output; CTS as seen from pc

;STLStatus defines
STL_STATUS_ADC_DATA_READY		EQU	0
STL_STATUS_ADC_ACQUIRING 		EQU	1
STL_STATUS_ADC_END		 		EQU	2
STL_STATUS_ADC_QUEUED			EQU	3
STL_STATUS_EXECUTE_AT			EQU	4
STL_STATUS_TONE_START			EQU	5	;tone has been switched on
STL_STATUS_ERROR_R2_OVERFLOW		EQU	7

;---------------------------------------------------


;STL_TMR0_VALUE				EQU	1000	;5kHz
;STL_TMR0_VALUE				EQU	2000
;STL_TMR0_LATENCY			EQU	30	

STL_TMR1_LP_LATENCY			EQU	33	
STL_TMR1_HP_LATENCY			EQU	28	
STL_TMR1_2500kHz			EQU	2000

STL_TMR1_TIMEBASE			EQU	STL_TMR1_2500kHz

	#IF STL_HP_INT_ENABLED
STL_TMR1_VALUE				=(0xffff-(STL_TMR1_TIMEBASE -STL_TMR1_HP_LATENCY))
	#ELSE
STL_TMR1_VALUE				=(0xffff-(STL_TMR1_TIMEBASE -STL_TMR1_LP_LATENCY))
	#ENDIF	


#define	STARTLE_CHAN0			0
#define	STARTLE_CHAN1			1
#define	STARTLE_CHAN2			2
#define	STARTLE_CHAN3			3
#define	STARTLE_CHAN4			4
#define	STARTLE_CHAN5			5
#define	STARTLE_CHAN6			6
#define	STARTLE_CHAN7			7

;-----------------------------------------------------
;	List 
;-----------------------------------------------------
STL_MAX_COMMAND_LENGTH			EQU	3
STL_MAX_COMMANDS_IN_LIST		EQU	30
;-----------------------------------------------------
;	Errors
;Public Const GSI_ERR_NONE = 0
;Public Const GSI_ERR_TIMEOUT = 1001
;Public Const GSI_ERR_INVALID_RECORD_LENGTH = 1002
;Public Const GSI_ERR_INVALID_RECORD_TYPE = 1003
;Public Const GSI_ERR_INVALID_RECORD_NODE = 1004
;Public Const GSI_ERR_ECHO = 1005
;Public Const GSI_ERR_FUNCTION_FAIL = 1006
;Public Const GSI_ERR_REPEAT = 1007
;Public Const GSI_ERR_ACK = 1008
;Public Const GSI_ERR_RANGE = 1009
;Public Const GSI_ERR_FUNCTION_UNSUPPORTED = 1010

STL_ERR_NONE			EQU	0
STL_ERR_TIMEOUT			EQU	1
STL_ERR_RECORD_LENGTH		EQU	2
STL_ERR_RECORD_TYPE		EQU	3
STL_ERR_RECORD_NODE		EQU	4
STL_ERR_ECHO			EQU	5
STL_ERR_FUNCTION_FAIL		EQU	6
STL_ERR_REPEAT			EQU	7
STL_ERR_ACK			EQU	8
STL_ERR_RANGE			EQU	9
STL_ERR_FUNCTION_UNSUPPORTED	EQU	10
;---------------------------------------------------

;----------------Commands ------------------------------
;data[0]=STL_COMMAND_XXX
STL_UNCONDITIONAL_RESET		EQU	0
STL_ALLOW_COMPLETION_RESET	EQU	1
STL_COMMAND_RESET		EQU	0
;	data[0]=0
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=STL_RESET_FLAG (UNCONDITIONAL\COMPLETION)
;	data[3]=unused
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

STL_COMMAND_SET_AMPLITUDE	EQU	1
;	data[0]=1
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb amplitude
;	data[3]=lsb amplitude
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

STL_COMMAND_SET_FREQUENCY	EQU	2
;In V2 this is the first 2 bytes of the 4 byte word
;on receipt of upper 2 bytes (STL_COMMAND_SET_FREQUENCY_UPPER)
;The 4 bytes are sent to the DDS
;	data[0]=2
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[3]=msb frequency
;	data[3]=lsb frequency
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

STL_COMMAND_SET_DELAY		EQU	3
;	data[0]=3
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb TICK (TICK=10us??)
;	data[3]=lsb TICK
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

STL_COMMAND_ACQUIRE		EQU	4
;	data[0]=4
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2] msb number data points
;	data[3] lsb number data points
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

STL_COMMAND_SET_CHANNEL		EQU	5
;	data[0]=5
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=channel 0-7
;	data[3]=msb tick to execute on (if QUEUED)
;	data[4]=lsb tick to execute on (if QUEUED)

STL_COMMAND_EXECUTE		EQU	6
;	data[0]=6
;		always IMMEDIATE

STL_COMMAND_TONE		EQU	7
;	data[0]=7
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=ON/OFF; OFF=0 ON=1
;	data[3]=msb tick to execute on (if QUEUED)
;	data[4]=lsb tick to execute on (if QUEUED)

STL_COMMAND_TIMED_TONE_ON		EQU	8
;	data[0]=8
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb
;	data[3]=lsb
;	data[4]=prescaler
;	data[5]=msb tick to execute on (if QUEUED)
;	data[6]=lsb tick to execute on (if QUEUED)

STL_COMMAND_SET_NOISE_AMPLITUDE	EQU	9
;	data[0]=9
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb amplitude
;	data[3]=lsb amplitude
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

STL_COMMAND_GET_STATUS	EQU	10
;Always immediate
;	data[0]=10
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb amplitude
;	data[3]=lsb amplitude
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

#if STARTLE_V2
;User code will send a STL_COMMAND_SET_FREQUENCY which holds
;the lower 2 bytes of 4 byte word.
;When the 2 upper bytes are received via this command, the
;4 byte word is written to the DDS.
;This command only has meaning in DDS (V2) of startle box
STL_COMMAND_SET_FREQUENCY_UPPER	EQU	11
;Always immediate
;	data[0]=10
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=lsb amplitude of byte 2
;	data[3]=msb amplitude of byte 3, 
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)
#endif
;---------------------------------------------------------

;----------FREQUENCY------------------------
;require a minimum frequency or else the sine generator
;will not start till we have selected a much higher frequency
;probably need to select a larger C value for oscillator??
STL_MINIMUM_FREQUENCY_LSB	EQU	LOW  305	;305 approx 1k
STL_MINIMUM_FREQUENCY_MSB	EQU	HIGH 305


STARTLE_TONE_OFF	macro
	bcf	LATB,4
	ENDM

STARTLE_TONE_ON		macro
	bsf	LATB,4
	ENDM


STARTLE_SET_CHAN	MACRO	reg
	local _exit,l1
	movff	reg,__GtTemp
	movlw	1
	movwf	__GtChan
	movf	__GtTemp
	bz	_exit
l1:	
	rlncf	__GtChan
	decfsz	__GtTemp
	bra	l1	
_exit:
	movff	__GtChan,LATD

	ENDM

STARTLE_AMPLITUDE_SELECT	EQU	0
STARTLE_FREQUENCY_SELECT	EQU	1
STARTLE_AMPLITUDE_NOISE_SELECT	EQU 	2

;Active low for CS
SPI_CS_AMPLITUDE	MACRO
	bsf	LATC,STARTLE_FREQUENCY_SELECT	;deselect 2nd chip	
	bsf	LATC,STARTLE_AMPLITUDE_NOISE_SELECT ;deselect
	bcf	LATC,STARTLE_AMPLITUDE_SELECT
	ENDM

SPI_AMPLITUDE_STROBE	MACRO
	bsf	LATC,STARTLE_AMPLITUDE_SELECT	;strobe the data
	bcf	LATC,STARTLE_AMPLITUDE_SELECT		
	ENDM

SPI_CS_AMPLITUDE_NOISE	MACRO
	bsf	LATC,STARTLE_FREQUENCY_SELECT	;deselect
	bsf	LATC,STARTLE_AMPLITUDE_SELECT	;deselct
	bcf	LATC,STARTLE_AMPLITUDE_NOISE_SELECT ;select
	ENDM

SPI_AMPLITUDE_NOISE_STROBE	MACRO
	bsf	LATC,STARTLE_AMPLITUDE_NOISE_SELECT	;strobe the data
	bcf	LATC,STARTLE_AMPLITUDE_NOISE_SELECT		
	ENDM

SPI_CS_FREQUENCY	MACRO
	bsf	LATC,STARTLE_AMPLITUDE_SELECT		;deselect
	bsf	LATC,STARTLE_AMPLITUDE_NOISE_SELECT	;deselect
	bcf	LATC,STARTLE_FREQUENCY_SELECT	
	ENDM

SPI_FREQUENCY_STROBE	MACRO
	bsf	LATC,STARTLE_FREQUENCY_SELECT
	bcf	LATC,STARTLE_FREQUENCY_SELECT		
	ENDM


SPI_CS_NONE	MACRO
	bsf	LATC,STARTLE_AMPLITUDE_SELECT
	bsf	LATC,STARTLE_FREQUENCY_SELECT	
	bsf	LATC,STARTLE_AMPLITUDE_NOISE_SELECT
	ENDM


STL_SAVE_FSR0        macro
        movff   FSR0L,GlbGtTFSR0
        movff   FSR0H,GlbGtTFSR0+1
        endm

STL_RESTORE_FSR0     macro
        movff   GlbGtTFSR0,FSR0L
        movff   GlbGtTFSR0+1,FSR0H
        endm

STL_SEND_SUCCESS_FAIL	macro
	STL_SAVE_FSR0
	call	R2SendProcessRecordSuccessFail
	banksel GtBank
	STL_RESTORE_FSR0
	endm

CMD_EXECUTE_IN_ACQUIRE_TABLE MACRO
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
	goto	DEBUGCauseWDT	;STLStatus
#if STARTLE_V2
	goto	STLSetFrequencyUpper
#endif
	ENDM

CMD_EXECUTE_TABLE MACRO
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
	goto	DEBUGCauseWDT	;STLStatus
#if STARTLE_V2
	goto	STLSetFrequencyUpper
#endif
	ENDM


#IF	0
pic resource usage

-------PORTA----------------------------
RA0-3	Analogue input
RA4	Unused open drain output
Ra5	Analogue input
----------------------------------------
------PORTB-----------------------------
RB0	Unused

RB1	CTS/RTS (doesn't work, VB fails to recognise)

RB2	CAN
RB3	CAN	CAN currently not used

RB4	Mute

RB5	Test point (digital output for cro)

RB6	ICD2
RB7	ICD2
-------------------------------------------
----------PORTC	---------------------------

RC0	spi CS Amplitude
RC1	spi CS Frequency

RC2	?Taken out with analogue channels?

RC3-5	Unused

RC6	RS232
RC7	RS232
-------------------------------------------
-------PORTD-------------------------------
RD0-RD7	Audio channel select

-----------------------------------------
--------PORTE----------------------------
RE0-2	Analogue input
-----------------------------------------

TMR0 timed tones
TMR1 ADC timer
TMR2 1msec period for task despatcher
TMR3 used for non-interrupt timing loops


Calibration data

Sine 	Rheostat on 3

SPL(dB)		Volts across Inductor in startle box

65		60.8mV
70		154mV
80		376mV
90		1.06V
95		1.94V
100		3.44V
105		6.16V
110		11.1V
115		20.6V
120		30.2V



White noise

SPL		Volts
65		220mV		most commonly used
80		1.3V
90		4.2V
100		10.2V
#endif

#ENDIF ;#if 0
#ENDIF
