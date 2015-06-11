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
#ifndef GSI_COMMANDS_HPP
#define GSI_COMMANDS_HPP

#include "Record.hpp"
#include "device.hpp"
#include "socket.hpp"


/*!
	class GSICommandXXX
*/

class GSICommandXXX
{
public:
        //explicit GSICommand();
        //GSICommandXXX();
            GSICommandXXX(wxTextCtrl * const status_text=NULL);
    virtual ~GSICommandXXX();

    virtual	errorType	Write();		//write to device if any set
    virtual	errorType	FillGSIRecord();

            GSIRecord & GetGSIRecord() {return m_GSIRecord;}
            GSIRecord const & GetGSIRecord() const {return m_GSIRecord;}

            wxString	GetCommandString() const {return (m_CommandString);}
            void		SetCommandString(const wxString &str) {m_CommandString=str;}

    //We test the pointer to see if a Device has been set, so can't use a reference.
    //can probably change this behaviour and ensure a default device is always present
    const   GSIDevice * const GetDevice() const {return m_Device;}      //pointer can't change, what is pointed to can't change
            GSIDevice * const GetDevice() {return m_Device;}      //pointer can't change what's pointed to can change
              //GSIDevice * GetDevice() {return m_Device;}            //pointer and what's pointed to can change

        //GSIDevice const &GetDevice() const {return *m_Device;}
        //GSIDevice       &GetDevice()  {return *m_Device;}
            void		    AppendText(const wxString & text);
            wxInt16		    GetNode() const {return (GetGSIRecord().GetNode());}
            errorType	    SetNode(GSINodeHandle node);
            void		    SetStatusReportLevel(StatusReportLevel level) {m_statusReportLevel=level;}
            StatusReportLevel GetStatusReportLevel() const {return m_statusReportLevel;}
            bool		    IsWritable() const {return m_isWritable;}

                            wxString	    GetErrorString()const  {return m_ErrorString;}

        //virtual functions
            virtual		    errorType	Execute(GSIFrame &Frame,GSISocketReturnData &sr);	//execute the class specific command
            virtual		    errorType	Read(wxSocketBase &sock); //read from Socket
            void		    SetStatusText(wxTextCtrl * const status_text) {m_StatusText=status_text;}

protected:
            void	    	SetIsWritable(bool is_writable) {m_isWritable=is_writable;}

            void	    	SetDevice(GSIDevice * const device) {m_Device=device;}

            void		    SetErrorString(const wxString &str) {m_ErrorString=str;}
            void		    SetErrorString(errorType ec);
            wxTextCtrl      *const GetStatusText()  {return m_StatusText;} //pointer is const, what's pointed to isn't
    const   wxTextCtrl    *const GetStatusText() const {return m_StatusText;} //both constant
private:
            void		Init();
            GSIRecord m_GSIRecord;
            wxString m_CommandString;
            GSIDevice * m_Device;
            wxTextCtrl * m_StatusText;
            StatusReportLevel m_statusReportLevel;
            bool		m_isWritable;
            wxString	m_ErrorString;

};

/*!
class GSICommandOpenDeviceXXX : public GSICommandXXX
class that holds hardware specific data.
A Node* or Device* can Write to a GSIDevice using this class.
This class holds a GSIRecord that can be read by the microcontrolelr firmware
*/
class GSICommandOpenDeviceXXX : public GSICommandXXX
{
public:
    GSICommandOpenDeviceXXX(const wxString & dev_str="");

    virtual ~GSICommandOpenDeviceXXX();

//Access functions
	void				SetDeviceStr(const wxString &dev_str)	{m_devStr=dev_str;}
	wxString			GetDeviceString() const {return m_devStr;}
	errorType			FillGSIRecord();
	GSIDevHandle		GetDeviceHandle() const {return m_handle;}
	void				SetDeviceHandle(GSIDevHandle h) {m_handle=h;}
private:
	wxString			m_devStr;	//Ensure Client and Server use same types
	GSIDevHandle		m_handle;
	int					m_devType;
};




/*!
class GSICommandCloseDeviceXXX : public GSICommandXXX
class that holds hardware specific data.
A Node* or Device* can Write to a GSIDevice using this class.
This class holds a GSIRecord that can be read by the microcontrolelr firmware
*/
class GSICommandCloseDeviceXXX : public GSICommandXXX
{
public:
    GSICommandCloseDeviceXXX(GSIDevHandle h);

	explicit GSICommandCloseDeviceXXX() {m_handle=GSINotAValidDevHandle;}

    virtual ~GSICommandCloseDeviceXXX();

//Access functions
	errorType			FillGSIRecord();
	GSIDevHandle		GetDeviceHandle() const {return m_handle;}
	errorType			SetDeviceHandle(GSIDevHandle hdev);

private:
	GSIDevHandle		m_handle;
};


//-----End GSICommandCloseDeviceXXX -----------------


/*!
class GSICommandAddNodeXXX : public GSICommandXXX
class that holds hardware specific data.
A Node* or Device* can Write to a GSIDevice using this class.
This class holds a GSIRecord that can be read by the microcontrolelr firmware
*/
class GSICommandAddNodeXXX : public GSICommandXXX
{
public:
						GSICommandAddNodeXXX(GSINodeHandle h,GSIDevHandle hd=GSINotAValidDevHandle);
						explicit			GSICommandAddNodeXXX() {m_hnode=GSINotAValidNodeHandle,m_hdev=GSINotAValidDevHandle;}
    virtual				~GSICommandAddNodeXXX();
//Access functions
	void		SetNodeHandle(GSINodeHandle h) {m_hnode=h;}
	GSINodeHandle	GetNodeHandle() const {return m_hnode;}
		void		SetDeviceHandle(GSIDevHandle h) {m_hdev=h;}
	GSINodeHandle	GetDeviceHandle() const {return m_hdev;}

	errorType		FillGSIRecord();


private:
	GSINodeHandle		m_hnode;
	GSIDevHandle		m_hdev;
};


/*!
class GSICommandRemoveNodeXXX : public GSICommandXXX
class that holds hardware specific data.
A Node* or Device* can Write to a GSIDevice using this class.
This class holds a GSIRecord that can be read by the microcontrolelr firmware
*/
class GSICommandRemoveNodeXXX : public GSICommandXXX
{
public:
						GSICommandRemoveNodeXXX(GSINodeHandle h,GSIDevHandle=GSINotAValidDevHandle);
	explicit			GSICommandRemoveNodeXXX();

    virtual ~GSICommandRemoveNodeXXX();

//Access functions
			errorType	FillGSIRecord();
			GSINodeHandle	GetNode() {return m_hnode;}
			GSIDevHandle	GetDeviceHandle() {return m_hdev;}

			void			SetNode(GSINodeHandle hnode) {m_hnode=hnode;}
			void			SetDeviceHandle(GSIDevHandle hdev) {m_hdev=hdev;}
private:
	GSINodeHandle m_hnode;
	GSIDevHandle  m_hdev;
};




#endif
