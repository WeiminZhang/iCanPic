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
#  pragma implementation "GSIClientCommands.cpp"
#  pragma interface "GSIClientCommands.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
//#include "wx/wxprec.h"
#include "wx/wx.h"

#ifdef __BORLANDC__
#  pragma hdrstop
#endif



#ifdef __VISUALC__
 #pragma warning(push, 1)
#endif

//std m_headers in here for warning level 1

#ifdef __VISUALC__
#pragma warning(pop)
#pragma hdrstop
#endif


#include "wx/stopwatch.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

//#include "gsi.hpp"
#include "include/gsicq.hpp"
#include "include/gsidefs.h"
#include "include/record.hpp"


#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#endif

/********************************************************************
    GSICQueue(long buff_size=GSIDefaultQueueSize);
 ctor
********************************************************************/
GSICQueue::GSICQueue(long buff_size)
:CQueue(buff_size)
{
}

/********************************************************************
    GSICQueue(long buff_size=GSIDefaultQueueSize);
 dtor
********************************************************************/

GSICQueue::~GSICQueue()
{
}

//********************************************
//     CQueue Write(GSIRecord)
//*******************************************
errorType GSICQueue::Write(const GSIRecord &record)
{
int i;
GSIChar buff[GSIRecordMaxLength],num;

    if(m_numInBuff+GSIRecordFixedLength+record.GetLength() > m_maxBuffSize)  //room for chars?
    {
      return(errQueueOverflow);
    }
	num=record.RecordToBuffer(buff);		//returns total length of buffer

	if(num==0)													//returns overall length. 0 if a bounds error
		return(errBounds);								//length is too great

    for(i=0; i<num;i++)
    {
        *(m_head) ++ = buff[i];  //store the char
        if (m_head > m_lastBuffPos)
            m_head=m_buffer;  //overflow, so wrap to front of buff
    }


    wxCriticalSectionLocker locker(m_GSIThreadCritSect);
    m_numInBuff += num;

    return(errNone);
}

//********************************************
//     CQueue Read(GSIRecord)
//*******************************************
errorType GSICQueue::Read(GSIRecord &record)
{


    GSIChar l;
    char * oldTailPtr;
    size_t i;
    GSIChar type,node;

    if(m_numInBuff==0)
	    return(errQueueEmpty);

    if(m_numInBuff<GSIRecordFixedLength)
	    return(errBounds);

    oldTailPtr=m_tail;

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
	    m_tail=oldTailPtr;		//reset m_tail to start
	    return(errBounds);
    }

    for(i=0; i<l;i++)
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
