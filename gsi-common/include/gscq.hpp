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
#ifndef CQ_HPP_INCLUDED
#define CQ_HPP_INCLUDED

#ifndef GSIRECORD_DEFINED
class GSIRecord;
#endif

#include <gerror.hpp>

#define CQUEUE_DEFINED

//#define QUEUE_DEFAULT_SIZE 1024
const int CQDefaultQueueSize=256;

class CQueue
{
public:
    CQueue(long buff_size=CQDefaultQueueSize);
    //CQueue();
    virtual ~CQueue();

    size_t GetNumInBuff()
    {
        return m_numInBuff;
    }
    char *GetHeadPtr()
    {
        return(m_head);
    }
    char *GetTailPtr()
    {
        return(m_tail);
    }
    char *GetLastBuffPos()
    {
        return m_lastBuffPos;
    }
    size_t GetMaxBuffSize()
    {
        return m_maxBuffSize;
    }
    char *GetBufferPtr()
    {
        return m_buffer;
    }

    errorType Write(char chr);
    errorType Write(char *chr,long num);
    errorType Write(wxString str);
    errorType Write(GSIRecord &record);

    errorType Read(char *chr);
    errorType Read(char *chr,long timeout);
    errorType Read(char *chr,size_t num,size_t * num_read,long timeout);
    errorType Read(GSIRecord &record);

    void Flush(void);
    //errorType Read(wxString *str,size_t num);
    errorType Read(wxString  &str,size_t num);//
    //errorType Read(GSIRecord record);
    //errorType Read(PGSIBINRECORD r);
    size_t	GetNumInBuffer() const
    {
        return(m_numInBuff);
    }
    //errorType Read(PGSIBINRECORD r,int fd,long timeout);
protected:
    char *m_head;        //ptr to insertion point
    size_t m_maxBuffSize; //max size of the buffer
    char *m_tail;        //ptr to extraction point
    size_t m_numInBuff; //num currently in Q
    char *m_buffer; //buffer storing the data
    char *m_lastBuffPos;  //* to end of buff
    wxCriticalSection m_GSIThreadCritSect;

private:

};


#endif
