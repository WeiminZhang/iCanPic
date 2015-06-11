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
#ifndef STARTLEDEFS_H_INCLUDED
#define STARTLEDEFS_H_INCLUDED

#include "gsidefs.h"

#define _CRTDBG_MAP_ALLOC

#define STL_VERSION_1		1
#define STL_VERSION_2		2

#define STL_VERSION STL_VERSION_2

#define STL_DEBUG_USING_XXX 1

#define STL_USE_EXCEPTIONS 1

#if STL_USE_EXCEPTIONS
    #define STL_THROW(wxStr,err) throw(gsException(err,wxString(wxStr)));
#else
    #define STL_THROW(wxStr,err);
#endif

const	char STLDefaultSineLUTFname[]="Sine.lut";
const	char STLDefaultNoiseLUTFname[]="WhiteNoise.lut";

const	int STLFirstId	=GSIFirstUserId+500;
const	int STL_DDS_CLOCK =30;				//AMCQ=30 MHz

enum
{
	STL_STARTLE_TEST1=STLFirstId,
	STL_STARTLE_TEST2,
	STL_STARTLE_TEST3,
	STL_STARTLE_LUT_HELPER,
	STL_STARTLE_LUT_LOAD,
};

const int STLMaxNumChans=8;

/*-------- RS232 --------------------------------------*/
const int STLMaxNumComPorts=6;

/*---------STL record (fixed length wxInt16's) ---------------------*/
const int STLMaxNumParameters=6;            //send fixed number of parameters through socket
const int STLRecordArraySize=STLMaxNumParameters+3; //6+id+type+flag


/*----------- Queue/Immediate flag --------*/
enum STLQueuedFlag
{
STL_QUEUED=0,
STL_IMMEDIATE=1
};
/*--------- Reset ----------------------*/
//const wxInt16 STLUncondtionalReset=0;
//const wxInt16 STLAllowCompletionReset=1;

/*-------Ampliitude--------------*/
const wxInt16 STLMaxAmplitudeDAC       =1023; //also used for LUT
const wxInt16 STLMaxNoiseAmplitudeDAC  =1023;  //also used for LUT
const wxInt16 STLMaxAmplitudeDB       =127; //also used for LUT
const wxInt16 STLMaxNoiseAmplitudeDB  =127;  //also used for LUT
const wxInt16 STLMaxLUTIndex		  =127;  //0=max vol 127=max volume
const wxInt16 STLMaxLM1972Value		  =127;

const wxInt16 STLMaxDB              =127;   //max dB
const wxInt16 STLMaxNoiseDB         =127;   //max dB
const wxInt16 STLMinDB				=0;
const wxInt16 STLMinNoiseDB			=0;


/*-------- Frequency ----------------*/
//const wxInt16 STLMaxFrequencyDAC    =1023;
//const wxInt16 STLMaxFrequencyDAC    =1023;
//const double STLFrequencySlope       = 31.09848;
//const double STLFrequencyIntercept   =282.5468;
const double STLMinFrequency         = 0;
const double STLMaxFrequency          =65535;		//limitation of using wxUint16 for parameter (can change if reqd)

/*-----------Channel------------------*/
const wxInt16 STLMinChan					= 0;
const wxInt16 STLMaxChan					=7;
const wxInt16 STLNumChans					=8;


/*--------- ADC ----------------- */
const wxInt16 STLMaxADCArraySize=20000;
const wxInt16 STLMaxADCDataPoints=STLMaxADCArraySize/sizeof(wxInt16);
const int       STLADCImmediateTimeout=5000;    //timeout of 5 secs for immediate adc
const int       STLADCQueuedTimeout=15000;      //timeout of 15 secs for queued adc

/*-------- Status bits ------------*/
//lsb
const   wxUint16 STLStatusADCDataReady	=0x1;       //bit 0
const   wxUint16 STLStatusADCAcquiring	=0x2;
const   wxUint16 STLStatusADCEnd         =0x4;
const   wxUint16 STLStatusADCQueued      =0x8;
const   wxUint16 STLStatusExecuteAt      =0x10;
const   wxUint16 STLStatusToneStart      =0x20;       //a tone is on
const   wxUint16 STLStatusBit6           =0x40;
const   wxUint16 STLStatusErrorR2Overflow=0x80;
//msb
const   wxUint16 STLStatusTimedTone       =0x100;   //a timed tone is on

/*--------- Socket -----------------*/
//const size_t STLMaxSocketData=STLMaxADCArraySize+2;     //allow a coupe of extra bytes for future status
const size_t STLMaxSocketData=STLMaxADCArraySize+2;     //allow a coupe of extra bytes for future status
//----- commands -------------------------
const bool  STLExternal=0;                               //default for commands, most are from the client
const bool  STLInternal=1;

const bool  STLToneOff=0;
const bool  STLToneOn=1;

enum STLRecOffset
{
STL_REC_OFFSET_TYPE=0,		//offset within GSIRecord for these fields
STL_REC_OFFSET_FLAG=1,
STL_REC_OFFSET_P0_MSB=2,		//offset to parameter0 (msb, so msb=offset2 : lsb=offset3)
STL_REC_OFFSET_P0_LSB=3,
STL_REC_OFFSET_P1_MSB=4,		//offset to parameter1
STL_REC_OFFSET_P1_LSB=5,
STL_REC_OFFSET_P2_MSB=6,		//offset to parameter2
STL_REC_OFFSET_P2_LSB=7,
STL_REC_OFFSET_P3_MSB=8,		//offset to parameter3
STL_REC_OFFSET_P3_LSB=9,
STL_REC_OFFSET_P4_MSB=10,		//offset to parameter4
STL_REC_OFFSET_P4_LSB=11
};

enum STLPrescaler
{
PRESCALER2=2,
PRESCALER4=4,
PRESCALER8=8,
PRESCALER16=16,
PRESCALER32=32,
PRESCALER64=64,
PRESCALER128=128,
PRESCALER256=256
};

#if STL_VERSION == STL_VERSION_1
enum STLEnumCommand
{
STL_COMMAND_RESET=0,                      //Use WaitForCompletion if startle box is running, this will stop all running tasks in box
STL_COMMAND_SET_AMPLITUDE=1,               //Set the Amplitude DAC, p0=0-STLMaxAmplitudeDAC
STL_COMMAND_SET_FREQUENCY=2,				//used in conjuction with STL_COMMAND_SET_FREQUENCY_UPPER for DDS version
STL_COMMAND_SET_DELAY=3,
STL_COMMAND_ACQUIRE=4,
STL_COMMAND_SET_CHANNEL=5,
STL_COMMAND_EXECUTE=6,
STL_COMMAND_TONE=7,                       //p0=
STL_COMMAND_TIMED_TONE_ON=8,                //p0=period,p1=prescaler,p2=at_tick
STL_COMMAND_SET_NOISE_AMPLITUDE=9,          //Set the Noise DAC, p0=0-STLMaxNoiseAmplitudeDAC
STL_COMMAND_GET_STATUS=10,                 //Used internally. See StatusBits if you really want to use it
STL_COMMAND_SET_FREQUENCY_UPPER=11,			//V2 specific
//reserve 10 commands for more external (outbound to controller) commands


//Start of internal commands. Commands the server will handle or fiddle with
STL_COMMAND_WAIT_FOR_COMPLETION=21,          //po=timeout (max 32767) in msec. Waits till the startle box has finished all its tasks. Uses the outbound GetStatus()
STL_COMMAND_OPEN_DEVICE=22,                  //open the comport, p0=comport number (1-6)

STL_COMMAND_SET_AMPLITUDE_SPL=23,           //p0=dB 1-126, 126 is max output from amp
STL_COMMAND_SET_NOISE_AMPLITUDE_SPL=24,      //p0=dB 1-126, 126 is max output from amp
STL_COMMAND_SET_FREQUENCY_HZ=25,            //p0=f (STLMinFrequency-STLMaxFrequency)

STL_COMMAND_SET_AMPLITUDE_LUT=26,           //set an element of the LUT p0=index, p1=value
STL_COMMAND_SET_NOISE_LUT=27,                //set an element of the LUT p0=index, p1=value
STL_COMMAND_CLOSE_DEVICE=28

};

#endif

#if STL_VERSION == STL_VERSION_2
enum STLEnumCommand
{
STL_COMMAND_RESET=0,                      //Use WaitForCompletion if startle box is running, this will stop all running tasks in box
STL_COMMAND_SET_AMPLITUDE=1,               //Set the Amplitude DAC, p0=0-STLMaxAmplitudeDAC
STL_COMMAND_SET_FREQUENCY=2,				//used in conjuction with STL_COMMAND_SET_FREQUENCY_UPPER for DDS version
//STL_COMMAND_SET_DELAY=3,
STL_COMMAND_ACQUIRE=4,
STL_COMMAND_SET_CHANNEL=5,
STL_COMMAND_EXECUTE=6,
//STL_COMMAND_TONE=7,                       //p0=
//STL_COMMAND_TIMED_TONE_ON=8,                //p0=period,p1=prescaler,p2=at_tick
STL_COMMAND_SET_NOISE_AMPLITUDE=9,          //Set the Noise DAC, p0=0-STLMaxNoiseAmplitudeDAC
STL_COMMAND_GET_STATUS=10,                 //Used internally. See StatusBits if you really want to use it
STL_COMMAND_SET_FREQUENCY_UPPER=11,			//V2 specific
//reserve 10 commands for more external (outbound to controller) commands


//Start of internal commands. Commands the server will handle or fiddle with
STL_COMMAND_WAIT_FOR_COMPLETION=21,          //po=timeout (max 32767) in msec. Waits till the startle box has finished all its tasks. Uses the outbound GetStatus()
STL_COMMAND_OPEN_DEVICE=22,                  //open the comport, p0=comport number (1-6)

STL_COMMAND_SET_AMPLITUDE_SPL=23,           //p0=dB 1-126, 126 is max output from amp
STL_COMMAND_SET_NOISE_AMPLITUDE_SPL=24,      //p0=dB 1-126, 126 is max output from amp
//STL_COMMAND_SET_FREQUENCY_HZ=25,            //p0=f (STLMinFrequency-STLMaxFrequency)

STL_COMMAND_SET_AMPLITUDE_LUT_VALUE=26,           //set an element of the LUT p0=index, p1=value
STL_COMMAND_SET_NOISE_AMPLITUDE_LUT_VALUE=27,                //set an element of the LUT p0=index, p1=value
STL_COMMAND_CLOSE_DEVICE=28

};

#endif



#if 0
;----------------Commands ------------------------------
;data[0]=STL_COMMAND_XXX
STL_COMMAND_RESET		EQU	0
;	data[0]=0
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=STL_RESET_FLAG (UNCONDITIONAL\COMPLETION)
;	data[3]=unused
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

STL_COMMAND_SET_AMPLITUDE	EQU	1
;	data[0]=1
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb amplitude
;	data[3]=lsb amplitude
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

STL_COMMAND_SET_FREQUENCY	EQU	2
;	data[0]=2
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[3]=msb frequency
;	data[3]=lsb frequency
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

STL_COMMAND_SET_DELAY		EQU	3
;	data[0]=3
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb TICK (TICK=10us?)
;	data[3]=lsb TICK
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

STL_COMMAND_ACQUIRE		EQU	4
;	data[0]=4
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2] msb number data points
;	data[3] lsb number data points
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

STL_COMMAND_SET_CHANNEL		EQU	5
;	data[0]=5
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=channel 0-7
;	data[3]=msb tick to execute on (if QUEUED)
;	data[4]=lsb tick to execute on (if QUEUED)

STL_COMMAND_EXECUTE		EQU	6
;	data[0]=6
;		always IMMEDIATE

STL_COMMAND_TONE		EQU	7
;	data[0]=7
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=ON/OFF; OFF=0 ON=1
;	data[3]=msb tick to execute on (if QUEUED)
;	data[4]=lsb tick to execute on (if QUEUED)

STL_COMMAND_TIMED_TONE_ON		EQU	8
;	data[0]=8
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb
;	data[3]=lsb
;	data[4]=prescaler
;	data[5]=msb tick to execute on (if QUEUED)
;	data[6]=lsb tick to execute on (if QUEUED)

STL_COMMAND_SET_NOISE_AMPLITUDE	EQU	9
;	data[0]=9
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb amplitude
;	data[3]=lsb amplitude
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

STL_COMMAND_GET_STATUS	EQU	10
;Always immediate
;	data[0]=10
;	data[1]=QUEUE=0; IMMEDIATE=1
;	data[2]=msb amplitude
;	data[3]=lsb amplitude
;	data[4]=msb tick to execute on (if QUEUED)
;	data[5]=lsb tick to execute on (if QUEUED)

#endif



#endif
