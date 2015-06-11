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
#if defined(__GNUG__) && !defined(__APPLE__)
#  pragma implementation "cq.cpp"
#  pragma interface "cq.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"


#ifdef __BORLANDC__
#  pragma hdrstop
#endif

// for all others, include the necessary headers


#ifdef __VISUALC__
#pragma warning(push, 1)
#endif

#include <windows.h>

//std m_headers in here for warning level 1

#ifdef __VISUALC__
#pragma warning(pop)
#pragma hdrstop
#endif

#include <wx/utils.h>       //defines wxSleep()
#include "wx/stopwatch.h"

#include <wx/thread.h>
//#include "gsi.hpp"
#include <gscq.hpp>

#if __VISUALC__
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
#else
#define DEBUG_NEW new
#endif
#else
#define DEBUG_NEW new
#endif

#if __GNUC__
#define DEBUG_NEW new
#endif

//helper function

//static char RecordToBuffer(GSIRecord &record,char *buff);       //moved to record.hpp, now part of GSIRecord class

//***************************************************

//           CQueue functions

//**************************************************

//********************************************
//      CQueue ctor
//*******************************************

CQueue::CQueue(long size)
{
    wxASSERT(size >0);
    m_buffer=DEBUG_NEW char[size];
    m_head=m_tail=m_buffer;
    m_lastBuffPos=m_buffer+size-1;
    m_maxBuffSize=size;
    m_numInBuff=0;
}



//********************************************
//      CQueue dtor
//*******************************************
CQueue::~CQueue()
{
    delete m_buffer;
}

//********************************************
//     CQueue Write(char chr)
//Note:
//This is called from a service thread
//*******************************************
errorType CQueue::Write(char chr)
{
    if(m_numInBuff+1 > m_maxBuffSize)  //room for char?
    {
        return(errQueueOverflow);
    }
    *m_head++ = chr;  //store the char
    if (m_head > m_lastBuffPos)
    {
        m_head=m_buffer;  //overflow, so wrap to front of buff
    }
    wxCriticalSectionLocker locker(m_GSIThreadCritSect);
    m_numInBuff++;		//if this is not atomic then the m_numInBuff might report incorrectly
    return(errNone);
}
//********************************************
//     CQueue Write(char chr, long num)
//*******************************************
errorType CQueue::Write(char *buff,long num)
{
    int i;
    if(m_numInBuff+num > m_maxBuffSize)  //room for char?
    {
        return(errQueueOverflow);
    }
    wxCriticalSectionLocker locker(m_GSIThreadCritSect);
    for(i=0; i<num; i++)
    {
        *m_head++ = buff[i];  //store the char
        if (m_head > m_lastBuffPos)
        {
            m_head=m_buffer;  //overflow, so wrap to front of buff
        }
    }

    m_numInBuff+=num;
    return(errNone);
}

//********************************************
//     CQueue Write(wxString)
//*******************************************
errorType CQueue::Write(wxString str)
{
    //Write strlen chars
    wxCriticalSectionLocker locker(m_GSIThreadCritSect);
    return(errNone);
}

#if 0
//********************************************
//     CQueue Write(GSIRecord)
//*******************************************
errorType CQueue::Write(GSIRecord &record)
{

    int i;
    char buff[GSIRecordMaxLength],num;

    if(m_numInBuff+GSIRecordFixedLength+record.GetLength() > m_maxBuffSize)  //room for chars?
    {
        return(errQueueOverflow);
    }

    num=record.RecordToBuffer(buff);		//returns total length of buffer

    if(num==0)													//returns overall length. 0 if a bounds error
        return(errBounds);								//length is too great



    for(i=0; i<num; i++)
    {
        *m_head++ = buff[i];  //store the char
        if (m_head > m_lastBuffPos)
        {
            m_head=m_buffer;  //overflow, so wrap to front of buff
        }
    }

    wxCriticalSectionLocker locker(m_GSIThreadCritSect);
    m_numInBuff+=num;

    return(errNone);
}

#endif

//********************************************
//     CQueue Read(char)
//*******************************************

errorType CQueue::Read(char *chr)
{
    if(m_numInBuff == 0)
        return(errQueueEmpty);

    wxCriticalSectionLocker locker(m_GSIThreadCritSect);

    *chr= *(m_tail) ++;
    if (m_tail > m_lastBuffPos)
        m_tail=m_buffer;
    m_numInBuff--;
    return(errNone);
}

//*****************************************************
//     CQueue Read(char *c, long timeout)
//As above read but keep checking the Q for the char
//for timeout mS
//******************************************************

errorType CQueue::Read(char *chr,long timeout)
{
    wxStopWatch sw;

    if(m_numInBuff==0)
    {
        do
        {
            //wait for the service thread to place a char in buff
            //wxSafeYield();    yield and sleep both cause a crash!
            wxSleep(0);
        }
        while( (sw.Time()<timeout) && (m_numInBuff==0) );  //quit if timeout
        if(m_numInBuff==0)
            return(errTimeout);
    }
    wxCriticalSectionLocker locker(m_GSIThreadCritSect);


    wxASSERT(m_numInBuff > 0);

    *chr= *(m_tail) ++;
    if (m_tail > m_lastBuffPos)
        m_tail=m_buffer;
    m_numInBuff--;			//this should be the only variable requiring criticalSection protection

    return(errNone);
}

//*****************************************************
//     CQueue Read(char *c, size_t num,long timeout)
//
//Read n chars
//If timeout then return the chars currently present
//for timeout mS
//******************************************************

errorType CQueue::Read(char *chr,size_t max_num,size_t *num_read,long timeout)
{
    wxStopWatch sw;
    errorType rv=errNone;
    size_t i=0;

    do
    {
        if(Read(&chr[i])==errNone)
            i++;		//char has been read
    }
    while(sw.Time()<timeout && i<max_num);          //only read up to max_num

    *num_read=i;
    if(i<max_num)
        rv=errTimeout;

    return(rv);
}



//********************************************
//     CQueue Read (wxString)
//*******************************************
errorType CQueue::Read(wxString  &str,size_t num)
{
    size_t i;
    //read strlen chars to string
    //std::cout << "&wx \r\n";
    wxCriticalSectionLocker locker(m_GSIThreadCritSect);

    for(i=0; i<num; i++)
    {
        if(m_numInBuff == 0)
            return(errQueueEmpty);
        //std::cout << *(m_tail);

        str += (wxChar) *(m_tail) ++; //fill the wxString
        //std::cout <<"!" << *str << "! "  ;

        if (m_tail > m_lastBuffPos)
            m_tail=m_buffer;
        m_numInBuff--;
    }
    return(errNone);
}

#if 0
//********************************************
//     CQueue Read(GSIRecord)
//*******************************************
errorType CQueue::Read(GSIRecord &record)
{


    char l;
    char * oldm_tail;
    size_t i;
    char type,node;

    if(m_numInBuff==0)
        return(errQueueEmpty);

    if(m_numInBuff<GSIRecordFixedLength)
        return(errBounds);

    oldm_tail=m_tail;

    //record.SetType(*(m_tail) ++);
    type=(*(m_tail) ++);
    if (m_tail > m_lastBuffPos)
        m_tail=m_buffer;

    //record.SetNode(*m_tail++);
    node=(*m_tail++);
    if (m_tail > m_lastBuffPos)
        m_tail=m_buffer;

    l=*(m_tail) ++;
    if (m_tail > m_lastBuffPos)
        m_tail=m_buffer;
    //record.SetLength(l);


    wxASSERT(l <= GSIRecordMaxDataLength);

    if(m_numInBuff < (size_t) (l+GSIRecordFixedLength))
    {
        m_tail=oldm_tail;		//reset m_tail to start
        return(errBounds);
    }

    for(i=0; i<l; i++)
    {
        record.SetData(i,*m_tail++);  //store the char
        if (m_tail > m_lastBuffPos)
            m_tail=m_buffer;
    }
    record.SetLength(l);
    record.SetNode(node);
    record.SetType(type);       //call this function last, it requires a length to be set

    wxCriticalSectionLocker locker(m_GSIThreadCritSect);
    m_numInBuff-=(l+GSIRecordFixedLength);

    return(errNone);
}
#endif
/**************************************************************************
	void CQueue::Flush(void)
***************************************************************************/
void CQueue::Flush(void)
{
    wxCriticalSectionLocker locker(m_GSIThreadCritSect);

    m_head=m_tail=m_buffer;
    m_numInBuff=0;
}



#if 0

Moved to record.cpp
/***************************************************************
Helper function to convert record to buffer
****************************************************************/


static char RecordToBuffer(GSIRecord &record,char *buff)
{
    if(record.GetLength() >GSIRecordMaxDataLength)

        return(0);				//error value for this function, should always be >0


    buff[RS232RecordOffsetToTypeField]=record.GetType();
    buff[RS232RecordOffsetToNodeField]=record.GetNode();
    buff[RS232RecordOffsetToLengthField]=record.GetLength();
    for(char i=RS232RecordOffsetToDataField; i< RS232RecordOffsetToDataField+record.GetLength(); i++)
    {
        buff[i]=record.GetData(i-RS232RecordOffsetToDataField);
    }
    return((char) (record.GetLength()+GSIRecordFixedLength));
}
#endif
