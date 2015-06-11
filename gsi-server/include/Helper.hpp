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
#ifndef HELPER_HPP_INCLUDED
#define HELPER_HPP_INCLUDED


#include "gsidefs.h"

#define UNUSED(x) ((void)(x));

/*!
************************************************************************
  class  GSIDevices
Helper function to obtain:
ID
device name
device handle

    device types:
GSI_DEVICE_RS232,
GSI_DEVICE_USB,
GSI_DEVICE_CAN,            //internal CAN card
GSI_DEVICE_USER,
GSI_DEVICE_NULL
***********************************************************************/

class GSIDevices
{
public:
	GSIDevices ();
	~GSIDevices();
	GSIDevHandle	GetDeviceHandle(const wxString &dev, int *dev_type=NULL) const;
	GSIDevHandle	GetDeviceHandle(int id) const;
	wxString		GetDeviceString(GSIDevHandle);
	wxString		GetDeviceString(int id);
	int				GetDeviceId(const wxString &dev);

private:
};



#endif //CAN_HPP_INCLUDED


