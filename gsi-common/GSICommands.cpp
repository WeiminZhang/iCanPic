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

/*----------------------------------------------------------------------------------------
Use this code to establish the type of frame to instantiate and initialise

With this method it is possible to have a hierarchy of classes that inherit from
the GSIFrame type. The  'new  xxxFrame' in bool MyApp::OnInit() determines which
class will actually be instantiated.
If required #defines could be used to exclude code from unused frames.
The linker will probably do this in any case
-----------------------------------------------------------------------------------------*/

#if defined(__GNUG__) && !defined(__APPLE__)
#  pragma implementation "GSICommands.cpp"
#  pragma interface "GSICommands.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#  pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
#  include "wx/wx.h"
#endif


#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#endif


//#include "include/gsi.hpp"
#include "gsi-common/include/helper.hpp"
#include "include/gsbexcept.hpp"
#include "include/gsiCommands.hpp"
#include "include/microcontroller.h"


#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#endif


/*!
GSICommandXXX::GSICommandXXX()
	ctor
Base class for commands.
Some will be sent to the hardware with a Write() to a Device()
Others will be dispatched internally. Eg. opening a device.
This ctor allows user to send a string that will be sent to
StatusText (wxTextCtrl) when the record is written
*/

GSICommandXXX::GSICommandXXX(wxTextCtrl * const status_text)
:m_StatusText(status_text)
{
	Init();
}
/*!
	dtor
*/
GSICommandXXX::~GSICommandXXX()
{
}

/*!
Set variables to sensible values
*/

void GSICommandXXX::Init()
{
	GSIRecord&r=GetGSIRecord();
	r.SetType(NULL);
	r.SetNode(GSI_NODE_THIS);
	r.SetLength(0);
	SetStatusText(NULL);
	SetDevice(NULL);
	SetCommandString("");
	SetStatusReportLevel(GSIStatusReportHigh);
	SetIsWritable(false);
	SetErrorString("No error!");

}

/*!
errorType GSICommandXXX::Write()
Default write is to send the GSIRecord
*/

errorType GSICommandXXX::Write()
{
errorType rv;
wxString statusStr;
wxString outStr;

	if(GetDevice())
	{
		if(IsWritable())
			rv=GetDevice()->Write(GetGSIRecord());
		else
			rv=errAccess;
	}
	else
		rv=errBadDevice;
	if(rv)
		{
			ErrorClass err(rv);
			statusStr.Printf(_("Write %s Failed\n %s"),GetCommandString().c_str(),err.GetString().c_str());
			AppendText(statusStr);
		}
	return rv;
}



/*!
*/
errorType GSICommandXXX::FillGSIRecord()
{
	wxFAIL_MSG("Called base class FillGSIRecord, virtual function missing?");
	return errNotImplemented;
}



/*!
GSICommandXXX::AppendText(const wxString & text)
*/

void GSICommandXXX::AppendText(const wxString & text)
{
	switch(GetStatusReportLevel())
	{
	case GSIStatusReportLow:
	case GSIStatusReportMedium:
	case GSIStatusReportHigh:
		if(GetStatusText())
		{
			GetStatusText()->AppendText(text);
		}
		break;
	default:
	case GSIStatusReportNone:
		break;
	}
}


/*!
errorType  GSICommandXXX::Execute()

Default behaviour is to do nothing
*/
errorType  GSICommandXXX::Execute(GSIFrame &Frame,GSISocketReturnData &sr)
{
	sr;
	Frame;
	return errNone;
}


/*!
errorType GSICommandXXX::SetNode(GSINodeHandle node)
*/
errorType GSICommandXXX::SetNode(GSINodeHandle node)
{
	if(node <0 || node > GSIMaxNumNodeHandles)
		return errBounds;
	GetGSIRecord().SetNode(node);
	return errNone;
}


/*!
errorType GSICommandXXX::Read(wxSocketBase &sock)
*/
errorType GSICommandXXX::Read(wxSocketBase &sock)
{
	sock;
	wxFAIL_MSG("Called GSICommandXXX::Read. Need to implement?" );
	return errNotImplemented;
}

//------End GSICommandXXX-------------------


/*!

***************************************************************************************************************
		ctor
GSICommandOpenDeviceXXX(const wxString & dev_str="")
Don't know how to utilise usb as yet, so will assume that the device will have a string "GSIUSBxxx"
Think that we might need to open a generic data communication device, passing it our id (which might be the GSIUSBxxx"
string
Currently expect to receive GSIUSBxxx, will attempt to parse it but do nothing else for the present

This function will just fill in the handle and filename information for the device
The open will be performed by the GSIServer code, calling in to GSIFrame
****************************************************************************************************************/
GSICommandOpenDeviceXXX::GSICommandOpenDeviceXXX(const wxString & dev_str)
{
wxString s;

GSIDevices Devices;
	SetDeviceStr(dev_str);

    m_handle=Devices.GetDeviceHandle(dev_str);
}

/*!
	dtor
*/

GSICommandOpenDeviceXXX::~GSICommandOpenDeviceXXX()
{
}

/*!
GSICommandOpenDevice::FillGSIRecord()
	//No need to fill the record, it will not be sent to the hardware
	//just need this code as a placeholder
*/
errorType GSICommandOpenDeviceXXX::FillGSIRecord()
{
	//No need to fill the record, it will not be sent to the hardware
	//just need this code as a placeholder
	return errNone;
}

//-----End GSICommandOpenDeviceXXX---------------



/*!
***************************************************************************************************************
GSICommandCloseDeviceXXX()
	ctor
Don't know how to utilise usb as yet, so will assume that the device will have a string "GSIUSBxxx"
Think that we might need to open a generic data communication device, passing it our id (which might be the GSIUSBxxx"
string
Currently expect to receive GSIUSBxxx, will attempt to parse it but do nothing else for the present

This function will just fill in the handle and filename information for the device
The open will be performed by the GSIServer code, calling in to GSIFrame
****************************************************************************************************************/
GSICommandCloseDeviceXXX::GSICommandCloseDeviceXXX(GSIDevHandle h)
{
    m_handle=h;
}

/*!
	dtor
*/
GSICommandCloseDeviceXXX::~GSICommandCloseDeviceXXX()
{
}


/*!
GSICommandCloseDevice::FillGSIRecord()
	//No need to fill the record, it will not be sent to the hardware
	//just need this code as a placeholder
*/
errorType GSICommandCloseDeviceXXX::FillGSIRecord()
{
	return errNone;
}


/*!
* errorType GSICommandCloseDeviceXXX::SetDeviceHandleXXX(GSIHandle h)
*/

errorType GSICommandCloseDeviceXXX::SetDeviceHandle(GSIHandle h)
{
errorType rv=errNone;

    m_handle=GSINotAValidDevHandle;

    if(h>0 && h<GSIMaxNumDeviceHandles)
    {
        m_handle=h;
    }
	else
	    rv=errRange;

    return rv;
}


//------End GSICommandCloseDeviceXXX -----------------------

/*!
***************************************************************************************************************
GSICommandAddNodeXXX::GSICommandAddNodeXXX()
	ctor
Don't know how to utilise usb as yet, so will assume that the device will have a string "GSIUSBxxx"
Think that we might need to open a generic data communication device, passing it our id (which might be the GSIUSBxxx"
string
Currently expect to receive GSIUSBxxx, will attempt to parse it but do nothing else for the present

This function will just fill in the handle and filename information for the device
The open will be performed by the GSIServer code, calling in to GSIFrame

Though we have a GSIDevHandle, we currently only allow one device per server
****************************************************************************************************************/
GSICommandAddNodeXXX::GSICommandAddNodeXXX(GSINodeHandle h,GSIDevHandle hd)
{

    m_hnode=h;
	m_hdev=hd;
}

/*!
	dtor
*/
GSICommandAddNodeXXX::~GSICommandAddNodeXXX()
{
}


/*!
GSICommandAddNode::FillGSIRecord()
*/
errorType GSICommandAddNodeXXX::FillGSIRecord()
{
	return errNone;
}

/*!
***************************************************************************************************************
GSICommandRemoveNodeXXX()
	ctor
Don't know how to utilise usb as yet, so will assume that the device will have a string "GSIUSBxxx"
Think that we might need to open a generic data communication device, passing it our id (which might be the GSIUSBxxx"
string
Currently expect to receive GSIUSBxxx, will attempt to parse it but do nothing else for the present

This function will just fill in the handle and filename information for the device
The open will be performed by the GSIServer code, calling in to GSIFrame

Though we have a GSIDevHandle, we currently only allow one device per server
****************************************************************************************************************/
GSICommandRemoveNodeXXX::GSICommandRemoveNodeXXX(GSINodeHandle h,GSIDevHandle hd)
{

    m_hnode=h;
	m_hdev=hd;
}

/*!
	ctor

*/
GSICommandRemoveNodeXXX::GSICommandRemoveNodeXXX()
{
	m_hnode=GSI_NODE_THIS;
	m_hdev=GSINotAValidDevHandle;
}

/*!
	dtor
*/
GSICommandRemoveNodeXXX::~GSICommandRemoveNodeXXX()
{
}

/*!
GSICommandRemoveNodeXXX::FillGSIRecord()
*/
errorType GSICommandRemoveNodeXXX::FillGSIRecord()
{
	return errNone;
}


//------------End GSICommandRemoveNodeXXX ---------------

