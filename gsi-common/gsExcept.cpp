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
#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include "wx/wx.h"
#endif



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

#ifdef __VISUALC__
#pragma warning(pop)
// without this pragma, the stupid compiler precompiles #defines below so that
// changing them doesn't "take place" later!
#pragma hdrstop
#endif

//#include <gerror.hpp>
#include "include/gsbexcept.hpp"

//#include "mpatrol.h"
/*********************************************************************************************************************
    ctor
gsException::gsException(errorType e,const wxString &s,unsigned int data0,unsigned int data1, unsigned int data2)

ctor that parially fills extra data array, can also explicitly set it with member function
**********************************************************************************************************************/
gsException::gsException(errorType e,const wxString &s,unsigned int data0,unsigned int data1, unsigned int data2)
//  :std::runtime_error(s.c_str())
{
    m_err=e;
	m_UserStr=s;
    m_Str.Empty();
    for(int i=0;i<GSExcMaxData;i++)
    {
        m_data[i]=0;
    }
    m_data[0]=data0;
    m_data[1]=data1;
    m_data[2]=data2;
}

/*************************************************************************************
    dtor
gsException::~gsException()

Do nothing dtor, required as we have defined it as virtual so that we can allocate
memory safely in derived classes
*************************************************************************************/
gsException::~gsException()
{
}


/*----------------------------------------------
    GSIException::GetString()
Use the errorType to return an error string
------------------------------------------------*/
const wxString &gsException::GetString()
{
ErrorClass e(GetErrorCode());
    m_Str=e.GetString();
    return(m_Str);
}


/*------------------------------------------
    gsException::GetUserString()
-------------------------------------------*/
const wxString & gsException::GetUserString()
{
    return(m_UserStr);

}

/******************************************************
    int ErrorClass::GetData(int index)

********************************************************/
unsigned int gsException::GetData(int index)
{
    wxASSERT_MSG(index < GSExcMaxData,"Index out of range");

    if(index >= GSExcMaxData)
        return(0xffffffff);
    return(m_data[index]);
}

/******************************************************
    int ErrorClass::SetData(int index)

********************************************************/

errorType   gsException::SetData(int index,unsigned int data)
{
    wxASSERT_MSG(index < GSExcMaxData,"Index out of range");
    if(index >= GSExcMaxData) return(errBounds);

    m_data[index]=data;
    return(errNone);
}

