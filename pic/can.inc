;******************************************************************************
;* @file    can.inc part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;can.inc
;CAN error flags
;Same as COMSTAT flags, see page 205 of pic18fxx8 pdf manual
#ifndef CAN_INC_INCLUDED
#define CAN_INC_INCLUDED

#IF GSI_CAN_SUPPORT

#include CANPrTx.inc

;const CANXTDFrameNoRTR  =(CANTxFrameBit & CANTxNoRTRFrame) | (CANTxRTRBit & CANTxNoRTRFrame);
CAN_TX_FLAG_XTD_NO_RTR EQU ((CAN_TX_FRAME_BIT & CAN_TX_XTD_FRAME) | (CAN_TX_RTR_BIT & CAN_TX_NO_RTR_FRAME))

CAN_XFRAME_LENGTH	EQU	14	;id:4,buff:8,length:1,flags:1


CAN_NODE_MASTER		EQU	0
CAN_LAST_NODE		EQU	249
CAN_NODE_NO_NETWORK	EQU	255

CAN_NODE_THIS		EQU	254	;rs232 or can message is for this node


;Slaves from 1 to 250


CAN_MSSG_MAX_SIZEOF_ID	EQU	4
CAN_MSSG_SIZEOF_LENGTH	EQU	1
CAN_MSSG_SIZEOF_FLAG	EQU	1	;1

CAN_MSSG_FIXED_MSSG_SIZE EQU	CAN_MSSG_MAX_SIZEOF_ID+CAN_MSSG_SIZEOF_LENGTH	;id:4,len:1
CAN_MSSG_MAX_DATA_SIZE	EQU	8	;8 
CAN_MSSG_MAX_SIZE	EQU	CAN_MSSG_FIXED_MSSG_SIZE+ CAN_MSSG_MAX_DATA_SIZE ;id:4,len:1	
CAN_MSSG_OFFSET_ID	EQU	0
CAN_MSSG_OFFSET_DATA	EQU	4
CAN_MSSG_OFFSET_LENGTH	EQU	12
CAN_MSSG_OFFSET_FLAGS	EQU	13
;CAN_MSSG_CQ_RX_SIZE	EQU	CAN_MSSG_FIXED_MSSG_SIZE * CAN_RX_CQ_MAX_MSSG

CQ_CAN_MSSG_FIXED_MSSG_SIZE EQU CAN_MSSG_FIXED_MSSG_SIZE+CAN_MSSG_SIZEOF_FLAG
CQ_CAN_MSSG_MAX_SIZE	EQU	CAN_MSSG_MAX_SIZE+CAN_MSSG_SIZEOF_FLAG
CQ_CAN_RX_MAX_MSSGS	EQU	4
CQ_CAN_RX_BUFFER_SIZE	EQU	CQ_CAN_MSSG_MAX_SIZE * CQ_CAN_RX_MAX_MSSGS

CQ_CAN_RX_R2_MAX_MSSGS	EQU	4
CQ_CAN_RX_R2_BUFFER_SIZE EQU	CQ_CAN_MSSG_MAX_SIZE * CQ_CAN_RX_R2_MAX_MSSGS

;CAN_SYNC_POSTSCALER	EQU	50	;only send every n SYNCS
CAN_SYNC_POSTSCALER	EQU	150	;only send every n SYNCS
CAN_BASE_OFFSET		EQU	d'4096'	;every node separated by 4096 bytes

;------------------------------------------------
;Constants for CnMode
;The current state of the CAN software/hardware
;------------------------------------------------
CAN_MODE_RUNNING	EQU	0
CAN_MODE_SLAVE		EQU	1
CAN_MODE_MASTER		EQU	2
CAN_MODE_PC_MASTER	EQU	3
CAN_MODE_COPY_TO_PC	EQU	4	;copy all Rx'd messages to pc
CAN_MODE_LISTENING	EQU	5	;listen to all messages on the CAN


;---------------------------------------------------------
;Constants for GlbCANState
;GlbCANState,GlbCANState1 defines (no State1 bits as yet)
;----------------------------------------------------------
CAN_SET_SLAVE_ID	EQU	7	;wait for master to set an id

;-----------------------------------------------------
;Constants for CnErrorState
;----------------------------------------------------
CN_ERROR_EWARN		EQU	0
CN_ERROR_RXWARN		EQU	1
CN_ERROR_TXWARN		EQU	2
CN_ERROR_RXBP		EQU	3
CN_ERROR_TXBP		EQU	4
CN_ERROR_TXBO		EQU	5
CN_ERROR_RXB1OVFL	EQU	6
CN_ERROR_RXB0OVFL	EQU	7
;CnErrorState1
CN_ERROR1_IRXIF		EQU	0		;CnErrorState1
CN_ERROR1_INIT		EQU	1		;Failed to change mode
CN_ERROR1_RX_CQ_OVF	EQU	2		;Rx CQ ovf

;--------------------------------------------------------------
;	Constants for CnFilter0Mask
;filter out (don't put in the RxBuffer)
;still received by the CAN
;these are only filter 0 frames
;This is currently only to filter out Sync messages
;which otherwise make debugging difficult as the pc
;can't block while reading a message or the CQ (in this code)
;overflows
;See DISABLE_CN_SYNC_RX in debug.inc
;-------------------------------------------------------------
CN_FILTER0_NO_SYNC		EQU	0		;set means no SYNC

;----------------------------------------------------------------
;	CAN message flags
;status flags for CAN messages (flag usually data0)
;----------------------------------------------------------------
;flags for CAN_ID_GET_SET_SLAVE_ID
CAN_ID_SET			EQU	0
CAN_PC_PRESENT		EQU	1
;-------------------------------------
;		CAN Id's
;-------------------------------------

;----------------------------------------------------------------
;	CAN_BASE
;Every node will have a base address.
;First master will be at 0 and use id's 0-4096
;Subsequent nodes will have BASE address set by the master at
;interval of 4096 id's.
;All nodes will use filter0 at the same ID for high priority
;messages between master <--> slave. 
;-----------------------------------------------------------------
;CAN_ID 0-16 sytem level mssg's
;CAN_ID_SYNC			EQU	0
;CAN_ID_CRITICAL_ERROR		EQU	1
;CAN_ID_TIMESTAMP		EQU	2
;CAN_ID_SYSTEM_			EQU	3
;CAN_ID_4-11			EQU	4-11  USER ID's
;CAN_ID_SYSTEM_12-15		EQU	12-15 SYSTEM_ID's
;----------------------------------------------------------------
;	FILTER 0 Rx0
;	id 0-15
;filter0
;0-15, GSI/User time critical (GSI )
;----------------------------------------------------------------
;	FILTER1 Rx0
;	CAN_BASE+16-31
;CAN_BASE+16-23 USER
;CAN_BASE+24-31 SYSTEM
;----------------------------------------------------------------

;mask0 value for f0/f1
;f0/f1=11111111 11111111 11111111 11110000 	
;Accept 0-15 irrespective of filter value (GSI/User time critical)
;f0   =00000000 00000000 00000000 00010000 (16-31 in filter 0)
;f1   =00000000 00000000 00000000 00100000 (CAN_BASE+32-47 in filter 1)

;mask1 value for f2-f5:accept 0-31 (ignored as mask0 does the same)
;f2/f5=11111111 11111111 11111111 11100000
;Accept 0-31
;f2=00000000 00000000 00001111 100 00000 (accept CAN_BASE+3968-3999)   
;f3=00000000 00000000 00001111 101 00000 (accept CAN_BASE+4000-4031)
;f4=00000000 00000000 00001111 110 00000 (accept CAN_BASE+4032-4063)
;f5=00000000 00000000 00001111 111 00000 (accept CAN_BASE+4064-4095)

;filter2 
;filter3
;filter4
;filter5

CAN_BASE					EQU	0
CAN_GSI_BASE				EQU	d'4064'	;0xfe0
;-------------------------------------
;	0-15 =CAN_TIME_CRITICAL	
;-------------------------------------
;	id's 0-3 reserved for GSI
CAN_ID_SYNC					EQU	0
CAN_ID_CRITICAL_ERROR		EQU	1
CAN_ID_TIMESTAMP			EQU	2
;----------------------------------------------------------------
;	id's 4-11 User time critical (filter 0)
;----------------------------------------------------------------
;	id's 12-15 GSI time critical (filter 0)
;----------------------------------------------------------------
;	id's CAN_BASE+0 - CAN_BASE+31 (filter 1)
;	id's CAN_BASE+0 - CAN_BASE+15 User	
;	id's CAN_BASE+16 - CAN_BASE+31 System	
;----------------------------------------------------------------
;	id's CAN_BASE+3968 - CAN_BASE+3999 user  (filter2)
;----------------------------------------------------------------
;	id's CAN_BASE+4000-CAN_BASE+4031 reserved  (filter3)
;----------------------------------------------------------------
;	id's CAN_BASE+4032-CAN_BASE+4063 reserved  (filter4)
;----------------------------------------------------------------
;	id's CAN_BASE+4064-CAN_BASE+4095 GSI services  (filter5)
;----------------------------------------------------------------
CAN_ID_GET_SET_SLAVE_ID		EQU	CAN_GSI_BASE ;
;Master listens on id CAN_GSI_BASE (4064) and will Tx nodes
; base id on CAN_GSI_BASE
;s<->m
;D0: GET/SET id
;b0=CAN_ID_SET
;b1=CAN_PC_PRESENT (only used on CAN_ID_SET m->s)

;-----------------------------------------------------------------
;ESC and CTR codes that terminal and record based pc->node call
;Use same offset so that same jmp table can be used
CAN_ID_TERMINAL_COMMAND		EQU	CAN_GSI_BASE + d'1'
;m<->s

;----------------------------------------------------------------
CAN_ID_RS232_MSSG			EQU	CAN_GSI_BASE + d'2'
CAN_ID_LCD_MSSG				EQU	CAN_GSI_BASE + d'3'
CAN_ID_KEYPAD_MSSG			EQU	CAN_GSI_BASE + d'4'
CAN_ID_SPI_MSSG				EQU	CAN_GSI_BASE + d'5'
CAN_ID_I2C_MSSG				EQU	CAN_GSI_BASE + d'6'
CAN_ID_DIO_MSSG				EQU	CAN_GSI_BASE + d'7'
CAN_ID_ADC_MSSG				EQU	CAN_GSI_BASE + d'8'
CAN_ID_IOCTL_MSSG			EQU	CAN_GSI_BASE + d'9'
CAN_ID_GSI_IOCTL_MSSG		EQU	CAN_GSI_BASE + d'10'
CAN_ID_ERROR_MSSG			EQU	CAN_GSI_BASE + d'11'
CAN_ID_PING_MSSG			EQU	CAN_GSI_BASE + d'12' ;find out if ids (0-250 are on the CAN)
;----------------------------------------------------------------
;These mssgs all use the can data buffer like this:
;d0=cnThisNode	to let receiver know who sent the mssg
;d1=subfuction
;d2:d7 misc data
;----------------------------------------------------------------

;----------------------------------------------------------------
;		Sub-functions
;----------------------------------------------------------------
;----------------------------------------------------------------
;	CAN_ID_KEYPAD_MSSG subfunctions
;----------------------------------------------------------------
CAN_KD_MSSG_KEY_DN			EQU	0
CAN_KD_MSSG_KEY_UP			EQU	1
CAN_KD_MSSG_SET_MODE		EQU	2
CAN_KD_MSSG_GET_MODE		EQU	3

;----------------------------------------------------------------
;	CAN_ID_I2C_MSSG subfunctions
;----------------------------------------------------------------
CAN_I2C_MSSG_STOP_CONDTION	EQU	0
CAN_I2C_MSSG_KEY_UP			EQU	1
CAN_I2C_MSSG_SET_MODE		EQU	2
CAN_I2C_MSSG_GET_MODE		EQU	3

;----------------------------------------------------------
;	CAN_ID_IOCTL_MSSG
;----------------------------------------------------------
CAN_IOCTL_GET_NODE			EQU 0
CAN_IOCTL_GET_MODE			EQU 1
CAN_IOCTL_SET_MODE			EQU 2
CAN_IOCTL_NEW_NODE			EQU 3	;found a new node on CAN
;can mean that a network has started, thus a CAN master changes from
;node==255(no network) to 0, or a slave has attached to a master


;messages going from node->master require separate id's
;otherwise on Rx we Tx the result, which immediately gets
;sent back!
CAN_IOCTL_GET_NODE_N2M		EQU 4
CAN_IOCTL_GET_MODE_N2M		EQU 5
CAN_IOCTL_SET_MODE_N2M		EQU 6
CAN_IOCTL_NEW_NODE_N2M		EQU 7	;found a new node on CAN

;------------------------------------------------------------

CAN_PING_SEND				EQU	1	;data[1]=CAN_PING_SEND if originator
CAN_PING_RECEIVED			EQU	2	;data[1]=CAN_PING_RECEIVED if echoing a PING
;In above cases, data[0] is the node of the executing code
;-----------------------------------------------------

;--------------------------------------
;	CAN MODES
;--------------------------------------
CAN_IOCTL_MODE_NORMAL		EQU 0
CAN_IOCTL_MODE_DISABLE		EQU 1
CAN_IOCTL_MODE_LOOPBACK		EQU 2
CAN_IOCTL_MODE_LISTEN		EQU 3
CAN_IOCTL_MODE_CONFIGURATION	EQU 4
CAN_IOCTL_MODE_ERROR_RECOGNITION EQU 5
;----------------------------------------

;----------------------------------------------------------
;	CAN_ID_GSI_IOCTL_MSSG
;----------------------------------------------------------
CAN_GSI_IOCTL_GET_REG		EQU 0
CAN_GSI_IOCTL_SET_REG		EQU 1
CAN_GSI_IOCTL_1				EQU 2
CAN_GSI_IOCTL_2				EQU 3
CAN_GSI_IOCTL_3				EQU 4
CAN_GSI_IOCTL_4				EQU 5
CAN_GSI_IOCTL_5				EQU 6
CAN_GSI_IOCTL_6				EQU 7
CAN_GSI_IOCTL_7				EQU 8
CAN_GSI_IOCTL_LAST			EQU 8
;------------------------------------------------------------
;messages going from node->master require separate id's
;otherwise on Rx we Tx the result, which immediately gets
;sent back!
CAN_GSI_IOCTL_GET_REG_N2M	EQU CAN_GSI_IOCTL_LAST+1
CAN_GSI_IOCTL_SET_REG_N2M	EQU CAN_GSI_IOCTL_LAST+2
CAN_GSI_IOCTL_1_N2M			EQU CAN_GSI_IOCTL_LAST+3
CAN_GSI_IOCTL_2_N2M			EQU CAN_GSI_IOCTL_LAST+4
CAN_GSI_IOCTL_3_N2M			EQU CAN_GSI_IOCTL_LAST+5
CAN_GSI_IOCTL_4_N2M			EQU CAN_GSI_IOCTL_LAST+6
CAN_GSI_IOCTL_5_N2M			EQU CAN_GSI_IOCTL_LAST+7
CAN_GSI_IOCTL_6_N2M			EQU CAN_GSI_IOCTL_LAST+8
CAN_GSI_IOCTL_7_N2M			EQU CAN_GSI_IOCTL_LAST+9

;------------------------------------------------------------
;	CAN_ID_ERROR_MSSG subfunctions
;------------------------------------------------------------
;CAN_ERROR_
;The message id is sent as data[1] (data[0] is CnNode. See defs.inc



;----------------------------------------------------------
#ENDIF ;GSI_CAN_SUPPORT

#endif ;CAN_INC_INCLUDED
