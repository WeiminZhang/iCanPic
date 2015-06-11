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
#ifndef STARTLE_HPP_INCLUDED
#define STARTLE_HPP_INCLUDED

#include "startledefs.h"
#include "..\socket\socket.hpp"
#include "..\socket\record.hpp"
#include <wx/TextFile.h>
//forward references


/*-----------------------------------------------------------------
    class LUT
------------------------------------------------------------------*/

class LUT
{
public:
    LUT(int num_elements, int max_value, int *init_array=NULL);
    virtual ~LUT();
	//LUT & operator =(const LUT & lut);
	LUT(const LUT& lut);
	LUT& operator=(const LUT& lut);

    errorType Replace(int index,int value);
    int Item(int index);
	int			GetNumElements() const {return m_maxIndex+1;}
	int			GetMaxValue() const		{return m_maxValue;}
private:
    int         *m_buff;
    int         m_maxIndex;
    int         m_maxValue;
};



class StartleLUT: public LUT
{
public:
	StartleLUT(int *init_array=NULL);
	~StartleLUT();
	wxString		GetString(int index);
	errorType		SetString(int index,const wxString & string);
	//errorType		Write(const wxString &fname);	//write to disk
	errorType		Write(wxTextFile *file);
	errorType		Load(wxTextFile *file); //requires an opened wxTextFile
private:
	wxString m_Text[STLMaxLUTIndex+1];

};

/*-----------------------------------------------------------------
    class StartleRecord
------------------------------------------------------------------*/
class StartleRecord
{
public:
    StartleRecord();
    virtual ~StartleRecord();
    errorType MakeGSIRecord();      //using variables m_type,m_flag etc. Create a GSIRecord

    GSIRecord &GetGSIRecord()    {return m_Record;}

    wxInt16 GetType()           const   { return(m_type);}
    wxInt16 GetFlag()           const   { return m_flag;}
    wxInt16 GetParam0()         const   { return m_param0;}
    wxInt16 GetParam1()         const   { return m_param1;}
    wxInt16 GetParam2()         const   { return m_param2;}
    wxInt16 GetParam3()         const   { return m_param3;}
    wxInt16 GetParam4()         const   { return m_param4;}
    wxInt16 GetParam5()         const   { return m_param5;}

    void SetType(wxInt16 type)          {m_type=type;}
    void SetFlag(wxInt16 flag)          {m_flag=flag;}
    void SetParam0(wxInt16 param0)      {m_param0=param0;}
    void SetParam1(wxInt16 param1)      {m_param1=param1;}
    void SetParam2(wxInt16 param2)      {m_param2=param2;}
    void SetParam3(wxInt16 param3)      {m_param3=param3;}
    void SetParam4(wxInt16 param4)      {m_param4=param4;}
    void SetParam5(wxInt16 param5)      {m_param5=param5;}
private:
    wxInt16 m_type;
    wxInt16 m_flag;
    wxInt16 m_param0;
    wxInt16 m_param1;
    wxInt16 m_param2;
    wxInt16 m_param3;
    wxInt16 m_param4;
    wxInt16 m_param5;
    GSIRecord m_Record;
};


#endif //STARTLE_HPP_INCLUDED

