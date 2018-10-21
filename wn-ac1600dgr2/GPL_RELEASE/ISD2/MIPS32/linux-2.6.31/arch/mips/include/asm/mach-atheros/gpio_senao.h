/*****************************************************************************
 * ;
 * ;   (C) Unpublished Work of SENAO Networks Incorporated.  All Rights Reserved.
 * ;
 * ;       THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,
 * ;       PROPRIETARY AND TRADESECRET INFORMATION OF SENAO INCORPORATED.
 * ;       ACCESS TO THIS WORK IS RESTRICTED TO (I) SENAO EMPLOYEES WHO HAVE A
 * ;       NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE OF THEIR ASSIGNMENTS
 * ;       AND (II) ENTITIES OTHER THAN SENAO WHO HAVE ENTERED INTO APPROPRIATE
 * ;       LICENSE AGREEMENTS.  NO PART OF THIS WORK MAY BE USED, PRACTICED,
 * ;       PERFORMED, COPIED, DISTRIBUTED, REVISED, MODIFIED, TRANSLATED,
 * ;       ABBRIDGED, CONDENSED, EXPANDED, COLLECTED, COMPILED, LINKED, RECAST,
 * ;       TRANSFORMED OR ADAPTED WITHOUT THE PRIOR WRITTEN CONSENT OF SENAO.
 * ;       ANY USE OR EXPLOITATION OF THIS WORK WITHOUT AUTHORIZATION COULD
 * ;       SUBJECT THE PERPERTRATOR TO CRIMINAL AND CIVIL LIABILITY.
 * ;
 * ;------------------------------------------------------------------------------
 * ;
 * ;    Project : AR7240
 * ;    Creator : Jay
 * ;    File    : senao_gpio.h
 * ;    Abstract:
 * ;
 * ;       Modification History:
 * ;       By              Date     Ver.   Modification Description
 * ;       --------------- -------- -----  --------------------------------------
 * ;       Jay           20101119        Newly Create
 * ;*****************************************************************************/

#ifndef __SENAO_GPIO_H
#define __SENAO_GPIO_H
/*-----------------------------------------------------------------------------
 *                     include files
 *----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *                     macros, defines, typedefs, enums
 *----------------------------------------------------------------------------*/

typedef enum SN_BTN_ACTION
{
    SN_ATH_BTN_INIT=0,
    /*Below is button behaviour*/
    SN_ATH_BTN_TRIGGER_24G,
    SN_ATH_BTN_TRIGGER_5G,
    SN_ATH_BTN_TRIGGER_24G_5G,
    SN_ATH_BTN_NON_OP,
    SN_ATH_BTN_RESET,
    SN_ATH_BTN_RESET_2_DEF,
	SN_ATH_BTN_FACTORY_MODE,
    SN_ATH_BTN_ECO_1,
    SN_ATH_BTN_ECO_2,
    SN_ATH_BTN_CHILD_1,
    SN_ATH_BTN_CHILD_2,
    SN_ATH_BTN_TV,
    SN_ATH_BTN_CLONE_AP,
    SN_ATH_BTN_MRHESS_WPS,
    SN_ATH_BTN_STOP_24G,
    SN_ATH_BTN_STOP_5G
} SN_BTN_ACTION;

typedef enum SN_BTN_TYPE
{
	SN_ATH_BUTTON_TYPE_INIT=0,
    SN_ATH_BUTTON_TYPE_WPS,
    SN_ATH_BUTTON_TYPE_RESET,
    SN_ATH_BUTTON_TYPE_OP1,
    SN_ATH_BUTTON_TYPE_OP2
} SN_BTN_TYPE;

#define MAX_GPIO_NUMBER 23

typedef struct {
	int checkPiont;
    int signal;
	int led_gpio;
	int isUsing;
} atheros_gpio_info;

typedef struct {
    int gpio;           //gpio number (0 ~ 23)
    unsigned int on;        //interval of led on
    unsigned int off;       //interval of led off
    unsigned int blinks;        //number of blinking cycles
    unsigned int rests;     //number of break cycles
    unsigned int times;     //blinking times
} ralink_gpio_led_info;

typedef struct  {
    int ticks;
    unsigned int ons;
    unsigned int offs;
    unsigned int resting;
    unsigned int times;
} ralink_gpio_led_status_t;
enum LED_ACTION
{
    LED_ACTION_BLINKING=1,
    LED_ACTION_OFF,
    LED_ACTION_ON,
    LED_ACTION_FAST_BLINKING,
	LED_ACTION_WPS_INPROCESS = 11,
	LED_ACTION_WPS_FAIL = 14,
	LED_ACTION_WPS_PASS = 15,
    LED_ACTION_CUSTOM_BLINK_1 = 21,
    LED_ACTION_CUSTOM_BLINK_2 = 22,
    LED_ACTION_CUSTOM_BLINK_3 = 23,
    LED_ACTION_CUSTOM_BLINK_4 = 24
};

/*-----------------------------------------------------------------------------
 *                     data declarations, extern, static, const
 *----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 *                     functions declarations
 *----------------------------------------------------------------------------*/

#endif


