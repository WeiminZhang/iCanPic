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
#ifndef GERROR_HPP_INCLUDED
#define GERROR_HPP_INCLUDED
#include "gerror.h"

class wxString;

class ErrorClass
{
public:
    ErrorClass();
    explicit ErrorClass(errorType err);

    virtual ~ErrorClass();
    const wxString & GetString(errorType err);
    const wxString & GetString();
	errorType GetErrorCode(){return m_err;}
private:
	errorType m_err;
    wxString m_ErrString;
    wxString m_TempErrString;
};


#endif //GERROR_HPP_INCLUDED
