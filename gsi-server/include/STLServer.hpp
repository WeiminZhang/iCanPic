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
#ifndef STARTLE_SERVER_HPP_INCLUDED
#define STARTLE_SERVER_HPP_INCLUDED

#include "server.hpp"
/*------------------------------------------------------------------------
class StartleServer :public GSIServer
Derive a class from GSIServer.
Let it handle any standard GSI records.
Override ProcessUser1Record(wxSocketBase &sock)
The base class will call ProcessUser1Record when it finds an id of GSISocketUser1
We use GSISocketUser1 for startle commands
-------------------------------------------------------------------------*/
class wxStatusBar;
class wxTextCtrl;
class GSIServer;
class StartleFrame;

class StartleServer :public GSIServer
{
public:
    StartleServer(unsigned short sock_address,wxStatusBar *status_bar=NULL,wxTextCtrl *text_ctrl=NULL,StartleFrame * startle_frame=NULL);
    virtual ~StartleServer();
	virtual errorType ProcessUser1Record(wxSocketBase &sock); //override this function only
//Access functions
    StartleFrame * const  GetStartleFrame() const {return m_StartleFrame;}// {return m_StartleFrame;}
	StartleFrame &  GetStartleFrameRef() const    {return *m_StartleFrame;}// {return m_StartleFrame;}
	LUT	&			GetAmplitudeLUT() {return *m_dBLUT;}
	LUT	&			GetNoiseAmplitudeLUT() {return *m_dBNoiseLUT;}
	wxInt16			GetNumADCDataPoints() {return m_numADCDataPoints;}
	void			SetNumADCElements(wxUint16 num) {m_numADCDataPoints=num;}
private:
	StartleFrame * m_StartleFrame;
	bool m_queuedADC;           //are we awaiting an ADC? ie will startle box send us data
    wxInt16 m_numADCDataPoints;
    LUT *m_dBLUT; 
    LUT * m_dBNoiseLUT;
};

#endif //#ifndef STARTLE_SERVER_HPP_INCLUDED
