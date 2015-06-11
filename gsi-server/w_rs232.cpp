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

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif


#if !wxUSE_THREADS
    #error "This file requires thread support!"
#endif // wxUSE_THREADS

#include <wx/thread.h>



#ifdef __VISUALC__
#pragma warning(push, 3)
#endif


//#include<signal.h>
//#include <errno.h>

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



//#include "gsi.hpp"
#include "include/rs232.hpp"
#include "include/gsi.hpp"
#include "include/GSICq.hpp"
#include "include/socket.hpp"
#include "include/node.hpp"
#include "include/microcontroller.h"
//#include <gsSerial.hpp>

//#include "socket.hpp"

//#define BAUDRATE B115200


#ifdef WIN32

/************************************************************************
        ctor
GSISerialPort(const wxString& device, gsBaud baud_rate)
*************************************************************************/
GSISerialPort::GSISerialPort(const wxString& device, gsBaud baud_rate,GSIRS232Device *r2_device,int rx_buffer_size)
:gsSerialPort(device,baud_rate,rx_buffer_size)
{

    //wxCriticalSectionLocker Lock(m_waitingForEchoLock);
    m_RS232Device=r2_device;

#if 0
This code simply holds the Tx line high for an interval, then drops it again. High meaning +v voltage, which
is actually a space using rs232 mark-space conventions (?)

    char c='a';
    WritePort(&c,1);
    SetCommBreak(GetHandle());
    Sleep(0);
    ClearCommBreak(GetHandle());
    Sleep(0);
#endif
}

/************************************************************************
        dtor
~GSISerialPort(const wxString& device, gsBaud baud_rate)
*************************************************************************/

GSISerialPort::~GSISerialPort()
{

}

/****************************************************************************
void  GSISerialPort::ComEvent(gsComEvent ev)

The gsSerialPort has detected received data or a BREAK (see mask value)
This code is called from the service thread,
*****************************************************************************/
void  GSISerialPort::ComEvent(gsComEvent ev)
{
    //gsSerialPort::ComEvent(ev);             //send it on for now
    switch(ev)
    {
    case gsSerialRxChar:
        int length;
        do
        {
#warning ("Really should access this through a member function")
            if(ReadPort(m_rxBuff,GS_SERIAL_MAX_BUFFER_SIZE,&length))
            {
                //an error
            }
            else
            if (length)
            {
                //char(s) received
                if(m_RS232Device->GetMode() == GSI_MODE_ASCII)
                    InQ()->Write((char *)m_rxBuff,length);
                else
                {   //binary mode char has arrived
                    //if(m_RS232Device->GetMasterSlave() == GSIMaster)
                    if(m_RS232Device->IsTxMaster())
                    {
                        //Can't use Locker as we don't want to block while waiting to own the resource
                        //can see two bytes (as long as 1st is ACK, 2nd should be 59, the start of an RS232Message
                        wxASSERT_MSG(length <3,"More than two bytes in RS232 buffer");

                        //Can't use Locker as we don't want to block while waiting to own the resource
                        //wxMutexLocker mx(m_WritingRecordMutex);
#warning ("This 'if' block to be removed once known to be never taken (8/1/07)")
                        wxMutexError mv;
                        wxMutex &CommsMutex=m_RS232Device->GetCommsMutex(); //

                        mv=CommsMutex.TryLock();    //mv=m_TxMutex.TryLock();
                        if (mv == wxMUTEX_NO_ERROR)
                        //if(mx.IsOk())
                        {//we have the lock, so this should be an async byte
                            errorType rv;
                            wxFAIL_MSG("Shouldn't arrive here anymore!");   //added 5/1/07
                            rv=m_RS232Device->AssembleAsyncRecord(*m_rxBuff);       //will return error of SlaveReq if null in Type field
                            if(rv == errGSISlaveReq)
                            {
                                //m_RS232Device->SetMasterSlave(GSISlave);
                                m_RS232Device->SetMaster();
                                mv=CommsMutex.Unlock();//mv=m_TxMutex.Unlock();
                            }
                        }
                        else if (mv==wxMUTEX_BUSY)
                        {//lock is owned elsewhere so send on the char (should be a char!)
                            wxASSERT_MSG(length <3,"More than one char in rs232 buffer");
                            InQ()->Write((char *)m_rxBuff,length);
                        }
                        else
                            wxFAIL_MSG("mutex error");
                    }
                    else    //if(m_RS232Device->GetMasterSlave() == GSIMaster)
                    {       //added 5/1/07
                            //byte(s) arrived in slave mode
#if STARTLE_SERVER
						//Startle box sends back acquisition data so don't want this assertion to fire
						//however would like to keep it for debugging.
						//longer term must look at supporting GSI_BLOCK_MODE
#else
						wxASSERT_MSG(length <3,"More than one char in rs232 buffer");
#endif
						InQ()->Write((char *)m_rxBuff,length);
                    }
                }
            }
        }while ( length > 0 ) ;
        break;
    case gsSerialBreak:
    {
/*
Break received. Pic will send a code informing us of the type of break.
If this code has just sent the first byte of a record then we could receive two bytes.
An echo of the byte just sent and the Break code.
We should try to lock the mutex. If we suceed then we have a simple break code following
If we fail then we have just started a Tx and should ignore this break. The pic will do likewise
and issue another break after handling the record
As this code is effectively an ISR, we must set a flag, LookingForBreak, and use the next call to this handler
to see what type of break was requested.

When pic want to send a break. It will first check that no Rx is occuring. If it deems it safe it will
#send break
#loop, waiting for BreakAck
#if it receives anything other than a BreakAck, then it means that the pc has just started sending
 a record. pic must abort the break, leaving the char in the queue for the binary processing code to retrieve.
 ??need a CQPeek function??
#if BreakAck received. Pc has a lock on the serialTx. We can send our Break type etc. If we send it a SlaveReq
 it will become slave, waiting for our record(s) to be sent.
*/
        errorType rv=BreakHandler();
		if(rv)
		{
			wxFAIL_MSG("Do something with this error!");
		}
        break;
    }
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

/***************************************************************************************************************
    errorType  GSISerialPort::BreakHandler()

microcontroller has sent a break (Tx held high for > 1 Char Tx time)
Don't think it is possible to receive a double break? (A break within this handler), it will be detected once
this code returns.
1) mc wants to send us data
2) mc has wdt or error reset
3) mc has reset while txing a record


mc will also send a BreakType (if in binary mode)
If the mc has reset then we must reset as well

When the break is received, the comport will also detect a NULL char. This is probably because a break is really
a framing error with data=0.
We should discard the NULL using direct file read

Note: Also have the condition where the pic sends a BREAK at the same instant that we begin sending a record.
In this case the pic is waiting in a loop looking for the ACK. When it receives a char other than the ACK it
will abort the break sequence and try again later. The char it receives will never be an ACK as it will be a
record Type byte
For this function, we will receive a break and NULL. As we are currenlty Txing we now have to decide if this is
a RESET or a cross-over error. If a cross-over we can ignore it.
For this to work reliably the mc needs to send a BREAK immediately followed by a BreakType byte. Thus, we detect
the break and its type, then can either send an ACK to tell the mc to process as usual. This could also mean aborting
any Tx we are doing if the mc has reset. If the mc is requesting that it be made master BUT we are currently Txing,
we simply ignore the request, send nothing from this function. The Write(GSIRecord) function will send its data and
the mc will fail to find an ACK, and will send a break later, after it has finished processing

Note that we can receive a 2nd Break condition for the 1 reset if the rs232 line stays low for too long
Might need a "ProcessingBreak" variable in this class so that we can ignore multiple breaks.
The Event handler can reset this, indeed won't need to as it will close this port. Perhaps we should disable
further Breaks by setting the CommMask?
*****************************************************************************************************************/

#warning ("********************* Need to disable BREAK?? See above comments*****************************")
#warning ("Test by sending a long break??")

errorType  GSISerialPort::BreakHandler()
{
//#warning ("?Need to use a hardware reset to mc, attach to INT0 on pic") //put the code in the initialization of the device
//#warning ("Use a break condition for the mc,pic can detect as frame error with data==0")
    //#error here
GSIRS232Device *Dev=m_RS232Device;
GSIChar recvd;
int numRead;
errorType rv=errNone;
wxStopWatch sw;
#warning ("Does this need to be a critical section? Will fail if we wxYield() after posting msg")
    //xp driver will also read a null associated with the break. A break is a NULL with a framing error
    //indeed, more than one NULL can be read, so read them!
    rv=ReadPort((char *) &recvd,1,&numRead,GSIEchoTimeout);           //use a direct read to read the associated null, ignore the CQ

    do         //for(i=0;i<20;i++)
    {
        rv=ReadPort((char *) &recvd,1,&numRead,GSIEchoTimeout*100);           //use a direct read to get the reset type
        //if(numRead==0 || rv != errNone) break;                                                 //exit the loop
        //if(recvd != 0)
        //{
        //     break;          //remove at least 10 nulls in case the reset takes a long time, so more NULLs appear??
        //}
    } while(rv == errNone && recvd==0); //keep reading till either a timeout or a non NULL is received

    if(rv==errTimeout)
            recvd=GSI_BREAK_RESET_FATAL;

    if(Dev->GetMode()==GSI_MODE_BINARY)
    {
        if(Dev->IsTxSlave())            //the mc has probably wdt'd while sending a record
        {

        }

        switch(recvd)
        {
//No special handling of reset types, leave this switch here just in case.
        case GSI_BREAK_RESET_WDT:              //Watch Dog Timer
        case GSI_BREAK_RESET_BOR:              //Brown Out
        case GSI_BREAK_RESET_UNKNOWN:          //pic chips (18F458 at least!) running under ICD2 generate this
        case GSI_BREAK_RESET_USER_ABORT:        //User abort, shouldn't see this in this version
        case GSI_BREAK_RESET_POR:              //Power On
        case GSI_BREAK_RESET_STACK_OVF:              //Stack overflow
        case GSI_BREAK_RESET_STACK_UNF:              //Stack underflow
        case GSI_BREAK_RESET_FATAL:
        //suspend the service thread so we don't get re-entrant breaks. Happens with Stack overflow with the pic 18f458.
        //Sensible thing to do anyway since this GSIDevice object is about to be shut down. Code can still read and write the port
        //directly using ReadPort and WritePort
             bool dataReceived;
             PauseRx(&dataReceived);         //This is a 'safe' pause it will block in WaitCommEvent()
             break;
        case GSI_BREAK_SEND_RECORD:            //mc wants to send us a record, we become master immediately after receipt
        case GSI_BREAK_SEND_MULTIPLE_RECORDS:   //mc wants to send more than one record. keep receiving till an RS232Mssg:ReqSlave is found
        case GSI_BREAK_SEND_DATA:              //mc want to send a chunk of data ?? 4bytes numBytesToSend,bytes
            //don't pause if this is not a reset!
/*
If we cannot obtain a lock, then we are currently Txing
*/
            //Dev->Write(GSI_BREAK_SEND_RECORD);
            break;
        default:
            //On a reset, the mc will send its sign-on message. After a POR it is totally oblivious to any pc connection
            //that might exist.
            //Thus this routine could see the 'Unknown Reset' message or any of the other reset message strings. The mc
            //assuming it is starting up in default ascii mode
            //wxFAIL_MSG("Unknown Break condition received (GSISerialPort::BreakHandler)" );
            //must perform some form of reset here as this will fail
            break;

       } //switch(recvd)

       if(Dev->GetCustomEventHandler())		//if event handler set, use it
       {

            //If a RESET the comport has been stopped, it will wait in ComEvent(), com still enabled otherwise
	        wxCommandEvent eventCustom(wxEVT_GSI_MC_BREAK);
            eventCustom.SetClientData(Dev);
            eventCustom.SetInt(recvd);              //also send back the reset type received from the mc
            wxPostEvent(Dev->GetCustomEventHandler(), eventCustom);
            //wxYield();		xxx removed 10/4/07
        }
        else
        {
            Dev->SetMode(GSI_MODE_ASCII); //best we can do without an event handler!
	        wxFAIL_MSG("No GSI custom event handler specified");
        }


    }  //if(GetMode())
    else
    {       //In ascii mode. Almost certainly a wdt error
            //can probably ignore it as we will simply ignore any bytes received??
            //When we change to binary mode the CQ will be flushed in any case

    }


#if 0
    GSIEnumMode mode=m_RS232Device->GetMode();
    if(mode==GSI_MODE_ASCII)                      //in ascii mode this should be a wdt, or the microcontroller is in binary mode!
    {
        //This shouldn't happen. If microcontroller wdt's it should know it was in ascii mode and not send a break
        //alternatively, can handle as a standard break, this code will reset itself. This should be benign
        return errBadMode;
    }

    else
    {
        //If we can obtain the Tx mutex then we are currently not txing a record or data and we are able to give the mc control
        //if we already have the lock then the mc has sent a break while txing data to us

        //if(m_TxMutex.TryLock() == wxMUTEX_NO_ERROR) //tryLock returns immediately if it can't obtain the mutex, Lock() will wait for the lock
        if(CommsMutex.TryLock() == wxMUTEX_NO_ERROR) //tryLock returns immediately if it can't obtain the mutex, Lock() will wait for the lock
        {
        //we have the lock, this is a no-complications break from the pic
         //Send a  BreakAck to the pic to acknowledge the break
         //if the pic has wdt/stack reset etc. It will,what? Send 2 max record length nulls?
         //if the pic has a failure during record Tx, it will wdt, then send 2 * max record length nulls
         //this will ensure that the record acquisition code aborts (or does it, might need explicit check for NULL
         //as Type)
         //If we don't do this then this BREAK handler will not obtain the mutex, will not see the break!
            char c=GSI_ACK;
            WritePort(&c,1);

        }
        else
        {
            //no lock obtained
            //1)another part of this code has (or is about to) send a record
            //Send a BreakFail. Pic can check Type field. If it is BreakFail, it can safely ignore it.
            //Next char sent should be a Type field. If Pic has a problem it will have sent a series of nulls
            //before the BREAK to ensure we have flushed any partial record from the

            //2)The mc has reset while txing a record to us, send a GSI_ACK, restart this program
        }
    }
#endif
    return rv;
}

/**********************************************************************************
	errorType GSIRS232Device::AssembleRxRecord(GSIChar chr)
***********************************************************************************/
#warning ("Might be best to place this in GSIDevice, InQ needs to be there too??")
errorType GSIRS232Device::AssembleAsyncRecord(GSIChar chr)
{

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


	switch (m_asyncCursor)															//cursor defines the field we are currently in
	{
	case RS232RecordOffsetToTypeField:
				m_asyncIndex = 0;												//reset data index as this is the start of a record
				if(chr==0)												//the node is re-synching, or wants to be master, no record type of NULL
				{
					//send an rs232 message record to pic node with data[0]==GSISubfunction, data[1]==GSIRS232MessageReqSlave
					//Device()->InQ()->SetMasterSlave(GSISlave);		//defaults to GSINodeThis, otherwise send second param with node
					return(errGSISlaveReq);								//Tell caller pic wants us to be Slave
				}

				rv=Write(chr);						//echo the data
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
		Device()->Write(chr);									//echo the data
		m_AsyncRecord.SetNode(chr);
		break;

		case RS232RecordOffsetToLengthField:
			rv=Device()->Write(chr);	             //echo the data
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
					rv=Device()->Write(chr);	             //echo the data
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
						Device()->Write(GSI_ACK);
                   #endif
						//xxx need to put back into master mode before the ACK??
						//Device()->InQ()m_asyncRecordReceived=true;

						m_AsyncRecord.SetFilledFlag(true);
                        m_AsyncRecord.SetType(m_asyncType);   //must be done after record complete as it uses r.data[0] to determine subfunction info.

//If the message Q stops working then we will fill up the heap with lots of records
//Alternative is to write to the CQ as we previously did and let the event handler
//retrieve from the Q. The Q is of finite size.
                        //m_AsyncRecord->m_AsyncQ->Write(m_AsyncRecord);
						Device()->AsyncQ()->Write(m_AsyncRecord);       //Added 8/1/07

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

						//if(m_CustomEventHandler)		//if event handler set, use it
                        if(GetCustomEventHandler())		//if event handler set, use it
						{
							wxCommandEvent eventCustom(wxEVT_GSI_ASYNC_RECORD);
                            eventCustom.SetInt(Node()->GetNodeHandle());
							//Use Q rather than instantiate a record
                            //PostEvent clones the event. Nothing more to do.
							//wxPostEvent(m_CustomEventHandler, eventCustom);
                            wxPostEvent(GetCustomEventHandler(), eventCustom);
    					}
						else
						{
							wxFAIL_MSG("No GSI custom event handler specified");
                       #if DEBUG_DEFER_ACK
                            //need to ACK now as the handler has not been called!
                   			Device()->Write(GSI_ACK);
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
						Device()->Write(GSI_ERROR);
						//'rv = GSIRS232SetPCRS232Master
					}
					m_asyncIndex = 0;
					m_asyncCursor = -1;
					m_asyncBreakCount++;
                    m_AsyncRecord.SetFilledFlag(false);
					break;
				case GSI_ERROR:
					rv=Device()->Write(chr);	             //echo the data
					//'abort the reception;
					m_asyncIndex = 0;
					m_asyncCursor = -1;
                    m_AsyncRecord.SetFilledFlag(false);
					break;
				case GSI_FATAL:
					rv=Device()->Write(chr);	             //echo the data
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
				rv=Device()->Write(chr);	             //echo the data
				//record->.RecordData(index) = chr;
				m_AsyncRecord.SetData(m_asyncIndex,chr);
				m_asyncIndex++;
			}
	}//End Select

	m_asyncCursor++;
	return(rv);
	}



/**********************************************************************************

errorType  GSIRS232Device::Open(const wxString& device, gsBaud baud_rate)

***********************************************************************************/
errorType  GSIRS232Device::Open(const wxString& device, gsBaud baud_rate,int rx_buffer_size)          //platform specific, see w_rs232.cpp, l_rs232.cpp
{
wxString message;
errorType rv=errNone;
    try
    {
        m_ComDev=DEBUG_NEW GSISerialPort(device,baud_rate,this,rx_buffer_size);               //an expicit open
    }

    catch(...)
    {
        rv=errFail;
    }

    return rv;

}

/************************************************************
    errorType GSIRS232Device::CloseComPort()
************************************************************/
errorType GSIRS232Device::CloseComPort()
{
    //m_Dev->Close();
    if (m_ComDev)
        delete m_ComDev;
    m_ComDev=NULL;
    return errNone;
}

#endif  //WIN32

