/*****************************************************************************
*	GSI a socket interface for Micro-controllers and Controller Area Network *
*   Copyright (C) 2014 Glenn Self                                            *
*                                                                            *
*   This program is free software: you can redistribute it and/or modify     *
*   it under the terms of the GNU General Public License as published by     *
*   the Free Software Foundation, either version 3 of the License, or        *
*   (at your option) any later version.                                      *
*                                                                            *
*    This program is distributed in the hope that it will be useful,         *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
*    GNU General Public License for more details.                            *
*                                                                            *
*    You should have received a copy of the GNU General Public License       *
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
*****************************************************************************/
#ifndef CANDEFS_H_INCLUDED
#define CANDEFS_H_INCLUDED

#include "gsidefs.h"

/****************************************************************************
; *********************************************************************
; *from  Microchip CanPrtx.asm
; * CAN_TX_MSG_FLAGS
; *
; * This values define flags related to transmission of a
; * CAN message.  There could be more than one this flag
; * ANDed together to form multiple flags.
; *
; *********************************************************************

CAN_TX_PRIORITY_BITS:   = B'00000011'
CAN_TX_PRIORITY_0:      = B'11111100'   ;XXXXXX00
CAN_TX_PRIORITY_1:      = B'11111101'   ; XXXXXX01
CAN_TX_PRIORITY_2:      = B'11111110'   ; XXXXXX10
CAN_TX_PRIORITY_3:      = B'11111111'   ; XXXXXX11

CAN_TX_FRAME_BIT:       = B'00001000'
CAN_TX_STD_FRAME:       = B'11111111'   ; XXXXX1XX
CAN_TX_XTD_FRAME:       = B'11110111'   ; XXXXX0XX

CAN_TX_RTR_BIT:         = B'01000000'
CAN_TX_RTR_BIT_NO:      = 0x06
CAN_TX_NO_RTR_FRAME:    = B'11111111'   ; X1XXXXXX
CAN_TX_RTR_FRAME:       = B'10111111'   ; X0XXXXXX
**************************************************************************/
const unsigned char CANTxFrameBit     =0x8;               //Frame type (EXT or STD)
const unsigned char CANTxXtdFrame     =0xf7;              //AND with CANTxFrameBit for an XTD frame (29 bit)
const unsigned char CANTxStdFrame     =0xff;              //AND with CANTxFrameBit for a STD frame

const unsigned char CANTxRTRBit       =0x40;               //Remote transfer request control
const unsigned char CANTxRTRBitNum    =6;
const unsigned char CANTxNoRTRFrame   =0xff;              //AND with CANTxRTRBit for NO RTR (default)
const unsigned char CANTxRTRFrame     =0xbf;              //AND with CANTxRTRBit for RTR

const unsigned char CANXTDFrameNoRTR  =(CANTxFrameBit & CANTxXtdFrame) | (CANTxRTRBit & CANTxNoRTRFrame);

const GSIChar CANMaxData =8;

//const int GSICANIDPlusFlagLength = 5;
const GSIChar GSICANIDPlusFlagLength = 5;
const GSIChar GSICANXtdFrameLength=CANMaxData+GSICANIDPlusFlagLength+1; //+1 for the length byte

const GSIChar GSIRecordOffsetCANFrameID = 0;
const GSIChar GSIRecordOffsetCANFrameBuffer = 4; //offset to buffer within an R2Record
const GSIChar GSIRecordOffsetCANFrameOriginNode =GSIRecordOffsetCANFrameBuffer; //was +1 (17/7/06) offset to buffer within an R2Record

const GSIChar GSICANFrameOffsetNode	=0;  //node is first item of cn buff
const GSIChar GSICANFrameOffsetSubfunction=GSICANFrameOffsetNode+1;

//Note the following constants are for a native CAN frame.
//when a frame is received down rs232 it is a variable length
//and these constants only apply after the frame has been assembled
//by calling Record2Frame(record As BinRecord, frame As CANFrame)
const GSIChar GSICANFrameOffsetLength = 12;
const GSIChar GSICANFrameOffsetFlags = 13;

const long GSICANFrameBase = 4096;
const long GSICANBase = 4064;


const GSIChar GSICANIDSync = 0;
const GSIChar GSICANIDCriticalError = 1;
const GSIChar GSICANIDTimestamp = 2;
//;----------------------------------------------------------------
//;   id's 4-11 User time critical (filter 0)
//;----------------------------------------------------------------
//;   id's 12-15 GSI time critical (filter 0)
//;----------------------------------------------------------------
//;   id's CAN_BASE+0 - CAN_BASE+31 (filter 1)
//;   id's CAN_BASE+0 - CAN_BASE+15 User
//;   id's CAN_BASE+16 - CAN_BASE+31 System
//;----------------------------------------------------------------
//;   id's CAN_BASE+3968 - CAN_BASE+3999 user  (filter2)
//;----------------------------------------------------------------
//;   id's CAN_BASE+4000-CAN_BASE+4031 reserved  (filter3)
//;----------------------------------------------------------------
//;   id's CAN_BASE+4032-CAN_BASE+4063 reserved  (filter4)
//;----------------------------------------------------------------
//;   id's CAN_BASE+4064-CAN_BASE+4095 GSI services  (filter5)
//;----------------------------------------------------------------
const unsigned long GSICANIDGetSetSlaveID = GSICANBase;     //
//;Master listens on id GSICANBase (4064) and will Tx nodes
//; base id on GSICANBase
//;s<->m
//;D0: GET/SET id
//;b0=CAN_ID_SET
//;b1=CAN_PC_PRESENT (only used on CAN_ID_SET m->s)

//;-----------------------------------------------------------------
//;ESC and CTR codes that terminal and record based pc->node call
//;Use same offset so that same jmp table can be used
const long GSICANIDTerminalCommand = GSICANBase + 1;
//;m<->s

//;----------------------------------------------------------------
const long GSICANIDRS232Message = GSICANBase + 2;
const long GSICANIDLCDMessage = GSICANBase + 3;
const long GSICANIDKeypadMessage = GSICANBase + 4;
const long GSICANIDSPIMessage = GSICANBase + 5;
const long GSICANIDI2CMessage = GSICANBase + 6;
const long GSICANIDDIOMessage = GSICANBase + 7;
const long GSICANIDADCMessage = GSICANBase + 8;;
const long GSICANIDIOCTLMessage = GSICANBase + 9;
const long GSICANIDGSIIOCTLMessage = GSICANBase + 10;
const long GSICANIDErrorMessage= GSICANBase+11;
//const long GSICANIDSendMessage= GSICANBase+12;
const long  GSICANIDPingMessage=GSICANBase+12;
//;----------------------------------------------------------------

//----------------------------------------------------------------
//   GSICANIDKEYPADMessage subfunctions
//----------------------------------------------------------------
const GSIChar GSICANKeypadMessageKeyDown = 0;
const GSIChar GSICANKeypadMessageKeyUp = 1;
const GSIChar GSICANKeypadMessageSetMode = 2;
const GSIChar GSICANKeypadMessageGetMode = 3;
const GSIChar GSIKeypadMessageRead=	4	;  //read the KdStatus variable(s)
const GSIChar GSIKeypadMessageWrite=	5	;


//----------------------------------------------------------------
//   CAN_ID_I2CMessage subfunctions
//----------------------------------------------------------------
const GSIChar CANI2CMessageStopCondition = 0;
const GSIChar CANI2CMessageKeyUp = 1;
const GSIChar CANI2CMessageSetMode = 2;
const GSIChar CANI2CMessageGetMode = 3;

//----------------------------------------------------------
//   GSICANIDIOCTLMessage
//----------------------------------------------------------
const GSIChar CANIOCTLGetNode = 0;
const GSIChar CANIOCTLGetMode = 1;
const GSIChar CANIOCTLSetMode = 2;
const GSIChar CANIOCTLGetNodeNode2Master = 3;
const GSIChar CANIOCTLGetModeNode2Master = 4;
const GSIChar CANIOCTLSetModeNode2Master = 5;


const GSIChar CANModeNormal = 0;
const GSIChar CANModeDisable = 1;
const GSIChar CANModeLoopback = 2;
const GSIChar CANModeListen = 3;
const GSIChar CANModeConfiguration = 4;
const GSIChar CANModeErrorRecognition = 5;


//----------------------------------------------------------
//   CAN_ID_GSI_IOCTLMessage
//----------------------------------------------------------
const GSIChar CANGSIIOCTLGetReg = 0;
const GSIChar CANGSIIOCTLSetReg = 1;
const GSIChar CANGSIIOCTL1 = 2;
const GSIChar CANGSIIOCTL2 = 3;
const GSIChar CANGSIIOCTL3 = 4;
const GSIChar CANGSIIOCTL4 = 5;
const GSIChar CANGSIIOCTL5 = 6;
const GSIChar CANGSIIOCTL6 = 7;
const GSIChar CANGSIIOCTL7 = 8;
const GSIChar CANGSIIOCTLLast = 8;
//------------------------------------------------------------
//-------------------------------------------------------------
//      GSICANIDPingMessage
//-----------------------------------------------------------------------
const GSIChar CANGSIPingSend=1;	    //;data[1]=CAN_PING_SEND if originator
const GSIChar CANGSIPingReceived=2;	//data[1]=CAN_PING_RECEIVED if echoing a PING
//In above cases, data[0] is the node of the executing code
//----------------------------------------------------------------------

//messages going from node->master require separate id's
//otherwise on Rx we Tx the result, which immediately gets
//sent back!
const GSIChar CANGSIIOCTLGetRegNode2Master = CANGSIIOCTLLast + 1;
const GSIChar CANGSIIOCTLSetRegNode2Master = CANGSIIOCTLLast + 2;
const GSIChar CANGSIIOCTL1Node2Master = CANGSIIOCTLLast + 3;
const GSIChar CANGSIIOCTL2Node2Master = CANGSIIOCTLLast + 4;
const GSIChar CANGSIIOCTL3Node2Master = CANGSIIOCTLLast + 5;
const GSIChar CANGSIIOCTL4Node2Master = CANGSIIOCTLLast + 6;
const GSIChar CANGSIIOCTL5Node2Master = CANGSIIOCTLLast + 7;
const GSIChar CANGSIIOCTL6Node2Master = CANGSIIOCTLLast + 8;
const GSIChar CANGSIIOCTL7Node2Master = CANGSIIOCTLLast + 9;





#if 0

const int GSI_CAN_FRAME_OFFSET_ID = 0;
const int GSI_CAN_FRAME_OFFSET_BUFFER = 4;
//Note the following constants are for a native CAN frame.
//when a frame is received down rs232 it is a variable length
//and these constants only apply after the frame has been assembled
//by calling Record2Frame(record As BinRecord, frame As CANFrame)
const int GSI_CAN_FRAME_OFFSET_LENGTH = 12;
const int GSI_CAN_FRAME_OFFSET_FLAGS = 13;

const int GSI_FRAME_BASE = 4096;
const int GSI_CAN_BASE = 4064;

const int GSI_CAN_ID_SYNC = 0;
const int GSI_CAN_ID_CRITICAL_ERROR = 1;
const int GSI_CAN_ID_TIMESTAMP = 2;
//;----------------------------------------------------------------
//;   id's 4-11 User time critical (filter 0)
//;----------------------------------------------------------------
//;   id's 12-15 GSI time critical (filter 0)
//;----------------------------------------------------------------
//;   id's CAN_BASE+0 - CAN_BASE+31 (filter 1)
//;   id's CAN_BASE+0 - CAN_BASE+15 User
//;   id's CAN_BASE+16 - CAN_BASE+31 System
//;----------------------------------------------------------------
//;   id's CAN_BASE+3968 - CAN_BASE+3999 user  (filter2)
//;----------------------------------------------------------------
//;   id's CAN_BASE+4000-CAN_BASE+4031 reserved  (filter3)
//;----------------------------------------------------------------
//;   id's CAN_BASE+4032-CAN_BASE+4063 reserved  (filter4)
//;----------------------------------------------------------------
//;   id's CAN_BASE+4064-CAN_BASE+4095 GSI services  (filter5)
//;----------------------------------------------------------------
const int GSI_CAN_ID_GET_SET_SLAVE_ID = GSI_CAN_BASE;     //
//;Master listens on id GSI_CAN_BASE (4064) and will Tx nodes
//; base id on GSI_CAN_BASE
//;s<->m
//;D0: GET/SET id
//;b0=CAN_ID_SET
//;b1=CAN_PC_PRESENT (only used on CAN_ID_SET m->s)

//;-----------------------------------------------------------------
//;ESC and CTR codes that terminal and record based pc->node call
//;Use same offset so that same jmp table can be used
const int GSI_CAN_ID_TERMINAL_COMMAND = GSI_CAN_BASE + 1;
//;m<->s

//;----------------------------------------------------------------
const int GSI_CAN_ID_RS232_MSSG = GSI_CAN_BASE + 2;
const int GSI_CAN_ID_LCD_MSSG = GSI_CAN_BASE + 3;
const int GSI_CAN_ID_KEYPAD_MSSG = GSI_CAN_BASE + 4;
const int GSI_CAN_ID_SPI_MSSG = GSI_CAN_BASE + 5;
const int GSI_CAN_ID_I2C_MSSG = GSI_CAN_BASE + 6;
const int GSI_CAN_ID_DIO_MSSG = GSI_CAN_BASE + 7;
const int GSI_CAN_ID_ADC_MSSG = GSI_CAN_BASE + 8;
const int GSI_CAN_ID_IOCTL_MSSG = GSI_CAN_BASE + 9;
const int GSI_CAN_ID_GSI_IOCTL_MSSG = GSI_CAN_BASE + 10;
//;----------------------------------------------------------------

//----------------------------------------------------------------
//   GSI_CAN_ID_KEYPAD_MSSG subfunctions
//----------------------------------------------------------------

const int GSI_CAN_KD_MSSG_KEY_DN = 0;
const int GSI_CAN_KD_MSSG_KEY_UP = 1;
const int GSI_CAN_KD_MSSG_SET_MODE = 2;
const int GSI_CAN_KD_MSSG_GET_MODE = 3;


//----------------------------------------------------------------
//   CAN_ID_I2C_MSSG subfunctions
//----------------------------------------------------------------
const int CAN_I2C_MSSG_STOP_CONDTION = 0;
const int CAN_I2C_MSSG_KEY_UP = 1;
const int CAN_I2C_MSSG_SET_MODE = 2;
const int CAN_I2C_MSSG_GET_MODE = 3;

//----------------------------------------------------------
//   GSI_CAN_ID_IOCTL_MSSG
//----------------------------------------------------------
const int CAN_IOCTL_GET_NODE = 0;
const int CAN_IOCTL_GET_MODE = 1;
const int CAN_IOCTL_SET_MODE = 2;
const int CAN_IOCTL_GET_NODE_N2M = 3;
const int CAN_IOCTL_GET_MODE_N2M = 4;
const int CAN_IOCTL_SET_MODE_N2M = 5;


const int CAN_MODE_NORMAL = 0;
const int CAN_MODE_DISABLE = 1;
const int CAN_MODE_LOOPBACK = 2;
const int CAN_MODE_LISTEN = 3;
const int CAN_MODE_CONFIGURATION = 4;
const int CAN_MODE_ERROR_RECOGNITION = 5;


//----------------------------------------------------------
//   CAN_ID_GSI_IOCTL_MSSG
//----------------------------------------------------------
const int CAN_GSI_IOCTL_GET_REG = 0;
const int CAN_GSI_IOCTL_SET_REG = 1;
const int CAN_GSI_IOCTL_1 = 2;
const int CAN_GSI_IOCTL_2 = 3;
const int CAN_GSI_IOCTL_3 = 4;
const int CAN_GSI_IOCTL_4 = 5;
const int CAN_GSI_IOCTL_5 = 6;
const int CAN_GSI_IOCTL_6 = 7;
const int CAN_GSI_IOCTL_7 = 8;
const int CAN_GSI_IOCTL_LAST = 8;
//------------------------------------------------------------
//messages going from node->master require separate id's
//otherwise on Rx we Tx the result, which immediately gets
//sent back!
const int CAN_GSI_IOCTL_GET_REG_N2M = CAN_GSI_IOCTL_LAST + 1;
const int CAN_GSI_IOCTL_SET_REG_N2M = CAN_GSI_IOCTL_LAST + 2;
const int CAN_GSI_IOCTL_1_N2M = CAN_GSI_IOCTL_LAST + 3;
const int CAN_GSI_IOCTL_2_N2M = CAN_GSI_IOCTL_LAST + 4;
const int CAN_GSI_IOCTL_3_N2M = CAN_GSI_IOCTL_LAST + 5;
const int CAN_GSI_IOCTL_4_N2M = CAN_GSI_IOCTL_LAST + 6;
const int CAN_GSI_IOCTL_5_N2M = CAN_GSI_IOCTL_LAST + 7;
const int CAN_GSI_IOCTL_6_N2M = CAN_GSI_IOCTL_LAST + 8;
const int CAN_GSI_IOCTL_7_N2M = CAN_GSI_IOCTL_LAST + 9;
#endif //#if 0

#endif //CANDEFS_H_INCLUDED
