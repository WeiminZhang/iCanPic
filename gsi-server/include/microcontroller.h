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
#ifndef MICROCONTROLLER_H_INCLUDED
#define MICROCONTROLLER_H_INCLUDED


enum GSIEnumHandshake
{
    GSI_REPEAT = 53,	//start at non-zero as they are command/handshake bytes for the pic node
    GSI_ACK =	54, 	//53 start is an arbitrary choice
    GSI_ERROR=	55,
    GSI_FATAL=	56
};


/*-----------------------------------------------------------------
  record definitions

  RECORD TYPE    BYTE
  RECORD NODE    BYTE    (0-250) Node values 251-255 reserved for GSI (255 == this)
  RECORD LENGTH  BYTE    (0-8) Data only, doesn't include Type and Node
  RECORD     0-8 bytes
*/
enum GSIEnumRecord
  {
    rtNull=0,
    rtLCDCursBeginningOfLine=  1,
    rtLCDCursLeft=  2, /*goto   GSICursorLeft       ;2 ctr-b*/
    rtGSIUserAbort= 3, /*;goto   GSIUsrAbrt      ;3 ctr-c*/
    rtLCDDelCharFwd= 4,
    rtLCDCursEndofLine=  5, /*;goto   GSIEndOfLine        ;5 ctr-e*/
    rtLCDCursRight=  6, /*;goto   GSICursorRight      ;6 ctr-f*/
    rtGSIBell = 7, /*;goto   GSIBell     ;7 ctr-g*/
    rtLCDBackspace=  8, /*;goto   GSIBS           ;8 ctr-h*/
  rtLCDTab=  9, /*;goto   GSIHTab     ;9 ctr-i*/
  rtLCDCursDown = 10, /*;goto   GSICursorDown       ;10 ctrj*/
  rtLCDCursEndOfLine = 11, /*;goto   GSIEnd          ;11 ctr-k*/
/*;goto   GSICls          ;12 ctr-l*/
    rtLCDCls = 12,
/*;goto   GSICR           ;13 ctr-m*/
    rtLCDNewLine  =13,
/*;goto   GSICursorDown       ;14 ctr-n*/
    rtLCDCursDown1=  14,
/*goto   GSIEchoToggle       ;15 ctr-o*/
    rtRS232EchoToggle = 15,
/*goto   GSICursorUp     ;16 ctr-p*/
    rtLCDCursUp = 16,
/*goto   GSIXOn          ;17 ctr-q*/
    rtRS232XOn = 17,
/*goto   GSIBinaryMode       ;18 ctr-r*/
/*pc->master*/
 rtRS232BinaryMode=18,
//  rtRS232_BINARY_MODE  18
/*goto   GSIXOff     ;19 ctr-s*/
    rtRS232XOff  =19,
/*goto   GSICursorRight5 ;20 ctr-t*/
    rtLCDCursRight5=  20,
/*goto   GSIAsciiMode        ;21 ctr-u*/
/*pc->master*/
    rtRS232AsciiMode = 21,
/*goto   GSICursorLeft5      ;22 ctr-v*/
    rtLCDCursLeft5 = 22,
/*goto   GSICursorLeft10 ;23 ctr-w*/
    rtLCDCursLeft10 = 23,
/*goto   GSICRKeypadToggleState  ;24 ctr-x*/
    rtKeypadToggleState=  24,
/*goto   GSIKeypadToggleMode ;25 ctr-y*/
    rtKeypadToggleMode = 25,
/*goto   GSICtrZ     ;26 ctr-z*/
    rtCtrZ  =26,
    //    rtEND_CTR_CODES  26,
    rtEsc = 27,
/*--------------------------------------------
 *Record struct equivalent to ESC commands
 --------------------------------------------
*/
//these are currently not implemented. The pic node code appears dysfunctional
    //GSI_STArtESC_ID  28
    rtEscVersionString = 28,
    rtESC_CAPABILITIES = 29,
    rtESC_GET_BUFFER = 30,
    rtESC_SET_BUFFER  =31,
    rtESC_LCD_CLS = 32,
    rtESC_LCD_INSTRUCTION = 33,
    rtESC_GET_CUR_TO_EOL = 34,
    rtESC_GET_LINE = 35,
    rtESC_SET_LINE = 36,
    rtESC_HOME = 37,
    rtESC_FLOW_CONTROL_ON = 38,
    rtESC_FLOW_CONTROL_OFF = 39,
    rtESC_INIT = 40,
    rtESC_GET_BUTTON_STATE = 41,
    rtESC_CAN_SET_MASTER = 42,
    rtESC_CAN_SET_SLAVE = 43,
    //rtEND_ESC_CODES  43,
/*Added 6/4/05*/
    //rtSTArtUSER_CODES  44,
    rtUser0=  44,
    rtUser1 = 45,
    rtUser2 = 46,
    rtUser3 = 47,
    rtUser4 = 48,
    //rtEND_USER_CODES  48
    //rtSTArtGSI_CODES  53,
/*-----Repeat, an echo error (at master) or Rx error (at node) occured--------
  pc->master
  ---------------- NOT a RECORD a single byte comand -----------
*/
    //rtGSIRepeat = 53,
/* #define rtITERATOR  rtITERATOR + 1*/
/*;------Acknowledge receipt of data-----
  ;master will suspend processing till ACK is received
  ;pc->master.
  Note that ACK, REPEAT, ERROR, FATAL are not RECORDS but single bytes
  sent immediately after a record to confirm it was received
  ---------------- NOT a RECORD a single byte comand -----------
*/
    //rtGSIAck = 54,
/*-----ERROR*/
/*pc<->master*/
    //rtGSIError=  55,
/*;-----FATAL ERROR*/
/*;pc<->master*/
    //rtGSIFatal = 56,
/*;-----RESERVED1*/
/*;pc<->master*/
    rtGSIReserved1 = 57,
/*-----RESERVED2*/
/*pc<->master*/
    rtGSIReserved2 = 58,
/* #define rtITERATOR  rtITERATOR + 1*/
/*;-----Send a Message to an RS232 module on a node------------------------------*/
    rtGSIRS232Message = 59,
/*Data[0] values*/
//  GSIRS232MESSAGE_SUCCESS  0
//  GSIRS232MESSAGE_FAILURE  1
//  GSIRS232MESSAGE_REQ_SLAVE  2
//  GSIRS232MESSAGE_REQ_MASTER  3 /*not used, use GSIRS232MESSAGE_MASTER_REQ_BIT*/
//  GSIRS232MESSAGE_SEND_DATA  4
//  GSIRS232MESSAGE_GET_DATA  5
//  GSIRS232MESSAGE_n1  6
//  GSIRS232MESSAGE_n2  7
//  GSIRS232MESSAGE_n3  8

/*Also OR in or AND out this bit*/
//  GSIRS232MESSAGE_MASTER_REQ_BIT  7

/*rtGSIRS232Message           EQU rtITERATOR*/
/*RLRS232Message               EQU 5   ;4 bytes the node, 1 byte the Message*/
/*rtITERATOR             SET rtITERATOR+1*/
/*;Data position 0 equates*/
/*;Set record.data[0] to these for the specific function*/
/*;data bytes 1-R2_MAX_DATA_SIZE then hold other data*/
/*;As R2Records there can be > 8 data bytes*/
/*GSIRS232MESSAGE_SUCCESS   EQU 0*/
/*GSIRS232MESSAGE_FAILURE   EQU 1*/

/*;If receive REQ_SLAVE, then pc has allowed this node to be*/
/*;master. If sent, then this node has allowed pc to be master
  GSIRS232MESSAGE_REQ_SLAVE EQU 2
  GSIRS232MESSAGE_SEND_DATA EQU 3
  GSIRS232MESSAGE_GET_DATA  EQU 4
  GSIRS232MESSAGE_n1        EQU 5
  GSIRS232MESSAGE_n2        EQU 6
  GSIRS232MESSAGE_n3        EQU 7

  ;When returning success or failure can also OR in this bit
  ;to data[0]. If set, then PC knows we wish to be master

  *GSIRS232MESSAGE_MASTER_REQ_BIT EQU    7


  #define rtITERATOR  rtITERATOR + 1

  ;-----Send a Message to an LCD on a node------------------------------
*/
    rtGSIRS232BreakMessage=60,
/*
        case GSIRS232BreakPOReset: 1      //Power on reset detected
        case GSIRS232BreakBOReset:  2     //Brown out reset
        case GSIRS232BreakSOVFReset: 3        //Stack Overflow
        case GSIRS232BreakSUFReset:   4    //Stack underflow
        case GSIRS232BreakWDTReset:   5     //WDT reset
        case GSIRS232BreakUnknownReset: 6      //Unknown reset
        case GSIRS232BreakUserAbortReset: 7
        case GSIRS232BreakAsyncRecord:    8  //Asyncronous record (microcontroller->pc) request
*/
    rtGSILCDMessage = 61,
/*Data position 0 equates*/
/*Set record.data[0] to these for the specific function*/
/*data bytes 1-7 then hold other data*/
//  GSI_LCD_MESSAGE_INSTRUCTION  0
//  GSI_LCD_MESSAGE_WRITE_STRING  1
//  GSI_LCD_MESSAGE_WRITE_CHAR  2
//  GSI_LCD_MESSAGE_GET_STRING  3
//  GSI_LCD_MESSAGE_CTR_JMP_TBL  4
//  GSI_LCD_MESSAGE_GET_CAPABILITIES
/* #define rtITERATOR  rtITERATOR + 1*/

/*;-----Send a Message to an LCD on a node------------------------------*/
    rtGSIKeypadMessage = 62,
/* #define rtITERATOR  rtITERATOR + 1*/

/*;-----Send a Message to an SPI module on a node------------------------------*/
    rtGSISPIMessage = 63,
/* #define rtITERATOR  rtITERATOR + 1*/

/*;-----Send a Message to an I2C module on a node------------------------------*/
    rtGSII2CMessage = 64,
/* #define rtITERATOR  rtITERATOR + 1*/

/*;-----Send a Message to a dio on a node------------------------------*/
    rtGSIDIOMessage  =65,
/* #define rtITERATOR  rtITERATOR + 1*/

/*;-----Send a Message to a dio on a node------------------------------*/
    rtGSIADCMessage = 66,
/* #define rtITERATOR  rtITERATOR + 1*/

/*;-----------------------------------
  ;   RS232 CAN Records
  ;-----------------------------------

  ;---Tell node it is the master controlled by a pc
  ;pc->master
*/

    rtGSICANPCMaster = 67,
/* #define rtITERATOR  rtITERATOR + 1*/

/*;---Tell node it is the stand alone master*/
/*;pc->master*/
  rtGSICANMaster = 68,           /*As seen from PIC Master node*/
/* #define rtITERATOR  rtITERATOR + 1*/

/*;---Send a message down the CAN Message is in RECORD as a correctly ;formatted CAN MESSAGE
  ;pc->master
*/
  rtGSICANSendMessage = 69,     /*As seen from PIC Master node*/
/* #define rtITERATOR  rtITERATOR + 1*/


/*;----CAN master has encountered a CAN error the pc needs to know about
  ;master->pc*/
    rtGSICANError = 70,
/* #define rtITERATOR  rtITERATOR + 1*/



/*;--A remote node has rested an id ( #define rtTX_CANMessage will send the ID)
  ;master->pc
*/
    rtGSICANSetSlaveIDReq=  71,
/* #define rtITERATOR  rtITERATOR + 1*/

/*--rs232 is attached to a slave. Tell slave to send a rest to the master
  for an id. ivalent to holding 1 or switch at power up
  pc->slave
  ERRORLEVEL -208
*/
    rtGSICANSlaveGetSlaveIDReq = 72,
/*IOCTL control CAN
  Use generic CAN
*/
  rtGSICANIOCTLMessage = 73, /*;72*/
//  subfunctions
//const GSIChar CANIOCTLGetNode = 0;
//onst GSIChar CANIOCTLGetMode = 1;
//const GSIChar CANIOCTLSetMode = 2;
//const GSIChar CANIOCTLGetNodeNode2Master = 3;
//const GSIChar CANIOCTLGetModeNode2Master = 4;
//const GSIChar CANIOCTLSetModeNode2Master = 5;

  rtGSICANGSIIOCTLMessage = 74,


rtGSICANPingMessage         =75,
rtGSICANReserved1           =76,
rtGSICANReserved2           =77,
rtGSICANReserved3           =78,
rtGSICANReserved4           =79,
rtGSICANReserved5           =80,


/****************************************************
      RS232 messages
*****************************************************/
/*
The sender wants recipient to be RS232 MASTER
  The receiver must become MASTER if possible
*/
    rtRS232SetMaster = 81,
/*The sender wants to be MASTER
  the receiver must become SLAVE if poss
*/
    rtRS232ReqMaster = 82,
/*       Node -> pc
	 These codes are for node->pc transmission
	 _NP: NODE-> PC
	 -------------------------------------------------------------
	 rtNODE_TO_PC_CODES             EQU rtITERATOR
	 ----CAN Master has received a message that the pc needs to see
	 node->pc
*/
    rtStartNode2PCCodes = 83,
    rtGSIR2RxCANMessageNode2PC=  83,
    //rtEND_GSI_CODES  76,
    //rtEND_RECORD_CODES  rtEND_GSI_CODES
  };


//    rtGSIRS232Message values
const unsigned char GSIRS232MessageSuccess=0;
const unsigned char GSIRS232MessageFailure=  1;
const unsigned char  GSIRS232MessageReqSlave=  2;
const unsigned char  GSIRS232MessageReqMaster=  3; /*not used, use GSIRS232MessageMASTER_REQ_BIT*/
const unsigned char GSIRS232MessageSendData=  4;
const unsigned char GSIRS232MessageGetData=  5;
const unsigned char GSIRS232MessageChangeNode=  6;	//master has probably registered itself (node changes from 255:noNetwork, to :master)
const unsigned char GSIRS232MessageNewNode = 7;		//slave has probably registered itself to the master we are attached to.
const unsigned char GSIRS232MessageN3 = 8;


//rtGSIRS232BreakMessage subfunction values
const GSIChar GSIRS232BreakPOReset      =1;      //Power on reset detected
const GSIChar GSIRS232BreakBOReset      =2;     //Brown out reset
const GSIChar GSIRS232BreakSOVFReset    =3;        //Stack Overflow
const GSIChar GSIRS232BreakSUFReset     =4;    //Stack underflow
const GSIChar GSIRS232BreakWDTReset     =5;     //WDT reset
const GSIChar GSIRS232BreakUnknownReset =6;      //Unknown reset
const GSIChar GSIRS232BreakUserAbortReset=7;
const GSIChar GSIRS232BreakAsyncRecord   =8;  //Asyncronous record (microcontroller->pc) request

#endif
