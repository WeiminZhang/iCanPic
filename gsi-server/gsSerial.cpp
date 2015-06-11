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

#define _POSIX_SOURCE 1 /* POSIX compliant source */

//#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif



#include <wx/thread.h>

#if !wxUSE_THREADS
    #error "This file requires thread support!"
#endif // wxUSE_THREADS



#include <wx/stopwatch.h>
#include <wx/string.h>
#include <wx/wfstream.h> //for debugging
#include <wx/txtstrm.h>
#include <wx/app.h>



#ifdef __VISUALC__
#pragma warning(push, 3)
#endif


#include<signal.h>
#include <errno.h>
#include <stdlib.h>
//std headers in here

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
    #define __min(a,b) (((a) <(b)) ? (a) :(b))
#endif

#ifdef __VISUALC__
#pragma warning(pop)
// without this pragma, the stupid compiler precompiles #defines below so that
// changing them doesn't "take place" later!
#pragma hdrstop
#endif


//#include <wx/ctb/serport.h>
//#include <GSIexcept.hpp>
#include <gsSerial.hpp>
//#include <cq.hpp>

#define BAUDRATE B115200



/*****************************************************************************************
                            ctor

    gsSerialPort::gsSerialPort(const wxString& device, gsBaud baud_rate)

Parameters
device=device name (com1, com2 etc. under windows)
gsBaud baud rate using CTB define. Haven't been able to make ctb work correctly
hence my own serial port class. CTB readv always waits for the timeout interval
before reading the port. We need it to block until data (or status change) is available
*******************************************************************************************/
gsSerialPort::gsSerialPort(const wxString& device, gsBaud baud_rate,int qsize)          //platform specific, see w_rs232.cpp, l_rs232.cpp
{
wxString message;
DCB dcb;
COMMTIMEOUTS commTimeOuts;

#if DEBUG_gsSerialPort
    m_DEBUGOutStream=NULL;
#endif
	m_handle=CreateFile(device,                       //filename
					GENERIC_READ | GENERIC_WRITE,	/*attribs*/
					0,								/*share exclusive access*/
					NULL,							/*security*/
					OPEN_EXISTING,	                //disposition
					FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
					NULL);
	if(m_handle == INVALID_HANDLE_VALUE)
    {
        GS_SERIAL_THROW(wxString("Failed to open comm port"),errFail);
    }
//detect RxChar
   	if( SetCommMask(m_handle,EV_RXCHAR) ==FALSE) /*EV_ERR | EV_RXCHAR*/
	{

        GS_SERIAL_THROW (wxString("Failed to set CommMask"),errBadHandle);
	}
//Set buffer sizes. Don't need large ones for this program
	if(SetupComm(m_handle,1024,GS_SERIAL_MAX_BUFFER_SIZE) == FALSE)
	{
		printf("\n\aError in SetupComm()");
	}

//PURGE_RXABORT Terminates all outstanding overlapped read operations and returns immediately, even if the read operations have not been completed
	if (PurgeComm(m_handle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR ) ==FALSE)
	{
		GS_SERIAL_THROW(wxString("Failed to set PurgeComm"),errFail);
	}

/**********************************************************************
Important note
serial port with Overlapped comms is very sensitive to these settings.
Also sensitive to commTimeout settings
When creating overlapped structure be sure to zero all fields.
***********************************************************************/
	//dcb.DCBlength = sizeof( DCB ); //28
	GetCommState(m_handle, &dcb ) ;
    dcb.BaudRate=baud_rate;         //115200
    dcb.fBinary=true;               //true
    dcb.fParity=false;              //true
    dcb.fOutxCtsFlow=false;         //false
    dcb.fOutxDsrFlow=false;         //false
    dcb.fDtrControl=DTR_CONTROL_ENABLE; //ENABLE,turns on and leaves on dtr
    dcb.fDsrSensitivity=false;      //false
    dcb.fTXContinueOnXoff=false;     //false
	dcb.fOutX=false;                //false
	dcb.fInX=false;                 //false
    dcb.fErrorChar=false;           //false
    dcb.fNull=false;                //false
    dcb.fRtsControl=RTS_CONTROL_ENABLE; //ENABLE, turns on and leaves on rts
    dcb.fAbortOnError=false;        //false
    //dcb.fDummy                    //false
    //dcb.wReserved=0;              //0
    //dcb.XonLim=;                  //100
    //dcb.XoffLim=;                 //100
    dcb.ByteSize=8;                 //8
    dcb.Parity=NOPARITY;            //
    dcb.StopBits=ONESTOPBIT;        //
    //dcb.XonChar=;
    //dcb.XoffChar=;
    //dcb.ErrorChar=;
    //dcb.EofChar=;
    //dcb.EvtChar=;
    //dcb.wReserved1=;

    SetCommState(m_handle,&dcb);

	//GetCommState( handle, &dcb ); /*Used in debugging 11/11/01*/

//timeouts for overlapped read/write
	commTimeOuts.ReadIntervalTimeout = 0xffffffff;
	commTimeOuts.ReadTotalTimeoutMultiplier = 0;
	commTimeOuts.ReadTotalTimeoutConstant = 1000;
    commTimeOuts.WriteTotalTimeoutMultiplier =2 ; /*2ms should be enough*/
	commTimeOuts.WriteTotalTimeoutConstant = 0 ;

	SetCommTimeouts(m_handle,&commTimeOuts);

//Overlapped structure controlling Writes
    memset( &m_ovlWrite, 0, sizeof( OVERLAPPED ) ) ;

	m_ovlWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(m_ovlWrite.hEvent == NULL)
	{
		/*these are detected by WaitCommEvent*/
        GS_SERIAL_THROW(wxString("Failed to set CreateEvent"),errFail);
    }


//Create the CQueue to hold incoming data
    if(qsize < GS_SERIAL_MIN_Q_SIZE)
        qsize=GS_SERIAL_MIN_Q_SIZE;

    m_InQ=new CQueue(qsize);     //

    StartSerialThread();
#if DEBUG_GS_SERIAL_PORT
    m_writeToFileFlag=false;
    m_OStream=DEBUG_NEW wxFileOutputStream(wxT("gsSerialPort.txt"));
    m_DEBUGOutStream=DEBUG_NEW wxTextOutputStream(*m_OStream);
#endif
}


/*****************************************************************************************
            dtor
    gsSerialPort::~gsSerialPort()
Parameters
*******************************************************************************************/

gsSerialPort::~gsSerialPort()
{
DWORD e;

    wxStopWatch sw;
    BOOL bv;

    SetCommMask(GetHandle(),0);     //disable all events


    m_ServiceThread->QuitThread();
    bv=PurgeComm(GetHandle(), PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );
    if(bv==0)
    {
        uint32_t e;
        e=GetLastError();
    }

    m_ServiceThread->Delete();
    //m_ServiceThread->Wait();
    delete m_ServiceThread;

#if 0
    void * ec;
    ec=m_ServiceThread->Wait();
    if(ec)
        ec=ec;
#endif

    ClosePort();
    if (m_ovlWrite.hEvent != 0)
    {
        if (CloseHandle (m_ovlWrite.hEvent)==0)
        {
            e=GetLastError();
        }
    }
#if DEBUG_GS_SERIAL_PORT
    if (m_DEBUGOutStream)
        delete m_DEBUGOutStream;
    if(m_OStream)
        delete m_OStream;
#endif

    if(m_InQ)
    {
        delete m_InQ;
        m_InQ=NULL;
    }
    return;
}


/********************************************************************************
errorType gsSerialThread::StartServiceThread(gsSerialThread *thread)
********************************************************************************/
errorType gsSerialPort::StartSerialThread()
{
//Create the service thread
    //m_ServiceThread = new  gsSerialThread (this,m_InQ);
    m_ServiceThread = DEBUG_NEW  gsSerialThread (this);
    if(m_ServiceThread ==NULL)
    {
        GS_SERIAL_THROW(wxString("Error new gsSerialThread"),errFail);
        return(errFail);
    }
    int wError=m_ServiceThread->Create();

    if(wError != wxTHREAD_NO_ERROR )
    {
        GS_SERIAL_THROW(wxString("Error Thread Create"),errFail);
        return(errFail);
    }
//xxx Addition 28/2/08
//attempt to stop GSIEchoTimeout with 20mSec value
	m_ServiceThread->SetPriority(WXTHREAD_DEFAULT_PRIORITY+20);


    if ( m_ServiceThread->Run() != wxTHREAD_NO_ERROR )
    {

        GS_SERIAL_THROW(wxString("Error Thread Run()"),errFail);
        return(errFail);
    }
    return(errNone);
}


/************************************************************
    errorType gsSerialPort::CloseComPort()
use a Close() func rather than dtor so we can do platform
specific stuff more portably
************************************************************/
errorType gsSerialPort::ClosePort()
{
    CloseHandle(GetHandle());
    m_handle=INVALID_HANDLE_VALUE;
    return errNone;
}

/**************************************************************************
    void gsSerialPort::ComEvent(gsComEvent ev)
Some comport activity has been detected
We currently only look for RxChar
**************************************************************************/
void gsSerialPort::ComEvent(gsComEvent ev)
{

    switch(ev)
    {
    case gsSerialRxChar:
        int length;
        do
        {
            if(ReadPort(m_rxBuff,GS_SERIAL_MAX_BUFFER_SIZE,&length))
            {
                //an error
            }
            else
            if (length)
            {
                m_InQ->Write((char *)m_rxBuff,length);
            }
        }while ( length > 0 ) ;
        break;
    case gsSerialBreak:
        break;
    case gsSerialCTS:
        break;
    case gsSerialDSR:
        break;
    case gsSerialRing:
        break;
    case gsSerialRLSD:
        break;
    case gsSerialRxFlag:
        break;
    case gsSerialTxEmpty:
    case gsSerialFrameError:
    case gsSerialOverrunError:
    case gsSerialParityError:
        break;
    default:
        break;
    }
}


/***************************************************************************************
    bool gsSerialPort::IsPaused()
****************************************************************************************/
bool gsSerialPort::IsPaused() const
{
    return(m_ServiceThread->IsPaused());
}

/***************************************************************************************
    gsSerialPort::PauseRx()
#Return from the blocked ReadFile with any pending char and process it.
Set flag to indicate a char was received

#Pause the service thread
#Caller must call UnPauseRx() to continue the service thread
#Must also disable the Windows Event
****************************************************************************************/
errorType gsSerialPort::PauseRx(bool *data_received)
{
errorType rv;

    rv=m_ServiceThread->PauseThread();
    if (rv)
        return(rv);

    *data_received=false;
    if(m_InQ->GetNumInBuff())               //let caller know there was some data in the Q
        *data_received=true;

    return(rv);
}
/*************************************************************************
errorType gsSerialPort::UnPauseRx(bool *data_received)

*************************************************************************/
void gsSerialPort::UnPauseRx()
{
    m_ServiceThread->UnPauseThread();
}


/******************************************************************************
errorType gsSerialPort::ReadPort(char *buff, size_t max_length,int * num_read)
*******************************************************************************/

errorType gsSerialPort::ReadPort(char *buff, size_t max_length,int * num_read)
{
    return(m_ServiceThread->ReadPort(buff,max_length,num_read));
}


/*********************************************************************************************
errorType gsSerialPort::ReadPort(char *buff, size_t max_length,int * num_read,long timeout)
***********************************************************************************************/

errorType gsSerialPort::ReadPort(char *buff, size_t max_length,int * num_read, long timeout)
{
wxStopWatch sw;

    do
    {
        m_ServiceThread->ReadPort(buff,max_length,num_read);
        if(*num_read==0) wxSleep(0);                         //give up our time slice
    }while(sw.Time()<timeout && *num_read==0);
    if(*num_read ==  (int) max_length)
        return(errNone);
    return(errTimeout);
}
/************************************************************************
errorType gsSerialPort::WriteWithEcho(char sent,char *recd,long timeout)
************************************************************************/
errorType gsSerialPort::WriteWithEcho(char sent,char *recd,long timeout)
{
errorType rv=errNone;
//static char prevSent;
//char localPrevSent=prevSent;


    WritePort((char *)&sent,1);
    rv=InQ()->Read(recd,timeout);
    if(rv==errNone)
    {//no timeout, so check for a match
        if(sent != *recd)
            rv=errEcho;
    }
    return rv;
}

/*****************************************************************************************

errorType gsSerialPort::WriteWithEcho(char *sent,char *recd, size_t num,long timeout)


sent    :buffer large enough to accomodate num to send
recd    :buffer large enough to accomodate echoed data, should be same as above parameter
num     :number of bytes to send/receive
timeout :will return timeout error if any char takes longer (in ms) to echo
******************************************************************************************/

errorType gsSerialPort::WriteWithEcho(char *sent,char *recd, size_t num,long timeout)
{
errorType rv=errNone;
size_t i;
bool echoError=false;
bool timeoutError=false;

    for(i=0;i<num;i++)
    {
        rv=WritePort(&sent[i],1);
        if(rv)
            return(rv);
        rv=InQ()->Read(&recd[i],timeout);
        if(rv == errTimeout)
            timeoutError=true;
        if(sent[i] != recd[i])
            echoError=true;
    }
    if(timeoutError)
        return errTimeout;
    if(echoError)
        return(errEcho);
    return errNone;
}


/*****************************************************************************************
    HANDLE gsSerialPort::GetHandle()
return the handle for the device
******************************************************************************************/
HANDLE gsSerialPort::GetHandle()
{

    if(m_handle == INVALID_HANDLE_VALUE)
		GS_SERIAL_THROW(wxString("GetHandle"),errFail);

//code will execute if we are not using exceptions
    if(m_handle == INVALID_HANDLE_VALUE)
        return(INVALID_HANDLE_VALUE);


    return(m_handle);
}


//---------------------------------------------------------------------------
// BOOL gsSerialPort::WritePort(char *buff ,unsigned int num_to_write)
//
//  Description:
//  Write a buffer to the COM port
//
//  Parameters:
//
//     char *buff
//        pointer to data to write to port
//function taken from MS MSDN TTY.c application
//---------------------------------------------------------------------------

errorType gsSerialPort::WritePort(char *buff ,unsigned int num_to_write)
{
BOOL    bv;                //use BOOL rather than bool as it is an int
DWORD   bytesWritten ;
DWORD   errorFlags;
DWORD   error;
DWORD   bytesSent=0;
COMSTAT comStat;

resend_label:   //On a write timeout we simply resend, danger of infinite loop though seems to work ok.
   bv= WriteFile( m_handle, buff, num_to_write,
                  &bytesWritten, &m_ovlWrite );

   // Note that normally the code will not execute the following
   // because the driver caches write operations. Small I/O requests
   // (up to several thousand bytes) will normally be accepted
   // immediately and WriteFile will return true even though an
   // overlapped operation was specified

   if (!bv)
   {
      if(GetLastError() == ERROR_IO_PENDING)
      {
         // We should wait for the completion of the write operation
         // so we know if it worked or not

         // This is only one way to do this. It might be beneficial to
         // place the write operation in a separate thread
         // so that blocking on completion will not negatively
         // affect the responsiveness of the UI

         // If the write takes too long to complete, this
         // function will timeout according to the
         // CommTimeOuts.WriteTotalTimeoutMultiplier variable.
         // This code logs the timeout but does not retry
         // the write.

         while(!GetOverlappedResult(m_handle,
            &m_ovlWrite, &bytesWritten, TRUE ))
         {
            error = GetLastError();
            if(error == ERROR_IO_INCOMPLETE)
            {
               // normal result if not finished
               bytesSent += bytesWritten;
               continue;
            }
            else
            {
               // an error occurred, try to recover
               ClearCommError( m_handle, &errorFlags, &comStat ) ;
               if ((errorFlags > 0))
               {
                    errorFlags=errorFlags;
               }
               break;
            }
         }

         bytesSent += bytesWritten;

         if( bytesSent != num_to_write )
         {
            bytesSent=bytesSent;            //probable Write timeout
            error=GetLastError();           //resend the byte
            goto  resend_label;
         }
         else
             bytesSent=bytesSent;
      }
      else
      {
         // some other error occurred
         ClearCommError( m_handle, &errorFlags, &comStat ) ;
         if (errorFlags > 0)
         {
         }
         return ( errFail );
      }
   }
#if DEBUG_GS_SERIAL_PORT
    if(m_writeToFileFlag)
    {
        for (int i=0;i<bytesSent;i++)
        {
            m_DEBUGString.Printf("%i\n",buff[i]);
            m_DEBUGOutStream->WriteString(m_DEBUGString);
        }
    }
#endif
   return ( errNone ) ;

} // end of WritePort()


//******************************************************************************************************
//
//          Start gsSerialThread
//
//******************************************************************************************************

/***********************************************************************
            	ctor

gsSerialThread::gsSerialThread(gsSerialPort *dev,CQueue *q)

************************************************************************/

gsSerialThread::gsSerialThread(gsSerialPort *device)
	: wxThread(wxTHREAD_JOINABLE)
{
    m_isPaused=false;               //service thread will set this if it is paused, false otherwise
	m_gsSerialPort=device;
	m_CustomEventHandler=NULL;
    m_handle=device->GetHandle();

    memset( &m_ovlRead, 0, sizeof( OVERLAPPED ) ) ;
   	m_ovlRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(m_ovlRead.hEvent == NULL)
	{
		/*these are detected by WaitCommEvent*/
        GS_SERIAL_THROW(wxString("Failed to set CreateEvent"),errFail);
    }
}

/*******************************************************************
	dtor
	GSISerialThread::~GSISerialThread()
********************************************************************/
gsSerialThread::~gsSerialThread()
{
wxUint32 e=0;

    if (m_ovlRead.hEvent !=0)
    {
        if (CloseHandle (m_ovlRead.hEvent)==0)
        {
            e=GetLastError();
        }
    }
}


//---------------------------------------------------------------------------
//  int gsSerialThread::ReadPort(char *buff, int max_length )
//
//  Description:
//     Reads the COM port and places data it into buff
//
//  Parameters:
//
//     char * buff
//        data buffer to store data
//
//     int max_length
//        max length of block to read
//Taken from MS MSDN TTY.c
//---------------------------------------------------------------------------

errorType gsSerialThread::ReadPort(char *buff, size_t max_length,int * num_read)
{
BOOL       brv;
COMSTAT    ComStat ;
DWORD      dwErrorFlags;
//DWORD      num;
DWORD      dwError;
DWORD      numToRead;
errorType rv=errNone;

   // only try to read number of bytes in queue
    ClearCommError( m_handle, &dwErrorFlags, &ComStat ) ;
    numToRead = __min( (DWORD) max_length, ComStat.cbInQue ) ;
    *num_read=0;

    if (numToRead > 0)
    {
      brv = ReadFile( m_handle, buff,
		                    numToRead, (unsigned long *)num_read, &m_ovlRead ) ;
      if (!brv)
      {
         if (GetLastError() == ERROR_IO_PENDING)
         {
            OutputDebugString("\n\rIO Pending");
            // We have to wait for read to complete.
            // This function will timeout according to the
            // CommTimeOuts.ReadTotalTimeoutConstant variable
            // Every time it times out, check for port errors
            while(!GetOverlappedResult( m_handle,
               &m_ovlRead, &numToRead, TRUE ))
            {
               dwError = GetLastError();
               if(dwError == ERROR_IO_INCOMPLETE)
                  // normal result if not finished
                  continue;
               else
               {
                  // an error occurred, try to recover
                  ClearCommError( m_handle, &dwErrorFlags, &ComStat );
                  rv=errFail;
                  break;
               }
            }
	      }
         else
         {
            // some other error occurred
            numToRead = 0 ;
            ClearCommError( m_handle, &dwErrorFlags, &ComStat ) ;
            if ( dwErrorFlags > 0)
            {
            }
         }
      }
    }
    return (errNone) ;

} // end of ReadPort()



/********************************************************************************
void gsSerialThread::OnExit()
********************************************************************************/
void gsSerialThread::OnExit()
{
	int i;
	i=1;
}


/************************************************************************************
wxThread::ExitCode GSIRS232Device:: Entry()

thread for servicing the COM port. Reads and stuffs any data
into a CQ.
Called once the RS232 port is opened
**************************************************************************************/
wxThread::ExitCode gsSerialThread:: Entry()
{
wxUint32 eventMask ;
OVERLAPPED  ovl ;

    //m_quit=false;

    memset( &ovl, 0, sizeof( OVERLAPPED ) ) ;
   // create I/O event used for overlapped read

    ovl.hEvent = CreateEvent( NULL,    // no security
                            TRUE,    // explicit reset req
                            FALSE,   // initial event reset
                            NULL ) ; // no name
    if (ovl.hEvent == NULL)
        goto __exit;

    if (!SetCommMask( m_handle, EV_RXCHAR | EV_BREAK | EV_CTS))
        goto __exit;

    m_quit=false;

    while (TestDestroy()==false && m_quit==false)
    {
        //m_isPaused=false;
        eventMask = 0 ;
        WaitCommEvent( m_handle, (unsigned long *) &eventMask, NULL );

        if ((eventMask & EV_RXCHAR) == EV_RXCHAR)   //char(s) received
        {
            m_gsSerialPort->ComEvent(gsSerialRxChar);               //user overides this virtual function
        }
        if(eventMask & EV_BREAK)
        {
                                                    //break detected
            m_gsSerialPort->ComEvent(gsSerialBreak);                //user overides this
        }
        if(eventMask & EV_CTS)
        {
                                                    //DSR changed state
            m_gsSerialPort->ComEvent(gsSerialCTS);                  //user overides this
        }
        //while(m_pause)                              //allow once through the loop before checking
        //{
        //    m_isPaused=true;
        //    Sleep(1);                               //user wants to control the port directly
        //}
    }//while(m_quit)
    CloseHandle( ovl.hEvent ) ;

__exit:
    //m_quit=false;                                   //use as handshake
	if(IsDetached() == false)
        return(NULL);
		//Exit(0);		//joinable thread exit

	return(NULL);

}




/***************************************************************************************
    gsSerialThread::PauseThread()
#Return from the blocked ReadFile with any pending char and process it.
Set flag to indicate a char was received

#Pause the service thread
#Caller must call UnPauseRx() to continue the service thread
#Must also disable the Windows Event
****************************************************************************************/


errorType gsSerialThread::PauseThread()
{
BOOL bv;
wxStopWatch sw;


    if(GetCommMask(m_handle,&m_oldCommMask)==0)
    {
        GS_SERIAL_THROW("Failed to GetCommMask",errFail);
        return(errFail);
    }
    if(SetCommMask(m_handle,0)==0)          //disable all events
    {
        GS_SERIAL_THROW("Failed to SetCommMask",errFail);
        return(errFail);                    //ok, the thread is paused, will block in WaitCommEvent()
    }

    bv=PurgeComm(GetHandle(),PURGE_RXABORT);
    if(bv==0)
    {
        uint32_t e;
        e=GetLastError();
        SetCommMask(m_handle,m_oldCommMask);      //set CommMask to re-enable events
        GS_SERIAL_THROW("Failed to PurgeComm",errFail);
        return errFail;
    }

    m_isPaused=true;

    return(errNone);
}

errorType gsSerialThread::UnPauseThread()
{
    if(SetCommMask(m_handle,m_oldCommMask)==0)      //set CommMask to re-enable events
    {
        GS_SERIAL_THROW("Failed to SetCommMask",errFail);
        return errFail;
    }
    m_isPaused=false;

    return(errNone);
}



