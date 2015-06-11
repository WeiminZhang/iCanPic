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
#  pragma implementation "node.cpp"
#  pragma interface "node.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"


#ifdef __BORLANDC__
#  pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#  include "wx/wx.h"
#endif

/*
For vc600 use gsi workspace
use console as active project
use console GSIDllDebug as active configuration
ensure that gsi.dll is copied to debug directory of thread project

To test the dll
Use thread project set to Thread Win32 Debug

*/


#include<wx/stopwatch.h>
#include <wx/thread.h>
//#define _POSIX_SOURCE 1 /* POSIX compliant source */



#ifdef __VISUALC__
#pragma warning(push, 1)
#endif

//std headers in here

#ifdef __VISUALC__
#pragma warning(pop)
// without this pragma, the stupid compiler precompiles #defines below so that
// changing them doesn't "take place" later!
#pragma hdrstop
#endif

#ifdef DMALLOC
#include <dmalloc.h>
#endif

//#include <gsExcept.hpp>
#include "include/gsbexcept.hpp"
#include "include/node.hpp"
//#include "include/gsi.hpp"
#include "include/lcd.hpp"
#include "include/device.hpp"
#include "include/record.hpp"

#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#endif
#if __GNUC__
    #define DEBUG_NEW new
#endif


/*******************************************************************************************
				ctor
    GSINode(GSIFrame * frame, GSIDevHandle h_device,GSIHandle h_node=GSI_NODE_THIS,wxSocketBase *sock=NULL)
The pc will query if there are any nodes on the CAN if so, we instantiate using this ctor
setting the node to be that which the master node has returned for each of the slaves
Thus we already have a comm link to the CAN so we use the GSIDevice * of this master node
*********************************************************************************************/

//GSINode::GSINode(GSIFrame * frame, GSIDevHandle h_device,GSIHandle h_node,wxSocketBase *sock)
//:m_Frame(frame)
GSINode::GSINode(GSIFrame * frame, GSIDevice *device,GSIHandle h_node,wxSocketBase *sock)
:m_Frame(frame)

{
    if(device->IsOpen()==false)
    {
        GSI_THROW("Device not open",errNotOpen);
    }

	SetLCD(DEBUG_NEW GSILCD(device));
	SetNodeHandle(h_node);
    SetHdwNode(GSI_NODE_NO_NETWORK);
    SetNodeDevice(device);
	LCD()->SetNode(h_node);
    m_Sock=sock;

}

/*---------------------------------------------------------------------------
void	GSINode::SetNodeDevice(GSIDevice *d)

This function only sets the device for the Node and its subsystems (LCD etc)
It does not modify any Device() or dervived classes
----------------------------------------------------------------------------*/
void	GSINode::SetNodeDevice(GSIDevice *d)
{
bool isOpen;
	m_CommDevice=d;
    if(d)       //Can be NULL, in which case ignore
    {
    //errorType GSIFrame::GetDeviceHandle(const wxString &dev,GSIHandle *handle,bool *is_open)
    //need to obtain the deviceHandle
    //XXXGS mod 7/4/12 using local handle rather than Frame()->GetHandle()

        //m_deviceHandle=Frame()->GetDeviceHandle(d->GetDeviceName(),&isOpen);
        //the handle is set in ctor init list

	    //LCD()->SetCommDevice(d);
		LCD()->SetDevice(d);
        d->SetNode(this);
    }
}


void	GSINode::SetNodeHandle(GSIHandle node_handle)
{
    wxASSERT_MSG(node_handle >=0 && node_handle <GSIMaxNumNodes,"Node handle error");

    if(node_handle >GSIMaxNumNodes)
        node_handle=GSI_NODE_NO_NETWORK;

    m_handle=node_handle;
    LCD()->SetNode(node_handle);

#warning ("?? need to set device GSINode* variable here??")

}

void	GSINode::SetLCD(GSILCD * lcd)
{
	m_Lcd=lcd;

}

#if 0
void	GSINode::SetCustomEventHandler(wxEvtHandler * evt)
{
	m_CustomEventHandler=evt;
#warning ("Set the GSIDevice CustomEventHandler")
}

void	GSINode::SetSockServer(GSIServer * s)
{
	m_SockServer=s;
}
#endif


GSIDevice * const GSINode::Device() const
{
	if(m_CommDevice ==NULL)
	{
		GSI_THROW("No Device object (GSINode::Device)",errNoAlloc)
	}
	return(m_CommDevice);
}

//*******************************************************

//                     dtor

//******************************************************

GSINode::~GSINode()
{
	if(m_Lcd)
    {
		delete m_Lcd;
        m_Lcd=NULL;
    }
}


/***************************************************************
        GSIChar GSINode::ReadNode()
talk to the node attached to this pc using the assigned Device
Set m_node
****************************************************************/
errorType GSINode::ReadNode(GSIHandle *h)
{
errorType rv=errNone;

    GSIRecord r(rtGSICANIOCTLMessage);      //instantiate the record, a generic IOCTL
    r.SetData(0,CANIOCTLGetNode);
    r.SetLength(1);
    rv=Device()->Write(r);
    switch (rv)
    {
        case errNone:
        if(r.IsReturnedRecord())
        {
            //with a successful IOCTL, the node will send back data with the RS232Record.
            //data[0] will be 0, signifying SUCCESS, data[1] will be the node.
            //A node of 255 signifies there is no CAN running.
            SetHdwNode((GSIChar) r.GetReturnedRecord()->GetData(RS232RecordOffsetToSubfunction+1));
            *h= GetHdwNode();
        }
        break;

        default:
        break;
    }

    return(rv);
#if GSI_USE_MORE_EXCEPTIONS

#endif


}
//**************************************************************************************************
//	errorType GSINode::DefaultProcessAsyncRecord(GSIRecord &record )
//
//
//the node has sent us some data. This can be from a remote node or the
//'node attached to the pc. It will probably most frequently be a SYNC
//from the CAN master (which may or may not be attached to the pc)
//Need to extract the OriginNode and call the NodeArray[OriginNode]
//so the node can handle the message

//This is all that this fuction needs to do??

//Note:
//This function is called from the GSIFrame object in gsi.cpp
//When we have a:
//case rtGSIR2RxCANMessageNode2PC:
//and there is no node associated with the message, it means that a new (un-allocated) node
//is on the network and we need to create a node object to service it
//Currently we call the Frame()->CreateNode() function and then the Frame->DefaultProcessAsyncRecord()
//function, which will of course call back to here.
//This looks inneficient and potentially a recurion problem

//Solution would be for GSIFrameDefaultProcessAsyncRecord to check for rtGSIR2RxCANMessageNode2PC:
//with no node object to be called and create the node BEFORE calling this function

//However this still leaves us with DefaultProcessAsyncCANFrame() which can also require us
//to add a node to the network!

//if we can eliminate these problems then we don't need to have a GSIFrame * embedded in a Node object
//************************************************************************************************

errorType GSINode::DefaultProcessAsyncRecord(GSIRecord &record )
{
errorType rv=errNone;
int subFunction;

wxString  mssgStr1;

#if DEBUG_DEFER_ACK
    AckHelper Ack(Device());           //use this to write the Ack, will write it in dtor if not otherwise written
#endif
    //const GSIBINRECORD &r=record.GetBinRecord();
GSICANFrame frame,rxFrame;
//const GSICANFrame &rxFrame=record.GetGSICANFrameRef();
  //DebugWriteRecord(record);

    switch (record.GetType())
    {

    case rtGSIReserved1:
	    break;
    case rtGSIReserved2:
	    break;
    case rtGSIRS232Message:
	    subFunction = record.GetData(RS232RecordOffsetToTypeField);
	    switch( subFunction)
	    {
	    case GSIRS232MessageSuccess:
		    break;
	    case GSIRS232MessageFailure:
	    break;
	    case GSIRS232MessageReqSlave:
/*
Node wants to be slave
will be called when node has requested it be master (by sending a NULL
while this code is master)
After it has completed its service it will send a
rtGSIRS232Message, MessageReqSlave
    */
		    //rv=Device()->SetMasterSlave(GSIMaster);	//node wants to be slave so we must be master
            rv=Device()->SetMaster();	//node wants to be slave so we must be master
		    if (rv)
		    {
			    wxLogMessage("Failed to set PC as Master");
                wxFAIL_MSG("Failed to set PC as Master");
		    }
		    break;
	    case GSIRS232MessageReqMaster:  //'not used, use GSIRS232MessageMASTERReqBIT
		    break;
	    case GSIRS232MessageSendData:
		    break;
	    case GSIRS232MessageGetData:
		    break;
	    case GSIRS232MessageChangeNode:
		    break;
	    case GSIRS232MessageNewNode:
		    break;
	    case GSIRS232MessageN3:
		    break;
	    default:
		    //MsgBox "Unknown RS232Message", vbExclamation;
		    wxLogMessage("%i %s Unknown RS232 message\n\r",__LINE__,__FILE__);
	    } //switch (subfunction)
	    break;
	    //for simplicity the node should always send an rtGSIR2RXCANMessageNode2PC
	    //as the following messages are the special case when the node that has
	    //had its keypad pressed (or whatever) happens to be attached to the pc
    case rtGSILCDMessage:            //currently never seen as LCD is read only. This might change in future versions
	    break;
    case rtGSIKeypadMessage:
	    switch(record.GetData(0))
	    {
		    //keypad subfunctions
	    case GSICANKeypadMessageKeyUp:
		    mssgStr1 = "Key up: ";
		    mssgStr1 = mssgStr1 + (wxChar) record.GetData(1);
		    break;
	    case GSICANKeypadMessageKeyDown:
		    mssgStr1 = "Key down";
		    mssgStr1 = mssgStr1 + (wxChar) record.GetData(1);
		    break;
	    case GSICANKeypadMessageSetMode:
		    //currently unsupported as a received message
		    //message can be sent
		    mssgStr1 = "Keypad Set mode";
	    case GSICANKeypadMessageGetMode:
		    //currently unsupported as a received message
		    //message can be sent
		    mssgStr1 = "Keypad Get mode";
		    break;
	    default:
		    mssgStr1 = "Keypad Unknown mssg";
	    } //switch(rxFrame.GetBufferValue(1))
	    break;

    case rtGSISPIMessage:
	    break;
    case rtGSII2CMessage:
	    break;
    case rtGSIDIOMessage:
	    break;
    case rtGSIADCMessage:
	    break;
    case rtGSICANPCMaster: //should never be called this is a pc->node command
	    break;
    case rtGSICANMaster:    //should never be called this is a pc->node command
	    break;
    case rtGSICANSendMessage: //should never be called this is a pc->node command
	    break;
    case rtGSICANError:     //node has received an error
	    break;
    case rtGSICANSetSlaveIDReq:
	    break;
    case rtGSICANSlaveGetSlaveIDReq:
	    break;
	    //
    case rtRS232SetMaster://Receiver must become Master if possible
	    break;
    case rtRS232ReqMaster:   //75 receiver must be become slave if possible
	    break;
/*-----------------------------------------------------------------------------------
Above functions will all be fielded by the node object attached to this pc
if a CAN frame is returned then we need to find its origin node and call that node object
selected from the NodeArray[]. This will maintain node specific states
------------------------------------------------------------------------------------*/
    case rtGSIR2RxCANMessageNode2PC:   //Node2PC=Node->Pc. Node has a CAN message for the pc
#if ADD_ASYNC_NODE_IN_GSIFRAME    //now handled by Frame().DefaultPricessAsyncRecord() added 12/4/12
        wxFAIL_MSG("This should be handled in GSIFrame()");
#else
    //A can message
	    rv=record.MakeGSICANFrame(rxFrame);
	    if(rv)
	    {
		    wxLogMessage("%i %s Record2Frame error\n\r",__LINE__,__FILE__);
		    return(rv);
	    }
//int n=rxFrame.GetOriginNode();
//Frame()->
//note the id also contains the CANNode of the receiver (attached to this device) as base 4096
//ie CANNode 0=0, CANNode 1=4096, CANNode 2= 8192
//if CANID was 1 then 4097 would be an ID of 1 can frame
//to CANNode 1, caught here, therefore our node

        //A node can send us a message before we know it is online, before we have allocated a Node for it
        GSIChar n=rxFrame.FindOriginNode();

        if(Frame()->Node(n) == NULL)        //Don't have a node object for this hardware
        {
            //Frame()->AddNode(n);            //so create one
            Frame()->CreateNode(GetDeviceHandle(),n);            //so create one
        }
		wxASSERT_MSG(Frame()->Node(n) != NULL,"No node allocated");
        Frame()->Node(n)->DefaultProcessAsyncCANFrame(rxFrame);
        break;
#endif

    }   //End of case r.Type()
    return(rv);
}


/**************************************************************************
    errorType GSINode::DefaultProcessAsyncCANFrame(GSICANFrame & rxFrame)
**************************************************************************/
errorType GSINode::DefaultProcessAsyncCANFrame(GSICANFrame & rxFrame)
{
errorType rv=errNone;
wxString  mssgStr1;
GSIChar n=GSI_NODE_THIS;


    switch(rxFrame.GetGSIID())
    {
    case GSICANIDSync:      //Only see this if the device is attached to a slave node
	    //rv=Write(GSIAck); //??no echo required??
	    //Write rtAck is treated as a record! we need a simple GSIChar write
	    //GSI.ComPort.Output = Chr(rtGSIACK)]
	    //GSI.TxtRx.Text = GSI.TxtRx.Text + "CAN Sync" + Constants.vbCrLf;
        rxFrame.SetIDString(_("GSICANIDSync"));
	    rv = errNone;
	    break;

    case GSICANIDCriticalError:
        rxFrame.SetIDString(_("GSICANIDCriticalError"));    //buff[0]=OriginNode, buff[1]=error code
	    break;

    case GSICANIDTimestamp:
        rxFrame.SetIDString(_("GSICANIDTimestamp"));
	    break;

    case	GSICANIDGetSetSlaveID:      //only received slave->master, if we receive this the pc is attached to the Master node
    /*You will receive this message when a slave node attempts to register itself with the master.
    When the master receives the GetSetSlaveID it will send the ID back to the new slave and forward the
    CAN mssg to any pc attached to the master.
	    */
        rxFrame.SetIDString(_("GSICANIDGetSetSlaveID"));
	    break;

    case GSICANIDTerminalCommand:
        rxFrame.SetIDString(_("GSICANIDTerminalCommand"));
	    break;

    case GSICANIDRS232Message:
	    wxLogMessage("CAN RS232 Message %i %s\n\r",__LINE__,__FILE__);
	    //GSI.TxtRx.Text = GSI.TxtRx.Text + "CAN RS232 MSSG" + Constants.vbCrLf;
	    rv = errNone;
	    break;

    case GSICANIDLCDMessage:
	    //GSI.TxtRx.Text = GSI.TxtRx.Text + "CAN LCD MSSG" + Constants.vbCrLf;
	    wxLogMessage("CAN LCD message %i %s\n\r",__LINE__,__FILE__);
	    rv = errNone;
	    break;

    case GSICANIDKeypadMessage:
	    //movff CnThisCANNode, CnTxBuff
	    //movlw CANKDMessageKeyUp
	    //movff WREG, CnTxBuff + 1
	    //movff KdCurrentKey, CnTxBuff + 2

	    wxLogMessage("CAN keypad message %i %s\n\r",__LINE__,__FILE__);
	    //switch(rxFrame.GetBufferValue(1))
	    switch(GSICANFrameOffsetSubfunction)
	    {
		    //keypad subfunctions
	    case GSICANKeypadMessageKeyUp:
		    mssgStr1 = "Key up: ";
		    mssgStr1 = mssgStr1 + (wxChar) rxFrame.GetBufferValue(GSICANFrameOffsetSubfunction+1);
		    mssgStr1=mssgStr1+ " node ";
		    mssgStr1= mssgStr1+ (wxChar) (rxFrame.GetOriginNode()+0x30);
		    break;
	    case GSICANKeypadMessageKeyDown:
		    mssgStr1 = "Key down";
		    mssgStr1 = mssgStr1 + (wxChar) rxFrame.GetBufferValue(GSICANFrameOffsetSubfunction+1);
		    break;
	    case GSICANKeypadMessageSetMode:
		    //currently unsupported as a received message
		    //message can be sent
		    mssgStr1 = "Keypad Set mode";
		    break;
	    case GSICANKeypadMessageGetMode:
		    //currently unsupported as a received message
		    //message can be sent
		    mssgStr1 = "Keypad Get mode";
		    break;
	    default:
		    mssgStr1 = "Keypad Unknown mssg";
	    } //switch(rxFrame.GetBufferValue(1))

	    //GSI.TxtRx.Text = GSI.TxtRx.Text + "CANNode(" + CStr(rxFrame.GetBuffer(0)) + ") " + mssgStr1 + Constants.vbCrLf;
	    wxLogMessage("CANNode(%x %s) \n\r",(GSIChar) rxFrame.GetBufferValue(0)+'0',mssgStr1.c_str());
	    rv = errNone;
        break; //case GSICANIDKeypadMessage::

    case GSICANIDSPIMessage:
	    //GSI.TxtRx.Text = GSI.TxtRx.Text + "CAN SPI MSSG" + Constants.vbCrLf;
	    wxLogMessage("CAN SPI MSSG\n\r");
	    rv = errNone;
	    break;

    case GSICANIDI2CMessage:
	    //	  GSI.TxtRx.Text = GSI.TxtRx.Text + "CAN I2C MSSG" + Constants.vbCrLf;
	    wxLogMessage("CAN I2C MSSG\n\r");
	    rv = errNone;
	    break;

    case GSICANIDDIOMessage:
	    //GSI.TxtRx.Text = GSI.TxtRx.Text + "CAN DIO MSSG" + Constants.vbCrLf;
	    wxLogMessage("CAN DIO MSSG\n\r");
	    rv = errNone;
	    break;

    case GSICANIDADCMessage:
	    //	  GSI.TxtRx.Text = GSI.TxtRx.Text + "CAN ADC MSSG" + Constants.vbCrLf;
	    wxLogMessage("CAN ADC MSSG\n\r");
	    rv = errNone;
	    break;

    case GSICANIDIOCTLMessage:
	    //GSI.TxtRx.text = GSI.TxtRx.text + "CAN IOCTL MSSG"
	    mssgStr1 = "CAN IOCTL MSSG ";
	    switch ( rxFrame.GetBufferValue(1))
	    {
	    case CANIOCTLGetNodeNode2Master:
		    mssgStr1 = "Get Node (CANNode=" + rxFrame.GetBufferValue(2)+'0';
		    break;
	    case CANIOCTLGetModeNode2Master:
		    mssgStr1 = "Get Mode (Mode=" + rxFrame.GetBufferValue(2)+'0';
		    break;
	    case CANIOCTLSetModeNode2Master:
		    mssgStr1 = "Set Mode (Prev Mode=" + rxFrame.GetBufferValue(2)+'0';
		    break;
	    default:
		    mssgStr1 = "Unknown CAN IOCTL MSSG";
	    } //switch ( rxFrame.GetBufferValue(1))

	    rv = errNone;
		    //	  GSI.TxtRx.Text = GSI.TxtRx.Text + mssgStr1 + ")" + Constants.vbCrLf;
	    wxLogMessage("%s\n\r ",mssgStr1.c_str());
	    break;

    case GSICANIDGSIIOCTLMessage:
		    //GSI.TxtRx.text = GSI.TxtRx.text + "CAN IOCTL MSSG"
	    //  mssgStr1 = GSI.TxtRx.Text + "CAN GSI IOCTL MSSG ";
	    wxLogMessage("CAN GSI IOCTL MSSG\n\r");
	    switch( rxFrame.GetBufferValue(1))
	    {
	    case CANGSIIOCTLGetRegNode2Master:
		    mssgStr1 = "Get CANNode: CANNode=" + rxFrame.GetBufferValue(2)+'0';
		    break;
	    case CANGSIIOCTLSetRegNode2Master:
		    mssgStr1 = "Get Mode: Mode=" + rxFrame.GetBufferValue(2)+'0';
		    break;
	    case CANGSIIOCTL1Node2Master:
		    mssgStr1 = "Set Mode: Prev Mode="+ rxFrame.GetBufferValue(2)+'0';
		    break;

	    default:
		    mssgStr1 = "Unknown CAN GSI IOCTL MSSG";
	    }//switch( rxFrame.GetBufferValue(1))

	    rv = errNone;
	    //GSI.TxtRx.Text = GSI.TxtRx.Text + mssgStr1 + ")" + Constants.vbCrLf;
	    wxLogMessage("%s\n\r",mssgStr1.c_str());
	    break;

    case GSICANIDErrorMessage:
	    GSIChar temp;
	    temp=rxFrame.GetBufferValue(1);

	    switch(rxFrame.GetBufferValue(1))		//the id of the message str. See defs.inc
	    {
	    case VersionMessage:
		    break;
	    case WDTResetMessage:
		    break;
	    case BrownoutResetMessage:
		    break;
	    case UnknownResetMessage:
		    break;
	    case UserAbortResetMessage:
		    break;
	    case PowerOnResetMessage:
		    break;
	    case StackOverflowResetMessage:
		    break;
	    case StackUnderflowResetMessage:
		    break;
	    case ErrorI2CNoACKFromSlaveMessage:
		    break;
	    case ErrorI2CNoDataMessage:
		    break;
	    case ErrorI2CBusCollisionMessage:
		    break;
	    case UnkownErrorMessage:
		    break;
	    case CANEWarnMessage:
		    break;
	    case CANRxWarnMessage:
		    break;
	    case CANTxWARNMessage:
		    break;
	    case CANRxBPMessage:
		    break;
	    case CANTxBPMessage:
		    break;
	    case CANTxBOMessage:
		    break;
	    case CANRxB1OVFLMessage:
		    break;
	    case CANRxB0OVFLMessage:
		    break;
	    case CANIRXIFMessage:
		    break;
	    case CANErrorInitMessage:
		    break;
	    case UnknownCANErrorMessage:
		    break;
		    /*Should never see any of these. Used to write ascii CAN messages to rs232*/
	    case CANMessageString:  //;23 About to write a CAN mssg
	    case CANMessageIDString:  //;24 mssg Id str
	    case CANMessageDataString: //;25 mssg Data str
	    case CANMessageLengthString: //;26 mssg Data Length str
	    case CANMessageFlagString: // ;27 mssg Flag str
		    break;
	    case CANMessageSetSlaveIDString: // ;28 mssg Set id of slave
		    break;
	    case R2ErrorEchoTxString:
		    break;
	    case CANRxCQOverflowMessage:
		    break;
	    default:
		    mssgStr1 = "Unknown CAN Error MSSG";
	    }

	    break;  //GSICANIDErrorMessage:

    case GSICANIDPingMessage:
        //Add the node to the NodeArray
#warning ("!! HERE !!")      //only add the node if it is a PING received, can also receive PING Send through a Node()-Device()
        n=rxFrame.GetOriginNode();
        //Frame()->AddNode(n);
#if GSI_SERVER
        Frame()->CreateNode(n);
#endif
        break;
    default:
        rv=errFail;

    } //End of case rxFrame.GetGSIID())
    return(rv);
}

GSIChar GSICANFrame::FindOriginNode()
{
//Some frames return the node of the sender in buff[0]. Others do not.
//pic probably needs work to ensure that all nodes return their sender as a node byte in buff[0]
//this function is a stopgap
GSIChar n=GSI_NODE_THIS;

    switch( GetGSIID())
    {
    case GSICANIDSync:                  //n but always node 0
        n=GSI_NODE_MASTER;
        break;
    case GSICANIDCriticalError:         //y
    case GSICANIDKeypadMessage:     //y
    case GSICANIDErrorMessage:      //y
    case GSICANIDPingMessage:       //y
        n=GetOriginNode();
        break;

    case GSICANIDTimestamp:             //n
    case GSICANIDGetSetSlaveID:      //n Only received if this is a master
    case GSICANIDTerminalCommand: //?
    case GSICANIDRS232Message:      //n but should!
    case GSICANIDLCDMessage:        //n shouldn't send frames, should only recevie them
    case GSICANIDSPIMessage:        //n
    case GSICANIDI2CMessage:        //n
    case GSICANIDDIOMessage:        //n
    case GSICANIDADCMessage:        //n
    case GSICANIDIOCTLMessage:      //n
    case GSICANIDGSIIOCTLMessage:   //n
    default:
        n=GSI_NODE_THIS;
    } //End of case rxFrame.GetGSIID())

    return(n);        //by default use the node attached to this device
}



GSIHandle GSINode::GetHdwNode() const
{
    return((GSIHandle)m_hdwNode);
}

const GSIHandle GSINode::GetNodeHandle() const
{
    return(m_handle);
}

void    GSINode::SetHdwNode(GSIHandle hnode)
{
    wxASSERT(hnode >=0 && hnode < GSIMaxNumPhysicalNodes+GSIMaxNumSpecialNodes);

    m_hdwNode= (GSIChar) hnode;
}



/*******************************************************************************
    GSINetwork::GSINetwork()
        ctor
*******************************************************************************/
GSINetwork::GSINetwork()
{
//void *memset( void *dest, int c, size_t count );

    memset(&m_Node[0],0,sizeof(GSINode *)*GSIMaxNumNodes);
    m_arrayFilled=false;
}

/*******************************************************************************
    GSINetwork::~GSINetwork()
        dtor
*******************************************************************************/

GSINetwork::~GSINetwork()
{
}

/*******************************************************************************
    GSINetwork::GetNode(int index, GSINode *node)
Get node[index]
*******************************************************************************/

errorType GSINetwork::GetNode(int index, GSINode *node) const
{
    if(IsFilled())
    {
        node=m_Node[index];
        return errNone;
    }
    return errFail;
}

/*******************************************************************************
    GSINetwork::SetNode(int index, GSINode *node)
Get node[index]
*******************************************************************************/

errorType GSINetwork::SetNode(GSIHandle handle, GSINode *node)
{
    wxASSERT(handle <0 || handle >= GSIMaxNumNodes);

    if(handle <0 || handle >= GSIMaxNumNodes)
        return errBounds;

    node=m_Node[handle];
    return errNone;
}

/*******************************************************************************
    GSINetwork::GetNodeArray(GSINode **node[GSIMaxNumNodes])
return the node array
*******************************************************************************/

errorType GSINetwork::GetNodeArray(GSINode *node[GSIMaxNumNodes])
{
    node=m_Node;
    return errNone;
}

#if 0
void GSINetwork::SetNodeArray(GSINode **node[GSIMaxNumNodes])
{
    m_Node=node;
}
#endif
#if 0
{
public:
    GSINetwork();
    virtual ~GSINetwork();
    SetNodeArray(const GSINode *node) {m_Node=node;}
    errorType GSINode * GetNode(int index);
private:
    GSINode			*m_Node[GSIMaxNumNodes];	//a pointer for all possible nodes on the network
};
#endif
