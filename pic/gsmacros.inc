;******************************************************************************
;* @file    gsmacros.inc part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************

;	radix <dec>
#ifndef GSMACROS_INC_INCLUDED
#define GSMACROS_INC_INCLUDED

;CY flag is inverse of 80x86 so use macro
;pic store High byte, Low byte 


RB0		EQU	0
RB1		EQU	1
RB2		EQU	2
RB3		EQU	3
RB4		EQU	4
RB5		EQU	5
RB6		EQU	6
RB7		EQU	7

RA0		EQU	0
RA1		EQU	1
RA2		EQU	2
RA3		EQU	3
RA4		EQU	4
RA5		EQU	5
RA6		EQU	6
RA7		EQU	7

Er	MACRO
	;Ra0=040
	;Ra1=225;
	;Ra2=04
	;Ra3=13

	ENDM

BANK0	MACRO
	IFDEF	P18FXXX
		movlb	0		;bank0
	ELSE
		bcf	STATUS,RP0
		bcf	STATUS,RP1
	ENDIF	;P18FXXX
	ENDM

BANK1	MACRO
	IFDEF	P18FXXX
		movlb	1		;bank1
	ELSE
		bsf	STATUS,RP0
		bcf	STATUS,RP1
	ENDIF	;P18FXXX
	ENDM

BANK2	MACRO
	IFDEF	P18FXXX
		movlb	2		;bank2
	ELSE
		IFDEF __16F877
			bcf	STATUS,RP0
			bsf	STATUS,RP1
		ENDIF
	ENDIF	;P18FXXX
	ENDM

BANK3	MACRO
	IFDEF	P18FXXX
		movlb	3		;bank0
	ELSE

	IFDEF __16F877
		bsf	STATUS,RP0
		bsf	STATUS,RP1
	ENDIF
	
	ENDIF	;P18FXXX
	ENDM


PAGE0	MACRO
	IFDEF __16F877
	bcf	PCLATH,3
	bcf	PCLATH,4
	ENDIF
	ENDM	

PAGE1	MACRO
	IFDEF __16F877
	bsf	PCLATH,3
	bcf	PCLATH,4
	ENDIF
	ENDM	

PAGE2	MACRO
	IFDEF __16F877
	bcf	PCLATH,3
	bsf	PCLATH,4
	ENDIF
	ENDM	

PAGE3	MACRO
	IFDEF __16F877
	bsf	PCLATH,3
	bsf	PCLATH,4
	ENDIF
	ENDM	


SET8	MACRO	reg,literal
;working 12/1/2000
	movlw	literal
	movwf	reg
	ENDM

SET16	MACRO reg,literal
;working 12/1/2000
	movlw	HIGH literal
	movwf	reg
	;
	movlw	LOW literal
	movwf	reg+1

	ENDM

INC16	MACRO	reg		;16 bit inc
;working 12/1/2000
	incf	reg+1,F		;inc low byte
	btfsc	STATUS,Z
	incf	reg,F		;inc high byte if rollover

	;could be used as DEC if 2's comp counter
	;values then use INC instead of DEC
	;this fn would need to perform movf reg+1,F
	;to perform last check
	ENDM


;INC16	MACRO	reg		;16 bit inc
;	incf	reg+1,F		;inc low byte
;	btfsc	STATUS,Z
;	incf	reg,F		;inc high byte if rollover
;	movf	reg,F
;	btfsc	STATUS,Z
;	movf	reg+1,F		;perform last Z check
;using this fn as "DEC" saves one clock
;see above and below
;	ENDM

DEC16	MACRO	reg
;working 12/1/2000. Sets Z on zero otherwise NZ
	movlw	1
	subwf	reg+1,F		;low byte
	;-c=0 if ovf z=0
	btfss	STATUS,C		;c=0 ==ovf
	decf	reg,F
	movf	reg,F		;sets Z flag
	btfsc	STATUS,Z
	movf	reg+1,F		;sets Z flag
	ENDM




ADD16RR	MACRO	r1,r2		;register+register
;working
	movf	r2,W		;add the low bytes
	addwf	r1,F
	btfsc	STATUS,C
	incf	r1+1,F		;add the carry
	movf	r2+1,W
	addwf	r1+1,F		;add the high bytes
	ENDM

	IF 0

SUB16RR	MACRO	r1,r2		;register-register
;working
	movf	r1,W
	subwf	r2,W
	movwf	r1
	btfss	STATUS,C
	incf	r1+1,F		;same as decf r2+1, but leave r2 unchanged
	movf	r1+1,W
	subwf	r2+1,W
	movwf	r1+1
	ENDM

	ENDIF

SUB16RR	MACRO	r1,r2		;register-register
;working
;receive HL
;eg org 0ch CounterH,CounterL : SUB16RR CounterH,OtherVrblH
	movf	r1+1,W
	subwf	r2+1,W
	movwf	r1+1
	btfss	STATUS,C
	incf	r1,F		;same as decf r2+1, but leave r2 unchanged
	movf	r1,W
	subwf	r2,W
	movwf	r1
	ENDM


;SUB16	MACRO	 
;        movf    r2lo,w 
;        subwf   r1ho,f 
;        movf    r2hi,w 
;        skpc 
;        incfsz r2hi,w 
;        subwf r1hi,f 
;	ENDM


CMP16RR	MACRO	r1,r2		;register==literal	
;returns:
; C:r1>r2
; Z:r1==r2
; NZ,NC: r1<r2
;IMPORTANT the caller must test C before Z. Z can be set functionally

	movf	r1,W
	subwf	r2,W
	movf	r1+1,W
	btfss	STATUS,C
	subwf	r2+1,W
	ENDM

CMP16RL	MACRO	r1,literal		;register==literal	
;returns:
; C:r1>literal
; Z:r1==literal
; NZ,NC: r1<literal
;IMPORTANT the caller must test C before Z. Z can be set functionally

	movf	r1,W
	sublw	HIGH literal
	movf	r1+1,W
	btfss	STATUS,C
	sublw	LOW literal
	ENDM

	IF 0
CMPU16RL	MACRO	r1,literal		;unsigned compare
;returns:
; C:r1>literal
; Z:r1==literal
; NZ,NC: r1<literal
;IMPORTANT the caller must test C before Z. Z can be set functionally


	movlw	HIGH literal
	movwf	MTempH
	movlw	LOW literal
	movwf	MTempL
	;
	movf	r1,W
	movwf	MTemp1			;use a copy of r1 as may modify
	call	UnsignedCompare
	;
	btfsc	MTemp1,7		;check bit 15 of reg
	goto	rg128
	
rg128		;reg greater than 128
	btfsc	MTempH,7		;literal also > 128?
	goto	bothg128
	;no, so return reg > literal
	bsf	STATUS,C
	ENDM 	

bothg128	;reg and literal >128
	bcf	MTempH,7		;copy of literal
	bcf	MTemp1,7		;copy of reg

compare
	movf	MTemp1,W
	subwf	MTempH
	movf	r1+1,W
	btfss	STATUS,C
	subwf	MTempL


	ENDM

	ENDIF


ADD16RL	MACRO	reg,literal		;register+register
;working 12/1/2000
	movlw	LOW literal		;add the low bytes
	addwf	reg+1,F
	btfsc	STATUS,C
	incf	reg,F		;add the carry
	movlw	HIGH literal
	addwf	reg,F		;add the high bytes
	ENDM


SUB16RL	MACRO	r1,r2		;register-register
;working 
	movf	r1,W
	subwf	r2,W
	movwf	r1
	btfss	STATUS,C
	incf	r1+1,F		;same as decf r2+1, but leave r2 unchanged
	movf	r1+1,W
	subwf	High r2,W
	movwf	r1+1

	ENDM

SET_TMR0 MACRO t
	movlw	t
	movwf	TMR0
	ENDM

#endif ;GS_MACROS_INCLUDED
