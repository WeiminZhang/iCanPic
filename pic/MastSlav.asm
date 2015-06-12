;******************************************************************************
;* @file    MastSlave.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;MastSlav.asm
#define IN_MASTSLAV.ASM

;#include "p18xxx8.inc"
#include "GSI.inc"

;#IF DEVICE == _18F458
;#include "p18xxx8.inc"
;#ELSE
;#INCLUDE "P18F4680.inc"
;#ENDIF


#include "rs232.inc"
#include "error.inc"
#include "can.inc"
#include "i2c.inc"		;only for debugging

	#IF GSI_CAN_SUPPORT
#include "CANPrTx.inc"

	GLOBAL CANSetPCMaster232,CANSetMaster232,CANSetSlaveId232,CANSetSlave232
			
;----------------from can.asm----------------------
	EXTERN	CopyR2RecordToCANTxBuff,CnThisNode
#if DISABLE_CN_SYNC_RX	;controls storage of filter0 messages (debugging)
	EXTERN CnFilter0Mask	
#endif	

;--------- variables all defined in CAN.asm
	EXTERN	CnErrorState,CnErrorState1,CnRxMssg,CnRxBuff,CnRxBuffLength
	EXTERN	CnRxFlags,CnTxRecord,CnTxMssgId,CnTxBuff,CnTxBuffLength
	EXTERN	CnTxFlags,CnMode,CnTemp,CnTemp32,CnSyncCounter
	EXTERN	CnSlaveBase
	EXTERN	GlbCANState
;------------from lcd.asm------------------------

;----------------from rs232.asm------------------
	IF DEBUG
		EXTERN DEBUGGSISendByte232
		EXTERN DEBUGGSISendBinary232
	ENDIF
	EXTERN R2TxRecordType,R2TxRecordNode,R2TxRecordLength,R2TxRecordData

;-------------------from init.asm------------------
	#IF EEPROM_RESTORE_STATE_ENABLED
		EXTERN GiEEPROMAddress,EpCANMasterId,EpCANSlaveId
	#ENDIF

MAST_SLAV	CODE
;------------------------------------------------------------------------------
;	GSICANPCMaster/232
;This node is attached to a pc and is the PCMaster
;Set filters to capture:
;lowest priority:
;f5:all messages (so we can keep the pc informed of bus activity)
;f4: CAN_BASE=0. 
;ID's 4064-4095 the GSI ids
;f1:32-47, GSI/User time critical (GSI 32-39, User 40-47)
;f0:0-31,GSI/User time critical (GSI 0-3, User 4-7, GSI 8-19, User 20-31)
;
;
;#Install filters
;#Send SYNC messag every CAN_SYNC_COUNTER clocks
;#Timer 3 as CAN_SYNC clock. 20MHz prescalar=8, count=62500 for 100ms
;------------------------------------------------------------------------------
CANSetPCMaster232		
	call	CopyR2RecordToCANTxBuff
GSICANSetPCMaster:
	banksel	CnBank
	bcf	CnMode,CAN_MODE_MASTER
	bcf	CnMode,CAN_MODE_SLAVE
	bsf	CnMode,CAN_MODE_PC_MASTER
	bra	CANMaster
CANSetMaster232		
	call	CopyR2RecordToCANTxBuff
GSICANSetMaster:
	banksel	CnBank
	bcf	CnMode,CAN_MODE_SLAVE
	bcf	CnMode,CAN_MODE_PC_MASTER
	bsf	CnMode,CAN_MODE_MASTER
	;clrf	CnThisNode		;0=master defer this till we have sent RS232_MSSG to pc
	bsf	GlbSIOMode,SIO_M_CAN_RX_TO_SERIAL	;send RX'd CAN messages to serial port Tx buffer
;mask0 value for f0/f1
;m0=11111111 11111111 11111111 11110000 	
;Accept 0-15 irrespective of filter value (GSI/User time critical)
;Filters
;f0   =11111111 11111111 11111111 11110000 (16-31 in filter 0)
;f1   =11111111 11111111 11111111 11100000 (CAN_BASE+32-47 in filter 1)
;----------------------------------------------------------------------------
;mask1 value for f2-f5:accept 0-31 (ignored as mask0 does the same)
;m1=11111111 11111111 11111111 11100000
;Accept 0-31
;f2=00000000 00000000 11111111 1000 0000 (accept CAN_BASE+3968-3999)   
;f3=00000000 00000000 11111111 1010 0000 (accept CAN_BASE+4000-4031)
;f4=00000000 00000000 11111111 1100 0000 (accept CAN_BASE+4032-4063)
;f5=00000000 00000000 11111111 1110 0000 (accept CAN_BASE+4064-4095)
CANMaster:
	;Set PC_MASTER
	ERRORLEVEL -311
        mCANPrTxSetOpMode     CAN_OP_MODE_CONFIG

	;------ Mask B0 ---------
	;mask b0 will accept id 0-15 in Rx0
	mCANPrTxSetReg CAN_MASK_B0, 0xfffffff0, CAN_CONFIG_XTD_MSG

	;------- Filters ---------------------------------------------
	;f0 will capture id 16-31 (so will hold 0-31)
	mCANPrTxSetReg CAN_FILTER_B0_F1,0x00000000, CAN_CONFIG_XTD_MSG

	;f1 will capture id 32-48
	mCANPrTxSetReg CAN_FILTER_B0_F2,0x00000010, CAN_CONFIG_XTD_MSG

	;----------------------------------------------------------------
	;mask B1 accept Filtern:0-31 in Rx1
	mCANPrTxSetReg CAN_MASK_B1,0xffffffe0, CAN_CONFIG_XTD_MSG

	;----------- Filters ------------------------
	;CAN_BASE==0 by default, correct for a master
	;b1:f1=CAN_BASE+3968 to 3999
	mCANPrTxSetReg CAN_FILTER_B1_F1,CAN_BASE | 0x00000f80, CAN_CONFIG_XTD_MSG

	;b1:f2=CAN_BASE+4000-4031
	mCANPrTxSetReg CAN_FILTER_B1_F2,CAN_BASE | 0x00000fa0, CAN_CONFIG_XTD_MSG

	;b1:f3=CAN_BASE+4032-4063
	mCANPrTxSetReg CAN_FILTER_B1_F3,CAN_BASE | 0x00000fc0, CAN_CONFIG_XTD_MSG

	;b1:f4=CAN_BASE+4064-4095
	mCANPrTxSetReg CAN_FILTER_B1_F4,CAN_BASE | 0x00000fe0, CAN_CONFIG_XTD_MSG

#if CAN_LOOPBACK
	mCANPrTxSetOpMode     CAN_OP_MODE_LOOP
#else	
        mCANPrTxSetOpMode     CAN_OP_MODE_NORMAL
#endif
	ERRORLEVEL +311
;------------Start timer3 as a 100msec timer-------------------
	SET16	TMR3L, COUNT_100_MSEC
#if CLOCK == 10
	movlw	b'11010001'	;as below but 1:4 ps
#else
	movlw	b'11011001'	;16 bit rw,t3=ccp,1:8ps,Fosc/4,enable
#endif
	movwf	T3CON
	bcf	PIR2,TMR3IF	;clr any pending int	
	bsf	PIE2,TMR3IE	;enable int
	bsf	IPR2,TMR3IP	;set TMR3 as high priority
	
;----send rs232 msg to let pc know we have changed node
	banksel	R2Bank
;this node is the master and has id of node 0	
;set up an rs232 record

	messg Need a Tx CQ, this might be overwritten!

	movlw	RT_GSI_RS232_MSSG
	movwf	R2TxRecordType		
	movff	CnThisNode,R2TxRecordNode
	movlw	3
	movwf	R2TxRecordLength

	movlw	GSI_RS232_MESSAGE_CHANGE_NODE
	movwf	R2TxRecordData				;subtype

	movff	CnThisNode,R2TxRecordData+1		;change from CnThisNode

	clrf	WREG
	movwf	R2TxRecordData+2			;to node 0 (we are master	
	movff	WREG,CnThisNode

	bsf		GlbSIOStatus2,SIO_S2_RS232_DATA_PENDING_TX	;main prog loop should send it...

gcpm_exit:
	#IF EEPROM_RESTORE_STATE_ENABLED
	banksel	CnBank
	movlw	EpCANMasterId		;adddress of variable
	movff	WREG,GiEEPROMAddress	;store for WriteEEPROM
	call	GSIReadEEPROM	;see if we are the same value	
	subwf	CnThisNode
	bz	_gcpm_exit_1	;already have this value set in eeprom
	movf	CnThisNode,W	;
	call	GSIWriteEEPROM	;store  node value in EEPROM
	#ENDIF
_gcpm_exit_1:
	banksel	GiBank	;return to main bank

	return


;----------------------------------------------------------------
;			CANSetSlaveId/232
;Can be called from r232 or CAN code
;if from rs232 then we must send the data in R2Record back to slave
;according to R2RecordNode as a CAN mssg.
;R2Record will hold Flag;SLAVE_BASE
;If from a CAN Messg, then we must send the message up to the PC
;(if one is present). If not present then must send back a SLAVE_BASE
;This should only be called if we are a master.
;The Slave has found a SYNC and has sent a CAN_ID_GET_SET_SLAVE_ID
;with d0:b0=0
;We respond with a CAN_ID_GET_SET_SLAVE_ID with:
;d0:b0=1 (CAN_ID_SET)
;d0:b0=n (CAN_PC_PRESENT)
;If rs232 must send 4 byte SLAVE_BASE (d1-5) node address and
;8 bit CnThisNode (unshifted base address) in d6
;---------------------------------------------------------------
CANSetSlaveId232	;arrives here if called from rs232
	call	CopyR2RecordToCANTxBuff
GSICANSetSlaveId

	banksel CnBank
;----- Fill the TxBuff flag with correct info for the slave -----
	clrf	CnTxBuff		;clr flag (d0 is a flag)
	bsf	CnTxBuff,CAN_ID_SET	;we are Master, sending to slave
	bsf	CnTxBuff,CAN_PC_PRESENT	;assume PC is present if not change flag later...

	btfsc	CnMode,CAN_MODE_PC_MASTER
	bra	gcssir1
;------ Stand alone master, so must generate our own SLAVE_BASE ---
	bcf	CnTxBuff,CAN_PC_PRESENT	;No PC present if we are here
	incf	CnSlaveBase,F		;max 256 
	movf	CnSlaveBase,W
	movwf	CnTxBuff+5		;5th byte is CnThisNode as an 8 bit int
	btfsc	STATUS,C
	incf	CnSlaveBase+1		;ripple C, (this is redundant!)
	
	;-
	;CnTxBuff+0 is flag
	clrf	CnTxBuff+1		;do a rol,8
	movff	CnSlaveBase,CnTxBuff+2	;by starting at second byte
	movff	CnSlaveBase+1,CnTxBuff+3;
	clrf	CnTxBuff+4		;clrf for following rlcf
	;must do a 24 bit rol,4	
	movlw	4
gcssir2
	rlcf	CnTxBuff+2
	rlcf	CnTxBuff+3
	rlcf	CnTxBuff+4
	decfsz	WREG
	bra	gcssir2
;------CnTxBuff+1 - +4 is now rotated left 12 bits (x 4096) ---------------	

gcssir1:
;----- Ready to send the message ----
        mCANPrTxSendMsg  CAN_ID_GET_SET_SLAVE_ID,CnTxBuff,6,CAN_TX_XTD_FRAME
        addlw   0x00            ;Check for return value of 0 in W
        bz      gcssir1         ;Buffer Full, Try again
	banksel	CnBank		;restore our bank

	bsf	CnMode,CAN_MODE_RUNNING	;at least one slave node is online

;ok, we have just registerd a new node. Should now tell the pc that
;we have one!

;----send rs232 msg to let pc know we have registered a new node
	banksel	R2Bank

	messg Need a Tx CQ, this might be overwritten!

	movlw	RT_GSI_RS232_MSSG
	movwf	R2TxRecordType		
	movff	CnThisNode,R2TxRecordNode
	movlw	2
	movwf	R2TxRecordLength

	movlw	GSI_RS232_MESSAGE_NEW_NODE
	movwf	R2TxRecordData				;subtype

	movff	CnSlaveBase,(R2TxRecordData+1)		;change from CnThisNode


	bsf		GlbSIOStatus2,SIO_S2_RS232_DATA_PENDING_TX	;main prog loop should send it...
	banksel CnBank
	return


;-----------------------------------------------------------------	
;	CANSlave/232 
;Set this node as a SLAVE
;Can be called from rs232(if slave has an rs232) or CAN code
;This function will request a SLAVE id from the master:
;# wait till a CAN_ID_SYNC is seen 
;# send a CAN_ID_GET_SET_SLAVE_ID with d0:CAN_ID_SET=0 (bcf)
;# wait for a CAN_ID_GET_SET_SLAVE_ID with d0:CAN_ID_SET=1 (bsf)
;# D1-D4 will be the slave id
;Probably need to check the eeram to see if we already have a 
;slave id assigned before obtaining a new one. Could send any
;found id with the GET request. 
;-----------------------------------------------------------------
CANSetSlave232
GSICANSetSlave

;Send a message to the LCD
#IF GSI_LCD_SUPPORT
	call	GSIHome			
	movlw	CAN_MSSG_SET_SLAVE_ID_STR
	call	GSIWriteStr
	call	GSIHome
#ENDIF
	banksel	CnBank
;following bit is set during special reset. Set again as this code
;can be reached through an rs232 message (rs232 attached to slave)
;tell prog we are looking for an id
	bsf	GlbCANState,CAN_SET_SLAVE_ID

	ERRORLEVEL -311
        mCANPrTxSetOpMode     CAN_OP_MODE_CONFIG

;---------------- In config mode --------------------------

;---------------- Mask B0 ------------------------
;0xffffffff, check all bits in filter against id
;-------------------------------------------------
	mCANPrTxSetReg CAN_MASK_B0,0xffffffff, CAN_CONFIG_XTD_MSG

;---------------- Filters ---------------------------------------------
;f0/f1 will capture only id CAN_ID_SYNC and CAN_ID_GET_SET_SLAVE_ID 
;----------------------------------------------------------------------
	mCANPrTxSetReg CAN_FILTER_B0_F1,CAN_ID_SYNC, CAN_CONFIG_XTD_MSG
	mCANPrTxSetReg CAN_FILTER_B0_F2,CAN_ID_GET_SET_SLAVE_ID, CAN_CONFIG_XTD_MSG

	;now must wait for the SYNC message

;------------------ In Normal /Loopback mode----------------------
#if CAN_LOOPBACK
	mCANPrTxSetOpMode     CAN_OP_MODE_LOOP
#else	
        mCANPrTxSetOpMode     CAN_OP_MODE_NORMAL
#endif
;------------ Look for the SYNC --------------

gcsgsir1:
	clrwdt
	banksel	CnBank
	lfsr	FSR0,CnRxMssg
	call	GSICANGetMsgFromCQ	
	bz	gcsgsir1

	;check to make sure it is mssg rec'd from F1

	movf	CnRxFlags,W	
	andlw	CAN_RX_FILTER_BITS	;mask out non filter bits
	bnz	gcsgsir1		;0 if F1 (F0 in MChip define)
#if DISABLE_CN_SYNC_RX
	bsf	CnFilter0Mask,CN_FILTER0_NO_SYNC	;don't place SYNC messages in buffer (actually, currently *all* filter 0 messages)
#endif
;------------- SYNC received -----------------------------
	bcf	PIR3,RXB0IF	;remove the lock
	bsf	PIE3,RXB0IE	;re-enable int				
;-----------------------------------------
	;-Send a CAN_ID_GET_SET_SLAVE_ID
sendGetSlaveIdMssg
	banksel	CnBank
;set d[0]=0 the code for CAN_ID_SET
	clrf	CnTxBuff	;bcf CnTxBuff, CAN_ID_SET	(already, as data==0)
        mCANPrTxSendMsg  CAN_ID_GET_SET_SLAVE_ID,CnTxBuff,1,CAN_TX_XTD_FRAME
        addlw   0x00            ;Check for return value of 0 in W
        bz      sendGetSlaveIdMssg         ;Buffer Full, Try again
	banksel	CnBank		;restore our bank

;-------------------------------------------------------------
;CAN_ID_GET_SET_SLAVE msg has been sent, now wait for reply
;-------------------------------------------------------------
gcsgsir2
	clrwdt
;-------------------------------------------
;msg received, check it is the correct one
;-------------------------------------------
	lfsr	FSR0,CnRxMssg
	call	GSICANGetMsgFromCQ	
	bz	gcsgsir2
	IF DEBUG
gcgs_error
	nop
	bc	gcgs_error
	ENDIF

	movf	CnRxFlags,W	
	andlw	CAN_RX_FILTER_BITS	;mask out non filter bits
	decfsz	WREG			;F2=1 so decf==0 if correct	
	bra	gcsgsir2		;0 if F1 (F0 in MChip define)

;----------- CAN_ID_GET_SET_SLAVE received -------------
	;D0 should have bit 0 set (CAN_ID_SET)
	;D1-5 = SLAVE_BASE
	;d6= SLAVE_BASE unshifted (1-250)
	mCANPrTxSetOpMode     CAN_OP_MODE_CONFIG


;-------------- In config mode ----------------------------
	
	;------ Mask B0 ---------
	;mask b0 will accept id 0-15 in Rx0
	;mCANPrTxSetReg CAN_MASK_B0, 0xfffffff0, CAN_CONFIG_XTD_MSG
	mCANPrTxSetReg CAN_MASK_B0, 0xfffffff0, CAN_CONFIG_XTD_MSG

	;mCANPrTxSetReg CAN_MASK_B0, 0x00, CAN_CONFIG_XTD_MSG

	;------- Filters ---------------------------------------------
	;mask B0
	;M0:f0 will capture id's 0-15
	mCANPrTxSetReg CAN_FILTER_B0_F1,0x0000000, CAN_CONFIG_XTD_MSG

	banksel	CnBank
	;M0:f1 will capture id SLAVE_BASE+0-15
	movlw	0x10
	movwf	CnRxBuff+1	
	mCANPrTxSetReg_IV CAN_FILTER_B0_F2,CnRxBuff+1, CAN_CONFIG_XTD_MSG

;----------------------------------------------------------------
	mCANPrTxSetReg CAN_MASK_B1,0xffffffe0, CAN_CONFIG_XTD_MSG

;----------- Filters ------------------------
	;CAN_BASE==0 by default, correct for a master
	;M1:f2=CAN_BASE+3968-3999 
	banksel	CnBank
	movlw	0x80
	movwf	CnRxBuff+1
	movlw	0x0f
	iorwf	CnRxBuff+2
	;mCANPrTxSetReg CAN_FILTER_B1_F1,CAN_BASE | 0x00000f80, CAN_CONFIG_XTD_MSG
	mCANPrTxSetReg_IV CAN_FILTER_B1_F1,CnRxBuff+1, CAN_CONFIG_XTD_MSG

	banksel	CnBank
	movlw	0xa0
	movwf	CnRxBuff+1
	movlw	0x0f
	iorwf	CnRxBuff+2
	;-byte 2 already correct value (0x0f)
	;M1:f3=CAN_BASE+4000-4031
	;mCANPrTxSetReg CAN_FILTER_B1_F2,CAN_BASE | 0x00000fa0, CAN_CONFIG_XTD_MSG
	mCANPrTxSetReg_IV CAN_FILTER_B1_F2,CnRxBuff+1, CAN_CONFIG_XTD_MSG

	banksel	CnBank
	movlw	0xc0
	movwf	CnRxBuff+1
	movlw	0x0f
	iorwf	CnRxBuff+2
	;M1:f4=CAN_BASE+4032-4063
	;mCANPrTxSetReg CAN_FILTER_B1_F3,CAN_BASE | 0x00000fc0, CAN_CONFIG_XTD_MSG
	mCANPrTxSetReg_IV CAN_FILTER_B1_F3,CnRxBuff+1, CAN_CONFIG_XTD_MSG

	banksel	CnBank
	movlw	0xe0
	movwf	CnRxBuff+1
	movlw	0x0f
	iorwf	CnRxBuff+2
	;M1:f5=CAN_BASE+4064-4095
	;mCANPrTxSetReg CAN_FILTER_B1_F4,CAN_BASE | 0x00000fe0, CAN_CONFIG_XTD_MSG
	mCANPrTxSetReg_IV CAN_FILTER_B1_F4,CnRxBuff+1, CAN_CONFIG_XTD_MSG

	banksel	CnBank
#if CAN_LOOPBACK
	;SetOpMode doesn't affect BSR
	mCANPrTxSetOpMode     CAN_OP_MODE_LOOP
#else	
        mCANPrTxSetOpMode     CAN_OP_MODE_NORMAL
#endif
	ERRORLEVEL +311
	movf	CnRxBuff+5,W		;get unshifted node id
	movwf	CnThisNode

	bcf	GlbCANState,CAN_SET_SLAVE_ID	
	bsf	CnMode,CAN_MODE_SLAVE	
	bsf	CnMode,CAN_MODE_RUNNING	;at least master and this node are online
	bcf	GlbSIOMode,SIO_M_CAN_RX_TO_SERIAL	;Don't send Rx'd CAN msg to rs232 buff

	;re-enable any ints that might have been used
	;and remove lock
	bcf	PIR3,RXB0IF	;remove the lock
	bsf	PIE3,RXB0IE	;re-enable int				
	bcf	PIR3,RXB1IF	;remove the lock
	bsf	PIE3,RXB1IE	;re-enable int				
	#IF EEPROM_RESTORE_STATE_ENABLED
	movlw	EpCANSlaveId		;adddress of variable
	movff	WREG,GiEEPROMAddress	;store for WriteEEPROM
	call	GSIReadEEPROM	;see if we are the same value	
	subwf	CnThisNode
	bz	_gcsgsir_exit	;already have this value set in eeprom
	movf	CnThisNode,W	;
	call	GSIWriteEEPROM	;store  node value in EEPROM
	#ENDIF
_gcsgsir_exit:

#IF GSI_LCD_SUPPORT
	banksel	LdBank
	call	GSIHome			
	call	GSIClrLine
	movlw	CAN_NODE_THIS_STR
	call	GSIWriteStr
	movff	CnThisNode,WREG
	call	GSILCDWriteAscii
	call	GSIHome
#ENDIF	
	banksel	CnBank
	return
;-----------------------------------------------------------
; END CANSlave/232
;-----------------------------------------------------------

	#endif ;GSI_CAN_SUPPORT

	END
