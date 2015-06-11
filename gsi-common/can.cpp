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
#  pragma implementation "can.cpp"
#  pragma interface "can.cpp"
#endif

// For compilers that support precompilation,

#include "wx/wx.h"

#ifdef __BORLANDC__
#  pragma hdrstop
#endif


#include <wx/string.h>

#ifdef __VISUALC__	//supress level 4 warnings in MSC headers, keep them for rest of source code
#pragma warning(push, 1)
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef __VISUALC__
#pragma warning(pop)
// without this pragma, the stupid compiler precompiles #defines below so that
// changing them doesn't "take place" later!
#pragma hdrstop
#endif



#ifdef DMALLOC
#include <dmalloc.h>
#endif

//#include "gsi.hpp"

#include "include/gsbexcept.hpp"
#include "include/can.hpp"
#include "include/record.hpp"

//forward reference



#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#endif

//********************************************************************************
//
//                  Start GSICANFrame
//derived from CANFrame
//
//********************************************************************************

//*******************************************************************
//       default ctor
//       CANFrame::CANFrame()
//*******************************************************************



GSICANFrame::GSICANFrame()
{

  m_originNode=GSI_NODE_THIS;
  m_filled=false;
}


//*********************************************************************************
//           ctor
//GSICANFrame::GSICANFrame(wxUint32 id,GSIChar *buff,GSIChar length,GSIChar flags)
//*********************************************************************************

GSICANFrame::GSICANFrame(wxUint32 id,GSIChar *buff,GSIChar length, GSIChar flags)
 :CANFrame(id,buff,length,flags)
{

  m_filled=true;
  MakeNode();

}

//******************************************************************************
//           ctor
//GSICANFrame::GSICANFrame(GSIRecord &r)

//******************************************************************************

GSICANFrame::GSICANFrame(GSIRecord &r)
{

		errorType rv;
		rv=r.MakeGSICANFrame(*this);

#if 0
  long n1,n2,n3,n4,tnode;
  int i;
  GSIChar const *buffer;

  buffer=r.GetDataBuffer();
  SetLength((GSIChar) (r.GetLength() - GSICANIDPlusFlagLength));

  n1=buffer[GSICANFrameOffsetID];
  n2=buffer[GSICANFrameOffsetID+1];
  n3=buffer[GSICANFrameOffsetID+2];
  n4=buffer[GSICANFrameOffsetID+3];

  tnode=n4*256*256*256;
  tnode+= n3*65536;
  tnode+=n2*256;
	node=tnode >> 4;						//really a >> 12 as we have ignored the last byte
		//effectively a divide by 4096 throwing away the extra bits
  tnode+=n1;						//tnode now equals the CAN ID, has embedded GSI id code in it

	id=tnode/(node *GSICANFrameBase); //currently 4096

	//node =tnode >>12;					///4096


  SetNode((GSIChar)tnode);

   for(i=0;i < m_frame.length;i++)
     {
       m_frame.buffer[i]=buffer[i + GSICANFrameOffsetBuffer];

     }
//flags stored after last data in rs232 record
   m_frame.flags=buffer[i + GSICANFrameOffsetBuffer];
#endif

}
/****************************************************
								dtor
				GSICANFrame::~GSICANFrame()
******************************************************/
GSICANFrame::~GSICANFrame()
{

}

//******************************************************************************
//     GSICANFrame::MakeNode()
//Use existing ID
//*****************************************************************************
//GSICANFrame::MakeNode()
//{
//  node=m_frame.id/GSICANFrameBase;
//}





//********************************************************************************
//
//                 Start CANFrame
//
//********************************************************************************



//*********************************************************************************
//           ctor
//CANFrame::CANFrame(wxUint32 id,GSIChar *buff,GSIChar length,GSIChar flags)
//*********************************************************************************

CANFrame::CANFrame(wxUint32 id,GSIChar *buff,GSIChar length,GSIChar flags)
{
	flags;length;buff;id;
}

//*******************************************************************
//       default ctor
//       CANFrame::CANFrame()
//*******************************************************************
CANFrame::CANFrame()
{

}


//************************************************************
//      CANFrame::GetBuffer(int index)
//*********************************************************
const GSIChar CANFrame::GetBufferValue(int index) const
{
    wxASSERT(index >=0 && index <= CANMaxData);
#if GSI_USE_MORE_EXCEPTIONS
    if(index <0 || index > CANMaxData)
        throw(gsException(wxString(_("Bad Index")),errBounds));
#else
    if(index <0 || index > CANMaxData)
        index=0;
#endif
    return(m_frame.buffer[index]);
}

//********************************************************************************
//  errorType CANFrame::SetLength(GSIChar l)
//********************************************************************************
errorType CANFrame::SetLength(GSIChar l)
{
  if(l > CANMaxData)
    return(errBounds);
  m_frame.length=l;
  return(errNone);
}

//********************************************************************************
//		errorType SetID(wxUint32 n )
//********************************************************************************

errorType CANFrame::SetID(wxUint32 n )
{
	m_frame.id=n;
	return(errNone);
}


//********************************************************************************
//		errorType SetFlage(GSIChar f )
//********************************************************************************
errorType CANFrame::SetFlags(GSIChar f )
{
	m_frame.flags=f;
	return(errNone);
}

#if 0
//*********************************************************************************
//   errorType CANFrame::SetNode(GSIChar char l)
//********************************************************************************
errorType CANFrame::SetNode(GSIChar n)
{
  node=n;
  return(errNone);
}
#endif


//*********************************************************************************
//   errorType CANFrame::SetBuffer(GSIChar value,int index)
//********************************************************************************
errorType CANFrame::SetBuffer(int index,GSIChar value)
{
  if(index<0 || index >CANMaxData)
    return(errNone);
  m_frame.buffer[index]=value;
  return(errNone);
}

#if 0
inline in can.hpp
//*****************************************************************************
//void  CANFrame::SetFlag(GSIChar char);
//****************************************************************************
void SetFlag(GSIChar f)
{
  m_frame.flags=f;
}
#endif

//*******************************************************************
//       CANFrame::~CANFrame()
//*******************************************************************
CANFrame::~CANFrame()
{

}



/****************************************************************
GSICANFrame::SetStrings()

Set strings in the GSICANFrame from the underlying CANFrame
****************************************************************/
void GSICANFrame::SetStrings()
{
int i;
wxString s,s1;

    s.Empty();
    SetInformationString1(s);       //ensure blank so that can ignore when we come to print the frame
    SetInformationString2(s);

    s.Printf("%i",GetLength());
    SetLengthString(s);

    s.Printf("%i",GetGSIID());
    SetGSIIDString(s);

    s.Printf("%i",GetID());
    SetIDString(s);

    s.Printf("%i",GetOriginNode());
    SetOriginNodeString(s);

    for(i=0;i < GetLength();i++)
    {
        s1.Printf("%i ",GetBufferValue(i));
        s+=s1;
    }
    SetDataString(s);

    s.Printf("%x hex",GetFlags());
    SetFlagsString(s);


    switch(GetGSIID())
    {
    case GSICANIDSync:
	    //rv=Write(GSIAck); //??no echo required??
	    //Write rtAck is treated as a record! we need a simple GSIChar write
	    //GSI.ComPort.Output = Chr(rtGSIACK)]
	    //GSI.TxtRx.Text = GSI.TxtRx.Text + "CAN Sync" + Constants.vbCrLf;
	    SetIDString("GSICANIDSync");
	    break;
    case GSICANIDCriticalError:
	    SetIDString("GSICANIDCriticalError");
	    break;
    case GSICANIDTimestamp:
	    SetIDString("GSICANIDTimestamp");
	    break;
    case GSICANIDGetSetSlaveID:
	    SetIDString("GSICANIDGetSetSlaveID");

    /*You will receive this message when a slave node attempts to register itself with the master.
    When the master receives the GetSetSlaveID it will send the ID back to the new slave and forward the
    CAN mssg to any pc attached to the master.
	    */
	    break;
    case GSICANIDTerminalCommand:
	    SetIDString("GSICANIDTerminalCommand");

	    break;
    case GSICANIDRS232Message:
	    SetIDString("GSICANIDRS232Message");
	    break;
    case GSICANIDLCDMessage:
	    SetIDString("GSICANIDLCDMessage");
	    break;
    case GSICANIDKeypadMessage:
	     SetIDString("GSICANIDKeypadMessage");

	    //movff CnThisCANNode, CnTxBuff
	    //movlw CANKDMessageKeyUp
	    //movff WREG, CnTxBuff + 1
	    //movff KdCurrentKey, CnTxBuff + 2
	    //switch(rxFrame.GetBufferValue(1))
	    switch(GetBufferValue(GSICANFrameOffsetSubfunction))
	    {
		    //keypad m_Subfunctions
	    case GSICANKeypadMessageKeyUp:
    	     SetSubfuncString(_("GSICANKeypadMessageKeyUp"));
            //InformationsString(s) need \n as they may be blank. We want to print a null string
            //or a \n terminated string
             s.Printf(_("Key='%c'\n"),GetBufferValue(GSICANFrameOffsetSubfunction+1));
             SetInformationString1(s);
		    break;
	    case GSICANKeypadMessageKeyDown:
		     SetSubfuncString(_("GSICANKeypadMessageKeyDown"));
             s.Printf(_("Key='%c'\n"),GetBufferValue(GSICANFrameOffsetSubfunction+1));
             SetInformationString1(s);
		    break;
	    case GSICANKeypadMessageSetMode:
		     SetSubfuncString(_("GSICANKeypadMessageSetMode"));
		    //currently unsupported as a received message
		    //message can be sent
		    break;
	    case GSICANKeypadMessageGetMode:
		     SetSubfuncString(_("GSICANKeypadMessageGetMode"));
		    //currently unsupported as a received message
		    //message can be sent
		    break;
	    default:
		     SetSubfuncString(_("Unkown message"));
	    } //switch(rxFrame.GetBufferValue(1))

	    //GSI.TxtRx.Text = GSI.TxtRx.Text + "CANNode(" + CStr(rxFrame.GetBuffer(0)) + ") " + mssgStr1 + Constants.vbCrLf;
	    break; //case rtGSIR2RxCANMessageNode2PC:

	    case GSICANIDSPIMessage:
		    SetIDString(_("GSICANIDSPIMessage"));
		    break;
	    case GSICANIDI2CMessage:
		    SetIDString(_("GSICANIDI2CMessage"));
		    break;
	    case GSICANIDDIOMessage:
		    SetIDString(_("GSICANIDDIOMessage"));
		    break;
	    case GSICANIDADCMessage:
		    SetIDString(_("GSICANIDADCMessage"));
		    break;
	    case GSICANIDIOCTLMessage:
		    SetIDString(_("GSICANIDIOCTLMessage"));
		    //switch ( rxFrame.GetBufferValue(1))

		    switch(GetBufferValue(GSICANFrameOffsetSubfunction))
		    {
		    case CANIOCTLGetNodeNode2Master:
			    SetSubfuncString(_("CANIOCTLGetNodeNode2Master"));
			    //mssgStr1 = "Get Node (CANNode=" + rxFrame.GetBufferValue(2)+'0';
			    break;
		    case CANIOCTLGetModeNode2Master:
			    SetSubfuncString(_("CANIOCTLGetModeNode2Master"));
			    //mssgStr1 = "Get Mode (Mode=" + rxFrame.GetBufferValue(2)+'0';
			    break;
		    case CANIOCTLSetModeNode2Master:
			    SetSubfuncString(_("CANIOCTLSetModeNode2Master"));
			    //mssgStr1 = "Set Mode (Prev Mode=" + rxFrame.GetBufferValue(2)+'0';
			    break;
		    default:
			    SetSubfuncString(_("Unknown CANIOCTL message"));
		    } //switch ( rxFrame.GetBufferValue(1))

		    break;

        case GSICANIDGSIIOCTLMessage:
			SetIDString(_("GSICANIDGSIIOCTLMessage"));
			wxLogMessage("CAN GSI IOCTL MSSG\n\r");
			    //switch( rxFrame.GetBufferValue(1))

            switch(GetBufferValue(GSICANFrameOffsetSubfunction))
		    {
		    case CANGSIIOCTLGetRegNode2Master:
			    SetSubfuncString(_("CANGSIIOCTLGetRegNode2Master"));
			    //mssgStr1 = "Get CANNode: CANNode=" + rxFrame.GetBufferValue(2)+'0';
			    break;
		    case CANGSIIOCTLSetRegNode2Master:
			    SetSubfuncString(_("CANGSIIOCTLSetRegNode2Master"));
				    //mssgStr1 = "Get Mode: Mode=" + rxFrame.GetBufferValue(2)+'0';
			    break;
		    case CANGSIIOCTL1Node2Master:
			    SetSubfuncString(_("CANGSIIOCTL1Node2Master"));	 //for future additions
			    //mssgStr1 = "Set Mode: Prev Mode="+ rxFrame.GetBufferValue(2)+'0';
			    break;
		    default:
				    SetSubfuncString(_("Unknown CAN GSI IOCTL message"));	   //for future additions
		    }//switch( rxFrame.GetBufferValue(1))

		    break;

        case GSICANIDErrorMessage:
	        SetIDString(_("GSICANIDErrorMessage"));

			switch(GetBufferValue(GSICANFrameOffsetSubfunction))
			{
			case VersionMessage:
				SetSubfuncString(_("VersionMessage"));
				break;
			case WDTResetMessage:
    			SetSubfuncString(_("WDTResetMessage"));
	    		break;
		    case BrownoutResetMessage:
			    SetSubfuncString(_("BrownoutResetMessage"));
		        break;
			case UnknownResetMessage:
			    SetSubfuncString(_("UnkownResetMessage"));
			    break;
			case UserAbortResetMessage:
			    SetSubfuncString(_("UserAbortResetMessage"));
			    break;
			case PowerOnResetMessage:
			   SetSubfuncString(_("PowerOnResetMessage"));
			    break;
			case StackOverflowResetMessage:
			    SetSubfuncString(_("StackOverflowMessage"));
			    break;
		    case StackUnderflowResetMessage:
			    SetSubfuncString(_("StackUnderflowMessage"));
			    break;
			case ErrorI2CNoACKFromSlaveMessage:
			    SetSubfuncString(_("ErrorI2CNoACKFromSlaveMessage"));
			    break;
		    case ErrorI2CNoDataMessage:
			    SetSubfuncString(_("ErrorI2CNoDataMessage"));
			    break;
			case ErrorI2CBusCollisionMessage:
			    SetSubfuncString(_("ErrorI2CBusCollisionMessage"));
			    break;
			case UnkownErrorMessage:
			    SetSubfuncString(_("Unkown error message"));
			    break;
			case CANEWarnMessage:
			    SetSubfuncString(_("CANEWarnMessage"));
			    break;
			case CANRxWarnMessage:
			    SetSubfuncString(_("CANRxWarnMessage"));
			    break;
			case CANTxWARNMessage:
			    SetSubfuncString(_("CANTxWarnMessage"));
			    break;
			case CANRxBPMessage:
			    SetSubfuncString(_("CANRxBPMessage"));
			    break;
			case CANTxBPMessage:
			    SetSubfuncString(_("CANTxBPMessage"));
			    break;
			case CANTxBOMessage:
			    SetSubfuncString(_("CANTxB0Message"));
			    break;
		    case CANRxB1OVFLMessage:
			    SetSubfuncString(_("CANRxB1OVFLMessage"));
			    break;
			case CANRxB0OVFLMessage:
			    SetSubfuncString(_("CANRxB0OVFLMessage"));
			    break;
		    case CANIRXIFMessage:
			    SetSubfuncString(_("CANIRXIFMessage"));
				break;

			case CANErrorInitMessage:
			    SetSubfuncString(_("CANErrorInitMessage"));
			    break;

			case UnknownCANErrorMessage:
			    SetSubfuncString(_("UnknownCANErrorMessage"));
			    break;
					    /*Should never see any of these. Used to write ascii CAN messages to rs232*/
			case CANMessageString:	//;23 About to write a CAN mssg
			case CANMessageIDString:  //;24 mssg Id str
			case CANMessageDataString: //;25 mssg m_Data str
			case CANMessageLengthString: //;26 mssg m_Data Length str
			case CANMessageFlagString: // ;27 mssg Flag str
					    SetSubfuncString(_("Should never see this message!"));
			    break;
			case CANMessageSetSlaveIDString: // ;28 mssg Set id of slave
					    SetSubfuncString(_("CANMessageSetSlaveIDString"));
			    break;
			case R2ErrorEchoTxString:
					    SetSubfuncString(_("R2ErrorEchoTxString"));
			    break;

			case CANRxCQOverflowMessage:
			    SetSubfuncString(_("CANRxCQOverflowMessage"));
			    break;
			default:
			    SetSubfuncString(_("Unkown CAN error message"));
			}

		    break;  //end of GSICANIDErrorMessage

        case GSICANIDPingMessage:
            //frame buffer[0]=node of sender
            //frame buffer[1]=constant  CANGSIPingSend if node is originator
            //or  CANGSIPingReceived if node is echoing a PING sent from this node
            //GetBufferValue(0);
            SetIDString(_("GSICANIDPingMessge"));
            //s.Printf( _("Key='%c'\n"),GetBufferValue(GSICANFrameOffsetSubfunction+1));
            s.Printf( _("%c"),GetOriginNode()+'0' );
            SetOriginNodeString(s);
            SetSubfuncString(_("CANGSIPingReceived"));
            if (GetBufferValue(1) == CANGSIPingSend)
                SetSubfuncString(_("CANGSIPingSend"));
            break;

	    default:
            SetIDString(_("Unknown GSI CAN Message received"));
    } //End of case rtGSIR2_RX_CANMessage_Node2PC
}
