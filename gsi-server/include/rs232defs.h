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
#ifndef RS232DEFS_H_INCLUDED
#define RS232DEFS_H_INCLUDED

#include "gsidefs.h"

//ComPort
const int GSIRS232DataBits=8;
const int GSIRS232StopBits=1;
const int GSIRS232Parity=0;



//#define GSIRS232DATA_BITS 8
//#define GSIRS232STOP_BITS 1
//#define GSIRS232PARITY 0

//mc will send us a BREAK (Tx held high >1 char) on resets or when it wishes our attention (it has data to send etc.)
enum GSIBreak
{
GSI_BREAK_RESET_WDT=1,
GSI_BREAK_RESET_BOR,
GSI_BREAK_RESET_UNKNOWN,
GSI_BREAK_RESET_USER_ABORT,
GSI_BREAK_RESET_POR,
GSI_BREAK_RESET_STACK_OVF,
GSI_BREAK_RESET_STACK_UNF,
GSI_BREAK_RESET_FATAL,         //currently used if we find a timeout while waiting for reset type id after a BREAK recvd from mc.
GSI_BREAK_SEND_RECORD=20,      //pc receives a record, immediately returns to be commsMaster
GSI_BREAK_SEND_MULTIPLE_RECORDS,		//more than one rec, pc stays slave till receives rs232 msg req_slave
GSI_BREAK_SEND_DATA			//send arbitrary data (like adc) protocol yet to be determined

};






#if 0
/*------------ Strings associated with above -------------*/
  LCD_BEGINING_OF_LINE_STR  "LCD Begining of line"
 LCD_CURS_LEFT_STR  "LCD Cursor Left"   /*goto   GSICursorLeft       ;2 ctr-b*/
 GSI_USER_ABOrtSTR  "GSI User Abort (Ctr-c)"  /*;goto   GSIUsrAbrt      ;3 ctr-c"*/
 LCD_DEL_CHAR_FWD_STR  "LCD Delete fwd"    /*;goto   GSIDeleteCharFwd    ;4 ctr-d"*/
 LCD_END_OF_LINE_STR  "LCD end of line" /*;goto   GSIEndOfLine        ;5 ctr-e"*/
 LCD_CURS_RIGHT_STR  "LCD Cursor right" /*;goto   GSICursorRight      ;6 ctr-f"*/
 GSI_BELL_STR  "GSI Bell"   /*;goto   GSIBell     ;7 ctr-g*/*/
 LCD_BS_STR  "LCD Backspace" /*;goto   GSIBS           ;8 ctr-h""*/
 LCD_TAB_STR  "LCD Tab"   /*;goto   GSIHTab     ;9 ctr-i""*/
 LCD_CURS_DOWN_STR  "LCD Cursor down" /*;goto   GSICursorDown       ;10 a duplicate ctrj""*/
 LCD_CURS_END_STR  "LCD Cursor end"   /*;goto   GSIEnd          ;11 ctr-k""*/
 LCD_CLS_STR  "LCD Cls"  /*;goto   GSICls          ;12 ctr-l""*/
 LCD_CR_STR  "LCD <CR>"  /*;goto   GSICR           ;13 ctr-m""*/
 LCD_CURS_DOWN1_STR  "LCD Cursor Down"  /*;goto   GSICursorDown       ;14 ctr-n""*/
 RS232_ECHO_TOGGLE_STR  "RS232 Toggle Echo"  /*goto   GSIEchoToggle       ;15 ctr-o""*/
 LCD_CURS_UP_STR  "LCD Cursor up"  /*goto   GSICursorUp     ;16 ctr-p""*/
 RS232_XON_STR  "RS232 XOn"  /*goto   GSIXOn          ;17 ctr-q""*/
/*pc->master*/
 RS232_BINARY_MODE_STR  "RS232 Set binary mode"  /*goto   GSIBinaryMode       ;18 ctr-r""*/
 RS232_XOFF_STR  "RS232 XOff"  /*goto   GSIXOff     ;19 ctr-s""*/

 LCD_CURS_RIGHT_5_STR  "LCD Cursor right 5"  /*goto   GSICursorRight5 ;20 ctr-t""*/
/*pc->master*/
 RS232_ASCII_MODE_STR  "RS232 Ascii mode"  /*goto   GSIAsciiMode        ;21 ctr-u""*/
 LCD_CURS_LEFT_5_STR  "LCD Cursor left 5"  /*goto   GSICursorLeft5      ;22 ctr-v""*/
 LCD_CURS_LEFT_10_STR  "LCD Cursor left 10"  /*goto   GSICursorLeft10 ;23 ctr-w""*/
 KEYPAD_TOGGLE_STATE_STR  "KEYPAD Toggle state "  /*goto   GSICRKeypadToggleState  ;24 ctr-x""*/
 KEYPAD_TOGGLE_MODE_STR  "KEYPAD Toggle mode"  /*goto   GSIKeypadToggleMode ;25 ctr-y""*/

/*--------------------------------------------*/
/*Record struct equivalent to ESC commands*/
/*--------------------------------------------*/
 STArtESC_CODES_STR  ""

 GSI_VERSION_STRING_STR  "GSI Get version"
 GSI_CAPABILITIES_STR  "GSI Get capabilities"
 LCD_GET_BUFFER_STR  "LCD Get buffer"
 LCD_SET_BUFFER_STR  "LCD Set buffer"
/* #define LCD_CLS_STR  LCD_CLS_STR*/
 LCD_INSTRUCTION_STR  "LCD Instruction"
 LCD_GET_TO_EOL_STR  "LCD Get to end of line"
 LCD_GET_LINE_STR  "LCD Get line"
 LCD_SET_LINE_STR  "LCD Set line"
 LCD_HOME_STR  "LCD Home"
 RS232_FLOW_CONTROL_ON_STR  "RS232 Flow control on"
 RS232_FLOW_CONTROL_OFF_STR  "RS232 Flow control off"
 GSI_INIT_STR  "GSI Init"
 KEYPAD_GET_BUTTON_STATE_STR  "KEYPAD Get button state"
 CAN_SET_MASTER_STR  "CAN Set master"
 CAN_SET_SLAVE_STR  "CAN Set slave"

#endif



#if 0
#define GSIRS232REPEAT_COUNT 4 /*repeat a failed echo*/
#define GSIRS232DEFAULT_TIMEOUT  1  /*1 sec using timer()*/
#define rtFIRST_CODE  1
#define rtSTArtCTR_CODES  1
#define GSIRS232NULL  0
#define LCD_BEGINING_OF_LINE  1
#define LCD_CURS_LEFT  2    /*goto   GSICursorLeft       ;2 ctr-b*/
#define GSI_USER_ABORT  3  /*;goto   GSIUsrAbrt      ;3 ctr-c*/
#define LCD_DEL_CHAR_FWD  4    /*;goto   GSIDeleteCharFwd    ;4 ctr-d*/
#define LCD_END_OF_LINE  5    /*;goto   GSIEndOfLine        ;5 ctr-e*/
#define LCD_CURS_RIGHT  6   /*;goto   GSICursorRight      ;6 ctr-f*/
#define GSI_BELL  7   /*;goto   GSIBell     ;7 ctr-g*/
#define LCD_BS  8 /*;goto   GSIBS           ;8 ctr-h*/
#define LCD_TAB  9   /*;goto   GSIHTab     ;9 ctr-i*/
#define LCD_CURS_DOWN  10 /*;goto   GSICursorDown       ;10 a duplicate ctrj*/
#define LCD_CURS_END  11   /*;goto   GSIEnd          ;11 ctr-k*/
#define LCD_CLS  12 /*;goto   GSICls          ;12 ctr-l*/
#define LCD_CR  13 /*;goto   GSICR           ;13 ctr-m*/
#define LCD_CURS_DOWN1  14 /*;goto   GSICursorDown       ;14 ctr-n*/
#define RS232_ECHO_TOGGLE  15 /*goto   GSIEchoToggle       ;15 ctr-o*/
#define LCD_CURS_UP  16 /*goto   GSICursorUp     ;16 ctr-p*/
#define RS232_XON  17 /*goto   GSIXOn          ;17 ctr-q*/
/*pc->master*/
#define RS232_BINARY_MODE  18 /*goto   GSIBinaryMode       ;18 ctr-r*/
#define RS232_XOFF  19 /*goto   GSIXOff     ;19 ctr-s*/

#define LCD_CURS_RIGHT_5  20 /*goto   GSICursorRight5 ;20 ctr-t*/
/*pc->master*/
#define RS232_ASCII_MODE  21 /*goto   GSIAsciiMode        ;21 ctr-u*/
#define LCD_CURS_LEFT_5  22 /*goto   GSICursorLeft5      ;22 ctr-v*/
#define LCD_CURS_LEFT_10  23 /*goto   GSICursorLeft10 ;23 ctr-w*/
#define KEYPAD_TOGGLE_STATE  24 /*goto   GSICRKeypadToggleState  ;24 ctr-x*/
#define KEYPAD_TOGGLE_MODE  25 /*goto   GSIKeypadToggleMode ;25 ctr-y*/
#define CTR_Z  26

/* #define rtEND_CTR_CODES  26 see later*/

/* #define ESC_CODE  27 see later*/

/*--------------------------------------------
  Record struct equivalent to ESC commands
  --------------------------------------------
*/
#define rtSTArtESC_CODES  28

#define ESC_GSI_VERSION_STRING  28
#define ESC_GSI_CAPABILITIES  29
#define ESC_LCD_GET_BUFFER  30
#define ESC_LCD_SET_BUFFER  31
#define ESC_LCD_CLS  32
#define ESC_LCD_INSTRUCTION  33
#define ESC_LCD_GET_CUR_TO_EOL  34
#define ESC_LCD_GET_LINE  35
#define ESC_LCD_SET_LINE  36
#define ESC_LCD_HOME  37
#define ESCRS232FLOW_CONTROL_ON  38
#define ESCRS232FLOW_CONTROL_OFF  39
#define ESC_GSI_INIT  40
#define ESC_KEYPAD_GET_BUTTON_STATE  41
#define ESC_CAN_SET_MASTER  42
#define ESC_CAN_SET_SLAVE  43

/* #define rtEND_ESC_CODES  43 see later*/

/*-----------------------------------------------------------------
  record definitions

  RECORD TYPE    BYTE
  RECORD NODE    BYTE    (0-250) Node values 251-255 reserved for GSI (255 == this)
  RECORD LENGTH  BYTE    (0-8) Data only, doesn't include Type and Node
  RECORD     0-8 bytes
*/

#define rtLCD_BEGINING_OF_LINE  1
#define RL_LCD_BEGINING_OF_LINE  0

#define rtLCD_CURS_LEFT  2 /*goto   GSICursorLeft       ;2 ctr-b*/
#define RL_LCD_CURS_LEFT  0

#define rtGSI_USER_ABORT  3 /*;goto   GSIUsrAbrt      ;3 ctr-c*/
#define RL_GSI_USER_ABORT  0


#define rtLCD_DEL_CHAR_FWD  4 /*;goto   GSIDeleteCharFwd    ;4 ctr-d*/

#define RL_LCD_DEL_CHAR_FWD  0


#define rtLCD_END_OF_LINE  5 /*;goto   GSIEndOfLine        ;5 ctr-e*/
#define RL_LCD_END_OF_LINE  0


#define rtLCD_CURS_RIGHT  6 /*;goto   GSICursorRight      ;6 ctr-f*/
#define RL_LCD_CURS_RIGHT  0


#define rtGSI_BELL  7 /*;goto   GSIBell     ;7 ctr-g*/
#define RL_GSI_BELL  0


#define rtLCD_BS  8 /*;goto   GSIBS           ;8 ctr-h*/
#define RL_LCD_BS  0


#define rtLCD_TAB  9 /*;goto   GSIHTab     ;9 ctr-i*/
#define RL_LCD_TAB  0


#define rtLCD_CURS_DOWN  10 /*;goto   GSICursorDown       ;10 ctrj*/
#define RL_LCD_CURS_DOWN  0


#define rtLCD_CURS_END  11 /*;goto   GSIEnd          ;11 ctr-k*/
#define RL_LCD_CURS_END  0

/*;goto   GSICls          ;12 ctr-l*/
#define rtLCD_CLS  12
#define RL_LCD_CLS  0

/*;goto   GSICR           ;13 ctr-m*/
#define rtLCD_CR  13
#define RL_LCD_CR  0

/*;goto   GSICursorDown       ;14 ctr-n*/
#define rtLCD_CURS_DOWN1  14
#define RL_LCD_CURS_DOWN1  0

/*goto   GSIEchoToggle       ;15 ctr-o*/
#define rtRS232_ECHO_TOGGLE  15
#define RLRS232ECHO_TOGGLE  0

/*goto   GSICursorUp     ;16 ctr-p*/
#define rtLCD_CURS_UP  16
#define RL_LCD_CURS_UP  0

/*goto   GSIXOn          ;17 ctr-q*/
#define rtRS232_XON  17
#define RLRS232XON  0

/*goto   GSIBinaryMode       ;18 ctr-r*/
/*pc->master*/
 rtRS232BinaryMode=18;
//#define rtRS232_BINARY_MODE  18
#define RLRS232BINARY_MODE  0

/*goto   GSIXOff     ;19 ctr-s*/
#define rtRS232_XOFF  19
#define RLRS232XOFF  0

/*goto   GSICursorRight5 ;20 ctr-t*/
#define rtLCD_CURS_RIGHT_5  20
#define RL_LCD_CURS_RIGHT_5  0

/*goto   GSIAsciiMode        ;21 ctr-u*/
/*pc->master*/
#define rtRS232_ASCII_MODE  21
#define RLRS232ASCII_MODE  0

/*goto   GSICursorLeft5      ;22 ctr-v*/
#define rtLCD_CURS_LEFT_5  22
#define RL_LCD_CURS_LEFT_5  0

/*goto   GSICursorLeft10 ;23 ctr-w*/
#define rtLCD_CURS_LEFT_10  23
#define RL_LCD_CURS_LEFT_10  0

/*goto   GSICRKeypadToggleState  ;24 ctr-x*/
#define rtKEYPAD_TOGGLE_STATE  24
#define RL_KEYPAD_TOGGLE_STATE  0

/*goto   GSIKeypadToggleMode ;25 ctr-y*/
#define rtKEYPAD_TOGGLE_MODE  25
#define RL_KEYPAD_TOGGLE_MODE  0

/*goto   GSICtrZ     ;26 ctr-z*/
#define rtCTR_Z  26
#define RL_CTR_Z  0
#define rtEND_CTR_CODES  26

#define rtESC_ESC  27
#define RL_ESC_ESC  0

/*--------------------------------------------
 *Record struct equivalent to ESC commands
 --------------------------------------------
*/
#define GSI_STArtESC_ID  28


#define rtESC_VERSION_STRING  28
#define RL_ESC_VERSION_STRING  0

#define rtESC_CAPABILITIES  29
#define RL_ESC_CAPABILITIES  0

#define rtESC_GET_BUFFER  30
#define RL_ESC_GET_BUFFER  0

#define rtESC_SET_BUFFER  31
#define RL_ESC_SET_BUFFER  0

#define rtESC_LCD_CLS  32
#define RL_ESC_LCD_CLS  0

#define rtESC_LCD_INSTRUCTION  33
#define RL_ESC_LCD_INSTRUCTION  0

#define rtESC_GET_CUR_TO_EOL  34
#define RL_ESC_GET_CUR_TO_EOL  0

#define rtESC_GET_LINE  35
#define RL_ESC_GET_LINE  0

#define rtESC_SET_LINE  36
#define RL_ESC_SET_LINE  0

#define rtESC_HOME  37
#define RL_ESC_HOME  0

#define rtESC_FLOW_CONTROL_ON  38
#define RL_ESC_FLOW_CONTROL_ON  0

#define rtESC_FLOW_CONTROL_OFF  39
#define RL_ESC_FLOW_CONTROL_OFF  0

#define rtESC_INIT  40
#define RL_ESC_INIT  0

#define rtESC_GET_BUTTON_STATE  41
#define RL_ESC_GET_BUTTON_STATE  0

#define rtESC_CAN_SET_MASTER  42
#define RL_ESC_CAN_SET_MASTER  0

#define rtESC_CAN_SET_SLAVE  43
#define RL_ESC_CAN_SET_SLAVE  0

#define rtEND_ESC_CODES  43


/*Added 6/4/05*/
#define rtSTArtUSER_CODES  44

#define rtUSER_0  44
#define RL_USER_0  0

#define rtUSER_1  45
#define RL_USER_1  0

#define rtUSER_2  46
#define RL_USER_2  0

#define rtUSER_3  47
#define RL_USER_3  0

#define rtUSER_4  48
#define RL_USER_4  0

#define rtEND_USER_CODES  48



#define rtSTArtGSI_CODES  53


/*-----Repeat, an echo error (at master) or Rx error (at node) occured--------
  pc->master
  ---------------- NOT a RECORD a single byte comand -----------
*/
#define rtGSI_REPEAT  53
#define RL_GSI_REPEAT  0

/* #define rtITERATOR  rtITERATOR + 1*/

/*;------Acknowledge receipt of data-----
  ;master will suspend processing till ACK is received
  ;pc->master.
  Note that ACK and REPEAT are not RECORDS but single bytes
  sent immediately after a record to confirm it was received
  ---------------- NOT a RECORD a single byte comand -----------
*/
#define rtGSI_ACK  54
#define RL_GSI_ACK  0

/*-----ERROR*/
/*pc<->master*/
#define rtGSI_ERROR  55
#define RL_GSI_ERROR  0

/*;-----FATAL ERROR*/
/*;pc<->master*/
#define rtGSI_FATAL  56
#define RL_GSI_FATAL  0

/*;-----RESERVED1*/
/*;pc<->master*/
#define rtGSI_RESERVED_1  57
#define RL_GSI_RESERVED_1  0

/*-----RESERVED2*/
/*pc<->master*/
#define rtGSI_RESERVED_2  58
#define RL_GSI_RESERVED_2  0

/* #define rtITERATOR  rtITERATOR + 1*/

/*;-----Send a Message to an RS232 module on a node------------------------------*/
#define rtGSIRS232Message  59
#define RL_GSIRS232Message  5    /*;1 byte the MessageType variable data length*/
/*Data[0] values*/

#define GSIRS232MESSAGE_SUCCESS  0
#define GSIRS232MESSAGE_FAILURE  1
#define GSIRS232MESSAGE_REQ_SLAVE  2
#define GSIRS232MESSAGE_REQ_MASTER  3 /*not used, use GSIRS232MESSAGE_MASTER_REQ_BIT*/
#define GSIRS232MESSAGE_SEND_DATA  4
#define GSIRS232MESSAGE_GET_DATA  5
#define GSIRS232MESSAGE_n1  6
#define GSIRS232MESSAGE_n2  7
#define GSIRS232MESSAGE_n3  8

/*Also OR in or AND out this bit*/
#define GSIRS232MESSAGE_MASTER_REQ_BIT  7

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

#define rtGSI_LCDMessage  60
#define RL_GSI_LCDMessage  6          /*;4 bytes the node, 1 byte the Message, 1 byte the Message subtype*/
/*Data position 0 equates*/
/*Set record.data[0] to these for the specific function*/
/*data bytes 1-7 then hold other data*/
#define GSI_LCD_MESSAGE_INSTRUCTION  0
#define GSI_LCD_MESSAGE_WRITE_STRING  1
#define GSI_LCD_MESSAGE_WRITE_CHAR  2
#define GSI_LCD_MESSAGE_GET_STRING  3
#define GSI_LCD_MESSAGE_CTR_JMP_TBL  4
/* #define rtITERATOR  rtITERATOR + 1*/

/*;-----Send a Message to an LCD on a node------------------------------*/
#define rtGSI_KEYPADMessage  61
#define RL_GSI_KEYPADMessage  5           /*;4 bytes the node, 1 byte the Message*/
/* #define rtITERATOR  rtITERATOR + 1*/

/*;-----Send a Message to an SPI module on a node------------------------------*/
#define rtGSI_SPIMessage  62
#define RL_GSI_SPIMessage  6           /*4 bytes the node, 1 byte the address,1 byte the Message,*/
/* #define rtITERATOR  rtITERATOR + 1*/


/*;-----Send a Message to an I2C module on a node------------------------------*/
#define rtGSI_I2CMessage  63
#define RL_GSI_I2CMessage  5           /*4 bytes the node, 1 byte the Message*/
/* #define rtITERATOR  rtITERATOR + 1*/

/*;-----Send a Message to a dio on a node------------------------------*/
#define rtGSI_DIOMessage  64
#define RL_GSI_DIOMessage  6           /*;4 bytes the node, 1 byte the dio,1 byte the Message*/
/* #define rtITERATOR  rtITERATOR + 1*/

/*;-----Send a Message to a dio on a node------------------------------*/
#define rtGSI_ADCMessage  65
#define RL_GSI_ADCMessage  6           /*4 bytes the node, 1 byte the adc,1 byte the Message*/
/* #define rtITERATOR  rtITERATOR + 1*/

/*;-----------------------------------
  ;   RS232 CAN Records
  ;-----------------------------------

  ;---Tell node it is the master controlled by a pc
  ;pc->master
*/

#define rtGSI_CAN_PC_MASTER  66
#define RL_GSI_CAN_PC_MASTER  4                      /*Node id in case more than one pc master*/
/* #define rtITERATOR  rtITERATOR + 1*/

/*;---Tell node it is the stand alone master*/
/*;pc->master*/
#define rtGSI_CAN_MASTER  67           /*As seen from PIC Master node*/
#define RL_GSI_CAN_MASTER  0
/* #define rtITERATOR  rtITERATOR + 1*/

/*;---Send a message down the CAN Message is in RECORD as a correctly ;formatted CAN MESSAGE
  ;pc->master
*/
#define rtGSI_CAN_SENDMessage  68     /*As seen from PIC Master node*/
#define RL_GSI_CAN_SENDMessage  0
/* #define rtITERATOR  rtITERATOR + 1*/


/*;----CAN master has encountered a CAN error the pc needs to know about
  ;master->pc*/
#define rtGSI_CAN_ERROR  69
#define RL_GSI_CAN_ERROR  0
/* #define rtITERATOR  rtITERATOR + 1*/



/*;--A remote node has rested an id ( #define rtTX_CANMessage will send the ID)
  ;master->pc
*/
#define rtGSI_CAN_SET_SLAVE_ID_REQ  70
#define RL_GSI_CAN_SET_SLAVE_ID_REQ  5     /*Flag +SLAVE_BASE*/
/* #define rtITERATOR  rtITERATOR + 1*/

/*--rs232 is attached to a slave. Tell slave to send a rest to the master
  for an id. ivalent to holding 1 or switch at power up
  pc->slave
  ERRORLEVEL -208
*/
#define rtGSI_CAN_SLAVE_GET_SLAVE_ID_REQ  71
#define RL_GSI_CAN_SLAVE_GET_SLAVE_ID_REQ  0

/*IOCTL control CAN
  Use generic CAN
*/

#define rtGSI_CAN_IOCTLMessage  72 /*;72*/
#define RL_GSI_CAN_IOCTLMessage  0



#define rtGSI_CAN_GSI_IOCTLMessage  73
#define RL_GSI_CAN_GSI_IOCTLMessage  0


/*The sender wants recipient to be RS232 MASTER
  The receiver must become MASTER if possible
*/
#define rtRS232_SET_MASTER  74
#define RLRS232SET_MASTER  0

/*The sender wants to be MASTER
  the receiver must become SLAVE if poss
*/
#define rtRS232_REQ_MASTER  75
#define RLRS232REQ_MASTER  0

/*       Node -> pc
	 These codes are for node->pc transmission
	 _NP: NODE-> PC
	 -------------------------------------------------------------
	 rtNODE_TO_PC_CODES             EQU rtITERATOR
	 ----CAN Master has received a message that the pc needs to see
	 node->pc
*/
#define rtSTArtNP_CODES  76

#define rtGSI_R2_RX_CANMessage_NP  76
#define RL_GSI_R2_RX_CANMessage_NP  0

#define rtEND_GSI_CODES  76
#define rtEND_RECORD_CODES  rtEND_GSI_CODES



/*------------ Strings associated with above -------------*/
#define LCD_BEGINING_OF_LINE_STR  "LCD Begining of line"
#define LCD_CURS_LEFT_STR  "LCD Cursor Left"   /*goto   GSICursorLeft       ;2 ctr-b*/
#define GSI_USER_ABOrtSTR  "GSI User Abort (Ctr-c)"  /*;goto   GSIUsrAbrt      ;3 ctr-c"*/
#define LCD_DEL_CHAR_FWD_STR  "LCD Delete fwd"    /*;goto   GSIDeleteCharFwd    ;4 ctr-d"*/
#define LCD_END_OF_LINE_STR  "LCD end of line" /*;goto   GSIEndOfLine        ;5 ctr-e"*/
#define LCD_CURS_RIGHT_STR  "LCD Cursor right" /*;goto   GSICursorRight      ;6 ctr-f"*/
#define GSI_BELL_STR  "GSI Bell"   /*;goto   GSIBell     ;7 ctr-g*/*/
#define LCD_BS_STR  "LCD Backspace" /*;goto   GSIBS           ;8 ctr-h""*/
#define LCD_TAB_STR  "LCD Tab"   /*;goto   GSIHTab     ;9 ctr-i""*/
#define LCD_CURS_DOWN_STR  "LCD Cursor down" /*;goto   GSICursorDown       ;10 a duplicate ctrj""*/
#define LCD_CURS_END_STR  "LCD Cursor end"   /*;goto   GSIEnd          ;11 ctr-k""*/
#define LCD_CLS_STR  "LCD Cls"  /*;goto   GSICls          ;12 ctr-l""*/
#define LCD_CR_STR  "LCD <CR>"  /*;goto   GSICR           ;13 ctr-m""*/
#define LCD_CURS_DOWN1_STR  "LCD Cursor Down"  /*;goto   GSICursorDown       ;14 ctr-n""*/
#define RS232_ECHO_TOGGLE_STR  "RS232 Toggle Echo"  /*goto   GSIEchoToggle       ;15 ctr-o""*/
#define LCD_CURS_UP_STR  "LCD Cursor up"  /*goto   GSICursorUp     ;16 ctr-p""*/
#define RS232_XON_STR  "RS232 XOn"  /*goto   GSIXOn          ;17 ctr-q""*/
/*pc->master*/
#define RS232_BINARY_MODE_STR  "RS232 Set binary mode"  /*goto   GSIBinaryMode       ;18 ctr-r""*/
#define RS232_XOFF_STR  "RS232 XOff"  /*goto   GSIXOff     ;19 ctr-s""*/

#define LCD_CURS_RIGHT_5_STR  "LCD Cursor right 5"  /*goto   GSICursorRight5 ;20 ctr-t""*/
/*pc->master*/
#define RS232_ASCII_MODE_STR  "RS232 Ascii mode"  /*goto   GSIAsciiMode        ;21 ctr-u""*/
#define LCD_CURS_LEFT_5_STR  "LCD Cursor left 5"  /*goto   GSICursorLeft5      ;22 ctr-v""*/
#define LCD_CURS_LEFT_10_STR  "LCD Cursor left 10"  /*goto   GSICursorLeft10 ;23 ctr-w""*/
#define KEYPAD_TOGGLE_STATE_STR  "KEYPAD Toggle state "  /*goto   GSICRKeypadToggleState  ;24 ctr-x""*/
#define KEYPAD_TOGGLE_MODE_STR  "KEYPAD Toggle mode"  /*goto   GSIKeypadToggleMode ;25 ctr-y""*/

/*--------------------------------------------*/
/*Record struct equivalent to ESC commands*/
/*--------------------------------------------*/
#define STArtESC_CODES_STR  ""

#define GSI_VERSION_STRING_STR  "GSI Get version"
#define GSI_CAPABILITIES_STR  "GSI Get capabilities"
#define LCD_GET_BUFFER_STR  "LCD Get buffer"
#define LCD_SET_BUFFER_STR  "LCD Set buffer"
/* #define LCD_CLS_STR  LCD_CLS_STR*/
#define LCD_INSTRUCTION_STR  "LCD Instruction"
#define LCD_GET_TO_EOL_STR  "LCD Get to end of line"
#define LCD_GET_LINE_STR  "LCD Get line"
#define LCD_SET_LINE_STR  "LCD Set line"
#define LCD_HOME_STR  "LCD Home"
#define RS232_FLOW_CONTROL_ON_STR  "RS232 Flow control on"
#define RS232_FLOW_CONTROL_OFF_STR  "RS232 Flow control off"
#define GSI_INIT_STR  "GSI Init"
#define KEYPAD_GET_BUTTON_STATE_STR  "KEYPAD Get button state"
#define CAN_SET_MASTER_STR  "CAN Set master"
#define CAN_SET_SLAVE_STR  "CAN Set slave"

/*-----------------------------------------------------------------*/
/*record definitions*/

/*RECORD TYPE    BYTE*/
/*RECORD NODE    BYTE    (0-250) Node values 251-255 reserved for GSI (255  this)*/
/*RECORD LENGTH  BYTE    (0-8) Data only, doesn't include Type and Node*/
/*RECORD     0-8 bytes*/

#define rtLCD_BEGINING_OF_LINE_STR  LCD_BEGINING_OF_LINE_STR
#define rtLCD_CURS_LEFT_STR  LCD_CURS_LEFT_STR
#define rtGSI_USER_ABOrtSTR  GSI_USER_ABOrtSTR
#define rtLCD_DEL_CHAR_FWD_STR  LCD_DEL_CHAR_FWD_STR
#define rtLCD_END_OF_LINE_STR  LCD_END_OF_LINE_STR
#define rtLCD_CURS_RIGHT_STR  LCD_CURS_RIGHT_STR
#define rtGSI_BELL_STR  GSI_BELL_STR
#define rtLCD_BS_STR  LCD_BS_STR
#define rtLCD_TAB_STR  LCD_TAB_STR
#define rtLCD_CURS_DOWN_STR  LCD_CURS_DOWN_STR
#define rtLCD_CURS_END_STR  LCD_CURS_END_STR
#define rtLCD_CLS_STR  LCD_CLS_STR
#define rtLCD_CR_STR  LCD_CR_STR
#define rtLCD_CURS_DOWN1_STR  LCD_CURS_DOWN_STR
#define rtECHO_TOGGLE_STR  RS232_ECHO_TOGGLE_STR
#define rtLCD_CURS_UP_STR  LCD_CURS_UP_STR

#define rtXON_STR  RS232_XON_STR
/*pc->master*/
#define rtBINARY_MODE_STR  RS232_BINARY_MODE_STR
/*goto   GSIXOff     ;19 ctr-s*/
#define rtXOFF_STR  RS232_XOFF_STR
#define rtLCD_CURS_RIGHT_5_STR  LCD_CURS_RIGHT_5_STR /*goto   GSICursorRight5 ;20 ctr-t*/
/*pc->master*/
#define rtASCII_MODE_STR  RS232_ASCII_MODE_STR /*goto   GSIAsciiMode        ;21 ctr-u*/
/*goto   GSICursorLeft5      ;22 ctr-v*/
#define rtLCD_CURS_LEFT_5_STR  LCD_CURS_LEFT_5_STR
/*goto   GSICursorLeft10 ;23 ctr-w*/
#define rtLCD_CURS_LEFT_10_STR  LCD_CURS_LEFT_10_STR
/*goto   GSICRKeypadToggleState  ;24 ctr-x*/
#define rtKEYPAD_TOGGLE_STATE_STR  KEYPAD_TOGGLE_STATE_STR
/*goto   GSIKeypadToggleMode ;25 ctr-y*/
#define rtKEYPAD_TOGGLE_MODE_STR  KEYPAD_TOGGLE_MODE_STR
/*goto   GSICtrZ     ;26 ctr-z*/
#define rtCTR_Z_STR  ""


#define rtESC_ESC_STR  ""
#define RL_ESC_ESC_STR  ""

/*--------------------------------------------*/
/*Record struct equivalent to ESC commands*/
/*--------------------------------------------*/

#define rtESC_VERSION_STRING_STR  GSI_VERSION_STRING_STR
#define rtESC_CAPABILITIES_STR  GSI_CAPABILITIES_STR
#define rtESC_GET_BUFFER_STR  LCD_GET_BUFFER_STR
#define rtESC_SET_BUFFER_STR  LCD_SET_BUFFER_STR
#define rtESC_LCD_CLS_STR  LCD_CLS_STR
#define rtESC_LCD_INSTRUCTION_STR  LCD_INSTRUCTION_STR
#define rtESC_GET_CUR_TO_EOL_STR  LCD_GET_TO_EOL_STR
#define rtESC_GET_LINE_STR  LCD_GET_LINE_STR
#define rtESC_SET_LINE_STR  LCD_SET_LINE_STR
#define rtESC_HOME_STR  LCD_HOME_STR
#define rtESC_FLOW_CONTROL_ON_STR  RS232_FLOW_CONTROL_ON_STR
#define rtESC_FLOW_CONTROL_OFF_STR  RS232_FLOW_CONTROL_OFF_STR
#define rtESC_INIT_STR  GSI_INIT_STR
#define rtESC_GET_BUTTON_STATE_STR  KEYPAD_GET_BUTTON_STATE_STR
#define rtESC_CAN_SET_MASTER_STR  CAN_SET_MASTER_STR
#define rtESC_CAN_SET_SLAVE_STR  CAN_SET_SLAVE_STR


/* #define rtITERATOR_STR  ""
   #define rtSTA #define rtGSI_CODES_STR  ""
   #define STArtGSI_CODES_STR  ""

   -----Repeat, an echo error (at master) or Rx error (at node) occured--------
   pc->master
*/

#define GSI_REPEAT_STR  "RS232 repeat last record"
/* #define rtITERATOR_STR  "" _ITERATOR + 1*/

/*;------Acknowledge receipt of data-----*/
/*;master will suspend processing till ACK is received*/
/*;pc->master*/
#define rtGSI_ACK_STR  "RS232 Acknowledge"
/* #define rtITERATOR_STR  "" _ITERATOR + 1*/
/*;-----Send a Message to an RS232 module on a node------------------------------*/
#define rtGSIRS232Message_STR  "Message RS232"
/* #define rtITERATOR_STR  "" _ITERATOR + 1*/

/*;-----Send a Message to an LCD on a node------------------------------*/
#define rtGSI_LCDMessage_STR  "Message LCD"
/* #define rtITERATOR_STR  "" _ITERATOR + 1*/

/*;-----Send a Message to an LCD on a node------------------------------*/

#define rtGSI_KEYPADMessage_STR  "Message Keypad"
/* #define rtITERATOR_STR  "" _ITERATOR + 1*/

/*;-----Send a Message to an SPI module on a node------------------------------*/
#define rtGSI_SPIMessage_STR  "Message SPI"
#define RL_GSI_SPIMessage_STR  ""           /*4 bytes the node, 1 byte the address,1 byte the Message,""*/
/* #define rtITERATOR_STR  "" _ITERATOR + 1*/

/*;-----Send a Message to an I2C module on a node------------------------------*/
#define rtGSI_I2CMessage_STR  "Message I2C"
/* #define rtITERATOR_STR  "" _ITERATOR + 1*/

/*;-----Send a Message to a dio on a node------------------------------*/
#define rtGSI_DIOMessage_STR  "Message DIO"
/* #define rtITERATOR_STR  "" _ITERATOR + 1*/

/*;-----Send a Message to a dio on a node------------------------------*/
#define rtGSI_ADCMessage_STR  "Message ADC"
/* #define rtITERATOR_STR  "" _ITERATOR + 1*/

/*;-----------------------------------*/
/*;   RS232 CAN Records*/
/*;-----------------------------------*/
/*;---Tell node it is the master controlled by a pc*/
/*;pc->master*/
#define rtGSI_CAN_PC_MASTER_STR  "GSI PC master"
/* #define rtITERATOR_STR  "" _ITERATOR + 1*/

/*;---Tell node it is the stand alone master*/
/*;pc->master*/
#define rtGSI_CAN_MASTER_STR  "GSI Master"              /*As seen from PIC Master node""*/
/* #define rtITERATOR_STR  "" _ITERATOR + 1*/

/*;---Send a message down the CAN Message is in RECORD as a correctly ;formatted CAN MESSAGE*/
/*;pc->master*/
#define rtGSI_CAN_TXMessage_STR  "CAN send Message"      /*As seen from PIC Master node""*/
/* #define rtITERATOR_STR  "" _ITERATOR + 1*/

/*;----CAN Master has received a message that the pc needs to see*/
/*;master->pc*/
#define rtGSI_CAN_RXMessage_STR  "CAN received Message"
/* #define rtITERATOR_STR  "" _ITERATOR + 1*/

/*;----CAN master has encountered a CAN error the pc needs to know about*/
/*;master->pc*/
#define rtGSI_CAN_ERROR_STR  "CAN error"
/* #define rtITERATOR_STR  "" _ITERATOR + 1*/

/*;--A remote node has requested an id ( #define rtTX_CANMessage will send the ID)*/
/*;master->pc*/
#define rtGSI_CAN_SET_SLAVE_ID_REQ_STR  "CAN Set slave id"
/* #define rtITERATOR_STR  "" _ITERATOR + 1*/

/*--rs232 is attached to a slave. Tell slave to send a rest to the master*/
/*for an id. ivalent to holding 1 or switch at power up*/
/*pc->slave*/
/*   ERRORLEVEL -208*/
#define rtGSI_CAN_SLAVE_GET_SLAVE_ID_REQ_STR  "CAN Get slave id"
/*    ERRORLEVEL 208*/
/* #define rtITERATOR_STR  "" _ITERATOR + 1*/

/* #define rtEND_GSI_CODES_STR  "" _ITERATOR - 1*/
#endif

#endif

#if 0
Er	MACRO
	;Ra0=040
	;Ra1=225;
	;Ra2=04
	;Ra3=13

	ENDM
#endif
