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
#  pragma implementation "device.cpp"
#  pragma interface "device.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
//#include "wx/wxprec.h"
#include "wx/wx.h"


#ifdef __BORLANDC__
#  pragma hdrstop
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
#include <wx/socket.h>
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

//#include <wx/ctb/serport.h>

//#include "gsi.hpp"


#include "include/gsi.hpp"
#include "include/device.hpp"
#include "include/GSICq.hpp"

#include "include/microcontroller.h"
#include "include/record.hpp"
#include "include/gsidefs.h"
#include "gsExcept.hpp"


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


BEGIN_EVENT_TABLE(GSIDevice,wxEvtHandler)

END_EVENT_TABLE()



/*--------------------------------------------------------------------------------------------
    ctor
GSIDevice::GSIDevice(GSIFrame *gsi_frame, wxWindow * parentL,wxEvtHandler *custom_event_handler)
---------------------------------------------------------------------------------------------*/
GSIDevice::GSIDevice(const wxString &device_name,GSIHandle handle,GSIFrame *gsi_frame,wxEvtHandler *custom_event_handler)
:m_handle(handle)
{
    m_isOpen=false;
	SetFrame(gsi_frame);
    if(custom_event_handler)
	    m_CustomEventHandler=custom_event_handler;
    else
#ifdef GSI_CLIENT
#else
        //m_CustomEventHandler=(wxEvtHandler *)gsi_frame;		//had to put the cast in 27/814
    	m_CustomEventHandler=gsi_frame;
#endif

    if(m_CommsMutex.IsOk()==false)
    {
        GSI_THROW("Failed to obtain Comms mutex",errFail);
        wxFAIL_MSG("Failed to obtain Comms mutex");
    }
    m_DeviceName=device_name;
    m_Device=NULL;
    m_controllerHasReset=false;         //if controller resets, the Break Handler will indirectly set this variable
    m_AsyncQ=DEBUG_NEW GSICQueue(GSIDefaultQueueSize);     //

    int devType;        //returns a constant of the type of device (COM, USB etc)

    //XXXGS mod 7/4/11 use handle sent to ctor set in ctor init list
    //m_handle=Frame()->GetDeviceHandle(device_name, &devType);
}

/*******************************************************************
		dtor
		GSIDevice::~GSIDevice()
********************************************************************/

GSIDevice::~GSIDevice()
{
	//delete m_DefaultAsyncQ;
wxString s;
    s.Printf(_("Closing device %s\n"),m_DeviceName.c_str());
#ifdef GSI_CLIENT
#else
    Frame()->TextCtrl()->AppendText(s);
#endif
    SetMaster();                                    //make us master, shouldn't matter the device is closing, but want to keep mc happy as well
    delete m_AsyncQ;
}


void		GSIDevice::SetCustomEventHandler(wxEvtHandler * custom_event_handler)
{
	m_CustomEventHandler=custom_event_handler;
}


/********************************************************************
//   errorType GSIDevice::ReSynchNode(GSIChar *recd)
//
//A null was recd as the Type field of a record. This can be a
//request by the pic to be Master or attempt by the pic to re-synch
//after an error.
//if it is a re-synch it will send a stream of nulls
//if it is a Master req it should echo the Type field we have just sent

//IMPORTANT NOTE.
Assumes the SerialThread is paused
;--------------------------------------------------------
;	RS232SendReSyncNulls
;Modifies R2BTemp
;if comms problem we will send R2_MAX_RECORD_LEN*2+1 nulls
;so that pc can re-synchronise to us.
;It should ensure that at least one null record is read
;by the pc which it recognises as a re-sync
;Should also do whatever is required to place rs232 into
;a known state.
;Perhaps should do an init??
;--------------------------------------------------------

RS232SendReSyncNulls
	banksel	R2BTemp
	call	GSIInitRS232		;init everything

	movlw	(R2_MAX_RECORD_LEN*2)+1
	movwf	R2BTemp
	clrf	WREG
rsrsn:
	call	GSISendByte232
	decf	R2BTemp,F
	bnz	rsrsn

	movlw	ACK		;send an ACK when we are through
	call	GSISendByte232
rsrsn1:
	call	GSIGetFromRS232RxBuff	;wait for an echo
	bz	rsrsn1		;wdt on error
	clrwdt

	call	GSIFlushR2RxBuffer
	return
*******************************************************************/
errorType GSIDevice::ReSynchNode(GSIChar *recd, int *null_count)
{
wxStopWatch sw;
errorType rv;

    do
	{
	    rv=Read(recd);    //can't use this, must use direct read of the device, the serial thread is paused

		if(rv==errNone)
		{
			//have recd a byte
			if(*recd == '\0')
			{
				(*null_count)++;
				sw.Start(0);
				continue; //restart the loop, looking for another null
			}
            //Here if a non-null was received, if nullcount>1 then this is either a resync or an error
			if(*recd==GSI_ACK)       //if a resync the char should be a GSI_ACK
			{
				rv=Write(GSI_ACK,recd,GSIEchoTimeout);		//pic node will wait for this
				if(rv)	//echoError or timeout are both fatal
					return(rv);

                return(errGSIReSync);        //hopefully a recovery from a resynch
			}
            if((*null_count) >1)
			    return(errFail);
            else
                return(errGSISlaveReq);        // a single null, node wants to be master

		}
	}while(sw.Time() < GSIEchoTimeout); //only pause for 1mSec between receipt of nulls


	return(errTimeout);
}


/******************************************************************************************
errorType GSIDevice::AsyncRecordReceived(GSIRecord &record)
*******************************************************************************************/
errorType GSIDevice::AsyncRecordReceived(GSIRecord &record)
{
	record;
		return(errNone);
}

//**************************************************************************************************
//	errorType GSIDevice::DefaultProcessAsyncRecord(GSIRecord &record )
//
//
//the node has sent us some data. This can be from a remote node or the
//'node attached to the pc. It will probably most frequently be a SYNC
//from the CAN master (which may or may not be attached to the pc)
//After receipt of "first" message from the node it will continue sending
//till it has sent what it wishes
//then send a REQUEST_SLAVE, (which will cause GSIRS232SetPCRS232Master (sic) to be executed)
//this code will then re-take control

//This is the default handler. Must also see if the user has provided us with a function *
//if so then we should call the user function * first. If the user wants to field the function
//then we can ignore it. Otherwise the user can request that we handle it with the default handler
//if the user code crashes then we crash too!
//Perhaps we should simply stuff the record into a user space Q and write it
//to a pipe.
//can VB use pipes? Would like this dll to operate with VB

//Initially set a flag that the user space can see. User can then call a set of functions for
//reading the record
//************************************************************************************************



errorType GSIDevice::DefaultProcessAsyncRecord(GSIRecord &record )
{

    errorType rv=errNone;     //Handled in GSIFrame::DefaultProcessAsyncRecord
    record;
#if 0


  int subFunction;

  wxString  mssgStr1;

  const GSIBINRECORD &r=record.GetBinRecord();
  GSICANFrame frame,rxFrame;
  //DebugWriteRecord(record);
  switch (r.type)
	{

	case rtGSIReserved1:
		break;
	case rtGSIReserved2:
		break;
	case rtGSIRS232Message:
		subFunction = r.data[0];
		switch( subFunction)
		{
		case GSIRS232MessageSuccess:
			break;
		case GSIRS232MessageFailure:
		break;
		case GSIRS232MessageReqSlave:
		/*
		node wants to be slave
		will be called when node has requested it be master (by sending a NULL
		while this code is master)
		After it has completed its service it will send a
		rtGSIRS232Message, MessageReqSlave
			*/
			rv = errNone;
			rv = SetMasterSlave(GSIMaster);	//node wants to be slave so we must be master
			if (rv == errNone)
			{
				//the ACKis echoed by the calling function
				//GSI.ComPort.Output = Chr(rtGSI_ACK)
				//Write((GSIChar) GSI_ACK);
			}
			else
			{
				//GSI.ComPort.Output = Chr(rtGSIError)
				//Write((GSIChar) GSIError);

			}
			break;
		case GSIRS232MessageReqMaster://'not used, use GSIRS232MessageMASTERReqBIT
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
	case rtGSIR2RxCANMessageNode2PC:   //Node2PC=Node->Pc. Node has a CAN message for the pc
		//A can message
		rv=record.MakeGSICANFrame(rxFrame);
		if(rv)
		{
			wxLogMessage("%i %s Record2Frame error\n\r",__LINE__,__FILE__);
			return(rv);
		}
		//note the id also contains the CANNode as base 4096
		//ie CANNode 0=0, CANNode 1=4096, CANNode 2= 8192
		//if CANID was 1 then 4097 would be an ID of 1 can frame
		//from CANNode 1
		switch( rxFrame.GetGSIID())
		{

		case GSICANIDSync:
			//rv=Write(GSI_ACK); //??no echo required??
			//Write rtAck is treated as a record! we need a simple GSIChar write
			//GSI.ComPort.Output = Chr(rtGSI_ACK)]
			//GSI.TxtRx.Text = GSI.TxtRx.Text + "CAN Sync" + Constants.vbCrLf;
            rxFrame.SetIDString(_("GSICANIDSync"));
			rv = errNone;
			break;

		case GSICANIDCriticalError:
            rxFrame.SetIDString(_("GSICANIDCriticalError"));
			break;

		case GSICANIDTimestamp:
            rxFrame.SetIDString(_("GSICANIDTimestamp"));
			break;

		case	GSICANIDGetSetSlaveID:
		/*You will receive this message when a slave node attempts to register itself with the master.
		When the master receives the GetSetSlaveID it will send the ID back to the new slave and forward the
		CAN mssg to any pc attached to the master.
			*/
            rxFrame.SetIDString(_("GSICANIDGetSetSlaveID"));
			break;

		case 	GSICANIDTerminalCommand:
            rxFrame.SetIDString(_("GSICANIDTerminalCommand"));
			break;

		case GSICANIDRS232Message:
			wxLogMessage("CAN RS232 Message\n\r",__LINE__,__FILE__);
			//GSI.TxtRx.Text = GSI.TxtRx.Text + "CAN RS232 MSSG" + Constants.vbCrLf;
			rv = errNone;
			break;

		case GSICANIDLCDMessage:
			//GSI.TxtRx.Text = GSI.TxtRx.Text + "CAN LCD MSSG" + Constants.vbCrLf;
			wxLogMessage("CAN LCD message\n\r",__LINE__,__FILE__);
			rv = errNone;
			break;

		case GSICANIDKeypadMessage:
			//movff CnThisCANNode, CnTxBuff
			//movlw CANKDMessageKeyUp
			//movff WREG, CnTxBuff + 1
			//movff KdCurrentKey, CnTxBuff + 2
			wxLogMessage("CAN keypad message\n\r",__LINE__,__FILE__);
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
			wxLogMessage("CANNode(%x %s) /n/r",(GSIChar) rxFrame.GetBufferValue(0)+'0',mssgStr1.c_str());
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
            break;
	    default:
			rv=rv;
		} //End of case rxFrame.GetGSIID())
    }   //End of case r.Type()
#endif
		return(rv);
}



AckHelper::AckHelper(GSIDevice *dev)
{
    m_ackFlag=false;
    m_Dev=dev;
}

AckHelper::~AckHelper()
{
    if(m_ackFlag==false)
        WriteAck();
}

void AckHelper::WriteAck()
{
    if(m_ackFlag==false)
    {
        m_Dev->Write(GSI_ACK);
        m_ackFlag=true;
    }
}



/****************************************************************************************************************
    GSIDevice::SetMaster()
Set master requires that we wait for the CommsMutex to unlock. It will be locked if the comms device is
currently receiving an async record from the mc. Can also be locked if we are Txing a record, though in this
instance we would already be Master
****************************************************************************************************************/
errorType GSIDevice::SetMaster(GSIChar node)
{
    node;
#if DEBUG_ENABLE_SET_MASTER
    wxASSERT_MSG(node==GSINodeThis,"Need to add node code here!");
    if(IsCommsSlave())
    {
        return errFail;
    }
    m_master=GSIMaster;
#endif
    return errNone;
}

/****************************************************************************************************************
    GSIDevice::SetSlave()
A call to SetSlave indicates that an asyncronous record request has been identified.
We must attempt to obtain the CommsMutex. If we fail then must return immediately as we are currently
Txing a record. The mc will attempt to Tx again
****************************************************************************************************************/

errorType GSIDevice::SetSlave(GSIChar node)
{
    node;
#if DEBUG_ENABLE_SET_SLAVE
    wxASSERT_MSG(node==GSINodeThis,"Need to add node code here!");

    wxMutex &CommsMutex=GetCommsMutex();
    if(CommsMutex.TryLock() == wxMUTEX_NO_ERROR)
    {
        m_master=GSISlave;              //we have the lock, set the variable
        return errNone;
    }
    return errFail;
#else
    return errNone;
#endif

}

void GSIDevice::OnQuit(wxCommandEvent& WXUNUSED(event))
{
    // true is to force the frame to close
    //Close(true);

}
