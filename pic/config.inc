;******************************************************************************
;* @file    config.inc part of gsi
;* @author  Glenn Self
;* @version V1.0
;* @date    April 2004
;* Copyright Glenn Self 2004
;******************************************************************************


;config

#ifndef CONFIG_INC_INCLUDED

#define CONFIG_INC_INCLUDED
#IF CLOCK ==0
	ERROR No clock set
#ENDIF
#IF PLL == TRUE
	ERROR wrong
#define GSI_OSC_TYPE	HSPLL
#ELSE
#define GSI_OSC_TYPE	HS
#ENDIF

#IF DEVICE == _18F458
	CONFIG OSC=HS,BOR=ON,BORV=27,STVR=ON

#ELSE
;BOR is enabled in hardware, disable in software 
CONFIG OSC=HS,BOREN = BOACTIVE ,BORV=2,STVREN=ON
	ERROR wrong config??
#ENDIF
	
#if ICD2

	CONFIG WDT=OFF,LVP=OFF

#else
	CONFIG WDT=ON
#endif


#if 0
;Program Configuration Register 1H
		__CONFIG    _CONFIG1H, _OSCS_OFF_1H & _HS_OSC_1H

;Program Configuration Register 2L
		;__CONFIG    _CONFIG2L, _BOR_ON_2L & _BORV_27_2L & _PWRT_OFF_2L
		__CONFIG    _CONFIG2L, _BOR_ON_2L & _BORV_27_2L & _PWRT_OFF_2L

	IF ICD2
;Program Configuration Register 2H
	_CONFIG    _CONFIG2H, _WDT_OFF_2H  & _WDTPS_128_2H  
	
	ELSE
;Program Configuration Register 2H
		_CONFIG    _CONFIG2H, _WDT_ON_2H & _WDTPS_128_2H
	ENDIF
;Program Configuration Register 3H
;		__CONFIG    _CONFIG3H, _CCP2MX_ON_3H

;Program Configuration Register 4L
		__CONFIG    _CONFIG4L, _STVR_ON_4L & _LVP_OFF_4L & _DEBUG_OFF_4L

;Program Configuration Register 5L
;		__CONFIG    _CONFIG5L, _CP0_OFF_5L & _CP1_OFF_5L & _CP2_OFF_5L & _CP3_OFF_5L 

;Program Configuration Register 5H
;		__CONFIG    _CONFIG5H, _CPB_ON_5H & _CPD_OFF_5H

;Program Configuration Register 6L
;		__CONFIG    _CONFIG6L, _WRT0_OFF_6L & _WRT1_OFF_6L & _WRT2_OFF_6L & _WRT3_OFF_6L 

;Program Configuration Register 6H
;		__CONFIG    _CONFIG6H, _WRTC_OFF_6H & _WRTB_OFF_6H & _WRTD_OFF_6H

;Program Configuration Register 7L
;		__CONFIG    _CONFIG7L, _EBTR0_OFF_7L & _EBTR1_OFF_7L & _EBTR2_OFF_7L & _EBTR3_OFF_7L

;Program Configuration Register 7H
;		__CONFIG    _CONFIG7H, _EBTRB_OFF_7H

;ID Locations Register 0
;		__IDLOCS    _IDLOC0, <expression>

;ID Locations Register 1
;		__IDLOCS    _IDLOC1, <expression>

;ID Locations Register 2
;		__IDLOCS    _IDLOC2, <expression>

;ID Locations Register 3
;		__IDLOCS    _IDLOC3, <expression>

;ID Locations Register 4
;		__IDLOCS    _IDLOC4, <expression>

;ID Locations Register 5
;		__IDLOCS    _IDLOC5, <expression>

;ID Locations Register 6
;		__IDLOCS    _IDLOC6, <expression>

;ID Locations Register 7
;		__IDLOCS    _IDLOC7, <expression>

;Device ID registers hold device ID and revision number and can only be read
;Device ID Register 1
;               DEV2, DEV1, DEV0, REV4, REV3, REV2, REV1, REV0
;Device ID Register 2
;               DEV10, DEV9, DEV8, DEV7, DEV6, DEV5, DEV4, DEV3

#endif
#endif ; CONFIG_INC_INCLUDED
