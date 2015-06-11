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
#ifndef NODE_HPP_INCLUDED
#define NODE_HPP_INCLUDED

#include "gsidefs.h"
#include <gerror.h>

//forward references
class GSIFrame;
class wxSocketBase;
class GSIRecord;
class GSICANFrame;
class GSIDevice;
class GSILCD;

class GSINode
{
public:
    //GSINode(GSIFrame * frame, GSIDevHandle d_handle,GSINodeHandle hnode=GSI_NODE_THIS,wxSocketBase *sock=NULL);
    GSINode(GSIFrame * frame, GSIDevice *device,GSINodeHandle hnode=GSI_NODE_THIS,wxSocketBase *sock=NULL);
	//Master node ctor
	//GSINode(GSIFrame * frame,wxSocketBase *sock=NULL,GSIDevice *device=NULL);
    virtual ~GSINode();

    virtual errorType DefaultProcessAsyncRecord(GSIRecord &r);
    virtual errorType DefaultProcessAsyncCANFrame(GSICANFrame & frame);

	GSINodeHandle GetHdwNode() const;
    const GSINodeHandle GetNodeHandle() const;
    void    SetNodeHandle(GSINodeHandle node);

	GSIDevice * const Device() const;	// {return(m_CommDevice);}
	GSILCD * const LCD() const {return(m_Lcd);}   //LCD on this node
    GSIFrame *const Frame() const {return(m_Frame);}

	void	SetNodeDevice(GSIDevice *d);        //currently only an RS232 device is valid
    const GSIDevHandle   GetDeviceHandle() const {return m_deviceHandle;}

    void    SetFrame(GSIFrame *f);

    errorType ReadNode(GSINodeHandle *node);  //use device to talk to node, obtain its CAN node (255, no node or error) check rv for error
                                       //will throw exception on error if GSI_USE_MORE_EXCEPTIONS !=0
	void	SetHdwNode(GSINodeHandle node);

    wxSocketBase & GetSocket() const {return *m_Sock;}

protected:
	void	SetLCD(GSILCD * lcd);
    void    SetSocket(wxSocketBase *sock) {m_Sock=sock;}

private:
	GSIDevice *m_CommDevice;            //the device we are attached to, if any
    wxSocketBase  *m_Sock;        //client. If we receive messages from mc we can send them down this socket
	GSILCD *  m_Lcd;					//the lcd subsytem, if it is present.
    GSIFrame *m_Frame;                  //the parent window
	//simple variables
	GSIChar m_hdwNode;					//the hardware node this code talks to
    GSINodeHandle m_handle;                       //index within nodeArray
    GSIDevHandle   m_deviceHandle;          //index into GSIFrame::m_deviceArray
};


//---------------------------------------------------------------------------------
//             class GSINetwork
//---------------------------------------------------------------------------------
class GSINetwork
{
public:
    GSINetwork();
    virtual ~GSINetwork();
    //void SetNodeArray(GSINode *node[GSIMaxNumNodes]);
    //GSINode * GetNode(int index) {return m_Node[index];}
    errorType GetNode(int index, GSINode *node) const;
    errorType SetNode(GSINodeHandle handle, GSINode *node);
    errorType GetNodeArray(GSINode *node[GSIMaxNumNodes]);
    const bool        IsFilled() const {return m_arrayFilled;}

private:
    GSINode			*m_Node[GSIMaxNumNodes];	//a pointer for all possible nodes on the network
    bool            m_arrayFilled;
};

#endif //NODE_HPP_INCLUDED
