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
#  pragma implementation "GSIClientCommands.cpp"
#  pragma interface "GSIClientCommands.cpp"
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

#include "include/helper.hpp"

#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#endif




#ifdef __VISUALC__
#pragma warning(pop)
// without this pragma hdrstop, the stupid compiler precompiles #defines below so that
// changing them doesn't "take place" later!
#pragma hdrstop
#endif



/*!
***********************************************************************
	ctor
Helper class to convert filename <--> handle etc.
*/


GSIDevices::GSIDevices()
{
}

GSIDevices::~GSIDevices()
{
}

/************************************************************************
    GSIDevHandle GSIDevices::GetDeviceHandle(const wxString &dev, int *dev_type,bool *is_open)
Helper function to obtain a wxID from the device name

dev_type can be NULL, in which case we ignore, caller only wants the handle
    device types:
GSI_DEVICE_RS232,
GSI_DEVICE_USB,
GSI_DEVICE_CAN,            //internal CAN card
GSI_DEVICE_USER,

GSI_DEVICE_NULL
***********************************************************************/
GSIDevHandle GSIDevices::GetDeviceHandle(const wxString &dev, int *dev_type) const
{
int id=0;

    wxString s=dev.Upper();     //constant string so return a new one (use dev.MakeUpper() on a non-const string)

    if(s=="COM1")
        id= GSI_ID_COM1;
    else if(s=="COM2")
        id= GSI_ID_COM2;
    else if(s=="COM3")
        id= GSI_ID_COM3;
    else if(s=="COM4")
        id= GSI_ID_COM4;
    else if(s=="COM5")
        id= GSI_ID_COM5;
    else if(s=="COM6")
        id= GSI_ID_COM6;

//--------- is it an rs232 device? ------------
    if(id)
    {
        if(dev_type)                        //default to NULL, so can just obtain Handle
            *dev_type=GSI_DEVICE_RS232;
        return (GSIHandle) id-GSI_ID_COM1;
    }


//----------See if usb type--------------------
    if(s=="GSIUSB000")
       id= GSI_ID_USB000;
    else if(s=="GSIUSB001")
        id= GSI_ID_USB001;
    else if(s=="GSIUSB002")
        id=  GSI_ID_USB002;
    else if(s=="GSIUSB003")
        id=  GSI_ID_USB003;
    else if(s=="GSIUSB004")
        id= GSI_ID_USB004;
    else if(s=="GSIUSB005")
        id= GSI_ID_USB005;
    else if(s=="GSIUSB006")
        id=  GSI_ID_USB006;
    else if(s=="GSIUSB007")
        id= GSI_ID_USB007;

//---- is it a usb device ------
    if(id)
    {
        if(dev_type)
            *dev_type=GSI_DEVICE_USB;
        return (GSIHandle) id-GSI_ID_COM1;
    }
//---------not a device type we recognise--------------------
    if(dev_type)
        *dev_type=GSI_DEVICE_NULL;

    return GSINotAValidDevHandle;
}

/*!
GSIDevHandle GSIDevices::GetDeviceHandle(nt id) const
*/
GSIDevHandle GSIDevices::GetDeviceHandle(int id) const
{

	return(id);		//currently these are the same thing
}


wxString	GSIDevices::GetDeviceString(GSIDevHandle h)
{

	int id=h+GSI_ID_COM1;
	return(GetDeviceString(id));
}

/**************************************************************
const wxString & GSIDevices::GetDeviceStringId(int id)
****************************************************************/
wxString GSIDevices::GetDeviceString(int id)
{
wxString s;
    switch (id)
    {
    case GSI_ID_COM1:
        s="COM1";
        break;
    case GSI_ID_COM2:
       s="COM2";
       break;
    case GSI_ID_COM3:
        s="COM3";
        break;
    case GSI_ID_COM4:
        s="COM4";
        break;
    case  GSI_ID_COM5:
        s="COM5";
        break;
    case GSI_ID_COM6:
        s="COM6";
        break;
//----------See if usb type--------------------
    case GSI_ID_USB000:
        s="GSIUSB000";
        break;
    case GSI_ID_USB001:
        s="GSIUSB001";
        break;
    case GSI_ID_USB002:
        s="GSIUSB002";
        break;
    case GSI_ID_USB003:
        s="GSIUSB003";
        break;
    case GSI_ID_USB004:
        s="GSIUSB004";
        break;
    case GSI_ID_USB005:
        s="GSIUSB005";
        break;
    case GSI_ID_USB006:
        s="GSIUSB006";
        break;
    case GSI_ID_USB007:
        s="GSIUSB007";
        break;
    default:
        s="NULL";
        break;
    }

    return(s);
}


/*!int GSIHandle GSIFrame::GetDeviceId(const wxString &dev)
 *Return the id (for use in menus etc.) of a device, given its string
*/
int GSIDevices::GetDeviceId(const wxString &dev)
{

    wxString s=dev.Upper();     //constant string so return a new one (use dev.MakeUpper() on a non-const string)

    if(s=="COM1")
        return GSI_ID_COM1;
    else if(s=="COM2")
        return GSI_ID_COM2;
    else if(s=="COM3")
        return GSI_ID_COM3;
    else if(s=="COM4")
        return GSI_ID_COM4;
    else if(s=="COM5")
        return GSI_ID_COM5;
    else if(s=="COM6")
        return GSI_ID_COM6;

//----------See if usb type--------------------
    if(s=="GSIUSB000")
        return GSI_ID_USB000;
    else if(s=="GSIUSB001")
        return GSI_ID_USB001;
    else if(s=="GSIUSB002")
        return  GSI_ID_USB002;
    else if(s=="GSIUSB003")
        return  GSI_ID_USB003;
    else if(s=="GSIUSB004")
        return GSI_ID_USB004;
    else if(s=="GSIUSB005")
        return GSI_ID_USB005;
    else if(s=="GSIUSB006")
        return  GSI_ID_USB006;
    else if(s=="GSIUSB007")
        return GSI_ID_USB007;


    return -1;
}

