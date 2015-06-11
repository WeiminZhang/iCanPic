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
#if defined(__GNUG__) && !defined(__APPLE__)
#  pragma implementation "rs232.cpp"
#  pragma interface "rs232.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"


#ifdef __BORLANDC__
#  pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
#error "check this: http://wiki.codeblocks.org/index.php?title=Precompiled_headers"
// Include your minimal set of headers here, or wx.h
#  include "wx/wx.h"
#endif




#if !wxUSE_THREADS
    #error "This file requires thread support!"
#endif // wxUSE_THREADS


#ifdef __VISUALC__
#pragma warning(push, 3)
#endif


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
#endif

#ifdef __VISUALC__
#pragma warning(pop)
// without this pragma, the stupid compiler precompiles #defines below so that
// changing them doesn't "take place" later!
#pragma hdrstop
#endif


#include "include/rs232.hpp"
#include "include/gsi.hpp"
#include "include/microcontroller.h"


volatile int quit=FALSE;

class GSIPauseThread
{
public:
	GSIPauseThread(GSISerialThread * thread);
	~GSIPauseThread();
private:
	GSISerialThread * t;
};

GSIPauseThread::GSIPauseThread(GSISerialThread * thread)
{

	t=thread;
	t->PauseThread();
}

GSIPauseThread::~GSIPauseThread()
{
	t->ContinueThread();
}



/***************************************************************************
 * signal handler. sets wait_flag to FALSE, to indicate above loop that     *
 * characters have been received.                                           *
 ***************************************************************************/

#if 0
void signal_handler_IO (int status)
{
  GSIRS232DeviceDataReceivedWrapper();
  status=status;
}
#endif


//Important note
//When a write is made (the first one) to the com port the SIGIO is immediately raised
//thus this is immediately called and a char is indeed read even though the device
//has never sent a char.
//Thus we must always use the echo to determine if all was well
//Could also check this behaviour further. See if we can do an initial read/write to
//clear the first raise and see if it happens again

//First run of the test program also blocks in this function. Subsequent runs complete
//normally
//If a second terminal is opened and the program is run, the first terminal completes
//but the second one is frozen!
//A library bug??



//********************************************************************

//                GSIRS232Device functions


//*********************************************************************
//                ctor
//********************************************************************


GSIRS232Device::GSIRS232Device(const wxString& device, gsBaud baud_rate,size_t qsize,GSIHandle handle,GSIFrame *gsi_frame,int rx_buff_size)
:SerialPortDevice(device,handle, gsi_frame) //, GSISerialThread(wxTHREAD_DETACHED)
{
wxString message;
    rx_buff_size;
    qsize;

    m_ComDev=NULL;
    m_mode=GSI_MODE_ASCII;		//set variable to default value. The node powers up in Ascii mode
    m_echoFlag=true;					//pic initialises as ascii;echo
    m_binaryModeActionFlag=BIN_MODE_MASTER;			//default to write record
    SetMaster();                        //no longer used, directly use CommsMutex instead
    m_asyncRecordReceived=false;

    GSIDevice::SetDevice(this);

    //Port is open
    wxString s;
    s.Printf( _("Open %s,(%i,1,N,8).\n" ),device.c_str(),baud_rate);
    //m_isOpen=true;
    SetIsOpen(true);
    Frame()->TextCtrl()->AppendText(s);

}


//******************************************************************************

//             dtor

//****************************************************************
GSIRS232Device::~GSIRS232Device()
{

	if(GetMode()==GSI_MODE_BINARY)
	{
        if( ! IsControllerReset())
        {
		    SetMode(GSI_MODE_ASCII);
        }
	}
    CloseComPort();             //will delete the underlying GSISerialPort
    //need to delete the gsSerialPort object

	//Need to ensure the Service thread is terminated. It will quite likely be blocked waiting for a char
	//as we should now be in ascii mode. We should probably ask the thread to terminate, then send a NULL.
	//The pic will respond with a NULL which we should ignore.
	/*Must delete service thread before deleting Q's as thread uses them*/
    //SetIsOpen(false);
}




void	GSIRS232Device::SetCustomEventHandler(wxEvtHandler * custom_event_handler)
{
    custom_event_handler;

}


//*****************************************************************
//        errorType GSIRS232Device::NewCQ(CQueue *new_q,CQueue *old_q)
//Place input into new CQ
//return current CQ to caller
//*****************************************************************

errorType GSIRS232Device::NewInCQ(GSICQueue *new_q,GSICQueue *old_q)
{
    old_q;
    new_q;

  return(errNotSupported);
}

/**************************************************************************
	errorType GSIRS232Device::Init()

***************************************************************************/
errorType GSIRS232Device::Init()
{
    errorType rv=errNone;
	return(rv);
}

//************************************************

//    errorType GSIRS232Device::SetMode(GSIEnum flag)

//***********************************************

errorType GSIRS232Device::SetMode(GSIEnumMode flag)
{
  errorType rv=errNone;
  assert(flag==GSI_MODE_ASCII || flag==GSI_MODE_BINARY);

  if(flag == GetMode()) return(errNone);  //already in reqd mode
  switch(flag)
    {
    case GSI_MODE_ASCII:
      //in binary mode
      rv=Write(rtRS232AsciiMode);
      if (rv==errNone)
				m_mode=GSI_MODE_ASCII;
      else
			{
				//DLL_LOG_MESSAGE("%i Error setting Ascii mode\n\r",__LINE__);
				//DLL_LOG_MESSAGE("Error setting Ascii mode\n\r");
                //wxFAIL_MSG()
				return(rv);
			}
      break;
    case GSI_MODE_BINARY:
      //in ascii mode so this might be an initial power-up cycle
      // need to ensure node has sent power on message. We can either wait
      //or send it some innocuous command and wait to receive an echo
      //Such a command would be RT_RS232_ASCII_MODE. The node is currently
      //in ascii mode so it will echo and ignore. Indeed we should perhaps
      //modify pic code to ensure that if an ascii/binary command is received
      //during power-up it will stop sending data.
      {

	ComDevice()->InQ()->Flush();
	rv=Write(rtRS232AsciiMode); //will return errEcho if first char is not AsciiMode
	//this is probably due to a Power On Reset Message
	//now look for rtRS232AsciiMode
	if(rv==errTimeout)
	  return(rv);
	if (rv ) //errEcho ??errQueueEmpty possible ??others
	  {
	    wxStopWatch sw;
	    GSIChar c='\0';
	    bool found=false;
	    do
	      {
		rv=ComDevice()->InQ()->Read((char *)&c,GSIEchoTimeout);
		switch(rv)
		  {
		  case errNone:
		    if(c == (GSIChar) rtRS232AsciiMode)
		      {
		        //DLL_LOG_MESSAGE("Found rtAscii");
			    found=true;
			    break;
		      }
			break;
		  case errTimeout:
		    return(rv);
   		  case errEcho:
		    break;
		  case errQueueEmpty:
		    return(rv);
		  default:
		    //DLL_LOG_MESSAGE("%i Unhandled error",__LINE__);
				//DLL_LOG_MESSAGE("Unhandled error");
            wxFAIL_MSG("Unknown error");
		    break;
		  }
	      }while(sw.Time() < GSITimeout &&( found == false));

	    if(found==false)
	      {
		//DLL_LOG_MESSAGE("%i SetMode Failed to echo\n\r",__LINE__);
		//DLL_LOG_MESSAGE("SetMode Failed to echo\n\r");
		return(errFail);
	      }
	  }
	GSIChar recvd;
	rv=Write(rtRS232BinaryMode,&recvd,GSIEchoTimeout); //errNone, errEcho, errTimeout
	if (rv==errNone)
		{
			m_mode=GSI_MODE_BINARY;
			m_binaryModeActionFlag=BIN_MODE_MASTER; //default binary mode. Can also be Slave, Block.
		}
	else
	{
	    Frame()->TextCtrl()->AppendText("Error setting binary mode");
	}
	  //DLL_LOG_MESSAGE("%i Error setting Binary  m_mode rv=%i\n\r",__LINE__,rv);
		//DLL_LOG_MESSAGE("Error setting Binary  mode rv=%i\n\r");
	break;
      }
    case GSI_MODE_BLOCK:
    case GSI_MODE_USER1:
    case GSI_MODE_USER2:
    case GSI_MODE_USER3:
    wxFAIL;
    break;
    }
  return(rv);
}




//****************************************************************
//           errorType Enable(GSIEnumEnable flag)
//From virtual class GSIDevice
//flag==Enable:
//Direct the data received from the serial device into this objects
//CQueue by overwriting the global CQeue * GlbCQueue

//Flag=Disable:
//Do not accept data to this objects CQueue. Overwrite GlbCQueue *
//with previous value
//Note that if the Previous CQueue object is NULL
//***************************************************************

errorType GSIRS232Device::Enable()
{
	m_enableFlag=GSI_ENABLE;
	return(errNone);
}

errorType GSIRS232Device::Disable()
{
	m_enableFlag=GSI_DISABLE;
	return(errNone);
}


//***************************************************************
//*****************************************************************
//           CheckParameters
//****************************************************************
errorType GSIRS232Device::CheckParameters(long baud_rate)
{
#ifdef WIN32
		baud_rate;
#else

  long rate;

  switch (baud_rate)
    {
    case  9600:
      rate=B9600;
      break;
    case  38400:
      rate=    B38400;
      break;
    case 57600:
      rate=B57600;
    case 115200:
	rate=B115200;
	std::cout <<" 115200 baud\n";
	break;
    default:
      rate=B115200;
    }
  //currently only support 115200 so ignore the above switch

#endif	//WIN32
  return(errNone);
}



/********************************************************************
		errorType GSIRS232Device::EchoOn()

**********************************************************************/
errorType GSIRS232Device::EchoOn()
{
	if(m_echoFlag)
		return(errNone);		//already echoing

	switch(GetMode())
		{
			case GSI_MODE_BINARY:
				{
					GSIRecord r(rtRS232EchoToggle);		//record is filled in correctly for records with no data
					return(Write(r));
				}
			case 	GSI_MODE_ASCII:
				{
					GSIChar recd;
					return(Write(rtRS232EchoToggle,&recd));
				}
            case GSI_MODE_BLOCK:
            case GSI_MODE_USER1:
            case GSI_MODE_USER2:
            case GSI_MODE_USER3:
            wxFAIL;
			break;
		}
	return(errBadMode);
}


/********************************************************************
		errorType GSIRS232Device::EchoOff()

**********************************************************************/
errorType GSIRS232Device::EchoOff()
{
	if(m_echoFlag==false)
		return(errNone);		//already off

	switch(GetMode())
		{
			case GSI_MODE_BINARY:
				{
					GSIRecord r(rtRS232EchoToggle);		//record is filled in correctly for records with no data
					return(Write(r));
				}
			case 	GSI_MODE_ASCII:
				{
					GSIChar recd;
					return(Write(rtRS232EchoToggle,&recd));
				}
            case GSI_MODE_BLOCK:
            case GSI_MODE_USER1:
            case GSI_MODE_USER2:
            case GSI_MODE_USER3:
            wxFAIL;
            break;
		}
	return(errBadMode);
}


//******************************************************************
//   errorType GSIRS232Device::Write(GSIEnumRecord record)
//
//Send the enumerated record to the device, in this instance the serial port
//In Ascii mode this is not a full binary record, but a single byte record

//In binary mode it is a record
//NOTE
//In binary mode REPEAT,ACK,ERROR,FATAL are also single bytes
//*****************************************************************
errorType GSIRS232Device::Write(GSIEnumRecord e_record)
{
  GSIChar tChr=(GSIChar)e_record;
  errorType rv;

  if(GetMode()==GSI_MODE_BINARY)
    {
	  //look for special cases. Single byte "records" actually handshaking bytes
	  //if(e_record == rtGSI_REPEAT || e_record == rtGSI_ACK || e_record == rtGSI_ERROR || e_record == rtGSI_FATAL)
	//	  break;
      GSIRecord record(e_record); //Instantiate the Record class
      rv=Write(record);  //send the record class to the device
      return(rv);
    }

//This is a write to the comport using a library function
  GSIChar recvd;
  rv=Write(tChr,&recvd,GSIEchoTimeout);
  return(rv);
}

//******************************************************************
//   errorType GSIRS232Device::Write(const GSIChar chr,GSIChar *recvd)
//
//Send the char to the device, in this instance the serial port
//Note that binary Write is always an implicit echo. The BinaryMode
//function will set the flag and restore to original when Ascii mode
//is set
//*****************************************************************
errorType GSIRS232Device::Write(const GSIChar c,GSIChar *recvd)
{

  return(ComDevice()->WriteWithEcho((char)c,(char *)recvd,GSIEchoTimeout));

}


/****************************************************************************
//   errorType GSIRS232Device::Write(GSIRecord *record)
//
//Send the record to the device, in this instance the serial port
//must be in binary mode. This is also an implicit echo

	From pic code. Must look for NULL outside of
Must only use this function for sending records as we set a "sendingRecord"
variable and reset it after transmission. The service thread uses
this variable to determine if a NULL arrives outside of a record, in binary mode
If so, then pic node has data to send us.
Should probably make this a class so that if we use exeptions and this routine
is aborted then the lock will be restored.
;-------------------------------------------------------------------------------
;	GSICANDataPendingTxToRS232
;Called fom main loop when SIO_S1_CAN_DATA_PENDING_TX is set
;
;Data is in the CnTxRecord structure
;If we are in Slave mode then we need to become temporary master
;Do this by sending a NULL down rs232
;The pc will +not+ respond with an echo
;it will send an rs232 message subfunction GSI_RS232_MESSAGE_REQ_SLAVE
;once it is free to do so.
;on receipt of this, this node becomes RS232Master
;If the pc is currently sending data to this node then it will
;ignore the NULL. In this case this function will repeatedly be
;called by the main program loop till the request has been met.
;If we need to use NULL as a general signalling mechanism
;then we can send the NULL then receive an RS232 mssg requesting
;the node inform the master what function it requires.
;
;If we are not in a binary mode then bcf the SIO_S1_CAN_DATA_PENDING_TX bit
;-------------------------------------------------------------------------


Should probably pause the service thread, then write and read echo in this
function, otherwise we will be waiting on the service thread to run before
we see the echoed char and will thus slow our reception rate
***************************************************************************/
errorType GSIRS232Device::Write(GSIRecord &r)
{
  errorType rv=errNone;

  if(GetMode() != GSI_MODE_BINARY)
		return(errBadMode);

    wxMutexLocker mx(GetCommsMutex());
    if(mx.IsOk() == false)
    {
        GSI_THROW("Failed to lock mutex",errFail);
        return(errFail);
    }
    rv=SendRecordWithEcho(r);

    return(rv);

}


/*****************************************************************************************************
errorType  GSIRS232Device::SendRecordWithEcho(GSIRecord &r)
This function is effectively WriteRecord. However it is called with a CriticalSectionLocker in place
so that sync/asnyc records don't get mixed up

If the microcontroller has a problem during Tx it will wdt, then send 2 * max record length NULLS
followed by a BREAK
If this code detects a NULL when it expected a Type, then it should quietly abort.
The CommEvent() will detect the BREAK and take appropriate action
*******************************************************************************************************/
errorType  GSIRS232Device::SendRecordWithEcho(GSIRecord &r)
{
errorType rv=errNone;
bool echoError=false,slaveReq=false;
int i,repeatCount;
wxStopWatch sw;
GSIChar recd;

#warning  ("can remove this while loop?")
#if 1       //added 5/1/07
    //while( GetMasterSlave() == GSISlave) //If we are slave then yield()
    while( IsTxSlave()) //If we are slave then yield()
    {
        wxSafeYield(NULL,true);
        //Sleep(0);
    }
#endif
	wxString s;
	s.Printf("----------------------\n"
				"rs232 write: %s : %s\n"
		     "----------------------\n",
			  r.GetTypeString().c_str(), r.GetSubfuncString().c_str());

	Frame()->TextCtrl()->AppendText(s);
  //Write the record a byte at a time and check for an echo. If bad echo then
  //send a GSI_REPEAT rather than GSI_ACK when we find the ACK

    //GSIPauseThread pause(m_ServiceThread);				//Pauses thread, Continues in dtor


    for(repeatCount=0;repeatCount<GSIRepeatCount;repeatCount++)
	{
		echoError=false; //reset for next time through the loop
		//rv=WriteUnbuffered(r.GetType(),&recd,GSIEchoTimeout*100); //write/read recordType
        rv=ComDevice()->WriteWithEcho((char)r.GetType(),(char *)&recd,GSIEchoTimeout);
		//DLL_LOG_MESSAGE("type %x:%x %i\n\r",pr.type,recd,rv);
		switch(rv)
		{
		case errNone:
			break;
		case errEcho:
			if(recd=='\0')
			{
#if DEBUG_ENABLE_BREAK
                return(errFail);
#else
//No longer valid to receive a NULL as a type. We now use BREAK to initiate asyncronous coms
//a null means we should abort

            //if NULL then may be legitimate communication from node
				//If a single NULL then ignore it as it is the pic attempting
				//to send asych data. Our Tx'd bytes crossed. The pic will continue
				//in its main loop and will attempt to regain comms later.
				//If we find this asynch NULL from the pic then we must send
				//it a RequestSlave record. We become slave, it sends us the data
				//it sends a RequestSlave to become slave, we become master
				//Using this scheme the pc is effectively always the master.
				//It just needs to detect a NULL between records (while in binary
				//mode).
				//If more than one NULL then the pic is re-synching
//Look for other occurrences of this ReSynchCode!
                int nullCount=1;
				rv=ReSynchNode(&recd,&nullCount);
                switch (rv)
                {
                case errGSISlaveReq:        //node has sent a single null followed within timeout by a byte
                                            //we must process the byte as if it was received normally
                                            //must also talk to the node to get the data once processing has finished
#warning ("Need work here to retrieve asynchronous data from the node")
                    slaveReq=true;           //on exit will handle this condition
                    break;
                case errGSIReSync:          //received >1 NULL followed by an echoed GSI_ACK, node has reset itself
                    wxASSERT(0);
                    break;
                case errEcho:
                case errTimeout:
                default:
                    return(errFail);
                    break;      //All fatal
                }   //switch(rv)

								//continue processing, recd should hold pr->type
			    if(recd != r.GetType())
    			    echoError=true;
                break;
#endif //ENABLE_BREAK
			} //recd=='\0'
			else    // if(recd=='\0'), so a genuine echo error
                echoError=true; //continue with processing, as we will request a REPEAT
			break; //case errEcho
		case errQueueEmpty:
			//DLL_LOG_MESSAGE("Error:Q empty\n\r");
			break;
		case errTimeout:
            //these errors, certainly the Timeout error cannot be ignored!
            rv=rv;
		case errFail:
		default:
			return(rv);  //nasty error
		}

		//rv=WriteUnbuffered(r.GetNode(),&recd,GSIEchoTimeout);
        rv=ComDevice()->WriteWithEcho((char)r.GetNode(),(char *)&recd,GSIEchoTimeout);
		if (rv==errFail )
			return(rv); //write error is probably fatal
		if(rv==errTimeout)
			return(rv);
		if(rv==errEcho)
			echoError=true;

		//rv=WriteUnbuffered(r.GetLength(),&recd,GSIEchoTimeout);
        if(r.GetLength() > GSIRecordMaxDataLength)
        {
            wxASSERT_MSG(r.GetLength() <= GSIRecordMaxDataLength, "Bad record length");
        }
        rv=ComDevice()->WriteWithEcho((char)r.GetLength(),(char *)&recd,GSIEchoTimeout);
		if (rv==errFail)
			return(rv); //write error is probably fatal
		if(rv==errTimeout)
			return(rv);
		if(rv==errEcho)
			echoError=true;
		for(i=0;i<r.GetLength();i++)
		{
            //rv=WriteUnbuffered(r.GetData(i),&recd,GSITimeout);
            rv=ComDevice()->WriteWithEcho((char)r.GetData(i),(char *)&recd,GSIEchoTimeout);

			if (rv==errFail)
				return(rv); //write error is probably fatal
			if(rv==errTimeout)
				return(rv);
			if (rv==errEcho)
				echoError=true;
		}

#if 0
		if(repeatCount <2)
		{
			//DLL_LOG_MESSAGE("Debug code setting a dummy echo error");
			echoError=true;
		}
#endif
		//if(echoError)
		//	DLL_LOG_MESSAGE("Data echo error\n\r");

		//As the node doesn't know what was sent it can only echo what it received
		//so we must send an ACK or a REPEAT to the node once the record has been
		//sent. If we had an echo error we will send a repeat

		if(echoError)
		{

			//rv=WriteUnbuffered(GSI_REPEAT,&recd,GSIEchoTimeout);
            rv=ComDevice()->WriteWithEcho((char)GSI_REPEAT,(char *)&recd,GSIEchoTimeout);
			//DLL_LOG_MESSAGE("Echo error, sent REPEAT\n\r");
			if(rv)
			{
				//DLL_LOG_MESSAGE("Failed to write Repeat\n\r");
				return(errFail);
			}
			continue; //continue the for loop
		}
		else
		{

			//rv=WriteUnbuffered(GSI_ACK,&recd,GSIEchoTimeout);
            rv=ComDevice()->WriteWithEcho((char)GSI_ACK,(char *)&recd,GSIEchoTimeout);
			if(rv)
			{
				//DLL_LOG_MESSAGE("Failed to write ACK err:%i\n\r",rv);
				//DLL_LOG_MESSAGE("Failed to write ACK \n\r");
				return(errFail);//is this correct?? What does the pic do on a bad ACK echo?
			}
			else
			{
				GSIRecord r2Record;
				GSIChar rc;
				//DLL_LOG_MESSAGE("BinRecord written\n\r");
				//pic will now respond with an RS232Message Success/fail
				rv=Read(r2Record,GSIRecordReadTimeout);
				if(rv == errNone)
				{
					//no comms error so no check for a node error of some kind
					rc=r2Record.GetData(RS232RecordOffsetToSubfunction);

                    if(r2Record.GetLength()>1)
                    {
                        GSIRecord * ReturnedRecord;
                        ReturnedRecord=DEBUG_NEW GSIRecord(r2Record);     //will be deleted in dtor (has a recursion check in copy ctor)
                        r.SetReturnedRecord(ReturnedRecord);        //will also set m_isReturnedRecord

                    }
					switch(rc)
					{
					case GSIRS232MessageSuccess:        //May return extra data
						break;
					case GSIRS232MessageFailure:
						rv=errGSIFunctionFail;          //May also return extra data, see GSIErrxxx in gsidefs.h

						break;
					case GSIRS232MessageReqSlave:
						//DLL_ASSERT(0);								//not implemented??
						break;
					case GSIRS232MessageReqMaster: /*not used, use GSIRS232MessageMASTER_REQ_BIT*/
					case GSIRS232MessageSendData:
					case GSIRS232MessageGetData:
					case GSIRS232MessageChangeNode:
						break;
					case GSIRS232MessageNewNode:
						break;
					case GSIRS232MessageN3:
					default:
                        wxFAIL_MSG("unknown RS232 message recieved");
						//DLL_ASSERT(0);
					}
				}
#warning ("Check bit 7 of Success/Fail record")

				//The record has been sent, the RS232Record has been received, rv has been set
				break; //Record sent successfully. Exit enclosing for loop

			}//rv==errNone (an else clause)
		}//echoError so loop

	}  //GSI_REPEAT loop

		//all done. If echoError is still set then we have a bad com line
	if(repeatCount==0 && echoError)
	{
		Frame()->TextCtrl()->AppendText(
			"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
			"Failed to send record, errEcho\n"
			"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		return(errEcho);
	}
#warning ("Think this is important!!")

//If an asyncronous NULL was sent as recordType, indicating the node has data for us then the node
//will not tx any async data until it has received a SetMaster request from this code.
//So, it should be safe to send the request here, before the serialThread lock has been released
//as once it has been received it wil start txing. Even if that occurs before the lock has been released, the
//next data read should be by the serial thread function?? Can we ensure this is so?
//do we need a lock so that we don't try to send records to the node while we are Slave?? This would seem to be
//rather important!
//All synchronous writes should check that we are currently master. If we are not then we can simply yield
//our timeslice so that the SerialThread can complete its work

    if(slaveReq)
    {
        wxFAIL_MSG("Shouldn't arrive here!");
        //
        errorType rv1=SetSlave();
        if(rv1)
        {
#warning ("work needed here")
            wxASSERT(0);
        }
    }
    return(rv);

}

errorType GSIRS232Device::Write(GSIChar chr)	//write a single byte without echo (handshaking etc.)
{
	//Note that this function is "dangerous" in that if the node is echoing, the char
	//will be in the input Q. It is the responsibility of the caller to handle any echo
	WritePort(chr);
	return(errNone);
}


/**********************************************************************
int GSIRS232Device::WritePort(ComPortHandle h,GSIChar *chr,size_t l)

  Perform write to Com Port using c library (linux) or Win32 API
************************************************************************/
unsigned long GSIRS232Device::WritePort(GSIChar chr)
{
	unsigned long num;
	//int rc;
    wxString s;

    //cout << chr <<" " << this->GetDeviceName() << endl;

    s.Printf("%i %s\n",chr,this->GetDeviceName().c_str());

    //GlbOutString.Append(s);

    //enum errorType WritePort(char *buff ,unsigned int num_to_write);


    //virtual errorType WriteWithEcho(char sent,char *recd,long timeout);
    //virtual errorType WriteWithEcho(char *sent,char *recd, size_t num,long timeout);
    num=1;
#ifdef __WIN32__
    //if(m_Dev->WritePort((char *) &chr,1) != errNone)
        if(ComDevice()->WritePort((char *) &chr,1) != errNone)
       num=0;
#if 0
	rc=WriteFile (m_hPort,              // Port handle
     &chr,              // Pointer to the data to write
    l,                  // Number of bytes to write
    &num, // Pointer to the number of bytes
                           // written
    NULL                // Must be NULL for Windows CE
	);
	if(rc==0)	//failed
		num=0xffffffff;
#endif

#elif defined __LINUX__
	num=write(m_hPort,&tChr,l);	//linux write using fd
	if (num != l)
		num=-1;					//error
#endif
    //Sleep(0);				//give up the rest of our timeslice to allow worker thread to run, reading the echoed

							//char from the port
    //wxSafeYield(NULL,true);
    //Sleep(0);
    //::Yield(true);
	return(num);
}
//******************************************************************
//   errorType GSIRS232Device::Write(const GSIChar c,GSIChar *recvd,long timeout)
//
//Write char to device
//Read device to find echo of sent char
//return errTimeout if timeout mS exceeded

//Note that this function assumes that the service thread is paused
//*****************************************************************

errorType GSIRS232Device::Write(const GSIChar c,GSIChar *recvd,long timeout)
{
  //wxLongLong t;
  //wxStopWatch sw;
  errorType rv;
//  GSIChar tChr=c;
//  unsigned long num;

	if(GetMode() == GSI_MODE_ASCII && GetEchoState() == false)
		return(errBadMode);
  //sw.Start(0);

  //rv=Write(c,recvd);
  //PORT_WRITE(m_hPort,tChr,sizeof(c),&numReceived);
    rv=ComDevice()->WriteWithEcho(c,(char *)recvd,timeout);
    return(rv);
  //num=WritePort(tChr);	//returns num written or 0xffffffff if error
  //num=write(fd,&tChr,sizeof(c));
#if 0
  if(num==0)
    return(errFail);
  *recvd=(GSIChar) (c+1);		//ensure bad echo if later failure
  rv=Device()->InQ()->Read((char *)recvd,timeout);
  if(rv)
    return(rv);
  if(c != *recvd)
    {
      return(errEcho);
    }
  //DLL_LOG_MESSAGE("InQ Read rv=%i\n\r",rv);
#endif
  return(errNone);
}


//********************************************************************************************
//   errorType GSIRS232Device::WriteUnbuffered(const GSIChar c,GSIChar *recvd,long timeout)
//Similar to other Write functions except that the function reads the echo directly from the
//port without using the service thread. This offers greater com speed when writing records
//It also assumes that the service thread has been paused
//Write char to device
//Read device to find echo of sent char
//return errTimeout if timeout mS exceeded

//Note that this function assumes that the service thread is paused
//************************************************************************************************

errorType GSIRS232Device::WriteUnbuffered(const GSIChar c,GSIChar *recvd,long timeout)
{

    wxStopWatch sw;
    GSIChar tChr=c;
    unsigned long num;
	errorType rv=errNone;

    num=WritePort(tChr);	//returns num written or 0xffffffff if error

    if(num==0)
        return(errFail);
    *recvd=(GSIChar) (c+1);		//ensure bad echo if later failure

	if(GetMode()== GSI_MODE_ASCII && GetEchoState() == false)
		*recvd=c;			//if we are in ascii mode and not echoing then pretend we received the char
	else	//ok must look for an echo
	{

#if DEBUG_USE_CTB
    rv=Device()->InQ()->Read((char *)recvd,timeout);
#else
    #ifdef __WIN32__
	DWORD numReceived=0;
	do
	{

    	ReadFile (m_hPort, recvd, 1, &numReceived, 0);

	}while(sw.Time() <timeout && numReceived==0);
    #else
	    wxFAIL_MSG("Unsupported operating system");
    #endif
	    if(numReceived == 0)
		    return(errTimeout);
   	    wxASSERT_MSG(numReceived==1,"NumReceived != 1");
#endif //USE_CTB
    }


	if(rv==errTimeout)
        return(rv);
    if(c != *recvd)
	    return(errEcho);
    return(rv);
}


//***************************************************************
//    errorType GSIRS232Device:: SetPCAsMaster()
//
//The node attached to this pc has requested that it be the slave
//It has sent an RS232_MESSAGE_REQUEST_SLAVE
//***************************************************************
errorType GSIRS232Device:: SetPCAsMaster()
{

  //DLL_LOG_MESSAGE("%i %s SetPCAsMaster/n/r");
  //GlbFlags.RS232ReceivedReceivedSlaveReq = True;
  SetMaster();                              //no longer used, directly use CommsMutex instead
//The CommsMutex will currently be locked as we were Rxing from the mc. We must release the mutex
    GetCommsMutex().Unlock();

  return(errNone);
}





//*****************************************************************
//        errorType Read(GSIChar *recd);

//Read a byte from the node with default timeout
//****************************************************************
errorType GSIRS232Device::Read(GSIChar *recd) const
{
	//Incoming data will appear in the InQ object
	if(ComDevice()->InQ()->GetNumInBuffer())
	{
		return(ComDevice()->InQ()->Read((char *)recd));
	}
//
	return(errEmpty);
}


//*****************************************************************
//        errorType Read(GSIChar *rcd,long timeout);

//Read a byte from the node with user specified timeout
//****************************************************************
errorType GSIRS232Device::Read(GSIChar *recd,long timeout) const
{
	return(ComDevice()->InQ()->Read((char *)recd,timeout));
}


/***************************************************************
	errorType Read(GSIChar *buff,size_t num,long timeout)
Read buffer of GSIChars using user timeout
*****************************************************************/
errorType GSIRS232Device::Read(GSIChar *buff,size_t num,size_t *num_read,long timeout) const
{

	return(ComDevice()->InQ()->CQueue::Read((char *)buff,num,num_read,timeout));
}

/********************************************************************
	errorType Read(GSIChar *buff,size_t num)
Read buffer of n GSIChars using default timeout
*********************************************************************/
errorType GSIRS232Device::Read(GSIChar *buff,size_t num) const
{
	size_t i;
	errorType rv;
	for(i=0;i<num;i++)
	{
		rv=ComDevice()->InQ()->Read((char *)buff);
		if(rv != errNone)
			return(rv);
	}
	return(errNone);
}

/*********************************************************************
	errorType GSIRS232Device::Read(GSIRecord &record, long timeout)
Read record with default timeout
***********************************************************************/

errorType GSIRS232Device::Read(GSIRecord &record)
{

	return(Read(record,0));
}

//*******************************************************************
//        errorType Read(GSIRecord *record,int fd long timeout);

//Read a record being sent from the node
//Note that this function assumes that the Service thread is paused
//******************************************************************

errorType GSIRS232Device::Read(GSIRecord &record, long timeout)
{

  errorType rv=errNone;
  GSIChar c[GSIRecordFixedLength];
  wxStopWatch sw;
  int i,repeatCounter;
  GSIChar ackType;
  bool overallToError=true,localToError=true,success=false;

  //sw.Start(0);
  const GSIBINRECORD &br=record.GetBinRecord();

  //DLL_LOG_MESSAGE("In Read(GSRecord *,...\n\r");

    //GSIPauseThread pause(m_ServiceThread);

    for(repeatCounter=0;repeatCounter<GSIRepeatCount;repeatCounter++)
	{
		for(i=0;i<GSIRecordFixedLength;i++)
		{
            localToError=true;
            //c[0]=1,c[1]=2,c[2]=3;
			do
			{
				//still need a small timeout value for the Read's the timeout used
				//for the sw.Time is an overall timeout for reading the record
				rv=ReadUnbuffered(&c[i],GSIEchoTimeout); //XXX This timeout fails regularly when it is un-multiplied
                if( i == (int) RS232RecordOffsetToLengthField && c[i] > GSIRecordMaxLength)
                {
                    wxFAIL_MSG("Bad record length");
                }
				if(rv==errNone)
				{
					//write(m_hPort,&c[i],sizeof(GSIChar));  //echo the char just read
					WritePort(c[i]);
					//DLL_LOG_MESSAGE("acknowledging fixed length char\n\r");
                    localToError=false;
					break;           //next char
				}
				else
				{
					//DLL_LOG_MESSAGE("%i rv=%i\n\r",__LINE__,rv);
					return(rv); //serious error
				}
			}while(sw.Time()<timeout);
		}
		//DLL_LOG_MESSAGE("Fixed length read\n\r");
		wxASSERT_MSG(localToError==false,"Timeout failed, ??increase it??");
		//br.length=c[RS232RecordOffsetToLengthField];
		rv=record.SetLength(c[RS232RecordOffsetToLengthField]);	//use member func to set so that can use const binRecord

		if(rv != errNone)
        {
            wxASSERT_MSG(rv==errNone,"Bad record length");
			return(rv);
        }
		//fixed length of record read
		//now read the data, if any.
		for(i=0;i<br.length;i++)
		{
            localToError=true;
			do
			{
				GSIChar tchar;

				rv=ReadUnbuffered(&tchar,GSIEchoTimeout);
				//rv=Device()->InQ()->Read(&br.data[i],GSIEchoTimeout);
				if(rv==errNone)
				{
					rv=record.SetData(i,tchar);
					//DLL_ASSERT(record.SetData(i,tchar)==errNone);
                    wxASSERT(record.SetData(i,tchar)==errNone);
					if(rv)
                    {
                        wxASSERT(rv == errNone);
						return(rv);
                    }
					//write(m_hPort,&br.data[i],sizeof(GSIChar)); //echo the data
					WritePort(br.data[i]);
					//GSXXX get an error here if this message is not sent!!!
					//DLL_LOG_MESSAGE("Acknowledging data\n\r");
					//DLL_LOG_MESSAGE("If this isn't sent we get a Problem Setting Ascci\n\r");
                    localToError=false;
					break;
				}
				else
                {
                    wxASSERT_MSG(localToError==false,"Timeout failed, ??increase it??");
                    wxASSERT(rv==errNone);
					return(rv); //serious error
                }
			}while(sw.Time() < timeout);
		}
		//DLL_LOG_MESSAGE("Data read (if any)\n\r");
		//node should now send an ACK,REPEAT,ERROR,FATAL
        localToError=true;
		do
		{
			rv=ReadUnbuffered(&ackType,GSIEchoTimeout);
			if (rv==errNone)
			{
				//write(m_hPort,&ackType,sizeof(ackType));
				WritePort(ackType);
                localToError=false;
				break; //exit the do loop
			}
			else
            {
                wxASSERT_MSG(localToError==false,"Timeout failed, ??increase it??");
                wxASSERT(rv==errNone);
				return(rv);
            }
		}while(sw.Time() <timeout);

		//ack has been read
		overallToError=false;
		switch (ackType)
		{
		case GSI_ACK:
			//record read correctly
			//DLL_LOG_MESSAGE("BinRecord read\n\r");
			record.SetType(c[RS232RecordOffsetToTypeField]);
			record.SetNode(c[RS232RecordOffsetToNodeField]);
			rv=record.SetLength(c[RS232RecordOffsetToLengthField]);
			overallToError=false;
			success=true;
			record.SetFilledFlag(true);
			break;
		case GSI_REPEAT:
			sw.Start(0); //reset the stop watch
			break;
		case GSI_ERROR:
		case GSI_FATAL:
		default:
			return(rv);
		}
		if (success)
			break;
	}//RepeatCounter

  if(success)
    return(errNone);
  if(overallToError)
    return(errTimeout);

  return(rv);
}


errorType GSIRS232Device::ReadUnbuffered(GSIChar * recvd, long timeout) const
{


    errorType rv=ComDevice()->InQ()->Read((char *)recvd, timeout);

#if 0
#if DEBUG_USE_CTB
wxStopWatch sw;

    int numReceived;
    do
	{
	    numReceived=m_Dev->Read((char *)recvd,1);


	}while(sw.Time() <timeout && numReceived==0);

#else
    #ifdef __WIN32__
    DWORD numReceived=0;
	do
	{
		ReadFile (m_hPort, recvd, 1, &numReceived, 0);

	}while(sw.Time() <timeout && numReceived==0);
    #else
		    DLL_ASSERT(0);
    #endif
#endif

	if(numReceived==0)
		return(errTimeout);

	wxASSERT_MSG(numReceived == 1, "numReceived !=1");
#endif

  return(rv);
}

#if 0
errorType GSIRS232Device::SetMaster(GSIChar node)
{
    return(GSIDevice::SetMaster(node));
}

errorType GSIRS232Device::SetSlave(GSIChar node)
{
    node;
#if 0
    errorType rv=errNone;

	GSIRecord rs232Record();
	GSIRecord slaveRecord(rtGSIRS232Message);		//instantiate the record
	slaveRecord.SetLength(1);
	slaveRecord.SetNode(node);
	rv=slaveRecord.SetData(RS232RecordOffsetToSubfunction,GSIRS232MessageReqSlave);	//set offset 0 to ReqSlave
	rv=Write(slaveRecord);
	if(rv==errNone)
	{
		rv=SetSlave();
		//pic node should respond with an rs232Record??
		//pic will now start sending us data. A top level function should do something with it.
	}
#endif
    return(GSIDevice::SetSlave());
}
#endif //if 0

//************************************************************************

//         end of GSIRS232Device

//***********************************************************************


#if 0

/***********************************************************************
			GSISerialThread
	ctor
GSISerialThread::GSISerialThread(GSIRS232Device *device,CQueue *q)

************************************************************************/
GSISerialThread::GSISerialThread(GSIRS232Device *device,GSICQueue *mq,GSICQueue *aq,wxEvtHandler *custom_event_addr)
	//: wxThread()
	: wxThread(wxTHREAD_JOINABLE)
{
	m_Device=device;
	m_MasterQ=mq;
	m_AsyncQ=aq;
	m_CurrentQ=mq;		//used in service thread to send to master or slave Q depending on masterSlave flag
	m_CustomEventHandler=custom_event_addr;
	m_pause=false;
//Async varibles

    m_asyncIndex=0;
    m_asyncBreakCount=0;
    m_asyncCursor=0;
    m_asyncLength=0;
    m_asyncNode=0;
    m_asyncType=0;
}

/*******************************************************************
	dtor
	GSISerialThread::~GSISerialThread()
********************************************************************/
GSISerialThread::~GSISerialThread()
{
	//Delete();  Can't delete from here. It must be from another thread context, thus the RS232Device dtor
}

#endif


#if    DEBUG_USE_CTB

/************************************************************************************
wxThread::ExitCode GSIRS232Device:: Entry()

thread for servicing the COM port. Reads and stuffs any data
into a CQ.
Called once the RS232 port is opened

NOTE:
The pic node will send asynchronous data (CAN Sync messages, keypad presses etc.)
These will constitute a NULL from the pic node received while we are not expecting
data. If this happens then we must send an RS232RequestSlave record and place this
library code into slave mode.
The pic node will transmit all the data it wishes, then send RS232RequestSlave to
indicate that it now wishes to be the slave. Any data received from the pic node
should be placed into a m_SlaveQ ready for the user to process.
Thus, we need a global flag to inform this function that we are looking for an echo.
If we are not looking and data arrives, and we are in binary mode, and it is a NULL
then we have pic node data awaiting us
**************************************************************************************/
wxThread::ExitCode GSISerialThread:: Entry()
{
int to = 0;         //timeout flag for timer CTB timer function. Lifetime must exceed the timer object
    m_quit=false;
    char buff[128];
    int numReceived;
    timer t(GSISerialThreadTimeout,&to,NULL); //last parameter is an exit function should we want one called

	do
	{
    	if(m_quit) break;       //exit from the function
        to=0;                   //this is a flag,0==timeout hasn't occured
        t.start();
/*
   \brief
      readv() attempts to read up to len bytes from the interface
      into the buffer starting at buf.
      readv() is blocked till len bytes are readed or the timeout_flag
      points on a int greater then zero.
      \param buf starting adress of the buffer
      \param len count bytes, we want to read
      \param timeout_flag a pointer to an integer. If you don't want
             any timeout, you given a null pointer here.
		   But think of it: In this case, this function comes never
		   back, if there a not enough bytes to read.
	 \param nice if true go to sleep for one ms (reduce CPU last),
	        if there is no byte available (default is false)
    */
    //int Readv(char* buf,size_t len,int* timeout_flag,bool nice=false);
        numReceived=Device()->InQ->m_Dev->Readv((char *)buff,1,&to,true);  //returns number of bytes still to read, so 0=success, >0 ==timeout
        if(numReceived==0)          //char not read within timeout?
        {
            if(to != 1)        //if this is 0
            {
                struct wxSerialPort_EINFO e;
                Device()->InQ->m_Dev->Ioctl(CTB_SER_GETEINFO,(void *) &e);
                continue;      //continue loop, look for quit flag etc.
            }
        }
        else
        {
			switch(Device()->InQ->m_mode)
			{
			case GSI_MODE_ASCII:
				m_CurrentQ->CQueue::Write(buff,numReceived); //write the ascii char(s) to the Q. Check ECHO On ???
				break;
			case GSI_MODE_BINARY:
				//These are different actions to be taken when in binary mode
				switch(Device()->InQ->m_binaryModeActionFlag)
				{
				//case GSIRS232Device::BIN_MODE_MASTER:							//master
				//	currentQ->Write(recd,numReceived);	//current is either MasterQ or SlaveQ depending on masterSlave flag
				//	break;
				case GSIRS232Device::BIN_MODE_MASTER:							//master
                    m_CurrentQ->CQueue::Write(buff,numReceived);
                    break;
				case GSIRS232Device::BinModeSlave:
					{
/*Potential problem:
pic is currently master. It sends a ReqSlave record once it finishes its data Tx. On receipt of the final
//ACK it assumes it is now Slave. If a new CAN frame arrives it will send us a NULL, thinking we are the master.
This NULL will arrive here. Meanwhile at any time the main thread is about to set the m_mode variable to Master
Should probably defer the ACK till after the Mode has been set to Master
*/
						errorType rv;
						rv=AssembleAsyncRecord(buff[0]);                //placed in m_AsyncQ
					}
					break;
				case GSIRS232Device::BinModeBlockRead:
				case GSIRS232Device::BinModeBlockWrite:
				case GSIRS232Device::BinModeUser1:
				case GSIRS232Device::BinModeUser2:
				case GSIRS232Device::BinModeUser3:
				default:
				    wxFAIL_MSG("Unsupported binary mode action flag option");
				    break;
				}//switch(binaryActionFlag)
				break;
			default:
				wxFAIL_MSG("Unsupported binary mode"); //DLL_ASSERT(0);
		        break;
		    }//switch m->Device()->InQ()m_mode
        } //if (rc)
	} while (m_quit==false); //while (quit==false && m_quit==false);

	//DLL_LOG_MESSAGE("\n\rEnding Serial thread");
    t.stop();

	if(IsDetached() == false)

		Exit(0);		//joinable thread exit
	return(NULL);
}

#else //DEBUG_USE_CTB above code uses CTB

#if 0
/************************************************************************************
wxThread::ExitCode GSIRS232Device:: Entry()

thread for servicing the COM port. Reads and stuffs any data
into a CQ.
Called once the RS232 port is opened

NOTE:
The pic node will send asynchronous data (CAN Sync messages, keypad presses etc.)
These will constitute a NULL from the pic node received while we are not expecting
data. If this happens then we must send an RS232RequestSlave record and place this
library code into slave mode.
The pic node will transmit all the data it wishes, then send RS232RequestSlave to
indicate that it now wishes to be the slave. Any data received from the pic node
should be placed into a m_SlaveQ ready for the user to process.
Thus, we need a global flag to inform this function that we are looking for an echo.
If we are not looking and data arrives, and we are in binary mode, and it is a NULL
then we have pic node data awaiting us
**************************************************************************************/
wxThread::ExitCode GSISerialThread:: Entry()
{
#ifdef __WIN32__
	GSIChar recd[128]; //array shouldn't be required as we only request one byte at a time
	DWORD numReceived;
	ComPortHandle h=Device()->InQ->GetComPortHandle();
	m_quit=false;
		//errorType rv;
	//readingRecord=false;
	m_pause=false;

	do
	{
		while(m_pause)	//caller will set variable pause when it is reading/writing a record
									//thus any data read in binary mode *should* be an asynchronous send
									//from the pic node
		{
			quit=TestDestroy();
			if(m_quit) break;
            wxSafeYield(NULL, true);
		}		//platform independent pause. WIN32 can block in Readfile as doesn't use TestDestroy mechanism
		// Read the data from the serial port.
		ReadFile (h, recd, 1, &numReceived, 0);
		//TestDestroy will return 1 when we must exit
		//if GSIThread::Pause() is called then this call will not return till
		//the pause has ended.
		quit=TestDestroy();
		//_ASSERT(numReceived<2);
		if(numReceived)
		{
//Use a lock here to ensure that the record writing function cannot start while we are
//potentially processing a slave request from the pic node (a NULL outside of a record write/echo in binary mode)
//Need to re-write so that this thread writes/echoes records as well as handles all other i/o to pic node
//This way it can control master/slave modes. Also then allows user code to send us a record
//in a blocking function which doesn't return till complete (or error), or as a non-blocking call
//that returns immediately but which sets a flag on completion (or error).

		//wxCriticalSectionLocker locker(Device()->InQ()serviceThreadLock);

			switch(Device()->InQ->m_mode)
			{
			case GSI_MODE_ASCII:
				m_CurrentQ->CQueue::Write((char *)recd,numReceived); //write the ascii char(s) to the Q. Check ECHO On ???
				break;
			case GSI_MODE_BINARY:
				//These are different actions to be taken when in binary mode
				switch(Device()->InQ->m_binaryModeActionFlag)
				{
				//case GSIRS232Device::BIN_MODE_MASTER:							//master
				//	currentQ->Write(recd,numReceived);	//current is either MasterQ or SlaveQ depending on masterSlave flag
				//	break;
				case GSIRS232Device::BIN_MODE_MASTER:							//master
				case GSIRS232Device::BinModeSlave:
					{//slave
						errorType rv;
						//DLL_ASSERT(numReceived==1);									// >1 probably means a re-sync or power on reset]
						if(numReceived >1)
						{
							//rv=Device()->InQ()UnexpectedDataReceived();		//data is in currentQ
							DLL_ASSERT(1);
						}
						rv=AssembleAsyncRecord(buff[0]);
					}
					break;
				case GSIRS232Device::BinModeBlockRead:
				case GSIRS232Device::BinModeBlockWrite:
				case GSIRS232Device::BinModeUser1:
				case GSIRS232Device::BinModeUser2:
				case GSIRS232Device::BinModeUser3:
				default:
				//DLL_ASSERT(0);
				break;
				}//switch(binaryActionFlag)
				break;
			default:
				//DLL_ASSERT(0);
		break;
			}//switch m->Device()->InQ()m_mode
		}   //if numReceived

		// Display the data read.
		//if (dwBytesTransferred == 1)
		//        ProcessChar (Byte);

	} while (quit==false && m_quit==false);

#elif defined __LINUX__
#elif
#error Currently only tested on WIN32 and Linux
#endif
	//DLL_LOG_MESSAGE("\n\rEnding Serial thread");
	if(IsDetached() == false)

		Exit(0);		//joinable thread exit
	return(NULL);
}
#endif //DEBUG_USE_COMM_MASK

#endif

/**********************************************************************************
	errorType GSISerialThread::AssembleRxRecord(GSIChar chr)
***********************************************************************************/

errorType GSISerialThread::AssembleAsyncRecord(GSIChar chr)
{
    chr;
	//node is txing a record. We echo, after record is complete we look for an
	//ACK or REPEAT
	//rxRecord As BinRecord
//cant use static variables! Only one copy for ALL instances. Multiple serial ports
//receiving simultaneously will overwrite these varibles. Must use new operator in ctor
	//static GSIRecord record;
	//static int index,breakCount,cursor=0;		//initial value=0, which is default but make it explicit for readability
	//static GSIChar length,node,type;

    //static int index,breakCount,cursor=0;		//initial value=0, which is default but make it explicit for readability
	//static GSIChar length,node,type;
	errorType rv=errNone;

#if 0
	switch (m_asyncCursor)															//cursor defines the field we are currently in
	{
	case RS232RecordOffsetToTypeField:
				m_asyncIndex = 0;												//reset data index as this is the start of a record
				if(chr==0)												//the node is re-synching, or wants to be master, no record type of NULL
				{
					//send an rs232 message record to pic node with data[0]==GSISubfunction, data[1]==GSIRS232MessageReqSlave
					Device()->InQ()->SetMasterSlave(GSISlave);		//defaults to GSINodeThis, otherwise send second param with node
					return(errNone);								//exit. We are in Slave mode
				}

				rv=Device()->InQ()->Write(chr);						//echo the data
				if(rv)
					return(rv);
//SetType() reads binRecord.data[0] to establish the subfunction
//We haven't read the data yet so it is undefined.
//SetType then sets the type to 0, thinking it is an erroneous type
//However do all types have a subfunction??
//Defer the SetType till we have the complete record
				//record.SetType(chr);							//fill the record type field
                m_asyncType=chr;
				break;
	case RS232RecordOffsetToNodeField:
		Device()->InQ->Write(chr);									//echo the data
		m_AsyncRecord.SetNode(chr);
		break;

		case RS232RecordOffsetToLengthField:
			rv=Device()->InQ->Write(chr);	             //echo the data
			m_AsyncRecord.SetLength (chr);
			m_asyncLength = chr;
			if (m_asyncLength > GSIRecordMaxDataLength)
			{
				m_asyncLength = GSIRecordMaxDataLength;
				m_AsyncRecord.SetLength(GSIRecordMaxDataLength);
			}
			break;

		default:	//data
			//this test must be first for zero length data
			if (m_asyncIndex == m_asyncLength)
			{
				//this is an ACK,REPEAT,ERROR or FATAL
				switch (chr)
				{
				case GSI_REPEAT:													//not rt but enumHandshake
					rv=Device()->InQ->Write(chr);	             //echo the data
					//transmission error so retry
					m_asyncIndex = 0;
					m_asyncCursor = 0;
					return(errEcho);
				case GSI_ACK:
					//We appear to have rec'd the message ok, so despatch it
					//? Then send back an RS232Message success/failure
					//currently not implemented
					//rv = DespatchReceivedBinRecord(rxRecord)
					//DLL_LOG_MESSAGE("Received asynchronous record\n\r");
					if (rv==errNone)
					{

//of sending a REPEAT. Also ensures that when the node has sent a SlaveReq, this code is safely in Master mode
//otherwise the node could send us another SlaveReq, thinking that it is *already* slave!
                   #if DEBUG_DEFER_ACK
                   //see node.cpp and device.hpp, AckHelperClass()
                   #else
						Device()->InQ->Write(GSI_ACK);
                   #endif
						//xxx need to put back into master mode before the ACK??
						//Device()->InQ()m_asyncRecordReceived=true;

						m_AsyncRecord.SetFilledFlag(true);
                        m_AsyncRecord.SetType(m_asyncType);   //must be done after record complete as it uses r.data[0] to determine subfunction info.

//If the message Q stops working then we will fill up the heap with lots of records
//Alternative is to write to the CQ as we previously did and let the event handler
//retrieve from the Q. The Q is of finite size.
						m_AsyncQ->Write(m_AsyncRecord);

//This DefaultProcess should be performed in GSIFrame? Only the frame has access to the node array, so can
//despatch the records correctly.
//Alternatively, place the DefaultProcess in the node object. GSIFrame can extract the node it is intended for
//and despatch it to the correct node object.
//Thus, need a ProcessAsyncRecord in GSIFrame and a ProcessAsyncRecord in node objects

                        //Device()->InQ->DefaultProcessAsyncRecord(record);
                        //GSIDevice::Frame()->DefaultProcessAsyncRecord(record); //call the GSIFrame function

#warning ("work needed here")
//19/5/06
//Currently we write record to the Q and also instantiate a record and set a pointer to it in the custom event
//this duplication needs to be addressed. Must decide to do one or the other.

//Because the record data is ultimately sent through the socket interface as a simple array. We are probably
//best advised to store the incoming data in an array. Once the record has been successfully transferred from
//the node, simply write the array to the Q. The event handler can then pump this down the socket.
//Perhaps we should send to the socket from here? Probably not a good idea, best to centralise use of the socket
//in one location
#if 1
//NOTE. The OnProcessAsync function (in gsi.cpp) must use the correct node to handle these events
//we must send it with the event
//Shouldn't the event be on the heap? Example code creates a stack object, presumeably the event is cloned internally

						if(m_CustomEventHandler)		//if event handler set, use it
						{
							wxCommandEvent eventCustom(wxEVT_GSI_ASYNC_RECORD);
                            eventCustom.SetInt(Device()->InQ->Node()->GetNodeHandle());
							//Use Q rather than instantiate a record
                            //PostEvent clones the event. Nothing more to do.
							wxPostEvent(m_CustomEventHandler, eventCustom);
    					}
						else
						{
							wxFAIL_MSG("No GSI custom event handler specified");
                       #if DEBUG_DEFER_ACK
                            //need to ACK now as the handler has not been called!
                   			Device()->InQ->Write(GSI_ACK);
                       #endif

						}

#endif
						if (m_asyncBreakCount == 1)
						{
							m_asyncBreakCount = m_asyncBreakCount;
						}
					}
					//rv = GSIRS232SetPCRS232Master
					else
					{
						Device()->InQ->Write(GSI_ERROR);
						//'rv = GSIRS232SetPCRS232Master
					}
					m_asyncIndex = 0;
					m_asyncCursor = -1;
					m_asyncBreakCount++;
                    m_AsyncRecord.SetFilledFlag(false);
					break;
				case GSI_ERROR:
					rv=Device()->InQ->Write(chr);	             //echo the data
					//'abort the reception;
					m_asyncIndex = 0;
					m_asyncCursor = -1;
                    m_AsyncRecord.SetFilledFlag(false);
					break;
				case GSI_FATAL:
					rv=Device()->InQ->Write(chr);	             //echo the data
					//'abort the reception, probably need to  re-enter
					//'ascii mode
					//GSI.TxtRx.Text = GSI.TxtRx.Text + "AssembleCAN SYNC" + Constants.vbCrLf
					m_asyncIndex = 0;
					m_asyncCursor = 0;
                    m_AsyncRecord.SetFilledFlag(false);
					return(errFatal);
				default:
					break;
					//MsgBox "Assemble Rx Record error", vbExclamation
				} //switch
			}
			else
			{
				rv=Device()->InQ->Write(chr);	             //echo the data
				//record->.RecordData(index) = chr;
				m_AsyncRecord.SetData(m_asyncIndex,chr);
				m_asyncIndex++;
			}
	}//End Select

	m_asyncCursor++;
#endif
	return(rv);
	}

#if 0
/****************************************************************************
//   errorType GSISerialThread::Write(GSIRecord *record)

//Place this in Serial thread so that can write within thread, otherwise
//we will write,wait for m_ServiceThread timeslice, read.
//The write/echo cycle will take a time slice for each read+write
//this will limit transfer speed
//
//Send the record to the device, in this instance the serial port
//must be in binary mode. This is also an implicit echo

	From pic code. Must look for NULL outside of
Must only use this function for sending records as we set a "sendingRecord"
variable and reset it after transmission. The service thread uses
this variable to determine if a NULL arrives outside of a record, in binary mode
If so, then pic node has data to send us.
Should probably make this a class so that if we use exeptions and this routine
is aborted then the lock will be restored.
;-------------------------------------------------------------------------------
;	GSICANDataPendingTxToRS232
;Called fom main loop when SIO_S1_CAN_DATA_PENDING_TX is set
;
;Data is in the CnTxRecord structure
;If we are in Slave mode then we need to become temporary master
;Do this by sending a NULL down rs232
;The pc will +not+ respond with an echo
;it will send an rs232 message subfunction GSI_RS232_MESSAGE_REQ_SLAVE
;once it is free to do so.
;on receipt of this, this node becomes RS232Master
;If the pc is currently sending data to this node then it will
;ignore the NULL. In this case this function will repeatedly be
;called by the main program loop till the request has been met.
;If we need to use NULL as a general signalling mechanism
;then we can send the NULL then receive an RS232 mssg requesting
;the node inform the master what function it requires.
;
;If we are not in a binary mode then bcf the SIO_S1_CAN_DATA_PENDING_TX bit
;-------------------------------------------------------------------------
*********************************************************************/
errorType GSISerialThread::Write(GSIRecord &r)
{
  errorType rv=errNone;
  //PGSIBINRECORD recordEcho;

  if(Device()->InQ()GetMode() != GSI_MODE_BINARY)
		return(errBadMode);

	//GSILock MyLock(&Device()->InQ()readingRecord);			//will be made true, made false on exit from this function
  bool echoError=false,slaveRequested=false;
  int nullCount=0,i,repeatCount;
  wxStopWatch sw;
  GSIChar recd;


	//const    GSIBINRECORD &pr=r.GetBinRecord();//pointer to the record data

  //Write the record a byte at a time and check for an echo. If bad echo then
  //send a GSI_REPEAT rather than GSI_ACK when we find the ACK

  for(repeatCount=0;repeatCount<GSI_REPEATCount;repeatCount++)
	{
		echoError=false; //reset for next time through the loop
		rv=Device()->InQ()Write(r.GetType(),&recd,GSIEchoTimeout); //write/read recordType
		//DLL_LOG_MESSAGE("type %x:%x %i\n\r",pr.type,recd,rv);
		switch(rv)
		{
		case errNone:
			break;
		case errEcho:
			if(recd=='\0')
			{//if NULL then may be legitimate communication from node
				//If a single NULL then ignore it as it is the pic attempting
				//to send asych data. Our Tx'd bytes crossed. The pic will continue
				//in its main loop and will attempt to regain comms later.
				//If we find this asynch NULL from the pic then we must send
				//it a RequestSlave record. We become slave, it sends us the data
				//it sends a RequestSlave to become slave, we become master
				//Using this scheme the pc is effectively always the master.
				//It just needs to detect a NULL between records (while in binary
				//mode).
				//If more than one NULL then the pic is re-synching
#warning ("Error here! This code doesn't work properly when we receive a null!")
				rv=Device()->InQ()ReSynchNode(&recd,&nullCount);

				if(nullCount == 1) //function didn't find any more nulls in the timeout interval.
				{
					//If 1 null then Master req
					slaveRequested=true;
					DLL_LOG_MESSAGE("Pic requested master\n\r"); //do something!
					rv=errNone;
				}

				//continue processing, recd should hold pr->type
				if(recd != r.GetType())
					echoError=true;
			}
			else echoError=true;

			break;
		case errQueueEmpty:
			DLL_LOG_MESSAGE("Error:Q empty\n\r");
			break;
		case errTimeout:
		case errFail:
		default:
			return(rv);  //nasty error
		}

		if(echoError)
			DLL_LOG_MESSAGE("Type echo error\n\r");
		//write /read node
		rv=Device()->InQ()Write(r.GetNode(),&recd,GSIEchoTimeout);
		//DLL_LOG_MESSAGE(" 2\n\r");
		if (rv==errFail )
			return(rv); //write error is probably fatal
		if(rv==errTimeout)
			return(rv);
		if(rv==errEcho)
			echoError=true;
		if(echoError)
			DLL_LOG_MESSAGE("Node echo error\n\r");
		//write/read length

		rv=Device()->InQ()Write(r.GetLength(),&recd,GSIEchoTimeout);
		//DLL_LOG_MESSAGE(" 3\n\r");
		if (rv==errFail)
			return(rv); //write error is probably fatal
		if(rv==errTimeout)
			return(rv);
		if(rv==errEcho)
			echoError=true;
		if (echoError)
			DLL_LOG_MESSAGE("Length echo error\n\r");
		//DLL_LOG_MESSAGE(" 4\n\r");
		for(i=0;i<r.GetLength();i++)
		{
			rv=Device()->InQ()Write(r.GetData(i),&recd,GSITimeout);
			if (rv==errFail)
				return(rv); //write error is probably fatal
			if(rv==errTimeout)
				return(rv);
			if (rv==errEcho)
				echoError=true;
		}

#if 0
		if(repeatCount <2)
		{
			DLL_LOG_MESSAGE("Debug code setting a dummy echo error");
			echoError=true;
		}
#endif
		if(echoError)
			DLL_LOG_MESSAGE("Data echo error\n\r");

		//As the node doesn't know what was sent it can only echo what it received
		//so we must send an ACK or a REPEAT to the node once the record has been
		//sent. If we had an echo error we will send a repeat

		if(echoError)
		{
			rv=Device()->InQ()Write(GSI_REPEAT,&recd,GSIEchoTimeout);
			DLL_LOG_MESSAGE("Echo error, sent REPEAT\n\r");
			if(rv)
			{
				DLL_LOG_MESSAGE("Failed to write Repeat\n\r");
				return(errFail);
			}
			continue; //continue the for loop
		}
		else
		{
			rv=Device()->InQ()Write(GSI_ACK,&recd,GSIEchoTimeout);
			if(rv)
			{
				DLL_LOG_MESSAGE("Failed to write ACK err:%i\n\r",rv);
				return(errFail);//is this correct?? What does the pic do on a bad ACK echo?
			}
			else
			{
				GSIRecord r2Record;
				//DLL_LOG_MESSAGE("BinRecord written\n\r");
				//pic will now respond with an RS232Message Success/fail
				rv=Device()->InQ()Read(r2Record,GSIRecordReadTimeout);
				break; //Record sent successfully. Exit enclosing for loop

			}
		}//echoError

	}  //GSI_REPEAT loop

		//all done. If echoError is still set then we have a bad com line
		if(repeatCount==0 && echoError)
			return(errEcho);

		return(rv);
}
#endif   //#if 0

void GSISerialThread::OnExit()
{
	int i;
	i=1;
}





/*****************************************************************************
errorType GSIDevice::DefaultProcessAsyncRecord()
void parameter list. First read the record from the m_SlaveQ
******************************************************************************/
errorType GSIRS232Device::DefaultProcessAsyncRecord(GSIRecord &r)
{
	errorType rv;

	//GSIRecord r;
	//rv=m_AsyncQ->Read(r);
	//if(rv)
	//	return(rv);
	rv=GSIDevice::DefaultProcessAsyncRecord(r);
	return(rv);
}

#if 0
errorType GSIRS232Device::WaitForAsyncRecord(GSIRecord &record)		//overide the device class function
{
errorType rv=errNone;

	do
	{
	 //if(IsAsyncRecordReceived())
		if(m_AsyncQ->GetNumInBuffer())
		{
			break;
		}
	 else
		 //Sleep(1);
         //wxApp::wxYield(true);
         wxSafeYield(NULL,true);
	}while(1);
	record;
	return rv;
}
#endif


#if 0
#ifdef _DEBUG
//********************************************************************

//                GSIDebugRS232Device functions


//*********************************************************************
//                ctor
//********************************************************************
GSIDebugRS232Device::GSIDebugRS232Device(const wxString& device, long baud_rate,size_t qsize,wxTextCtrl *read_ctrl,wxTextCtrl *write_ctrl)
:GSIRS232Device(device,baud_rate,qsize),m_TbRead(read_ctrl),m_TbWrite(write_ctrl)
{
//#ifdef WIN32

	m_read=m_write=false;

	if (read_ctrl != NULL)
		m_read=true;

	if(write_ctrl)
		m_write=true;

	wxString s("GSIDebugRS232Device ctor");
	WriteTb(s,errNone);
}



//******************************************************************************

//             dtor

//****************************************************************
GSIDebugRS232Device::~GSIDebugRS232Device()
{
	wxString s("GSIDebugRS232Device dtor");
	WriteTb(s,errNone);
}



//*****************************************************************
//        errorType GSIDebugRS232Device::NewCQ(CQueue *new_q,CQueue *old_q)
//Place input into new CQ
//return current CQ to caller
//*****************************************************************

errorType GSIDebugRS232Device::NewInCQ(CQueue *new_q,CQueue *old_q)
{
		wxString s("NewInCq");
		errorType rv=GSIRS232Device::NewInCQ(new_q,old_q);

		WriteTb(s,rv);

  return(rv);
}

/**************************************************************************
	errorType GSIDebugRS232Device::Init()

  This doesn't work. The two funcions appear to need to be in user space
***************************************************************************/
errorType GSIDebugRS232Device::Init()
{
	errorType rv=GSIRS232Device::Init();

		wxString s("Init");
		WriteTb(s,rv);

		return(rv);
}

//************************************************

//    errorType GSIDebugRS232Device::SetMode(GSIEnum flag)

//***********************************************

errorType GSIDebugRS232Device::SetMode(GSIEnumMode flag)
{
	wxString s("Set mode to ");
	if(flag==GSI_MODE_BINARY)
		s=s+"Binary";
	else
		s=s+"Ascii";

	errorType rv=GSIRS232Device::SetMode(flag);

	Tb(s,rv);

  return(rv);
}



//****************************************************************
//           errorType Enable(GSIEnumEnable flag)
//From virtual class GSIDevice
//flag==Enable:
//Direct the data received from the serial device into this objects
//CQueue by overwriting the global CQeue * GlbCQueue

//Flag=Disable:
//Do not accept data to this objects CQueue. Overwrite GlbCQueue *
//with previous value
//Note that if the Previous CQueue object is NULL
//***************************************************************

errorType GSIDebugRS232Device::Enable()
{
	errorType rv;
	wxString s("Enable");

	rv=GSIRS232Device::Enable();


	WriteTb(s,rv);

	return(rv);
}

errorType GSIDebugRS232Device::Disable()
{
	wxString s("Disable");

	errorType rv=GSIRS232Device::Disable();

	WriteTb(s,rv);
	return(rv);
}



/********************************************************************
		errorType GSIDebugRS232Device::EchoOn()

**********************************************************************/
errorType GSIDebugRS232Device::EchoOn()
{
	errorType rv=GSIRS232Device::EchoOn();

	wxString s("Echo On");
	WriteTb(s,rv);

	return(rv);
}


/********************************************************************
		errorType GSIDebugRS232Device::EchoOff()

**********************************************************************/
errorType GSIDebugRS232Device::EchoOff()
{
	wxString s("Echo Off");
	errorType rv=GSIRS232Device::EchoOff();

	WriteTb(s,rv);
	return(rv);
}


//******************************************************************
//   errorType GSIDebugRS232Device::Write(GSIEnumRecord record)
//
//Send the enumerated record to the device, in this instance the serial port
//In Ascii mode this is not a full binary record, but a single byte record

//In binary mode it is a record
//NOTE
//In binary mode REPEAT,ACK,ERROR,FATAL are also single bytes
//*****************************************************************
errorType GSIDebugRS232Device::Write(GSIEnumRecord e_record)
{
	wxString s("Sent record: xxx function incomplete");
	errorType rv=GSIRS232Device::Write(e_record);

	WriteTb(s,rv);

  return(rv);
}

//******************************************************************
//   errorType GSIDebugRS232Device::Write(const GSIChar chr,GSIChar *recvd)
//
//Send the char to the device, in this instance the serial port
//Note that binary Write is always an implicit echo. The BinaryMode
//function will set the flag and restore to original when Ascii mode
//is set
//*****************************************************************
errorType GSIDebugRS232Device::Write(const GSIChar c,GSIChar *recvd)
{
	wxString s;
	errorType rv= GSIRS232Device::Write(c,recvd);

	s.Printf("Sent char %c, received char %c",c,*recvd);

	WriteTb(s,rv);

  return(rv);
}


/****************************************************************************
//   errorType GSIDebugRS232Device::Write(GSIRecord *record)
//
//Send the record to the device, in this instance the serial port
//must be in binary mode. This is also an implicit echo

	From pic code. Must look for NULL outside of
Must only use this function for sending records as we set a "sendingRecord"
variable and reset it after transmission. The service thread uses
this variable to determine if a NULL arrives outside of a record, in binary mode
If so, then pic node has data to send us.
Should probably make this a class so that if we use exeptions and this routine
is aborted then the lock will be restored.
;-------------------------------------------------------------------------------
;	GSICANDataPendingTxToRS232
;Called fom main loop when SIO_S1_CAN_DATA_PENDING_TX is set
;
;Data is in the CnTxRecord structure
;If we are in Slave mode then we need to become temporary master
;Do this by sending a NULL down rs232
;The pc will +not+ respond with an echo
;it will send an rs232 message subfunction GSI_RS232_MESSAGE_REQ_SLAVE
;once it is free to do so.
;on receipt of this, this node becomes RS232Master
;If the pc is currently sending data to this node then it will
;ignore the NULL. In this case this function will repeatedly be
;called by the main program loop till the request has been met.
;If we need to use NULL as a general signalling mechanism
;then we can send the NULL then receive an RS232 mssg requesting
;the node inform the master what function it requires.
;
;If we are not in a binary mode then bcf the SIO_S1_CAN_DATA_PENDING_TX bit
;-------------------------------------------------------------------------


Should probably pause the service thread, then write and read echo in this
function, otherwise we will be waiting on the service thread to run before
we see the echoed char and will thus slow our reception rate
***************************************************************************/
errorType GSIDebugRS232Device::Write(GSIRecord &r)
{
		errorType rv=GSIRS232Device::Write(r);
		WriteTb(r,rv);
		return(rv);
}


errorType GSIDebugRS232Device::Write(GSIChar chr)	//write a single byte without echo (handshaking etc.)
{
	//Note that this function is "dangerous" in that if the node is echoing, the char
	//will be in the input Q. It is the responsibility of the caller to handle any echo
	errorType rv=GSIRS232Device::Write(chr);
	wxString s("Warning! Write Char without echo");
	WriteTb(s,rv);
	return(errNone);
}


//******************************************************************
//   errorType GSIDebugRS232Device::Write(const GSIChar c,GSIChar *recvd,long timeout)
//
//Write char to device
//Read device to find echo of sent char
//return errTimeout if timeout mS exceeded

//Note that this function assumes that the service thread is paused
//*****************************************************************

errorType GSIDebugRS232Device::Write(const GSIChar c,GSIChar *recvd,long timeout)
{
	errorType rv=GSIRS232Device::Write(c,recvd,timeout);
	wxString s;
	s.Printf("Wrote char %i received char %i",(int)c,(int)*recvd);
	WriteTb(s,rv);
  return(errNone);
}



//***************************************************************
//    errorType GSIDebugRS232Device:: SetPCAsMaster()
//
//The node attached to this pc has requested that it be the slave
//It has sent an RS232_MESSAGE_REQUEST_SLAVE
//***************************************************************
errorType GSIDebugRS232Device:: SetPCAsMaster()
{

	wxString s("SetPCAsMaster");
	errorType rv=GSIRS232Device::SetPCAsMaster();
	WriteTb(s,rv);
  return(rv);
}



//*******************************************************************
//        errorType Read(GSIRecord *record,int fd long timeout);

//Read a record being sent from the node
//Note that this function assumes that the Service thread is paused
//******************************************************************

errorType GSIDebugRS232Device::Read(GSIRecord &record, long timeout)
{
	errorType rv=GSIRS232Device::Read(record,timeout);
	WriteTb(record,rv);
  return(rv);
}



/*********************************************************************
	errorType GSIDebugRS232Device::SetMasterSlave(GSIEnumMasterSlave mode)

The node attached to this pc has requested that it be the master or slave

The mode flage is relative to the PC, so GSIMaster==this PC is to become master
and the pic node has actually sent a GSIRS232MessageReqSlave message

If the pic has sent an GSIRS232MessageReqMaster message then this function
will be called with mode set to GSISlave, the PC must become a slave

If GSISlave
'Relinquish control
'Send GSI_RS232_MESSAGE_REQ_SLAVE to tell the node it is now master

'or we have requested that we be rs232 slave

'The node will send an rs232 message subfunction GSI_RS232_MESSAGE_REQ_SLAVE
'once it has finished its transmission and wishes the pc to be master
'the default condition

**********************************************************************/
errorType GSIDebugRS232Device::SetMasterSlave(GSIEnumMasterSlave mode,GSIChar pic_node)
{
	errorType rv=GSIRS232Device::SetMasterSlave(mode,pic_node);
	wxString s("SetMasterSlave");
	WriteTb(s,rv);
	return(rv);
}

/***************************************************************************
	GSIEnumMasterSlave GSIDebugRS232Device::GetMasterSlave(GSIEnumMasterSlave mode)
*****************************************************************************/
GSIEnumMasterSlave GSIDebugRS232Device::GetMasterSlave()
{
	errorType rv=errNone;
	wxString s("GetMasterSlave : we are ");

	GSIEnumMasterSlave flag=GSIRS232Device::GetMasterSlave();

	if(flag==GSIMaster)
		s.Append("Master");
	else
		s.Append("Slave");
	WriteTb(s,rv);
	return(flag);
}
/*****************************************************************************
errorType GSIDevice::DefaultProcessAsyncRecord()
void parameter list. First read the record from the m_SlaveQ
******************************************************************************/
errorType GSIDebugRS232Device::DefaultProcessAsyncRecord()
{
	errorType rv=GSIRS232Device::DefaultProcessAsyncRecord();

	return(rv);
}
/***************************************************************************************************
errorType GSIDebugRS232Device::WaitForAsyncRecord(GSIRecord &record)
***************************************************************************************************/
errorType GSIDebugRS232Device::WaitForAsyncRecord(GSIRecord &record)		//overide the device class function
{
errorType rv=errNone;

	rv=GSIRS232Device::WaitForAsyncRecord(record);
	return rv;
}


errorType GSIDebugRS232Device::WriteTb(wxString &str,errorType rv)
{
	wxString tstr;
	if(m_write)
		{
		if(rv)
		{
			tstr.Printf(": failed with error code %i \n",(int)rv);
			str=str+tstr;
		}
		else
		str=str+": success\n";

		m_TbWrite->AppendText(str);
	}
	else
		return(errFail);
	return(errNone);
}

/*******************************************************************
errorType GSIDebugRS232Device::WriteTb(GSIRecord &rec,errorType rv)
The record has been processed. Its return value is in rv.
Convert to a set of strings and send to TextCtrl
*******************************************************************/
errorType GSIDebugRS232Device::WriteTb(GSIRecord &rec,errorType rv)
{
	wxString s;
	if(rec.GetLength() >8)
		{
			int i=1;
			i=i;
		}
	if(m_write)
		{
			try
			{
				GSIStringRecord sr(rec);
				//wxString s,s1;
				m_TbWrite->AppendText("-Record-n");
				//s=sr.GetType();
				s=sr.GetType();
				m_TbWrite->AppendText(s);

				m_TbWrite->AppendText(sr.GetNode());
				if( ! sr.GetSubfunction().IsEmpty())
				{
					m_TbWrite->AppendText(sr.GetSubfunction());
					m_TbWrite->AppendText(s);
				}
				s=sr.GetData();
				if(s.Length() >36)
				{
						int i=0;
						i=i;
				}
				m_TbWrite->AppendText(sr.GetData());


				m_TbWrite->AppendText("-End Record-\n");
			}
			catch(...)
			{
			}
		}
	return(errNone);
}

errorType GSIDebugRS232Device::ReadTb(wxString &str,errorType rv)
{
	if(m_read)
		{
		}
	return(errNone);
}

errorType GSIDebugRS232Device::ReadTb(GSIRecord &rec,errorType rv)
{
	if(m_read)
		{
		}
	return(errNone);
}


bool GSIDebugRS232Device::GetEchoState() const
{
	wxString s;
	bool b=GSIRS232Device::GetEchoState();
	if(b)
		s="GetEchoState = echo";
	else
		s="GetEchoState = no echo";
	WriteTb(s,errNone);
	return(b);
}

GSIEnumMode GSIDebugRS232Device::GetMode()
{
	GSIEnumMode m=GSIRS232Device::GetMode();
	wxString s("GetMode= ");
	switch (m)
		{
	case GSI_MODE_ASCII:
		s.Append("ascii");
		break;
	case GSI_MODE_BINARY:
		s.Append("binary");
		break;
	case GSIModeBlock:
		s.Append("block");
		break;
	case GSIModeUser1:
		s.Append("user 1");
		break;
	case GSIModeUser2:
		s.Append("user 2");
		break;
	case GSIModeUser3:
		s.Append("user 3");
		break;
		}
	WriteTb(s,errNone);
	return(m);
}

//#endif //_DEBUG
//************************************************************************

//         end of GSIDebugRS232Device

//***********************************************************************


#endif //#if 0



#endif



/********************************************************************
//   errorType GSIRS232Device::ReSynchNode(GSIChar *recd)
//
//A null was recd as the Type field of a record. This can be a
//request by the pic to be Master or attempt by the pic to re-synch
//after an error.
//if it is a re-synch it will send a stream of nulls
//if it is a Master req it should echo the Type field we have just sent

//IMPORTANT NOTE.
Assumes the SerialThread is paused
;--------------------------------------------------------
;	RS232SendReSyncNulls
;Modifies R2BTemp
;if comms problem we will send R2_MAX_RECORD_LEN*2+1 nulls
;so that pc can re-synchronise to us.
;It should ensure that at least one null record is read
;by the pc which it recognises as a re-sync
;Should also do whatever is required to place rs232 into
;a known state.
;Perhaps should do an init??
;--------------------------------------------------------

RS232SendReSyncNulls
	banksel	R2BTemp
	call	GSIInitRS232		;init everything

	movlw	(R2_MAX_RECORD_LEN*2)+1
	movwf	R2BTemp
	clrf	WREG
rsrsn:
	call	GSISendByte232
	decf	R2BTemp,F
	bnz	rsrsn

	movlw	ACK		;send an ACK when we are through
	call	GSISendByte232
rsrsn1:
	call	GSIGetFromRS232RxBuff	;wait for an echo
	bz	rsrsn1		;wdt on error
	clrwdt

	call	GSIFlushR2RxBuffer
	return
*******************************************************************/
errorType GSIRS232Device::ReSynchNode(GSIChar *recd, int *null_count)
{
wxStopWatch sw;
errorType rv;

    do
	{
	    rv=ReadUnbuffered(recd,GSITimeout);    //can't use this, must use direct read of the device, the serial thread is paused
		if(rv==errNone)
		{
			//have recd a byte
			if(*recd == '\0')
			{
				*null_count++;
				sw.Start(0);
				continue; //restart the loop, looking for another null
			}
            //Here if a non-null was received, if nullcount>1 then this is either a resync or an error
			if(*recd==GSI_ACK)       //if a resync the char should be a GSI_ACK
			{
				rv=Write(GSI_ACK,recd,GSIEchoTimeout);		//pic node will wait for this
				if(rv)	//echoError or timeout are both fatal
					return(rv);

                return(errGSIReSync);        //hopefully a recovery from a resynch
			}
            if(*null_count >1)
			    return(errFail);
            else
                return(errGSISlaveReq);        // a single null, node wants to be master

		}
	}while(sw.Time() < GSIEchoTimeout); //only pause for 1mSec between receipt of nulls


	return(errTimeout);
}


void GSISerialThread::QuitThread()
{
    m_quit=true;
    quit=true;
}



errorType GSIRS232Device::Open()
{
errorType rv=errNone;

    return rv;
}

errorType GSIRS232Device::Close()
{
errorType rv=errNone;
    CloseComPort();
    return rv;
}


SerialPortDevice::SerialPortDevice(const wxString &device_name,GSIHandle handle,GSIFrame *gsi_frame)
:GSIDevice(device_name,handle,gsi_frame)
{
}

SerialPortDevice::~SerialPortDevice()
{
    //Close();

}
