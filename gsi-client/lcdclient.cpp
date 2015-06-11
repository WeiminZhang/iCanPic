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
/*
For vc600 use gsi workspace
use console as active project
use console GSIDllDebug as active configuration
ensure that gsi.dll is copied to debug directory of thread project

To test the dll
Use thread project set to Thread Win32 Debug

*/
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
// Include your minimal set of headers here, or wx.h
#include <wx/wx.h>
#endif
 
#include<wx/stopwatch.h>
#include <wx/thread.h>
#include <wx/socket.h>
//#define _POSIX_SOURCE 1 /* POSIX compliant source */

 

#ifdef __VISUALC__
#pragma warning(push, 1)
#endif

//std headers in here

#ifdef __VISUALC__
#pragma warning(pop)
// without this pragma, the stupid compiler precompiles #defines below so that
// changing them doesn't "take place" later!
#pragma hdrstop
#endif

#ifdef DMALLOC
#include <dmalloc.h>
#endif

//#include "gsi-common/include/gsi.hpp"
#include "client.hpp"
#include "lcdClient.hpp"

#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#endif



class GSILCDClient
{
public:
	GSILCDClient();
	virtual ~GSILCDClient();

	virtual errorType Cls();

private:

};


#if 0

errorType GSILCD::Write(const wxString str)
{
	errorType rv=errNone;
	size_t numFullRecords;
	size_t i,j,num;
	wxString tstr;
 
	num=str.Len();
	numFullRecords=num/(CANMaxData-1);			//see how many full records of data can be sent
	
	switch(Device->GetMode())
		{
		case GSIModeBinary:
			{
				GSIRecord lcd(rtGSILCDMessage);		//instantiate the record
				lcd.SetNode(node);
				lcd.SetData(RS232RecordOffsetToSubfunction,GSILCD::WriteString);
				if(numFullRecords)
				{
					lcd.SetLength(CANMaxData);						//send a full data buffer
					for(i=0;i<numFullRecords;i++)
					{
						tstr=str.Mid(i*(CANMaxData-1), CANMaxData-1);
							//Mid(size_t first, size_t count = wxSTRING_MAXLEN) const
						for(j=0;j<CANMaxData;j++)			//first data item is subfunction
							{
								lcd.SetData(j+1,tstr[j]);
							}
	
						rv=Device->Write(lcd);						//send the commands 7 at a time if possible
						if(rv)
							return(rv);
						num -= (CANMaxData-1);
					}
				}
				wxASSERT(num < CANMaxData);
				if(num)			//any residual data to send
				{
					lcd.SetLength((GSIChar)(num+1));		//don't forget the subfunction 
					tstr=str.Right(num);			//default value will read the rest of the string
					for(i=0;i<num;i++)
						{
							lcd.SetData(i+1,tstr[i]);				//fill final record
						}
					rv=Device->Write(lcd);						//send final data
					if (rv)
						return(rv);
				}
				break;
			}
		case GSIModeAscii:
				{
				size_t i;
				GSIChar recd;
				for(i=0;i<str.Len();i++)
				{
					rv=Device->Write(str[i],&recd);
				}
			}
			break;
		}
	return(rv);
}


errorType GSILCD::Write(const GSIChar chr)
{
	errorType rv=errNone;
	chr;
	return(rv);
}

errorType GSILCD::CursorBeginningOfLine()
{
	errorType rv=errNone;
	switch(Device->GetMode())
		{
		case GSIModeBinary:
			{
				GSIRecord lcd(rtGSILCDMessage);		//instantiate the record
				lcd.SetNode(node);
				lcd.SetData(RS232RecordOffsetToSubfunction,GSILCD::CtrJumpTable);
				lcd.SetData(RS232RecordOffsetToSubfunction+1,rtLCDCursBeginningOfLine);
				rv=lcd.SetLength(2);		// A single Don't forget the subfunction
				rv=Device->Write(lcd);
				if(rv)
					return(rv);
				break;
			}
		case GSIModeAscii:
			rv=Device->Write(rtLCDCursBeginningOfLine);
			if (rv)
				return(rv);
			break;
		}
	
	return(rv);
}


errorType GSILCD::Terminal(size_t num,GSIChar fn)
{
	errorType rv=errNone;
	size_t numFullRecords;
	size_t i;
	
	numFullRecords=num/(CANMaxData-1);			//see how many full records of data can be sent
	
	switch(Device->GetMode())
		{
		case GSIModeBinary:
			{
				GSIRecord lcd(rtGSILCDMessage,fn);		//instantiate the record, filling data buffer with NewLine codes
				lcd.SetNode(node);
				lcd.SetData(RS232RecordOffsetToSubfunction,GSILCD::CtrJumpTable);
				if(numFullRecords)
				{
					lcd.SetLength(CANMaxData);						//send a full data buffer
					for(i=0;i<numFullRecords;i++)
					{
						rv=Device->Write(lcd);						//send the commands 7 at a time if possible
						if(rv)
							return(rv);
						num -= (CANMaxData-1);
					}
				}
				wxASSERT(num < CANMaxData);
				if(num)			//any residual data to send
				{
					lcd.SetLength((GSIChar)(num+1));		//don't forget the subfunction 
					rv=Device->Write(lcd);						//send final data
					if (rv)
						return(rv);
				}
				break;
			}
		case GSIModeAscii:
			for(i=0;i<num;i++)
			{
				rv=Device->Write(fn);
				if (rv)
					return(rv);
			}
			break;
		}
	return(rv);
}


errorType GSILCD::Terminal(GSIChar fn)
{
	errorType rv=errNone;
	switch(Device->GetMode())
		{
		case GSIModeBinary:
			{
				GSIRecord lcd(rtGSILCDMessage);		//instantiate the record
				lcd.SetNode(node);
				lcd.SetData(RS232RecordOffsetToSubfunction,GSILCD::CtrJumpTable);
				lcd.SetData(RS232RecordOffsetToSubfunction+1,fn);
				rv=lcd.SetLength(2);		// A single Don't forget the subfunction
				rv=Device->Write(lcd);
					if(rv)
						return(rv);
				break;
			}
		case GSIModeAscii:
			rv=Device->Write(fn);
				if (rv)
					return(rv);
			break;
		}
	return(rv);
}

#endif


#if 0
errorType GSILCD::	CursorRight(size_t num,GSIChar node)
{
	return(Terminal(num,node,rtLCDCursRight));
}

errorType GSILCD::	CursorRight(GSIChar node)
{
	return(Terminal(node,rtLCDCursRight));
}
#endif
#if 0
errorType GSILCDClient::Cls()
{
	return(errNone);	
}
#endif
#if 0

errorType GSILCD::CursorLeft(size_t num,GSIChar node)
{
	errorType rv=errNone;
	size_t numFullRecords;
	size_t i;
	
	numFullRecords=num/(CANMaxData-1);			//see how many full records of data can be sent
	
	switch(Device->GetMode())
		{
		case GSIModeBinary:
			{
				GSIRecord lcd(rtGSILCDMessage,rtLCDCursLeft);		//instantiate the record, filling data buffer with NewLine codes
				lcd.SetNode(node);
				lcd.SetData(RS232RecordOffsetToSubfunction,GSILCD::CtrJumpTable);
				if(numFullRecords)
				{
					lcd.SetLength(CANMaxData);						//send a full data buffer
					for(i=0;i<numFullRecords;i++)
					{
						rv=Device->Write(lcd);						//send the commands 7 at a time if possible
						if(rv)
							return(rv);
						num -= (CANMaxData-1);
					}
				}
				wxASSERT(num < CANMaxData);
				if(num)			//any residual data to send
				{
					lcd.SetLength((GSIChar)(num+1));		//don't forget the subfunction 
					rv=Device->Write(lcd);						//send final data
					if (rv)
						return(rv);
				}
				break;
			}
		case GSIModeAscii:
			for(i=0;i<num;i++)
			{
			rv=Device->Write(rtLCDCursLeft);
			if (rv)
				return(rv);
		}
		break;
	}
	return(rv);
}

errorType GSILCD::CursorLeft(GSIChar node)
{
	errorType rv=errNone;
	switch(Device->GetMode())
		{
		case GSIModeBinary:
			{
				GSIRecord lcd(rtGSILCDMessage);		//instantiate the record
				lcd.SetNode(node);
				lcd.SetData(RS232RecordOffsetToSubfunction,GSILCD::CtrJumpTable);
				lcd.SetData(RS232RecordOffsetToSubfunction+1,rtLCDCursLeft);
				rv=lcd.SetLength(2);		// A single Don't forget the subfunction
				rv=Device->Write(lcd);
					if(rv)
						return(rv);
				break;
			}
		case GSIModeAscii:
			rv=Device->Write(rtLCDCursLeft);
				if (rv)
					return(rv);
			break;
		}

	return(rv);
}

errorType GSILCD::UserBreak(GSIChar node)
{
	errorType rv=errNone;
	return(rv);
}

errorType GSILCD:: DeleteCharFwd(size_t num,GSIChar node)
{
	errorType rv=errNone;
	return(rv);
}

errorType GSILCD:: DeleteCharFwd(GSIChar node)
{
	errorType rv=errNone;
	return(rv);
}

errorType GSILCD:: CursorEndOfLine(GSIChar node)
{
	errorType rv=errNone;
	switch(Device->GetMode())
		{
		case GSIModeBinary:
			{
				GSIRecord lcd(rtGSILCDMessage);		//instantiate the record
				lcd.SetNode(node);
				lcd.SetData(RS232RecordOffsetToSubfunction,GSILCD::CtrJumpTable);
				lcd.SetData(RS232RecordOffsetToSubfunction+1,rtLCDCursEndOfLine);
				rv=lcd.SetLength(2);		// A single Don't forget the subfunction
				rv=Device->Write(lcd);
				if(rv)
					return(rv);
				break;
			}
		case GSIModeAscii:
			rv=Device->Write(rtLCDCursEndOfLine);
			if (rv)
				return(rv);
			break;
		}

	return(rv);
}


errorType GSILCD::	Backspace(GSIChar node)
{
	errorType rv=errNone;
	return(rv);
}

errorType GSILCD::	Backspace(size_t num,GSIChar node)
{
	errorType rv=errNone;
	return(rv);
}

errorType GSILCD:: Tab(size_t num,GSIChar node)
{
	errorType rv=errNone;
	return(rv);
}

errorType GSILCD:: Tab(GSIChar node)
{
	errorType rv=errNone;
	return(rv);
}

errorType GSILCD:: Cls(GSIChar node)
{
	errorType rv=errNone;

	switch(Device->GetMode())
		{
		case GSIModeBinary:
			{
//This is currently the best method of calling terminal functions.
//use the jmp table location on the node.
//Alternative is to allow pic to decode and call the jmp table
//this method is probably quicker for the pic node
//need to test both
//Direct rtLCDCls record method is more intuitive
//Important note.
//The rtLCDCls is a direct call to the lcd attached to this pc only. It will not
//send down the CAN, thus node is ignored.
//To ensure CAN is honoured, use rtGSILCDMessage

//Note
//Can send more than one command in the record. The pic node will use record length
//to determine the number of commands, thus could send a string using this function
//Must keep overall length to 8 or less, the size of a CAN message to be sure it will
//operate correctly
				GSIRecord lcd(rtGSILCDMessage);		//instantiate the record
				lcd.SetNode(node);
				lcd.SetData(RS232RecordOffsetToSubfunction,GSILCD::CtrJumpTable);
				lcd.SetData(RS232RecordOffsetToSubfunction+1,rtLCDCls);
				rv=lcd.SetLength(2);		//Don't forget the subfunction

				rv=Device->Write(lcd);
				if(rv)
					return(rv);
				break;
			}
		case GSIModeAscii:
			Device->Write(rtLCDCls);
			break;
		}
	return(rv);
}

errorType GSILCD:: NewLine(GSIChar node)
{
	errorType rv=errNone;
	switch(Device->GetMode())
		{
		case GSIModeBinary:
			{
				GSIRecord lcd(rtGSILCDMessage);		//instantiate the record
				lcd.SetNode(node);
				lcd.SetData(RS232RecordOffsetToSubfunction,GSILCD::CtrJumpTable);
				lcd.SetData(RS232RecordOffsetToSubfunction+1,rtLCDNewLine);
				rv=lcd.SetLength(2);		// A single Don't forget the subfunction
				rv=Device->Write(lcd);
					if(rv)
						return(rv);
				break;
			}
		case GSIModeAscii:
			rv=Device->Write(rtLCDNewLine);
				if (rv)
					return(rv);
			break;
		}

	return(rv);

}

errorType GSILCD:: NewLine(size_t num,GSIChar node)
{
	errorType rv=errNone;
	size_t numFullRecords;
	size_t i;

	numFullRecords=num/(CANMaxData-1);			//see how many full records of data can be sent

	switch(Device->GetMode())
		{
		case GSIModeBinary:
			{
				GSIRecord lcd(rtGSILCDMessage,rtLCDNewLine);		//instantiate the record, filling data buffer with NewLine codes
				lcd.SetNode(node);
				lcd.SetData(RS232RecordOffsetToSubfunction,GSILCD::CtrJumpTable);
				if(numFullRecords)
				{
					lcd.SetLength(CANMaxData);						//send a full data buffer
					for(i=0;i<numFullRecords;i++)
					{
						rv=Device->Write(lcd);						//send the commands 7 at a time if possible
						if(rv)
							return(rv);
						num -= (CANMaxData-1);
					}
				}
				if(num)			//any residual data to send
				{
					lcd.SetLength((GSIChar)num);
					rv=Device->Write(lcd);						//send final data
					if (rv)
						return(rv);
				}
				break;
			}
		case GSIModeAscii:
			for(i=0;i<num;i++)
			{
				rv=Device->Write(rtLCDNewLine);
				if (rv)
					return(rv);
			}
			break;
		}
	return(rv);
}

errorType GSILCD:: CursorDown(size_t num,GSIChar node)
{
	errorType rv=errNone;
	size_t numFullRecords;
	size_t i;

	numFullRecords=num/(CANMaxData-1);			//see how many full records of data can be sent

	switch(Device->GetMode())
		{
		case GSIModeBinary:
			{
				GSIRecord lcd(rtGSILCDMessage,rtLCDCursDown);		//instantiate the record, filling data buffer with NewLine codes
				lcd.SetNode(node);
				lcd.SetData(RS232RecordOffsetToSubfunction,GSILCD::CtrJumpTable);
				if(numFullRecords)
				{
					lcd.SetLength(CANMaxData);						//send a full data buffer
					for(i=0;i<numFullRecords;i++)
					{
						rv=Device->Write(lcd);						//send the commands 7 at a time if possible
						if(rv)
							return(rv);
						num -= (CANMaxData-1);
					}
				}
				if(num)			//any residual data to send
				{
					lcd.SetLength((GSIChar)num);
					rv=Device->Write(lcd);						//send final data
					if (rv)
						return(rv);
				}
				break;
			}
		case GSIModeAscii:
			for(i=0;i<num;i++)
			{
				rv=Device->Write(rtLCDCursDown);
				if (rv)
					return(rv);
			}
			break;
		}


	return(rv);
}

errorType GSILCD:: CursorDown(GSIChar node)
{
	errorType rv=errNone;
	switch(Device->GetMode())
		{
		case GSIModeBinary:
			{
				GSIRecord lcd(rtGSILCDMessage);		//instantiate the record
				lcd.SetNode(node);
				lcd.SetData(RS232RecordOffsetToSubfunction,GSILCD::CtrJumpTable);
				lcd.SetData(RS232RecordOffsetToSubfunction+1,rtLCDCursDown);
				rv=lcd.SetLength(2);		// A single Don't forget the subfunction
				rv=Device->Write(lcd);
					if(rv)
						return(rv);
				break;
			}
		case GSIModeAscii:
			rv=Device->Write(rtLCDCursDown);
				if (rv)
					return(rv);
			break;
		}

	return(rv);
}

errorType GSILCD:: CursorUp(size_t num,GSIChar node)
{
	errorType rv=errNone;
	return(rv);
}

errorType GSILCD:: CursorUp(GSIChar node)
{
	errorType rv=errNone;
	return(rv);
}

#endif
