GSI
Firstly it appears that the last time this project was touched 
was in 2012 though all meaningful activity stopped about 2007, 
so I will be hazy on details.
Interested parties would be best off checking the include files 
and source code to establish accurate details.

GSI refers to GS (my initials) Interface.
The C++ and Pic code implement a set of library functions that 
can be accessed via a PC down the serial port and can utilize 
the CAN on the pic to talk to other nodes on the CAN.
The pc-> pic is through a socket interface. This means that you 
can remotely control the pic over the internet. You would do 
this by installing the gsi-server on your pc and the gsi-client 
on the pc attached to the pic and CAN via a serial port. 
Commands sent from the gsi-server will control the pic attached 
to the gsi-client.
Alternatively the gsi-server and client can both be run on the 
same pc attached to the pic CAN via a serial port.

The project was originally planned to be a simple CAN (Controller Area Network) 
interface allowing the CAN to communicate with other hardware. 
So a minimum of two nodes was conceived, one attached to a PC 
through an RS232 interface talking to a 'master' CAN node and 
the other node being attached to the 'Slave' hardware needing to 
be controlled.
My use of Master/Slave when referring to the CAN is purely one 
of convenience as CAN does not use the master/slave model. 
However it is used conceptually by the software, though is not 
enforced by either software or hardware.
Essentially, small (<17 bytes) records are used to send data to 
and from nodes through CAN or RS232.
For example,  node1 could be attached to a pc via rs232. Node2 
could be attached to node1. A keypad button on node2 might be 
pressed. Node2 would place this event on the CAN, node1 would 
receive and send it to the pc through the rs232. Hence the pc 
would see the key pressed on node2. PC could then echo the key 
to node2 by sending an LCD message containing node 2's id, to 
node1. Node1 would find that it was not the target node for the 
message so would place the message on the CAN for node2 to 
receive. Finding it was an LCD message directed at it, it would write the data 
to its LCD.


The Microchip PIC 18F458 was used for the CAN controller Other 
PIC's with CAN would work though might require changes to code.
I also began an ARM LPC2118 version but was sidetracked before 
this could be completed.

An Eagle PCB and schematic is included in this archive. The 
design appears to work as I have a board by the side of me which 
I am currently using to test some pic software. Earlier, working 
CAN networks were built on strip boards. I haven't tested the 
CAN interface on the pcb in this release but as it is only a 
couple of 
connections to the PIC it is unlikely to be faulty.
Also, cheap pic 18458 boards are available (or were the last 
time I looked) from futurlec.com. I think these are a better 
alternative to rolling your own unless you have specific 
requirements.

The code is pic assembler, no c code. The idea was to make the 
gsi a library with a small enough footprint that user code could 
make use of pic facilities over a CAN or RS232 port.

The pic code supports the following:
Hitachi HD44780 LCDs. A simple terminal interface is implemented
CAN
RS232
keypads
I2c, limited as this is very specific to the device in use
no spi support though similar caveat to I2C so usually best 
implemented as required

If you examine canrx.asm you will obtain an idea of what 
facilities are available.


The PC side of gsi uses a client server socket interface 
implemented with wxWidgets.
The environment for compiling code as a wxWidgets project can be 
tricky to setup.
I have working implementations under code::blocks using the gcc 
g++ compiler and mingw. Also several versions of MSVC and an 
eclipse CDT 
version.
These implementations will almost certainly require a bit of 
fiddling before you get a successful compile. I have compile 
instruction for Eclipse later in this file.

Proof of software/hardware design was construction of a piece of 
laboratory equipment required to send sine waves or white noise 
at SPL's from 60dB to 120dB and take measurements with a 
multiplexed ADC. 
Code with 'Startle' as part of the filename constitute this 
application. A relatively simple body of code making use of the 
underlying GSI.

The zip file contains an image of the startle project in the 
file startle1.jpg:
left top is LM1972 volume control/mixer
left bottom is power supply and DDS
centre top is level converter board converting +-12V signals to 
0-5V for the pic ADC.
centre middle is the PIC board, a cheap board from Futurlec.com 
(note, this project didn't use CAN and this board lacks a CAN 
transceiver)
centre bottom to right bottom a relay board. 120dB requires 
quite high currents so switched by relays. An external amp takes 
the sine/white noise signal from the LM1972 board and returns it 
to the relay board.
top right board is redundant. A previous version used an 
internal power amp to run the speakers, this is it but it is no 
longer in use.

Included in the documentation is a list of libraries needed for 
a compile.
The executables found in DEBUG directory will run on Win7.
They were compiled on Win 7 using:
boost 1.55.0
wxWidgets ver 2.8.12
compiled with GCC/G++ 4.6.1
Under Eclipse Helios CDT
wxFormBuilder was used for a number of the dialogs

The gsi-server will need to be allowed through the firewall or it 
will be unable to communicate with the client. 
If you run both pieces of software you should be able to connect 
to the server by clicking on <Socket OpenSession>
If you have a pic board with the pic gsi library running, then 
opening <Node OpenNodeOnDevice> you should establish comms with 
the board. The gsi-client program as it stands does very little. The 
Startle demonstration is a fully functional application. It will 
exercise the code for you but without the hardware attached to 
the pic will be non-functional.
It should serve as a demonstration of how to use the underlying 
GSI on both the pic and in C++.


Finally, the directory tree you see on upacking is that used to 
compile the two executables in gsi-client/debug and 
gsi-server/debug respectively.

		Building
If you create two eclipse C++ cdt projects. Make one called 
'server' in director gsi-server and the other called 'client' in 
directory gsi-client. Then for each of these projects create a 
folder <File New Folder> Choose the Advanced>> button and select 
the Link to alternate location (linked folder) radio button, 
then navigate to 
the gsi-common folder and select it as the folder.
This will place the files in the gsi-common folder in the build 
for both the server and client
You might need to right click the project and select 'refresh' 
before it will compile.

Its quite likely that I have missed out a few files in this 
first distribution as on my pc they are a little more scattered 
than in this version. If you find any missing files, please let 
me know and I will add them.
 
Glenn Aug 2014.
