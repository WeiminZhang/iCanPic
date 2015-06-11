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
//NOTE this source file has not been tested. Suggest a re-write using boost asio
/* linux specific rs232, not tested
*/
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#if defined(__GNUG__) && !defined(__APPLE__)
#  pragma implementation "l_rs232.cpp"
#  pragma interface "l_rs232.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"


#ifdef __BORLANDC__
#  pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
#error "check this: http://wiki.codeblocks.org/index.php?title=Precompiled_headers"
#  include "wx/wx.h"
#endif


#if !wxUSE_THREADS
    #error "This file requires thread support!"
#endif // wxUSE_THREADS

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

#ifdef __VISUALC__
#pragma warning(pop)
// without this pragma, the stupid compiler precompiles #defines below so that
// changing them doesn't "take place" later!
#pragma hdrstop
#endif




#include "include/gsi.hpp"
#include "include/socket.hpp"

#define BAUDRATE B115200


void signal_handler_IO (int status);    //definition of signal handler
volatile sig_atomic_t  GlbCharReceivedFlag=false;                     //FALSE while no signal received

long BAUD;                      // derived baud rate from command line
long DATABITS;
long STOPBITS;
long PARITYON;
long PARITY;


#ifndef WIN32       //linux version

InitComPort()
{

  //errorType rv;
    master=true;  //start out life as master
    m_echoFlag=true;
    m_DefaultInQ=NULL;  //if we throw an exception dont want to erroneously delete the Q in dtor
    m_DefaultOutQ=NULL;
    m_DefaultsyncQ=NULL;

    input = fopen("/dev/tty", "r");      //open the terminal keyboard
    output = fopen("/dev/tty", "w");     //open the terminal screen

    if (!input || !output)
    {
        strcpy(message,strerror(errno));
        strcat(message,device.c_str());
      //strcat(message,device);
#if GSI_USE_EXCEPTIONS
        throw(gsException(message,errAccess));
#endif
    }

    m_DefaultInQ=DEBUG_NEW CQueue(qsize); //default cq, use can change this
    m_DefaultOutQ=DEBUG_NEW CQueue(qsize); //default cq, use can change this
	m_DefaultSlaveQ=DEBUG_NEW CQueue(qsize);
	m_DefaultAsyncQ=DEBUG_NEW CQueue(qsize);//default cq, user cannot change

    m_InQ=m_DefaultInQ;
    m_OutQ=m_DefaultOutQ;
    m_SlaveQ=m_DefaultSlaveQ;
	//m_AsyncQ=m_DefaultAsyncQ;		//m_AsyncQ, user can change this

	if(m_InQ == NULL || m_OutQ == NULL || m_SlaveQ == NULL)
    {
        fclose(input);
        fclose(output);
	    delete(m_InQ);		//safe to delete these as we can delete a NULL safely
	    delete(m_OutQ);
	    delete(m_SlaveQ);
#if GSI_USE_EXCEPTIONS
        throw(gsException(message,errNew));
#endif
    }


    CheckParameters(baud_rate);
  //GSIClassIntHandler=this.DataReceivedInterrupt;


    sprintf(message,"Data Bits=%i  Stop Bits=%i  Parity=%i  Format=%i\r\n"
	      ,GSIRS232DataBits, GSIRS232StopBits, GSIRS232Parity, 1);
    fputs(message,output);

    tty = open("/dev/tty", O_RDWR | O_NOCTTY | O_NONBLOCK); //set the user console port up


    tcgetattr(tty,&oldkey); // save current port settings   //so commands are interpreted right for this program
  // set new port settings for non-canonical input processing  //must be NOCTTY
    newkey.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    newkey.c_iflag = IGNPAR;
    newkey.c_oflag = 0;
    newkey.c_lflag = 0;       //ICANON;
    newkey.c_cc[VMIN]=1;
    newkey.c_cc[VTIME]=0;
    tcflush(tty, TCIFLUSH);
    tcsetattr(tty,TCSANOW,&newkey);


    BAUD=BAUDRATE;
    DATABITS=CS8;
    STOPBITS=0;
    PARITYON=0;
    PARITY=0;

    DLL_ASSERT(BAUD==baud_rate);
    //DLL_ASSERT(1);


    //open the device(com port) to be non-blocking (read will return immediately)
    //std::cout << "opening " << device << " \n";
    DLL_LOG_MESSAGE("Opening %s\n\r",device.c_str());
    //std::cout << "??Use wxFile?? Can obtain fd from wxFile\n";
    DLL_LOG_MESSAGE("?? Use wxFile ?? Can obtain fd from wxFile\n\r");

    //this open appears to work, however we change NON_BLOCK/ASYNC using fctl later
    //fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK |O_FSYNC); //??O_FSYNC
    fd = open(device.c_str(), O_RDWR | O_NOCTTY); //??O_FSYNC

    if (fd < 0)
    {
      //perror(devicename);
      //restore old params
      tcsetattr(fd,TCSANOW,&oldtio);
      tcsetattr(tty,TCSANOW,&oldkey);
      tcsetattr(tty,TCSANOW,&oldkey);
      close(tty);
      fclose(input);
      fclose(output);

      strcpy(message,strerror(errno));
      strcat(message,device.c_str());
    #if GSI_USE_EXCEPTIONS
      throw(gsException(message,errAccess));
    #endif
    }

    GlbGSIRS232Device=this;

    //install the serial handler before making the device asynchronous
    saio.sa_handler = signal_handler_IO;
    //saio.sa_handler=GSIClassIntHandler;
    sigemptyset(&saio.sa_mask);   //saio.sa_mask = 0;
    saio.sa_flags = 0;
    saio.sa_restorer = NULL;
    sigaction(SIGIO,&saio,NULL);

    // allow the process to receive SIGIO
    fcntl(fd, F_SETOWN, getpid());
    // Make the file descriptor asynchronous (the manual page says only
    // O_APPEND and O_NONBLOCK, will work with F_SETFL...)
    {
    int oldFlags,rc;
    oldFlags = fcntl (fd, F_GETFL, 0); //get flags
    if(oldFlags==-1)
      throw(gsException(message,errFail));

    oldFlags |= O_NONBLOCK;  //or in our flags
    oldFlags |= FASYNC;
    rc=fcntl (fd, F_SETFL, oldFlags); //set the flag
    if(rc == -1)
    {
    #if GSI_USE_EXCEPTIONS
      throw(gsException(message,errFail));
    #endif
    }

    /*Set bit fields for input/output*/
    tcgetattr(fd,&oldtio); // save current port settings
    // set new port settings for canonical input processing

    tcgetattr(fd,&newtio);//get a copy of current cflags
    //----------------------------------------
    //            iflag members
    //----------------------------------------
    //input parity disable
    newtio.c_iflag &= ~INPCK;//zero bits

    //set IGNPAR Field
    newtio.c_iflag &= ~IGNPAR;//zero bits
    newtio.c_iflag |= IGNPAR;

    //don't strip to 7 bits
    newtio.c_iflag &= ~ISTRIP;

    //IGNBRK ignore received breaks. Might want them enabled later
    newtio.c_iflag &= ~IGNBRK;//zero bits
    newtio.c_iflag |= IGNBRK;
    //BRKINT only used if IGNBRK is 0

    //IGNCR allow CR through
    newtio.c_iflag &= ~IGNCR;//zero bits

    //ICRNL don't conver \r to \n
    newtio.c_iflag &= ~ICRNL;//zero bits

    //INLCR don't convert \n to \r
    newtio.c_iflag &= ~INLCR;

    //IXOFF no xon-xoff on input
    newtio.c_iflag &= ~IXOFF;//zero bits
    //IXON no xon-xoff on output
    newtio.c_iflag &= ~IXON;//zero bits

    /**************************************************
           output flags
    **************************************************/
    //OPOST tx as is, no translations
    newtio.c_oflag &= ~OPOST;

    //ONLCR no \n -> \r\l
    newtio.c_oflag &= ~ONLCR;

    //OXTABS no tab-> space conversion
    //newtio.c_oflag &= ~OXTABS;

    //ONOEOT keep C-d
    //newtio.c_oflag &= ~ONOEOT;

    /**************************************************
           control flags
    **************************************************/

    //CLOCAL, attached locally so ignore modem lines
    newtio.c_cflag &= ~CLOCAL;
    newtio.c_cflag |=CLOCAL;

    //HUPCL no modem close
    newtio.c_cflag &= ~HUPCL;

    //CREAD can read input from terminal
    newtio.c_cflag &= ~CREAD;
    newtio.c_cflag |= CREAD;

    //CSTOPB one stop bit
    newtio.c_cflag &= ~CSTOPB;

    //PARENB parity disabled
    newtio.c_cflag &= ~PARENB;
    //newtio.c_cflags |=DATABITS; parity is off so ignore this field
    //PARODD
    //can ignore this field as only has meaning if parity enabled

    //CSIZE set to 8 bits/byte
    newtio.c_cflag &= ~CSIZE;
    newtio.c_cflag |= CS8;

    /**********************************************************
               Local modes
    **********************************************************/
    //ICANON
    newtio.c_lflag &= ~ICANON;

    //ECHO no echo to terminal (pic will do this)
    newtio.c_lflag &= ~ECHO;

    //ECHOE only meaningful when ICANON set

    //ECHOPRT ignore BSD extension

    //ECHOK only meaningful when ICANON set

    //ISIG disable recognition of special chars manual urges caution!
    newtio.c_lflag &= ~ISIG;

    //NOKERNINFO disable STATUS char
    //newtio.c_lflag &= ~NOKERNINFO;

    /*******************************************************
           set speed
    *********************************************************/

    //set baud rate
    cfsetispeed (&newtio,BAUD );
    cfsetospeed (&newtio,BAUD );


    //VMIN number of bytes before a read will return
    newtio.c_cc[VMIN]=1;
    // VTIME how long to wait for input before return in .1 secs
    newtio.c_cc[VTIME]=0;


    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);

}

#endif

#if 0
//********************************************************************

//                GSIRS232Device functions


//*********************************************************************
//                ctor
//********************************************************************
//GSIRS232Device::GSIRS232Device(GSINode * gsi_node,const wxString& device, long baud_rate,size_t qsize,GSIFrame *gsi_frame)
//:GSIDevice(gsi_node,gsi_frame) //, GSISerialThread(wxTHREAD_DETACHED)

  //GSIRS232Device::GSIRS232Device(char * device, long baud_rate,size_t qsize)

GSIRS232Device::GSIRS232Device(const wxString& device, long baud_rate,size_t qsize,GSIFrame *gsi_frame)
:GSIDevice(device,gsi_frame) //, GSISerialThread(wxTHREAD_DETACHED)

{
wxString message;

    m_mode=GSIModeAscii;		//set variable to default value. The node powers up in Ascii mode
    m_echoFlag=true;					//pic initialises as ascii;echo
    m_binaryModeActionFlag=BinModeMaster;			//default to write record
    m_masterSlave=GSIMaster;	//initially a master

    m_asyncRecordReceived=false;
    m_ServiceThread=0;
    m_InQ=0;
    m_OutQ=0;
    m_SlaveQ=0;
    //m_AsyncQ=0;


#ifdef WIN32
	DWORD dwError;
	m_hPort = CreateFile (device.c_str(), // Pointer to the name of the port
		GENERIC_READ | GENERIC_WRITE,
		// Access (read-write) mode
		0,            // Share mode
		NULL,         // Pointer to the security attribute
		OPEN_EXISTING,// How to open the serial port
		0,            // Port attributes (non-overlapped)
		NULL);        // Handle to port with attribute
	// to copy
#if GSI_USE_EXCEPTIONS
	if (m_hPort== INVALID_HANDLE_VALUE)
	{
		throw(gsException(errFileOpen,wxString(_("Invalid handle:"))));
	}
#else
    if (m_hPort== INVALID_HANDLE_VALUE)
        return;
#endif

	// Initialize the DCBlength member.
	m_portDCB.DCBlength = sizeof (DCB);

	// Get the default port setting information.
	GetCommState (m_hPort, &m_portDCB);

	// Change the DCB structure settings.
#if DEBUG_USE_COMM_MASK
	m_portDCB.BaudRate = baud_rate;              // Current baud
	m_portDCB.fBinary = TRUE;               // Binary mode; no EOF check
	m_portDCB.fParity = FALSE;               // Enable parity checking
	m_portDCB.fOutxCtsFlow = FALSE;         // No CTS output flow control
	m_portDCB.fOutxDsrFlow = FALSE;         // No DSR output flow control
	m_portDCB.fDtrControl = DTR_CONTROL_DISABLE; //DTR_CONTROL_ENABLE;
	// DTR flow control type
	m_portDCB.fDsrSensitivity = FALSE;      // DSR sensitivity
	m_portDCB.fTXContinueOnXoff = TRUE;     // XOFF continues Tx
	m_portDCB.fOutX = FALSE;                // No XON/XOFF out flow control
	m_portDCB.fInX = FALSE;                 // No XON/XOFF in flow control
	m_portDCB.fErrorChar = FALSE;           // Disable error replacement
	m_portDCB.fNull = FALSE;                // Disable null stripping
	m_portDCB.fRtsControl = RTS_CONTROL_DISABLE; //RTS_CONTROL_ENABLE;
	// RTS flow control
	m_portDCB.fAbortOnError = true;        // Do not abort reads/writes on error
	m_portDCB.ByteSize = 8;                 // Number of bits/byte, 4-8
	m_portDCB.Parity = NOPARITY;            // 0-4=no,odd,even,mark,space
	m_portDCB.StopBits = ONESTOPBIT;        // 0,1,2 = 1, 1.5, 2
#else
	m_portDCB.BaudRate = baud_rate;              // Current baud
	m_portDCB.fBinary = TRUE;               // Binary mode; no EOF check
	m_portDCB.fParity = FALSE;               // Enable parity checking
	m_portDCB.fOutxCtsFlow = FALSE;         // No CTS output flow control
	m_portDCB.fOutxDsrFlow = FALSE;         // No DSR output flow control
	m_portDCB.fDtrControl = DTR_CONTROL_DISABLE; //DTR_CONTROL_ENABLE;
	// DTR flow control type
	m_portDCB.fDsrSensitivity = FALSE;      // DSR sensitivity
	m_portDCB.fTXContinueOnXoff = TRUE;     // XOFF continues Tx
	m_portDCB.fOutX = FALSE;                // No XON/XOFF out flow control
	m_portDCB.fInX = FALSE;                 // No XON/XOFF in flow control
	m_portDCB.fErrorChar = FALSE;           // Disable error replacement
	m_portDCB.fNull = FALSE;                // Disable null stripping
	m_portDCB.fRtsControl = RTS_CONTROL_DISABLE; //RTS_CONTROL_ENABLE;
	// RTS flow control
	m_portDCB.fAbortOnError = FALSE;        // Do not abort reads/writes on
	// error
	m_portDCB.ByteSize = 8;                 // Number of bits/byte, 4-8
	m_portDCB.Parity = NOPARITY;            // 0-4=no,odd,even,mark,space
	m_portDCB.StopBits = ONESTOPBIT;        // 0,1,2 = 1, 1.5, 2

#endif

	// Configure the port according to the specifications of the DCB
	// structure.
//EV_BREAK A break was detected on input.
//EV_CTS The CTS (clear-to-send) signal changed state.
//EV_DSR The DSR (data-set-ready) signal changed state.
//EV_ERR A line-status error occurred. Line-status errors are CE_FRAME, CE_OVERRUN, and CE_RXPARITY.
//EV_RING A ring indicator was detected.
//EV_RLSD The RLSD (receive-line-signal-detect) signal changed state.
//EV_RXCHAR A character was received and placed in the input buffer.
//EV_RXFLAG The event character was received and placed in the input buffer. The event character is specified in the device's DCB structure, which is applied to a serial port by using the SetCommState function.
//EV_TXEMPTY The last character in the output buffer was sent.
#if DEBUG_USE_COMM_MASK
    if(!SetCommMask(m_hPort,EV_BREAK || EV_CTS || EV_RXCHAR))
    {
		// Could not set the comm mask
		dwError = GetLastError ();
		CloseHandle(m_hPort);		//close the handle, it is open
		//MessageBox (hMainWnd, TEXT("Unable to configure the serial port"),
			//TEXT("Error"), MB_OK);
		message << __FILE__<<" "<<__LINE__ <<_("SetCommMask");
#if GSI_USE_EXCEPTIONS
		throw(gsException(errFail,message));
#endif
	}

#endif
//WaitCommEvent
//BOOL PurgeComm(
//HANDLE hFile,  // handle to communications resource
//DWORD dwFlags  // action to perform
//);
//PURGE_RXABORT Terminates all outstanding overlapped read operations and returns immediately, even if the read operations have not been completed

	if (!SetCommState (m_hPort, &m_portDCB))
	{
		// Could not configure the serial port.

		dwError = GetLastError ();
		CloseHandle(m_hPort);		//close the handle, it is open
		//MessageBox (hMainWnd, TEXT("Unable to configure the serial port"),
			//TEXT("Error"), MB_OK);
		message<<__FILE__<<" "<<__LINE__ <<_("SetCommState");
#if GSI_USE_EXCEPTIONS
		throw(gsException(errFail,message));
#endif
	}

	GetCommTimeouts (m_hPort, &m_commTimeouts);

	// Change the m_commTimeouts structure settings.
#if DEBUG_USE_COMM_MASK
	//m_commTimeouts.ReadIntervalTimeout = GSIEchoTimeout;  //will return a char if no others received within this timeout

    m_commTimeouts.ReadIntervalTimeout=MAXDWORD;
	m_commTimeouts.ReadTotalTimeoutMultiplier = MAXDWORD;          //don't use timeouts for reads otherwise
    m_commTimeouts.ReadTotalTimeoutConstant = 10000;		        //MAX,MAX,1000 return with timeout every 1000msec, otherwise return as son as char received

//m_commTimeouts.ReadIntervalTimeout = GSIEchoTimeout;  //will return a char if no others received within this timeout
    //m_commTimeouts.ReadIntervalTimeout=0;
	//m_commTimeouts.ReadTotalTimeoutMultiplier = 0;          //don't use timeouts for reads otherwise
	//m_commTimeouts.ReadTotalTimeoutConstant = 0;		        //MAX,MAX,1000 return with timeout every 1000msec, otherwise return as son as char received
	m_commTimeouts.WriteTotalTimeoutMultiplier = 10;
	m_commTimeouts.WriteTotalTimeoutConstant = 1000;

#else
	m_commTimeouts.ReadIntervalTimeout = MAXDWORD;
	m_commTimeouts.ReadTotalTimeoutMultiplier = 0;
	m_commTimeouts.ReadTotalTimeoutConstant = 0;		//this combination (MAXDWORD,0,0) ensures we return as soon as data is available

	m_commTimeouts.WriteTotalTimeoutMultiplier = 10;
	m_commTimeouts.WriteTotalTimeoutConstant = 1000;
#endif
// Set the timeout parameters for all read and write operations
// on the port.
	if (!SetCommTimeouts (m_hPort, &m_commTimeouts))
	{
  // Could not set the timeout parameters.
	//MessageBox (hMainWnd, TEXT("Unable to set the timeout parameters"),
              //TEXT("Error"), MB_OK);
		dwError = GetLastError ();
		message<<__FILE__<<" "<<__LINE__ <<"SetCommTimeouts";
		CloseHandle(m_hPort);		//close the handle, it is open
#if GSI_USE_EXCEPTIONS
		throw(message,errFail);
#endif
	}


	//_CrtMemDumpAllObjectsSince( NULL );
//instantiate Q's but use them through an alias. This way the user
//can instantiate their own Q and delete it. We retain control of
//our default Q's and can delete them.

	m_DefaultInQ=DEBUG_NEW CQueue(qsize); //default cq, user cannot change this
	m_DefaultOutQ=DEBUG_NEW CQueue(qsize); //default cq, user cannot change this
	m_DefaultSlaveQ=DEBUG_NEW CQueue(qsize);//default cq, user cannot change
	//m_DefaultAsyncQ=DEBUG_NEW CQueue(qsize);//default cq, user cannot change
	//_CrtMemDumpAllObjectsSince( NULL );
	m_InQ=m_DefaultInQ;				//m_InQ, user can change this
	m_OutQ=m_DefaultOutQ;			//m_OutQ, user can change this
	m_SlaveQ=m_DefaultSlaveQ;		//m_SlaveQ, user can change this
	//m_AsyncQ=m_DefaultAsyncQ;		//m_AsyncQ, user can change this

	if(m_InQ == NULL || m_OutQ == NULL || m_SlaveQ == NULL || m_AsyncQ==NULL)
    {
      //fclose(input);
      //fclose(output);
		delete(m_InQ);		//safe to delete these as we can delete a NULL safely
		delete(m_OutQ);
		delete(m_SlaveQ);
		//delete(m_AsyncQ);

		CloseHandle(m_hPort);		//close the handle, it is open
#if GSI_USE_EXCEPTIONS
        throw(gsException(errNew,message));
#endif
    }
	//Run()		  //after the #else so that linux code also uses it


#else //WIN32 linux code follows

  //errorType rv;
    master=true;  //start out life as master
    m_echoFlag=true;
    m_DefaultInQ=NULL;  //if we throw an exception dont want to erroneously delete the Q in dtor
    m_DefaultOutQ=NULL;
    m_DefaultsyncQ=NULL;

    input = fopen("/dev/tty", "r");      //open the terminal keyboard
    output = fopen("/dev/tty", "w");     //open the terminal screen

    if (!input || !output)
    {
        strcpy(message,strerror(errno));
        strcat(message,device.c_str());
      //strcat(message,device);
#if GSI_USE_EXCEPTIONS
        throw(gsException(message,errAccess));
#endif
    }

    m_DefaultInQ=DEBUG_NEW CQueue(qsize); //default cq, use can change this
    m_DefaultOutQ=DEBUG_NEW CQueue(qsize); //default cq, use can change this
	m_DefaultSlaveQ=DEBUG_NEW CQueue(qsize);
	//m_DefaultAsyncQ=DEBUG_NEW CQueue(qsize);//default cq, user cannot change

    m_InQ=m_DefaultInQ;
    m_OutQ=m_DefaultOutQ;
    m_SlaveQ=m_DefaultSlaveQ;
	//m_AsyncQ=m_DefaultAsyncQ;		//m_AsyncQ, user can change this

	if(m_InQ == NULL || m_OutQ == NULL || m_SlaveQ == NULL)
    {
        fclose(input);
        fclose(output);
	    delete(m_InQ);		//safe to delete these as we can delete a NULL safely
	    delete(m_OutQ);
	    delete(m_SlaveQ);
        //delete(m_AsyncQ);
#if GSI_USE_EXCEPTIONS
        throw(gsException(message,errNew));
#endif
    }


    CheckParameters(baud_rate);
  //GSIClassIntHandler=this.DataReceivedInterrupt;


    sprintf(message,"Data Bits=%i  Stop Bits=%i  Parity=%i  Format=%i\r\n"
	      ,GSIRS232DataBits, GSIRS232StopBits, GSIRS232Parity, 1);
    fputs(message,output);

    tty = open("/dev/tty", O_RDWR | O_NOCTTY | O_NONBLOCK); //set the user console port up


    tcgetattr(tty,&oldkey); // save current port settings   //so commands are interpreted right for this program
  // set new port settings for non-canonical input processing  //must be NOCTTY
    newkey.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
    newkey.c_iflag = IGNPAR;
    newkey.c_oflag = 0;
    newkey.c_lflag = 0;       //ICANON;
    newkey.c_cc[VMIN]=1;
    newkey.c_cc[VTIME]=0;
    tcflush(tty, TCIFLUSH);
    tcsetattr(tty,TCSANOW,&newkey);


    BAUD=BAUDRATE;
    DATABITS=CS8;
    STOPBITS=0;
    PARITYON=0;
    PARITY=0;

    DLL_ASSERT(BAUD==baud_rate);
    //DLL_ASSERT(1);


    //open the device(com port) to be non-blocking (read will return immediately)
    //std::cout << "opening " << device << " \n";
    DLL_LOG_MESSAGE("Opening %s\n\r",device.c_str());
    //std::cout << "??Use wxFile?? Can obtain fd from wxFile\n";
    DLL_LOG_MESSAGE("?? Use wxFile ?? Can obtain fd from wxFile\n\r");

    //this open appears to work, however we change NON_BLOCK/ASYNC using fctl later
    //fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK |O_FSYNC); //??O_FSYNC
    fd = open(device.c_str(), O_RDWR | O_NOCTTY); //??O_FSYNC

    if (fd < 0)
    {
      //perror(devicename);
      //restore old params
      tcsetattr(fd,TCSANOW,&oldtio);
      tcsetattr(tty,TCSANOW,&oldkey);
      tcsetattr(tty,TCSANOW,&oldkey);
      close(tty);
      fclose(input);
      fclose(output);

      strcpy(message,strerror(errno));
      strcat(message,device.c_str());
    #if GSI_USE_EXCEPTIONS
      throw(gsException(message,errAccess));
    #endif
    }

    GlbGSIRS232Device=this;

    //install the serial handler before making the device asynchronous
    saio.sa_handler = signal_handler_IO;
    //saio.sa_handler=GSIClassIntHandler;
    sigemptyset(&saio.sa_mask);   //saio.sa_mask = 0;
    saio.sa_flags = 0;
    saio.sa_restorer = NULL;
    sigaction(SIGIO,&saio,NULL);

    // allow the process to receive SIGIO
    fcntl(fd, F_SETOWN, getpid());
    // Make the file descriptor asynchronous (the manual page says only
    // O_APPEND and O_NONBLOCK, will work with F_SETFL...)
    {
    int oldFlags,rc;
    oldFlags = fcntl (fd, F_GETFL, 0); //get flags
    if(oldFlags==-1)
      throw(gsException(message,errFail));

    oldFlags |= O_NONBLOCK;  //or in our flags
    oldFlags |= FASYNC;
    rc=fcntl (fd, F_SETFL, oldFlags); //set the flag
    if(rc == -1)
    {
    #if GSI_USE_EXCEPTIONS
      throw(gsException(message,errFail));
    #endif
    }

    /*Set bit fields for input/output*/
    tcgetattr(fd,&oldtio); // save current port settings
    // set new port settings for canonical input processing

    tcgetattr(fd,&newtio);//get a copy of current cflags
    //----------------------------------------
    //            iflag members
    //----------------------------------------
    //input parity disable
    newtio.c_iflag &= ~INPCK;//zero bits

    //set IGNPAR Field
    newtio.c_iflag &= ~IGNPAR;//zero bits
    newtio.c_iflag |= IGNPAR;

    //don't strip to 7 bits
    newtio.c_iflag &= ~ISTRIP;

    //IGNBRK ignore received breaks. Might want them enabled later
    newtio.c_iflag &= ~IGNBRK;//zero bits
    newtio.c_iflag |= IGNBRK;
    //BRKINT only used if IGNBRK is 0

    //IGNCR allow CR through
    newtio.c_iflag &= ~IGNCR;//zero bits

    //ICRNL don't conver \r to \n
    newtio.c_iflag &= ~ICRNL;//zero bits

    //INLCR don't convert \n to \r
    newtio.c_iflag &= ~INLCR;

    //IXOFF no xon-xoff on input
    newtio.c_iflag &= ~IXOFF;//zero bits
    //IXON no xon-xoff on output
    newtio.c_iflag &= ~IXON;//zero bits

    /**************************************************
           output flags
    **************************************************/
    //OPOST tx as is, no translations
    newtio.c_oflag &= ~OPOST;

    //ONLCR no \n -> \r\l
    newtio.c_oflag &= ~ONLCR;

    //OXTABS no tab-> space conversion
    //newtio.c_oflag &= ~OXTABS;

    //ONOEOT keep C-d
    //newtio.c_oflag &= ~ONOEOT;

    /**************************************************
           control flags
    **************************************************/

    //CLOCAL, attached locally so ignore modem lines
    newtio.c_cflag &= ~CLOCAL;
    newtio.c_cflag |=CLOCAL;

    //HUPCL no modem close
    newtio.c_cflag &= ~HUPCL;

    //CREAD can read input from terminal
    newtio.c_cflag &= ~CREAD;
    newtio.c_cflag |= CREAD;

    //CSTOPB one stop bit
    newtio.c_cflag &= ~CSTOPB;

    //PARENB parity disabled
    newtio.c_cflag &= ~PARENB;
    //newtio.c_cflags |=DATABITS; parity is off so ignore this field
    //PARODD
    //can ignore this field as only has meaning if parity enabled

    //CSIZE set to 8 bits/byte
    newtio.c_cflag &= ~CSIZE;
    newtio.c_cflag |= CS8;

    /**********************************************************
               Local modes
    **********************************************************/
    //ICANON
    newtio.c_lflag &= ~ICANON;

    //ECHO no echo to terminal (pic will do this)
    newtio.c_lflag &= ~ECHO;

    //ECHOE only meaningful when ICANON set

    //ECHOPRT ignore BSD extension

    //ECHOK only meaningful when ICANON set

    //ISIG disable recognition of special chars manual urges caution!
    newtio.c_lflag &= ~ISIG;

    //NOKERNINFO disable STATUS char
    //newtio.c_lflag &= ~NOKERNINFO;

    /*******************************************************
           set speed
    *********************************************************/

    //set baud rate
    cfsetispeed (&newtio,BAUD );
    cfsetospeed (&newtio,BAUD );


    //VMIN number of bytes before a read will return
    newtio.c_cc[VMIN]=1;
    // VTIME how long to wait for input before return in .1 secs
    newtio.c_cc[VTIME]=0;


    tcflush(fd, TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);
#endif //WIN32

  //Service thread handles input from rs232 line. It places received chars
  //into currentQ. A CQueue * class. The class is either the masterQ
  //or the m_SlaveQ, depending on the value of masterSlave flag.
  //masterSlave is modified when data arrives from the pic node
  //"unexpectedly", ie the pic has asynchronous data to send us.

  //In order for this scheme to work it is necessary to make the SlaveQ
  //the default. Before sending data we must swap Q's and swap them back after
  //receipt of data. Indeed must swap Q before sending the last byte (an ACK/ERROR)
  //to ensure that any asynch data arrives at the correct Q. This is because the
  //pic will not send asynch data untill any pending pic->pc record is complete

	m_ServiceThread = DEBUG_NEW  GSISerialThread (this,m_InQ,m_AsyncQ,GetCustomEventHandler());


//Note that all WIN32 threads are joinable.
	int wError=m_ServiceThread->Create();
	if(wError)
	{
    	delete(m_InQ);		//safe to delete these as we can delete a NULL safely
    	delete(m_OutQ);
		delete(m_SlaveQ);
		delete(m_AsyncQ);

		CloseHandle(m_hPort);		//close the handle, it is open

#if GSI_USE_EXCEPTIONS
		throw(wxString("Error Thread Create"),errFail);
#endif
	}
	m_ServiceThread->SetPriority(GSIServiceThreadPriority);
  //Start the service thread
  if ( m_ServiceThread->Run() != wxTHREAD_NO_ERROR )
    {
    	delete(m_InQ);		//safe to delete these as we can delete a NULL safely
    	delete(m_OutQ);
		delete(m_SlaveQ);
		delete(m_AsyncQ);

		CloseHandle(m_hPort);		//close the handle, it is open

        wxLogError(wxT("Can't start thread!"));
#if GSI_USE_EXCEPTIONS
		throw(wxString("Error Thread Run"),errFail);
#endif
  }

	//m_ServiceThread->Delete();
    //Port is open
    wxString s;
    s.Printf( _("Open %s,(115200,1,N,8).\n" ),device.c_str());
    Frame()->TextCtrl()->AppendText(s);

}


errorType GSIRS232Device::CloseComPort()
{
  fflush(stdout);
  //restore previous signal handler
  sigaction(SIGIO,&prevsaio,NULL);//get old action struct for restoration

  tcsetattr(fd,TCSANOW,&oldtio);
  tcsetattr(tty,TCSANOW,&oldkey);
  close(tty);
  close(fd);        //close the com port
  fclose(input);
  fclose(output);
  return(errNone);
}
#endif
