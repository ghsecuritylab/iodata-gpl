/*****************************************************************************
;
;   (C) Unpublished Work of Senao Networks, Inc.  All Rights Reserved.
;
;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
;       PROPRIETARY AND TRADESECRET INFORMATION OF SENAO INCORPORATED.
;       ACCESS TO THIS WORK IS RESTRICTED TO (I) SENAO EMPLOYEES WHO HAVE A
;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
;       AND (II) ENTITIES OTHER THAN SENAO WHO HAVE ENTERED INTO APPROPRIATE
;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF SENAO.
;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
;
;------------------------------------------------------------------------------
;
;    Project : WN-AC1600DGR_SCO
;    Creator : 
;    File    : gpio.h
;    Abstract: include file of the application profile.
;
;       Modification History:
;       By              Date     	Ver.   	Modification Description
;       --------------- -------- 	-----  	--------------------------------------
;       leonard         2013/1/23       	Newly Create
;*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _SENAO_GPIO_H
#define _SENAO_GPIO_H

/* linux-2.6.31.x/arch/mips/atheros_ar934x/gpio.c LED control */
#define USE_WN_AG300DGR_LED
#define USE_SIMPLE_CONFIG_GPIO_LED
//#define USE_ENGENIUS_GPIO_LED

/*
 * LED
 */
#define SENAO_GPIO_LED_POWER       14
/* #define SENAO_GPIO_LED_WLAN        14 */
#define SENAO_GPIO_LED_WPS         22
#define SENAO_GPIO_LED_WPS2        21
#define SENAO_GPIO_LED_OP1         3
#define SENAO_GPIO_LED_OP2         2
#define SENAO_GPIO_LED_OP3         23

#define SENAO_GPIO_LED_5G_WPS      SENAO_GPIO_LED_WPS2
#define SENAO_GPIO_LED_ECO         SENAO_GPIO_LED_OP1
#define SENAO_GPIO_LED_WPS_COPY    SENAO_GPIO_LED_OP2

/* led status control for ECO mode */
#define CONFIG_ECO_LED_OFF		   30
#define CONFIG_ECO_LED_ON		   31
#define CONFIG_ECO_WPS_STATUS_INACTIVE		   40

/*
 * BUTTON
 */
#define SENAO_GPIO_BUTTON_WPS      16
#define SENAO_GPIO_BUTTON_RESET    17
#define SENAO_GPIO_BUTTON_OP_1     0
#define SENAO_GPIO_BUTTON_OP_2     15
#define SENAO_GPIO_BUTTON_ECO      SENAO_GPIO_BUTTON_OP_1
//#define SENAO_GPIO_BUTTON_CHILD    SENAO_GPIO_BUTTON_OP_2

/* 3 Level: L M R;  2 Level : L R
 *  
 * SWITCH
 * _____________________________________
 * |_L_M_R_|_|_W_|_L1_|_L2_|_L3_|_L4_|__|
 * |                                    |
 * |                                    |
 * |              PCB                   |
 * |                                    | 
 * |____________________________________|
 */

#define SENAO_MODESWITCH_GPIO_L		1
#define SENAO_MODESWITCH_GPIO_R		19

#define WPS_LED_GPIO	SENAO_GPIO_LED_WPS
#define JUMPSTART_GPIO	SENAO_GPIO_BUTTON_WPS


/*
	Button Timer
*/
/*there are several timer, trigger 2.4G WPS, trigger 5G wps, do reboot, do res to default*/
#ifdef SENAO_GPIO_BUTTON_WPS
/*unit second*/
#define BUTTON_PROBE_WPS_TIME_0             0   /*init from zero */
#define BUTTON_PROBE_WPS_TIME_1             3   /*0~10unit :Trigger 2.4G WPS*/
#define BUTTON_PROBE_WPS_TIME_2             6   /*10~20unit :Do nothing*/
#define BUTTON_PROBE_WPS_TIME_3             65535  /*20~30unit :Reset*/
#define BUTTON_PROBE_WPS_TIME_4             65535 /*>30unit : Reset to default*/
#define BUTTON_PROBE_WPS_TIME_5             65535 /*nothing*/
// #define BUTTON_PROBE_WPS_ACTION_0			SN_RTL819X_BTN_NON_OP
#define BUTTON_PROBE_WPS_ACTION_1			SN_ATH_BTN_NON_OP
#define BUTTON_PROBE_WPS_ACTION_2			SN_ATH_BTN_TRIGGER_24G
#define BUTTON_PROBE_WPS_ACTION_3			SN_ATH_BTN_TRIGGER_5G
#define BUTTON_PROBE_WPS_ACTION_4			SN_ATH_BTN_NON_OP
#define BUTTON_PROBE_WPS_ACTION_5			SN_ATH_BTN_NON_OP
#endif

#ifdef SENAO_GPIO_BUTTON_RESET
/*unit second*/
#define BUTTON_PROBE_RESET_TIME_0			0   	/*from zero */
#define BUTTON_PROBE_RESET_TIME_1        	3 		/*0~15second reset*/
#define BUTTON_PROBE_RESET_TIME_2        	65535 	/*> 15 reset to default*/
#define BUTTON_PROBE_RESET_TIME_3			65535 	/*nothing*/
#define BUTTON_PROBE_RESET_TIME_4			65535 	/*nothing*/
#define BUTTON_PROBE_RESET_TIME_5			65535 	/*nothing*/
// #define BUTTON_PROBE_RESET_ACTION_0			SN_RTL819X_BTN_NON_OP
#define BUTTON_PROBE_RESET_ACTION_1			SN_ATH_BTN_NON_OP
#define BUTTON_PROBE_RESET_ACTION_2			SN_ATH_BTN_RESET_2_DEF
#define BUTTON_PROBE_RESET_ACTION_3			SN_ATH_BTN_NON_OP
#define BUTTON_PROBE_RESET_ACTION_4			SN_ATH_BTN_NON_OP
#define BUTTON_PROBE_RESET_ACTION_5			SN_ATH_BTN_NON_OP
#endif

#ifdef SENAO_GPIO_BUTTON_OP_1
#define BUTTON_PROBE_BUTTON_OP1_TIME_0		0.1   	/*from zero */
#define BUTTON_PROBE_BUTTON_OP1_TIME_1      1 		
#define BUTTON_PROBE_BUTTON_OP1_TIME_2      3    	
#define BUTTON_PROBE_BUTTON_OP1_TIME_3		65535 	/*nothing*/
#define BUTTON_PROBE_BUTTON_OP1_ACTION_1	SN_ATH_BTN_ECO_1
#define BUTTON_PROBE_BUTTON_OP1_ACTION_2	SN_ATH_BTN_NON_OP
#define BUTTON_PROBE_BUTTON_OP1_ACTION_3	SN_ATH_BTN_ECO_2
#endif
#ifdef SENAO_GPIO_BUTTON_OP_2
#define BUTTON_PROBE_BUTTON_OP2_TIME_0		0   	/*from zero */
#define BUTTON_PROBE_BUTTON_OP2_TIME_1      3 		
#define BUTTON_PROBE_BUTTON_OP2_TIME_2      65535
#define BUTTON_PROBE_BUTTON_OP2_TIME_3		65535 	/*nothing*/
#define BUTTON_PROBE_BUTTON_OP2_ACTION_1	SN_ATH_BTN_NON_OP
#define BUTTON_PROBE_BUTTON_OP2_ACTION_2	SN_ATH_BTN_CLONE_AP
#define BUTTON_PROBE_BUTTON_OP2_ACTION_3	SN_ATH_BTN_NON_OP
#endif


#define BLINKING_PWR_LED_TIME				BUTTON_PROBE_RESET_TIME_1

#define BLINKING_WPS_LED_TIME				BUTTON_PROBE_WPS_TIME_1
#define BLINKING_WPS2_LED_TIME				BUTTON_PROBE_WPS_TIME_2

#define BLINKING_OP_2_LED_TIME              BUTTON_PROBE_BUTTON_OP2_TIME_1

/*WPS gpio action*/
#define WPS_LED_GPIO_NOP_NUM	 		0 /*no action*/
#define WPS_START_LED_GPIO_NUM			1 /*wps start*/
#define WPS_PBC_OVERLAPPING_GPIO_NUM 	2 /*wps overlapping*/
#define WPS_ERROR_LED_GPIO_NUM 			3 /*wps error*/
#define WPS_SUCCESS_LED_GPIO_NUM	 	4 /*wps success*/


/* WPS LED In Process blink interval */
#define WPS_LED_IN_PROCESS_BLINK_INTERVAL   200 /* msec */
/* total duration will be WPS_LED_BLINK_TIME_DURATION X WPS_LED_BLINK_TIME_INTERVAL */
#define WPS_LED_IN_PROCESS_BLINK_TIME_COUNT 1000

/* WPS LED SUCCESS blink interval */
#define WPS_LED_SUCCESS_BLINK_INTERVAL   300000 /* msec */
/* total duration will be WPS_LED_BLINK_TIME_DURATION X WPS_LED_BLINK_TIME_INTERVAL */
#define WPS_LED_SUCCESS_BLINK_TIME_COUNT 2

/* WPS LED Fail blink interval */
#define WPS_LED_FAIL_BLINK_INTERVAL   100 /* msec */
/* total duration will be WPS_LED_BLINK_TIME_DURATION X WPS_LED_BLINK_TIME_INTERVAL */
#define WPS_LED_FAIL_BLINK_TIME_COUNT 1200

/* POWER LED Blink interval */
#define PWR_LED_BLINK_INTERVAL      100  /* msec */



/* CUSTOM BLINK ACTION 1 ON interval */
#define CUSTOM_BLINK_1_ON_INTERVAL   2000 /* msec */
/* CUSTOM BLINK ACTION 1 OFF interval */
#define CUSTOM_BLINK_1_OFF_INTERVAL  1000 /* msec */


#ifdef SENAO_GPIO_LED_OP2
/* CUSTOM OP 2 BLINK interval */
#define OP_2_LED_BLINK_INTERVAL      200 /* msec */
#endif

#ifdef SENAO_GPIO_LED_ECO
#define IODATA_ECO_LED_SPECIAL_ACTION 3 /* sec */
#endif


#endif /* _SENAO_GPIO_H */

#ifdef __cplusplus
}
#endif
