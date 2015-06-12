;DeviceNet defines

#if 0
;Error Code (hex) Meaning

00xx Error Reset or No Error
10xx Generic Error
20xx Current
21xx Current, device input side
22xx Current inside the device
23xx Current, device output side
30xx Voltage
31xx Mains Voltage
32xx Voltage inside the device
33xx Output Voltage
40xx Temperature
41xx Ambient Temperature
42xx Device Temperature
50xx Device Hardware
60xx Device Software
61xx Internal Software
62xx User Software
63xx Data Set
70xx Additional Modules
80xx Monitoring
81xx Communication
90xx External Error
F0xx Additional Functions
FFxx Device specific


Byte 0 1 2 3 4 5 6 7
byte 0,1 =Error code (see above)
byte 2   =Error register (object 1001H) ...don't know what this is!
byte 3-7 =Manufacturer specific error data 


#endif
