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
#ifndef RECORD_HPP_INCLUDED
#define RECORD_HPP_INCLUDED

#include "include/gsidefs.h"

#include "gerror.h"

#include "include/can.hpp"              //reqd for emedded CANFrame class

#include "include/microcontroller.h"

//forward references
class GSIRecord;
class CANFrame;
class GSICANFrame;

enum GSIEnumRecord;     //gcc doesn't allow forward declarations, so must #include "microcontroller.h"

#pragma pack(1)
struct GSIRecord_s
{
  GSIChar type; //char is 8 bits by definition. Still use wxChar for safety
  GSIChar node;
  GSIChar length;
  GSIChar data[GSIRecordMaxDataLength];
};
typedef GSIRecord_s GSIBINRECORD, *PGSIBINRECORD;
#pragma pack()

class GSISocketData;

class GSIRecord
{
public:
//Note, explict ctor only has meaning for single argument ctors. Don't want te
//compiler making implicit conversions to these ctors. User must explicitly specify
	GSIRecord();
	explicit	GSIRecord(GSIEnumRecord record);
				GSIRecord(GSIEnumRecord r,GSIChar fill);	//fill the data buffer with chr fill
	explicit	GSIRecord(const wxString & s);						//binary string->record
				GSIRecord(const GSIChar *buff,wxUint32 length);
	explicit	GSIRecord(const GSIRecord &r);										//copy ctor, can be invoked with "="

    virtual ~GSIRecord();
	//GSIRecord & operator =(const GSIRecord & r);
	GSIRecord& operator=(const GSIRecord& r);

    GSIChar GetType()					const   {return(m_binRecord.type);}
    GSIChar GetNode()					const   {return(m_binRecord.node);}
    GSIChar GetLength()					const   {return(m_binRecord.length);}
    GSIChar GetData(size_t index)		const   {return(m_binRecord.data[index]);}
    bool    IsReturnedRecord()          const   {return(m_isReturnedRecord);}
	bool	GetFilledFlag()			    const	{return(m_filled);}
    GSIRecord * const GetReturnedRecord()  const	{return(m_ReturnedRecord);}
    void    SetReturnedRecord(GSIRecord *r)     {
                                                    m_ReturnedRecord=r;
                                                    m_isReturnedRecord=true;
                                                    if(r==NULL)
                                                        m_isReturnedRecord=false;

                                                 }


    errorType SetLength(GSIChar l);
    //errorType SetReturnedDataLength(GSIChar l);
    void SetNode(int n=GSI_NODE_THIS)	        {m_binRecord.node=n; SetNodeString();}
    errorType SetType(GSIEnumRecord t);
    errorType SetType(GSIChar chr);
    void SetFilledFlag(bool filled)			    {m_filled=filled; SetFilledFlagString();}
    errorType SetData(int index,GSIChar chr);

    errorType SetReturnedData(int index,GSIChar chr);

    GSIChar   RecordToBuffer(GSIChar *buff) const;

    errorType SocketDataToRecord(GSISocketData &sd);
    const GSIBINRECORD & GetBinRecord(void) const {return(m_binRecord);}

    void      FrameToRecord(const CANFrame & f);             //create record from supplied CAN frame

	//This is not a const object, it can be modified, use with caution
	//perhaps should make constant and use MakeGSICANFrame
    GSICANFrame & GetGSICANFrameRef()      {return m_Frame;}        //return by value ie duplicate the frame
    errorType MakeGSICANFrame(GSICANFrame & frame);

    errorType FillGSICANFrame();

    void            SetTypeString(const char *str)          {m_TypeString=str;}
    void            SetSubfunctionString(const wxString &str)   {m_SubfuncString=str;}

    void            SetNodeString();
    void            SetDataString();
    errorType       SetTypeString();
    void            SetLengthString();
    void            SetFilledFlagString();
	void			SetInfoString1(const wxString &s) {m_InfoString1=s;}
	void			SetInfoString2(const wxString &s) {m_InfoString2=s;}

	wxString		MakeInfoString(); //return string that documents record in human readable form
    void            SetStrings();
	const wxString GetTypeString()      const{ return(m_TypeString); }
	const wxString GetNodeString()      const{ return(m_NodeString); }
	const wxString GetSubfuncString()   const{ return(m_SubfuncString); }
	const wxString GetDataString()      const{ return(m_DataString); }
    const wxString GetLengthString()    const{ return(m_LengthString); }
    const wxString GetFilledFlagString() const{ return(m_FilledFlagString); }
	const wxString GetInfoString1() const{ return(m_InfoString1); }
	const wxString GetInfoString2() const{ return(m_InfoString2); }

protected:

private:
	void ShallowCopy(const GSIRecord& r);	//used by copy ctor and operator=
    GSIBINRECORD m_binRecord;
    bool m_filled;
    GSICANFrame   m_Frame;                //embedded GSICANFrame for convenience. For when we receive them
   	wxString m_TypeString;
	wxString m_NodeString;
	wxString m_SubfuncString;
	wxString m_DataString;
    wxString m_LengthString;
    wxString m_FilledFlagString;
	wxString m_InfoString1;
	wxString m_InfoString2;
    //if a record is returned with data in it. ie not a simple RS232Record SUCCESS, the m_returnedDataBuffer
    //is filled with the data and the m_returnedDataLength variable is >0
    //So, a fail will return an RS232Record with subfunction set to GSIRS232MESSAGE_FAILURE
    //User can obtain a GSIRecord by using the GSIRecord(const GSIChar *buff,wxUint32 length) ctor.
    GSIRecord *m_ReturnedRecord;
    bool m_isReturnedRecord;
    //GSIChar m_ReturnedDataBuffer[GSIRecordMaxDataLength];
};

#endif
