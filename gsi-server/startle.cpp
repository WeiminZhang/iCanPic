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
/*------------------------------------------------------------------------------------------------

Startle box specific code in here.
Note that this file is shared with the StartleTest program, so only general purpose classes in here
GUI and server specific stuff is in StartleFrame.cpp
The Startle box is a piece of hardware that is controlled by a pic 18F458
It utilizes a DDS (Direct Digital Synthesis) chip for generating sine waves
and an LM1972 attenuator along with a set of relays and other assorted hardware
The GSI socket interface through the derived STLServerxxx, STLSocketxxx classes
controls this device through a PC serial port attached to the pic.
The STL classes are derived from GSIxxx classes.
-------------------------------------------------------------------------------------------------*/
// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#  pragma hdrstop
#endif

// for all others, include the necessary headers
#ifndef WX_PRECOMP
#  include "wx/wx.h"
#endif

#include "wx/socket.h"
#include <wx/stopwatch.h>
#include <wx/progdlg.h>
#include <wx/tokenzr.h>
//#include <wx/choicdlg.h>

#include "include/gsi.hpp"

#ifdef STARTLE_SERVER //see gsidefs.h
#warning  ("Compiling Startle box code")

#include "include/record.hpp"
#include "include/server.hpp"
#include "include/socket.hpp"
#include "include/startle.hpp"
#include "include/STLSocketCommands.hpp"
#include "include/STLServerCommands.hpp"
#include "include/StartleFrame.hpp"
#include "include/microcontroller.h"

#if __VISUALC__
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define DEBUG_NEW new(_NORMAL_BLOCK ,__FILE__, __LINE__)
	#else
		#define DEBUG_NEW new
	#endif
#endif

#if __GNUC__
    #define DEBUG_NEW new
#endif

// --------------------------------------------------------------------------
// resources
// --------------------------------------------------------------------------

// the application icon
#if defined(__WXGTK__) || defined(__WXX11__) || defined(__WXMOTIF__) || defined(__WXMAC__)
#  include "mondrian.xpm"
#endif

/**********************************************************************************

LUT::LUT(int numElements)

***********************************************************************************/

LUT::LUT(int num_elements, int max_value, int* init_array)
{
    m_buff=NULL;
    m_maxValue=max_value;
    if(num_elements >0)
    {
        m_maxIndex=num_elements-1;
    }
    else
    {
        STL_THROW("LUT requested array size <0!",errBounds);
        return;
    }

    m_buff=DEBUG_NEW int[num_elements];
    if(init_array)
    {
        for(int i=0;i<m_maxIndex+1;i++)
        {
            m_buff[i]=init_array[i];
        }
    }
	else
	{
		for(int i=0;i<m_maxIndex+1;i++)
			m_buff[i]=0;
	}
}

LUT::~LUT()
{

    delete m_buff;
}

/*!
LUT::LUT(const LUT& lut)
*/
LUT::LUT(const LUT& lut)
{
	int i;
	m_buff=DEBUG_NEW int[lut.m_maxIndex+1];	//allocate new
	m_maxIndex=lut.m_maxIndex;
	m_maxValue=lut.m_maxValue;

	for(i=0;i<lut.m_maxIndex+1;i++)
	{
		m_buff[i]= const_cast <LUT &> (lut).Item(i);
	}

}


/*!
assignment operator
*/
LUT & LUT::operator=(const LUT& lut)

{
	int i;
	int *array;
	if (this != &lut)
	{
		array=DEBUG_NEW int[m_maxIndex+1];	//allocate new
		for(i=0;i<m_maxIndex+1;i++)
		{
			array[i]= const_cast <LUT &>(lut).Item(i);
		}
		delete m_buff;						//delete original (if assignment)
		m_buff=array;
		m_maxIndex=lut.m_maxIndex;
		m_maxValue=lut.m_maxValue;
	}
	return *this;
}


/**********************************************************************************

errorType LUT::Insert(int index,int value)

***********************************************************************************/

errorType LUT::Replace(int index,int value)
{
    errorType rv=errNone;

    if( (index > m_maxIndex) || (index < 0))
    {
        STL_THROW("LUT bounds error",errBounds);
    }
    if(value <0 || value > m_maxValue)
    {
        return errBounds;
    }
    m_buff[index]=value;
    return rv;
}

/**********************************************************************************

int LUT::Item(int index)

***********************************************************************************/

int LUT:: Item(int index)
{
    if( (index > m_maxIndex) || (index < 0))
    {
		if(index >m_maxIndex)
			index=m_maxIndex;
		if(index<0)
			index=0;

        STL_THROW("LUT bounds error",errBounds);
        //throw(gsException(errBounds,wxString("LUT bounds error")));
    }
    return(m_buff[index]);
}

/*!
	ctor
StartleLUT::StartleLUT()

init data for the Amplitude LUT, 127 possible values, 1-125 dB and max output (127)
0=MUTE (104dB output diconnected from input in LM1972)

0,1,2,3,4,5,6,7,8,9					//0-
10,11,12,13,14,15,16,17,18,19,
20,21,22,23,24,25,26,27,28,29,
30,31,32,33,34,35,36,37,38,39,
40,41,42,43,44,45,46,47,48,49,
50,51,52,53,54,55,56,57,58,59,
60,61,62,63,64,65,66,67,68,69,
70,71,72,73,74,75,76,77,78,79,
80,81,82,83,84,85,86,87,88,89,
90,91,92,93,94,95,96,97,98,99,
100,101,102,103,104,105,106,107,108,109,
110,111,112,113,114,115,116,117,118,119,
120,121,122,123,124,125,126,127				//

M1972
  MSB: LSB
Address Register (Byte 0)
0000 0000 Channel 1
0000 0001 Channel 2
0000 0010 Channel 3
Data Register (Byte 1)
Contents Attenuation Level dB
0000 0000 0.0
0000 0001 0.5
0000 0010 1.0
0000 0011 1.5
: : : : : : :
0001 1110 15.0
0001 1111 15.5
0010 0000 16.0
0010 0001 16.5
0010 0010 17.0
: : : : : : :
0101 1110 47.0
0101 1111 47.5
0110 0000 48.0
0110 0001 49.0
0110 0010 50.0
: : : : : : :
0111 1100 76.0
0111 1101 77.0
0111 1110 78.0
0111 1111 100.0 (Mute)
1000 0000 100.0 (Mute)
: : : : : : :

*/
StartleLUT::StartleLUT(int *init_array)
:LUT(STLMaxLUTIndex+1,STLMaxAmplitudeDB,init_array)
{
#if 0
    int dbLUT[STLMaxDB+1]={
		      127,												 //100dB
				  126, 125, 124, 123, 122, 121, 120, 119, 118,   //70-78dB attenuation LM1972 .5dB steps low-> high volume
              117, 116, 115, 114, 113, 112, 111, 110, 109, 108,   //60-69dB attenuation
              107, 106, 105, 104, 103, 102, 101, 100, 99,  98,	  //50-59dB attenuation
              97,  96,											  //48-49dB attenuation
						95,  94,  93,  92,  91,  90,  89,  88,	  //44-47.5dB
              87,  86,  85,  84,  83,  82,  81,  80,  79,  78,	  //39-43.5dB attenuation
              77,  76,  75,	 74,  73,  72,  71,  70,  69,  68,	  //34-38.5dB attenuation										  //50-48dB attenuation
			  67,  66,  65,  64,  63,  62,  61,  60,  59,  58,	  //29-33.5dB attenuation
			  57,  56,  55,  54,  53,  52,  51,  50,  49,  48,	  //24-28.5dB attenuation
			  47,  46,  45,  44,  43,  42,  41,  40,  39,  38,	  //19-23.5dB attenuation
			  37,  36,  35,  34,  33,  32,  31,  30,  29,  28,	  //14-18.5dB attenuation
			  27,  26,  25,  24,  23,  22,  21,  20,  19,  18,	  //9-13.5dB attenuation
			  17,  16,  15,  14,  13,  12,  11,  10,  9,   8,	  //4-8.5dB attenuation
			  7,   6,   5,   4,   3,   2,   1,   0 				  //0.0-3.5dB
              };
#endif
	if(init_array ==NULL)
	{
		//No initial array to fill LUT so, do fill it with default values
		wxString s;
		for(int i=0;i<STLMaxLUTIndex+1;i++)
		{
			Replace(i,STLMaxLUTIndex-i);
			s.Printf("%i dB",STLMaxLUTIndex-i);
			SetString(i,s);
		}
	}
}

/*!
	dtor
StartleLUT::~StartleLUT()

*/
StartleLUT::~StartleLUT()
{

}

wxString StartleLUT::GetString(int index)
{
	if(index<0)
		index=0;
	if(index > STLMaxLUTIndex)
		index=STLMaxLUTIndex;
	return(m_Text[index]);
}

errorType StartleLUT::SetString(int index,const wxString & string)
{
	if(index < 0 ||	index > STLMaxLUTIndex )
		return errRange;

	m_Text[index]=string;
	return errNone;
}


#if 0
The startle box will not send async records so we should be able to use the default gsSerialPort
and the standard mutex controlled record read function. It will always obtain the lock.
When we Execute(), we can read the ADC data directly from the CQueue. Must make sure the Q is large enough
to hold 4k bytes??

#endif


/**************************************************************
    ctor
StartleRecord::StartleRecord()
**************************************************************/
StartleRecord::StartleRecord()
{
    m_type=0;
    m_flag=0;

}

/**************************************************************
    dtor
StartleRecord~StartleRecord()
**************************************************************/

StartleRecord::~StartleRecord()
{
}

/**********************************************************
errorType StartleRecord::MakeGSIRecord()
**********************************************************/
errorType StartleRecord::MakeGSIRecord()
{

//We MakeGSIRecord which ?? replaces ReturnedRecord.
//So in MakeGSIRecord, we should delete any currently existing ReturnedRecord, otherwise the pointer
//is overwritten and we lose the ability to delete it
//m_Record is an embedded GSIRecord
//GSIRecord ctor sets m_ReturnedRecord to NULL and will delete the pointer it finds there in its dtor
    if(m_Record.GetReturnedRecord())
    {
        delete m_Record.GetReturnedRecord();
        m_Record.SetReturnedRecord(NULL);
    }
    m_Record.SetType(rtUser0);
    m_Record.SetNode(GSI_NODE_THIS);

    m_Record.SetData(0,(GSIChar) m_type);
    m_Record.SetData(1,(GSIChar) m_flag);

/*------ startle box want msb:lsb ---------------------- */
    m_Record.SetData(3,(GSIChar) (m_param0 &0x00ff));   //lsb
    m_Record.SetData(2,(GSIChar) (m_param0 >>8));       //msb

    m_Record.SetData(5,(GSIChar) (m_param1 &0x00ff));
    m_Record.SetData(4,(GSIChar) (m_param1 >>8));

    m_Record.SetData(7,(GSIChar) (m_param2 &0x00ff));
    m_Record.SetData(6,(GSIChar) (m_param2 >>8));

    m_Record.SetData(9,(GSIChar) (m_param3 &0x00ff));
    m_Record.SetData(8,(GSIChar) (m_param3 >>8));

    m_Record.SetLength(10);              //fixed length for these records, caller can modify
    m_Record.SetStrings();

    return(errNone);
}


errorType StartleLUT::Write(wxTextFile *file)
{
wxString s;
wxString eol;
	eol=file->GetEOL();
	file->Clear();
	for(int i=0; i < GetNumElements(); i++)
	{
		s.Printf("%i\t%i\t%s%s",i,Item(i),GetString(i).c_str(),eol.c_str());
		file->AddLine(s);
	}
	file->Write();
	return errNone;
}

/*!
errorType StartleLUT::Load(wxTextFile *file)
*/
errorType StartleLUT::Load(wxTextFile *file)
{
wxString s;
wxString s1;
int c;
long index,value;
errorType e;
errorType rv=errNone;

	s=file->GetFirstLine();

	while(!file->Eof())
	{
		//First line
		//parse the line
		wxStringTokenizer token(s);
		c=token.CountTokens();
		if(c > 1)
		{
			s1=token.GetNextToken();	//get index
			if(s1 != ";")				//comment
			{
				s1.ToLong(&index,10);

				s1=token.GetNextToken();	//get lutValue
				s1.ToLong(&value,10);
				e=Replace(index,value);
				if(e)
					rv=e;
				s1=token.GetString();	//get rest of string
				e=SetString(index,s1);
				if(e)
					rv=e;
			}
		}
		s=file->GetNextLine();
	}

	return rv;
}


#endif //#ifdef STARTLE_SERVER
