#ifndef DEVICE_HPP_INCLUDED
#define DEVICE_HPP_INCLUDED

#include <wx\socket.h>
#include <gerror.h>
#include "gsidefs.h"
#include "microcontroller.h"

//forward declarations
class wxString;
class GSIFrame;
class GSICQueue;
class GSINode;
class GSIRecord;
//enum GSIEnumRecord;


//class GSI_API GSIDevice: public GSIFrame
class GSIDevice: public wxEvtHandler
{
public:
	//Place the Q's threads etc in here. Leave the GSIDevice class as an abstract class
	//that way we can derive for eg. a usb device from GSIDevice and not need to know about the
	//implementation details. The user can handle the details

	//GSIDevice(GSINode * gsi_node,GSIFrame *gsi_frame=NULL, wxWindow * parent=NULL,wxEvtHandler *custom_event_handler=NULL);
    //GSIDevice(const wxString& device,GSIFrame *gsi_frame=NULL, wxWindow * parent=NULL,wxEvtHandler *custom_event_handler=NULL);

	//explicit GSIDevice(const wxString& device,GSIFrame *gsi_frame=NULL,wxEvtHandler *custom_event_handler=NULL);
	//XXXGS mod 7/4 added GSIHandle to ctor
	explicit GSIDevice(const wxString& device,GSIHandle handle,GSIFrame *gsi_frame=NULL,wxEvtHandler *custom_event_handler=NULL);
    //explicit GSIDevice(const wxString& device,wxTextCtrl *textCtrl=NULL,wxEvtHandler *custom_event_handler=NULL);
    virtual ~GSIDevice();

    virtual errorType Open()=0;
    virtual errorType Close()=0;

    virtual GSICQueue	* const AsyncQ() const {return m_AsyncQ;}     //default async Q

	virtual errorType Enable()=0;
	virtual errorType Disable()=0;
    virtual GSIDevice * const Device() const {return m_Device;}
	virtual	void	SetNode(GSINode * gsi_node) {m_GSINode=gsi_node;} //pointer is constant
    virtual GSINode	* const Node() const {return m_GSINode;}

	virtual	void			SetFrame(GSIFrame * gsi_frame) {m_GSIFrame=gsi_frame;} //pointer is constant
    virtual GSIFrame	* const Frame() const {return m_GSIFrame;}
    const wxString &  GetDeviceName() const {return(m_DeviceName);}

	virtual errorType SetMode(GSIEnumMode flag)=0;

	virtual GSIEnumMode GetMode()const =0;

    //virtual errorType SetMasterSlave(GSIEnumMasterSlave mode,GSIChar pic_node=GSI_NODE_THIS) {m_master=mode;
                                                                                             //return errNone;}
    virtual errorType SetMaster(GSIChar node=GSI_NODE_THIS);                         //{m_master=GSI_MASTER; return errNone;}
    virtual errorType SetSlave(GSIChar node=GSI_NODE_THIS);

    virtual bool IsTxMaster()       const {return (m_master==GSI_MASTER);}
    virtual bool IsTxSlave()        const {return (m_master==GSI_SLAVE);}
	//virtual bool IsCommsMaster()    const {return (m_master==GSI_MASTER);}
    //virtual bool IsCommsSlave()     const {return (m_master==GSI_SLAVE);}
	virtual errorType Write(GSIEnumRecord e_record) =0;
	virtual errorType Write(const GSIChar c,GSIChar *recvd) =0;
	virtual errorType Write(GSIRecord &r) =0;
	virtual errorType Write(const GSIChar c,GSIChar *recvd,long timeout) =0;
	virtual errorType Write(GSIChar chr) =0;	//write a single byte without echo (handshaking etc.)

	virtual errorType Read(GSIChar *recd) const =0;
	virtual errorType Read(GSIChar *recd,long timeout) const =0;

	virtual errorType Read(GSIChar *buff,size_t num) const =0;
	//errorType Read(GSIChar *buff,size_t num,long timeout);
	virtual errorType Read(GSIChar *buff,size_t num,size_t *num_read,long timeout) const =0;

	virtual errorType Read(GSIRecord &record, long timeout) =0;
	virtual errorType Read(GSIRecord &record) =0;

//records received from pic node asyncronously. A CAN SYNC, a button press etc.
	virtual errorType AsyncRecordReceived(GSIRecord &record);
	//virtual errorType ReadAsyncRecord(GSIRecord &record);			//no wait returns errEmpty etc.
	//virtual errorType ReadAsyncRecord(GSIRecord &record,long timeout); //timeout
	//virtual errorType WaitForAsyncRecord(GSIRecord &record);

	virtual errorType DefaultProcessAsyncRecord(GSIRecord &record ); //This library processes

	//errorType EchoOn();
	//errorType EchoOff();
	virtual errorType EchoOn()=0;
	virtual errorType EchoOff()=0;
	virtual bool GetEchoState() const  =0;	//true=on false=off

	//device.cpp can handle this, it polymorphically uses our read functions

	virtual errorType ReSynchNode(GSIChar *recd, int *null_count);

//the derived class call SetDevice this so we can read and write the physical device from this
//high level class, abstracting out the details
    void                SetDevice(GSIDevice *device){ m_Device=device; }


	wxWindow			* const GetParent()	const			{return m_Parent;}
	void				SetParent(wxWindow * parent)  {m_Parent=parent;}
	virtual void		SetCustomEventHandler(wxEvtHandler * custom_event_handler);
	wxEvtHandler *		const GetCustomEventHandler() const	{return m_CustomEventHandler;}
	void OnQuit(wxCommandEvent& event);

    void                SetControllerHasReset(bool flag) {m_controllerHasReset=flag;}   //set when controller has reset itself, wdt/por/bor etc.
    bool                IsControllerReset()     {return m_controllerHasReset;}          //checked when deleting device to ensure error free deletion of resources
	virtual wxMutex     &GetCommsMutex() {return m_CommsMutex;}          //mutex to serialize control to underlying comms device
    const GSIHandle     GetHandle() const {return m_handle;}

    const  bool         IsOpen() const {return m_isOpen;}
protected:
	void                SetDeviceName(wxString& name)   {m_DeviceName=name;}
    void                SetIsOpen(bool val) {m_isOpen=val;}

private:

	wxWindow			*m_Parent;
	wxEvtHandler	    *m_CustomEventHandler;
	GSINode				*m_GSINode;									//The node we are embedded in
	GSIFrame			*m_GSIFrame;							//The GSIFrame we are embedded in
    wxString            m_DeviceName;
    GSIDevice           *m_Device;                              //The inherited device, so we can read/write physical device
    bool                m_controllerHasReset;
    GSIEnumMasterSlave   m_master;
    wxMutex             m_CommsMutex;                           //serialise access to underlying physical communiction device
    GSICQueue           *m_AsyncQ;
    GSIHandle           m_handle;                               //one handle slot for every possible device (in GSIFrame)
    bool                m_isOpen;
DECLARE_EVENT_TABLE()

};

class NullDevice :public GSIDevice
{
public:
    explicit NullDevice(const wxString &device_name,GSIFrame *gsi_frame=NULL);
    ~NullDevice();
private:
};


/************************************************************
        class AckHelper
Ensure we write the Ack after a record has been processed
*************************************************************/
class AckHelper
{
public:
    explicit AckHelper(GSIDevice *dev);
    ~AckHelper();
    void WriteAck();
private:
    bool m_ackFlag;
    GSIDevice *m_Dev;
};





#endif
