
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
#ifndef RS232_HPP_INCLUDED
#define RS232_HPP_INCLUDED

#ifndef __WIN32__

	#include <termios.h>
	#include <sys/signal.h>

#endif

#include <gsSerial.hpp>
#include "rs232defs.h"
#include "device.hpp"
#include "record.hpp"               //for embedded GSIRecord ?? use a pointer


//forward references
class wxString;
class GSIRS232Device;


class GSISerialPort:public gsSerialPort
{
public:
    GSISerialPort(const wxString& device, gsBaud baud_rate,GSIRS232Device *r2_device,int rx_buffer_size=GS_SERIAL_DEFAULT_Q_SIZE);
    //GSISerialPort();
    ~GSISerialPort();

    virtual void  ComEvent(gsComEvent ev);      //overide the gsSerialPort function to receive notification of Rx,Break,CTS etc.
    //enum errorType WritePort(char *buff ,unsigned int num_to_write);  gsSerialPort definition
    errorType Write(GSIChar chr) {return(WritePort( (char *)&chr,1));}
    //wxMutex &GetCommsMutex()    {return m_CommsMutex;}
    //wxCriticalSection m_waitingForRecordLock;         //when sending a record, lock the code
    //wxMutex m_TxMutex;
protected:
    errorType   BreakHandler();
private:
    GSIRS232Device * m_RS232Device;
    //wxMutex             m_CommsMutex;                           //serialize communication with the underlying communication device

};


class GSISerialThread;

#ifdef __WIN32__

class SerialPortDevice :public GSIDevice
{
public:
    SerialPortDevice(const wxString &device_name,GSIHandle handle, GSIFrame *gsi_frame=NULL);
    ~SerialPortDevice();
private:
};



class GSIRS232Device :public SerialPortDevice //,wxThread
{
friend class	GSISerialThread;	//allow the service thread access to our private data
enum GSIEnumBinaryModeActionFlag
{
	BIN_MODE_MASTER=0,		//simple master
	BIN_MODE_SLAVE=1,			//simple slave
	BIN_MODE_BLOCK_WRITE,  //block master
	BIN_MODE_BLOCK_READ,	  //block slave
	BIN_MODE_USER1,
	BIN_MODE_USER2,
	BIN_MODE_USER3
};

public:
	//Place the Q's threads etc in here. Leave the GSIDevice class as an abstract class
	//that way we can derive for eg. a usb device from GSIDevice and not need to know about the
	//implementation details. The user can handle the details
	//GSIRS232Device(GSINode * gsi_node,const wxString& device, long baud_rate,size_t qsize);
	//GSIRS232Device::GSIRS232Device(GSINode * gsi_node,const wxString& device, long baud_rate,size_t qsize,GSIFrame *gsi_frame);
    GSIRS232Device(const wxString& device, gsBaud baud_rate,size_t qsize,GSIHandle handle, GSIFrame *gsi_frame,int rx_buff_size=GS_SERIAL_DEFAULT_Q_SIZE);
	//GSIRS232Device::GSIRS232Device(gsSerialPort & com_port,size_t qsize,GSIFrame *gsi_frame,int rx_buff_size=GS_SERIAL_DEFAULT_Q_SIZE);
	virtual ~GSIRS232Device();

    //virtual errorType Open(const wxString& device, gsBaud baud_rate,size_t qsize);           //Use this to open the device (instantiate the underlying gsSerialPort or derived class)
    virtual errorType  Open(const wxString& device, gsBaud baud_rate,int rx_buffer_size);
                                        //if open not called then gsSerialPort is never instantiated
    virtual errorType Open();
    virtual errorType Close();

	wxThread::ExitCode Entry();

    //GSICQueue	* const AsyncQ() {return m_AsyncQ;}         //use default Q

	virtual errorType Enable();
	virtual errorType Disable();

	virtual errorType Init();

	virtual errorType SetMode(GSIEnumMode flag);

	virtual GSIEnumMode GetMode() const {return(m_mode);};

   //virtual errorType SetMasterSlave(GSIEnumMasterSlave mode,GSIChar pic_node=GSINodeThis);  //override
   //virtual  errorType SetMaster(GSIChar node=GSINodeThis);
   //virtual  errorType SetSlave(GSIChar node=GSINodeThis);
   //virtual GSIEnumMasterSlave GetMasterSlave() const;
#if ! DEBUG_USE_CTB

	ComPortHandle GetComPortHandle()const {return(m_hPort);}
#endif
	virtual errorType Write(GSIEnumRecord e_record);
	virtual errorType Write(const GSIChar c,GSIChar *recvd);
	virtual errorType Write(GSIRecord &r);      //Uses a CriticalSection to gain access
	virtual errorType Write(const GSIChar c,GSIChar *recvd,long timeout);
	virtual errorType Write(GSIChar chr);	//write a single byte without echo (handshaking etc.)
	//non-service thread write+echo of chars
	//Only use WriteUnbuffered after instantiating GSIPauseThread
	virtual errorType WriteUnbuffered(const GSIChar c,GSIChar *recvd,long timeout);

	errorType Read(GSIChar *recd) const;
	errorType Read(GSIChar *recd,long timeout) const;

	errorType Read(GSIChar *buff,size_t num) const;
	//errorType Read(GSIChar *buff,size_t num,long timeout);
	errorType Read(GSIChar *buff,size_t num,size_t *num_read,long timeout) const ;

	errorType Read(GSIRecord &record, long timeout);
	virtual errorType Read(GSIRecord &record);
	//ReadUnbuffered reads the port directly. The service thread must have been paused
	//using GSIPauseThread class
	virtual errorType ReadUnbuffered(GSIChar *chr, long timeout) const;

	virtual errorType EchoOn();
	virtual errorType EchoOff();
	virtual bool GetEchoState() const {return(m_echoFlag);};	//true=on false=off

	//device.cpp can handle this, it polymorphically uses our read functions
	//errorType GSIRS232Device::ReSynchNode(GSIChar *recd, int *null_count);
    virtual errorType ReSynchNode(GSIChar *recd, int *null_count);
	virtual errorType NewInCQ(GSICQueue *new_q,GSICQueue *old_q);

	//virtual errorType WaitForAsyncRecord(GSIRecord &record);		//overide the device class function
	//bool	IsAsyncRecordReceived(){return(asyncRecordReceived);}

	virtual errorType DefaultProcessAsyncRecord(GSIRecord &r);

	virtual errorType SetPCAsMaster();
	//Tty is a debugging function
	//virtual void GSIRS232Device:: Tty(void);


    //GSISerialPort * const ComDevice() const {return m_ComDev;}
    gsSerialPort * const ComDevice() const {return m_ComDev;}
    //void    SetComDevice(GSISerialPort *com_dev) {m_ComDev=com_dev;}
    void    SetComDevice(gsSerialPort *com_dev) {m_ComDev=com_dev;}

    virtual errorType AssembleAsyncRecord(GSIChar chr);
#if 0
    virtual errorType InitComPort(const wxString& device, gsBaud baud_rate,int rx_buffer_size=GS_SERIAL_DEFAULT_Q_SIZE);          //platform specific, see w_rs232.cpp, l_rs232.cpp

    virtual wxMutex &GetCommsMutex()    {return m_CommsMutex;}
#endif
protected:

	void	SetCustomEventHandler(wxEvtHandler * custom_event_handler);
    errorType  SendRecordWithEcho(GSIRecord &r);            //Not controlled by a lock
    //void    SetSerialPort(GSISerialPort *com_dev) {m_ComDev=com_dev;}
    void    SetSerialPort(gsSerialPort *com_dev) {m_ComDev=com_dev;}
private:

    errorType CloseComPort();                                             //platform specific

	//GSISerialThread *m_ServiceThread;	//detached threads must be on the heap

	//bool readingRecord;		//currently writing a record with echo
	//GSICQueue * m_DefaultInQ;
	//GSICQueue * m_DefaultOutQ;
	//GSICQueue * m_DefaultSlaveQ;
	//GSICQueue * m_DefaultAsyncQ;

	//GSICQueue * m_InQ;
	//GSICQueue * m_OutQ;
	//GSICQueue * m_SlaveQ;						//used for asynchronous reception. Controlled by masterSlave flag
	//GSICQueue * m_AsyncQ;

#if DEBUG_USE_CTB
    //wxIOBase* m_Dev;
#else
	//GSISerialPort *m_Dev;
    //GSISerialPort * m_ComDev;
#endif
    gsSerialPort * m_ComDev;
	COMMTIMEOUTS	m_commTimeouts;			//WIN32 timeout values
	ComPortHandle	m_hPort;					//handle of the port

    DCB			 m_portDCB;				//WIN32 comm parameters

	GSIEnumMode m_mode;						//binary/ascii
	//GSIEnumMasterSlave m_masterSlave;									//Controls use of outQ/slaveQ in conjunction with
	GSIEnumBinaryModeActionFlag m_binaryModeActionFlag;	 //binaryActionFlag

	bool m_echoFlag;
	int m_enableFlag;	//store all variables in derived class to allow any user implementation
	errorType CheckParameters(long baud_rate); //probably not required??


	unsigned long WritePort(GSIChar chr);
	unsigned long ReadPort(GSIChar *chr,size_t l);
    //Async Record reception variables
   	GSIRecord m_AsyncRecord;
   	bool m_asyncRecordReceived;
	int m_asyncIndex;
    int m_asyncBreakCount;
    int m_asyncCursor;		//initial value=0, which is default but make it explicit for readability
	GSIChar m_asyncLength,m_asyncNode,m_asyncType;
    //wxMutex     m_CommsMutex;
};



#else //__WIN32__

class GSI_API GSIDevice
{
public:
enum GSIEnumBinaryActionFlag
{
	ReadingRecord=0,
	ReadingAsyncRecord=1,
	ReadingBlock=2,
	User1=3,
	User2=4,
	User3=5
};

  GSIDevice(wxString device,long baud_rate,size_t qsize);
  //GSIDevice(char * device,long baud_rate,size_t qsize);
  virtual  ~GSIDevice();
  virtual errorType Enable(GSIEnumOnOff flag); //enable/disable
  virtual errorType CheckParameters(long baud_rate);
  void Tty(void);
  virtual errorType NewInCQ(GSICQueue *new_q,GSICQueue *old_q); //use a different input CQ
  //Processing of data should probably use a virtual function so user can
  //do specific processing should they change the pic rs232 functions.
  //For example sending back ADC data as a data stream
  //This would occur within the GSINode object which is derived from this object
  //keeping this as a pure data collection source.
  virtual void DataReceivedInterrupt(void);
  virtual errorType Write(const GSIChar chr,GSIChar *recvd);
  virtual errorType Write(GSIEnumRecord record);
  virtual errorType Write(const GSIChar chr,GSIChar *recvd,long timeout);
  //virtual errorType Write(GSIRecord *record);
  virtual errorType Write(GSIRecord &record);

  //virtual errorType Read(GSIRecord &record,ComPortHandle hPort, long timeout);
  errorType EchoToggle(bool *flag);
  virtual errorType SetPCAsMaster();

  //virtual errorType WriteRecord(GSIEnumRecord record);
  //volatile int DataReceivedFlag;
  GSICQueue * m_InQ;
protected:
  GSIEnumMode m_mode;
  GSICQueue * m_OutQ;
private:
	ComPortHandle m_hPort;
  //errorType ReSynchNode(GSIChar *recd, int *null_count);
  GSIEnumOnOff m_enableFlag;
	GSIEnumBinaryActionFlag m_binaryActionFlag;
  int m_tty; // Should use wxFile??
  bool m_echoFlag;
  struct termios m_oldkey,m_newkey;//tty settings
  struct termios m_oldtio,m_newtio;//serial port settings
  struct sigaction m_saio,m_prevsaio;               //definition of signal action
  GSICQueue *m_DefaultInQ; //user can swap q so must keep * here so can delete
  GSICQueue *m_DefaultOutQ;
  long m_baudRate;
  bool m_master;
};
#endif //__WIN32__
//typedef void (GSIDevice::*PGSIClassMFV)(void); //Pointer to GSICLass MemberFunction Void

//typedef  void (GSIDevice::*GSIDeviceMemFn)(void);
//#define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember)) 0


#warning ("Must embed parent node in this class")

class GSI_API GSISerialThread : public wxThread
{
public:
    //GSISerialThread(GSIRS232Device *device,CQueue *master_q,CQueue* slave_q,wxEvtHandler *custom_event_addr);
    GSISerialThread(GSIRS232Device *device,GSICQueue *mq,GSICQueue *aq,wxEvtHandler *custom_event_addr);
	virtual ~GSISerialThread();
    // thread execution starts here
	//virtual wxThread::ExitCode Entry();
	virtual void * Entry();
    // called when the thread exits - whether it terminates normally or is
    // stopped with Delete() (but not when it is Kill()ed!)
    virtual void OnExit();
	virtual errorType AssembleAsyncRecord(GSIChar chr);
	errorType Write(GSIRecord &r);
	void PauseThread(){m_pause=true;}
	void QuitThread();
	void ContinueThread(){m_pause=false;}
	void	SetCustomEventHandler(wxEvtHandler *customEventHandler){m_CustomEventHandler=customEventHandler;}


protected:

private:
	bool m_pause;
	bool m_quit;
	GSIRS232Device	*m_Device;
	GSICQueue *m_MasterQ;
	//GSICQueue *m_AsyncQ;
	GSICQueue *m_CurrentQ;
	wxEvtHandler * m_CustomEventHandler;

   	//GSIRecord m_AsyncRecord;
	//int m_asyncIndex;
    //int m_asyncBreakCount;
    //int m_asyncCursor;		//initial value=0, which is default but make it explicit for readability
	//GSIChar m_asyncLength,m_asyncNode,m_asyncType;

};





#endif //RS232_HPP_INCLUDED
