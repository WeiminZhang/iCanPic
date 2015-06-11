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
#ifndef GSI_HPP_INCLUDED
#define GSI_HPP_INCLUDED


//#define DEBUG_GSI_SOCKET_EVENT

//#include <wx/socket.h>
//#include <wx/ctb/serport.h>

//#include <gerror.hpp>
//#include <gsExcept.hpp>     //includes gerror.h gerror.hpp
#include<vector>
using namespace std;
#include "gsidefs.h"
#include "gerror.hpp"
#include "helper.hpp"
#include "include/GSIServerCommands.hpp"

class GSINode;			//forward references
class GSIRecord;
class GSIDevice;
class GSINetwork;
class GSIServer;
class wxSocketBase;
class GSICommandOpenDevice;
class GSICommandCloseDevice;
class GSICommandAddNode;
class GSICommandRemoveNode;

BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE(wxEVT_GSI_MC_BREAK, GSI_ID_MC_RESET)
    //DECLARE_EVENT_TYPE(wxEVT_GSI_SOCKET_COMMAND, GSI_ID_SOCKET_COMMAND)
END_DECLARE_EVENT_TYPES()


//From wxManual:
//it may also be convenient to define an event table macro for this event type
//Remember: no space between end of #define and '(' !!
#define EVT_GSI_MC_BREAK(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_GSI_MC_BREAK, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),

// Define a new frame type: this is going to be our main frame
class GSIFrame : public wxFrame
{
public:
    explicit GSIFrame(const GSIServer * sock=NULL);
    ~GSIFrame();
//// event handlers (these functions should _not_ be virtual)
    void OnQuit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

	void OnOpenDeviceRS232(wxCommandEvent& event);
	void OnOpenDeviceCAN(wxCommandEvent& event);

    errorType OpenDevice(const wxString &dev_str, GSIDevHandle *dev_handle,GSINodeHandle *node);
    errorType OpenDevice(const wxString &dev_str, GSIDevHandle *dev_handle);


	virtual   errorType ProcessServerCommand(GSICommandXXX &Cmd,wxSocketBase &sock);


    void OnOpenNode(wxCommandEvent& event);
    void OnNodeCOMScan(wxCommandEvent& event);
    void OnNodeCANScan(wxCommandEvent& event);

    virtual void OnCloseWindow(wxCloseEvent& event);        //close button 'X' pressed

//Have a debug menu for testing stuff. Onxxx function here should be placed on debug menu
	void OnFireCustom(wxCommandEvent& event);
    void OnProcessAsyncRecord(wxCommandEvent& event);
    void OnGSIMCBreak(wxCommandEvent& event);


    virtual errorType ProcessAsyncRecord(GSIRecord &record,GSINode *node);


    virtual void Test1(wxSocketBase *sock);
    virtual void Test2(wxSocketBase *sock);
    virtual void Test3(wxSocketBase *sock);
	virtual void Test4(wxSocketBase *sock);

    virtual errorType CloseNode(int id);            //essentially a synonym for RemoveNode(id)

    errorType   CreateNode(GSIDevHandle h_device, GSINodeHandle h_node=GSI_NODE_THIS,wxSocketBase *sock=NULL);

    errorType   AddNode(GSINode *node,GSINodeHandle h_node);        //adds a node to the m_Node[] with error checking
	GSINodeHandle FindNodeHandle(const GSINode *n) const;
    errorType   RemoveNode(GSINodeHandle n=GSI_NODE_THIS);              //integer rather than GSIChar as may use 'virtual node'
    errorType   RemoveNode(GSIDevice *device);              //removes all occurences of device from node[]
    errorType   FindNodeOnDevice(const wxString &device,GSINodeHandle *h_node,size_t *num_nodes) const;
    errorType   SearchForNodes(GSINodeHandle h_node,GSINetwork &net);

	wxEvtHandler * const GetCustomEventHandler()  const {return(m_CustomEventHandler);}
	const GSIServer & GetGSIServer() const;

	void	SetCustomEventHandler(wxEvtHandler * evt);
	void	SetSockServer(GSIServer * s);
//****************************************************************************
//This function needs work!
    errorType CloseDevice(GSIDevHandle dhandle);          //removes all nodes
//***********************************************************************
  // convenience functions
    void UpdateStatusBar();

//Access functions
    errorType   GetNode(GSINodeHandle hnode,GSINode **node) const;
	GSINode *const Node(GSINodeHandle n=GSI_NODE_THIS,bool use_exception_flag=true) const;       //integer rather than GSIChar as may use 'virtual node'
	GSIDevHandle GetDeviceHandle(const wxString &dev,bool *is_open=NULL) const;
    GSIDevHandle GetDeviceHandle(const wxString &str, int *dev_type=NULL) const;             //default to return handle given a string
    GSIDevHandle GetDeviceHandle(const wxString &dev,int *dev_type,bool *is_open) const;
    GSIDevice *GetDevice(GSIDevHandle h_device) const   {h_device; return m_CurrentDevice;}            //multiple device syntax for later addition
    bool    IsDeviceOpen(GSIDevHandle h_dev) const;
                                              //multiple device syntax for later addition
	wxTextCtrl			* const TextCtrl() const {return m_Text;}
	wxTextCtrl			* const GetStatusText() const {return m_Text;}
	wxStatusBar			* const StatusBar() const {return m_StatusBar;}

protected:
    wxMenuBar *GetMenuBar(){return m_menuBar;}

    const wxString GetDeviceString(int id);         //return string associated with this device
    const wxString GetDeviceString(GSIDevHandle device_handle);         //return string associated with this device
    int            GetDeviceId(const wxString &dev);           //return the id of the device string, -1 on failure
    //const GSIDevHandle   GetDeviceHandle(wxInt16 index) const;
    errorType           CloseAllNodesOnDevice(GSIDevHandle device_index);
    errorType   SetCurrentDevice(GSIDevice *device);

private:
	wxEvtHandler    *m_CustomEventHandler;
	GSIServer	    * m_SockServer;

    wxTextCtrl      *m_Text;
	wxStatusBar		*m_StatusBar;

    wxMenu          *m_menuFile;
	wxMenu          *m_menuDevice;
   	wxMenu          *m_menuNode;

	wxMenu			*m_menuDebug;
	wxMenu			*m_menuHelp;
    wxMenuBar       *m_menuBar;
    bool            m_busy;

	GSINode			*m_Node[GSIMaxNumNodes];	//a pointer for all possible nodes on the network
#if GSI_DUPLICATE_WITH_CONTAINERS

	vector<GSIDevHandle> m_deviceHandle_Vec;
#endif
    GSIDevice       *m_CurrentDevice;                            //currently only support one device
    GSIDevHandle    m_deviceHandle[GSIMaxNumDevices];


    bool            m_rs232DeviceOpen;             //debugging only
    DECLARE_EVENT_TABLE()
};



#endif//GSI_HPP_INCLUDED
