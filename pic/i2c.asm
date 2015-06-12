;******************************************************************************
;* @file    i2c.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;i2c.asm
#define IN_I2C.ASM

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
#include "can.inc"

	#IF GSI_CAN_SUPPORT
#include "CANPrTx.inc"
	#ENDIF

#define I2C_IDLE_CONDITION_INDEX 	(i2c_ic - i2c_sc)/4

		GLOBAL InI2CInt,TestI2C
		GLOBAL I2CStateIndex
;-----------------------------------------
;	from can.asm
;-----------------------------------------
	#IF GSI_CAN_SUPPORT
	EXTERN 	CnThisNode,CnTxBuff,CnMode
	#ENDIF
;----------------------------------------
;	Access ram data
;----------------------------------------
I2C_GLOBAL_DATA		UDATA_ACS

GlbI2CTxByte		RES 1
GlbI2CRxByte		RES 1		;have an Rx/Tx so we can process while sending
GlbI2CStatus		RES 1

;-----------------------------------------
;	Private data currently in GSI_BANK
;----------------------------------------
I2C_DATA	UDATA
I2CBank			RES 0
I2CStateIndex		RES 1		;index to next I2C code to execute
I2CTxBuff		RES I2C_TX_BUFF_SIZE
I2CTxNumInBuff		RES 1
I2CTxRCursor		RES 1
I2CTxWCursor		RES 1
I2CTxBuffEnd		RES 1

I2CTemp			RES 1
I2CReturnData		RES 1		;might need to increase as
					;devices will send more data than this
;---------------------------------------------------------------------
;	  InI2CInt 
;called via interrupt handler
;Call functions through an index
;every interrupt advance the index till the I2C cycle is complete
;Also called once after loading the buffer with req'd data from
;user code to initiate the i2c transfer. After first call, ints
;will be generated to complete the transfer
;----------------------------------------------------------------------
INTERRUPT_CODE	CODE	

GSIInitiateI2CTransfer
	banksel	I2CTxBuff
	clrf	I2CStateIndex		;ensure state 0
InI2CInt
	movlw	HIGH  I2CJump	        ;fetch upper byte of jump table address
	movwf	PCLATH			;load into upper PC latch
	movlw	I2C_SIZE_MASK		;0xf so can't be more than 15 jumps
	andwf	I2CStateIndex,W         ;retrieve current I2C state
	;WREG=0 to 15
	addwf	WREG,W			;2 byte 18f instructions, byte PC
	addwf	WREG,W			;2 word instruction, so *4 index
	;WREG holds byte offset to correct goto
	;Now add offset to the jump table
	addlw	LOW  (I2CJump + 2)      ;calc state machine jump addr into W		
	btfsc	STATUS,C                ;skip if carry occured
	incf	PCLATH,F		;otherwise add carry
I2CJump	; address were jump table branch occurs, this addr also used in fill 
	movwf	PCL  		        ;index into state machine jump table
	;jump to processing for each state= I2CStateIndex value for each state
i2c_sc	;i2c_sc is used to calculate the number of gotos
	goto	I2CStartCondition        ;start sequence
	goto	I2CSendSlaveAddress      ;send address, R/W=1
	goto	I2CSendByte		 ;select dac,pd,reset
	goto	I2CSendByte		 ;8 bit data for 1 dac	
    	goto 	I2CStopConditionAck      ;stop condition+check ACK from previous
	;set I2CStateIndex to point to this after an error so that
	;final interrupt (after the STOP is complete) comes here
i2c_ic	;i2c_ic is used to determine the index of I2CIdleCondition
	goto	I2CIdleCondition

I2CJumpEnd		 
	;ensure any out of bounds jmp returns gracefully
        Fill (return),  ((I2CJump-I2CJumpEnd)/2) + I2C_SIZE_MASK 

;----------------------------------------------------------------------
;   ********************* Write data to Slave   *********************
;
;Only have an I2C DAC which is write only
;----------------------------------------------------------------------
; Generate I2C bus start condition    [ I2C STATE -> 0 ]
I2CStartCondition
	clrf	GlbErrState
	clrf	WREG
	cpfsgt	I2CTxNumInBuff
	goto	NoMoreData
	bsf	GlbI2CStatus,I2C_START_CONDITION	
	incf	I2CStateIndex,F          ;update I2C state variable
	bsf     SSPCON2,SEN              ;initiate I2C bus start condition
	return                            
NoMoreData
	bsf	GlbErrState,GSI_ERR_I2C_NO_DATA
	return

;I2C address write (R/W=0)           
I2CSendSlaveAddress
	
	incf	I2CStateIndex,F          ;update I2C state variable
	call	GSIGetFromI2CTxBuff	 ;returns first byte (an address) from buff
	movwf   SSPBUF                   ;initiate I2C bus write condition
	return                           

;I2C write data condition
I2CSendByte
	call	I2CAckTest		;on failure resets vrbls, sets STOP
	bc	sb_exit			;remember that this is a goto
	call	GSIGetFromI2CTxBuff	;returns next byte in WREG, the command byte
	bz	sb_no_data_error
	incf	I2CStateIndex,F         ;update I2C state variable
	movwf   SSPBUF                  ;initiate I2C bus write condition
	bsf	GlbI2CStatus,I2C_WRITE_BYTE_CONDITION
sb_exit
	return                          
sb_no_data_error
	bsf	GlbErrState,GSI_ERR_I2C_NO_DATA
	goto	I2CStopCondition

;-------------------------------------------------------------------------
;	I2C bus stop condition
;IMPORTANT:
;STATUC,C will be set on an ACK_TEST failure
;This function is used as an exit from I2CAckTest on failure
;and must not change C
;-----------------------------------------------------------------------
I2CStopConditionAck
	call	I2CAckTest
I2CStopCondition
	bsf     SSPCON2,PEN              	;initiate I2C bus stop condition
	bsf	GlbI2CStatus,I2C_STOP_CONDITION
	;force last int to be IDLE as we can be here through an error
	movlw	I2C_IDLE_CONDITION_INDEX	;one more int to come, signalling completion of STOP
	movwf	I2CStateIndex		
	;STATUS,C unchanged from entry at I2CStopCondition
	;also broadcast it down the can
_icsc1
	#IF GSI_CAN_SUPPORT
	movff	CnMode,I2CTemp		;get the current mode
	btfss	I2CTemp,CAN_MODE_RUNNING
	bra	_icsc_exit
	;-ok CAN is running so tx the mssg
	movff	CnThisNode,CnTxBuff
	movlw	CAN_I2C_MSSG_STOP_CONDTION
	movff	WREG,CnTxBuff+1
	movff	I2CReturnData,CnTxBuff+2	
;Note that this mssg will be sent to the MASTER
	mCANPrTxSendMsg CAN_ID_I2C_MSSG,CnTxBuff,3,CAN_TX_XTD_FRAME
	;next line sends to node 1 for debugging
	;mCANPrTxSendMsg CAN_ID_I2C_MSSG+4096,CnTxBuff,3,CAN_TX_XTD_FRAME

        addlw   0x00            ;Check for return value of 0 in W
        bz      _icsc1           ;Buffer Full, Try again
	#ENDIF ;GSI_CAN_SUPPORT

_icsc_exit:
	return               

I2CIdleCondition
	clrf	GlbI2CStatus		;idle
	clrf	I2CStateIndex
	return
	

; Generate I2C stop condition   [ I2C STATE -> 8 ]
ReadStop
	bcf	PIE1,SSPIE               	;disable SSP interrupt
	bsf     SSPCON2,PEN              	;initiate I2C bus stop condition
	clrf	I2CStateIndex                 	;reset I2C state variable
	;bsf	GlbI2CStatus,I2C_RW_DONE	;set read/write done flag
	movlw	'8'
	call	GSISendByte232
	return 

;-------------------------------------------------------------------------
; Test acknowledge after address and data write
;Returns NC
;C on error
;NOTE: we could use the slave address that failed as part of the error
;message, telling us exactly which device failed.
;-------------------------------------------------------------------------
I2CAckTest
	bcf	STATUS,C		;no error
	btfss   SSPCON2,ACKSTAT         ;test for acknowledge from slave (the address byte)
	return
I2CAckError
	bsf	GlbErrState,GSI_ERR_I2C_NO_ACK_FROM_SLAVE  ;set acknowledge error
	clrf	I2CTxNumInBuff		;flush the buffer
	bsf	STATUS,C	
	goto	I2CStopCondition

;---------------------------------------------------------------
;	GSIGetFromI2CTxBuffer
;Assumes MAIN_BANK
;Shouldn't really be part of the exposed interface
;Should only be used byt I2C code
;returns Z if empty
;--------------------------------------------------------------
GSIGetFromI2CTxBuff

	movf	I2CTxNumInBuff,W
	btfsc	STATUS,Z,A
	return					;buffer empty

	movf	I2CTxRCursor,W
	movwf	FSR2L,A				;Indirect access tail
	;Buffer management
	incf	I2CTxRCursor,F			;increment write cursor
	movf	I2CTxRCursor,W			;Load cursor to W
	subwf	I2CTxBuffEnd,W			;compare to buff end
	movf	I2CTxRCursor,W			;Cursor pos for no ovf
	btfss	STATUS,C,A			;nc = ovf
	movlw	I2CTxBuff			;ovf, get start pos of buffer
	;W holds next valid Insertion point
	movwf	I2CTxRCursor			;update cursor
	decf	I2CTxNumInBuff,F		;keep count
	btfsc	STATUS,Z,A
	bcf	GlbI2CStatus,I2C_CHAR_AVAILABLE	;signal buff empty

	movf	INDF2,W,A		;Get from buffer
	bcf	STATUS,Z,A
gfrb_exit	;Z=empty
	return

;--------------------------------------------------------------------
;	End of intterupt context code
;---------------------------------------------------------------------



I2C	CODE
;----------------------------------------------------------------------
;   ******************* Generic bus idle check ***********************
;----------------------------------------------------------------------
; test for i2c bus idle state; not implemented in this code (example only)
I2CIdle   
	btfsc   SSPSTAT,R_W              ;test if transmit is progress 
	goto    I2CIdle                  ;module busy so wait
i2c2
	movf    SSPCON2,W                ;get copy of SSPCON2 for status bits
	andlw   0x1F                     ;mask out non-status bits
	btfss   STATUS,Z                 ;test for zero state, if Z set, bus is idle
	goto    i2c2                     ;bus is busy so test again
	return                           ;return to calling routine



;--------------------------------------------
;	GSIAddToI2CTxBuff
;circular buffer for I2C Rx characters
;Assumes MAIN_BANK
;Byte to add is in WREG
;--------------------------------------------
GSIAddToI2CTxBuff
;SlRxByte holds incoming data
;can be called at int time
	
	banksel	I2CTxWCursor
	movwf	GlbI2CTxByte
	bsf	GlbI2CStatus,I2C_CHAR_AVAILABLE
	movlw	I2C_TX_BUFF_SIZE			;check for buffer ovf
	cpfslt	I2CTxNumInBuff
	;subwf	I2CTxNumInBuff,W		
	;btfsc	STATUS,Z,A
	goto	atitb_ovf
	bcf	GlbI2CStatus,I2C_TX_BUFF_OVF	;reset flag if it is set

	movf	I2CTxWCursor,W			;Ptr to buff insertion point
	movwf	FSR2L,A				;Indirect access insertion point
	movf	GlbI2CTxByte,W			;Get the byte to queue
	movwf	INDF2,A				;put it in the buffer
	;Buffer management
	incf	I2CTxWCursor,F			;increment write cursor
	movf	I2CTxWCursor,W			;Load cursor to W
	subwf	I2CTxBuffEnd,W			;compare to buff end
	movf	I2CTxWCursor,W			;Cursor pos for no ovf
	btfss	STATUS,C,A			;nc = ovf
	movlw	I2CTxBuff			;start pos of buffer
	;W holds next valid Insertion point
	movwf	I2CTxWCursor			;update cursor
	incf	I2CTxNumInBuff,F		;keep count
atitb_ovf
	return
;----------------------------------------------------------------------
;  ******************* INITIALIZE MSSP MODULE  *******************
;----------------------------------------------------------------------

GSIInitI2C
	movlw   I2C_BAUD_RATE_VALUE    ;400kHz (0x45 for 100k, 0xb0 for 400k)
	movwf   SSPADD                 ;initialize I2C baud rate
	bcf     SSPSTAT,CKE             ;disable,SMBus options, select I2C input levels
	bcf     SSPSTAT,SMP             ;enable slew rate control

	movlw   b'00111000'            ;I2C Master mode, SSP enable
	movwf   SSPCON1                

	bsf	PIE1,SSPIE		;enable ssp interrupts
	bsf	PIE2,BCLIE		;enable bit collision ssp interrupt
	;allow to fall through to init vrbls
;-----------------------------
;	InitI2CVrbls
;called from init.asm
;-----------------------------
GSIInitI2CVrbls
	clrf    GlbI2CStatus         
	clrf	I2CStateIndex        

	movlw	I2CTxBuff
	movwf	I2CTxWCursor
	movwf	I2CTxRCursor
	addlw	I2C_TX_BUFF_SIZE-1
	movwf	I2CTxBuffEnd

	clrf	I2CTxNumInBuff

	return                         

#if 0
TestI2CSW
;Not that I2C 7 bit addresses are composed of bit 7-1 the address
;and bit 0 the r/w flag (0=transmit, 1=receive [as seen from master])
;Thus we take a 7 bit address and rl to get the address and set/clr
;bit 0 for r/w
;for MAX518
;1 write address of chip to TxBuff, 0x2c for this hardware
;2 write MAX518_CMD_SLCT_DAC0 to TxBuff
;3 write address of chip to TxBuff, 0x2c for this hardware
;4 write DAC value 0-255
;5 Set flag to initiate the i2c transfers

;I2c handler will:
;# send address
;# get ack
;# send MAX518_CMD
;# get ack
;# send stop

;# send address
;# get ack
;# send DAC value
;# get ack
;# send stop

;1 write address of chip to TxBuff, 0x2c for this hardware
;2 write MAX518_CMD_SLCT_DAC0 to TxBuff
;3 write address of chip to TxBuff, 0x2c for this hardware
;4 write DAC value 0-255
;5 Set flag to initiate the i2c transfers

	;movlw	MAX518_BASE_ADDR
	;call	GSIAddToI2CTxBuff
	;movlw	MAX518_CMD_SLCT_DAC0
	;call	GSIAddToI2CTxBuff
	
	bcf	PIE1,SSPIE		;disable ssp interrupts
	bcf	PIE2,BCLIE		;disable bit collision ssp interrupt
	;---------------------------	
	;start state generation
	bsf	SSPCON2,SEN
	movlw	'S'
start_loop
	clrwdt	
	call	GSISendByte232
	btfsc	SSPCON2,SEN		;start state complete?
	goto	start_loop
	;---------------------------
	;address generation	
	movlw	'A'
	call	GSISendByte232

	movlw	MAX518_BASE_ADDR+1
	movwf	SSPBUF
	;-look for BF flag
	movlw	'b'
bf_loop
	clrwdt
	call	GSISendByte232
	btfsc	SSPSTAT,BF
	goto	bf_loop		
	;-look for ACK
	movlw	'k'
	btfsc	SSPCON2,ACKSTAT
	goto	tic_error_exit
	call	GSISendByte232		;ACK received
	;----------------------
	;now send command byte
	movlw	MAX518_CMD_SLCT_DAC0	
	movwf	SSPBUF
	movlw	'b'
bf1_loop
	clrwdt
	call	GSISendByte232
	btfsc	SSPSTAT,BF
	goto	bf1_loop		
	;-look for ACK
	movlw	'k'
	btfsc	SSPCON2,ACKSTAT
	goto	tic_error_exit
	call	GSISendByte232		;ACK received

	;now send data byte
	movlw	0x80
	movwf	SSPBUF
	movlw	'b'
bf2_loop
	clrwdt
	call	GSISendByte232
	btfsc	SSPSTAT,BF
	goto	bf2_loop		
	;-look for ACK
	movlw	'k'
	btfsc	SSPCON2,ACKSTAT
	goto	tic_error_exit
	call	GSISendByte232		;ACK received
	;-----------------------
	;2nd command byte for dac1

	movlw	MAX518_CMD_SLCT_DAC1	
	movwf	SSPBUF
	movlw	'b'
bf1a_loop
	clrwdt
	call	GSISendByte232
	btfsc	SSPSTAT,BF
	goto	bf1a_loop		
	;-look for ACK
	movlw	'k'
	btfsc	SSPCON2,ACKSTAT
	goto	tic_error_exit
	call	GSISendByte232		;ACK received

	;now send data byte
	movlw	0x40
	movwf	SSPBUF
	movlw	'b'
bf2a_loop
	clrwdt
	call	GSISendByte232
	btfsc	SSPSTAT,BF
	goto	bf2a_loop		
	;-look for ACK
	movlw	'k'
	btfsc	SSPCON2,ACKSTAT
	goto	tic_error_exit
	call	GSISendByte232		;ACK received
	;-

	;Send STOP
	bsf	SSPCON2,PEN		;STOP
	movlw	'P'
stop_loop
	clrwdt	
	call	GSISendByte232
	btfsc	SSPCON2,PEN		;stop state complete?
	goto	stop_loop

	movlw	'!'
	call	GSISendByte232
	;success!

tic_error_exit	
	return

	movlw	MAX518_BASE_ADDR	;send slave address
	;movlw	0xaa
	call	GSIAddToI2CTxBuff

	movlw	MAX518_CMD_SLCT_DAC0	;send command byte
	call	GSIAddToI2CTxBuff
	;
	movlw	d'128'			;send DAC value 2.5V 
	call	GSIAddToI2CTxBuff
	;Would now set flag that main loop will find an despatch
	;for now despatch from here
	call	GSIServiceI2C

	return
#endif

TestI2C
;Not that I2C 7 bit addresses are composed of bit 7-1 the address
;and bit 0 the r/w flag (0=transmit, 1=receive [as seen from master])
;Thus we take a 7 bit address and rl to get the address and set/clr
;bit 0 for r/w
;for MAX518
;1 write address of chip to TxBuff, 0x2c for this hardware
;2 write MAX518_CMD_SLCT_DAC0 to TxBuff
;3 write address of chip to TxBuff, 0x2c for this hardware
;4 write DAC value 0-255
;5 Set flag to initiate the i2c transfers

;I2c handler will:
;# send address
;# get ack
;# send MAX518_CMD
;# get ack
;# send stop

;# send address
;# get ack
;# send DAC value
;# get ack
;# send stop

;1 write address of chip to TxBuff, 0x2c for this hardware
;2 write MAX518_CMD_SLCT_DAC0 to TxBuff
;3 write address of chip to TxBuff, 0x2c for this hardware
;4 write DAC value 0-255
;5 Set flag to initiate the i2c transfers


	;movlw	MAX518_BASE_ADDR+1	;force an ERR_NO_ACK_FROM_SLAVE
	movlw	MAX518_BASE_ADDR	;
	rlncf	WREG			;an 8 bit *rotation*, not a shift, be sure bit 7=0
	;no need to bcf WREG,0 as this will be zero. Bit 0 is the write bit
	call	GSIAddToI2CTxBuff	;add address to buff
	movlw	MAX518_CMD_SLCT_DAC0
	call	GSIAddToI2CTxBuff
	movlw	0x40
	call	GSIAddToI2CTxBuff	;output byte for dac0
	call	InI2CInt
#if SIMULATOR
	call	InI2CInt
	call	InI2CInt
	call	InI2CInt
	call	InI2CInt
	call	InI2CInt
#endif
	return
#if 0	
	;---------------------------	
	;start state generation
	bsf	SSPCON2,SEN
	movlw	'S'
start_loop
	clrwdt	
	call	GSISendByte232
	btfsc	SSPCON2,SEN		;start state complete?
	goto	start_loop
	;---------------------------
	;address generation	
	movlw	'A'
	call	GSISendByte232

	movlw	MAX518_BASE_ADDR+1
	movwf	SSPBUF
	;-look for BF flag
	movlw	'b'
bf_loop
	clrwdt
	call	GSISendByte232
	btfsc	SSPSTAT,BF
	goto	bf_loop		
	;-look for ACK
	movlw	'k'
	btfsc	SSPCON2,ACKSTAT
	goto	tic_error_exit
	call	GSISendByte232		;ACK received
	;----------------------
	;now send command byte
	movlw	MAX518_CMD_SLCT_DAC0	
	movwf	SSPBUF
	movlw	'b'
bf1_loop
	clrwdt
	call	GSISendByte232
	btfsc	SSPSTAT,BF
	goto	bf1_loop		
	;-look for ACK
	movlw	'k'
	btfsc	SSPCON2,ACKSTAT
	goto	tic_error_exit
	call	GSISendByte232		;ACK received

	;now send data byte
	movlw	0x80
	movwf	SSPBUF
	movlw	'b'
bf2_loop
	clrwdt
	call	GSISendByte232
	btfsc	SSPSTAT,BF
	goto	bf2_loop		
	;-look for ACK
	movlw	'k'
	btfsc	SSPCON2,ACKSTAT
	goto	tic_error_exit
	call	GSISendByte232		;ACK received
	;-----------------------
	;2nd command byte for dac1

	movlw	MAX518_CMD_SLCT_DAC1	
	movwf	SSPBUF
	movlw	'b'
bf1a_loop
	clrwdt
	call	GSISendByte232
	btfsc	SSPSTAT,BF
	goto	bf1a_loop		
	;-look for ACK
	movlw	'k'
	btfsc	SSPCON2,ACKSTAT
	goto	tic_error_exit
	call	GSISendByte232		;ACK received

	;now send data byte
	movlw	0x40
	movwf	SSPBUF
	movlw	'b'
bf2a_loop
	clrwdt
	call	GSISendByte232
	btfsc	SSPSTAT,BF
	goto	bf2a_loop		
	;-look for ACK
	movlw	'k'
	btfsc	SSPCON2,ACKSTAT
	goto	tic_error_exit
	call	GSISendByte232		;ACK received
	;-

	;Send STOP
	bsf	SSPCON2,PEN		;STOP
	movlw	'P'
stop_loop
	clrwdt	
	call	GSISendByte232
	btfsc	SSPCON2,PEN		;stop state complete?
	goto	stop_loop

	movlw	'!'
	call	GSISendByte232
	;success!

tic_error_exit	
	return

	movlw	MAX518_BASE_ADDR	;send slave address
	;movlw	0xaa
	call	GSIAddToI2CTxBuff

	movlw	MAX518_CMD_SLCT_DAC0	;send command byte
	call	GSIAddToI2CTxBuff
	;
	movlw	d'128'			;send DAC value 2.5V 
	call	GSIAddToI2CTxBuff
	;Would now set flag that main loop will find an despatch
	;for now despatch from here
	call	GSIServiceI2C

	return
#endif

	END

