;******************************************************************************
;* @file    rs232.inc part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************


#ifndef	RS232_INC_INCLUDED

#define RS232_INC_INCLUDED

	#ifdef IN_RS232.ASM
		GLOBAL  GlbSIOMode,GlbSIOMode1,GlbSIOStatus,GlbSIOStatus1,GlbSIOStatus2
	#else
		EXTERN  GlbSIOMode,GlbSIOMode1,GlbSIOStatus,GlbSIOStatus1,GlbSIOStatus2
	#endif	

;Brain dead assembler doesn't allow embedded 'if' in a #if block!
#IF 0
;IMPORTANT NOTE FOR BAUD RATES
;The HSPLL (pic internal multiplier for crystal *4) is NOT used
;for baud rate generator. The crystal on the pic is still used. 
;If we use a 10MHz crystal we cannot get close enough to 115200 baud
;We have 104K and 125K. Remember the *4 PLL does not apply to the 
;baud rate generator.
;Best to use a 20MHz crystal.
;
;Also, although various CBR_XXXX baud rates are written into the 
;windows SDK, 115200 is the maximum baud rate. Selecting higher will 
;select 115200.
#ENDIF



	IF (BAUD_CLOCK == d'10' )
BAUD_RATE_57600		EQU	D'10'
BAUD_RATE_62500		EQU	d'9'
BAUD_RATE_115200	EQU	D'4'
	ENDIF
	 IF (BAUD_CLOCK == d'20')
BAUD_RATE_57600		EQU	D'21'
BAUD_RATE_62500		EQU	d'19'
BAUD_RATE_115200	EQU	D'10'
BAUD_RATE_96000		EQU	D'12'
BAUD_RATE_1250000	EQU	d'0'
BAUD_RATE_625000	EQU	d'1'
BAUD_RATE_416666	EQU	d'2'
BAUD_RATE_312500	EQU	d'3'
BAUD_RATE_250000	EQU	d'4'
BAUD_RATE_208333	EQU	d'5'
	ENDIF

	 IF (BAUD_CLOCK == d'40')
BAUD_RATE_57600		EQU	D'43'
BAUD_RATE_62500		EQU	d'39'
BAUD_RATE_115200	EQU	D'21'
BAUD_RATE_96000		EQU	D'25'	
BAUD_RATE_1250000	EQU	d'1'
BAUD_RATE_625000	EQU	d'2'
BAUD_RATE_416666	EQU	d'4'
BAUD_RATE_312500	EQU	d'6'
BAUD_RATE_250000	EQU	d'8'
BAUD_RATE_208333	EQU	d'10'
	ENDIF


BAUD_RATE			EQU	BAUD_RATE_115200
;BAUD_RATE			EQU	BAUD_RATE_57600
;BAUD_RATE			EQU	BAUD_RATE_62500
;----------------------------------------------------------
;	Record assembly
;Binary mode assembly of record from the pc
;----------------------------------------------------------
R2_CURSOR_RT		EQU	0	;waiting for TYPE	
R2_CURSOR_RL		EQU	1	;waiting for LEN
R2_CURSOR_REC		EQU	2	;waiting for record bytes
R2_CURSOR_ACK		EQU	3	;waiting for ACK

#IF DEBUG
R2_DEFAULT_ECHO_TIMEOUT		EQU		0	;infinite timeout
#ELSE
R2_DEFAULT_ECHO_TIMEOUT		EQU		3	;n 13msec ticks	
#ENDIF
;-------------------------------
;	Rx/Tx
;-------------------------------
R2_RECORD_FIXED_LEN				EQU	3
R2_MAX_RX_RECORD_DATA			EQU		d'14'
R2_MAX_RECORD_LEN				EQU		R2_RECORD_FIXED_LEN+R2_MAX_RX_RECORD_DATA

R2_MAX_TX_RECORD_DATA			EQU		d'14'
;-------------------------------
;		RX
;-------------------------------
RX_BUFF_SIZE				EQU		d'16'
;-------------------------------
;		TX
;-------------------------------
TX_BUFF_SIZE				EQU		d'16'	;currently unused

;------------------------------------
;	BREAK codes
;-----------------------------------

R2_BREAK_RESET_WDT			EQU		WDT_RESET_MSSG
R2_BREAK_RESET_BOR			EQU		BROWNOUT_RESET_MSSG
R2_BREAK_RESET_UNKNOWN		EQU		UNKNOWN_RESET_MSSG
R2_BREAK_USER_ABORT			EQU		USER_ABRT_RESET_MSSG
R2_BREAK_RESET_POR			EQU		POWER_ON_RESET_MSSG
R2_BREAK_RESET_STACK_OVF	EQU		STACK_OVF_RESET_MSSG
R2_BREAK_RESET_STACK_UNF	EQU		STACK_UNF_RESET_MSSG
R2_BREAK_SEND_RECORD		EQU		20		;leave space for more resets
R2_BREAK_SEND_MULTIPLE_RECORDS		EQU		21		;more than one rec, pc stays slave till receives rs232 msg req_slave
R2_BREAK_SEND_DATA			EQU		22		;send arbitrary data (like adc) protocol yet to be determined

;-------------------------------
;	GlbSIOStatus:
;-------------------------------
SIO_XOFF_SENT				EQU		0
SIO_XON_SENT				EQU		1

SIO_XOFF_RECVD				EQU		2
SIO_XON_RECVD				EQU		3

SIO_RX_BUFF_OVF				EQU		4
SIO_TX_BUFF_OVF				EQU		5

SIO_DOUBLE_ERR				EQU		6
SIO_USR_ABRT				EQU		7

;---------------------------------
;	GlbSIOStatus1:
;---------------------------------
;Master, the pc will not Tx asynchronously. This node can Tx and check
;the RxBuffer for echoed chars from the pc without danger of interleaving
;data. 
SIO_S1_MASTER				EQU		0
SIO_S1_CAN_DATA_PENDING_TX	EQU		1 ;CAN data is awaiting Tx
SIO_S1_REQ_MASTER			EQU		2 ;node wants to be 232 master
SIO_S1_BREAK_RECEIVED		EQU		3
SIO_S1_ESC					EQU		4
SIO_S1_CHAR_AVAILABLE		EQU		5
SIO_S1_ERR_ECHO_TX			EQU		6 ;also see R2ErrState variable for general Error handling
SIO_S1_ERR_ECHO_ACK_TX		EQU		7 ;the ACK (or REPEAT) didn't echo correctly

;---------------------------------
;	GlbSIOStatus2:
;---------------------------------
SIO_S2_RS232_DATA_PENDING_TX EQU	1
;------------------------------
;	GlbSIOMode
;------------------------------
SIO_M_BINARY				EQU		0
SIO_M_CAN_RX_TO_SERIAL		EQU		1	;Received CAN messages sent to serial port
;------------------------
;	GlbSIOMode1
;------------------------
SIO_M1_ECHO				EQU		0
SIO_M1_DEFAULT_FLAGS			EQU		(1 << SIO_M1_ECHO)

;-------------------------
;	R2ErrState
;-------------------------
SIO_ERR_ECHO_TX				EQU		0
SIO_ERR_ECHO_RX				EQU		0

;----------------------------
NEWLINE_RS232	MACRO
	movlw	0x0d
	call	GSISendByte232
	movlw	0x0a
	call	GSISendByte232
	ENDM

SPACE_RS232	MACRO
	movlw	' '
	call	GSISendByte232
	ENDM

;----------------------------------------------------------------
;		Binary mode
;----------------------------------------------------------------
;RECORD TYPE	BYTE
;RECORD NODE	BYTE	(0-250) Node values 251-255 reserved for GSI (255 == this)
;RECORD LENGTH	BYTE	(0-8) Data only, doesn't include Type and Node
;RECORD 	0-8 bytes
;-----------------------------------------------------------------------
;			NOTE
;The RECORD_NODE will be ignored for RT_CTR and RT_ESC records as it is
;implicit that it is for the node attached to the pc.
;May change this behaviour later
;NOTE :
;All these codes are pc->node5
;--------------------------------------------------------------------------

RT_ITERATOR				SET	1
RT_START_CTR_CODES			EQU	1

;goto	GSIBeginningOfLine	;1 ctr-a
RT_BEGINING_OF_LINE_ASC			EQU	1
RL_BEGINING_OF_LINE_ASC			EQU	0

;goto	GSICursorLeft		;2 ctr-b
RT_CURSOR_LEFT_ASC			EQU	2
RL_CURSOR_LEFT_ASC			EQU	0

;goto	GSIUsrAbrt		;3 ctr-c
RT_USER_ABRT_ASC				EQU	3
RL_USER_ABRT_ASC				EQU	0

;goto	GSIDeleteCharFwd	;4 ctr-d
RT_DELETE_CHAR_FWD_ASC			EQU	4
RL_DELETE_CHAR_FWD_ASC			EQU	0

;goto	GSIEndOfLine		;5 ctr-e
RT_END_OF_LINE_ASC				EQU	5
RL_END_OF_LINE_ASC				EQU	0

;goto	GSICursorRight		;6 ctr-f
RT_CURSOR_RIGHT_ASC				EQU	6
RL_CURSOR_RIGHT_ASC				EQU	0

;goto	GSIBell				7 ctr-g
RT_RS232_REQ_MASTER_ASC				EQU	7
RL_RS232_REQ_MASTER_ASC				EQU	0

;goto	GSIBS			;8 ctr-h
RT_BS_ASC					EQU	8
RL_BS_ASC					EQU	0

;goto	GSIHTab		;9 ctr-i
RT_HTAB_ASC					EQU	9
RL_HTAB_ASC					EQU	0

;goto	GSICursorDown		;10 ctrj
RT_CURSOR_LINE_FEED_ASC			EQU	d'10'
RL_CURSOR_LINE_FEED_ASC			EQU	0

;goto	GSIEnd			;11 ctr-k
RT_END_ASC					EQU	d'11'
RL_END_ASC					EQU	0

;goto	GSICls			;12 ctr-l
RT_CLS_ASC					EQU	d'12'
RL_CLS_ASC					EQU	0

;goto	GSICR			;13 ctr-m
RT_CR_ASC					EQU	d'13'
RL_CR_ASC					EQU	0

;goto	GSICursorDown		;14 ctr-n
RT_CURSOR_DOWN_ASC				EQU	d'14'
RL_CURSOR_DOWN_ASC				EQU	0

;goto	GSIEchoToggle		;15 ctr-o
RT_ECHO_TOGGLE_ASC				EQU	d'15'
RL_ECHO_TOGGLE_ASC				EQU	0

;goto	GSICursorUp		;16 ctr-p 
RT_CURSOR_UP_ASC				EQU	d'16'
RL_CURSOR_UP_ASC				EQU	0

;goto	GSIXOn			;17 ctr-q
RT_XON_ASC					EQU	d'17'
RL_XON_ASC					EQU	0

;goto	GSIBinaryMode		;18 ctr-r
;pc->master
RT_BINARY_MODE_ASC				EQU	d'18'
RL_BINARY_MODE_ASC				EQU	0

;goto	GSIXOff		;19 ctr-s
RT_XOFF_ASC					EQU	d'19'
RL_XOFF_ASC					EQU	0

;goto	GSICursorRight5	;20 ctr-t
RT_CURSOR_RIGHT_5_ASC			EQU	d'20'
RL_CURSOR_RIGHT_5_ASC			EQU	0

;goto	GSIAsciiMode		;21 ctr-u
;pc->master
RT_ASCII_MODE_ASC				EQU	d'21'
RL_ASCII_MODE_ASC				EQU	0

;goto	GSICursorLeft5		;22 ctr-v
RT_CURSOR_LEFT_5_ASC			EQU	d'22'
RL_CURSOR_LEFT_5_ASC			EQU	0

;goto	GSICursorLeft10	;23 ctr-w
RT_CURSOR_LEFT_10_ASC			EQU	d'23'
RL_CURSOR_LEFT_10_ASC			EQU	0

;goto	GSICRKeypadToggleState	;24 ctr-x
RT_KEYPAD_TOGGLE_STATE_ASC			EQU	d'24'
RL_KEYPAD_TOGGLE_STATE_ASC			EQU	0

;goto	GSIKeypadToggleMode	;25 ctr-y
RT_KEYPAD_TOGGLE_MODE_ASC			EQU	d'25'
RL_KEYPAD_TOGGLE_MODE_ASC			EQU	0

;goto	GSICtrZ		;26 ctr-z
RT_CTR_Z_ASC				EQU	d'26'
RL_CTR_Z_ASC				EQU	0
RT_END_CTR_CODES			EQU	d'26'



RT_ESC_ESC_ASC				EQU	d'27'
RL_ESC_ESC_ASC				EQU	0

;--------------------------------------------
;Record struct equivalent to ESC commands
;--------------------------------------------

RT_START_ESC_CODES			EQU	d'28'

RT_ESC_VERSION_STRING_ASC			EQU	d'28'
RL_ESC_VERSION_STRING_ASC			EQU	0

RT_ESC_CAPABILITIES_ASC			EQU	d'29'
RL_ESC_CAPABILITIES_ASC			EQU	0

RT_ESC_GET_BUFFER_ASC			EQU	d'30'
RL_ESC_GET_BUFFER_ASC			EQU	0

RT_ESC_SET_BUFFER_ASC			EQU	d'31'
RL_ESC_SET_BUFFER_ASC			EQU	0

RT_ESC_CLS_ASC				EQU	d'32'
RL_ESC_CLS_ASC				EQU	0

RT_ESC_LCD_INSTRUCTION_ASC			EQU	d'33'
RL_ESC_LCD_INSTRUCTION_ASC			EQU	0

RT_ESC_GET_CUR_TO_EOL_ASC			EQU	d'34'
RL_ESC_GET_CUR_TO_EOL_ASC			EQU	0

RT_ESC_GET_LINE_ASC				EQU	d'35'
RL_ESC_GET_LINE_ASC				EQU	0

RT_ESC_SET_LINE_ASC				EQU	d'36'
RL_ESC_SET_LINE_ASC				EQU	0

RT_ESC_HOME_ASC				EQU	d'37'
RL_ESC_HOME_ASC				EQU	0

RT_ESC_FLOW_CONTROL_ON_ASC			EQU	d'38'
RL_ESC_FLOW_CONTROL_ON_ASC			EQU	0

RT_ESC_FLOW_CONTROL_OFF_ASC			EQU	d'39'
RL_ESC_FLOW_CONTROL_OFF_ASC			EQU	0

RT_ESC_INIT_ASC				EQU	d'40'
RL_ESC_INIT_ASC				EQU	0

RT_ESC_GET_BUTTON_STATE_ASC			EQU	d'41'
RL_ESC_GET_BUTTON_STATE_ASC			EQU	0

RT_ESC_CAN_SET_MASTER_ASC			EQU	d'42'
RL_ESC_CAN_SET_MASTER_ASC			EQU	0

RT_ESC_CAN_SET_SLAVE_ASC			EQU	d'43'
RL_ESC_CAN_SET_SLAVE_ASC			EQU	0

RT_END_ESC_CODES			EQU	d'43'

RT_START_USER_CODES				EQU	d'44'

RT_USER_0				EQU	d'44'
RL_USER_0				EQU	0

RT_USER_1				EQU	d'45'
RL_USER_1				EQU	0

RT_USER_2				EQU	d'46'
RL_USER_2				EQU	0

RT_USER_3				EQU	d'47'
RL_USER_3				EQU	0

RT_USER_4				EQU	d'48'
RL_USER_4				EQU	0

RT_END_USER_CODES			EQU	d'48'


RT_ITERATOR				SET	d'53'
RT_START_GSI_CODES			EQU	d'53'

;-----Repeat, an echo error (at master) or Rx error (at node) occured--------
;pc->master
RT_GSI_REPEAT				EQU	RT_ITERATOR	;53
RL_GSI_REPEAT				EQU	0

RT_ITERATOR				SET	RT_ITERATOR+1

;--------------------------------------------------------------
;Single byte codes
;-------------------------------------------------------------
;------Acknowledge receipt of data-----

;master will suspend processing till ACK is received
;pc->master
RT_GSI_ACK				EQU	RT_ITERATOR	;54
RL_GSI_ACK				EQU	0

RT_ITERATOR				SET	RT_ITERATOR+1

;-----ERROR
;pc<->master
RT_GSI_ERROR				EQU	RT_ITERATOR	;55
RL_GSI_ERROR				EQU	0

RT_ITERATOR				SET	RT_ITERATOR+1

;-----FATAL ERROR
;pc<->master
RT_GSI_FATAL				EQU	RT_ITERATOR	;56
RL_GSI_FATAL				EQU	0

RT_ITERATOR				SET	RT_ITERATOR+1
;-----RESERVED1
;pc<->master
RT_GSI_RESERVED_1			EQU	RT_ITERATOR	;57
RL_GSI_RESERVED_1				EQU	0

RT_ITERATOR				SET	RT_ITERATOR+1
;-----RESERVED2
;pc<->master
RT_GSI_RESERVED_2			EQU	RT_ITERATOR	;58
RL_GSI_RESERVED_2			EQU	0

RT_ITERATOR				SET	RT_ITERATOR+1

;-----Send a Mssg to an RS232 module on a node------------------------------
RT_GSI_RS232_MSSG			EQU	RT_ITERATOR	;59
RL_RS232_MSSG				EQU	1	;1 byte the MssgType variable data length
RT_ITERATOR				SET	RT_ITERATOR+1
;Data position 0 equates
;Set record.data[0] to these for the specific function
;data bytes 1->R2_MAX_DATA_SIZE then hold other data
;As R2Records there can be > 8 data bytes
GSI_RS232_MAX_SUCCESS_FAIL_DATA	EQU	8
GSI_RS232_MESSAGE_SUCCESS	EQU	0	
GSI_RS232_MESSAGE_FAIL		EQU	1

;If receive REQ_SLAVE, then pc has allowed this node to be
;master. If sent, then this node has allowed pc to be master
GSI_RS232_MESSAGE_REQ_SLAVE	EQU	2	
;on receipt of NULL outside of a mssg, interpret as idle time
;the pc has nothing to send. This node responds with a null:
;it has nothing to send, or a record, which can be the data or it
;can be an rs232 record MESSAGE_REQ_MASTER, telling pc this node wants
;to be master. 
;During times of full transmission by the pc we must use the 
;GSI_RS232_MESSAGE_MASTER_REQ_BIT method as detailed above

GSI_RS232_MESSAGE_REQ_MASTER	EQU	3	

GSI_RS232_MESSAGE_SEND_DATA	EQU	4
GSI_RS232_MESSAGE_GET_DATA	EQU	5
GSI_RS232_MESSAGE_CHANGE_NODE	EQU	6	;usually master has changed node from 255 (no network) to 0:master
GSI_RS232_MESSAGE_NEW_NODE		EQU	7	;usually when a new slave has registered
GSI_RS232_MESSAGE_n3		EQU	8

;When returning success or failure can also OR in this bit
;to data[0]. If set, then PC knows we wish to be master

GSI_RS232_MESSAGE_MASTER_REQ_BIT EQU	7


RT_GSI_RS232_BREAK_MSSG			EQU	RT_ITERATOR	;60
RL_GSI_RS232_BREAK_MSSG			EQU	1
RT_ITERATOR			SET	RT_ITERATOR+1
;Subfunction constants (data[0])
GSI_RS232_BREAK_POR      		EQU 1 ;    Power on reset detected
GSI_RS232_BREAK_BOR      		EQU 2 ;    Brown out reset
GSI_RS232_BREAKSOVF    			EQU 3 ;    Stack Overflow
GSI_RS232_BREAKSUF     			EQU 4 ;    Stack underflow
GSI_RS232_BREAKWDT     			EQU 5 ;    WDT reset  
GSI_RS232_BREAKUnknownReset 	EQU 6 ;    Unknown reset
GSI_RS232_BREAKUserAbortReset	EQU 7 ;
GSI_RS232_BREAKAsyncRecord   	EQU 8 ;    Async

;-----Send a Mssg to an LCD on a node------------------------------
RT_GSI_LCD_MSSG			EQU	RT_ITERATOR	;61
RL_GSI_LCD_MSSG			EQU	6	;4 bytes the node, 1 byte the Mssg, 1 byte the subfunction
RT_ITERATOR			SET	RT_ITERATOR+1
;Data position 0 equates
;Set record.data[0] to these for the specific function
;data bytes 1-7 then hold other data
GSI_LCD_MESSAGE_INSTRUCTION	EQU	0
GSI_LCD_MESSAGE_WRITE_STRING	EQU	1
GSI_LCD_MESSAGE_WRITE_CHAR	EQU	2
GSI_LCD_MESSAGE_GET_STRING	EQU	3
GSI_LCD_MESSAGE_CTR_JMP_TBL	EQU	4
GSI_LCD_MESSAGE_GET_CAPABILITIES EQU 5
;----------------------------------------------------------------

;-----Send a Mssg to a Keypad on a node------------------------------
RT_GSI_KEYPAD_MSSG			EQU	RT_ITERATOR	;62
RL_GSI_KEYPAD_MSSG			EQU	5	;4 bytes the node, 1 byte the Mssg
RT_ITERATOR				SET	RT_ITERATOR+1
;Data position 0 equates
;Set record.data[0] to these for the specific function
;data bytes 1-7 then hold other data
;Note that these are not bit variables, we have 256 posssible commands

CAN_KD_MSSG_KEY_DN		EQU	0
CAN_KD_MSSG_KEY_UP		EQU	1
CAN_KD_MSSG_SET_MODE	EQU	2
CAN_KD_MSSG_GET_MODE	EQU	3
GSI_KD_MSSG_READ		EQU	4	;read the KdStatus variable(s)
GSI_KD_MSSG_WRITE		EQU	5	;write the KdStatus variable(s)
;---------------------------------------------------------------

;-----Send a Mssg to an SPI module on a node------------------------------
RT_GSI_SPI_MSSG			EQU	RT_ITERATOR	;63
RL_GSI_SPI_MSSG			EQU	6	;4 bytes the node, 1 byte the address,1 byte the Mssg,
RT_ITERATOR				SET	RT_ITERATOR+1

;-----Send a Mssg to an I2C module on a node------------------------------
RT_GSI_I2C_MSSG			EQU	RT_ITERATOR	;64
RL_GSI_I2C_MSSG			EQU	5	;4 bytes the node, 1 byte the Mssg
RT_ITERATOR				SET	RT_ITERATOR+1

;-----Send a Mssg to a dio on a node------------------------------
RT_GSI_DIO_MSSG			EQU	RT_ITERATOR	;65
RL_GSI_DIO_MSSG			EQU	6	;4 bytes the node, 1 byte the dio,1 byte the Mssg
RT_ITERATOR				SET	RT_ITERATOR+1

;-----Send a Mssg to a dio on a node------------------------------
RT_GSI_ADC_MSSG			EQU	RT_ITERATOR	;66
RL_GSI_ADC_MSSG			EQU	6	;4 bytes the node, 1 byte the adc,1 byte the Mssg
RT_ITERATOR				SET	RT_ITERATOR+1

;-----------------------------------
;	RS232 CAN Records
;-----------------------------------
;---Tell node it is the master controlled by a pc
;pc->master
RT_GSI_CAN_SET_PC_MASTER		EQU	RT_ITERATOR	;67
RL_GSI_CAN_SET_PC_MASTER		EQU	4			;Node id in case more than one pc master
RT_ITERATOR				SET	RT_ITERATOR+1

;---Tell node it is the stand alone master
;pc->master
RT_GSI_CAN_SET_MASTER			EQU	RT_ITERATOR		;68 As seen from PIC Master node
RL_GSI_CAN_SET_MASTER			EQU	0
RT_ITERATOR				SET	RT_ITERATOR+1

;---Send a message down the CAN Message is in RECORD as a correctly ;formatted CAN MESSAGE
;pc->master
RT_GSI_CAN_SEND_MSSG			EQU	RT_ITERATOR		;69 As seen from PIC Master node
RL_GSI_CAN_SEND_MSSG			EQU	0
RT_ITERATOR				SET	RT_ITERATOR+1

;----CAN master has encountered a CAN error the pc needs to know about
;master->pc
RT_GSI_CAN_ERROR			EQU	RT_ITERATOR	;70
RL_GSI_CAN_ERROR			EQU	0
RT_ITERATOR				SET	RT_ITERATOR+1

;--A remote node has requested an id (RT_TX_CAN_MSSG will send the ID)
;master->pc
RT_GSI_CAN_SET_SLAVE_ID_REQ		EQU	RT_ITERATOR	;71
RL_GSI_CAN_SET_SLAVE_ID_REQ		EQU	5 ;Flag +SLAVE_BASE
RT_ITERATOR				SET	RT_ITERATOR+1

;--rs232 is attached to a slave. Tell slave to send a request to the master
;for an id. Equivalent to holding '1' or switch at power up
;pc->slave
	ERRORLEVEL -208
RT_GSI_CAN_SLAVE_GET_SLAVE_ID_REQ		EQU	RT_ITERATOR ;72
RL_GSI_CAN_SLAVE_GET_SLAVE_ID_REQ		EQU	0
	ERRORLEVEL +208
RT_ITERATOR				SET	RT_ITERATOR+1

RT_GSI_CAN_IOCTL		EQU	RT_ITERATOR ;73
RL_GSI_CAN_IOCTL		EQU	0
RT_ITERATOR				SET	RT_ITERATOR+1

RT_GSI_GSI_IOCTL		EQU	RT_ITERATOR ;74
RL_GSI_GSI_IOCTL		EQU	0
RT_ITERATOR			SET	RT_ITERATOR+1

RT_GSI_CAN_PING			EQU	RT_ITERATOR ;75
RL_GSI_CAN_PING			EQU	0
RT_ITERATOR				SET	RT_ITERATOR+1

RT_GSI_CAN_RESERVED_1		EQU	RT_ITERATOR ;76
RL_GSI_CAN_RESERVED_1		EQU	0
RT_ITERATOR			SET	RT_ITERATOR+1

RT_GSI_CAN_RESERVED_2		EQU	RT_ITERATOR ;77
RL_GSI_CAN_RESERVED_2		EQU	0
RT_ITERATOR			SET	RT_ITERATOR+1

RT_GSI_CAN_RESERVED_3		EQU	RT_ITERATOR ;78
RL_GSI_CAN_RESERVED_3		EQU	0
RT_ITERATOR			SET	RT_ITERATOR+1

RT_GSI_CAN_RESERVED_4		EQU	RT_ITERATOR ;79
RL_GSI_CAN_RESERVED_4		EQU	0
RT_ITERATOR			SET	RT_ITERATOR+1

RT_GSI_CAN_RESERVED_5		EQU	RT_ITERATOR ;80
RL_GSI_CAN_RESERVED_5		EQU	0
RT_ITERATOR			SET	RT_ITERATOR+1

;-------------------------------------------------
;rs232 codes
;The caller wants this code to be MASTER
;
RT_RS232_SET_MASTER			EQU	RT_ITERATOR	;81
RL_RS232_SET_MASTER			EQU	0

RT_ITERATOR				SET	RT_ITERATOR+1

;The sender wants to be MASTER
;the receiver must become SLAVE if poss
RT_RS232_REQ_MASTER			EQU	RT_ITERATOR	;82
RL_RS232_REQ_MASTER			EQU	0

;implemented in can,inc. Doesn't need to be visible here
;RT_GSI_TERMINAL_MSSG			EQU	RT_ITERATOR	
;RL_GSI_TERMINAL_MSSG			EQU	0

RT_ITERATOR				SET	RT_ITERATOR+1	;83
RT_END_PC_TO_NODE_CODES			EQU	RT_ITERATOR
;-------------------------------------------------------------
;		Node -> pc	
;These codes are for node->pc transmission
;_NP: NODE-> PC
;-------------------------------------------------------------
RT_NODE_TO_PC_CODES				EQU	RT_ITERATOR
;----CAN Master has received a message that the pc needs to see
;node->pc
RT_GSI_R2_RX_CAN_MSSG_NP			EQU	RT_ITERATOR ;83
RL_GSI_R2_RX_CAN_MSSG_NP			EQU	0
RT_ITERATOR				SET	RT_ITERATOR+1	;84



RT_END_GSI_CODES			EQU	RT_ITERATOR-1




;don't include >1 for each compilation unit
#endif	;RS232_INC_INCLUDED


	
