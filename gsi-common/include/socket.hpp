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
#ifndef SOCKET_HPP_INCLUDED
#define SOCKET_HPP_INCLUDED

#include <wx/socket.h>     //wxWidgets socket.h
//#include <gerror.h>
#include <gsexcept.hpp>
#include "gsidefs.h"


const bool GSIReportSocketCommandStatusFlag=true; //controls output to text ctrl

const wxInt16 GSIMaxADCArraySize=8192;
const size_t GSIMaxSocketData=GSIMaxADCArraySize+2;     //allow a couple of extra bytes for future status
const int GSIMaxDefaultSocketReturnDataArray=32;

//forward references
class GSISocketData;
class wxSocketBase;
class GSIClientNBPanelData;
enum
{
    GSI_SERVER_QUIT = 1000,
    GSI_SERVER_START,
    // id for sockets
    GSI_SERVER_ID,
    GSI_SOCKET_ID
};
//Posted event ids
enum
{
GSI_SOCKET_INPUT,
GSI_SOCKET_OUTPUT,
GSI_SOCKET_CONNECTION,
GSI_SOCKET_LOST,

GSI_SOCKET_RECORD,              //these must be between 0 and 255
GSI_SOCKET_ASYNC_RECORD,

GSI_SOCKET_RESERVED1,
GSI_SOCKET_RESERVED2,
GSI_SOCKET_RESERVED3,
GSI_SOCKET_RESERVED4,
GSI_SOCKET_RESERVED5,
GSI_SOCKET_RESERVED6,
GSI_SOCKET_RESERVED7,
GSI_SOCKET_RESERVED8,
GSI_SOCKET_RESERVED9,
GSI_SOCKET_RESERVED10,
GSI_SOCKET_RESERVED11,
GSI_SOCKET_RESERVED12,
GSI_SOCKET_RESERVED13,
GSI_SOCKET_RESERVED14,
GSI_SOCKET_RESERVED15,
GSI_SOCKET_RESERVED16,
GSI_SOCKET_USER1,
GSI_SOCKET_USER2,
GSI_SOCKET_USER3,
GSI_SOCKET_USER4,
GSI_SOCKET_USER5,
GSI_SOCKET_USER6,
GSI_SOCKET_USER7,
GSI_SOCKET_USER8,



};

const int GSIServerNumStatusBarFields=4;
enum
{
GSI_STATUS_BAR_FRAME=0,			//general status
GSI_STATUS_BAR_NODE,					//node status
GSI_STATUS_BAR_DEVICE,				//Device status
GSI_STATUS_BAR_SOCKET		//socket status
};

//Client


//Socket Server
const int GSISocketAdress=3000;
//const int GSIMaxSocketBufferSize=64;		GSIRecordMaxLength

//Maximum/minimum values
const int GSIMaxNode=256;



BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE(wxEVT_GSI_ASYNC_RECORD, GSI_ID_ASYNC_EVT)
	DECLARE_EVENT_TYPE(wxEVT_GSI_SOCKET, GSI_ID_SOCKET_EVT)
END_DECLARE_EVENT_TYPES()

//From wxManual:
//it may also be convenient to define an event table macro for this event type
//Remember: no space between end of #define and '(' !!
#define EVT_GSI_ASYNC_RECORD(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_GSI_ASYNC_RECORD, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),



//Custom event for socket events

//-------------wxEVT_GSI_SOCKET_COMMAND---------------------------
class wxGSISocketCommandEvent: public wxCommandEvent
//class wxGSISocketCommandEvent: public wxCommandEvent
{
public:
    wxGSISocketCommandEvent(wxEventType commandType=wxEVT_NULL,
        int id=0):wxCommandEvent(commandType,id)
    {}

    wxGSISocketCommandEvent(const wxGSISocketCommandEvent& event):wxCommandEvent(event)
    {}
#if 0       //Never seems to be called!
    virtual ~wxGSISocketCommandEvent()     //delete any added GSIObject here
    {
        if(GetClientObject())
            delete(GetClientObject());      //delete any object we allocated

    }
#endif
    virtual wxEvent *Clone() const
                    { return new wxGSISocketCommandEvent(*this);}   //should this clone the ClientObject as well??

    void    SetSockData(GSISocketData * sd) {m_SockData=sd;}
    const   GSISocketData   * GetSockData() const   {return m_SockData;}
    void    SetSocketBase(wxSocketBase *sock)       {m_SockBase=sock;}
    const   wxSocketBase *GetSocket()       const   {return m_SockBase;}
	void	SetNBPanelData(GSIClientNBPanelData * pd) {m_Pd=pd;}
	const   GSIClientNBPanelData * GetNBPanelData() const {return m_Pd;}
private:
    GSISocketData   *   m_SockData;
    wxSocketBase    *   m_SockBase;
	const GSIClientNBPanelData * m_Pd;
DECLARE_DYNAMIC_CLASS(wxGSISocketCommandEvent);
};

typedef void (wxEvtHandler::*wxGSISocketCommandEventFunction) (wxGSISocketCommandEvent&);



//-------------wxEVT_GSI_SOCKET_RECORD_RECEIVED---------------------------
class wxGSISocketRecordReceivedEvent: public wxCommandEvent
{
public:
    wxGSISocketRecordReceivedEvent(wxEventType commandType=wxEVT_NULL,
        int id=0):wxCommandEvent(commandType,id)
    {}

    explicit wxGSISocketRecordReceivedEvent(const wxGSISocketRecordReceivedEvent& event):wxCommandEvent(event)
    {}
    virtual wxEvent *Clone() const
                    { return new wxGSISocketRecordReceivedEvent(*this);}   //should this clone the ClientObject as well??

    void    SetSockData(GSISocketData * sd) {m_SockData=sd;}
    const   GSISocketData   * GetSockData() const   {return m_SockData;}
    void    SetSocketBase(wxSocketBase *sock)       {m_SockBase=sock;}
    const   wxSocketBase *GetSocket()       const   {return m_SockBase;}
private:
    GSISocketData   *   m_SockData;
    wxSocketBase    *   m_SockBase;
DECLARE_DYNAMIC_CLASS(wxGSISocketRecordReceivedEvent);
};

typedef void (wxEvtHandler::*wxGSISocketRecordReceivedEventFunction) (wxGSISocketRecordReceivedEvent&);



BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE(wxEVT_GSI_SOCKET_COMMAND, GSI_ID_SOCKET_COMMAND)
    DECLARE_EVENT_TYPE(wxEVT_GSI_SOCKET_RECORD_RECEIVED, GSI_ID_SOCKET_RECORD_RECEIVED)
END_DECLARE_EVENT_TYPES()


#define EVT_GSI_SOCKET_COMMAND(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_GSI_SOCKET_COMMAND, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) ( wxGSISocketCommandEventFunction) &fn,\
        (wxObject *) NULL \
    ),
//(wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ),

#define EVT_GSI_SOCKET_RECORD_RECEIVED(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        wxEVT_GSI_SOCKET_RECORD_RECEIVED, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) ( wxGSISocketRecordReceivedEventFunction) &fn,\
        (wxObject *) NULL \
    ),

//---------------wxEVT_GSI_SOCKET_RECORD_RECEIVED------------------------
//-----------------------END ------------------------------------



/*******************************************************
    class GSISocketException:public gsException
*/
//doesn't really add anything to underlying class, we just need to differentiate it so that we can catch() it specifically
class GSISocketException:public gsException
{
public:
    GSISocketException(const wxSocketBase &sock, errorType e, const wxString &s="",unsigned int data0=0,unsigned int data1=0, unsigned int data2=0);
    virtual ~GSISocketException();
    const wxSocketBase & GetSock() {return m_Sock;}
private:
    const wxSocketBase &m_Sock;
};


/*****************************************************************************
			GSISocketData
Small helper class to pass socket data received from GSIServer on to
user application using a custom event. The event stores a GSISocketData *
WARNING!
The socket interface only sends the underlying data stream. The m_length member
will only be filled in *if* the socket client does so from the underlying socket
message data length
*****************************************************************************/
class GSISocketData
{
public:
	explicit GSISocketData(int id);
    GSISocketData(GSIChar *buff,wxUint32 len,int node);

	virtual ~GSISocketData();

	const GSIChar          *    GetBuffer() const {return(m_buffer);}
	int                     GetID() const {return(m_id);}
    size_t                  GetLength() const {return m_length;}

    GSIChar GetData(size_t index)		const;
	errorType SetData(int index,GSIChar chr);
    void SetID(int id) {m_id=id;}
    void    SetNode(int node);              //check for out of range, so can set GSINotAValidNode
    int GetNode() const {return m_node;}
private:
	GSIChar m_buffer[GSIRecordMaxLength];
	int	m_id;
    size_t m_length;
//GSISocketClient when opened will cause the server to open a new connection. The client will send a node value down that
//connection, telling the server that the wxSocketBase of that connection is connected to that node (or some other scheme!)
//When the node talks to the server, the server will send that record through the socket.
//The GSIClient listening to that socket will receive the data. It knows what node it is attached to.
//GSIClient then posts its own event containg a GSISocketData * (which will have its node field set by GSIClient socket handler)
// so that the top level event handler can be alerted to the reception of data.
// It can read the node, to see which panel to update.

    int m_node;                 //node from which the server sent. Stored in GSIClient (sent to Server as a wxSockBase)
};

//----------------------------------------------------------------------------------------
//class SocketReturn
//Small class to send back success/fail+data to caller
//send back in dtor
//--------------------------------------------------------------------------------------------
class GSISocketReturnData
{

public:
enum GSISocketReturnValue
{
    GSI_SUCCESS=0,
    GSI_FAIL=1,
    GSI_PARAM_ERROR=2,
    GSI_TIMEOUT=3,
    GSI_WDT=4,
    GSI_ALREADY_OPEN=5,
    //GSINoNode=6,
	GSI_NO_SOCKET=6,
    GSI_NOT_FOUND=7,
    GSI_OPEN_FAILED=8,
    GSI_CLOSE_FAILED=9,
    GSI_TOO_MANY_OPEN_DEVICES=10,
	GSI_NO_DEVICE=11,

    GSI_SERVER_ERROR=50,

    GSI_NOT_A_RETURN_VALUE=98,			//derived classes can use id's >
	GSI_FIRST_USER_GSI_SOCKET_VALUE=99	//User code can do something
										//like this.
										//	enum GSISocketReturnValue
										//	{
										//		USER_RV1=GSISocketReturnValue::GSI_FIRST_USER_GSI_SOCKET_VALUE+1,
};										//		USER_RV2,
										//		USER_RV3
										//	};
										// if user codes are sent to GSISocketReturnValue it will default to "unknown error"

    GSISocketReturnData(wxSocketBase &sock,bool internal_flag=false);
    GSISocketReturnData(wxSocketBase &sock,enum GSISocketReturnValue sr,bool internal_flag=false);
	GSISocketReturnData(wxSocketBase &sock,int sr,bool internal_flag=false);
    virtual ~GSISocketReturnData();

    virtual errorType Write();                         //write the class to the socket
//Access functions
    virtual void    SetReturnCode(wxInt16 code);
	GSISocketReturnValue SetReturnCode(errorType rv);	//convert errorType to GSI GSISocketReturnValue
    virtual void    SetNumInts(wxInt16 num_ints) {m_numInts=num_ints;}
    virtual void    SetDataArray(wxInt16 * array) {m_data=array;}
    virtual void    SetInternalFlag(bool flag) {m_internal=flag;}
    virtual void    SetWrittenFlag(bool flag)   {m_written=flag;}


    virtual wxInt16 GetReturnCode() const {return m_code;}


    virtual wxInt16 GetNumInts() const {return m_numInts;}
    virtual wxInt16 * const GetDataArray() const {return m_data;}
    virtual wxString GetErrorString() const {return m_ErrorString;}

    bool            IsWritten()  const {return m_written;}
    bool            IsInternal() const {return m_internal;}
    bool			GetInternalFlag() {return m_internal;}	//aliases of above functions
	bool			GetWrittenFlag() {return m_written;}	//for consistency
	GSISocketReturnValue MapError(errorType error);				//convert errorType to GSI GSISocketReturnValue

    wxSocketBase   & GetSocket()  const {return m_Sock;}
	errorType		ResizeDataBuff(int elements);

protected:
    virtual void    SetErrorString(GSISocketReturnValue sr);
	//virtual void    SetErrorString(GSISocketReturnValue sr);
    virtual void    SetErrorString(const wxString & error_string) {m_ErrorString=error_string;}

private:
    wxInt16 m_code;               //success fail
    wxInt16 m_numInts;            //we are about to send this many ints to the socket
    wxInt16 *m_data;              //the data that we are going to send, might be adc array data
	int		m_bufferSize;			//size of the data buffer
    wxSocketBase &m_Sock;
    bool    m_written;          //if already written, don't write in dtor
    bool    m_internal;         //don't write if internal flag set
    wxString m_ErrorString;
    //wxInt16 m_defaultDataArray[GSIMaxDefaultSocketReturnDataArray];

};


class MapWxSocketError:public ErrorClass
{
public:
	MapWxSocketError();
	explicit MapWxSocketError(enum wxSocketError error);

	virtual ~MapWxSocketError();

	errorType MapError(enum wxSocketError error);

	errorType GetErrorType() {return m_rv;}
private:
	errorType m_rv;
};


#endif //SOCKET_HPP_INCLUDED
