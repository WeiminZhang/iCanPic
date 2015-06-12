GSI
Firstly it appears that the last time this project was touched was in 2012 though all meaningful activity stopped about 2006, so I will be hazy on details.
Interested parties would be best off checking the include files and source code to establish accurate details.

GSI refers to GS (my initials) Interface.
Originally planned to be a simple CAN (Controller Area Network) interface allowing the CAN to communicate with other hardware. So a minimum of two nodes was conceived, one attached to a PC through an RS232 interface talking to a 'master' CAN node and the other node being attached to the 'Slave' hardware needing to be controlled.
My use of Master/Slave when referring to the CAN is purely one of convenience as CAN does not use the master/slave model. However it is used conceptually by the software, though is not enforced by either software or hardware.
Essentially, small (<17 bytes) records are used to send data to and from nodes through CAN or RS232.
For example,  node1 could be attached to a pc via rs232. Node2 could be attached to node1. A keypad button on node2 might be pressed. Node2 would place this event on the CAN, node1 would receive and send it to the pc through the rs232. Hence the pc would see the key pressed on node2. PC could then echo the key to node2 by sending an LCD message containing node 2's id, to node1. Node1 would find that it was not the target node for the message so would place the message on the CAN for node2 to receive. Finding it was an LCD message it would write the data to its LCD.


The Microchip PIC 18F458 was used for the CAN controller Other PIC's with CAN would work though might require changes to code.

An Eagle PCB and schematic is included in this archive. The design appears to work as I have a board by the side of me which I am currently using to test some pic software. Earlier, working CAN networks were built on strip boards. I haven't tested the CAN interface on the board but as it is only a couple of connections to the PIC it is unlikely to be faulty.
Also, cheap pic 18458 boards are available (or were the last time I looked) from futurlec.com. I think these are a better alternative to rolling your own unless you have specific requirements.

The code is pic assembler, no c code. The idea was to make the gsi a library with a small enough footprint that user code could make use of pic facilities over a CAN or RS232 port.

The pic code supports the following:
Hitachi HD44780 LCDs. A simple terminal interface is implemented
CAN
RS232
keypads
I2c, limited as this is very specific to the device in use
no spi support though similar caveat to I2C so usually best implemented as required

If you examine canrx.asm you will obtain an idea of what facilities are available.


The PC side of gsi uses a client server socket interface implemented with wxWidgets.
The environment for compiling code as a wxWidgets project can be tricky to setup.
I had working implementations under code::blocks using the gcc g++ compiler. Also several versions of MSVC.
These implementations will almost certainly not work on your pc, so you will certainly need to spend time to get a successful compile.

Proof of software/hardware design was construction of piece of laboratory equipment required to send sine waves or white noise at SPL's from 60dB to 120dB and take measurements with a multiplexed ADC. 
Code with 'Startle' as part of the filename constitute this application. A relatively simple body of code making use of the underlying GSI.

Time has moved on and if I were implementing the same body of code today I would be using an ARM. 

The zip file that includes this readme do not contain the source code for the PC side. This is because, as previously stated, the compile environment is quite complicated. If I can simplify it then I will make it available.
If I don't have time to do the simplification then I will post as-is or send out copies on demand. 
In any case, the c++ code is of little use without the pic code and working hardware. 

The zip file contains an image of the startle project in the file startle1.jpg:

left top is LM1972 volume control/mixer
left bottom is power supply and DDS
centre top is level converter board converting +-12V signals to 0-5V for the pic ADC.
centre middle is the PIC board, a cheap board from Futurlec.com (note, this project didn't use CAN and this board lacks a CAN transceiver)
centre bottom to right bottom a relay board. 120dB requires quite high currents so switched by relays. An external amp takes the sine/white noise signal from the LM1972 board and returns it to the relay board.
top right board is redundant. A previous version used an internal power amp to run the speakers, this is it but it is no longer in use.

Glenn Feb 2014.
