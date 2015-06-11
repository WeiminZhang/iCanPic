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
