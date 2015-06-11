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


#define CQUEUE_DEFINED

//#define QUEUE_DEFAULT_SIZE 1024
const int GSIDefaultQueueSize=1024;

class GSI_API CQueue
{
public:
  CQueue(long buff_size=GSIDefaultQueueSize);
  //CQueue();
  virtual ~CQueue();
  
  errorType Write(GSIChar chr);
  errorType Write(GSIChar *chr,long num);
  errorType Write(wxString str);
  errorType Write(GSIRecord &record);

  errorType Read(GSIChar *chr);
  errorType Read(GSIChar *chr,long timeout);
  errorType Read(GSIChar *chr,size_t num,size_t * num_read,long timeout); 
	errorType Read(GSIRecord &record);
  
  void Flush(void);
  //errorType Read(wxString *str,size_t num);
  errorType Read(wxString  &str,size_t num);//
  //errorType Read(GSIRecord record);
  //errorType Read(PGSIBINRECORD r);
  size_t	GetNumInBuffer() const {return(m_numInBuff);}
  //errorType Read(PGSIBINRECORD r,int fd,long timeout);
private:
	wxCriticalSection m_GSIThreadCritSect;
  size_t m_maxBuffSize; //max size of the buffer
  GSIChar *m_head;        //ptr to insertion point
  GSIChar *m_tail;        //ptr to extraction point
  size_t m_numInBuff; //num currently in Q
  GSIChar *m_buffer; //buffer storing the data
  GSIChar *m_lastBuffPos;  //* to end of buff
};




#endif
