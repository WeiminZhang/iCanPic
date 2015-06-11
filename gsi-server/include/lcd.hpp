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
#ifndef LCD_HPP_INCLUDED
#define LCD_HPP_INCLUDED

#include <gerror.h>
#include "gsidefs.h"
#include "microcontroller.h"

/*
LCD node support
 rtGSILCDMessage = 60,
Data position 0 equates
Set record.data[0] to these for the specific function
data bytes 1-7 then hold other data
 GSI_LCD_MESSAGE_INSTRUCTION  0
 GSI_LCD_MESSAGE_WRITE_STRING  1
 GSI_LCD_MESSAGE_WRITE_CHAR  2
 GSI_LCD_MESSAGE_GET_STRING  3
 GSI_LCD_MESSAGE_CTR_JMP_TBL  4
 GSI_LCD_MESSAGE_GET_CAPABILITES 5
*/

//forward references
class GSIDevice;

//const LCDOffetSubfunction=0;	//use GSIOffsetSubfunction
class GSILCDCapabilities
{
public:
	GSILCDCapabilities();
	virtual ~GSILCDCapabilities();
	int GetRows	() const {return rows;}
	int GetCols() const {return(cols);}
private:
	int rows;
	int cols;
	GSIChar flag;
	bool	graphics;
};

class GSI_API GSILCD: public GSILCDCapabilities
{
public:

enum GSILCDEnum
{
	LCD_INSTRUCTION=0,
	LCD_WRITE_STRING=1,
	LCD_WRITE_CHAR=2,
	LCD_GET_STRING=3,
	LCD_CTR_JUMP_TABLE=4,
	LCD_GET_CAPABILITIES=5			//returns COL, ROW as 16 bit value, Flag as 8 bit value (currently always 0)
};
	GSILCD();
	explicit GSILCD(GSIDevice *d);
	virtual ~GSILCD();

	//virtual SetCommDevice(GSIDevice * d) {m_CommDevice=d;};
	virtual void SetDevice(GSIDevice * d) {m_CurrentDevice=d;};
	GSIDevice * const GetDevice() const {return m_CurrentDevice;}

	GSIChar GetNode() const {return m_node;}
	void SetNode(int n) {m_node=n;}
	//virtual SetMode(GSIEnumMode flag) {mode=flag;}		;Use Device->GetMode
	//virtual GSIEnumMode	GetMode() const {return(mode);}

	virtual errorType Write(const wxString &str);	//
	virtual	errorType Write(GSIChar chr);

	virtual errorType CursorBeginningOfLine();

	virtual errorType CursorLeft(size_t num){return(Terminal(num,rtLCDCursLeft));}
	virtual errorType CursorLeft(){return(Terminal(m_node,rtLCDCursLeft));}

	virtual errorType UserBreak(){return(Terminal(rtGSIUserAbort));}

	virtual errorType DeleteCharFwd(size_t num){return(Terminal(num,rtLCDDelCharFwd));}
	virtual errorType DeleteCharFwd(){return(Terminal(rtLCDDelCharFwd));}

	virtual errorType CursorEndOfLine(){return(Terminal(rtLCDCursEndOfLine));}

	virtual	errorType	CursorRight(size_t num){return(Terminal(num,rtLCDCursRight));}
	virtual	errorType	CursorRight(){return(Terminal(rtLCDCursRight));}

	virtual	errorType	Backspace(){return(Terminal(rtLCDBackspace));}
	virtual	errorType	Backspace(size_t num){return(Terminal(num,rtLCDBackspace));}

	virtual errorType Tab(size_t num){return(Terminal(num,rtLCDTab));}
	virtual errorType Tab(){return(Terminal(rtLCDTab));}

	virtual errorType Cls(){return(Terminal(rtLCDCls));}

	virtual errorType NewLine(){return(Terminal(rtLCDNewLine));}
	virtual errorType NewLine(size_t num){return(Terminal(num,rtLCDNewLine));}

	virtual errorType CursorDown(size_t num){return(Terminal(num,rtLCDCursDown));}
	virtual errorType CursorDown(){return(Terminal(rtLCDCursDown));}

	virtual errorType CursorUp(size_t num){return(Terminal(num,rtLCDCursUp));}
	virtual errorType CursorUp(){return(Terminal(rtLCDCursUp));}

	virtual errorType Terminal(size_t num,GSIChar fn);
	virtual errorType Terminal(GSIChar fn);

protected:

private:
	int m_node;
	GSIEnumMode	m_mode;
	//GSIDevice *m_CommDevice;			//allows us to write the records to the device (default is rs232)
	GSIDevice *m_CurrentDevice;
};


#endif //LCD_H_INCLUDED


