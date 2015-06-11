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

#if defined(__GNUG__) && !defined(__APPLE__)
#  pragma implementation "record.cpp"
#  pragma interface "record.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
//#include "wx/wxprec.h"
#include "wx/wx.h"

#ifdef __BORLANDC__
#  pragma hdrstop
#endif



#ifdef __VISUALC__
 #pragma warning(push, 1)
#endif


//std headers in here

#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#else
    #define DEBUG_NEW new
#endif




#ifdef __VISUALC__
#pragma warning(pop)
// without this pragma hdrstop, the stupid compiler precompiles #defines below so that
// changing them doesn't "take place" later!
#pragma hdrstop
#endif


//#include "gsi.hpp"
//#include "socket.hpp"

#include "include/record.hpp"
#include "include/can.hpp"
#include "include/rs232defs.h"
#include "include/socket.hpp"
#include "include/lcd.hpp"

//******************************************************
//
//             GSIRecord
//*****************************************************

//***************************************************
//             ctor
//**************************************************

GSIRecord::GSIRecord(void)
{
  SetFilledFlag(false);
  SetType((GSIChar) 0);
  SetNode(GSI_NODE_THIS);
  SetLength(0);
  //SetReturnedDataLength(0);
  m_ReturnedRecord=NULL;
  m_isReturnedRecord=false;
}

/***************************************************
		ctor copy
GSIRecord::GSIRecord(GSIRecord &r)
***************************************************/
GSIRecord::GSIRecord(const GSIRecord &r)
{
    static int recursionCounter=0;
    GSIChar l;
    SetNode(r.GetNode());
	l=r.GetLength();
	wxASSERT(l <= GSIRecordMaxDataLength);
//Better put this in in case user code sends us a dud record
//we can't change it as this is a const &. However user should be using SetData(index,data)
//which will not overwrite the buffer
	if(l >GSIRecordMaxDataLength)
			l=GSIRecordMaxDataLength;
	SetLength(l);

	for(int i=0;i<GetLength();i++)
	{
		SetData(i,r.GetData(i));			//should never have an error here as we have performed a length check
	}
    SetType(r.GetType());   //must come after the data copy as it uses the data to determine subfunction string
    SetFilledFlag(r.GetFilledFlag()); 						//honour the filled flag, as this might be an empty record
    SetStrings();

  //The GSIRecord might also contain an embedded record returned by the node. This will be a pointer
  //so if this pointer is !NULL then we must make a copy of it as well.
  //the dtor must delete the embedded record if it is !NULL, then set it to be NULL. There is a danger of
  //a recursive delete here!
    if(r.m_isReturnedRecord && m_ReturnedRecord)
    {
        if(recursionCounter == 0)
        {
            recursionCounter++;
            m_isReturnedRecord=true;
        //This is a recursive call to this function, though the is_returnedRecord will be false so it
        //will not be an endless recursion! If user has set the record explicitly, then it could be true,
        //so we need to have a recursion counter
            m_ReturnedRecord=DEBUG_NEW GSIRecord(r);
            //next lines stop any recursion, however, user of error could have set these values to non-null
            //remember that these lines operate on the copy of the record
            m_ReturnedRecord->m_ReturnedRecord=NULL;
            m_ReturnedRecord->m_isReturnedRecord=false;
            recursionCounter=0;
        }
        else
        {
        //error, recursive call!
            wxASSERT(l);
            m_isReturnedRecord=false;
            m_ReturnedRecord=NULL;
            recursionCounter=0;
        }
    }
    else
    {
	    m_isReturnedRecord=false;
	    m_ReturnedRecord=NULL;	  //new record doesn't have an embedded returned record
    }


}

//***************************************************
//             ctor GSIRecord(GSIEnumRecord record)
//
//Zero data for records created from an EnumRecord
//**************************************************

GSIRecord::GSIRecord(GSIEnumRecord r)
{
  SetType(r);
  m_binRecord.node=(GSIChar) GSI_NODE_THIS;
  m_binRecord.length=0;
  SetFilledFlag(true);
  SetStrings();
  m_ReturnedRecord=NULL;
  m_isReturnedRecord=false;
}

//****************************************************************
//             ctor GSIRecord(GSIEnumRecord record,GSIChar fill)
//
//Fill the m_Data buffer with fill char. Useful for commands that
//have multiples, eg cursor right. The data can be filled with
//cursor right codes. Record length will determine how many of them
//are executed by the pic node
//******************************************************************

GSIRecord::GSIRecord(GSIEnumRecord r,GSIChar fill)
{
	int i;
  m_binRecord.type=(GSIChar)r;
  m_binRecord.node=(GSIChar) GSI_NODE_THIS;
  m_binRecord.length=0;

	for (i=0;i<GSIRecordMaxDataLength;i++)
		{
			m_binRecord.data[i]=fill;
		}
  SetFilledFlag(true);
  SetStrings();
  m_ReturnedRecord=NULL;
  m_isReturnedRecord=false;
}

/********************************************************
Convert a wxString ("binary" string) to a record
*********************************************************/
GSIRecord::GSIRecord(const wxString & s)
{
	SetType(s[RS232RecordOffsetToTypeField]);
	SetNode(s[RS232RecordOffsetToNodeField]);
	SetLength(s[RS232RecordOffsetToLengthField]);
	wxASSERT(m_binRecord.length <= GSIRecordMaxDataLength);

	if(GetLength()>GSIRecordMaxDataLength) SetLength(GSIRecordMaxDataLength);

	for(int i=0;i<GetLength();i++)
		{
			SetData(i,s[RS232RecordOffsetToDataField+i]);
		}
    SetStrings();
    m_ReturnedRecord=NULL;
    m_isReturnedRecord=false;
}

/********************************************************
Convert buffer to GSIRecord
Probably received through a wxSocket ReadMsg()
Note that length is the length of the buffer which should
be the total length of the record
*********************************************************/
GSIRecord::GSIRecord(const GSIChar *buff,wxUint32 length)
{
	SetType(buff[RS232RecordOffsetToTypeField]);
	SetNode(buff[RS232RecordOffsetToNodeField]);
	SetLength(buff[RS232RecordOffsetToLengthField]);
	wxASSERT(GetLength() <= GSIRecordMaxDataLength);
	wxASSERT(length == (wxUint32) (GetLength()+GSIRecordFixedLength));
	if(GetLength() > GSIRecordMaxDataLength) SetLength(GSIRecordMaxDataLength);

	for(int i=0;i<GetLength();i++)
		{
			SetData(i,buff[RS232RecordOffsetToDataField+i]);
		}
	SetFilledFlag(true);
    SetStrings();
    m_ReturnedRecord=NULL;
    m_isReturnedRecord=false;
}


/*****************************************************************************

	errorType GSIRecord::SetLength(GSIChar l)

******************************************************************************/

errorType GSIRecord::SetLength(GSIChar l)
{
#if __VISUALC__
#warning ("Must Set length,node etc. string")
#endif

	if(l > GSIRecordMaxDataLength)
		return(errBounds);
	m_binRecord.length=l;
	return(errNone);
}


/**********************************************************************
	errorType GSIRecord::SetData(int index,GSIChar chr)
**********************************************************************/

errorType GSIRecord::SetData(int index,GSIChar chr)
{
	if (index < 0 || index > GSIRecordMaxDataLength)
		return(errBounds);
	m_binRecord.data[index]=chr;

    SetDataString();

	return(errNone);
}
/****************************************************************************
    void      FrameToRecord
Fill the data buffer with the contents of the CANFrame
NOTE:
This is a direct copy of a frame *not* the R2Record compressed
format, where length byte is after ID, thus the frame length may be 0-8
but the R2Record length will always be 14 bytes, the length of an xtd frame
id=4, buffer=8,length=1,flag=1. total=14
****************************************************************************/

void   GSIRecord::FrameToRecord(const CANFrame & f)             //create record from supplied CAN frame
{
GSIChar c1,c2,c3,c4;
    wxFAIL_MSG("Re-coded with no checking...");
    c1=(GSIChar) f.GetID();
    c2=(GSIChar) (f.GetID() >> 8);
    c3=(GSIChar) (f.GetID() >> 16);
    c4=(GSIChar) (f.GetID() >> 24);

    SetData(0,c1);
    SetData(1,c2);
    SetData(2,c3);
    SetData(3,c4);


    wxASSERT_MSG(f.GetLength() <= CANMaxData,"CAN data length error");
    //if(f.GetLength() > CANMaxData)
     //f.>length=CANMaxData;

    for(int i=0;i<f.GetLength();i++)
    {
         SetData(i+4,f.GetBufferValue(i));
    }

    SetData(12,f.GetLength());          //
    SetData(13,f.GetFlags());

    SetNode(GSI_NODE_THIS);         //
    SetLength(GSICANXtdFrameLength);  //always max frame length
    if(GetType()==0)
        SetType(rtGSICANSendMessage);
    SetFilledFlag(true);
}


//*************************************************
//             dtor
//************************************************

GSIRecord::~GSIRecord()
{
    if(m_ReturnedRecord)
    {
        delete m_ReturnedRecord;        //delete any embedded record. We have a recursion check in the copy ctor
        m_ReturnedRecord=NULL;
     }                                   //so should not have a recursive delete.
}



//**********************************************************
//
//*********************************************************
#if 0
errorType  GSIRecord::RecordToWxString(const wxString &s)
{
#warning ("All wxString arguments should be const wxString &, should all return wxString")
  size_t i;
    wxString str=s;

  if (!GetFilledFlag())
    return(errEmpty);
    str.Empty();
  //str << m_binRecord.type << " " << m_binRecord.node <<" " << m_binRecord.length;
    str.Printf("%c%c%c",m_binRecord.type,m_binRecord.node,m_binRecord.length);
    wxASSERT_MSG(str.length() ==3,"incorrect string length");
//#warning ("Error here")
  for(i=0;i<m_binRecord.length;i++)
    //str <<" " << m_binRecord.data[i];
		str=str+(wxChar)m_binRecord.data[i];
  return(errNone);
}
#endif
//*************************************************************************
//  errorType GSIRecord::RecordToCANFrame(CANFrame * frame)
//
//'Note to increase speed we send variable length can frames rather than
//'fixed 8 byte frames. Thus the frame is held in the record.buffer as
//'4 bytes id
//'0-8 bytes m_Data
//'1 byte m_Data length
//'1 byte flags
//'The length of the m_Data in the frame must be obtained from the rs232 record length
//'and the known fixed length of the CAN frame-8 bytes
//
//************************************************************************
errorType GSIRecord::MakeGSICANFrame(GSICANFrame & f)
{
long n1,n2,n3,n4; //,tnode;
int id,gsiId;
int i;
GSIChar originNode;

    f.SetLength( (GSIChar) (m_binRecord.length - GSICANIDPlusFlagLength));

    n1=m_binRecord.data[GSIRecordOffsetCANFrameID]; //buff[0]
    n2=m_binRecord.data[GSIRecordOffsetCANFrameID+1];
    n3=m_binRecord.data[GSIRecordOffsetCANFrameID+2];
    n4=m_binRecord.data[GSIRecordOffsetCANFrameID+3];

    id=n4*256*256*256;
    id+= n3*65536;
    id+=n2*256;
	id+=n1;
	f.SetID(id);				//sets the underlying can frame id, has node modula 4096 emedded within it
								//always node 0 if we are attached to a master as slaves all send
								//m_Data to master. The id is therefore always relative to node 0 (ie <4096)
								//use separate id's so that we can later attach to a slave and see all CAN
								//messages on the line.

	//node=tnode/GSICANFrameBase;
    originNode=m_binRecord.data[GSIRecordOffsetCANFrameOriginNode];	 //the node this message originated from
    //Now calculate the GSI library function

    gsiId=(int) fmod((float)id,(float)GSICANFrameBase);

	//f.SetGSIID(id-GSICANBase); //(id-GSICANFrameBase)); //the GSI library function id; frameBase == 4096
    f.SetGSIID(gsiId);
    f.SetOriginNode(originNode);												//The pic node (0-250)

    for(i=0;i < f.GetLength();i++)
    {
        f.SetBuffer(i,m_binRecord.data[i + GSIRecordOffsetCANFrameBuffer]) ;
    }
    f.SetFlags(m_binRecord.data[i + GSIRecordOffsetCANFrameBuffer]);
  //Each node is at a 4096 boundary. So node0 has can id's 0-4095, node1 has id's 4096-8191 etc.
   //id of 4096 is actually id 0 sent to node1
  //node is the id base 4096
  //node = frame.id / GSI_FRAME_BASE
	f.SetStrings();
    return(errNone);
}

//*************************************************************************
//errorType GSIRecord::FillGSICANFrame(GSICANFrame & f)
//Same as above function but operates on the internal CAN frame
//
//'Note to increase speed we send variable length can frames rather than
//'fixed 8 byte frames. Thus the frame is held in the record.buffer as
//'4 bytes id
//'0-8 bytes m_Data
//'1 byte m_Data length
//'1 byte flags
//'The length of the m_Data in the frame must be obtained from the rs232 record length
//'and the known fixed length of the CAN frame-8 bytes
//
//************************************************************************
errorType GSIRecord::FillGSICANFrame()
{
long n1,n2,n3,n4; //,tnode;
long id;
int i,gsiId;
GSIChar originNode,c;
//wxString s;

    GSICANFrame &frame=GetGSICANFrameRef();

//Set Length variables
    frame.SetLength( (GSIChar) (m_binRecord.length - GSICANIDPlusFlagLength));
    //s.Printf("%i",frame.GetLength());
    //frame.SetLengthString(s);

//Set ID (Type) variables.
#if __VISUALC__
#warning ("Need to settle on Type or Id for records!")
#endif

    n1=m_binRecord.data[GSIRecordOffsetCANFrameID]; //buff[0]
    n2=m_binRecord.data[GSIRecordOffsetCANFrameID+1];
    n3=m_binRecord.data[GSIRecordOffsetCANFrameID+2];
    n4=m_binRecord.data[GSIRecordOffsetCANFrameID+3];

    id=n4*256*256*256;
    id+= n3*65536;
    id+=n2*256;
    id+=n1;
    frame.SetID(id);			//sets the underlying can frame id, has node modula 4096 emedded within it
							    //always node 0 if we are attached to a master as slaves all send
							    //m_Data to master. The id is therefore always relative to node 0 (ie <4096)
							    //use separate id's so that we can later attach to a slave and see all CAN
							    //messages on the line.
    gsiId=(int) fmod((float)id,(float)GSICANFrameBase);
    frame.SetGSIID(gsiId);

    //s.Printf("%i",frame.GetGSIID());
    //frame.SetGSIIDString(s);

    //s.Printf("%i",frame.GetID());
    //frame.SetIDString(s);


    //Set node variables
    originNode=m_binRecord.data[GSIRecordOffsetCANFrameOriginNode];	 //the node this message originated from
    frame.SetOriginNode(originNode);
    												//The pic node (0-250)
    //s.Printf("%i",originNode);
    //frame.SetOriginNodeString(s);

    //s.Empty();
    //wxString s1;
    //Set Binary data variables
    for(i=0;i < frame.GetLength();i++)
    {
        c=m_binRecord.data[i + GSIRecordOffsetCANFrameBuffer];
        frame.SetBuffer(i,c) ;
        //s1.Printf("%i ",c);
        //s+=s1;
    }
    //SetDataString(s);

    frame.SetFlags(m_binRecord.data[i + GSIRecordOffsetCANFrameBuffer]);

    //s.Printf("%x hex",frame.GetFlags());
    //frame.SetFlagsString(s);
   //frame

  //Each node is at a 4096 boundary. So node0 has can id's 0-4095, node1 has id's 4096-8191 etc.
   //id of 4096 is actually id 0 sent to node1
  //node is the id base 4096
  //node = frame.id / GSI_FRAME_BASE
    frame.SetStrings();
    return(errNone);
}


/******************************************************************************
            errorType GSIRecord::SocketDataToRecord(GSISocketData &sd)
The socket data class is a general purpose mechanism, so we must read the first
byte (an id), then read the record if the id is compatible.
Use the GetData() member rather than GetBuffer() as it will return 0
if we attempt to read beyond the end of the buffer. Use GetLength() to find the
number of bytes in the buffer.
Note that this class is implemented in the client. The server only sends a stream
of GSIChars.
*******************************************************************************/
errorType GSIRecord::SocketDataToRecord(GSISocketData &sd)
{

errorType rv=errNone;

	SetFilledFlag(false);					//set here in case user is recycling a record
	switch(sd.GetID())
	{
		case GSI_SOCKET_RECORD:
		case GSI_SOCKET_ASYNC_RECORD:
			{
				size_t i,len;
				len=sd.GetLength(); 		//the length does not include the id byte
				if(len < GSIRecordFixedLength)
					return(errBadId);

				SetNode(sd.GetData(RS232RecordOffsetToNodeField));
				SetLength(sd.GetData(RS232RecordOffsetToLengthField));
				if(sd.GetData(RS232RecordOffsetToLengthField) >GSIRecordMaxDataLength)	//quick consistency check
				{
					SetLength(0);
					return(errFail);
				}
				for(i=0;i<GetLength();i++)
				{
					SetData(i,sd.GetData(i+RS232RecordOffsetToDataField));
				}
                //SetType must be last as it uses data and length fields
				SetType(sd.GetData(RS232RecordOffsetToTypeField));		 //first item is the id of the record

				SetFilledFlag(true);
                SetStrings();
				}
			break;
		default:			//GSISocketInput,GSISocketOutput,GSISocketConnection,GSISocketLost,
			rv=errBadId;
			break;

	}
	return rv;

}

/*********************************************************
    String set functions
**********************************************************/
void GSIRecord::SetNodeString()
{
	m_NodeString.Printf("%i", GetNode());
	if(GetNode()==GSI_NODE_THIS)
		m_NodeString+=" (Local Node)";

}

void GSIRecord::SetLengthString()
{
	m_LengthString.Printf("%i", GetLength());
}

void GSIRecord::SetFilledFlagString()
{
    if(GetFilledFlag())
        m_FilledFlagString="Filled";
    else
        m_FilledFlagString="Empty";

}

void GSIRecord::SetDataString()
{
    wxString s;
    GSIChar c;
	int i;

    m_DataString.Empty();
    if(GetLength())
    {
	    for(i=0;i<GetLength();i++)
	    {
		    c=GetData(i);
		    s.Printf("%i, ",c);
		    m_DataString=m_DataString+s;
	    }
        m_DataString[m_DataString.Find(',',true)]=' ';

        m_DataString+=" (";
	    for(i=0;i<GetLength();i++)
	    {
		    c=GetData(i);
		    s.Printf("%x, ",c);
		    m_DataString=m_DataString+s;
	    }
        m_DataString[m_DataString.Find(',',true)]=' ';
        m_DataString+=" Hex)";

    }
}
/***************************************************************
errorType SetType(GSIChar chr)
error check is required for this function as it is not an enum
****************************************************************/

errorType GSIRecord::SetType(GSIChar chr)
{
    errorType rv=errNone;
    m_binRecord.type=chr;
//we will rely on the string set function for error checking so must ensure it is up to date
    rv=SetTypeString();
    if(rv)
        m_binRecord.type=0;     //use the null record type. The controllor will ignore this if it is rec'd as
        //a record field. Unfortunately the next byte we send will be the node byte. The microcontroller will
        //interpret this as the record type! We simply must attempt to never send erroneous records.
#if __VISUALC__
#warning ("Need to perform record consistency test before sending")
#endif
    return(rv);
}

/********************************************************************************************
        errorType GSIRecord::SetTypeString()
This function sets text strings detailing the type of the record and its subfunction (if any)

Also a very good spot to do range checking of the supplied parameters
Note that this function can be called when code sets the Type field.
This function relies on data[] and length being correctly filled
So, we must check length and return without error if it is incorrectly filled
The calling code will (if it is correctly functioning) fill in the length and data
fields and call the SetStrings() function which will call this routine
Probably need a better method than this. These functions were called in case the caller
changes any particular field, in which case the strings need to change. However it does
lead to this problem.
Should perhaps only update when FilledFlag is set to true? Problem here is that code might
change values with the flag set, so these will not be updated.
*********************************************************************************************/
errorType GSIRecord::SetTypeString()
{
	GSIChar t;
    int subfunction;
    errorType rv=errNone;
#if __VISUALC__
#warning ("Perform some range checking in this function")
#endif
    if(GetLength() > GSIRecordMaxLength)
        return(errNone);        //return without error, will be filled in later

	t=GetType();
    SetSubfunctionString("none");
	SetInfoString1("");
	SetInfoString2("");

    switch (GetType())
    {

    case rtGSIReserved1:
	    SetTypeString("rtGSIReserved1");
	    break;
    case rtGSIReserved2:
	    SetTypeString("rtGSIReserved2");
	    break;
    case rtGSIRS232Message:
	    SetTypeString("rtGSIRS232Message");
        if(GetFilledFlag())     //only set a subfunction string if the record is complete
        {
            subfunction = GetData(0);
	        switch( subfunction)
	        {
	        case GSIRS232MessageSuccess:
		        SetSubfunctionString("GSIRS232MessageSuccess");
		        break;
	        case GSIRS232MessageFailure:
		        SetSubfunctionString("GSIRS232MessageFailure");
	        break;
	        case GSIRS232MessageReqSlave:
		        SetSubfunctionString("GSIRS232MessageReqSlave");
	        /*
	        node wants to be slave
	        will be called when node has requested it be master (by sending a NULL
	        while this code is master)
	        After it has completed its service it will send a
	        rtGSIRS232Message, MessageReqSlave
	        GSI.TxtRx.text = GSI.TxtRx.text + "ReqSlave (make this code the master)" + Constants.vbCrLf
		        */
		        break;
	        case GSIRS232MessageReqMaster://'not used, use GSIRS232MessageMASTERReqBIT
	            SetSubfunctionString("GSIRS232MessageReqMaster");
		        break;
	        case GSIRS232MessageSendData:
		        SetSubfunctionString("GSIRS232MessageSendm_Data");
		        break;
	        case GSIRS232MessageGetData:
		        SetSubfunctionString("GSIRS232MessageGetData");
		        break;
	        case GSIRS232MessageChangeNode:
				{
					SetSubfunctionString("GSIRS232ChangeNode");
					GSIChar originalNode=GetData(1);
					wxString s;
					s.Printf("Original node : %i",originalNode);
					SetInfoString1(s);
					s.Printf("New node : %i",GetData(2));
					SetInfoString2(s);
				}
		        break;
	        case GSIRS232MessageNewNode:
				{
					SetSubfunctionString("GSIRS232NewNode");
					wxString s;
					s.Printf("New node : %i",GetData(1));
					SetInfoString1(s);
				}
		        break;
	        case GSIRS232MessageN3:
		        SetSubfunctionString("GSIRS232MessageN3");
		        break;
	        default:
		        SetSubfunctionString("Unknown subfunction");
                rv=errParameter;
		        //MsgBox "Unknown RS232Message", vbExclamation;
	        } //switch (subfunction)

        }
        else    //GetFilledFlag()
        {
		        SetSubfunctionString("Unknown: incomplete record");
        }

	    //for simplicity the node should always send an rtGSIR2RXCANMessageNode2PC
	    //as the following messages are the special case when the node that has
	    //had its keypad pressed (or whatever) happens to be attached to the pc
        break;
    case rtGSIRS232BreakMessage:
        SetTypeString("rtGSIRS232BreakMessage");
        switch(GetData(RS232RecordOffsetToSubfunction))
        {
        case GSIRS232BreakPOReset:       //Power on reset detected
            SetSubfunctionString("Power On reset");
        case GSIRS232BreakBOReset:       //Brown out reset
            SetSubfunctionString("Brown out reset");
        case GSIRS232BreakSOVFReset:         //Stack Overflow
            SetSubfunctionString("Stack overflow");
        case GSIRS232BreakSUFReset:       //Stack underflow
            SetSubfunctionString("Stack underflow");
        case GSIRS232BreakWDTReset:        //WDT reset
            SetSubfunctionString("Watchdog timer timeout");
        case GSIRS232BreakUnknownReset:      //Unknown reset
            SetSubfunctionString("Unknown reset (MCLR ? reset)");
        case GSIRS232BreakUserAbortReset:
            SetSubfunctionString("User abort reset (Ctr-C)");
        case GSIRS232BreakAsyncRecord:      //Asyncronous record (microcontroller->pc) request
            SetSubfunctionString("Asynchronous record"); //could also use rs232 message with SlaveReq subfunction
            break;
        default:
            break;
        }
        break;
    case rtGSILCDMessage:
	    SetTypeString("rtGSILCDMessage");
	    switch(GetData(RS232RecordOffsetToSubfunction))
	    {
	    case GSILCD::LCD_INSTRUCTION:
		    SetSubfunctionString("LCD_INSTRUCTION");
		    break;
	    case GSILCD::LCD_WRITE_STRING:
		    SetSubfunctionString("LCD_WRITE_STRING");
		    break;
	    case GSILCD::LCD_WRITE_CHAR:
		    SetSubfunctionString("LCD_WRITE_CHAR");
		    break;
	    case GSILCD::LCD_GET_STRING:
		    SetSubfunctionString("LCD_GET_STRING");
		    break;
	    case GSILCD::LCD_CTR_JUMP_TABLE:
		    SetSubfunctionString("LCD_CTR_JUMP_TABLE");
		    break;
	    case GSILCD::LCD_GET_CAPABILITIES:			//returns COL, ROW as 16 bit value, Flag as 8 bit value (currently always 0)
		    SetSubfunctionString("LCD_GET_CAPABILITIES");
		    break;
	    }

	    break;
    //These Keypad messages should not be seen here as these are records being written
    case rtGSIKeypadMessage:
	    SetTypeString("rtGSIKeypadMessage");
	    switch(GetData(0))
	    {
		    //keypad subfunctions
	    case GSICANKeypadMessageKeyUp:
		    SetSubfunctionString("GSICANKeypadMessageKeyUp");
		    m_DataString=(wxChar) GetData(1);
		    //mssgStr1 = "Key up: ");
		    //mssgStr1 = mssgStr1 + (wxChar) record.GetData(1);
		    break;
	    case GSICANKeypadMessageKeyDown:
		    SetSubfunctionString("GSICANKeypadMessageKeyDown");
		    m_DataString=(wxChar) GetData(1);
		    //mssgStr1 = "Key down");
		    //mssgStr1 = mssgStr1 + (wxChar) record.GetData(1);
		    break;
	    case GSICANKeypadMessageSetMode:
		    SetSubfunctionString("GSICANKeypadMessageSetMode");
		    //currently unsupported as a received message
		    //message can be sent
		    //mssgStr1 = "Keypad Set mode");
	    case GSICANKeypadMessageGetMode:
		    SetSubfunctionString("GSICANKeypadMessageGetMode");
		    //currently unsupported as a received message
		    //message can be sent
		    //mssgStr1 = "Keypad Get mode");
		    break;
	    default:
            rv=errParameter;
		    SetSubfunctionString("Unknown subfunction");
		    //mssgStr1 = "Keypad Unknown mssg");
	    } //switch(rxFrame.GetBufferValue(1))
	    break;

    case rtGSISPIMessage:
	    SetTypeString("rtGSISPIMessage");
	    break;
    case rtGSII2CMessage:
	    SetTypeString("rtGSII2CMessage");
	    break;
    case rtGSIDIOMessage:
	    SetTypeString("rtGSIDIOMessage");
	    break;
    case rtGSIADCMessage:
	    SetTypeString("rtGSIADCMessage");
	    break;
    case rtGSICANPCMaster: //should never be called this is a pc->node command
	    SetTypeString("rtGSICANPCMaster");
	    break;
    case rtGSICANMaster:	//should never be called this is a pc->node command
	    SetTypeString("rtGSICANMaster");
	    break;
    case rtGSICANSendMessage: //should never be called this is a pc->node command
	    SetTypeString("rtGSICANSendMessage");
	    break;
    case rtGSICANError: 	//node has received an error
	    SetTypeString("rtGSICANError");
	    break;
    case rtGSICANSetSlaveIDReq:
	    SetTypeString("rtGSICANSetSlaveIDReq");
	    break;
    case rtGSICANSlaveGetSlaveIDReq:
	    SetTypeString("rtGSICANSlaveGetSlaveIDReq");
	    break;
    case rtGSICANIOCTLMessage:
        SetTypeString("rtGSICANIOCTLMessage");
        //subfunctions for IOCTL
   	    switch(GetData(0))
	    {
        case CANIOCTLGetNode:
            SetSubfunctionString("CANIOCTLGetNode");
            break;
        case CANIOCTLGetMode:
            SetSubfunctionString("CANIOCTLGetMode");
            break;
        case CANIOCTLSetMode:
            SetSubfunctionString("CANIOCTLSetMode");
            break;
        case CANIOCTLGetNodeNode2Master:
            SetSubfunctionString("CANIOCTLGetNodeNode2Master");
            break;
        case CANIOCTLGetModeNode2Master:
            SetSubfunctionString("CANIOCTLGetModeNode2Master");
            break;
        case CANIOCTLSetModeNode2Master:
            SetSubfunctionString("CANIOCTLSetModeNode2Master");
            break;
        default:
            SetSubfunctionString("Unknown subfunction");
        }//end switch GetData(0);
        break;
    case rtGSICANGSIIOCTLMessage:
        SetTypeString("rtGSICANGSIIOCTLMessage");
        break;
    case rtGSICANPingMessage:
        SetTypeString("rtGSICANPingMessage");
        switch(GetData(1))
        {
        case CANGSIPingSend:	    //;data[1]=CAN_PING_SEND if originator
            SetSubfunctionString("CANGSIPingSend");
            break;
        case CANGSIPingReceived:	//data[1]=CAN_PING_RECEIVED if echoing a PING
            SetSubfunctionString("CANGSIPingReceived");
            break;
        default:
            SetSubfunctionString("Unknown subfunction");
            break;
        } //end switch(GetData(0))

        break;
    case rtGSICANReserved1:
        SetTypeString("rtGSICANReserved1");
        break;
    case rtGSICANReserved2:
        SetTypeString("rtGSICANReserved2");
        break;
    case rtGSICANReserved3:
        SetTypeString("rtGSICANReserved3");
        break;
    case rtGSICANReserved4:
        SetTypeString("rtGSICANReserved4");
        break;
    case rtGSICANReserved5:
        SetTypeString("rtGSICANReserved5");
        break;

    case rtRS232SetMaster://Receiver must become Master if possible
	    SetTypeString("rtRS232SetMaster");
	    break;
    case rtRS232ReqMaster:	 //75 receiver must be become slave if possible
	    SetTypeString("rtRS232ReqMaster");
	    break;
    case rtGSIR2RxCANMessageNode2PC:   //Node2PC=Node->Pc. Node has a CAN message for the pc
	    SetTypeString("rtGSIR2RxCANMessageNode2PC");
	    //A can message
	    //rv=MakeGSICANFrame(&rxFrame);
	    //rv=MakeGSICANFrame(GetGSICANFrame().);
	    rv=FillGSICANFrame();
		    //rv=MakeGSICANFrame(m_Frame);
    #if GSI_USE_MORE_EXCEPTIONS
	    if(rv)
	    {
		    throw(gsException(wxString("Failed to make CAN Frame"),rv));
	    }
    #endif
		break;

	case rtUser0:
		SetTypeString("rtUser0");		//use will have set subtype if any
		break;
    } //Switch (GetType())
    return(rv);
}

/*******************************************************
GSIStringRecord::GSIStringRecord(GSIRecord &r)
Make a copy of the GSIRecord sent to us
********************************************************/


//GSIStringRecord::GSIStringRecord(GSIRecord &r)
void GSIRecord::SetStrings()
{

    SetNodeString();
    SetLengthString();
    SetTypeString();
    SetDataString();
    SetFilledFlagString();

}

/****************************************************************************
GSIChar GSIRecord::RecordToBuffer(GSIChar *buff)
Assumes a buffer of at least GSIRecordMaxDataLength has been allocated
return the record length
****************************************************************************/
GSIChar GSIRecord::RecordToBuffer(GSIChar *buff) const
{
	if(GetLength() >GSIRecordMaxDataLength)
		return(0);				//error value for this function, should always be >0

	buff[RS232RecordOffsetToTypeField]=GetType();
	buff[RS232RecordOffsetToNodeField]=GetNode();
	buff[RS232RecordOffsetToLengthField]=GetLength();
	for(GSIChar i=RS232RecordOffsetToDataField;i< RS232RecordOffsetToDataField+GetLength();i++)
		{
			buff[i]=GetData(i-RS232RecordOffsetToDataField);
		}
	return((GSIChar) (GetLength()+GSIRecordFixedLength));
}
/**********************************************************
errorType GSIRecord::SetType(GSIEnumRecord t)
***********************************************************/
errorType GSIRecord::SetType(GSIEnumRecord t)
{
    m_binRecord.type=(GSIChar)t;
     return(SetTypeString() );
}



/*! wxString GSIRecord::MakeInfoString()
 * make a string that documents the record in user readable form
*/
wxString GSIRecord::MakeInfoString()
{
    //GSIStringCANFrame sf;
    int originNode;
//if not a CAN msg then this will be the origin. If CAN msg then this is the node attached to the device
//node of origin is embedded in the CAN msg and we will overwrite this variable
    originNode=GetNode();

    wxString s;
     // \ is continuation character
    s.Printf(_("GSISocketAsyncRecord:\n"
    "-----------------------------------------\n"
    "Type=%s\nNode=%s\nSubfunction=%s\nData=%s\nInfo1=%s\nInfo2=%s\n"
    "------------------------------------------\n"),
    GetTypeString().c_str(),
    GetNodeString().c_str(),
    GetSubfuncString().c_str(),
    GetDataString().c_str(),
	GetInfoString1().c_str(),
	GetInfoString2().c_str()
	);

    //TextCtrl()->AppendText(s);
    if(GetType()==rtGSIR2RxCANMessageNode2PC)
    {

    #if 1
        GSICANFrame & frame=GetGSICANFrameRef();
        originNode=frame.GetOriginNode();
        s.Printf(_("CAN frame received:\n"
        "CAN id=%s\n"
        "Origin  Node=%s\n"
        "subfunction=%s\n"
        "%s%s"),                      //two optional information strings, null or /n terminated
        frame.GetIDString().c_str(),
        frame.GetOriginNodeString().c_str(),
        frame.GetSubfuncString().c_str(),
        frame.GetInformationString1().c_str(),
        frame.GetInformationString2().c_str()
        );

        //TextCtrl()->AppendText(s);
    #endif

    }
	return s;
}
