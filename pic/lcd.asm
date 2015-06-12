;******************************************************************************
;* @file    lcd.asm part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;lcd.asm
#define	IN_LCD.ASM

;#include <G:\prog\mplab\MPLAB IDE\MCHIP_Tools\p18f458.inc>
;#include "p18xxx8.inc"
#include "GSI.inc"

;#IF DEVICE == _18F458
;#include "p18xxx8.inc"
;#ELSE
;#INCLUDE "P18F4680.inc"
;#ENDIF


#include "lcd.inc"
#include "keypad.inc"
#include "rs232.inc"

;-----------------------------------
;	Exported from this module
;-----------------------------------
	#IF GSI_LCD_SUPPORT
	GLOBAL	LdBTemp,InitLCDVrbls
	#ENDIF
	GLOBAL GSINull,GSIBeginningOfLine,GSICursorLeft,GSIUsrAbrt,GSIDeleteCharFwd
	GLOBAL GSIEndOfLine,GSICursorRight,GSIR2ReqMaster,GSIBS,GSIHTab
	GLOBAL GSICursorDown,GSIEnd,GSICls,GSICR,GSICursorDown,GSIEchoToggle	
	GLOBAL GSICursorUp,GSIXOn,GSIBinaryMode,GSIXOff,GSICursorRight5,GSIAsciiMode
	GLOBAL GSICursorLeft5,GSICursorLeft10,GSICRKeypadToggleState,GSIKeypadToggleMode,GSICtrZ
	GLOBAL GSIVersionString,GSICapabilities,GSIGetBuffer,GSISetBuffer
	GLOBAL GSIInstruction,GSIGetCurToEol,GSIGetLine,GSISetLine
	GLOBAL GSIHome,GSIFlowControlOn,GSIFlowControlOff,GSILCDInit
	GLOBAL GSIGetButtonState
	
	

;---------------------------------
;	definition in	user code
;----------------------------------

;-----------------------------------
;	definitions in rs232.asm
;-----------------------------------
	EXTERN BinaryProcessRxByte
	EXTERN	GlbSIOMode,GlbSIOMode1,GlbSIOStatus,GlbSIOStatus1
;----------------------------------------------------

	EXTERN	GlbKeypadStatus	,GlbKeypadKeyDown1,GlbKeypadKeyDown2
	EXTERN	PgmMssgTbl,GlbCompOffset
	EXTERN	GlbLocalTempL,GlbLocalTempH
;	data

	#IF GSI_LCD_SUPPORT
	GLOBAL LdTempArray
;----------------------------
	GLOBAL	LdLCDRow,LdLCDCol

;LCD_DATA is currently same as GSI_DATA so no bank selection required on init
LCD_DATA UDATA 
LdBank			RES 0
LdTemp 			RES 1
LdBTemp 		RES 1
LdBTemp1 		RES 1
LdLocalTempL 		RES 1
LdLocalTempH 		RES 1
LdLCDTemp 		RES 1
LdLCDWriteTemp 		RES 1
LdLCDCounter 		RES 1
LdLCDCounter1 		RES 1
LdLCDControl 		RES 1
LdLCDRow 		RES 1
LdLCDCol 		RES 1
LdAsciiTemp		RES 1
LdTempArray			RES	3		;big enough for an ascii conversion buff

LCD_GLOBAL_DATA	UDATA_ACS

	#ENDIF ;GSI_LCD_SUPPORT

	;in GSI.ASM
	;EXTERN GSICommonReturn

LCD	CODE
CopyrightText	DATA	"Copyright Glenn Self 2003",0

;-----------------------------
;	LCDInit
;Use Hitachi reset algorithm
;Assumes GSI_BANK, which is the same as LCD_DATA
;-----------------------------
LCDInit
	IF GSI_LCD_SUPPORT
	IF (SIMULATOR == FALSE)
	call	GSIDelay50ms
	call	GSIDelay50ms
	;-
	clrf	LdLCDControl		;rs=rw=e=0
	movlw  	0x03                   	;Reset Command
	call   	LCDWriteNybble         	;Send the Nybble
	call	GSIDelay5ms		;Wait 5 msecs before Sending Again
	LCD_E_STROBE			;2nd reset
	call	GSIDelay160us		;Wait 160 usecs before Sending the second Time
	LCD_E_STROBE			;3rd reset
	call	GSIDelay160us		;Wait 160 usecs before Sending the Third Time
	;------------------
	movlw	0x02			;Set 4 Bit Mode (37us)
	call   	LCDWriteNybble              
	;These delays are probably not reqd, or rather a 1us delay is reqd
	call	GSIDelay160us		

	LCD_E_STROBE			;28h:2 line display
	call	GSIDelay160us
	movlw	0x08			
	call	LCDWriteNybble
	call	GSIDelay160us

	movlw	0			;08:display off, no cursor,no blink
	call	LCDWriteNybble
	call	GSIDelay160us
	movlw	8
	call	LCDWriteNybble	
	call	GSIDelay160us

	movlw	0			;01:cls
	call	LCDWriteNybble
	call	GSIDelay160us
	movlw	1
	call	LCDWriteNybble
	call	GSIDelay1.53ms

	movlw	0			;06,entry mode, cursor blink inc ddram move right
	call	LCDWriteNybble
	call	GSIDelay160us
	movlw	6
	call	LCDWriteNybble
	call	GSIDelay160us

	clrf	LdLCDRow			;0,0
	clrf	LdLCDCol

    call    GSISetCursor        ;added 18/6/2012

	ENDIF	;SIMULATOR
	ENDIF	;GSI_LCD_SUPPORT
	return

;-----------------------------
;	GSILCDInstruction
;Assumes GSI_BANK
;Requires instruction in WREG
;-----------------------------
GSILCDInstruction				;Send the Instruction to the LCD
	IF GSI_LCD_SUPPORT
	IF (SIMULATOR == FALSE)
	movwf	LdLCDTemp			;Save the Temporary Value

	swapf	LdLCDTemp, w		;Send the High Nybble
	bcf	LdLCDControl,LCD_CTR_RS
	call	LCDWriteNybble

	movf	LdLCDTemp,W		;Send the Low Nybble
	call	LCDWriteNybble

	call	GSIDelay39us

	ENDIF	;SIMULATOR
	ENDIF	;GSI_LCD_SUPPORT
	return



;---------------------------------------------------------
;	LCDWriteNybble
;
;LdLCDControl holds E,RW,RS
;Assumes GSI_BANK
;---------------------------------------------------------

	IF GSI_LCD_SUPPORT
LCDWriteNybble                ; Send a Nybble to the LCD
	movwf	LdLCDWriteTemp		;Save the Nybble to Shift Out
	swapf	LdLCDWriteTemp,F		;switch nybble to top half
	movlw	b'00001111'
	andwf	PORTD,F,A
	;set up to or in the nybble
	movlw	b'11110000'
	andwf	LdLCDWriteTemp,W		;bottom nybble==0
	iorwf	LCD_DATA_PORT,F,A	;don't modify bottom nybble of port
	;Data is being asserted on the port
	;set the control lines
	;bcf	LdLCDControl,LCD_CTR_E	;ensure E is low
	;can just copy as only 3 bits implemented
	movff	LdLCDControl,LCD_CTR_PORT
	;reqLdre tsetup of 80ns before E asserted
	;movff already inserts one nop, so only 1 extra nop req'd for 40MHz device
	;control lines are being asserted (RS =x, RW=x, E=0)
	DELAY_80_NS
    LCD_E_STROBE

	return
	ENDIF	;GSI_LCD_SUPPORT



;------------------------------------
;	LCDWriteChar
;Assumes GSI_BANK
;Write char in WREG to LCD
;Preserves WREG
;------------------------------------
	#IF GSI_LCD_SUPPORT
LCDWriteChar				;Send the Character to the LCD
	
	movwf	LdLCDTemp		;Save the Temporary Value

	swapf	LdLCDTemp, w		;Send the High Nybble
	bsf	LdLCDControl,LCD_CTR_RS	;RS = 1, data
	bcf	LdLCDControl,LCD_CTR_RW	;rw=0, write
	call	LCDWriteNybble

	movf	LdLCDTemp, w		;Send Low Nybble
	call	LCDWriteNybble

	call	GSIDelay43us
	movf	LdLCDTemp,W

	return
	#ENDIF ;GSI_LCD_SUPPORT

GSINull			;0
	;goto	GSICommonReturn
	return

;-------------------------------------
;	Begining of line
;-------------------------------------
GSIBeginningOfLine		;ctr-a, 1
	#IF GSI_LCD_SUPPORT
	clrf	LdLCDCol			;column 0
	#ENDIF ;GSI_LCD_SUPPORT

	goto	GSISetCursor		;exits to GSICommonReturn
	;on line 1 or 3 so FF

;--------------------------------------
;	cursor left
;--------------------------------------
GSICursorLeft			;ctr-b,2
	#IF GSI_LCD_SUPPORT
	decf	LdLCDCol,F
	btfsc	LdLCDCol,7	;check for ovf
	incf	LdLCDCol,F	;ovf, restore
	goto	GSISetCursor
	#ENDIF	;GSI_LCD_SUPPORT

GSIUsrAbrt			;ctr-c
;no ints as we use LdStatusTemp to store STATUS at reset.
;This allows are to check for WDT reset. If an int ocurred
;after we had Set LdStatusTemp we would lose the value
	bcf	INTCON,GIEL	;disallow ints as we use LdStatusTemp
	btfsc	INTCON,GIEL
	bra	GSIUsrAbrt
_di_h
	bcf	INTCON,GIEH
	btfsc	INTCON,GIEH
	bra	_di_h

	bsf	GlbSIOStatus,SIO_USR_ABRT
	;goto	GSICommonReturn
	return
GSIDeleteCharFwd		;ctr-d,4
	;goto	GSICommonReturn
	return
GSIEndOfLine			;ctr-e,5	difficult as we don't know where the last char is! 
	;goto	GSICommonReturn
	return


;------------------------------------
;	cursor right
;------------------------------------
GSICursorRight			;ctr-f,6
	#IF GSI_LCD_SUPPORT

	incf	LdLCDCol,W
	sublw	LCD_COLS-1	
	btfsc	STATUS,C,A
	incf	LdLCDCol,F	;no ovf
	#ENDIF ;GSI_LCD_SUPPORT

	goto	GSISetCursor
;Sender wants to be rs232 master. This code has received it
;So this means this code must set itself as the SLAVE
;When received the pc will Tx, and the node will echo (default)
;When sent. This node Tx's and the pc echoes.
GSIR2ReqMaster			;ctr-g,7
	call	GSIRS232SetSlave	;received so make this node slave
	;goto	GSICommonReturn
	return

GSIBS:				;ctr-h,8
	#IF GSI_LCD_SUPPORT
	movlw	8
	;call	GSIEchoByte232
	;cursor left
	decf	LdLCDCol,F
	btfsc	LdLCDCol,7	;check for ovf
	incf	LdLCDCol,F	;ovf, restore
	;inline GSISetCursor
	call	GSISetModule
	clrf	WREG		;assume line 0
	btfsc	LdLCDRow,0	;set== 2nd line
	movlw	0x40		;2nd line
	addwf	LdLCDCol,W	;New DDRAM Address
	LCD_DDRAM_ADDRESS
	;end of inline GSISetCursor

	movlw	' '
	call	LCDWriteChar

	LCD_CURSOR_LEFT
	#ENDIF ;GSI_LCD_SUPPORT

	;goto	GSICommonReturn
	return

GSIHTab			;ctr-i,9
	#IF GSI_LCD_SUPPORT
	movlw	9
	;call	GSIEchoByte232
	#ENDIF ;GSI_LCD_SUPPORT
	;goto	GSICommonReturn
	return

;----------------------------------------
;	line feed
;----------------------------------------
GSILF				;ctr-j,10
	;a place holder, jump table calls CursorDown
GSIEnd			;ctr-k,11
	;END. Set cursor to last line last col
	#IF GSI_LCD_SUPPORT
	
	movlw	LCD_LINES-1
	movwf	LdLCDRow
	movlw	LCD_COLS-1
	movwf	LdLCDCol
	#ENDIF ;GSI_LCD_SUPPORT
	goto	GSISetCursor

;--------------------------------------
;		Cls
;--------------------------------------
GSICls				;ctr-l,12
	#IF GSI_LCD_SUPPORT

	IF LCD_LINES == 4
	SLCT_LCD	1
	call	GSIDelay160us
	LCD_CLS
	SLCT_LCD 	0
	call	GSIDelay160us
	LCD_CLS
	ELSE	;LCD_LINES
	LCD_CLS
	ENDIF	;LCD_LINES

	clrf	LdLCDCol			;reset vrbls
	clrf	LdLCDRow
	#ENDIF ;GSI_LCD_SUPPORT

	goto	GSISetCursor

;------------------------------------
;	Carriage return
;------------------------------------
GSICR				;ctr-m,13 carriage return
	#IF GSI_LCD_SUPPORT

	movlw	0x0d
	;call	GSIEchoByte232
	clrf	LdLCDCol		;col 0
	#ENDIF ;GSI_LCD_SUPPORT
	;allow to fall through to cursor down
;-------------------------------
;	GSICursorDown
;------------------------------
GSICursorDown				;ctr-n,14
	#IF GSI_LCD_SUPPORT
	movlw	0x0a
	;call	GSIEchoByte232	;echo lineFeed to rs232
	incf	LdLCDRow,W	;bump row counter, store in WREG
	sublw	LCD_LINES-1	;do the test
	btfsc	STATUS,C	;already on bottom line?
	incf	LdLCDRow,F	;not on bottom line
	#ENDIF ;GSI_LCD_SUPPORT
	goto	GSISetCursor

;----------------------------------------
;	toggle echo to rs232
;----------------------------------------
GSIEchoToggle
	movlw	1 << SIO_M1_ECHO
	xorwf	GlbSIOMode1,F
	movlw	RT_ECHO_TOGGLE_ASC
	btfss	GlbSIOMode,SIO_M_BINARY
	call	GSISendByte232				;echo the byte (if in ascii mode) to let user know we are alive
	;goto	GSICommonReturn
	return
;--------------------------------
;	Cursor Up
;--------------------------------
GSICursorUp			;ctr-p,16	(Down is a LF)
	#IF GSI_LCD_SUPPORT
	movf	LdLCDRow,F	
	btfsc	STATUS,Z,A
	;goto	GSICommonReturn	;already at top line
	return	
	decf	LdLCDRow,F
	#ENDIF ;GSI_LCD_SUPPORT
	goto	GSISetCursor		;using LdLCDRow,Col

GSIXOn				;ctr-q,17 XOn
	;goto	GSICommonReturn
	return

GSIBinaryMode			;ctr-r,18
	#IF GSI_LCD_SUPPORT
	movlw	0x40+LCD_COLS-1
	LCD_DDRAM_ADDRESS
	movlw	'B'
	call	LCDWriteChar
	#ENDIF

	bsf	GlbSIOMode,SIO_M_BINARY
	bcf	GlbSIOStatus1,SIO_S1_MASTER	;implicit that on setting binary the pc is master and node is slave
	call	GSIRS232SetSlave

GSIXOff			;ctr-s,19	XOff
	;goto	GSICommonReturn
	return

;----------------------------------
;	cursor right 5 chars
;----------------------------------
GSICursorRight5			;ctr-t,20
	#IF GSI_LCD_SUPPORT
	movlw	5
	addwf	LdLCDCol,W
	sublw	LCD_COLS
	btfss	STATUS,C,A
	goto	scr5_ovf		;ovf
	movlw	5
	addwf	LdLCDCol,F		;no ovf
scr5_1
	#ENDIF ;GSI_LCD_SUPPORT

	goto	GSISetCursor

	#IF GSI_LCD_SUPPORT
scr5_ovf
	movlw	LCD_COLS-1		;last pos
	movwf	LdLCDCol
	goto	scr5_1
	#ENDIF ;GSI_LCD_SUPPORT

;-----------------------------------------------
;	Set Ascci Mode (see Set Binary Mode,18)
;-----------------------------------------------
GSIAsciiMode			;ctr-u,21
	call	GSISetRS232AsciiMode
	goto	GSISetCursor

GSICursorLeft5			;ctr-v,22	
	;goto	GSICommonReturn
	return
GSICursorLeft10		;ctr-w,23
	;goto	GSICommonReturn	
	return

GSICRKeypadToggleState			;ctr-x,24	
	#IF GSI_KEYPAD_SUPPORT
	movlw	1 << KEY_ENABLED
	xorwf	GlbKeypadStatus,F,A	;in access bank
	#ENDIF ;GSI_KEYPAD_SUPPORT
	;goto	GSICommonReturn 	]
	return
GSIKeypadToggleMode			;ctr-y,25
	;toggle between raw and cooked
	#IF GSI_KEYPAD_SUPPORT
	movlw	1 << KEY_RAW
	xorwf	GlbKeypadStatus,F,A
	#ENDIF ;GSI_KEYPAD_ENABLED

	;goto	GSICommonReturn 
	return
GSICtrZ			;ctr-z,26
	;goto	GSICommonReturn
	;Use to check for a wdt reset
	goto	GSICtrZ


;-----------------------
;	esc codes
;-----------------------
GSIVersionString
	;goto	GSICommonReturn
	return
GSICapabilities
	;goto	GSICommonReturn
	return
GSIGetBuffer
	;goto	GSICommonReturn
	return
GSISetBuffer
	;goto	GSICommonReturn
	return
;GSICls	Duplicated as a control code
;	return
GSIInstruction
	;goto	GSICommonReturn
	return
GSIGetCurToEol
	;goto	GSICommonReturn
	return
GSIGetLine
	;goto	GSICommonReturn
	return
GSISetLine
	;goto	GSICommonReturn
	return
GSIHome
	#IF GSI_LCD_SUPPORT

	clrf	LdLCDCol
	clrf	LdLCDRow
	#ENDIF ;GSI_LCD_SUPPORT

	goto	GSISetCursor

GSIFlowControlOn
	;goto	GSICommonReturn
	return
GSIFlowControlOff
	;goto	GSICommonReturn
	return


;---------------------------------------------------------
;	GSIInit
;ASSUMES GSI_BANK
;Init variables, LCD(s)
;---------------------------------------------------------
GSILCDInit
	#IF GSI_LCD_SUPPORT

	call	InitLCDVrbls
	IF	LCD_LINES > 2
		SLCT_LCD	1
		call	LCDInit
		call	GSICls
		SLCT_LCD	0
	ENDIF	;LCD_LINES

	call	LCDInit
	call	GSICls
	#ENDIF ;GSI_LCD_SUPPORT

	;goto	GSICommonReturn
	return

GSIGetButtonState
	#IF GSI_KEYPAD_SUPPORT

	movf	GlbKeypadKeyDown1,W,A
	call	GSISendByte232
	movf	GlbKeypadKeyDown2,W,A
	call	GSISendByte232	
	#ENDIF ;GSI_KEYPAD_SUPPORT

	;goto	GSICommonReturn
	return


#IF GSI_LCD_SUPPORT
;---------------------------
;LCDSetModule
;--------------------------
GSISetModule

	LCD_CURSOR_OFF
	SLCT_LCD	1
	call	GSIDelay160us	;give line time to settle
	movf	LdLCDRow,W
	sublw	1		;1 not 2, as zero condition is NC
	btfss	STATUS,C,A	;if set, use lcd module 1
	goto	ssm1
	SLCT_LCD	0
ssm1
	call	GSIDelay160us	;give line time to settle
	LCD_CURSOR_ON
	return
	ENDIF ;GSI_LCD_SUPPORT

;----------------------------------------
;	GSISetCursor
;using LdLCDRow,Cursor, set the cursor...
;Assumes GSI_BANK
;----------------------------------------
GSISetCursor

	#IF GSI_LCD_SUPPORT

	call	GSISetModule

	movlw	0		;assume line 0
	btfsc	LdLCDRow,0	;set== 2nd line
	movlw	0x40		;2nd line
	addwf	LdLCDCol,W	;New DDRAM Address
	LCD_DDRAM_ADDRESS

	ENDIF			;GSI_LCD_SUPPORT
	;goto	GSICommonReturn
	return

;-------------------------------------
;	GSIWriteStr
;assumes GSI_BANK
;------------------------------------
	#IF GSI_LCD_SUPPORT
GSIWriteStr
	movwf	GlbLocalTempL		;store string vector
	call	GSISetCursor
	movf	GlbLocalTempL,W
	call	LCDWriteStr
	return
	#ENDIF

;---------------------------------------
;	GSIWriteBCD8
;Write an 8 bit BCD (3 digit) to LCD
;at current DDRAM address
;Receive:
;BCD in LdLocalTempH:LdLocalTempL
;****************************
;This might be Microchip code
;Don't think it is mine
;****************************
;---------------------------------------
	#IF GSI_LCD_SUPPORT
GSIWriteBCD8
;Packed BCD
	IF(SIMULATOR == FALSE)
	movf	LdLocalTempH,W
	btfsc	STATUS,Z,A
	goto	w8_d0		;MSB is 0
	;-
	addlw	0x30
	call	LCDWriteChar
w8_d0
	movf	LdLocalTempL,W
	addlw	0x30
	call	LCDWriteChar

	movlw	' '
	call	LCDWriteChar	;In case LSB was 0
	ENDIF
	return
	#ENDIF ;GSI_LCD_SUPPORT


;---------------------------------
;	GSIDisplayBinary
;---------------------------------
	#IF GSI_LCD_SUPPORT

GSIDisplayBinary
	IF(SIMULATOR == FALSE)
	movwf	LdBTemp		;store WREG
	movlw	8
	movwf	LdLCDCounter1
pcloop:
	clrf	LdBTemp1
	bcf	STATUS,C,A
	rlcf	LdBTemp,F
	rlcf	LdBTemp1,F
	movlw	30h
	addwf	LdBTemp1,W
	call	GSILCDWriteChar
	decf	LdLCDCounter1,F	
	btfss	STATUS,Z,A
	goto	pcloop
	movlw	' '
	call	GSILCDWriteChar
	ENDIF
	return
	#ENDIF ; GSI_LCD_SUPPORT

;------------------------------------------------
;	GSILCDWriteAscii
;Receive byte in WREG
;convert to ascii and write to lcd
;------------------------------------------------
	#IF GSI_LCD_SUPPORT
GSILCDWriteAscii
	movwf	LdAsciiTemp
	movwf	GlbLocalTempL
	clrf	GlbLocalTempH
	movlw	d'100'

	btfss	GlbLocalTempL,7		;>=128?
	bra	ssai_2
	incf	GlbLocalTempH
	subwf	GlbLocalTempL,F	
ssai_2
	
ssai_hundreds
	subwf	GlbLocalTempL,F
	bn	ssai_tens
	incf	GlbLocalTempH,F
	bra	ssai_hundreds
ssai_tens
	addwf	GlbLocalTempL,F
	movf	GlbLocalTempH,W
	addlw	0x30	
	call	GSILCDWriteChar
	call	DEBUGGSISendByte232
	
	clrf	GlbLocalTempH
	movlw	d'10'
ssai_1
	subwf	GlbLocalTempL,F
	bn	ssai_units
	incf	GlbLocalTempH,F
	bra	ssai_1
ssai_units
	addwf	GlbLocalTempL,F
	movf	GlbLocalTempH,W
	addlw	0x30
	call	GSILCDWriteChar
	call	DEBUGGSISendByte232
	movf	GlbLocalTempL,W
	addlw	0x30
	call	GSILCDWriteChar
	call	DEBUGGSISendByte232	
	;SPACE_RS232
	movf	LdAsciiTemp,W
	return
	#ENDIF ;GSI_LCD_SUPPORT


;------------------------------------------------
;	GSIWriteMessage
;Write message to LCD
;Index in WREG
;------------------------------------------------
	#IF GSI_LCD_SUPPORT
LCDWriteStr
;Receive address of message in W
;we are in GSI_BANK
	addwf	WREG			;Word ptr
	movwf	TBLPTRL			;index to str

	movlw	LOW PgmMssgTbl		;add the offset
	addwf	TBLPTRL

	clrf	TBLPTRH
	movlw	HIGH PgmMssgTbl		;ripple any Cy
	addwfc	TBLPTRH

	clrf	TBLPTRU
	movlw	UPPER PgmMssgTbl	;through H and U
	addwfc	TBLPTRU
	;TBLPTR holds address of string table anywhere in memory

	TBLRD*+				 
	movf	TABLAT,W		;low byte of str address
	movwf	LdTemp

	TBLRD*
	movf	TABLAT,W		;High byte of str address
	movwf	TBLPTRH
	;Don't fill upper as it will be modified if req
	;by the str pointer code
	movff	LdTemp,TBLPTRL
lwm1	
	TBLRD*+
	movf	TABLAT,W
	;Test first so that we can ignore a null string
	tstfsz	WREG	
	bra		lwm2
	return
lwm2
	;call	DEBUGGSISendByte232
	call	LCDWriteChar
	call	GSICursorRight		;Addition 8/7/06
	bra		lwm1
lwmExit
	#ENDIF ;GSI_LCD_SUPPORT

;-----------------------------------------
;	GSIUserAbort
;-----------------------------------------
GSIUserAbort
	bsf	GlbGSIState,GSI_STATE_USER_ABORT
	return
;-----------------------------------------------
;	SignOn
;Assumes GSI_BANK
;-----------------------------------------------
GSISignOn
#if 0
	movlw	0x0a
	call	GSIEchoByte232
	movlw	0x0d
	call	GSIEchoByte232
#endif
	#IF GSI_LCD_SUPPORT
	movlw	LCD_LINES-1
	movwf	LdLCDRow
	movlw	VERSION_MSSG
	call	GSIWriteStr
	clrf	WREG
	movwf	LdLCDRow		;top row
	call	GSISetCursor	
	#ENDIF ;GSI_LCD_SUPPORT
#if 0
	movlw	0x0a
	call	GSIEchoByte232
	movlw	0x0d
	call	GSIEchoByte232
#endif
	return



;-------------------------------------
;	GSIClrLine
;clear the line in LdLCDRow
;set cursor to front of first col of row
;-------------------------------------
	#IF GSI_LCD_SUPPORT
GSIClrLine

	clrf	LdLCDCol	;col 0
	call	GSISetCursor	;change lcd cursor

	movlw	LCD_COLS
	movwf	LdLocalTempL
sc1
	movlw	' '
	call	LCDWriteChar		;preserves WREG
	call	GSICursorRight
	decfsz	LdLocalTempL
	goto	sc1
	call	GSIBeginningOfLine

	return
	#ENDIF ;GSI_LCD_SUPPORT

;---------------------------
;	InitLCDVrbls
;---------------------------
	#IF GSI_LCD_SUPPORT
InitLCDVrbls

	clrf	LdLCDRow
	clrf	LdLCDCol
	;clrf	GlbGSIState	defer clearing till after
	;user abort reset check in DetermineResetType

	return
	#ENDIF ;GSI_LCD_SUPPORT

	END
