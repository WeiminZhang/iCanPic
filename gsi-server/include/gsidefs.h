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
#ifndef GSIDEFS_H_INCLUDED
#define GSIDEFS_H_INCLUDED

//#include "helper.hpp"

//#define STARTLE_SERVER 0          //define if we are a startle box

#define GSI_DEBUG 1
#define DIALOG_BLOCKS 1             //conditional compile for DB code

#define ADD_ASYNC_NODE_IN_GSIFRAME 1        //See node.cpp and gsi.cpp DefaultProcessAsyncRecord()

#define GSI_DEFAULT_COM_PORT  2

#define GSI_USE_EXCEPTIONS     1        //only limited exception handling at present. Some ctors throw them.
#define GSI_USE_MORE_EXCEPTIONS 0       //use exceptions for potentially fatal errors
#define GSI_USE_SHARED_POINTERS		0
#define GSI_DUPLICATE_WITH_CONTAINERS 0     //DOn't use this yet. There are problems with it


#ifdef WXUSINGDLL

	#ifdef GSIDLLEXPORTS

		#define GSI_API __declspec(dllexport)

	#else //GSIDLLEXPORTS
		#define GSI_API __declspec(dllimport)

	#endif //GSIDLLEXPORTS
#else
	#define GSI_API
#endif //WXUSINGDLL


#if GSI_USE_EXCEPTIONS
    #define GSI_THROW(wxStr,err) throw(gsException(err,wxString(wxStr)));
    #define GSI_SOCKET_THROW(sock,wxStr,err) throw(GSISocketException(sock,err,wxString(wxStr)));
#else
    #define GSI_THROW(wxStr,err);
    #define GSI_SOCKET_THROW(wxStr,err);
#endif

#define DEBUG_USE_COMM_MASK     0
#define DEBUG_USE_CTB           0
#define DEBUG_DEFER_ACK         0
#define DEBUG_ENABLE_BREAK      1
#define DEBUG_ENABLE_SET_MASTER 0
#define DEBUG_ENABLE_SET_SLAVE  0
//#define  GSI_ASCII  0
//#define  GSI_BINARY  1

//CQUEUE
#define  GSI_RS232_RX_MASTER 0
#define GSI_RS232_RX_SLAVE 1
#define GSI_RS232_USER_1 2
#define GSI_RS232_USER_2 3


enum
{
//Note that these are also used as GSIHandles for sending device handles to and from client/server
	GSI_ID_COM1=1,      //must start at>0 as 0 is used as no device found
	GSI_ID_COM2,
	GSI_ID_COM3,
   	GSI_ID_COM4,
	GSI_ID_COM5,
	GSI_ID_COM6,
    GSI_ID_USB000,
    GSI_ID_USB001,
    GSI_ID_USB002,
    GSI_ID_USB003,
    GSI_ID_USB004,
    GSI_ID_USB005,
    GSI_ID_USB006,
    GSI_ID_USB007,
    GSI_ID_USB008,
    GSI_ID_USB009,

    GSI_ID_USER_DEVICE_1,
    GSI_ID_USER_DEVICE_2,
    GSI_ID_USER_DEVICE_3,
    GSI_ID_USER_DEVICE_4,
    GSI_ID_USER_DEVICE_5,
    GSI_ID_USER_DEVICE_6,
    GSI_ID_USER_DEVICE_7,
    GSI_ID_USER_DEVICE_8
};

//Constants
const	int		GSIFirstId= 500;
const	int		GSILastId=	999;
const	int		GSIFirstUserId= 1000;
typedef wxUint8 GSIChar;
typedef wxInt16 GSIHandle;
typedef wxInt16 GSIDevHandle;
typedef wxInt16 GSINodeHandle;


//const GSIHandle GSINotAValidHandle=-1;
const GSINodeHandle GSINotAValidNode= -1;
const GSINodeHandle GSINotAValidNodeHandle= -1;
const GSIDevHandle GSINotAValidDevHandle=-1;

const GSIDevHandle GSIFirstHandle=0;
const GSIDevHandle GSILastHandle=GSI_ID_USER_DEVICE_8;
/*----------------------------
    GSI dialog defines
-----------------------------*/
#define GSI_WARNING _("GSI Warning")        //used in dialog boxes


#ifdef __WIN32__
typedef HANDLE ComPortHandle;	//use HANDLE (void *) in WIN32

#elif defined  __LINUX__
typedef int ComPortHandle;	//use file descriptor in unix
//#define PORT_WRITE(fd,b,l,n) write(fd,b,l)	//fd,&buff,length,dummy parameter unused
//#define PORT_READ(fd,b,l,n) read(fd,b,l)
#endif

#ifndef __INTEL__
//on 32 bit  platform shouldn't need a critical section as the numInBuff variable
//that is at risk is 32 bit and should be incremented
wxCriticalSectionLocker locker(GSIThreadCritSect);
#else

#endif
//StatusBar
//Server

//Record related constants
const long GSITimeout=100; //1000 msec timeout
//had to change EchoTimeout to 100mSec otherwise occasional failures to
//set Binary mode. cq:Read fails with a timeout
//Also set gsSerialThread to higher priority, which made no difference
const long GSIEchoTimeout=100; //20mSec timeout for write/read opertions
const long GSIRecordReadTimeout=100;

const int GSIRepeatCount=4; //try reading a record 4 times before giving up

//Binary record
const GSIChar GSIRecordFixedLength=3;
const GSIChar GSIRecordMaxDataLength=14;
const GSIChar GSIRecordMaxLength=GSIRecordFixedLength+GSIRecordMaxDataLength;
const size_t RS232RecordOffsetToSubfunction=0;	//offset to subfunction in an RS232 record relative to data array

const size_t RS232RecordOffsetToTypeField=0;    //these are an offset relative to the binary structure
const size_t RS232RecordOffsetToNodeField=1;
const size_t RS232RecordOffsetToLengthField=2;
const size_t RS232RecordOffsetToDataField=3;

//CANFrame
//The GSI convention is that first data byte is a subfunction index for those
//messages that support subfunctions

#if 0
enum GSICANBufferIndex
  {
    GSICANBufferIndexNode=0,     //XXX Need to check if this is correct
    GSICANBufferIndexSubFunction=1,
  };
#endif


//const GSIRecordOffsetRecType=0;
//const GSIRecordOffsetRecNode=1

//#define RS232_RECORD_FIXED_LENGTH 3
//#define RS232_RECORD_MAX_DATA_LENGTH  14  //received from node
//#define RS232_RECORD_MAX_LENGTH   RS232_RECORD_FIXED_LENGTH + RS232_RECORD_MAX_DATA_LENGTH
//#define  RS232_RECORD_OFFSET_LENGTH_BYTE  2 //length field is 3rd (0,1,2)


//#define OFFSET_REC_TYPE  0
//#define OFFSET_REC_NODE  1
//#define  OFFSET_REC_LENGTH  2
//#define OFFSET_REC_DATA  3



/*********************************************************************************************
        Nodes
**********************************************************************************************/

/*!named node id
a node value of 254 refers to the node attached to the rs232 line
ie. it will not be a CAN message but direct control of node
node of 255 will allow us to extend the node with data bytes
alternatively a nz length will signal that data array holds node
*/

const   int GSIMaxNumPhysicalNodes=250;     //an array of Nodes is allocated in GSIFrame 0-249
const   int GSIMaxNumSpecialNodes=6;

const   int GSIFirstVirtualNode=256;   //first 'virtual' node starts here
const   int GSIMaxNumVirtualNodes=256;

const   int GSIMaxNumNodes=GSIMaxNumPhysicalNodes+GSIMaxNumSpecialNodes+GSIMaxNumVirtualNodes;     //an array of Nodes is allocated in GSIFrame
const   int GSIMaxNumNodeHandles=GSIMaxNumNodes;

const   int GSIMaxNumComPorts   =6;
const   int GSIMaxNumUSBPorts   =10;
const   int GSIMaxNumUserPorts  =2;
const   int GSIMaxNumPorts      =GSIMaxNumComPorts+GSIMaxNumUSBPorts+GSIMaxNumUserPorts;

const   GSIHandle GSIFirstUSBPortHandle=GSIMaxNumComPorts;
const   GSIHandle GSIFirstComPortHandle=0;

const   int GSIMaxNumDeviceHandles=GSIMaxNumPorts;

const   int GSIMaxNumDevices=GSIMaxNumComPorts+GSIMaxNumUSBPorts;




/*!
*Keypad defines
*/
const  int GSIMaxNumKeypadKeys=16;


//StatusReport
enum StatusReportLevel
{
GSIStatusReportNone=	0,
GSIStatusReportLow=		1,
GSIStatusReportMedium=	2,
GSIStatusReportHigh	=	3
};


enum GSIEnumDeviceType
{
    GSI_DEVICE_RS232=0,
    GSI_DEVICE_USB,
    GSI_DEVICE_CAN,            //internal CAN card
    GSI_DEVICE_USER,
    GSI_DEVICE_NULL
};

enum GSIEnumNodeType
  {
    GSI_NODE_MASTER=0,
    GSI_NODE_LISTEN_ONLY=253,
    GSI_NODE_THIS=254,
    GSI_NODE_NO_NETWORK=255,            //Superceded, now uses node number >255
    GSI_NODE_NONE       = -1             //should work ok as long as we use int
  };

//named NODE names

#define  NODE_MASTER_STR  "Master"
#define NODE_THIS_STR  "This node"

//-------------------------------------------------------------------------------------

enum GSIEnumMasterSlave
{
GSI_MASTER=0,
GSI_SLAVE=1
};

enum GSIEnumMode
{
GSI_MODE_ASCII=0,
GSI_MODE_BINARY=1,
GSI_MODE_BLOCK=2,

GSI_MODE_USER1=10,
GSI_MODE_USER2=11,
GSI_MODE_USER3=12
};



enum GSIEnumOnOffState{
GSI_ENABLE=1,
GSI_DISABLE=2,
GSI_USER1=6,
GSI_USER2=7,
GSI_USER3=8
};





const int GSIServiceThreadPriority = WXTHREAD_DEFAULT_PRIORITY + 5;


/*------------------------ GSI Error stuff ----------------------------------------*/
//RS232 messages can return an error code(s) in addition to generic success/fail

//see error.inc

const int GSIErrI2CNoAckFromSlave=0;
const int GSIErrI2CNoData=1;
const int GSIErrI2CBusCollision=2;
const int GSIErrCAN=3;                   //further error info will (might!) be sent with the record
const int GSIErrRS232=4;
//Errors 5-7 for internal error condition (these are bit positions)
//further generic errors errors
const int GSIErrRange=8;
const int GSIErrFail=9;
const int GSIErrUnsupported=10;

/*
Error/information message id's from gsi pic code. (defs.inc)
*/
const GSIChar VersionMessage= 0;
const GSIChar WDTResetMessage=1;
const GSIChar BrownoutResetMessage	=2;
const GSIChar UnknownResetMessage=3;
const GSIChar UserAbortResetMessage=4;
const GSIChar PowerOnResetMessage=5;
const GSIChar StackOverflowResetMessage=6;
const GSIChar StackUnderflowResetMessage=7;
const GSIChar ErrorI2CNoACKFromSlaveMessage= 8;
const GSIChar ErrorI2CNoDataMessage=9;
const GSIChar ErrorI2CBusCollisionMessage=10;
const GSIChar UnkownErrorMessage=11;
const GSIChar CANEWarnMessage			=12;
const GSIChar CANRxWarnMessage			=13;
const GSIChar CANTxWARNMessage			=14;
const GSIChar CANRxBPMessage			=15;
const GSIChar CANTxBPMessage			=16;
const GSIChar CANTxBOMessage			=17;
const GSIChar CANRxB1OVFLMessage		=18;
const GSIChar CANRxB0OVFLMessage		=19;
const GSIChar CANIRXIFMessage			=20;
const GSIChar CANErrorInitMessage		=21;
const GSIChar UnknownCANErrorMessage	=22;
const GSIChar CANMessageString			=23;  //;23 About to write a CAN mssg
const GSIChar CANMessageIDString		=24;  //;24 mssg Id str
const GSIChar CANMessageDataString	=25; //;25 mssg Data str
const GSIChar CANMessageLengthString=26; //;26 mssg Data Length str
const GSIChar CANMessageFlagString	=27; // ;27 mssg Flag str
const GSIChar CANMessageSetSlaveIDString=28; // ;28 mssg Set id of slave
const GSIChar R2ErrorEchoTxString		=29;
const GSIChar CANRxCQOverflowMessage =30;
/*end Error/information message id's from gsi pic code. (defs.inc)*/




#endif //GSIDEFS_H_INCLUDED



