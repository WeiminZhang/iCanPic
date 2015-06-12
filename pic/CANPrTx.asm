;****************************************************************************
;*
;*              CAN Library routines in Assembly
;*
;****************************************************************************
;* FileName:            CANPrTx.ASM
;* Dependencies:        CANPrTx.inc
;*                      CANPrTx.def
;*
;* Processor:           PIC 18XXX8
;* Compiler:            MPLAB 6.00.20 
;* Company:             Microchip Technology, Inc.
;*
;* Software License Agreement
;*
;* The software supplied herewith by Microchip Technology Incorporated
;* (the “Company”) for its PICmicro® Microcontroller is intended and
;* supplied to you, the Company’s customer, for use solely and
;* exclusively on Microchip PICmicro Microcontroller products. The
;* software is owned by the Company and/or its supplier, and is
;* protected under applicable copyright laws. All rights are reserved.
;* Any use in violation of the foregoing restrictions may subject the
;* user to criminal sanctions under applicable laws, as well as to
;* civil liability for the breach of the terms and conditions of this
;* license.
;*
;* THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
;* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
;* TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
;* PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
;* IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
;* CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
;*
;* Author               Date        Comment
;*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;* Gaurang Kavaiya      4/25/01         Original 
;* Gaurang Kavaiya      10/23/02        Modified for Application Maestro (V1.0)
;*
;*****************************************************************************

;;-----------------------------------------------------------
;Modfied by Glenn Self for use with gsi
;------------------------------------------------------------
#include 	"GSI.inc"
#if GSI_CAN_SUPPORT

;#IF DEVICE == _18F458
;#include "p18xxx8.inc"
;#ELSE
;#INCLUDE "P18F4680.inc"
;#ENDIF

#define         CAN_PRIOR_TX_INT_SOURCE

#define         CAN_MODULE
#define         _ADD_PROC_INC_FILE
#define         GEN_MODULE_ERROR

;#include 	"p18xxx8.inc"
#include        "CANPrTx.inc"
#include	"CANPrTx.def"


#include 	"error.inc"
#include 	"can.inc"
#include	"rs232.inc"

	

	IF SIMULATOR
		GLOBAL	GSCANReadMsgToCQ
	ENDIF

	GLOBAL GlbCANState,GlbCANState1
	EXTERN CnErrorState,CnErrorState1
	EXTERN	GlbErrState
;From CANRx.asm
#if DISABLE_CN_SYNC_RX
	EXTERN CnFilter0Mask	
#endif	
	IF DEBUG
	EXTERN DEBUGGSISendByte232,DEBUGGSISendBinary232ISR
	EXTERN DEBUGGSISendBinary232
	ENDIF
;From can.asm
	EXTERN	CnCQRxNumInBuff,CnCQRxBuff,CnCQRxWCursor	;CnCQRxBuffEnd
	EXTERN CnCQRxBuffEndLbl ;CnCQRxBuffEndLbl
	
	EXTERN CnCQRxR2WCursor,CnCQRxR2NumInBuff,CnCQRxR2Buff
	EXTERN CnCQRxR2BuffEndLbl 

;From gsi.asm
	EXTERN	GlbClock,GlbLocalTempL
		
TxBufData: = D'14' * MAX_TX_SOFT_BUFFER ;Total RAM locations requirement
TxBufPtrSize: = (MAX_TX_SOFT_BUFFER + 3) * 0x02 ;
TxBufMaxSize: = MAX_TX_SOFT_BUFFER + 3 ;


CAN_DATA        UDATA
        GLOBAL  vCANPrTxBufSize

CnTFSR0		RES	2		;used by GSCANReadMsgToCQ
CnTFSR1		RES	2		;by GSCANReadMsgToCQ
_CnRxFlags	RES	1		;used by GSCANReadMsgToCQ
;-------------------------------------------------------------------
;Note the next two data items must not change position as Reg1_O
;stores the id to be added to the CQRxBuff. Length follows this in
;the CQ, so can use indirect addressing and a single loop to add both
;id and length
_CnvReg1_O	RES	4		;used by GSCANReadMsgToCQ
_CnDataLength	RES	1		;used by GSCANReadMsgToCQ
;-------------------------------------------------------------------
_CnAddToCQCounter RES	1		;used by GSCANReadMsgToCQ	
;_CnReadMsgRxByte RES	1		;byte we are processing	

TxBuffer        RES     TxBufData       ;Software Transmit Buffer
;TxBufPtr:      RES     TxBufPtrSize    ;TxBuffer Pointer
vCANPrTxBufSize  RES     01              ;Tx Buffer Size value
;XXXGS addition
CnPrevBSR	RES	1
CnISRCompOffset	RES	1

;Working regs. to be saved during Interrupt
ITemp32Data     RES     08
IReg1           RES     04
ITxFlags        RES     01
Im_TxFlags      RES     01
ITemp1          RES     01
ITemp2          RES     01
ITemp3          RES     01
IFSR0           RES     02
IFSR1           RES     02

CAN_GLOBAL_DATA	UDATA_ACS
TxFlags         RES     01
_Temp1_A        RES     01
Temp2           RES     01
Temp3           RES     01
;XXXGS addition
GlbCANCONTemp	RES	1
GlbCANSTATTemp	RES	1
GlbCANState	RES	1
GlbCANState1	RES	1

	IF DEBUG_CAN
DCWTemp		RES	01

	ENDIF

        GLOBAL  _Temp1_A


;Used by mCANPrTxInit, mCANPrTxSetOpMode and mCANPrTxSetBaud function
CAN_DATA_OVR            UDATA_OVR
_vCANPrTxSJW_O          RES     01      ;SJW value
_vCANPrTxBRP_O          RES     01      ;BRP value
_vCANPrTxPHSEG1_O       RES     01      ;Phase Segment 1 value
_vCANPrTxPHSEG2_O       RES     01      ;Phase Segment 2 value
_vCANPrTxPROPSEG2_O     RES     01      ;Propagation Segment 1 value
_vFlags1_O              RES     01      ;Configuration Flags
_vReg1_O                RES     04      ;32 bit register for intermediate storage
;vTempPtr_O              RES     02
_vTemp32Data            RES     08
TFSR0                   RES     02
TFSR1                   RES     02

_DataLength             RES     01      ;Tx message data length
_RxFlags
_TxFlags                RES     01


        GLOBAL  _vCANPrTxSJW_O, _vCANPrTxBRP_O, _vCANPrTxPHSEG1_O, _vCANPrTxPHSEG2_O, _vCANPrTxPROPSEG2_O
        GLOBAL   _vFlags1_O, _vReg1_O, _vTemp32Data, _DataLength, _RxFlags, _TxFlags


GSSaveFSR0        macro
        movff   FSR0L,CnTFSR0
        movff   FSR0H,CnTFSR0+1
        endm

GSSaveFSR1        macro
        movff   FSR1L,CnTFSR1
        movff   FSR1H,CnTFSR1+1
        endm

GSRestoreFSR0     macro
        movff   CnTFSR0,FSR0L
        movff   CnTFSR0+1,FSR0H
        endm

GSRestoreFSR1     macro
        movff   CnTFSR1,FSR1L
        movff   CnTFSR1+1,FSR1H
        endm


;****************************************************************************
;* Macro:               SaveFSR0
;*
;* PreCondition:        None
;*
;* Input:               None
;*
;* Output:              None
;*
;* Side Effects:        None
;*
;* Overview:            It copies the content of FSR0 in Temp. FSR0 reg.
;*
;****************************************************************************
SaveFSR0        macro
        movff   FSR0L,TFSR0
        movff   FSR0H,TFSR0+1
        endm

;****************************************************************************
;* Macro:               SaveFSR1
;*
;* PreCondition:        None
;*
;* Input:               None
;*
;* Output:              None
;*
;* Side Effects:        None
;*
;* Overview:            It copies the content of FSR1 in Temp. FSR1 reg.
;*
;****************************************************************************
SaveFSR1        macro
        movff   FSR1L,TFSR1
        movff   FSR1H,TFSR1+1
        endm



;****************************************************************************
;* Macro:               RestoreFSR0
;*
;* PreCondition:        None
;*
;* Input:               None
;*
;* Output:              None
;*
;* Side Effects:        None
;*
;* Overview:            It restores the content of FSR0 from Temp. FSR0 reg.
;*
;****************************************************************************
RestoreFSR0     macro
        movff   TFSR0,FSR0L
        movff   TFSR0+1,FSR0H
        endm

;****************************************************************************
;* Macro:               RestoreFSR1
;*
;* PreCondition:        None
;*
;* Input:               None
;*
;* Output:              None
;*
;* Side Effects:        None
;*
;* Overview:            It restores the content of FSR1 from Temp. FSR1 reg.
;*
;****************************************************************************
RestoreFSR1     macro
        movff   TFSR1,FSR1L
        movff   TFSR1+1,FSR1H
        endm





CANLib  CODE
;****************************************************************************
;* Function:            void CANPrTxInit(BYTE SJW,
;*                      BYTE BRP,
;*                      BYTE PHSEG1,
;*                      BYTE PHSEG2,
;*                      BYTE PROPSEG,
;*                      enum CAN_CONFIG_FLAGS flags)
;*
;* Input:               _vCANPrTxSJW_O - SJW value as defined in 18CXX8 datasheet
;*                      (Must be between 1 thru 4)
;*                      _vCANPrTxBRP_O - BRP value as defined in 18CXX8 datasheet
;*                      (Must be between 1 thru 64)
;*                      _vCANPrTxPHSEG1_O - PHSEG1 value as defined in 18CXX8 datasheet
;*                      Must be between 1 thru 8)
;*                      _vCANPrTxPHSEG2_O - PHSEG2 value as defined in 18CXX8 datasheet
;*                      (Must be between 1 thru 8)
;*                      _vCANPrTxPROPSEG2_O - PROPSEG value as defined in 18CXX8
;*                      datasheet
;*                      (Must be between 1 thru 8)
;*                      _vFlags1_O - Value of type enum CAN_CONFIG_FLAGS
;*
;* Output:              CAN bit rate is set. All masks registers are set
;*                      '0' to allow all messages.
;*                      Filter registers are set according to flag value.
;*                      If (config & CAN_CONFIG_VALID_XTD_MSG)
;*                      Set all filters to XTD_MSG
;*                      Else if (config & CONFIG_VALID_STD_MSG)
;*                      Set all filters to STD_MSG
;*                      Else
;*                      Set half of the filters to STD while rests to
;*                      XTD_MSG.
;*
;* Side Effects:        All pending transmissions are aborted.
;*                      W, STATUS, BSR and FSR0 changed
;*
;* Overview:            Initializes CAN module
;*
;* Stack requirements:  3 level deep
;*
;****************************************************************************

CANPrTxInit:

        GLOBAL  CANPrTxInit

;Set configuration mode
	
        mCANPrTxSetOpMode     CAN_OP_MODE_CONFIG      ;
        call    CANPrTxSetBaudRate	;does banksel _vCANPrTxBRP_O

        movlw   CAN_CONFIG_MSG_BITS
        andwf   _vFlags1_O,W
        movwf   RXB0CON         ;Load Rx Buffer 0 Control reg.
        btfsc   _vFlags1_O,CAN_CONFIG_DBL_BUFFER_BIT_NO
        ;bsf     RXB0CON,RX0DBEN ;Set double Buffer Enable bit
		bsf		RXB0CON,RXB0DBEN	;XXX GS


        movff   RXB0CON,RXB1CON ;Load same configuration in Rx Buffer 1
                                ;control register


        mCANPrTxSetReg_IF    CAN_MASK_B0, 0, _vFlags1_O        ;Set Mask B0


        mCANPrTxSetReg_IF    CAN_MASK_B1, 0, _vFlags1_O        ;Set Mask B1
        mCANPrTxSetReg_IF    CAN_FILTER_B0_F1, 0, _vFlags1_O   ;Set Filter 1
        mCANPrTxSetReg_IF    CAN_FILTER_B0_F2, 0, _vFlags1_O   ;Set Filter 2
        mCANPrTxSetReg_IF    CAN_FILTER_B1_F1, 0, _vFlags1_O   ;Set Filter 3
        mCANPrTxSetReg_IF    CAN_FILTER_B1_F2, 0, _vFlags1_O   ;Set Filter 4
        mCANPrTxSetReg_IF    CAN_FILTER_B1_F3, 0, _vFlags1_O   ;Set Filter 5
        mCANPrTxSetReg_IF    CAN_FILTER_B1_F4, 0, _vFlags1_O   ;Set Filter 6

; Restore to Normal mode.
        mCANPrTxSetOpMode     CAN_OP_MODE_NORMAL      ;
        banksel vCANPrTxBufSize
        clrf    vCANPrTxBufSize       ;Clear Tx Buffer data counter

#if 0
	MESSG	GS MOD
#ifdef  CANIntLowPrior
        bsf     RCON,IPEN       ;Enable Priority levels on Int.
        movlw   B'11100011'
        andwf   IPR3,F          ;Set low priority for Tx. Int.
#else
        movlw   B'00011100'
        iorwf   IPR3,F          ;Set high priority for Tx. Int. (Default)
#endif
#endif

	clrf	CnErrorState	;Still in CAN_DATA BANK


        ;bsf     INTCON,GIE      ;Enable global Interrupt
        ;bsf     INTCON,PEIE     ;Enable Peripheral Interrupt

        return





;****************************************************************************
;* Function:            void CANPrTxSetOpMode(CAN_OP_MODE mode)
;*
;* PreCondition:        None
;*
;* Input:               W reg   - Operation mode code
;*
;* Output:              MCU is set to requested mode
;*
;* Side Effects:        W, STATUS changed
;*
;* Overview:            Given mode byte is copied to CANCON and made
;*                      sure that requested mode is set.
;*
;* Note:                This is a blocking call.  It will not return until
;*                      requested mode is set.
;*
;* Stack requirements:  1 level deep
;*
;****************************************************************************

CANPrTxSetOpMode:

        GLOBAL  CANPrTxSetOpMode
        movwf   CANCON          ;Request desired mode
        movwf   _Temp1_A        ;Store value in Temp reg. for future
	;XXXGS code
;NOTE
;This will make GlbClock indicate a tick earlier than usual
;As this is initialization code it shouldn't matter. Would be
;better to use GlbClock (GlbCLock code was written after this function)
	clrf	TMR0H
	movlw	0xf0
	movwf	TMR0L
	;movff	GlbClock,GlbLocalTempL	;
	;incf	GlbLocalTempL
	;incf	GlbLocalTempL
ChkModeSet:
	clrwdt
	movf	TMR0L,W
 	bz	cptsom_timeout
        movlw   CAN_OP_MODE_BITS
        andwf   CANSTAT,W       ;Chk CAN status register
        xorwf   _Temp1_A,W
	;this branch will cause a timeout if there is a CAN problem
	;reload TMR0 and wait for one rollover (13ms) timeout if still here
        bnz     ChkModeSet      ;Wait till desired mode is set
        return
cptsom_timeout
	bsf	CnErrorState1,CN_ERROR1_INIT
	bsf	GlbErrState,GSI_ERR_CAN
	return




;****************************************************************************
;* Function:            void CANPrTxSetBaudRate(BYTE SJW,
;*                                              BYTE BRP,
;*                                              BYTE PHSEG1,
;*                                              BYTE PHSEG2,
;*                                              BYTE PROPSEG,
;*                                              enum CAN_CONFIG_FLAGS flags)
;*
;* PreCondition:        MCU must be in Configuration mode or else these
;*                      values will be ignored.
;*
;* Input:               _vCANPrTxSJW_O   - SJW value as defined in 18CXX8 datasheet
;*                              (Must be between 1 thru 4)
;*                      _vCANPrTxBRP_O   - BRP value as defined in 18CXX8 datasheet
;*                              (Must be between 1 thru 64)
;*                      _vCANPrTxPHSEG1_O - PHSEG1 value as defined in 18CXX8
;*                               datasheet
;*                              (Must be between 1 thru 8)
;*                      _vCANPrTxPHSEG2_O - PHSEG2 value as defined in 18CXX8
;*                               datasheet
;*                              (Must be between 1 thru 8)
;*                      m_PROPSEG - PROPSEG value as defined in 18CXX8
;*                               datasheet
;*                              (Must be between 1 thru 8)
;*                              flags   - Value of type enum CAN_CONFIG_FLAGS
;*
;* Output:              CAN bit rate is set as per given values.
;*
;* Side Effects:        W, STATUS, BSR, PRODL is changed
;*
;* Overview:            Given values are bit adjusted to fit in 18CXX8
;*                      BRGCONx registers and copied.
;*
;* Stack requirements:  1 level deep
;*
;****************************************************************************

CANPrTxSetBaudRate:

        GLOBAL  CANPrTxSetBaudRate

        movff   _vCANPrTxSJW_O,_Temp1_A
        decf    _Temp1_A,F      ;Align values for offfset from 0
        movlw   0x40            ;Multiply by 40H to align the bits
        mulwf   _Temp1_A        ;for BRGCON1 requirement
        movff   PRODL,_Temp1_A  ;Transfer Result to _Temp1_A

        movlw   0xc0
        andwf   _Temp1_A,F      ;Mask bits
        banksel _vCANPrTxBRP_O
        decf    _vCANPrTxBRP_O,W        ;Align values for offset from 0
        iorwf   _Temp1_A,W      ;Calculate value for BRGCON1
        movwf   BRGCON1         ;Transfer value to BRGCON1

        movff   _vCANPrTxPHSEG1_O,_Temp1_A
        decf    _Temp1_A,F
        rlncf   _Temp1_A,F      ;Align the bits
        rlncf   _Temp1_A,F
        rlncf   _Temp1_A,W
;        banksel _vCANPrTxPROPSEG2_O
        decf    _vCANPrTxPROPSEG2_O,F    ;Align values for offset 0
        iorwf   _vCANPrTxPROPSEG2_O,W    ;Calculate value for BRGCON2
        movwf   BRGCON2         ;Transfer value to BRGCON2
        btfsc   _vFlags1_O,CAN_CONFIG_PHSEG2_PRG_BIT_NO
        bsf     BRGCON2,SEG2PHTS        ;Set SEG2PHTS
        btfsc   _vFlags1_O,CAN_CONFIG_SAMPLE_BIT_NO
        bsf     BRGCON2,SAM     ;Set SEG2PHTS

        movff   _vCANPrTxPHSEG2_O,BRGCON3        ;Transfer PHSEG2 value to BRGCON3
        decf    BRGCON3,F
        btfsc   _vFlags1_O,CAN_CONFIG_LINE_FILTER_BIT_NO
        bsf     BRGCON3,WAKFIL  ;Set WAKFIL bit

        return





;****************************************************************************
;* Function:            CANPrTxSetReg   RegAddr,
;*                                      unsigned long val,
;*                                      CAN_CONFIG_FLAGS type
;*
;* PreCondition:        None
;*
;* Input:               FSR0H:FSR0L - Starting address of a 32-bit buffer to
;*                                      be updated
;*                      _vReg1_O:_vReg1_O+3 - 32-bit value to be converted
;*                      (_vReg1_O= LL, _vReg1_O+1 =LH, _vReg1_O+2 = HL and _vReg1_O+3 = HH
;*                       byte)
;*                      _vFlags1_O - Type of message Flag - either
;*                              CAN_CONFIG_XTD_MSG or CAN_CONFIG_STD_MSG
;*
;* Output:              Given CAN id value 'val' is bit adjusted and copied
;*                      into corresponding PIC18XXX8 CAN registers
;*
;* Side Effects:        Databank, W, STATUS and FSR0 changed
;*
;* Overview:            If given id is of type standard identifier,
;*                      only SIDH and SIDL are updated
;*                      If given id is of type extended identifier,
;*                      bits val<17:0> is copied to EIDH, EIDL and SIDH<1:0>
;*                      bits val<28:18> is copied to SIDH and SIDL
;*
;* Stack requirements:  2 level deep
;*
;****************************************************************************
CANPrTxSetReg:

        GLOBAL  CANPrTxSetReg
        banksel _vFlags1_O
        btfss   _vFlags1_O,CAN_CONFIG_MSG_TYPE_BIT_NO
        bra     SetExtFram

        movlw   0x05            ;Rotate Left 5 times the 32 bit number
        rcall   _RotateLeft32N
        movff   _vReg1_O+1,POSTINC0
 ;       banksel _vReg1_O
        movlw   0xe0
        andwf   _vReg1_O,W
        movwf   INDF0
        return

SetExtFram
        movlw   0x03            ;Rotetae Left 5 times the 32 bit number
        rcall   _RotateLeft32N
        movff   _vReg1_O+3,POSTINC0 ;Set EID<28:21>
        movff   _vReg1_O+2,INDF0    ;Set EID <20:18>

        movlw   0x03            ;Rotate Right 5 times the 32 bit number
        rcall   _RotateRight32N ;to restore original number
        movlw   0xe0            ;Mask other bits except EID<20:18>
        andwf   INDF0,F
        bsf     INDF0,EXIDE     ;Set  EXIDE bit
        movlw   0x03
        andwf   _vReg1_O+2,W
        iorwf   POSTINC0,F      ;Set EID<17:16>

        movff   _vReg1_O+1,POSTINC0
        movff   _vReg1_O,POSTINC0

        return






;****************************************************************************
;* Function:            _RotateLeft32N
;*
;*
;* PreCondition:        None
;*
;* Input:               W reg: Value for total rotation required
;*
;*                      _vReg1_O:_vReg1_O+3 - 32-bit value to be converted
;*                      (_vReg1_O= LL, _vReg1_O+1 =LH, _vReg1_O+2 = HL and _vReg1_O+3 = HH
;*                       byte)
;*
;* Output:              32 bit value in _vReg1_O shifted left by value in W times
;*
;* Side Effects:        Data Bank, STATUS, W changed
;*
;* Overview:            It shifts 32-bit value in _vReg1_O by W times and returns
;*                      result in _vReg1_O
;*
;****************************************************************************
_RotateLeft32N:

;        GLOBAL  _RotateLeft32N

        banksel _vReg1_O
RotateLMore:
        bcf     STATUS,C
        btfsc   _vReg1_O+3,7        ;Pre-read bit 31 to be copied into
        bsf     STATUS,C        ;Bit 0

        rlcf    _vReg1_O,F
        rlcf    _vReg1_O+1,F
        rlcf    _vReg1_O+2,F
        rlcf    _vReg1_O+3,F
        decfsz  WREG,F
        bra     RotateLMore

        return



;*********************************************************************
;* Function:            CANPrTxReadReg     RegAddr,
;*                                      Val,
;*                                      CAN_CONFIG_FLAGS type
;*
;* PreCondition:        None
;*
;* Input:               FSR0 - Starting address of a 32-bit buffer to be
;*                              read
;*                      FSR1 - Starting address of a memory location to
;*                              copy read 32 bit value
;*
;* Output:              Corresponding CAN id registers are read  and value is
;*                      bit adjusted and copied into 32-bit destination
;*
;* Side Effects:        Databank, W, STATUS, FSR0, FSR1 changed
;*
;* Overview:            If given id is of type standard identifier,
;*                      only SIDH and SIDL are read
;*                      If given id is of type extended identifier,
;*                      then EIDH and EIDL are read too to form 32 bit
;*                      value
;*
;* Stack requirements:  2 level deep
;*
;********************************************************************/
CANPrTxReadReg:

        GLOBAL  CANPrTxReadReg

        movf    POSTINC0,W      ;To increment Pointer to point towards
                                ;TxBnSIDL reg.
        btfsc   POSTDEC0,EXIDE  ;Check for Extended Fram
        bra     ReadExtFram     ;Yes Ext Frame, Read it.
                                ;No, Standard Frame
        banksel _vReg1_O
        clrf    _vReg1_O+2          ;Set High Word to 00 for STD message
        clrf    _vReg1_O+3
        movff   POSTINC0,_vReg1_O+1 ;Copy LH byte
        movff   INDF0,_vReg1_O      ;Save LL byte
        movlw   0x05
        rcall   _RotateRight32N  ;Rotate right for 5 times
        movlw   0x1f
        andwf   _vReg1_O+3,F        ;Mask EID <31:28>
        bra     CopyIDVal

ReadExtFram
        banksel _vReg1_O
        movff   POSTINC0,_vReg1_O+3 ;Save EID <28:21>
        movff   INDF0,_vReg1_O+2    ;save EID<20:16>
        clrf    _vReg1_O+1
        clrf    _vReg1_O
        movlw   0x03            ;Position EID <28:18>
        rcall   _RotateRight32N  ;
        movlw   0xfc            ;Mask EID<17:16>
        andwf   _vReg1_O+2,F
        movlw   0x03            ;Mask all except EID<17:16>
        andwf   POSTINC0,W
        iorwf   _vReg1_O+2,F
        movff   POSTINC0,_vReg1_O+1 ;Save EID<15:8>
        movff   POSTINC0,_vReg1_O   ;Save EID<7:0>

CopyIDVal
        movff   _vReg1_O,POSTINC1   ;Return LL byte
        movff   _vReg1_O+1,POSTINC1 ;Return LH byte
        movff   _vReg1_O+2,POSTINC1 ;Return HL byte (What if EXt?)
        movff   _vReg1_O+3,POSTINC1 ;Return HH byte

        return


;****************************************************************************
;* Function:            _RotateRight32N
;*
;*
;* PreCondition:        None
;*
;* Input:               W reg: Value for total rotation required
;*
;*                      _vReg1_O:_vReg1_O+3 - 32-bit value to be converted
;*                      (_vReg1_O= LL, _vReg1_O+1 =LH, _vReg1_O+2 = HL and _vReg1_O+3 = HH
;*                       byte)
;*
;* Output:              32 bit value in _vReg1_O shifted right by value in W times
;*
;* Side Effects:        Data Bank, W and STATUS changed
;*
;* Overview:            It shifts 32-bit value in _vReg1_O by W times and returns
;*                      result in _vReg1_O
;*
;****************************************************************************
_RotateRight32N:

;        GLOBAL  _RotateRight32N

        banksel _vReg1_O
RotateRMore:
        bcf     STATUS,C
        btfsc   _vReg1_O,0          ;Pre-read bit 0 to be copied into
        bsf     STATUS,C        ;Bit 31

        rrcf    _vReg1_O+3,F
        rrcf    _vReg1_O+2,F
        rrcf    _vReg1_O+1,F
        rrcf    _vReg1_O,F
        decfsz  WREG,F
        bra     RotateRMore

        return





;****************************************************************************
;* Function:            CANPrTxSendMsg
;*
;*
;* PreCondition:        None
;*
;* Input:               _vReg1_O<3:0> 32-bit Message ID
;*                      FSR1: starting address of data buffer
;*                      _DataLength - Data Length
;*                      m_Flags - CAN_TX_MSG_FLAGS type
;*
;* Output:              W reg = 0, If buffer is Full
;*                      W reg = 1, If successful
;*
;* Side Effects:        Databank, W, STATUS, FSR0, FSR1 changed
;*
;* Overview:            It copies the data in available hardware or software
;*                      buffer. If present data is of higher priority then
;*                      the data in hardware buffer then it aborts
;*                      transmission of lowest priority data in HW buffer and
;*                      copies it to SW buffer and copies present data to HW
;*                      Buffer for immediate transmission
;*
;* Stack requirements:  3 level deep
;*
;****************************************************************************
CANPrTxSendMsg:

        GLOBAL  CANPrTxSendMsg

        banksel vCANPrTxBufSize
        movf    vCANPrTxBufSize,W
        xorlw   TxBufMaxSize
        bnz     BufEmpty
        retlw   0x00            ;Send Error Code that Buffer is full

BufEmpty:
        clrf    TxFlags         ;Clear Buf set flags

        movlw   0x03
        subwf   vCANPrTxBufSize,W
        bnc     LoadHWBuf
        bra     LoadSWBuf

LoadHWBuf:

        banksel TXB0CON
        btfsc   TXB0CON,TXREQ,BANKED
        bra     ChkNxtHWBuf1

        movlw   low(TXB0SIDH)
        movwf   FSR0L           ;Save the address of destination register
        movlw   high(TXB0SIDH)
        movwf   FSR0H
        bsf     TxFlags,CANTxBuf0Flag   ;Indicate Buf 0 Set
        bra     CopyBufData


ChkNxtHWBuf1:
        banksel TXB1CON
        btfsc   TXB1CON,TXREQ, BANKED
        bra     ChkNxtHWBuf2
        movlw   low(TXB1SIDH)
        movwf   FSR0L           ;Save the address of destination register
        movlw   high(TXB1SIDH)
        movwf   FSR0H
        bsf     TxFlags,CANTxBuf1Flag   ;Indicate Buf 1 Set
        bra     CopyBufData

ChkNxtHWBuf2:
        banksel TXB2CON
        btfsc   TXB2CON,TXREQ, BANKED
        bra     CopyBufData     ; What ?
        movlw   low(TXB2SIDH)
        movwf   FSR0L           ;Save the address of destination register
        movlw   high(TXB2SIDH)
        movwf   FSR0H
        bsf     TxFlags,CANTxBuf2Flag   ;Indicate Buf 2 set

CopyBufData:
        nop
        SaveFSR0        ;FSR0 contains starting address of Reg to be read
        SaveFSR1        ;FSR1 contains starting address of Data Buffer
        mCANPrTxSetReg_PREG_DV_IF _TxFlags  ;Copy ID value in corresponding Regs.
        RestoreFSR0
        RestoreFSR1
        movlw   0x04            ;Point towards TXBnDLC reg.
        movff   _DataLength,PLUSW0
        btfss   _vFlags1_O,CAN_TX_RTR_BIT_NO	
        bsf     PLUSW0,TXRTR

        incf    WREG            ;Point towards TxBnD0
        addwf   FSR0L,F         ;Add W value into FSR0 to find
        movlw   0x00
        addwfc  FSR0H,F         ;new pointer value in FSR for TxBnD0
        banksel _DataLength
        movf    _DataLength,W  ;Data Length Counter to copy all data

CopyNxtTxData:
        movff   POSTINC1,POSTINC0
        decfsz  WREG            ;All data copied?
        bra     CopyNxtTxData   ;No, Copy next data

        mDisableCANTxInt              ;Disable Interrupt occurrence

        banksel vCANPrTxBufSize
        incf    vCANPrTxBufSize       ;Indicate new Buffer size

        btfss   TxFlags,CANTxSoftBufFlag
        bra     HWBufAnal

;This section checks for higher priority message in SW buffer than the HW
;buffer. If SW buf contains higher priority message then it Aborts the
;Transmission of lowest priority message in HW buf and Exchanges the data

        call    FindPriorSWBuf  ;Returns Highest Priority buffer number in
                                ;Temp2 and Data in _vTemp32Data+4

        banksel TXB0CON
        movlw   0x03
        andwf   TXB0CON,W,BANKED
        bnz     ChkBuf1
        mCANPrTxReadReg CAN_TX_IDB0, _vTemp32Data+4    ;
        call    _Cmp32Bit        ;Check if SW buffer priority is higher than
        bc      HWBufAnal       ;TxBuf0 Priority
        banksel TXB0CON
        bcf     TXB0CON,TXREQ,BANKED    ;If yes, Request Abort

WaitAbort0:
        btfsc   PIR3,TXB0IF     ;If TxBuf0 is transmitting then TxBuf0 Empty?
        bra     TxBuf0Emptied
        btfss   TXB0CON,TXABT,BANKED    ;Message Aborted?
        bra     WaitAbort0
        bra     TxB0Abtd        ;Message was successfully Aborted

TxBuf0Emptied:
        bcf     PIR3,TXB0IF     ;If TxBuf has finished the Transmission of
        bcf     PIE3,TXB0IE     ;current message than decrease buffer size
        banksel vCANPrTxBufSize
        decf    vCANPrTxBufSize,F


TxB0Abtd:
        movlw   low(TXB0SIDH)
        movwf   FSR0L
        movlw   high(TXB0SIDH)
        movwf   FSR0H
        call    ExchangeBuffers         ;Xchng highest priority SW Buffer data
        banksel TXB0CON                 ;with Tx Buffer 0 and
        bsf     TXB0CON,TXREQ,BANKED    ;Request Transmission
        bra     HWBufAnal


ChkBuf1:
        banksel TXB1CON
        movlw   0x03
        andwf   TXB1CON,W,BANKED
        bnz     ChkBuf2

        mCANPrTxReadReg CAN_TX_IDB1, _vTemp32Data+4    ;
        call    _Cmp32Bit        ;Check if SW Buf priority is higher than
        bc      HWBufAnal       ;TxBuf1
        banksel TXB1CON
        bcf     TXB1CON,TXREQ,BANKED    ;If yes, Request Abort


WaitAbort1:
        btfsc   PIR3,TXB1IF     ;TxBuf0 Empty?
        bra     TxBuf1Emptied
        btfss   TXB1CON,TXABT,BANKED    ;Message Aborted?
        bra     WaitAbort1
        bra     TxB1Abtd        ;Message was successfully Aborted

TxBuf1Emptied:
        bcf     PIR3,TXB1IF     ;If TxBuf has finished the Transmission of
        bcf     PIE3,TXB1IE     ;current message than decrease buffer size
        banksel vCANPrTxBufSize
        decf    vCANPrTxBufSize,F


TxB1Abtd:
        movlw   low(TXB1SIDH)
        movwf   FSR0L
        movlw   high(TXB1SIDH)
        movwf   FSR0H
        call    ExchangeBuffers         ;Exchange highest priority SW buffer
        banksel TXB1CON                 ;data with Tx Buffer 1 and
        bsf     TXB1CON,TXREQ,BANKED    ;Request Transmission
        bra     HWBufAnal


ChkBuf2:
        banksel TXB2CON
        movlw   0x03
        andwf   TXB2CON,W,BANKED
        bnz     HWBufAnal

        mCANPrTxReadReg CAN_TX_IDB2, _vTemp32Data+4    ;, _TxFlags
        call    _Cmp32Bit        ;Check if SW buffer priority is higher than
        bc      HWBufAnal       ;TxBuf2
        banksel TXB2CON
        bcf     TXB2CON,TXREQ,BANKED    ;If yes, Request Abort

WaitAbort2:
        btfsc   PIR3,TXB2IF     ;TxBuf0 Empty?
        bra     TxBuf2Emptied
        btfss   TXB2CON,TXABT,BANKED    ;Message Aborted?
        bra     WaitAbort2
        bra     TxB2Abtd        ;Message was successfully Aborted

TxBuf2Emptied:
        bcf     PIR3,TXB2IF     ;If TxBuf has finished the transmission of
        bcf     PIE3,TXB2IE     ;current message then decrease Buffer Size
        banksel vCANPrTxBufSize
        decf    vCANPrTxBufSize,F

TxB2Abtd:
        movlw   low(TXB2SIDH)
        movwf   FSR0L
        movlw   high(TXB2SIDH)
        movwf   FSR0H
        call    ExchangeBuffers         ;Exchange Highest priority SW buf data
        banksel TXB2CON                 ;with TxBuf2 and
        bsf     TXB2CON,TXREQ,BANKED    ;Request Transmission

HWBufAnal:
        rcall   _SetHWBufPrior

        mEnableCANTxInt
		movlw	1
		return
        ;retlw   0x01            ;Return success Code



LoadSWBuf:
        mullw   D'14'           ;Buffer size in bytes
        movf    PRODL,W         ;Get low byte of result in W reg
        addlw   low(TxBuffer)
        movwf   FSR0L           ;Find Starting address of Soft. Buffer
        clrf    FSR0H
        movlw   high(TxBuffer)
        addwfc  FSR0H,F
        bsf     TxFlags,CANTxSoftBufFlag
        bra     CopyBufData





;****************************************************************************
;* Function:            _SetHWBufPrior
;*
;*
;* PreCondition:        None
;*
;* Input:               None
;*
;*
;* Output:              It sets the priority of all HW Tx Buffer depending on
;*                      the Message ID in Buffer. Lower the ID value higher
;*                      the priority
;*
;*
;* Side Effects:        Databank, W and STATUS changed
;*
;* Overview:            It sets the CAN TX HW Buffer priority depending on the
;*                      CAN ID in buffer.
;*
;*
;****************************************************************************
_SetHWBufPrior:

;        GLOBAL  _SetHWBufPrior
        banksel TXB0CON
        movlw   0xfc
        andwf   TXB0CON,F,BANKED        ;Set Priority level 0 for TxBuf-0
        banksel TXB1CON
        andwf   TXB1CON,F,BANKED        ;Set Priority level 0 for TxBuf-1
        banksel TXB2CON
        andwf   TXB2CON,F,BANKED        ;Set Priority level 0 for TxBuf-2

        mCANPrTxReadReg CAN_TX_IDB0, _vTemp32Data      ;
        mCANPrTxReadReg CAN_TX_IDB1, _vTemp32Data+4    ;
        rcall   _Cmp32Bit
        btfsc   STATUS,C
        bra     Buf1HighPrior
        banksel TXB0CON
        incf    TXB0CON,F,BANKED
        bra     NxtCompare1

Buf1HighPrior:
        banksel TXB1CON
        incf    TXB1CON,F,BANKED

NxtCompare1:
        mCANPrTxReadReg CAN_TX_IDB0, _vTemp32Data      ;
        mCANPrTxReadReg CAN_TX_IDB2, _vTemp32Data+4    ;
        rcall   _Cmp32Bit
        btfsc   STATUS,C
        bra     Buf2HighPrior
        banksel TXB0CON
        incf    TXB0CON,F,BANKED
        bra     NxtCompare2

Buf2HighPrior:
        banksel TXB2CON
        incf    TXB2CON,F,BANKED

NxtCompare2:
        mCANPrTxReadReg CAN_TX_IDB1, _vTemp32Data      ;
        mCANPrTxReadReg CAN_TX_IDB2, _vTemp32Data+4    ;
        rcall   _Cmp32Bit
        btfsc   STATUS,C
        bra     Buf2HighPrior1
        banksel TXB1CON
        incf    TXB1CON,F,BANKED
        bra     NxtCompare3

Buf2HighPrior1:
        banksel TXB2CON
        incf    TXB2CON,F,BANKED

NxtCompare3:

        btfss   TxFlags,CANTxBuf0Flag
        bra     SetNxtBuf1
        banksel TXB0CON
        bsf     TXB0CON,TXREQ, BANKED
        bsf     PIE3,TXB0IE     ;Enable TxBuf0 Int.
SetNxtBuf1:
        btfss   TxFlags,CANTxBuf1Flag
        bra     SetNxtBuf2
        banksel TXB1CON
        bsf     TXB1CON,TXREQ, BANKED
        bsf     PIE3,TXB1IE     ;Enable TxBuf0 Int.
SetNxtBuf2:
        btfss   TxFlags,CANTxBuf2Flag
        bra     SetNxtBuf3
        banksel TXB2CON
        bsf     TXB2CON,TXREQ, BANKED
        bsf     PIE3,TXB2IE     ;Enable TxBuf0 Int.
SetNxtBuf3:


        return




;****************************************************************************
;* Function:            _Cmp32Bit
;*
;*
;* PreCondition:        None
;*
;* Input:               _vTemp32Data -  32-bit data 1
;*                      _vTemp32Data + 4 - 32-bit Data -2
;*
;* Output:              If Data 1 = Data 2 C =1, Z=1
;*                      If Data 1 > Data 2 C =1, Z=0
;*                      If Data 1 < Data 2 C =0, Z=0
;*
;*
;* Side Effects:        Databank W and STATUS changed
;*
;* Overview:            It compares two 32-bit data and sets the Flag
;*                      accordingly.
;*
;*
;****************************************************************************
_Cmp32Bit:

        GLOBAL  _Cmp32Bit

        banksel _vTemp32Data
        movf    _vTemp32Data+4,w
        subwf   _vTemp32Data,W

        movf    _vTemp32Data+5,w
        subwfb  _vTemp32Data+1,W

        movf    _vTemp32Data+6,w
        subwfb  _vTemp32Data+2,W

        movf    _vTemp32Data+7,w
        subwfb  _vTemp32Data+3,W

        return





;****************************************************************************
;* Function:            FindPriorSWBuf
;*
;* PreCondition:        None
;*
;* Input:               None
;*
;* Output:              Max. Priority buffer number into Temp2
;*
;* Side Effects:        Databank, W, STATUS and FSR0 changed
;*
;* Overview:            It finds highest priority Message Buffer number
;*
;****************************************************************************
FindPriorSWBuf:
        clrf    Temp3           ;Clear Buffer Counter
        banksel _vTemp32Data
        setf    _vTemp32Data      ;+4     ;Initialize Maximum data to 0
        setf    _vTemp32Data+1    ;+5
        setf    _vTemp32Data+2    ;+6
        setf    _vTemp32Data+3    ;+7

ChkNxtBufID:
        movf    Temp3,W
        mullw   D'14'           ;Buffer size in bytes
        movf    PRODL,W         ;Get result in W
        addlw   low(TxBuffer)
        movwf   FSR0L           ;Find Starting address of Soft. Buffer
        clrf    FSR0H
        movlw   high(TxBuffer)
        addwfc  FSR0H,F
;       mCANPrTxReadReg_PREG _vTemp32Data+4, CAN_CONFIG_XTD_MSG
        mCANPrTxReadReg_PREG _vTemp32Data+4
        rcall   _Cmp32Bit
        bnc     PrsntDataBig
        movff   _vTemp32Data+4 ,_vTemp32Data        ;Present Data is
                                                ;bigger so
        movff   _vTemp32Data+5 ,_vTemp32Data+1      ;save it for next comparison
        movff   _vTemp32Data+6 ,_vTemp32Data+2
        movff   _vTemp32Data+7 ,_vTemp32Data+3
        movff   Temp3,Temp2     ;Save software Buffer Number

PrsntDataBig:
        incf    Temp3,F
        movf    Temp3,W
        addlw   0x03
        banksel vCANPrTxBufSize
        xorwf   vCANPrTxBufSize,W
        bnz     ChkNxtBufID

        return





;****************************************************************************
;* Function:            ArrangeBuffers
;*
;* PreCondition:        None
;*
;* Input:               FSR0 - Starting address of buffer 1
;*                      Temp2 - SW buffer number to start sorting
;*
;* Output:              It copies buffer n+1 data to buffer n
;*                      It starts with the buffer number passed in Temp2
;*                      and ends at highest buffer
;*
;* Side Effects:        Databank, W, STATUS, FSR0 and FSR1 changed
;*
;* Overview:            It arranges the data in Software buffer for next
;*                      transmission.
;*
;****************************************************************************
ArrangeBuffers:
;       clrf    Temp2           ;Temp2 contains Buffer start number
                                ;Clear if want to start with Buf 0 to make
                                ;FIFO
XchngNxtBuf:
        movf    Temp2,W
        mullw   D'14'           ;Buffer size in bytes
        movf    PRODL,W         ;Get result in W
        addlw   low(TxBuffer)
        movwf   FSR0L           ;Find Starting address of present Soft. Buffer
        clrf    FSR0H
        movlw   high(TxBuffer)
        addwfc  FSR0H,F

        incf    Temp2,F
        movf    Temp2,W
        addlw   0x03
        banksel vCANPrTxBufSize
        subwf   vCANPrTxBufSize,W     ;Check if Max. Buf size has been reached
        bnc     EndXchng        ;Yes, then end exchanging
;       bz      EndXchng

        movf    Temp2,W
        mullw   D'14'           ;Buffer size in bytes
        movf    PRODL,W         ;Get result in W
        addlw   low(TxBuffer)
        movwf   FSR1L           ;Find Starting address of next Soft. Buffer
        clrf    FSR1H
        movlw   high(TxBuffer)
        addwfc  FSR1H,F


        movlw   D'13'
XchngNxtData:
        movff   POSTINC1,POSTINC0
        decfsz  WREG
        bra     XchngNxtData
        bra     XchngNxtBuf


EndXchng:
        return



;****************************************************************************
;* Function:            ExchangeBuffers
;*
;* PreCondition:        None
;*
;* Input:               FSR0 - Starting address of buffer 1
;*                      Temp2 - SW buffer number
;*
;* Output:
;*
;* Side Effects:        W, STATUS, PRODL, PRODH and FSR1 changed
;*
;* Overview:            It exchanges the data in Buffer pointed by FSR0 and
;*                      Software buffer (number in Temp2).
;*
;****************************************************************************
ExchangeBuffers:
        movf    Temp2,W
        mullw   D'14'           ;Buffer size in bytes
        movf    PRODL,W         ;Get result in W
        addlw   low(TxBuffer)
        movwf   FSR1L           ;Find Starting address of Soft. Buffer
        clrf    FSR1H
        movlw   high(TxBuffer)
        addwfc  FSR1H,F

        movlw   D'13'
XchngNxtData1:
        movff   INDF0,_Temp1_A
        movff   INDF1,POSTINC0
        movff   _Temp1_A,POSTINC1
        decfsz  WREG
        bra     XchngNxtData1

        return




;****************************************************************************
;* Function:            CANPrTxReadMsg
;*
;* PreCondition:        None
;*
;* Input:               FSR0 - Starting address of the buffer to store received
;*                      data
;*
;* Output:              W reg =0, If no data is pending
;*                      W reg= 1, If data is available
;*                      _vTemp32Data - Received message ID
;*                      _DataLength - Total data bytes received
;*                      _RxFlags - Type of message flags.
;*
;* Side Effects:        Databank, W, STATUS, FSR0, FSR1 changed
;*
;* Overview:            It formats the data in Rx Buffer and returns it.
;*                      It stores 32-bit Message ID, Recd. data, Data Length
;*                      and Receiver Status Flags into user supplied memory
;*                      locations.
;*
;* Stack requirements:  3 level deep
;*
;*****************************************************************************
CANPrTxReadMsg
        GLOBAL  CANPrTxReadMsg
        SaveFSR0

        banksel _RxFlags
        clrf    _RxFlags

        btfss   RXB0CON,RXFUL
        bra     ChkRxBuf1

        movlw   B'11110001'
        andwf   CANCON,F        ;Select WIN bits for buffer 0
        bcf     PIR3,RXB0IF     ;Clear flag indicating new message in Buf 0
        btfss   COMSTAT,RXB0OVFL        ;Check for Buf 0 Overflow
        bra     B0NotOvrFlow
        bsf     _RxFlags,CAN_RX_OVERFLOW_BIT_NO

B0NotOvrFlow:
        btfsc   RXB0CON ,FILHIT0
        bsf     _RxFlags,CAN_RX_FILTER_1_BIT_NO
        bra     CopyBuffer

ChkRxBuf1:

        banksel RXB1CON
        btfss   RXB1CON,RXFUL,BANKED
        bra     ReturnRxErrCode ;Return Error code

        movlw   B'11110001'
        andwf   CANCON,F        ;Select WIN bits for buffer 1
        movlw   B'00001010'     ;
        iorwf   CANCON,F        ;
        bcf     PIR3,RXB1IF     ;Clear flag indicating new message in Buf 1

        banksel _RxFlags
        btfss   COMSTAT,RXB1OVFL        ;Check for Buf 1 Overflow
        bra     B1NotOvrFlow
        bsf     _RxFlags,CAN_RX_OVERFLOW_BIT_NO

B1NotOvrFlow:
        movlw   CAN_RX_FILTER_BITS
        andwf   RXB0CON,W       ;Because of WIN bits accessing RXB1CON
        iorwf   _RxFlags       ;Store the Acceptance filter value into
                                ;Rx_Flags
        movlw   0xf8            ;Mask all bits except 3 LSB's
        andwf   RXB0CON,W       ;And with RXB1CON
        sublw   0x02            ;Whether Filter 0 or 1 caused hit
        bz      CopyBuffer      ;No, Then copy buffer data
        bnc     CopyBuffer

        bsf     _RxFlags,CAN_RX_DBL_BUFFERED_BIT_NO


CopyBuffer:
        movlw   0x0f
        andwf   RXB0DLC,W       ;Depending on the WIN bits actual register
                                ;will be accessed.
        banksel _DataLength
        movwf   _DataLength    ;copy Data length value into register

        banksel _RxFlags
        ;btfsc   RXB0CON,RXRTRRO ;Check for Remote Frame bit
#IF DEVICE == _18F458        	;XXX GS
		btfsc	RXB0CON,RXRTRRO	;XXX GS
#ELSE
        btfsc	RXB0CON,RXRTRRO_RXB0CON
#ENDIF        
        bsf     _RxFlags,CAN_RX_RTR_FRAME_BIT_NO

        btfsc   RXB0SIDL,EXID   ;Check for Remote Frame bit
        bsf     _RxFlags,CAN_RX_XTD_FRAME_BIT_NO

        movff   FSR0L,TFSR1     ;Transfer data in Temp. reg. for FSR1
        movff   FSR0H,TFSR1+1

        movlw   low(RXB0SIDH)   ;Point towards starting address of ID
        movwf   FSR0L		;gs get msg id
        movlw   high(RXB0SIDH)
        movwf   FSR0H

        mCANPrTxReadReg_PREG _vTemp32Data      ;, CAN_CONFIG_XTD_MSG
        RestoreFSR1             ;Get source data pointer in FSR1

        movlw   low(RXB0D0)     ;Point towards starting address of ID
        movwf   FSR0L
        movlw   high(RXB0D1)	;gs should this be D0? or does it matter?
        movwf   FSR0H

        banksel _DataLength
        movf    _DataLength,w
CopyNxtRxData:
        movff   POSTINC0,POSTINC1	;gs copy data from RXB0D0...
        decfsz  WREG
        bra     CopyNxtRxData

        bcf     RXB0CON,RXFUL   ;Indicate that buffer is open to receive Msg.

        movlw   B'11110001'
        andwf   CANCON,F        ;Select default WIN bits

        btfss   PIR3,IRXIF      ;Check for any invalid message occurrence on
        bra     NoInvldMsg      ;the CAN bus
        bcf     PIR3,IRXIF
        banksel _RxFlags       ;Return Invalid message flag bit
        bsf     _RxFlags,CAN_RX_INVALID_MSG_BIT_NO
NoInvldMsg:
		movlw	1		
		return
        ;retlw   0x01            ;Return Success code

ReturnRxErrCode:
		movlw	0
		return
        ;retlw   0x00            ;Return Error Code




;****************************************************************************
;* Function:            _SaveWorkRegs
;*
;* PreCondition:        None
;*
;* Input:               None
;*
;* Output:              None
;*
;* Side Effects:        None
;*
;* Overview:            It saves all GPR registers used by different functions
;*                      in Temp. locations.
;*
;****************************************************************************
_SaveWorkRegs
        movff   _vTemp32Data,ITemp32Data          ;Save _vTemp32Data
        movff   _vTemp32Data+1,ITemp32Data+1
        movff   _vTemp32Data+2,ITemp32Data+2
        movff   _vTemp32Data+3,ITemp32Data+3
        movff   _vTemp32Data+4,ITemp32Data+4
        movff   _vTemp32Data+5,ITemp32Data+5
        movff   _vTemp32Data+6,ITemp32Data+6
        movff   _vTemp32Data+7,ITemp32Data+7

        movff   _vReg1_O,IReg1      ;Save _vReg1_O
        movff   _vReg1_O+1,IReg1+1
        movff   _vReg1_O+2,IReg1+2
        movff   _vReg1_O+3,IReg1+3

        movff   _TxFlags,Im_TxFlags
        movff   TxFlags,ITxFlags
        movff   _Temp1_A,ITemp1    ;Save _Temp1_A
        movff   Temp2,ITemp2    ;Save Temp2
        movff   Temp3,ITemp3    ;Save Temp3

        movff   FSR0L,IFSR0     ;Save FSR 0
        movff   FSR0H,IFSR0+1

        movff   FSR1L,IFSR1     ;Save FSR 1
        movff   FSR1H,IFSR1+1

        return



;****************************************************************************
;* Function:            _RestoreWorkRegs
;*
;* PreCondition:        None
;*
;* Input:               None
;*
;* Output:              None
;*
;* Side Effects:        None
;*
;* Overview:            It saves all GPR registers used by different functions
;*                      in Temp. locations.
;*
;****************************************************************************
_RestoreWorkRegs
        movff   ITemp32Data,_vTemp32Data          ;Restore _vTemp32Data
        movff   ITemp32Data+1,_vTemp32Data+1
        movff   ITemp32Data+2,_vTemp32Data+2
        movff   ITemp32Data+3,_vTemp32Data+3
        movff   ITemp32Data+4,_vTemp32Data+4
        movff   ITemp32Data+5,_vTemp32Data+5
        movff   ITemp32Data+6,_vTemp32Data+6
        movff   ITemp32Data+7,_vTemp32Data+7

        movff   IReg1,_vReg1_O      ;Restore _vReg1_O
        movff   IReg1+1,_vReg1_O+1
        movff   IReg1+2,_vReg1_O+2
        movff   IReg1+3,_vReg1_O+3

        movff   ITxFlags,TxFlags
        movff   Im_TxFlags,_TxFlags
        movff   ITemp1,_Temp1_A    ;Restore _Temp1_A
        movff   ITemp2,Temp2    ;Restore Temp2
        movff   ITemp3,Temp3    ;Restore Temp3

        movff   IFSR0,FSR0L     ;Restore FSR 0
        movff   IFSR0+1,FSR0H

        movff   IFSR1,FSR1L     ;Restore FSR 1
        movff   IFSR1+1,FSR1H

        return






;*****************************************************************************
;* Function:            CANPrTxIsTxPassive()
;*
;* PreCondition:        None
;*
;* Input:               None
;*
;* Output:              C = 1, if CAN transmit module is error passive as
;*                      defined by CAN specifications.
;*                      C = 0, if CAN transmit module is in active state
;* Side Effects:        STATUS changed
;*
;*****************************************************************************
#ifdef  ADD_CANPrTxIsTxPassive
CANPrTxIsTxPassive
        GLOBAL  CANPrTxIsTxPassive
        bcf     STATUS,C
        btfsc   COMSTAT,TXBP
        bsf     STATUS,C        ;If Bus is in Tx passive state, set C
        return
#endif


;*****************************************************************************
;* Function:            CANPrTxIsRxPassive()
;*
;* PreCondition:        None
;*
;* Input:               None
;*
;* Output:              C = 1, if CAN receive module is error passive as
;*                      defined by CAN specifications.
;*                      C = 0, if CAN receive module is in active state
;* Side Effects:        STATUS changed
;*
;*****************************************************************************
#ifdef  ADD_CANPrTxIsRxPassive
CANPrTxIsRxPassive
        GLOBAL  CANPrTxIsRxPassive
        bcf     STATUS,C
        btfsc   COMSTAT,RXBP
        bsf     STATUS,C        ;If Bus is in Rx passive state, set C
        return
#endif



;*****************************************************************************
;* Function:            CANPrTxIsBusOff()
;*
;* PreCondition:        None
;*
;* Input:               None
;*
;* Output:              C = 1, if CAN transmit module is in Bus Off state as
;*                      defined by CAN specifications.
;*                      C = 0, if CAN module is not in bus Off state
;* Side Effects:        STATUS changed
;*
;*****************************************************************************
#ifdef  ADD_CANPrTxIsBusOff
CANPrTxIsBusOff

        GLOBAL  CANPrTxIsBusOff
        bcf     STATUS,C
        btfsc   COMSTAT,TXBO
        bsf     STATUS,C        ;If Bus is in off state, set C
        return
#endif



;****************************************************************************
;* Function:            BOOL  CANPrTxIsRxReady()
;*
;* PreCondition:        None
;*
;* Input:               None
;*
;* Output:              C=1, If one of the receive buffer is empty
;*                      C=0, If none of the receive buffer is empty.
;*
;* Side Effects:        Data banks, STATUS  changed
;*
;****************************************************************************
#ifdef  ADD_CANPrTxIsRxReady
CANPrTxIsRxReady

        GLOBAL  CANPrTxIsRxReady
        bcf     STATUS,C
        btfsc   RXB0CON,RXFUL
        bsf     STATUS,C        ;If buffer is empty than set carry to
                                ;indicate

        banksel RXB1CON
        btfsc   RXB1CON,RXFUL,BANKED
        bsf     STATUS,C

        return
#endif


;****************************************************************************
;* Macro:               BOOL CANPrTxIsTxReady()
;*
;* PreCondition:        None
;*
;* Input:               None
;*
;* Output:              C=1, If at least one CAN transmit buffer is empty
;*                      C=0, If all CAN transmit buffers are full
;*
;* Side Effects:        Databank, W and STATUS changed
;*
;****************************************************************************
#ifdef  ADD_CANPrTxIsTxReady
CANPrTxIsTxReady

        GLOBAL  CANPrTxIsTxReady
        banksel vCANPrTxBufSize
        movf    vCANPrTxBufSize,W
        sublw   TxBufMaxSize
        bnz     TxReady1
        bcf     STATUS,C        ;
TxReady1:
        return
#endif


;****************************************************************************
;* Function:            CANPrTxISR
;*
;* PreCondition:        None
;*
;* Input:               None
;*
;* Output:              None
;*
;* Side Effects:        W, STATUS and BSR changed
;*
;* Overview:            If any data is pending for transmission then it will
;*                      copy highest priority data in Empty buffer and
;*                      requests Transmission.
;*
;* Stack requirements:  4 level deep
;*

;****************************************************************************
CANPrTxISR:
        GLOBAL  CANPrTxISR
	;movlw	'I'
	;call	DEBUGGSISendByte232

	movf	PIE3,W			;get enable mask
	andwf	PIR3,W			;see if int source is enabled
        ;movf	PIR3,W			;check all CAN int sources
	;btfsc	STATUS,Z
        ;goto	EndCANPrTxISR       	;end processing if none pending	
	btfsc	STATUS,Z
	goto	EndCANPrTxISR
	;-
	movff	BSR,CnPrevBSR		;store prev BSR

	btfsc	PIR3,IRXIF		;check for invalid message on CAN network
	goto	InvalidMessage

	;movwf	DCWTemp
	;call	GSISendBinary232ISR	;send binary to rs232
	;movf	DCWTemp,W

	movff	CANCON,GlbCANCONTemp	
	movff	CANSTAT,GlbCANSTATTemp
	movf	GlbCANSTATTemp,W		;get ICODE bits
	andlw	b'00001110'		;jmp vector assist flag

	banksel	CnBank
#if DEBUG_CAN
	;movwf	DCWTemp			;jmp vector already rol,1
	;rrncf	WREG			;Byte ptr 
	;call	GSISendBinary232ISR	;send binary to rs232
	;movlw	'I'
	;call	DEBUGGSISendByte232
	;movf	DCWTemp,W
#endif
	;addwf	WREG		;not reqd, already left shifted,1
	movwf	CnISRCompOffset
	movlw 	LOW ISRJmpTbl 	;get low 8 bits of address
	addwf 	CnISRCompOffset,F	;do an 8-bit add operation
	movlw 	HIGH ISRJmpTbl 	;get high 5 bits of address
	btfsc 	STATUS,C,A 	;page crossed?
	addlw 	1 		;yes then increment high address
	movwf	PCLATH,A 		;load high address in latch
	movf	CnISRCompOffset,W	;load computed offset in w reg

	;bsf	LATC,1

	movwf	PCL

ISRJmpTbl
	bra	NoInterrupt		;an error
	bra	ErrorInterrupt
	bra	TxInt			;all handled by prioritised
	bra	TxInt			;Tx buffer handler
	bra	TxInt			;TxB0,TxB1,TxB2
	bra	RxB1Interrupt
	bra	RxB0Interrupt
WakeupInterrupt				;part of the computed goto
					;wake up code must go here
	;bcf	LATC,1
	bcf	PIR3,WAKIF
	movff	CnPrevBSR,BSR		;restore BSR
	bra	NoErrorReturn
	
NoInterrupt				;An error, so cause a wdt
;-------------------------------------------------------------
;No int can be caused by CAN ints (eg bcf PIE3,RXBIE) being disabled
;and another non-can int firing. The PIR3,RXB1IF can be set because
;a CAN int condition has ocurred thus setting the RXB1IF int flag,
;however it was not the cause of the interrupt. On re-enable of the
;PIE3,RXBIE flag the int will occur.
;SOLUTION:
;Simple: Use the PIE3 as an AND mask for PIR3, quit the ISR if 
;masked test fails to find an int
	movlw	'N'
	call	DEBUGGSISendByte232
NiLoop
	;bcf	LATC,1
	movf	CANSTAT,W
#IF GSI_LCD_SUPPORT	
	banksel	LdBank
	call	GSIDisplayBinary
#ENDIF	
__cli
	bcf	INTCON,GIEH		;stop all ints
	btfsc	INTCON,GIEH
	bra	__cli

NiLoop1
	;clrwdt
	;bra	NiLoop1
	;goto	DEBUGCauseWDT
	movlw	FATAL_ERROR_ISR_NO_INTERRUPT
	call	DEBUGCauseWDT	;use call, then can return using icd2
	;bra	NiLoop			;wdt to flag error

ErrorInterrupt
;COMSTAT holds error state info
	;bcf	LATC,1
	call	CNErrorHandlerErrInt	
	bcf	PIR3,ERRIF		;allow further ERROR ints
	bra	NoErrorReturn

RxB1Interrupt
	;bsf	LATC,1
	;bcf	LATC,1
	;movlw	'B'
	;call	DEBUGGSISendByte232
	;call	CANAddToRxBuff
	;Note that the CQ holds messages ready to be sent down rs232 line
	call	GSCANReadMsgToCQ

	bsf	PIE3,RXB1IE		;disable further ints
	bcf	PIR3,RXB1IF		;PIR3 RXB1IF cleared by ReadMsgToXQ

	;bsf	GlbCANState,CAN_RX_1
	bra	NoErrorReturn

;-----------------------------------------------------------------------------
;The message is guaranteed to be in Rx0
;an Rx1 interrupt could also be a high priority message sent to Rx1 because Rx0
;is currently full
;------------------------------------------------------------------------------
RxB0Interrupt:
	;movlw	'A'
	;call	DEBUGGSISendByte232

	;bcf	PIR3,RXB0IF		;cleared by TxReadMsg

	;bsf	LATC,1
	;bcf	LATC,1
	call	GSCANReadMsgToCQ
	bsf	PIE3,RXB0IE		;enable ints
	bcf	PIR3,RXB0IF		;PIR3 RXB0IF cleared by ReadMsgToXQ
	;remove lock
	;bsf	GlbCANState,CAN_RX_0
	bra	NoErrorReturn

TxInt:
	;movlw	'T'
	;call	DEBUGGSISendByte232
	;bcf	LATC,1

        call    _SaveWorkRegs
        clrf    TxFlags
        btfss   PIR3,TXB0IF     ;TxBuf0 Empty?
        bra     ChkTxBuf1Int
        bcf     PIR3,TXB0IF
        bcf     PIE3,TXB0IE
        bsf     TxFlags,CANTxBuf0Flag
        call    FindPriorSWBuf  ;Returns Highest Priority buffer number in
                                ;Temp2 and Data in _vTemp32Data+4
        movlw   low(TXB0SIDH)
        movwf   FSR0L
        movlw   high(TXB0SIDH)
        movwf   FSR0H
        bra     BufSet
ChkTxBuf1Int:

        btfss   PIR3,TXB1IF     ;TxBuf1 Empty?
        bra     ChkTxBuf2Int
        bcf     PIR3,TXB1IF
        bcf     PIE3,TXB1IE
        bsf     TxFlags,CANTxBuf1Flag
        call    FindPriorSWBuf  ;Returns Highest Priority buffer number in
                                ;Temp2 and Data in _vTemp32Data+4
        movlw   low(TXB1SIDH)
        movwf   FSR0L
        movlw   high(TXB1SIDH)
        movwf   FSR0H
        bra     BufSet
ChkTxBuf2Int:

        btfss   PIR3,TXB2IF     ;TxBuf2 Empty?
        bra     NoBufEmpty
        bcf     PIR3,TXB2IF
        bcf     PIE3,TXB2IE
        bsf     TxFlags,CANTxBuf2Flag
        call    FindPriorSWBuf  ;Returns Highest Priority buffer number in
                                ;Temp2 and Data in _vTemp32Data+4
        movlw   low(TXB2SIDH)
        movwf   FSR0L
        movlw   high(TXB2SIDH)
        movwf   FSR0H

BufSet:
        movlw   0x03            ;Check whether any data is pending in Buffer
        banksel vCANPrTxBufSize
        decf    vCANPrTxBufSize,F
        subwf   vCANPrTxBufSize,W
        bc      BufPending
        bz      BufPending
        bra     NoBufEmpty

BufPending:
        call    ExchangeBuffers
        rcall   _SetHWBufPrior
        call    ArrangeBuffers

NoBufEmpty:
        rcall   _RestoreWorkRegs
	IF	DEBUG_CAN
		movwf	DCWTemp
		movlw	0x0a
		call	DEBUGGSISendByte232
		movlw	0x0d
		call	DEBUGGSISendByte232
		movf	DCWTemp,W
	ENDIF
NoErrorReturn
	;restore CANCON WIN bits
	movf	CANCON,W
	andlw	b'11110001'
	iorwf	GlbCANCONTemp,W
	movwf	CANCON
im_return
	movff	CnPrevBSR,BSR		;restore BSR

EndCANPrTxISR
	;bcf	LATC,1
        return
InvalidMessage
	;Not in CAN BANK
	banksel	CnErrorState1	
	;bcf	LATC,1

	bcf	PIE3,IRXIE		;disable int
	bcf	PIR3,IRXIF		;ack the int, allow more
	bsf	CnErrorState1,CN_ERROR1_IRXIF ;invalid message
	bsf	GlbErrState,GSI_ERR_CAN	;inform Error.asm
	bra	im_return

;-----------------------------------------------------------------
;	CNErrorHandlerErrInt
;Multiple sources for the Error Interrupt
;find which error(s) and re-enable int
;Must re-enable the Error int and clr the individual error source
;bits in COMSTAT
;--------------------------------------------------------------------

CNErrorHandlerErrInt

	banksel	CnErrorState
	;bcf	LATC,1

	IF DEBUG
	movlw	'E'
	call	DEBUGGSISendByte232
	ENDIF 
	movf	COMSTAT,W
	call	DEBUGGSISendBinary232ISR
	movf	COMSTAT,W

	;tell error.asm about it so will print message to lcd/rs232

	bsf	GlbErrState,GSI_ERR_CAN	

	;must clr the COMSTAT flag that caused the int, if more than
	;one flag set then prioritise and despatch
	btfss	COMSTAT,RXB0OVFL	
	bra	ceh1
	;Rx buffer 0 ovf
	bsf	CnErrorState,CN_ERROR_RXB0OVFL	
	bcf	COMSTAT,RXB0OVFL	;pretend we fixed it
ceh1
	btfss	COMSTAT,RXB1OVFL	
	bra	ceh2
	;Rx buffer 1 ovf
	bsf	CnErrorState,CN_ERROR_RXB1OVFL	
	bcf	COMSTAT,RXB1OVFL	;pretend we fixed it
ceh2
	btfss	COMSTAT,TXBO
	bra	ceh3
	;Tx Bus Off
	bsf	CnErrorState,CN_ERROR_TXBO
	bcf	COMSTAT,TXBO		;pretend we fixed it
ceh3		
	btfss	COMSTAT,TXBP
	bra	ceh4
	;Tx  Bus Passive
	bsf	CnErrorState,CN_ERROR_TXBP
	bcf	COMSTAT,TXBP		;pretend we fixed it
ceh4		
	btfss	COMSTAT,RXBP
	bra	ceh5
	;Rx  Bus Passive
	bsf	CnErrorState,CN_ERROR_RXBP
	bcf	COMSTAT,RXBP		;pretend we fixed it
ceh5		
	btfss	COMSTAT,TXWARN
	bra	ceh6
	;Tx warning: Tx error counter >95  (<128)
	bsf	CnErrorState,CN_ERROR_TXWARN
	bcf	COMSTAT,TXWARN		;pretend we fixed it
ceh6		
	btfss	COMSTAT,RXWARN
	bra	ceh7
	;Rx warning: Rx error counter >95  (<128)
	bsf	CnErrorState,CN_ERROR_RXWARN
	bcf	COMSTAT,RXWARN		;pretend we fixed it
ceh7	
	;EWARN is flag for RW/TXWARN bit.
	;As we test both explicitly, no need to process, just clr
	btfss	COMSTAT,EWARN
	bra	ceh8
	;Rx  Bus Passive
	;bsf	CnErrorState,CN_ERROR_EWARN
	bcf	COMSTAT,EWARN		;pretend we fixed it
ceh8	
	
	IF DEBUG		
	movlw	'e'
	call	DEBUGGSISendByte232
	movf	CnErrorState,W
	call	DEBUGGSISendBinary232ISR
	movf	CnErrorState1,W
	call	DEBUGGSISendBinary232ISR
	ENDIF

	return

	IF 0
;-----------------------------------------------------------------------
;	CANAddToRxBuff
;called from an ISR, so must preserve any global resources and keep
;processing to a minimum
;ASSUMES BSR;CnBank
;a message is in the pics CAN rx buffer(s).
;extract it with the supplied macro. Flag byte will indicate the filter
;used to collect the mssg
;-----------------------------------------------------------------------
CANAddToRxBuff
;EXTERN	CnCQRxRCursor,CnCQRxWCursor,CnCQRxNumInQ,CnCQRx
;CAN_RX_CQ_MAX_MSSG	EQU	3
;CAN_FIXED_MSSG_SIZE	EQU	14	;id:4,data:8,len:1,flags:1	
;CAN_MSSG_OFFSET_ID	EQU	0
;CAN_MSSG_OFFSET_DATA	EQU	4
;CAN_MSSG_OFFSET_LENGTH	EQU	12
;CAN_MSSG_OFFSET_FLAGS	EQU	13

	movlw	CAN_RX_CQ_MAX_MSSG
	cpfslt	CnCQRxNumInBuff		;skip if f <W
	bra	_catrb_error

	movf	CnCQRxNumInBuff,W
	bz	_catrb0
	decf	WREG
	bz	_catrb1
	messg !!!!!!!!!! HERE !!!!!!!!!!!!!!!!!
	IF CAN_RX_CQ_MAX_MSSG >3
		error require code here
	ENDIF
_catrb2	
	;mCANPrTxReadMsg  CnCQRx+(3*CAN_FIXED_MSSG_SIZE),CnCQRx+(3*CAN_FIXED_MSSG_SIZE+CAN_MSSG_OFFSET_DATA),CnCQRx+ +(3*CAN_FIXED_MSSG_SIZE+CAN_MSSG_OFFSET_LENGTH),CnCQRx+(3*CAN_FIXED_MSSG_SIZE+CAN_MSSG_OFFSET_FLAGS)
	;mCANPrTxReadMsg  CnCQRx+42,CnCQRx+46,CnCQRx+54,CnCQRx+55; CnCQRx+CAN_MSSG_OFFSET_DATA+(3*CAN_FIXED_MSSG_SIZE),0,0; CnCQRx+(3*(CAN_FIXED_MSSG_SIZE+CAN_MSSG_OFFSET_LENGTH)),CnCQRx+(3*(CAN_FIXED_MSSG_SIZE+CAN_MSSG_OFFSET_FLAGS))
	mCANPrTxReadMsg  CnCQRxBuff+28,CnCQRxBuff+32,CnCQRxBuff+40,CnCQRxBuff+41; CnCQRx+CAN_MSSG_OFFSET_DATA+(3*CAN_FIXED_MSSG_SIZE),0,0; CnCQRx+(3*(CAN_FIXED_MSSG_SIZE+CAN_MSSG_OFFSET_LENGTH)),CnCQRx+(3*(CAN_FIXED_MSSG_SIZE+CAN_MSSG_OFFSET_FLAGS))
	movf	WREG
	bz	_catrb_error
	
	bcf	STATUS,C
	return
_catrb1	
	mCANPrTxReadMsg  CnCQRxBuff+14,CnCQRxBuff+18,CnCQRxBuff+26,CnCQRxBuff+27; CnCQRx+CAN_MSSG_OFFSET_DATA+(3*CAN_FIXED_MSSG_SIZE),0,0; CnCQRx+(3*(CAN_FIXED_MSSG_SIZE+CAN_MSSG_OFFSET_LENGTH)),CnCQRx+(3*(CAN_FIXED_MSSG_SIZE+CAN_MSSG_OFFSET_FLAGS))
	movf	WREG
	bz	_catrb_error
	return
_catrb0
	mCANPrTxReadMsg  CnCQRxBuff,CnCQRxBuff+4,CnCQRxBuff+12,CnCQRxBuff+13; CnCQRx+CAN_MSSG_OFFSET_DATA+(3*CAN_FIXED_MSSG_SIZE),0,0; CnCQRx+(3*(CAN_FIXED_MSSG_SIZE+CAN_MSSG_OFFSET_LENGTH)),CnCQRx+(3*(CAN_FIXED_MSSG_SIZE+CAN_MSSG_OFFSET_FLAGS))
	movf	WREG
	bz	_catrb_error
_catrb_no_error_exit
	incf	CnCQRxNumInBuff,F
	bcf	STATUS,C
	return	
_catrb_error
	bsf	STATUS,C
	return

	ENDIF


;****************************************************************************
;* Function:            _GSRotateRight32N
;*
;*
;* PreCondition:        None
;*
;* Input:               W reg: Value for total rotation required
;*
;*                      _vReg1_O:_vReg1_O+3 - 32-bit value to be converted
;*                      (_vReg1_O= LL, _vReg1_O+1 =LH, _vReg1_O+2 = HL and _vReg1_O+3 = HH
;*                       byte)
;*
;* Output:              32 bit value in _vReg1_O shifted right by value in W times
;*
;* Side Effects:        Data Bank, W and STATUS changed
;*
;* Overview:            It shifts 32-bit value in _vReg1_O by W times and returns
;*                      result in _vReg1_O
;*
;****************************************************************************
_GSRotateRight32N:

;        GLOBAL  _RotateRight32N

        ;banksel _vReg1_O
GSRotateRMore:
        bcf     STATUS,C
        btfsc   _CnvReg1_O,0          ;Pre-read bit 0 to be copied into
        bsf     STATUS,C        ;Bit 31

        rrcf    _CnvReg1_O+3,F
        rrcf    _CnvReg1_O+2,F
        rrcf    _CnvReg1_O+1,F
        rrcf    _CnvReg1_O,F
        decfsz  WREG,F
        bra     GSRotateRMore

        return


;*********************************************************************
;* Function:            GSCANReadReg     RegAddr,
;*                                      Val,
;*                                      CAN_CONFIG_FLAGS type
;*
;* PreCondition:        None
;*
;* Input:               FSR0 - Starting address of a 32-bit buffer to be
;*                              read
;*                      FSR1 - Starting address of a memory location to
;*                              copy read 32 bit value
;*
;* Output:              Corresponding CAN id registers are read  and value is
;*                      bit adjusted and copied into 32-bit destination
;*
;* Side Effects:        Databank, W, STATUS, FSR0, FSR1 changed
;*
;* Overview:            If given id is of type standard identifier,
;*                      only SIDH and SIDL are read
;*                      If given id is of type extended identifier,
;*                      then EIDH and EIDL are read too to form 32 bit
;*                      value
;*
;* Stack requirements:  2 level deep
;*
;********************************************************************/
GSCANReadReg:

        movf    POSTINC0,W      ;To increment Pointer to point towards
                                ;TxBnSIDL reg.
	IF SIMULATOR
		bsf	INDF0,EXIDE
	ENDIF
        btfsc   POSTDEC0,EXIDE  ;Check for Extended Frame
        bra     GSReadExtFram     ;Yes Ext Frame, Read it.
                                ;No, Standard Frame
        ;banksel _GSvReg1_O
        clrf    _CnvReg1_O+2          ;Set High Word to 00 for STD message
        clrf    _CnvReg1_O+3
        movff   POSTINC0,_CnvReg1_O+1 ;Copy LH byte
        movff   INDF0,_CnvReg1_O      ;Save LL byte
        movlw   0x05
        rcall   _GSRotateRight32N  ;Rotate right for 5 times
        movlw   0x1f
        andwf   _CnvReg1_O+3,F        ;Mask EID <31:28>
        bra     GSCopyIDVal

GSReadExtFram
        ;banksel _GSvReg1_O
        movff   POSTINC0,_CnvReg1_O+3 ;Save EID <28:21>
        movff   INDF0,_CnvReg1_O+2    ;save EID<20:16>
        clrf    _CnvReg1_O+1
        clrf    _CnvReg1_O
        movlw   0x03            ;Position EID <28:18>
        rcall   _GSRotateRight32N  ;
        movlw   0xfc            ;Mask EID<17:16>
        andwf   _CnvReg1_O+2,F
        movlw   0x03            ;Mask all except EID<17:16>
        andwf   POSTINC0,W
        iorwf   _CnvReg1_O+2,F
        movff   POSTINC0,_CnvReg1_O+1 ;Save EID<15:8>
        movff   POSTINC0,_CnvReg1_O   ;Save EID<7:0>

GSCopyIDVal
        ;movff   _GSvReg1_O,POSTINC1   ;Return LL byte
        ;movff   _GSvReg1_O+1,POSTINC1 ;Return LH byte
        ;movff   _GSvReg1_O+2,POSTINC1 ;Return HL byte (What if EXt?)
        ;movff   _GSvReg1_O+3,POSTINC1 ;Return HH byte

        return


;****************************************************************************
;* Function:            GSCANReadMsgToCQ
;*
;* PreCondition:        None
;*
;* Input:               FSR0 - Starting address of the buffer to store received
;*                      data
;* 			GS: data will be id:4 bytes, length:1 Byte, data:0-8 bytes, flag:1 byte
;*
;* Output:		C set if error
;*	                W reg =1, If no data is pending
;*                      W reg= 2, If queue overflow
;*
;* Side Effects:        Databank, W, STATUS, FSR0, FSR1 changed
;*
;* Overview:            It formats the id;length;data in Rx Buffer; flags
;*			into a circular queue.
;*			Called from the ISR so preserve resources
;*
;* Stack requirements:  3 level deep
;*
;*****************************************************************************
GSCANReadMsgToCQ:
;CnCQRxRCursor	RES	1
;CnCQRxWCursor	RES	1
;CnCQRxNumInQ	RES	1
;CnCQRx		RES	CAN_MSSG_FIXED_SIZE * CAN_RX_CQ_MAX_MSSG

        GLOBAL  GSCANReadMsgToCQ
        GSSaveFSR0
        GSSaveFSR1

	banksel	CnBank
	clrf    _CnRxFlags

	IF SIMULATOR == FALSE
	        btfss   RXB0CON,RXFUL
        	bra     GSChkRxBuf1
	ENDIF


        movlw   B'11110001'
        andwf   CANCON,F        ;Select WIN bits for buffer 0
        bcf     PIR3,RXB0IF     ;Clear flag indicating new message in Buf 0
        btfss   COMSTAT,RXB0OVFL        ;Check for Buf 0 Overflow
        bra     GSB0NotOvrFlow
        bsf     _CnRxFlags,CAN_RX_OVERFLOW_BIT_NO

GSB0NotOvrFlow:
        btfsc   RXB0CON ,FILHIT0
        bsf     _CnRxFlags,CAN_RX_FILTER_1_BIT_NO
        bra     GSCopyBuffer

GSChkRxBuf1:
        banksel RXB1CON
        btfss   RXB1CON,RXFUL,BANKED
	;banksel	CnBank
        bra     GSReturnRxErrCode ;Return Error code
	banksel	CnBank
 
        movlw   B'11110001'
        andwf   CANCON,F        ;Select WIN bits for buffer 1
        movlw   B'00001010'     ;
        iorwf   CANCON,F        ;
        bcf     PIR3,RXB1IF     ;Clear flag indicating new message in Buf 1

        ;banksel _RxFlags
	;banksel	CnBank
        btfss   COMSTAT,RXB1OVFL        ;Check for Buf 1 Overflow
        bra     GSB1NotOvrFlow
        bsf     _CnRxFlags,CAN_RX_OVERFLOW_BIT_NO

GSB1NotOvrFlow:
        movlw   CAN_RX_FILTER_BITS
        andwf   RXB0CON,W       ;Because of WIN bits accessing RXB1CON
        iorwf   _CnRxFlags       ;Store the Acceptance filter value into
                                ;Rx_Flags
        movlw   0xf8            ;Mask all bits except 3 LSB's
        andwf   RXB0CON,W       ;And with RXB1CON
        sublw   0x02            ;Whether Filter 0 or 1 caused hit
        bz      GSCopyBuffer      ;No, Then copy buffer data
        bnc     GSCopyBuffer

        bsf     _CnRxFlags,CAN_RX_DBL_BUFFERED_BIT_NO

GSCopyBuffer:
        movlw   0x0f		;AND out RTR and reserved bits
        andwf   RXB0DLC,W       ;Depending on the WIN bits actual register
                                ;will be accessed.
	IF SIMULATOR 
		movlw 7
	ENDIF
	movwf	_CnDataLength
#IF DEVICE == _18F458		;XXX GS
        btfsc   RXB0CON,RXRTRRO ;Check for Remote Frame bit
#ELSE        
   		btfsc   RXB0CON,RXRTRRO_RXB0CON ;Check for Remote Frame bit
#ENDIF   		
        bsf     _CnRxFlags,CAN_RX_RTR_FRAME_BIT_NO

        btfsc   RXB0SIDL,EXID   ;Check for extended Frame bit
        bsf     _CnRxFlags,CAN_RX_XTD_FRAME_BIT_NO
	IF SIMULATOR
        bsf     _CnRxFlags,CAN_RX_XTD_FRAME_BIT_NO
	ENDIF

        movlw   low(RXB0SIDH)   ;Point towards starting address of ID (uses WIN)
        movwf   FSR0L		
        movlw   high(RXB0SIDH)
        movwf   FSR0H
	;FSR0:src, FSR1:dst
        rcall	GSCANReadReg		;	;FSR0:src, returns id in _cnvReg1_O
;-Add the id bytes to the CQ
;-Only check the last one for an error

        movlw   low(RXB0D0)     ;Point towards starting address of data
        movwf   FSR0L
        movlw   high(RXB0D1)	;gs should this be D0? or does it matter?
        movwf   FSR0H
;On error CANAddToCQ returns NZ
	rcall	GSCANAddToCQ	;FSR0 points to mssg data NZ==error
	IF DEBUG
		movlw	FATAL_ERROR_CAN_CQ_OVF	;setup for error
		btfss	STATUS,Z
		call	DEBUGCauseWDT	
	    bcf     RXB0CON,RXFUL   ;Indicate that buffer is open to receive Msg.
	ENDIF
        movlw   B'11110001'
        andwf   CANCON,F        ;Select default WIN bits

        btfss   PIR3,IRXIF      ;Check for any invalid message occurrence on
        bra     GSNoInvldMsg      ;the CAN bus
        bcf     PIR3,IRXIF
	;messg	Need to add this bit to the buffered message: lost otherwise!!
        bsf     _CnRxFlags,CAN_RX_INVALID_MSG_BIT_NO 
GSNoInvldMsg:
	GSRestoreFSR0
	GSRestoreFSR1
	;movlw	0
	bcf	STATUS,C
        retlw   0x00            ;Return Success code
GSReturnRxErrCode:
	banksel	CnBank
	movlw	1
	bra	_gscrmtc_error_exit
_gscrmtc1_ovf:
	movlw	2
_gscrmtc_error_exit:
	GSRestoreFSR0
	GSRestoreFSR1
	bsf	STATUS,C
	return


;------------------------------------------------------------------------
;	GSCANAddToCQ
;circular buffer for CAN msg's
;CnCQRx holds mssgs awaiting processing by this node. This will happen in the main
;loop and is a synchronous process that should occur quickly, thus this Q can be small
;in size
;Assumes CnBank
;returns Z no error
;returns NZ error
;called at int time so no nested ints can occur. WARNING, a high priority int can occur!
;msg id in _CnvReg1_O,length in _CnDataLength
;total length of mssg is id(4)+length(1)+data(0-8)+flags(1)
;fixed length=6 bytes
;Store:
;id:4 bytes
;length:1 byte
;data:0-8 bytes
;flags:1 byte

;A second CQ CnCQRx2R2 is also written to.
;This CQ is used to hold the can messages while they are awaiting
;Tx down the rs232 line. This is an aycnronous process in that we depend on
;communication with a PC. It must agree to the Tx of the mssg's so this CQ
;should ideally be larger in size. 
;Currently they are both 3 messages of maximum (8 data bytes) size.
;this size include the flag. so 13 bytes *3=39 bytes for each cq

;------------------------------------------------------------------------
GSCANAddToCQ:
	DW	0xffff 

#if DISABLE_CN_SYNC_RX
;has to be controlled by a variable as need SYNC for getting slave id
	btfss	CnFilter0Mask,CN_FILTER0_NO_SYNC	;don't place SYNC messages in buffer (actually, currently *all* filter 0 messages)
	bra		gcatc_no_filter
;See if need to ignore all Filter 0 messages
	movf	_CnRxFlags,W	;see which filter accepted this message
	andlw	CAN_RX_FILTER_BITS	;mask out non-filter bits		
	bz	atcq_no_error_exit			;filter 0 message so ignore. Allows us to debug without SYNC id's always appearing
#endif

gcatc_no_filter:
	call	GSCANAddToR2CQ				;add to rs232 Q, for Tx to pc
	
	movf	_CnDataLength,W
	addwf	CnCQRxNumInBuff,W		;add to num bytes currently in buffer
	sublw	(CQ_CAN_RX_BUFFER_SIZE-CQ_CAN_MSSG_FIXED_MSSG_SIZE)
	bnc	atcqOvf

;ok mssg will fit in buffer.
;indirectly address the id
        lfsr	FSR0, _CnvReg1_O
;now add the rest of the id. id and length bytes are contiguous in memory so loop 
;counter can include it.
	movlw	CAN_MSSG_MAX_SIZEOF_ID+CAN_MSSG_SIZEOF_LENGTH 
	movwf	_CnAddToCQCounter		;counter
;Set up buffer insertion point
	movf	CnCQRxWCursor,W			;Ptr to buff insertion point
	movwf	FSR1L,A				;Indirect access insertion point
	movlw	HIGH CnCQRxBuff	
    movwf   FSR1H
	;
gcatc1:
;FSR1 points to correct insertion point, FSR0 points to id:length bytes
	movff	POSTINC0,POSTINC1		
;Buffer management
	movlw	CnCQRxBuffEndLbl		;address of end of buff. Keep in W for quicker access
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxBuff			;start pos of buffer	
	decfsz	_CnAddToCQCounter
	bra	gcatc1
;id and length safely in buffer, so set up for data transfer
;move it straight out of the pic's buffer
	movf	_CnDataLength,W
	bz	gcatc_add_flag
	movwf	_CnAddToCQCounter	;keep the length for the counter
	lfsr	FSR0,RXB0D0		;WINCON will select RXB1 if reqd (set by caller)
gcatc2:
;FSR1 points to correct insertion point, FSR0 points to id:length bytes
	movff	POSTINC0,POSTINC1		
;Buffer management
	movlw	CnCQRxBuffEndLbl		;address of end of buff.
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxBuff			;start pos of buffer	

	decfsz	_CnAddToCQCounter
	bra	gcatc2
gcatc_add_flag:
	movff	_CnRxFlags,POSTINC1
	movlw	CnCQRxBuffEndLbl		;address of end of buff. Keep in W for quicker access
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxBuff			;start pos of buffer	
;mssg+flag now in CQ
	;update the WCursor
	movff	FSR1L,CnCQRxWCursor
	;now adjust num in buffer
	movlw	CQ_CAN_MSSG_FIXED_MSSG_SIZE	;includes flag	
	addwf	_CnDataLength,W
	addwf	CnCQRxNumInBuff	
atcq_no_error_exit
	bsf	STATUS,Z			;no error
	return
atcqOvf
	bsf	CnErrorState1,CN_ERROR1_RX_CQ_OVF		;flag the error
	bsf	GlbErrState,GSI_ERR_CAN
	bcf	STATUS,Z			;send error
	return
	DW	0xffff

	return

;*********************************************************************
;		GSCANAddToR2CQ
;As above but store the message in a Q that will be used
;to send to rs232 port
;If ovf, overwrite previous message so that when we are
;back online we have the most recent messages.
;Note that this requires a critical section control
;as we mustn't overwrite a message that has been partially read
;Indeed the ReadQ should disable CAN ints while it is reading the mssg
;as there is no simple way to return from this function and re-enter
;after the non-interrupt context has finished.
;The CnCQRxBuff	Q should be safe as it will return if it is full, it will
;not overwrite but will report ovf.
;This Q needs to overwrite as it needs to keep current. 
;Can also quit early from this function if we are not attached to a pc, ie. not 
;in binary mode.

;Also:
;If this node is the master (cnThisNode=0) then it should always
;attempt to write to the com port, unless pc has specifically 
;disabled the function (not implemented)
;If this node is on a running CAN but is anything other than node0
;then it shouldn't send down the serial port unless specifically 
;instructed to do so, (not implemented)
;***********************************************************************
GSCANAddToR2CQ
	DW	0xffff 
	btfss	GlbSIOMode,SIO_M_CAN_RX_TO_SERIAL
	bra	atrcq_no_error_exit
#if	ENABLE_CAN_FILTER_0_TO_RS232 == FALSE
	movf	_CnRxFlags,W	;see which filter accepted this message
	andlw	CAN_RX_FILTER_BITS	;mask out non-filter bits		
	bz	atrcq_no_error_exit			;filter 0 message so ignore. Allows us to debug without SYNC id's always appearing
#endif

	btfss	GlbSIOMode,SIO_M_BINARY			;not binary, then can't tx so ignore
	bra atrcq_no_error_exit								;Z exit, no error
#IF 0
;Ignore ovf. Overwrite the message
	movf	_CnDataLength,W
	addwf	CnCQRxR2NumInBuff,W		;add to num bytes currently in buffer
	sublw	(CQ_CAN_RX_R2_BUFFER_SIZE-CQ_CAN_MSSG_FIXED_MSSG_SIZE)
	bnc	atrcqOvf

;ok mssg will fit in buffer.
#ENDIF

;indirectly address the id
        lfsr	FSR0, _CnvReg1_O
;now add the rest of the id. id and length bytes are contiguous in memory so loop 
;counter can include it.
	movlw	CAN_MSSG_MAX_SIZEOF_ID+CAN_MSSG_SIZEOF_LENGTH 
	movwf	_CnAddToCQCounter		;counter
;Set up buffer insertion point
	movf	CnCQRxR2WCursor,W			;Ptr to buff insertion point
	movwf	FSR1L,A				;Indirect access insertion point
	movlw	HIGH CnCQRxR2Buff	
    movwf   FSR1H
	;
gcatrc1:
;FSR1 points to correct insertion point, FSR0 points to id:length bytes
	movff	POSTINC0,POSTINC1		
;Buffer management
	movlw	CnCQRxR2BuffEndLbl		;address of end of buff. Keep in W for quicker access
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxR2Buff			;start pos of buffer	
	decfsz	_CnAddToCQCounter
	bra	gcatrc1
;id and length safely in buffer, so set up for data transfer
;move it straight out of the pic's buffer
	movf	_CnDataLength,W
	bz	gcatrc_add_flag
	movwf	_CnAddToCQCounter	;keep the length for the counter
	lfsr	FSR0,RXB0D0		;WINCON will select RXB1 if reqd (set by caller)
gcatrc2:
;FSR1 points to correct insertion point, FSR0 points to id:length bytes
	movff	POSTINC0,POSTINC1		
;Buffer management
	movlw	CnCQRxR2BuffEndLbl		;address of end of buff.
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxR2Buff			;start pos of buffer	

	decfsz	_CnAddToCQCounter
	bra	gcatrc2
gcatrc_add_flag:
	movff	_CnRxFlags,POSTINC1
	movlw	CnCQRxR2BuffEndLbl		;address of end of buff. Keep in W for quicker access
	subwf	FSR1L,W
	btfsc	STATUS,C
	lfsr	FSR1,CnCQRxR2Buff			;start pos of buffer	
;mssg+flag now in CQ
	;update the WCursor
	movff	FSR1L,CnCQRxR2WCursor
	;now adjust num in buffer
	movlw	CQ_CAN_MSSG_FIXED_MSSG_SIZE	;includes flag	
	addwf	_CnDataLength,W
	addwf	CnCQRxR2NumInBuff	
	bsf		GlbSIOStatus1,SIO_S1_CAN_DATA_PENDING_TX ;set for individ
atrcq_no_error_exit
	bsf	STATUS,Z			;no error
	return
atrcqOvf
	bcf	STATUS,Z			;send error
	return
	DW	0xffff
dsfd


	#endif ;GSI_CAN_SUPPORT



        END



