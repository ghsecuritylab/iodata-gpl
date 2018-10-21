#ifndef EXPORT_SYMTAB
#define EXPORT_SYMTAB
#endif

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/init.h>
#include <linux/resource.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/miscdevice.h>
#include <asm/types.h>
#include <asm/irq.h>
#include <asm/delay.h>
#include <asm/system.h>

#include "sysgpio.h"
#include "gconfig.h"
#include <gpio_senao.h>
#include <atheros.h>
#if SUPPORT_SYSTEM_OPERATION_MODE_LED_CTRL
#include "opmodes.h"
#endif

#define ATH_FACTORY_RESET		0x89ABCDEF

static atomic_t ath_fr_status = ATOMIC_INIT(0);
static volatile int ath_fr_opened = 0;
static wait_queue_head_t ath_fr_wq;
static u_int32_t push_time = 0;

#ifdef USE_SITECOM_GPIO_LED
//(WPS1 button + WPS2 button) * 10 seconds = reset2default
static int count_ignorepb=0;
static u_int32_t start_time=0;
int WPS_in_process =0;
int WPS_24G_is_disable=0;
int WPS_5G_is_disable=0;
int _wlan_led_off_ = 0;
EXPORT_SYMBOL(_wlan_led_off_);
#endif
#if DUAL_RADIO_WPS_BEHAVIOR_FOR_WPS_V2
wps_hw_button_press = 0;
#endif
struct timer_list os_timer_t;
struct timer_list os_reset_timer_t;
struct timer_list os_op_2_timer_t;
#if defined (BLINKING_WPS_LED_TIME) && defined (SENAO_GPIO_BUTTON_WPS)
struct timer_list  os_wps_led_timer_t;
#endif
#if defined (BLINKING_WPS_LED_STOP_TIME) && defined (SENAO_GPIO_BUTTON_WPS)
struct timer_list  os_wps_led_off_timer_t;
#endif
#if defined (BLINKING_WPS2_LED_TIME) && defined (SENAO_GPIO_BUTTON_WPS)
struct timer_list  os_wps2_led_timer_t;
#endif
#if defined (IODATA_ECO_LED_SPECIAL_ACTION) && defined (SENAO_GPIO_LED_ECO) && defined (SENAO_GPIO_BUTTON_ECO)
struct timer_list iodata_eco_timer_t;
#endif
#define _LED_OFF 1
#define _LED_ON 0

#define WPS_LED_OFF		_LED_OFF
#define WPS_LED_ON		_LED_ON

#define USB_LED_OFF		_LED_OFF
#define USB_LED_ON		_LED_ON

/* 2012-04-25, Support CONFIG_GPIO_LED like sysLedCtrlLedAction. */
#define SUPPORT_SN_LED_GPIO_CTRL 1

#if SUPPORT_SN_LED_GPIO_CTRL
#define SIMPLE_CONFIG_LED_CTRL 0

#define WPS_INACTIVE	0
#define WPS_ACTIVE		1

#define ECO_LED_ACTION_NONE	0
#define ECO_LED_ACTION_OFF	1
#define ECO_LED_ACTION_ON	2

struct sn_led_gpio_t
{
	/* LED on/off */
	int val;
	/* os_timer */
	struct timer_list t;
	/* timer start  */
	int timer_start;
	/* action */
	int action;
	/* WPS state*/
	int wps_state;
	/* ECO led*/
	int eco_led_state;
};

struct sn_led_gpio_t sn_led_gpio_data[MAX_GPIO_NUMBER+1];
static void sn_led_gpio_set(int gpio, int act);
#endif //SUPPORT_SN_LED_GPIO_CTRL


#define SIMPLE_CONFIG_OFF   1
#define SIMPLE_CONFIG_ON    2
#define SIMPLE_CONFIG_BLINK 3

#define ATHEROS_WPS_FAIL    4
#define ATHEROS_WPS_PASS    5

#if SIMPLE_CONFIG_SUPPORT_PWR_LED_CONTROL
/* Power LED control */
#define SIMPLE_CONFIG_PWR_LED_ACT_0  10
#define SIMPLE_CONFIG_PWR_LED_ACT_1  11
#define SIMPLE_CONFIG_PWR_LED_ACT_2  12
#define SIMPLE_CONFIG_PWR_LED_ACT_3  13
#endif

#if SIMPLE_CONFIG_SUPPORT_ETH_LED_CONTROL
/* Ethernet(WAN/LAN) LED control */
#define SIMPLE_CONFIG_ETH_LED_ON     20
#define SIMPLE_CONFIG_ETH_LED_OFF    21
#endif

#define SIMPLE_CONFIG_ECO_LED_OFF    30
#define SIMPLE_CONFIG_ECO_LED_ON    31
#define SIMPLE_CONFIG_ECO_WPS_STATUS_INACTIVE    40
#ifdef USE_SITECOM_GPIO_LED
#define SIMPLE_CONFIG_WPS_24G_DISABLE    41
#define SIMPLE_CONFIG_WPS_24G_ENABLE 42
#define SIMPLE_CONFIG_WPS_5G_DISABLE    43
#define SIMPLE_CONFIG_WPS_5G_ENABLE 44
#endif
/* we must get system operation mode to do some special led behavior*/
#if SUPPORT_SYSTEM_OPERATION_MODE_LED_CTRL
#define CONFIG_SYSTEM_OPERATION_MODE   99
#endif

/* WPS LED In Process blink interval */
#ifndef WPS_LED_IN_PROCESS_BLINK_INTERVAL
#define WPS_LED_IN_PROCESS_BLINK_INTERVAL   1000 /* msec */
#endif

/* total duration will be WPS_LED_BLINK_TIME_DURATION X WPS_LED_BLINK_TIME_INTERVAL */
#ifndef WPS_LED_IN_PROCESS_BLINK_TIME_COUNT
#define WPS_LED_IN_PROCESS_BLINK_TIME_COUNT 130
#endif

/* WPS LED SUCCESS blink interval */
#ifndef WPS_LED_SUCCESS_BLINK_INTERVAL
#define WPS_LED_SUCCESS_BLINK_INTERVAL   1000 /* msec */
#endif

/* total duration will be WPS_LED_BLINK_TIME_DURATION X WPS_LED_BLINK_TIME_INTERVAL */
#ifndef WPS_LED_SUCCESS_BLINK_TIME_COUNT
#define WPS_LED_SUCCESS_BLINK_TIME_COUNT 330
#endif

/* WPS LED SUCCESS blink interval */
#ifndef WPS_LED_FAIL_BLINK_INTERVAL
#define WPS_LED_FAIL_BLINK_INTERVAL   250 /* msec */
#endif

/* total duration will be WPS_LED_BLINK_TIME_DURATION X WPS_LED_BLINK_TIME_INTERVAL */
#ifndef WPS_LED_FAIL_BLINK_TIME_COUNT
#define WPS_LED_FAIL_BLINK_TIME_COUNT 1220
#endif

/* POWER LED blink interval */
#ifndef PWR_LED_BLINK_INTERVAL
#define PWR_LED_BLINK_INTERVAL   250 /* msec */
#endif

#define OS_TIMER_FUNC(_fn)	\
	void _fn(unsigned long timer_arg)

#define OS_GET_TIMER_ARG(_arg, _type)	\
	(_arg) = (_type)(timer_arg)

#define OS_INIT_TIMER(_osdev, _timer, _fn, _arg)	\
do {							\
	init_timer(_timer);				\
	(_timer)->function = (_fn);			\
	(_timer)->data = (unsigned long)(_arg);		\
} while (0)

#define OS_SET_TIMER(_timer, _ms)	\
	mod_timer(_timer, jiffies + ((_ms)*HZ)/1000)

#define OS_CANCEL_TIMER(_timer)		del_timer(_timer)

/*
 * GPIO interrupt stuff
 */
typedef enum {
	INT_TYPE_EDGE,
	INT_TYPE_LEVEL,
} ath_gpio_int_type_t;

typedef enum {
	INT_POL_ACTIVE_LOW,
	INT_POL_ACTIVE_HIGH,
} ath_gpio_int_pol_t;

/*
** Simple Config stuff
*/
typedef irqreturn_t (*sc_callback_t) (int, void *, void *, void *);

/*
 * Multiple Simple Config callback support
 * For multiple radio scenarios, we need to post the button push to
 * all radios at the same time.  However, there is only 1 button, so
 * we only have one set of GPIO callback pointers.
 *
 * Creating a structure that contains each callback, tagged with the
 * name of the device registering the callback.  The unregister routine
 * will need to determine which element to "unregister", so the device
 * name will have to be passed to unregister also
 */

typedef struct {
	char		*name;
	sc_callback_t	registered_cb;
	void		*cb_arg1;
	void		*cb_arg2;
} multi_callback_t;

typedef struct _ath_btn_t
{
    unsigned int    wps_btn_counter;
    unsigned int    wps_btn_state;
    pid_t           pid;
    SN_BTN_ACTION  	e_ath_btn_action;
    SN_BTN_TYPE		e_ath_btn_type;
    unsigned int    gpio;
} ath_btn_t;

static ath_btn_t g_ath_btn;
/*
 * Specific instance of the callback structure
 */
static multi_callback_t sccallback[2];
static volatile int ignore_pushbutton = 1;
static struct proc_dir_entry *simple_config_entry = NULL;
static struct proc_dir_entry *simulate_push_button_entry = NULL;
static struct proc_dir_entry *simple_config_led_entry = NULL;
static int wps_led_blinking = 0;
#if defined(SENAO_MODESWITCH_GPIO_L) || defined(SENAO_MODESWITCH_GPIO_M) || defined(SENAO_MODESWITCH_GPIO_R)
static int sn_modeswitch_init = 0;
#endif

#if SUPPORT_SYSTEM_OPERATION_MODE_LED_CTRL
/* sysOPmode */
static int sys_opmode=0;
#endif



void ath_gpio_config_int(int gpio,
			 ath_gpio_int_type_t type,
			 ath_gpio_int_pol_t polarity)
{
	u32 val;

	/*
	 * allow edge sensitive/rising edge too
	 */
	if (type == INT_TYPE_LEVEL) {
		/* level sensitive */
		ath_reg_rmw_set(ATH_GPIO_INT_TYPE, (1 << gpio));
	} else {
		/* edge triggered */
		val = ath_reg_rd(ATH_GPIO_INT_TYPE);
		val &= ~(1 << gpio);
		ath_reg_wr(ATH_GPIO_INT_TYPE, val);
	}

	if (polarity == INT_POL_ACTIVE_HIGH) {
		ath_reg_rmw_set(ATH_GPIO_INT_POLARITY, (1 << gpio));
	} else {
		val = ath_reg_rd(ATH_GPIO_INT_POLARITY);
		val &= ~(1 << gpio);
		ath_reg_wr(ATH_GPIO_INT_POLARITY, val);
	}

	ath_reg_rmw_set(ATH_GPIO_INT_ENABLE, (1 << gpio));
}

void ath_gpio_config_output(int gpio)
{
	//printk("%s: gpio=%d\n",__func__, gpio);
#if defined(CONFIG_MACH_AR934x) || \
    defined(CONFIG_MACH_QCA955x)
	ath_reg_rmw_clear(ATH_GPIO_OE, (1 << gpio));
#else
	ath_reg_rmw_set(ATH_GPIO_OE, (1 << gpio));
#endif
}
EXPORT_SYMBOL(ath_gpio_config_output);

void ath_gpio_config_input(int gpio)
{
	//printk("%s: gpio=%d\n",__func__, gpio);
#if defined(CONFIG_MACH_AR934x) || \
    defined(CONFIG_MACH_QCA955x)
	ath_reg_rmw_set(ATH_GPIO_OE, (1 << gpio));
#else
	ath_reg_rmw_clear(ATH_GPIO_OE, (1 << gpio));
#endif
}

void ath_gpio_out_val(int gpio, int val)
{
	if (val & 0x1) {
		ath_reg_rmw_set(ATH_GPIO_OUT, (1 << gpio));
	} else {
		ath_reg_rmw_clear(ATH_GPIO_OUT, (1 << gpio));
	}
}
EXPORT_SYMBOL(ath_gpio_out_val);

int ath_gpio_in_val(int gpio)
{
	return ((1 << gpio) & (ath_reg_rd(ATH_GPIO_IN)));
}

static void
ath_gpio_intr_enable(unsigned int irq)
{
	ath_reg_rmw_set(ATH_GPIO_INT_MASK,
				(1 << (irq - ATH_GPIO_IRQ_BASE)));
}

static void
ath_gpio_intr_disable(unsigned int irq)
{
	ath_reg_rmw_clear(ATH_GPIO_INT_MASK,
				(1 << (irq - ATH_GPIO_IRQ_BASE)));
}

static unsigned int
ath_gpio_intr_startup(unsigned int irq)
{
	ath_gpio_intr_enable(irq);
	return 0;
}

static void
ath_gpio_intr_shutdown(unsigned int irq)
{
	ath_gpio_intr_disable(irq);
}

static void
ath_gpio_intr_ack(unsigned int irq)
{
	ath_gpio_intr_disable(irq);
}

static void
ath_gpio_intr_end(unsigned int irq)
{
	if (!(irq_desc[irq].status & (IRQ_DISABLED | IRQ_INPROGRESS)))
		ath_gpio_intr_enable(irq);
}

static int
ath_gpio_intr_set_affinity(unsigned int irq, const struct cpumask *dest)
{
	/*
	 * Only 1 CPU; ignore affinity request
	 */
	return 0;
}

struct irq_chip /* hw_interrupt_type */ ath_gpio_intr_controller = {
	.name = "ATH GPIO",
	.startup = ath_gpio_intr_startup,
	.shutdown = ath_gpio_intr_shutdown,
	.enable = ath_gpio_intr_enable,
	.disable = ath_gpio_intr_disable,
	.ack = ath_gpio_intr_ack,
	.end = ath_gpio_intr_end,
	.eoi = ath_gpio_intr_end,
	.set_affinity = ath_gpio_intr_set_affinity,
};

void ath_gpio_irq_init(int irq_base)
{
	int i;

	for (i = irq_base; i < irq_base + ATH_GPIO_IRQ_COUNT; i++) {
		irq_desc[i].status = IRQ_DISABLED;
		irq_desc[i].action = NULL;
		irq_desc[i].depth = 1;
		//irq_desc[i].chip = &ath_gpio_intr_controller;
		set_irq_chip_and_handler(i, &ath_gpio_intr_controller,
					 handle_percpu_irq);
	}
}

void
ath_gpio_set_fn(int gpio, int fn)
{
#define gpio_fn_reg(x)	((x) / 4)
#define gpio_lsb(x)	(((x) % 4) * 8)
#define gpio_msb(x)	(gpio_lsb(x) + 7)
#define gpio_mask(x)	(0xffu << gpio_lsb(x))
#define gpio_set(x, f)	(((f) & 0xffu) << gpio_lsb(x))

	uint32_t *reg = ((uint32_t *)GPIO_OUT_FUNCTION0_ADDRESS) +
					gpio_fn_reg(gpio);

	ath_reg_wr(reg, (ath_reg_rd(reg) & ~gpio_mask(gpio)) | gpio_set(gpio, fn));
}


int32_t register_simple_config_callback(char *cbname, void *callback, void *arg1, void *arg2)
{
	printk("SC Callback Registration for %s\n", cbname);
	if (!sccallback[0].name) {
		sccallback[0].name = (char*)kmalloc(strlen(cbname), GFP_KERNEL);
		strcpy(sccallback[0].name, cbname);
		sccallback[0].registered_cb = (sc_callback_t) callback;
		sccallback[0].cb_arg1 = arg1;
		sccallback[0].cb_arg2 = arg2;
	} else if (!sccallback[1].name) {
		sccallback[1].name = (char*)kmalloc(strlen(cbname), GFP_KERNEL);
		strcpy(sccallback[1].name, cbname);
		sccallback[1].registered_cb = (sc_callback_t) callback;
		sccallback[1].cb_arg1 = arg1;
		sccallback[1].cb_arg2 = arg2;
	} else {
		printk("@@@@ Failed SC Callback Registration for %s\n", cbname);
		return -1;
	}
	return 0;
}
EXPORT_SYMBOL(register_simple_config_callback);

int32_t unregister_simple_config_callback(char *cbname)
{
	if (sccallback[1].name && strcmp(sccallback[1].name, cbname) == 0) {
		kfree(sccallback[1].name);
		sccallback[1].name = NULL;
		sccallback[1].registered_cb = NULL;
		sccallback[1].cb_arg1 = NULL;
		sccallback[1].cb_arg2 = NULL;
	} else if (sccallback[0].name && strcmp(sccallback[0].name, cbname) == 0) {
		kfree(sccallback[0].name);
		sccallback[0].name = NULL;
		sccallback[0].registered_cb = NULL;
		sccallback[0].cb_arg1 = NULL;
		sccallback[0].cb_arg2 = NULL;
	} else {
		printk("!&!&!&!& ERROR: Unknown callback name %s\n", cbname);
		return -1;
	}
	return 0;
}
EXPORT_SYMBOL(unregister_simple_config_callback);

static OS_TIMER_FUNC(wps_led_blink)
{
	static int WPSled = WPS_LED_ON;
	ath_gpio_out_val(WPS_LED_GPIO, WPSled);
	WPSled = !WPSled;
	OS_SET_TIMER(&os_timer_t, WPS_LED_IN_PROCESS_BLINK_INTERVAL);
}

static int atheros_wps_led_fail_sec = 0;
static OS_TIMER_FUNC(atheros_wps_led_fail)
{
        static int WPSled = WPS_LED_ON;
        ath_gpio_out_val(WPS_LED_GPIO, WPSled);
        WPSled = !WPSled;
        atheros_wps_led_fail_sec++;
        if (atheros_wps_led_fail_sec < WPS_LED_FAIL_BLINK_TIME_COUNT) {
                OS_SET_TIMER(&os_timer_t, WPS_LED_FAIL_BLINK_INTERVAL);
        } else {
                atheros_wps_led_fail_sec = 0;
                wps_led_blinking = 0;
                OS_CANCEL_TIMER(&os_timer_t);
#if defined(USE_WN_AG300DGR_LED) && SUPPORT_SYSTEM_OPERATION_MODE_LED_CTRL
                if( sys_opmode != SYS_OPM_CB )
                {
                    ath_gpio_out_val(WPS_LED_GPIO, WPS_LED_ON);
                }
                else
#endif
                {
#if defined(USE_WN_AG300DGR_LED) && !SUPPORT_SYSTEM_OPERATION_MODE_LED_CTRL
                    ath_gpio_out_val(WPS_LED_GPIO, WPS_LED_ON);
#else
                    ath_gpio_out_val(WPS_LED_GPIO, WPS_LED_OFF);
#endif
                }
        }
}

static int atheros_wps_led_pass_sec = 0;
static OS_TIMER_FUNC(atheros_wps_led_pass)
{
        static int WPSled = WPS_LED_ON;

        ath_gpio_out_val(WPS_LED_GPIO, WPSled);
#if WPS_LED_SUCCESS_BLINK_INTERVAL < 300000
        /* WPS_LED_SUCCESS always on when WPS_LED_SUCCESS_BLINK_INTERVAL >= 300 seconds. */
        WPSled = !WPSled;
#endif
        atheros_wps_led_pass_sec++;
        if (atheros_wps_led_pass_sec < WPS_LED_SUCCESS_BLINK_TIME_COUNT) {
                OS_SET_TIMER(&os_timer_t, WPS_LED_SUCCESS_BLINK_INTERVAL);
                ath_gpio_out_val(WPS_LED_GPIO, WPS_LED_ON);
        } else {
                atheros_wps_led_pass_sec = 0;
                wps_led_blinking = 0;
                OS_CANCEL_TIMER(&os_timer_t);
                ath_gpio_out_val(WPS_LED_GPIO, WPS_LED_OFF);
        }
}

static OS_TIMER_FUNC(pwr_led_blink)
{
   	static int PWRled = 0;

#ifdef SENAO_GPIO_LED_POWER
   	ath_gpio_out_val(SENAO_GPIO_LED_POWER, PWRled);
#endif
    
   	PWRled = !PWRled;

	OS_SET_TIMER(&os_reset_timer_t, PWR_LED_BLINK_INTERVAL/*ms*/);
}

#if defined(PWR_LED_ACT_1_ON_INTERVAL) && defined(PWR_LED_ACT_1_OFF_INTERVAL)
/* POWER LED ACTION 1 */
static OS_TIMER_FUNC(pwr_led_act_1_blink)
{
   	static int PWRled = 0;

   	ath_gpio_out_val(SENAO_GPIO_LED_POWER, PWRled);
    
	OS_SET_TIMER(&os_reset_timer_t, PWRled == _LED_ON ? PWR_LED_ACT_1_ON_INTERVAL : PWR_LED_ACT_1_OFF_INTERVAL/*ms*/);

   	PWRled = !PWRled;
}
#endif

#if defined (BLINKING_WPS_LED_TIME) && defined (SENAO_GPIO_BUTTON_WPS)
/* WPS LED blink */
static OS_TIMER_FUNC(wps_led_blink_cb)
{
   	static int WPSled = 0;
    ath_gpio_out_val(SENAO_GPIO_LED_WPS, WPSled);
    
   	WPSled=!WPSled;
#ifdef STATIC_WPS_LED
	OS_SET_TIMER(&os_wps_led_timer_t, 65535*1000);
#else
	OS_SET_TIMER(&os_wps_led_timer_t, WPS_LED_IN_PROCESS_BLINK_INTERVAL/*ms*/);
#endif
}

#if defined (BLINKING_WPS_LED_STOP_TIME) && defined (SENAO_GPIO_BUTTON_WPS)
static OS_TIMER_FUNC(wps_stop_led_blink_cb)
{
    OS_CANCEL_TIMER(&os_wps_led_timer_t);
#if defined (BLINKING_WPS2_LED_TIME) && defined (SENAO_GPIO_BUTTON_WPS)
    OS_CANCEL_TIMER(&os_wps2_led_timer_t);
#endif
    ath_gpio_out_val(SENAO_GPIO_LED_WPS, _LED_OFF);
#if defined (BLINKING_WPS2_LED_TIME) && defined (SENAO_GPIO_BUTTON_WPS)
    ath_gpio_out_val(SENAO_GPIO_LED_WPS2, _LED_OFF);
#endif
}
#endif

#if defined (BLINKING_WPS2_LED_TIME) && defined (SENAO_GPIO_BUTTON_WPS)
/* WPS2 LED blink */
static OS_TIMER_FUNC(wps2_led_blink_cb)
{
    static int WPSled = 0;
    ath_gpio_out_val(SENAO_GPIO_LED_WPS2, WPSled);

    WPSled=!WPSled;
#ifdef STATIC_WPS_LED
    OS_SET_TIMER(&os_wps2_led_timer_t, 65535*1000);
#else
    OS_SET_TIMER(&os_wps2_led_timer_t, WPS_LED_IN_PROCESS_BLINK_INTERVAL/*ms*/);
#endif
}

static OS_TIMER_FUNC(wps2_start_led_blink_cb)
{
    OS_CANCEL_TIMER(&os_wps_led_timer_t);
    OS_CANCEL_TIMER(&os_wps2_led_timer_t);

	// depend on eco mode
    ath_gpio_out_val(SENAO_GPIO_LED_WPS, sn_led_gpio_data[SENAO_GPIO_LED_WPS].eco_led_state == ECO_LED_ACTION_OFF ? _LED_OFF : _LED_ON);

    OS_INIT_TIMER(NULL, &os_wps2_led_timer_t, wps2_led_blink_cb, &os_wps2_led_timer_t);

    OS_SET_TIMER(&os_wps2_led_timer_t, WPS_LED_IN_PROCESS_BLINK_INTERVAL/*ms*/);
}
#endif
#endif //defined (BLINKING_WPS_LED_TIME) && defined (SENAO_GPIO_BUTTON_WPS)

#if defined (BLINKING_OP_2_LED_TIME) && defined (SENAO_GPIO_BUTTON_OP_2)
/* OP 2 LED BLINK */
static OS_TIMER_FUNC(op_2_led_blink)
{
   	static int op_2_led = 0;
    ath_gpio_out_val(SENAO_GPIO_LED_OP2, op_2_led);
    
   	op_2_led=!op_2_led;
	OS_SET_TIMER(&os_op_2_timer_t, OP_2_LED_BLINK_INTERVAL/*ms*/);
}
#endif

#if defined (IODATA_ECO_LED_SPECIAL_ACTION) && defined (SENAO_GPIO_LED_ECO) && defined (SENAO_GPIO_BUTTON_ECO) && SUPPORT_SN_LED_GPIO_CTRL
/* IODATA */
/* ECO LED :
   1. ON & Blink --> OFF
   2. OFF --> ON
 */
static OS_TIMER_FUNC(iodata_eco_led_special_action)
{
	if(sn_led_gpio_data[SENAO_GPIO_LED_ECO].action == LED_ACTION_OFF)
	{
		sn_led_gpio_set(SENAO_GPIO_LED_ECO, LED_ACTION_ON);
	}
	else if(sn_led_gpio_data[SENAO_GPIO_LED_ECO].action == LED_ACTION_ON || sn_led_gpio_data[SENAO_GPIO_LED_ECO].action == LED_ACTION_CUSTOM_BLINK_1)
	{
		sn_led_gpio_set(SENAO_GPIO_LED_ECO, LED_ACTION_OFF);
	}
}
#endif

#ifdef USE_SITECOM_GPIO_LED

void ath_gpio_pwr_blinking(void)
{
        unsigned char _offset_ = (SENAO_GPIO_LED_POWER%4) *8;

        if (SENAO_GPIO_LED_WLAN >=16 && SENAO_GPIO_LED_WLAN<=19)
        {
            ath_reg_wr(ATH_GPIO_OUT_FUNCTION4, ( ath_reg_rd(ATH_GPIO_OUT_FUNCTION4) & (~(0xff << _offset_))) | (0x2f << _offset_) );
        }
}

void wps_led_start_blinking(void)
{
	OS_CANCEL_TIMER(&os_wps_led_timer_t);
	OS_INIT_TIMER(NULL, &os_wps_led_timer_t, wps_led_blink_cb, &os_wps_led_timer_t);
	OS_SET_TIMER(&os_wps_led_timer_t,0);
}

#endif

#if 0
int ath_simple_config_invoke_cb(int simplecfg_only, int irq_enable, int cpl)
{
	printk("%s: sc %d, irq %d, ignorepb %d, jiffies %lu\n", __func__,
		simplecfg_only, irq_enable, ignore_pushbutton, jiffies);
	if (simplecfg_only) {
		if (ignore_pushbutton) {
			ath_gpio_config_int(JUMPSTART_GPIO, INT_TYPE_LEVEL,
						INT_POL_ACTIVE_HIGH);
			ignore_pushbutton = 0;
			push_time = jiffies;
			return IRQ_HANDLED;
		}

		ath_gpio_config_int(JUMPSTART_GPIO, INT_TYPE_LEVEL,
					INT_POL_ACTIVE_LOW);
		ignore_pushbutton = 1;
	}

	if (irq_enable)
		local_irq_enable();

	if (push_time) {
		push_time = jiffies - push_time;
	}
	printk ("simple_config callback.. push dur in sec %d\n", push_time/HZ);

	if (sccallback[0].registered_cb) {
		if (sccallback[0].cb_arg2) {
			*(u_int32_t *)sccallback[0].cb_arg2 = push_time/HZ;
		}
		sccallback[0].registered_cb (cpl, sccallback[0].cb_arg1, NULL, sccallback[0].cb_arg2);
	}
	if (sccallback[1].registered_cb) {
		if (sccallback[1].cb_arg2) {
			*(u_int32_t *)sccallback[1].cb_arg2 = push_time/HZ;
		}
		sccallback[1].registered_cb (cpl, sccallback[1].cb_arg1, NULL, sccallback[1].cb_arg2);
	}

	return IRQ_HANDLED;
}
#endif //if 0end

/*notify wps event to user space*/
static void ath_gpio_notify_user(ath_btn_t *ptr_ath_btn)
{
    struct task_struct *task_ptr = NULL;
    struct pid *pid;

    //don't send any signal if pid is 0 or 1
	if ((int)(ptr_ath_btn->pid) < 2)
	{
		return;
	}

    pid = find_get_pid(ptr_ath_btn->pid);

    /*here is a trick that we use init_pid_ns*/
	task_ptr = get_pid_task(pid, PIDTYPE_PID);

	if (NULL == task_ptr) 
	{
		printk(": no registered process to notify\n");
		return;
	}
    #if 0 
    if(g_ath_btn.e_ath_btn_action == SN_ATH_BTN_FACTORY_MODE)
    {
	send_sig(SIGUSR2, task_ptr, 0);
    }
    else
    #endif
    {
	send_sig(SIGUSR1, task_ptr, 0);
    }
}

/*
 * Irq for front panel SW jumpstart switch
 * Connected to XSCALE through GPIO4
 */
#if 0
irqreturn_t jumpstart_irq(int cpl, void *dev_id)
{
	unsigned int delay;

	if (atomic_read(&ath_fr_status)) {
		local_irq_disable();

#define UDELAY_COUNT 4000

		for (delay = UDELAY_COUNT; delay; delay--) {
			if (ath_gpio_in_val(JUMPSTART_GPIO)) {
				break;
			}
			udelay(1000);
		}

		if (!delay) {
			atomic_dec(&ath_fr_status);
			/*
			 * since we are going to reboot the board, we
			 * don't need the interrupt handler anymore,
			 * so disable it.
			 */
			disable_irq(ATH_GPIO_IRQn(JUMPSTART_GPIO));
			wake_up(&ath_fr_wq);
			printk("\nath: factory configuration restored..\n");
			local_irq_enable();
			return IRQ_HANDLED;
		} else {
			return (ath_simple_config_invoke_cb
				(0, 1, cpl));
		}
	} else
		return (ath_simple_config_invoke_cb(1, 0, cpl));
}
#else //0

#define ATH_GIPO_IRQg(_irq)	((_irq)-ATH_GPIO_IRQ_BASE)
irqreturn_t button_irq(int cpl, void *dev_id, struct pt_regs *regs)
{
	int duration=0;

	if(g_ath_btn.e_ath_btn_action==SN_ATH_BTN_FACTORY_MODE)
	{
		if (ignore_pushbutton) {
			ignore_pushbutton = 0;
            ath_gpio_config_int (ATH_GIPO_IRQg(cpl), INT_TYPE_LEVEL, INT_POL_ACTIVE_HIGH);
		    g_ath_btn.gpio |= (1<< ATH_GIPO_IRQg(cpl));//push button
		    //ath_gpio_notify_user(&g_ath_btn);

			return IRQ_HANDLED;
		}
        ath_gpio_config_int (ATH_GIPO_IRQg(cpl), INT_TYPE_LEVEL, INT_POL_ACTIVE_LOW);
        ignore_pushbutton = 1;
	    g_ath_btn.gpio &= ~(1<< ATH_GIPO_IRQg(cpl));//release button
	    //ath_gpio_notify_user(&g_ath_btn);
        printk("you are in factory mode, button will be disabled\n");

		return IRQ_HANDLED;
	}

    printk ("\n%s: simple_config callback.. ignorepb %d, jiffies %lu, gpio %d\n", __func__, ignore_pushbutton, jiffies,ATH_GIPO_IRQg(cpl));

#if defined(SENAO_MODESWITCH_GPIO_L) || defined(SENAO_MODESWITCH_GPIO_M) || defined(SENAO_MODESWITCH_GPIO_R)
	/* Do reset/reboot when the user move switch. */
	if(sn_modeswitch_init == 1)
	{
		switch(ATH_GIPO_IRQg(cpl))
		{
#if defined(SENAO_MODESWITCH_GPIO_L)
		case SENAO_MODESWITCH_GPIO_L:
#endif
#if defined(SENAO_MODESWITCH_GPIO_M)
		case SENAO_MODESWITCH_GPIO_M:
#endif
#if defined(SENAO_MODESWITCH_GPIO_R)
		case SENAO_MODESWITCH_GPIO_R:
#endif
			if (ignore_pushbutton) 
			{
				ignore_pushbutton = 0;
				ath_gpio_config_int (ATH_GIPO_IRQg(cpl), INT_TYPE_LEVEL, INT_POL_ACTIVE_HIGH);
			}
			else
			{
				ignore_pushbutton = 1;
				ath_gpio_config_int (ATH_GIPO_IRQg(cpl), INT_TYPE_LEVEL, INT_POL_ACTIVE_LOW);
			}
			g_ath_btn.e_ath_btn_action = SN_ATH_BTN_RESET;
			goto button_irq_exit;
		default:
			break;
		}
	}
#endif //defined(SENAO_MODESWITCH_GPIO_L) || defined(SENAO_MODESWITCH_GPIO_M) || defined(SENAO_MODESWITCH_GPIO_R)

	if (ignore_pushbutton) {
		ath_gpio_config_int (ATH_GIPO_IRQg(cpl), INT_TYPE_LEVEL, INT_POL_ACTIVE_HIGH);
		g_ath_btn.gpio |= (1<< ATH_GIPO_IRQg(cpl));//push button
#if defined(SENAO_MODESWITCH_GPIO_L) || defined(SENAO_MODESWITCH_GPIO_M) || defined(SENAO_MODESWITCH_GPIO_R)
		/* 2012-05-16, ModeSwitch will detect press-down when initial. We ignore this press-down.
		 * So We handle pushbotton, after finish initial.  */
		if(sn_modeswitch_init == 1)
#endif
		{
			ignore_pushbutton = 0;
#ifdef USE_SITECOM_GPIO_LED
			if(count_ignorepb ==0 )
			{
				count_ignorepb =1;
				start_time= jiffies;
			}
			else if(count_ignorepb ==1)
			{
			printk("\n\n Now time = %lu  - %lu = %lu   =========",jiffies,start_time,(jiffies - start_time));
				if((jiffies - start_time) < 50)
				{				
					count_ignorepb =2 ;
					/* power LED will blink after 10 seconds*/
					OS_CANCEL_TIMER(&os_reset_timer_t);
					OS_INIT_TIMER(NULL, &os_reset_timer_t, pwr_led_blink, &os_reset_timer_t);
					OS_SET_TIMER(&os_reset_timer_t, 10*1000);
				}
				else
				{
					count_ignorepb = 1;
					start_time = 0;
				}
			}
			else if(count_ignorepb ==2)
				count_ignorepb =3;
			else
				count_ignorepb =0;

			printk("\ncount_ignorepb = %d\n",count_ignorepb);
#endif
		}
		push_time = jiffies;
		g_ath_btn.e_ath_btn_action=SN_ATH_BTN_INIT;

#if defined (BLINKING_PWR_LED_TIME) && defined (SENAO_GPIO_BUTTON_RESET)
		/*jaykung add a timer to blink power led*/
		if(cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_RESET))
		{
			OS_CANCEL_TIMER(&os_reset_timer_t);
			OS_INIT_TIMER(NULL, &os_reset_timer_t, pwr_led_blink, &os_reset_timer_t);
			OS_SET_TIMER(&os_reset_timer_t, BLINKING_PWR_LED_TIME*1000);
		}
#endif
#if defined (BLINKING_WPS_LED_TIME) && defined (SENAO_GPIO_BUTTON_WPS)
		/* Add a timer to blink WPS LED */
		if(cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_WPS))
		{
#ifndef USE_SITECOM_GPIO_LED
#if DUAL_RADIO_WPS_BEHAVIOR_FOR_WPS_V2
			wps_hw_button_press = 1;
			OS_CANCEL_TIMER(&os_wps_led_timer_t);
                        OS_INIT_TIMER(NULL, &os_wps_led_timer_t, wps_led_blink_cb, &os_wps_led_timer_t);
                        OS_SET_TIMER(&os_wps_led_timer_t, BLINKING_WPS_LED_TIME*1000);
#if defined (BLINKING_WPS_LED_STOP_TIME)
                        OS_CANCEL_TIMER(&os_wps_led_off_timer_t);
                        OS_INIT_TIMER(NULL, &os_wps_led_off_timer_t, wps_stop_led_blink_cb, &os_wps_led_off_timer_t);
                        OS_SET_TIMER(&os_wps_led_off_timer_t, BLINKING_WPS_LED_STOP_TIME*1000);
#endif
#else
			OS_CANCEL_TIMER(&os_wps_led_timer_t);
			OS_INIT_TIMER(NULL, &os_wps_led_timer_t, wps_led_blink_cb, &os_wps_led_timer_t);
			OS_SET_TIMER(&os_wps_led_timer_t, BLINKING_WPS_LED_TIME*1000);
#if defined (BLINKING_WPS2_LED_TIME)
			OS_CANCEL_TIMER(&os_wps2_led_timer_t);
			OS_INIT_TIMER(NULL, &os_wps2_led_timer_t, wps2_start_led_blink_cb, &os_wps2_led_timer_t);
			OS_SET_TIMER(&os_wps2_led_timer_t, BLINKING_WPS2_LED_TIME*1000);
#endif
#if defined (BLINKING_WPS_LED_STOP_TIME)
			OS_CANCEL_TIMER(&os_wps_led_off_timer_t);
                        OS_INIT_TIMER(NULL, &os_wps_led_off_timer_t, wps_stop_led_blink_cb, &os_wps_led_off_timer_t);
                        OS_SET_TIMER(&os_wps_led_off_timer_t, BLINKING_WPS_LED_STOP_TIME*1000);
#endif
#endif
#endif
		}
#endif //defined (BLINKING_WPS_LED_TIME) && defined (SENAO_GPIO_BUTTON_WPS)
#if defined (BLINKING_WPS_LED_TIME) && defined (SENAO_GPIO_BUTTON_WPS1)
		/* Add a timer to blink WPS LED */
		if(cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_WPS1))
		{
#ifndef USE_SITECOM_GPIO_LED 
			OS_CANCEL_TIMER(&os_wps_led_timer_t);
			OS_INIT_TIMER(NULL, &os_wps_led_timer_t, wps_led_blink_cb, &os_wps_led_timer_t);
			OS_SET_TIMER(&os_wps_led_timer_t, BLINKING_WPS_LED_TIME*1000);
#if defined (BLINKING_WPS2_LED_TIME)
			OS_CANCEL_TIMER(&os_wps2_led_timer_t);
			OS_INIT_TIMER(NULL, &os_wps2_led_timer_t, wps2_start_led_blink_cb, &os_wps2_led_timer_t);
			OS_SET_TIMER(&os_wps2_led_timer_t, BLINKING_WPS2_LED_TIME*1000);
#endif
#if defined (BLINKING_WPS_LED_STOP_TIME)
			OS_CANCEL_TIMER(&os_wps_led_off_timer_t);
                        OS_INIT_TIMER(NULL, &os_wps_led_off_timer_t, wps_stop_led_blink_cb, &os_wps_led_off_timer_t);
                        OS_SET_TIMER(&os_wps_led_off_timer_t, BLINKING_WPS_LED_STOP_TIME*1000);
#endif
#endif
		}
#endif //defined (BLINKING_WPS_LED_TIME) && defined (SENAO_GPIO_BUTTON_WPS)
#if defined (BLINKING_PWR_LED_RESET2DEF_TIME) && defined (SENAO_GPIO_BUTTON_WPS)
		if(cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_WPS))
		{
			OS_CANCEL_TIMER(&os_reset_timer_t);
			OS_INIT_TIMER(NULL, &os_reset_timer_t, pwr_led_act_1_blink, &os_reset_timer_t);
			OS_SET_TIMER(&os_reset_timer_t, BLINKING_PWR_LED_RESET2DEF_TIME*1000);

		}
#endif
#if defined (BLINKING_OP_2_LED_TIME) && defined (SENAO_GPIO_BUTTON_OP_2)
		if(cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_OP_2))
		{
			OS_CANCEL_TIMER(&os_op_2_timer_t);
			OS_INIT_TIMER(NULL, &os_op_2_timer_t, op_2_led_blink, &os_op_2_timer_t);
			OS_SET_TIMER(&os_op_2_timer_t, BLINKING_OP_2_LED_TIME*1000);
		}
#endif
#if defined (IODATA_ECO_LED_SPECIAL_ACTION) && defined (SENAO_GPIO_LED_ECO) && defined (SENAO_GPIO_BUTTON_ECO)
		if(cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_ECO))
		{
			OS_CANCEL_TIMER(&iodata_eco_timer_t);
			OS_INIT_TIMER(NULL, &iodata_eco_timer_t, iodata_eco_led_special_action, &iodata_eco_timer_t);
			OS_SET_TIMER(&iodata_eco_timer_t, IODATA_ECO_LED_SPECIAL_ACTION*1000);
		}
#endif

		return IRQ_HANDLED;
	}

	ath_gpio_config_int (ATH_GIPO_IRQg(cpl), INT_TYPE_LEVEL, INT_POL_ACTIVE_LOW);
	ignore_pushbutton = 1;
	g_ath_btn.gpio &= ~(1<< ATH_GIPO_IRQg(cpl));//release button

    if (push_time) {
        push_time = jiffies - push_time;
		duration = push_time;
    }

#if defined (BLINKING_PWR_LED_TIME) && defined (SENAO_GPIO_BUTTON_RESET)
	/*jaykung cancel timer of blink power led*/
	if(cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_RESET))
	{
		/* 2012-04-27, When user release button, LED still Blink. */
		if(duration<=(HZ*BLINKING_PWR_LED_TIME))
		{
			OS_CANCEL_TIMER(&os_reset_timer_t);
		}
	}
#endif
#if defined (BLINKING_WPS_LED_TIME) && defined (SENAO_GPIO_BUTTON_WPS)
	/* Add a timer to blink WPS LED */
	if((cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_WPS))
 #if defined (SENAO_GPIO_BUTTON_WPS1)	
		||(cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_WPS1))
#endif
		)
	{
		OS_CANCEL_TIMER(&os_wps_led_timer_t);
#ifdef STATIC_WPS_LED
		ath_gpio_out_val(SENAO_GPIO_LED_WPS, _LED_OFF);
#endif
#if defined (BLINKING_WPS2_LED_TIME)
		OS_CANCEL_TIMER(&os_wps2_led_timer_t);
#ifdef STATIC_WPS_LED
		ath_gpio_out_val(SENAO_GPIO_LED_WPS2, _LED_OFF);
#endif
#endif // defined (BLINKING_WPS2_LED_TIME)
#if defined (BLINKING_WPS_LED_STOP_TIME) && defined (SENAO_GPIO_BUTTON_WPS)
		OS_CANCEL_TIMER(&os_wps_led_off_timer_t);
#endif
	}
#endif //defined (BLINKING_WPS_LED_TIME) && defined (SENAO_GPIO_BUTTON_WPS)

#if defined (BLINKING_PWR_LED_RESET2DEF_TIME) && defined (SENAO_GPIO_BUTTON_WPS)
	if(cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_WPS))
	{
		/* 2012-04-27, When user release button, LED still Blink. */
		if(duration<=(HZ*BLINKING_PWR_LED_RESET2DEF_TIME))
		{
			OS_CANCEL_TIMER(&os_reset_timer_t);
		}
	}
#endif
#if defined (BLINKING_OP_2_LED_TIME) && defined (SENAO_GPIO_BUTTON_OP_2)
	if(cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_OP_2))
	{
		OS_CANCEL_TIMER(&os_op_2_timer_t);
	}
#endif
#if defined (IODATA_ECO_LED_SPECIAL_ACTION) && defined (SENAO_GPIO_LED_ECO) && defined (SENAO_GPIO_BUTTON_ECO)
	if(cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_ECO))
	{
		OS_CANCEL_TIMER(&iodata_eco_timer_t);
	}
#endif

    printk ("\nath: calling simple_config callback.. gpio:%d,push dur in sec %d/%d\n",ATH_GIPO_IRQg(cpl), duration, HZ);

#ifdef SENAO_GPIO_BUTTON_WPS
	if(cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_WPS))
	{
		g_ath_btn.e_ath_btn_type=SN_ATH_BUTTON_TYPE_WPS;

#ifdef USE_SITECOM_GPIO_LED
		if(count_ignorepb==3)
		{
			printk("\n 2.4G (jiffies - start_time) = %lu\n",(jiffies - start_time));
			if((jiffies - start_time) >  ((HZ)*BUTTON_PROBE_RESET2DEFAULT_TIME)) 
			{
				printk("\n Push over 10 sec : reset to default\n");
				g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_1;
			}
			else
			{
				OS_CANCEL_TIMER(&os_reset_timer_t);
				g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_4;
			}

			count_ignorepb = start_time =0;
		}
		else if(count_ignorepb==2)
		{
			//printk("\n Waiting for next ignorepb\n");
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_4;
		}
		else if(count_ignorepb==1)
		{
			//count_ignorepb = 1 or 0
			if (	(WPS_in_process ==0) && (WPS_24G_is_disable ==0) &&
				(duration>(int)((HZ)*BUTTON_PROBE_WPS_TIME_0)) && (duration<=(HZ*BUTTON_PROBE_WPS_TIME_1)))
			{
				/* 5G LED OFF */
				// can not use ath_gpio_out_val(SENAO_GPIO_LED_WLAN2, _LED_OFF);
				// because 5g wifi driver control this GPIO too.
				ath_gpio_config_input(SENAO_GPIO_LED_WLAN2);
				printk("\n Push1~ 5 sec : Trigger 2.4G WPS\n");
				g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_2;
				wps_led_start_blinking();
				//WPS can not be interrupted, or LED behavior will be wrong.
				WPS_in_process =1;
			}
			else
			{
				printk("\n Push over 5 sec  or less then 1 sec : Do nothing\n");
				g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_4;
			}
			if(duration > 50)
			{
				//clear flags, start next cycle
				count_ignorepb = start_time =0;
				printk("\n duration > 50, clean flags");
			}
			OS_CANCEL_TIMER(&os_reset_timer_t);
		}

#else
#if DUAL_RADIO_WPS_BEHAVIOR_FOR_WPS_V2
		/* 20131015 austin : for wps2.0 spec , we need to trigger 2.4g & 5g wps process at the same time.*/
		if ( (duration>(int)((HZ)*BUTTON_PROBE_WPS_TIME_0)) && (duration<=(HZ*BUTTON_PROBE_WPS_TIME_1)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_1;
		}
		else if ( (duration>(HZ*BUTTON_PROBE_WPS_TIME_1)) && (duration<=(HZ*BUTTON_PROBE_WPS_TIME_3)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_4;
		}

#else
		if ( (duration>(int)((HZ)*BUTTON_PROBE_WPS_TIME_0)) && (duration<=(HZ*BUTTON_PROBE_WPS_TIME_1)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_1;
		}
		else if ( (duration>(HZ*BUTTON_PROBE_WPS_TIME_1)) && (duration<=(HZ*BUTTON_PROBE_WPS_TIME_2)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_2;
		}
		else if ( (duration>(HZ*BUTTON_PROBE_WPS_TIME_2)) && (duration<=(HZ*BUTTON_PROBE_WPS_TIME_3)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_3;
		}
		else if ( (duration>(HZ*BUTTON_PROBE_WPS_TIME_3)) && (duration<=(HZ*BUTTON_PROBE_WPS_TIME_4)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_4;
		}
		else if ( (duration>(HZ*BUTTON_PROBE_WPS_TIME_4)) && (duration<=(HZ*BUTTON_PROBE_WPS_TIME_5)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_5;	
		}
#endif
#endif
	}
	else
#endif
#ifdef SENAO_GPIO_BUTTON_WPS1
#ifdef USE_SITECOM_GPIO_LED
	if(cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_WPS1))
	{
		g_ath_btn.e_ath_btn_type=SN_ATH_BUTTON_TYPE_WPS;

		if(count_ignorepb==3)
		{
			printk("\n5G (jiffies - start_time) = %lu\n",(jiffies - start_time));
			if((jiffies - start_time) >  (int)((HZ)*BUTTON_PROBE_RESET2DEFAULT_TIME)) 
			{
				printk("\n Push over 10 sec : reset to default\n");
				g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_1;
			}
			else
				{
					OS_CANCEL_TIMER(&os_reset_timer_t);
					g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_4;
				}

			count_ignorepb = start_time =0;
		}
		else if(count_ignorepb==2)
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_4;
			//printk("\n Waiting for next ignorepb\n");
		}
		else if(count_ignorepb==1)
		{
			if ((WPS_in_process ==0) && (WPS_5G_is_disable ==0) &&
				(duration>(int)((HZ)*BUTTON_PROBE_WPS_TIME_0)) && (duration<=(HZ*BUTTON_PROBE_WPS_TIME_1)))
			{
				/* 2.4G LED OFF */
				ath_gpio_out_val(SENAO_GPIO_LED_WLAN, _LED_OFF);
				//tell wifi driver don't control 2.4G wifi led
				 _wlan_led_off_ = 1;
				printk("\n 1~ 5 sec : Trigger 5G WPS\n");
				g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_3;
				wps_led_start_blinking();
				//WPS can not be interrupted, or LED behavior will be wrong.
				WPS_in_process =1;
			}
			else
			{
				printk("\n Push over 5 sec  or less then 1 sec : Do nothing\n");
				g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_4;
			}
			if(duration > 50)
			{
				//clear flags, start next cycle
				count_ignorepb = start_time =0;
				printk("\n duration > 50, clean flags");
			}
			OS_CANCEL_TIMER(&os_reset_timer_t);
		}
	}
	else
#endif
#endif
#ifdef SENAO_GPIO_BUTTON_RESET
	if(cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_RESET))
	{
		g_ath_btn.e_ath_btn_type=SN_ATH_BUTTON_TYPE_RESET;

		if ( (duration>(int)((HZ)*BUTTON_PROBE_RESET_TIME_0)) && (duration<=(HZ*BUTTON_PROBE_RESET_TIME_1)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_RESET_ACTION_1;
		}
		else if ( (duration>(HZ*BUTTON_PROBE_RESET_TIME_1)) && (duration<=(HZ*BUTTON_PROBE_RESET_TIME_2)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_RESET_ACTION_2;
		}
		else if ( (duration>(HZ*BUTTON_PROBE_RESET_TIME_2)) && (duration<=(HZ*BUTTON_PROBE_RESET_TIME_3)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_RESET_ACTION_3;
		}
		else if ( (duration>(HZ*BUTTON_PROBE_RESET_TIME_3)) && (duration<=(HZ*BUTTON_PROBE_RESET_TIME_4)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_RESET_ACTION_4;
		}
		else if ( (duration>(HZ*BUTTON_PROBE_RESET_TIME_4)) && (duration<=(HZ*BUTTON_PROBE_RESET_TIME_5)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_RESET_ACTION_5;	
		}
	}
	else
#endif
#ifdef SENAO_GPIO_BUTTON_OP_1
	if(cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_OP_1))
	{
		g_ath_btn.e_ath_btn_type=SN_ATH_BUTTON_TYPE_OP1;

		if ( (duration>(int)((HZ)*BUTTON_PROBE_BUTTON_OP1_TIME_0)) && (duration<=(HZ*BUTTON_PROBE_BUTTON_OP1_TIME_1)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_BUTTON_OP1_ACTION_1;
		}
		else if ( (duration>(HZ*BUTTON_PROBE_BUTTON_OP1_TIME_1)) && (duration<=(HZ*BUTTON_PROBE_BUTTON_OP1_TIME_2)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_BUTTON_OP1_ACTION_2;
		}
		else if ( (duration>(HZ*BUTTON_PROBE_BUTTON_OP1_TIME_2)) && (duration<=(HZ*BUTTON_PROBE_BUTTON_OP1_TIME_3)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_BUTTON_OP1_ACTION_3;
		}
	}
	else
#endif
#ifdef SENAO_GPIO_BUTTON_OP_2
	if(cpl==ATH_GPIO_IRQn(SENAO_GPIO_BUTTON_OP_2))
	{
		g_ath_btn.e_ath_btn_type=SN_ATH_BUTTON_TYPE_OP2;

		if ( (duration>(int)((HZ)*BUTTON_PROBE_BUTTON_OP2_TIME_0)) && (duration<=(HZ*BUTTON_PROBE_BUTTON_OP2_TIME_1)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_BUTTON_OP2_ACTION_1;
		}
		else if ( (duration>(HZ*BUTTON_PROBE_BUTTON_OP2_TIME_1)) && (duration<=(HZ*BUTTON_PROBE_BUTTON_OP2_TIME_2)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_BUTTON_OP2_ACTION_2;
		}
		else if ( (duration>(HZ*BUTTON_PROBE_BUTTON_OP2_TIME_2)) && (duration<=(HZ*BUTTON_PROBE_BUTTON_OP2_TIME_3)) )
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_BUTTON_OP2_ACTION_3;
		}
	}
	else
#endif
    {
        printk("unknow cpl:%d\n",cpl);
    }


button_irq_exit:

	/*notify this event*/
	if (g_ath_btn.e_ath_btn_action!=SN_ATH_BTN_INIT)
	{
		printk("ath_btn: type:%d, action:%d, pid=%d\n", g_ath_btn.e_ath_btn_type, g_ath_btn.e_ath_btn_action, g_ath_btn.pid);
 		ath_gpio_notify_user(&g_ath_btn);
        /* 2012-08-08 Norkay
		 * Normal case :
		 * press button ---> SN_ATH_BTN_INIT ---> release button ---> BUTTON_PROBE_XXX_ACTION_X --SIGUSR1--> config_term ---> get BUTTON_PROBE_XXX_ACTION_X from push_button ---> sysconfd
		 * Worse case :
		 * press button ---> SN_ATH_BTN_INIT ---> release button ---> BUTTON_PROBE_XXX_ACTION_X --SIGUSR1--> config_term -----------> get SN_ATH_BTN_INIT from push_button --/--> sysconfd
		 *                                                                                       |-->  press button ---> SN_ATH_BTN_INIT --->
		 * Avoid worse case, so we add a little delay after ath_gpio_notify_user()
		 */
		udelay(10000);
	}


	return IRQ_HANDLED;
}
#endif //if 0 end

static int push_button_read(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int len;

	//printk("%s: \n",__func__);	
	if(g_ath_btn.e_ath_btn_action == SN_ATH_BTN_FACTORY_MODE)
	    len = sprintf(page, "%d %d\n", g_ath_btn.e_ath_btn_action, g_ath_btn.gpio);
	else
		len = sprintf(page, "%d %d\n", g_ath_btn.e_ath_btn_action, g_ath_btn.gpio);

	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len > count) len = count;
	if (len < 0) len = 0;
	return len;
}

static int push_button_write(struct file *file, const char *buf,
				unsigned long count, void *data)
{
    int num, state, pid;
	char flag[20];

	//printk("%s: \n",__func__);
	if (count < 2)
		return -EFAULT;

	if (buf && !copy_from_user(&flag, buf, 20)) 
	{
	    num = sscanf(flag, "%d %d", &state, &pid);
		
		if (num != 2) 
		{
			printk("invalid gpio parameter!\n");
		}
		//printk("state=%d\n",state);
        if (state==SN_ATH_BTN_INIT)
        {
	        g_ath_btn.e_ath_btn_action=SN_ATH_BTN_INIT;
	        g_ath_btn.pid = pid;
		}
		else if(state ==SN_ATH_BTN_FACTORY_MODE) /*in factory mode, button should be disabled*/
		{
	       	g_ath_btn.e_ath_btn_action=SN_ATH_BTN_FACTORY_MODE;
			g_ath_btn.pid = pid;
            /*pwr led blinking*/
			OS_CANCEL_TIMER(&os_reset_timer_t);
			OS_INIT_TIMER(NULL, &os_reset_timer_t, pwr_led_blink, &os_reset_timer_t);
			OS_SET_TIMER(&os_reset_timer_t, 0);
		}
		return count;
	}
	else
	{
		return -EFAULT;
	}
}

typedef enum {
	LED_STATE_OFF = 1,
	LED_STATE_ON = 2,
	LED_STATE_BLINKING = 3,
} led_state_e;

static led_state_e simple_config_led_state = LED_STATE_OFF;

static int gpio_simple_config_led_read(char *page, char **start, off_t off,
					int count, int *eof, void *data)
{
	//printk("%s: \n",__func__);
	return sprintf(page, "%d\n", simple_config_led_state);
}

#if 0 // no use
static int gpio_simple_config_led_write(struct file *file, const char *buf,
                                        unsigned long count, void *data)
{
    u_int32_t val, green_led_onoff = 0, yellow_led_onoff = 0;

        if (sscanf(buf, "%d", &val) != 1)
                return -EINVAL;

        if ((val == SIMPLE_CONFIG_BLINK) && !wps_led_blinking) { /* wps LED blinking */
                wps_led_blinking = 1;
                simple_config_led_state = SIMPLE_CONFIG_BLINK;
                ath_gpio_out_val(WPS_LED_GPIO, WPS_LED_ON);
                OS_INIT_TIMER(NULL, &os_timer_t, wps_led_blink, &os_timer_t);
                OS_SET_TIMER(&os_timer_t, 1000);
        } else if (val == SIMPLE_CONFIG_ON) {   /* WPS Success */
                wps_led_blinking = 0;
                simple_config_led_state = SIMPLE_CONFIG_ON;
                OS_CANCEL_TIMER(&os_timer_t);
                ath_gpio_out_val(WPS_LED_GPIO, WPS_LED_ON);
        } else if (val == SIMPLE_CONFIG_OFF) {  /* WPS failed */
                wps_led_blinking = 0;
		simple_config_led_state = SIMPLE_CONFIG_OFF;
                OS_CANCEL_TIMER(&os_timer_t);
                ath_gpio_out_val(WPS_LED_GPIO, WPS_LED_OFF);
        }
	
	return count;
}
#else //0

static int atheros_gpio_simple_config_led_write(struct file *file, const char *buf,
					unsigned long count, void *data)
{
    u_int32_t val, green_led_onoff = 0, yellow_led_onoff = 0;
#if SUPPORT_SN_LED_GPIO_CTRL
	u_int32_t sn_gpio, sn_val;
#endif

	//printk("%s: \n",__func__);
	if (sscanf(buf, "%d", &val) != 1)
		return -EINVAL;
	
	//printk("val =%d\n",val);
    switch(val)
    {
#if SUPPORT_SN_LED_GPIO_CTRL    
	case SIMPLE_CONFIG_LED_CTRL:
		{
			if (sscanf(buf, "%*d %d %d", &sn_gpio, &sn_val) != 2)
				return -EINVAL;
				//printk("sn_gpio =%d,sn_val=%d\n",sn_gpio,sn_val);
			sn_led_gpio_set(sn_gpio, sn_val);
		}
		break;
#endif

#if SUPPORT_SYSTEM_OPERATION_MODE_LED_CTRL
    case CONFIG_SYSTEM_OPERATION_MODE: /* opmode set by systemsetting */
        if (sscanf(buf, "%*d %d", &sys_opmode) != 1)
            return -EINVAL;
        break;
#endif

    case SIMPLE_CONFIG_BLINK: /* wps LED blinking */
        if(!wps_led_blinking)
        {
            wps_led_blinking = 1;
            simple_config_led_state = SIMPLE_CONFIG_BLINK;
            ath_gpio_out_val(WPS_LED_GPIO, WPS_LED_ON);
            OS_CANCEL_TIMER(&os_timer_t);
            OS_INIT_TIMER(NULL, &os_timer_t, wps_led_blink, &os_timer_t);
            OS_SET_TIMER(&os_timer_t, 1000);
        }
        break;
    case SIMPLE_CONFIG_ON: /* WPS_led_on */
		wps_led_blinking = 0;
		simple_config_led_state = SIMPLE_CONFIG_ON;
		OS_CANCEL_TIMER(&os_timer_t);
		atheros_wps_led_fail_sec = 0;
		atheros_wps_led_pass_sec = 0;
		ath_gpio_out_val(WPS_LED_GPIO, WPS_LED_ON);
        break;
    case SIMPLE_CONFIG_OFF: /* WPS_led off */
		wps_led_blinking = 0;
		simple_config_led_state = SIMPLE_CONFIG_OFF;
		OS_CANCEL_TIMER(&os_timer_t);
		OS_CANCEL_TIMER(&os_wps_led_timer_t);
		ath_gpio_out_val(SENAO_GPIO_LED_WPS, _LED_OFF);
#ifdef USE_SITECOM_GPIO_LED
		WPS_in_process =0;
		_wlan_led_off_=0;
		//recover 2.4G 5G wifi LED
		ath_gpio_out_val(SENAO_GPIO_LED_WLAN, _LED_ON);
		ath_reg_rmw_clear(ATH_GPIO_OE, (1 << SENAO_GPIO_LED_WLAN2));
		ath_gpio_out_val(SENAO_GPIO_LED_WLAN2, _LED_ON);
#endif
        break;
    case ATHEROS_WPS_FAIL: /* ATHEROS_WPS failed */
        wps_led_blinking = 0;
        simple_config_led_state = ATHEROS_WPS_FAIL;
        ath_gpio_out_val(WPS_LED_GPIO, WPS_LED_ON);
        OS_CANCEL_TIMER(&os_timer_t);
        OS_INIT_TIMER(NULL, &os_timer_t, atheros_wps_led_fail, &os_timer_t);
        OS_SET_TIMER(&os_timer_t, 250);
        break;
    case ATHEROS_WPS_PASS: /* ATHEROS_WPS pass */
        wps_led_blinking = 0;
        simple_config_led_state = ATHEROS_WPS_PASS;
        ath_gpio_out_val(WPS_LED_GPIO, WPS_LED_ON);
        OS_CANCEL_TIMER(&os_timer_t);
        OS_INIT_TIMER(NULL, &os_timer_t, atheros_wps_led_pass, &os_timer_t);
        OS_SET_TIMER(&os_timer_t, 1000);
        break;

#if SIMPLE_CONFIG_SUPPORT_PWR_LED_CONTROL
    case SIMPLE_CONFIG_PWR_LED_ACT_0:
        OS_CANCEL_TIMER(&os_reset_timer_t);
#ifdef SENAO_GPIO_LED_POWER
        ath_gpio_out_val(SENAO_GPIO_LED_POWER, _LED_ON);
#endif
        break;
#if defined(PWR_LED_ACT_1_ON_INTERVAL) && defined(PWR_LED_ACT_1_OFF_INTERVAL)
    case SIMPLE_CONFIG_PWR_LED_ACT_1:
        OS_CANCEL_TIMER(&os_reset_timer_t);
        OS_INIT_TIMER(NULL, &os_reset_timer_t, pwr_led_act_1_blink, &os_reset_timer_t);
        OS_SET_TIMER(&os_reset_timer_t, 0);
        break;
#endif
    case SIMPLE_CONFIG_PWR_LED_ACT_2:
        OS_CANCEL_TIMER(&os_reset_timer_t);
#ifdef SENAO_GPIO_LED_POWER
        ath_gpio_out_val(SENAO_GPIO_LED_POWER, _LED_OFF);
 	udelay(1000000);
	ath_gpio_out_val(SENAO_GPIO_LED_POWER, _LED_ON);
#endif
	break;	
#endif

#if SIMPLE_CONFIG_SUPPORT_ETH_LED_CONTROL
    case SIMPLE_CONFIG_ETH_LED_ON:
    	ath_gpio_config_output(SENAO_GPIO_WAN_1_LED);
    	ath_gpio_config_output(SENAO_GPIO_LAN_1_LED);
    	ath_gpio_config_output(SENAO_GPIO_LAN_2_LED);
    	ath_gpio_config_output(SENAO_GPIO_LAN_3_LED);
    	ath_gpio_config_output(SENAO_GPIO_LAN_4_LED);
        break;
	case SIMPLE_CONFIG_ETH_LED_OFF:
    	ath_gpio_config_input(SENAO_GPIO_WAN_1_LED);
    	ath_gpio_config_input(SENAO_GPIO_LAN_1_LED);
    	ath_gpio_config_input(SENAO_GPIO_LAN_2_LED);
    	ath_gpio_config_input(SENAO_GPIO_LAN_3_LED);
    	ath_gpio_config_input(SENAO_GPIO_LAN_4_LED);
        break;
#endif

#if SUPPORT_SN_LED_GPIO_CTRL
	case SIMPLE_CONFIG_ECO_LED_OFF:
		if (sscanf(buf, "%*d %d", &sn_gpio) != 1)
			return -EINVAL;

		if (sn_led_gpio_data[sn_gpio].wps_state == WPS_INACTIVE)
		{
			ath_gpio_out_val(sn_gpio, _LED_OFF);
			sn_led_gpio_data[sn_gpio].val = 0;
		}
		sn_led_gpio_data[sn_gpio].eco_led_state = ECO_LED_ACTION_OFF;
		break;
	case SIMPLE_CONFIG_ECO_LED_ON:
		if (sscanf(buf, "%*d %d", &sn_gpio) != 1)
			return -EINVAL;

		if (sn_led_gpio_data[sn_gpio].wps_state == WPS_INACTIVE)
		{
			ath_gpio_out_val(sn_gpio, _LED_ON);
			sn_led_gpio_data[sn_gpio].val = 1;
		}
		sn_led_gpio_data[sn_gpio].eco_led_state = ECO_LED_ACTION_ON;
		break;
	case SIMPLE_CONFIG_ECO_WPS_STATUS_INACTIVE:
		if (sscanf(buf, "%*d %d", &sn_gpio) != 1)
			return -EINVAL;

		sn_led_gpio_data[sn_gpio].wps_state = WPS_INACTIVE;
		break;
#endif
#ifdef USE_SITECOM_GPIO_LED
	case SIMPLE_CONFIG_WPS_24G_DISABLE:
		printk("\n\nWPS_is_disable=1");
		 WPS_24G_is_disable=1;
		break;

	case SIMPLE_CONFIG_WPS_24G_ENABLE:
		printk("\n\nWPS_is_disable=0");
		 WPS_24G_is_disable=0;
		break;

	case SIMPLE_CONFIG_WPS_5G_DISABLE:
		printk("\n\nWPS_is_disable=1");
		 WPS_5G_is_disable=1;
		break;

	case SIMPLE_CONFIG_WPS_5G_ENABLE:
		printk("\n\nWPS_is_disable=0");
		 WPS_5G_is_disable=0;
		break;

#endif
		default:
			printk("unknow val =%d\n",val);
		break;
    }

	return count;
}
#endif //if 0 end


#if SUPPORT_SN_LED_GPIO_CTRL
static void sn_led_gpio_init(void)
{
	int i;

	//printk("%s: MAX_GPIO_NUMBER=%d\n",__func__,MAX_GPIO_NUMBER);
	
	for(i = 0; i <= MAX_GPIO_NUMBER; ++i )
	{
		sn_led_gpio_data[i].val = 0;
		sn_led_gpio_data[i].timer_start = 0;
		sn_led_gpio_data[i].action = LED_ACTION_OFF;
		sn_led_gpio_data[i].wps_state = WPS_INACTIVE;
		sn_led_gpio_data[i].eco_led_state = ECO_LED_ACTION_NONE;
	}
}

static OS_TIMER_FUNC(sn_gpio_led_blink)
{
	int gpio = timer_arg;

	if(sn_led_gpio_data[gpio].val)
	{
        ath_gpio_out_val(gpio, _LED_OFF);
		sn_led_gpio_data[gpio].val = 0;
	}
	else
	{
        ath_gpio_out_val(gpio, _LED_ON);
		sn_led_gpio_data[gpio].val = 1;
	}

	/* blink 0.2 second */
	OS_SET_TIMER(&sn_led_gpio_data[gpio].t, 200);
}

static OS_TIMER_FUNC(sn_gpio_led_fast_blink)
{
	int gpio = timer_arg;

	if(sn_led_gpio_data[gpio].val)
	{
        ath_gpio_out_val(gpio, _LED_OFF);
		sn_led_gpio_data[gpio].val = 0;
	}
	else
	{
        ath_gpio_out_val(gpio, _LED_ON);
		sn_led_gpio_data[gpio].val = 1;
	}

	/* blink 0.1 second */
	OS_SET_TIMER(&sn_led_gpio_data[gpio].t, 100);
}

static OS_TIMER_FUNC(sn_gpio_led_wps_fail)
{
#ifdef USE_WN_AG300DGR_LED
		static int WPSled = _LED_ON, sec = 0;
		int gpio = timer_arg;

        ath_gpio_out_val(gpio, WPSled);
        WPSled = !WPSled;
        sec++;
        if (sec < WPS_LED_FAIL_BLINK_TIME_COUNT) {
                OS_SET_TIMER(&sn_led_gpio_data[gpio].t, WPS_LED_FAIL_BLINK_INTERVAL);
        } else {
                sec = 0;
				OS_CANCEL_TIMER(&sn_led_gpio_data[gpio].t);
#if SUPPORT_SYSTEM_OPERATION_MODE_LED_CTRL
				if (sys_opmode == SYS_OPM_CB)
				{
					ath_gpio_out_val(gpio, _LED_OFF);
				}
				else
#endif
				{
                	// depend on eco mode
					if (sn_led_gpio_data[gpio].eco_led_state)
					{
                    	ath_gpio_out_val(gpio, sn_led_gpio_data[gpio].eco_led_state == ECO_LED_ACTION_OFF ? _LED_OFF : _LED_ON);
					}
					else
					{
						/*After wps timeout fast blinking, let wps led off if we don't use ECO.  */
						ath_gpio_out_val(gpio, _LED_OFF);
					}
				}
				sn_led_gpio_data[gpio].wps_state = WPS_INACTIVE;
        }
#else
	int gpio = timer_arg;

	if(sn_led_gpio_data[gpio].val)
	{
        ath_gpio_out_val(gpio, _LED_OFF);
		sn_led_gpio_data[gpio].val = 0;
	}
	else
	{
        ath_gpio_out_val(gpio, _LED_ON);
		sn_led_gpio_data[gpio].val = 1;
	}

	/* blink 0.1 second */
	OS_SET_TIMER(&sn_led_gpio_data[gpio].t, 100);
#endif
}

static OS_TIMER_FUNC(sn_gpio_led_wps_pass)
{
	static int WPSled = _LED_ON, sec = 0;
	int gpio = timer_arg;

    ath_gpio_out_val(gpio, WPSled);
#if WPS_LED_SUCCESS_BLINK_INTERVAL < 300000
    /* WPS_LED_SUCCESS always on when WPS_LED_SUCCESS_BLINK_INTERVAL >= 300 seconds. */
    WPSled = !WPSled;
#endif
    sec++;
    if (sec < WPS_LED_SUCCESS_BLINK_TIME_COUNT) {
            OS_SET_TIMER(&sn_led_gpio_data[gpio].t, WPS_LED_SUCCESS_BLINK_INTERVAL);
            ath_gpio_out_val(gpio, _LED_ON);
    } else {
            sec = 0;
            OS_CANCEL_TIMER(&sn_led_gpio_data[gpio].t);
#ifdef USE_WN_AG300DGR_LED
        	// depend on eco mode
		if (sn_led_gpio_data[gpio].eco_led_state)
		{
            ath_gpio_out_val(gpio, sn_led_gpio_data[gpio].eco_led_state == ECO_LED_ACTION_OFF ? _LED_OFF : _LED_ON);
		}
		else
		{
			/*After wps timeout fast blinking, let wps led off if we don't use ECO.*/
			ath_gpio_out_val(gpio, _LED_OFF);
		}
			sn_led_gpio_data[gpio].wps_state = WPS_INACTIVE;
#else
            ath_gpio_out_val(gpio, _LED_OFF);
#endif
    }
}

#if defined(CUSTOM_BLINK_1_ON_INTERVAL) && defined(CUSTOM_BLINK_1_OFF_INTERVAL)
static OS_TIMER_FUNC(sn_gpio_led_custom_blink_1)
{
	int gpio = timer_arg;

	if(sn_led_gpio_data[gpio].val)
	{
        ath_gpio_out_val(gpio, _LED_OFF);
		sn_led_gpio_data[gpio].val = 0;
		/* blink xx second */
		OS_SET_TIMER(&sn_led_gpio_data[gpio].t, CUSTOM_BLINK_1_OFF_INTERVAL);
	}
	else
	{
        ath_gpio_out_val(gpio, _LED_ON);
		sn_led_gpio_data[gpio].val = 1;
		/* blink oo second */
		OS_SET_TIMER(&sn_led_gpio_data[gpio].t, CUSTOM_BLINK_1_ON_INTERVAL);
	}
}
#endif

static void sn_led_gpio_set(int gpio, int act)
{
	// protect!
	if(gpio <= 0 || gpio > MAX_GPIO_NUMBER)
	{
		return;
	}

	if(sn_led_gpio_data[gpio].timer_start)
	{
		OS_CANCEL_TIMER(&sn_led_gpio_data[gpio].t);
		sn_led_gpio_data[gpio].timer_start = 0;
	}

	sn_led_gpio_data[gpio].action = act;

	switch(act)
	{
	case LED_ACTION_BLINKING:
		OS_INIT_TIMER(NULL, &sn_led_gpio_data[gpio].t, sn_gpio_led_blink, gpio);
		OS_SET_TIMER(&sn_led_gpio_data[gpio].t, 100);
		sn_led_gpio_data[gpio].timer_start = 1;
		sn_led_gpio_data[gpio].wps_state = WPS_ACTIVE;
		break;
	case LED_ACTION_OFF:
		if(g_ath_btn.e_ath_btn_action == SN_ATH_BTN_FACTORY_MODE)
			OS_CANCEL_TIMER(&os_reset_timer_t);
        ath_gpio_out_val(gpio, _LED_OFF);
		sn_led_gpio_data[gpio].val = 0;
		//Don't let wifi driver control wifi led.
		//Until WPS finish process.
#ifdef USE_SITECOM_GPIO_LED
		_wlan_led_off_=1;
#endif
		break;
	case LED_ACTION_ON:
		if(g_ath_btn.e_ath_btn_action == SN_ATH_BTN_FACTORY_MODE)
			OS_CANCEL_TIMER(&os_reset_timer_t);
        ath_gpio_out_val(gpio, _LED_ON);
		sn_led_gpio_data[gpio].val = 1;
		break;
	case LED_ACTION_FAST_BLINKING:
		OS_INIT_TIMER(NULL, &sn_led_gpio_data[gpio].t, sn_gpio_led_fast_blink, gpio);
		OS_SET_TIMER(&sn_led_gpio_data[gpio].t, 100);
		sn_led_gpio_data[gpio].timer_start = 1;
		break;
	case LED_ACTION_WPS_PASS:
#if DUAL_RADIO_WPS_BEHAVIOR_FOR_WPS_V2
		OS_CANCEL_TIMER(&sn_led_gpio_data[SENAO_GPIO_LED_WPS].t);
		OS_CANCEL_TIMER(&sn_led_gpio_data[SENAO_GPIO_LED_WPS2].t);
		if (gpio == SENAO_GPIO_LED_WPS)
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_6;
		}
		else if (gpio == SENAO_GPIO_LED_WPS2)
		{
			g_ath_btn.e_ath_btn_action = BUTTON_PROBE_WPS_ACTION_5;
		}
		ath_gpio_notify_user(&g_ath_btn);
#endif

#if DUAL_RADIO_WPS_BEHAVIOR_FOR_WPS_V2
                /* Only use 24G WPS LED for WPSv2 H/W button behavior.*/
                if (wps_hw_button_press == 1)
                {
                        gpio = SENAO_GPIO_LED_WPS;
                }
#endif
		ath_gpio_out_val(gpio, _LED_ON);

		OS_INIT_TIMER(NULL, &sn_led_gpio_data[gpio].t, sn_gpio_led_wps_pass, gpio);
		OS_SET_TIMER(&sn_led_gpio_data[gpio].t, 1000);
		sn_led_gpio_data[gpio].timer_start = 1;
		break;
	case LED_ACTION_WPS_FAIL:
#if DUAL_RADIO_WPS_BEHAVIOR_FOR_WPS_V2
		OS_CANCEL_TIMER(&sn_led_gpio_data[SENAO_GPIO_LED_WPS2].t);
		OS_CANCEL_TIMER(&sn_led_gpio_data[SENAO_GPIO_LED_WPS].t);
		/* Only use 24G WPS LED for WPSv2 H/W button behavior.*/
		if (wps_hw_button_press == 1)
		{
			gpio = SENAO_GPIO_LED_WPS;
		}
#endif
		OS_INIT_TIMER(NULL, &sn_led_gpio_data[gpio].t, sn_gpio_led_wps_fail, gpio);
		OS_SET_TIMER(&sn_led_gpio_data[gpio].t, 100);
		sn_led_gpio_data[gpio].timer_start = 1;
		sn_led_gpio_data[gpio].wps_state = WPS_ACTIVE;
		break;
#if defined(CUSTOM_BLINK_1_ON_INTERVAL) && defined(CUSTOM_BLINK_1_OFF_INTERVAL)
	case LED_ACTION_CUSTOM_BLINK_1:
		OS_INIT_TIMER(NULL, &sn_led_gpio_data[gpio].t, sn_gpio_led_custom_blink_1, gpio);
		OS_SET_TIMER(&sn_led_gpio_data[gpio].t, 100);
		sn_led_gpio_data[gpio].timer_start = 1;
		break;
#endif
	default:
		break;
	}
}
#endif


#if 0
void ap_usb_led_on(void)
{
#ifdef CONFIG_MACH_AR934x
#if !defined(CONFIG_I2S) && defined(AP_USB_LED_GPIO)
    unsigned int rddata;

    if(AP_USB_LED_GPIO == 4) { 
     	rddata = ath_reg_rd(ATH_GPIO_OUT_FUNCTION1); //87- for USB suspend
    	rddata = rddata & 0xffffff00;
    	rddata = rddata | ATH_GPIO_OUT_FUNCTION1_ENABLE_GPIO_4(0x0);
    	ath_reg_wr(ATH_GPIO_OUT_FUNCTION1, rddata);
    }else if(AP_USB_LED_GPIO == 11) {
        rddata = ath_reg_rd(ATH_GPIO_OUT_FUNCTION2); //87- for USB suspend
        rddata = rddata & 0x00ffffff;
        rddata = rddata | ATH_GPIO_OUT_FUNCTION2_ENABLE_GPIO_11(0x0);
        ath_reg_wr(ATH_GPIO_OUT_FUNCTION2, rddata);
    }
    
    ath_reg_rmw_clear(ATH_GPIO_OE, (1<<AP_USB_LED_GPIO));
    ath_reg_rmw_clear(ATH_GPIO_OUT, (1<<AP_USB_LED_GPIO));
#endif
#else
	ath_gpio_out_val(AP_USB_LED_GPIO, USB_LED_ON);
#endif
}

EXPORT_SYMBOL(ap_usb_led_on);

void ap_usb_led_off(void)
{
#ifdef CONFIG_MACH_AR934x
#if !defined(CONFIG_I2S) && defined(AP_USB_LED_GPIO)
	ath_reg_rmw_set(ATH_GPIO_OUT, (1<<AP_USB_LED_GPIO));
#endif
#else
	ath_gpio_out_val(AP_USB_LED_GPIO, USB_LED_OFF);
#endif
}
EXPORT_SYMBOL(ap_usb_led_off);
#else //0

#if defined(CONFIG_MACH_QCA955x)
void ap_usb_led_on(void)
{
	//printk("%s: \n",__func__);
#ifdef SENAO_GPIO_LED_USB	
	//printk("%d\n",SENAO_GPIO_LED_USB);
	ath_gpio_out_val(SENAO_GPIO_LED_USB, USB_LED_ON);
#endif
}
EXPORT_SYMBOL(ap_usb_led_on);

void ap_usb2_led_on(void)
{
	//printk("%s: \n",__func__);
#ifdef SENAO_GPIO_LED_USB2		
	//printk("%d\n",SENAO_GPIO_LED_USB2);
	ath_gpio_out_val(SENAO_GPIO_LED_USB2, USB_LED_ON);
#endif
}
EXPORT_SYMBOL(ap_usb2_led_on);


void ap_usb_led_off(void)
{
	//printk("%s: \n",__func__);
#ifdef SENAO_GPIO_LED_USB	
	//printk("%d\n",SENAO_GPIO_LED_USB);
	ath_gpio_out_val(SENAO_GPIO_LED_USB, USB_LED_OFF);
#endif
}
EXPORT_SYMBOL(ap_usb_led_off);

void ap_usb2_led_off(void)
{
	//printk("%s: \n",__func__);
#ifdef SENAO_GPIO_LED_USB2	
	//printk("%d\n",SENAO_GPIO_LED_USB2);
	ath_gpio_out_val(SENAO_GPIO_LED_USB2, USB_LED_OFF);
#endif
}
EXPORT_SYMBOL(ap_usb2_led_off);
#endif //CONFIG_MACH_QCA955x
#endif //if 0 end


/* 2012-04-11, Atheros Led GPIO_OUT_FUNCTIONX mapping,
 * We should selected programmed value for GPIOX.
 * The value is 0. */

struct gpio_out_func_t {
    u32 addr;
    u32 val;
};

struct gpio_out_func_t gpio_out_func_list[MAX_GPIO_NUMBER+1] = {
     /* 0 - 3 */
    {ATH_GPIO_OUT_FUNCTION0, 0xffffff00}, {ATH_GPIO_OUT_FUNCTION0, 0xffff00ff}, {ATH_GPIO_OUT_FUNCTION0, 0xff00ffff}, {ATH_GPIO_OUT_FUNCTION0, 0x00ffffff},
     /* 4 - 7 */
    {ATH_GPIO_OUT_FUNCTION1, 0xffffff00}, {ATH_GPIO_OUT_FUNCTION1, 0xffff00ff}, {ATH_GPIO_OUT_FUNCTION1, 0xff00ffff}, {ATH_GPIO_OUT_FUNCTION1, 0x00ffffff},
     /* 8 - 11 */
    {ATH_GPIO_OUT_FUNCTION2, 0xffffff00}, {ATH_GPIO_OUT_FUNCTION2, 0xffff00ff}, {ATH_GPIO_OUT_FUNCTION2, 0xff00ffff}, {ATH_GPIO_OUT_FUNCTION2, 0x00ffffff},
     /* 12 - 15 */
    {ATH_GPIO_OUT_FUNCTION3, 0xffffff00}, {ATH_GPIO_OUT_FUNCTION3, 0xffff00ff}, {ATH_GPIO_OUT_FUNCTION3, 0xff00ffff}, {ATH_GPIO_OUT_FUNCTION3, 0x00ffffff},
     /* 16 - 19 */
    {ATH_GPIO_OUT_FUNCTION4, 0xffffff00}, {ATH_GPIO_OUT_FUNCTION4, 0xffff00ff}, {ATH_GPIO_OUT_FUNCTION4, 0xff00ffff}, {ATH_GPIO_OUT_FUNCTION4, 0x00ffffff},
     /* 20 - 23 */
    {ATH_GPIO_OUT_FUNCTION5, 0xffffff00}, {ATH_GPIO_OUT_FUNCTION5, 0xffff00ff}, {ATH_GPIO_OUT_FUNCTION5, 0xff00ffff}, {ATH_GPIO_OUT_FUNCTION5, 0x00ffffff}
};

static void sn_init_led(int gpio, int onoff)
{
	u32 val;

    /* configure gpio as outputs */
    ath_gpio_config_output(gpio);
    /* turn on the led */
    ath_gpio_out_val(gpio, onoff);

    if(gpio >= 0 && gpio <= MAX_GPIO_NUMBER)
    {
		val = ath_reg_rd(gpio_out_func_list[gpio].addr);
		val &= gpio_out_func_list[gpio].val;
		ath_reg_wr(gpio_out_func_list[gpio].addr, val);
    }
}

static int create_simple_config_led_proc_entry(void)
{
	if (simple_config_entry != NULL) {
		printk("Already have a proc entry for /proc/simple_config!\n");
		return -ENOENT;
	}

	simple_config_entry = proc_mkdir("simple_config", NULL);
	if (!simple_config_entry)
		return -ENOENT;

	simulate_push_button_entry = create_proc_entry("push_button", 0644,
							simple_config_entry);
	if (!simulate_push_button_entry)
		return -ENOENT;

	simulate_push_button_entry->write_proc = push_button_write;
	simulate_push_button_entry->read_proc = push_button_read;

	simple_config_led_entry = create_proc_entry("simple_config_led", 0644,
							simple_config_entry);
	if (!simple_config_led_entry)
		return -ENOENT;

	simple_config_led_entry->write_proc = atheros_gpio_simple_config_led_write;
	simple_config_led_entry->read_proc = gpio_simple_config_led_read;

#ifdef WPS_LED_GPIO	
	//printk("%s: WPS_LED_GPIO=%d\n",__func__,WPS_LED_GPIO);
    sn_init_led(WPS_LED_GPIO, _LED_OFF);
#endif    

#ifdef SENAO_GPIO_LED_POWER
	//printk("%s: SENAO_GPIO_LED_POWER=%d\n",__func__,SENAO_GPIO_LED_POWER);
    sn_init_led(SENAO_GPIO_LED_POWER, _LED_ON);
#endif

#ifdef SENAO_GPIO_LED_POWER_B
	//printk("%s: SENAO_GPIO_LED_POWER_B=%d\n",__func__,SENAO_GPIO_LED_POWER_B);
    sn_init_led(SENAO_GPIO_LED_POWER_B, _LED_ON);
#endif

#ifdef SENAO_GPIO_LED_POWER_O
	//printk("%s: SENAO_GPIO_LED_POWER_O=%d\n",__func__,SENAO_GPIO_LED_POWER_O);
    sn_init_led(SENAO_GPIO_LED_POWER_O, _LED_OFF);
#endif

#ifdef SENAO_GPIO_LED_WLAN
	//printk("%s: SENAO_GPIO_LED_WLAN=%d\n",__func__,SENAO_GPIO_LED_WLAN);
    sn_init_led(SENAO_GPIO_LED_WLAN, _LED_OFF);
#endif

#ifdef SENAO_GPIO_LED_WLAN2
	//printk("%s: SENAO_GPIO_LED_WLAN2=%d\n",__func__,SENAO_GPIO_LED_WLAN2);
    sn_init_led(SENAO_GPIO_LED_WLAN2, _LED_OFF);
#endif

#if 0//def SENAO_GPIO_LED_WPS == WPS_LED_GPIO
	//printk("%s: SENAO_GPIO_LED_WPS=%d\n",__func__,SENAO_GPIO_LED_WPS);
    sn_init_led(SENAO_GPIO_LED_WPS, _LED_OFF);
#endif

#ifdef SENAO_GPIO_LED_WPS2
	//printk("%s: SENAO_GPIO_LED_WPS2=%d\n",__func__,SENAO_GPIO_LED_WPS2);
    sn_init_led(SENAO_GPIO_LED_WPS2, _LED_OFF);
#endif

#ifdef SENAO_GPIO_LED_OP1
	//printk("%s: SENAO_GPIO_LED_OP1=%d\n",__func__,SENAO_GPIO_LED_OP1);
    sn_init_led(SENAO_GPIO_LED_OP1, _LED_OFF);
#endif

#ifdef SENAO_GPIO_LED_OP2
	//printk("%s: SENAO_GPIO_LED_OP2=%d\n",__func__,SENAO_GPIO_LED_OP2);
    sn_init_led(SENAO_GPIO_LED_OP2, _LED_OFF);
#endif

#ifdef SENAO_GPIO_LED_OP3
	//printk("%s: SENAO_GPIO_LED_OP3=%d\n",__func__,SENAO_GPIO_LED_OP3);
    sn_init_led(SENAO_GPIO_LED_OP3, _LED_OFF);
#endif

#ifdef SENAO_GPIO_LED_OP4
	//printk("%s: SENAO_GPIO_LED_OP4=%d\n",__func__,SENAO_GPIO_LED_OP4);
    sn_init_led(SENAO_GPIO_LED_OP4, _LED_OFF);
#endif

#ifdef SENAO_GPIO_LED_USB
	//printk("%s: SENAO_GPIO_LED_USB=%d\n",__func__,SENAO_GPIO_LED_USB);
	sn_init_led(SENAO_GPIO_LED_USB, _LED_OFF);
#endif

#ifdef SENAO_GPIO_LED_USB2
	//printk("%s: SENAO_GPIO_LED_USB2=%d\n",__func__,SENAO_GPIO_LED_USB2);
	sn_init_led(SENAO_GPIO_LED_USB2, _LED_OFF);
#endif

#if SUPPORT_SN_LED_GPIO_CTRL
	sn_led_gpio_init();
#endif

	return 0;
}

static int
athfr_open(struct inode *inode, struct file *file)
{
	if (MINOR(inode->i_rdev) != FACTORY_RESET_MINOR) {
		return -ENODEV;
	}

	if (ath_fr_opened) {
		return -EBUSY;
	}

	ath_fr_opened = 1;
	return nonseekable_open(inode, file);
}

static int
athfr_close(struct inode *inode, struct file *file)
{
	if (MINOR(inode->i_rdev) != FACTORY_RESET_MINOR) {
		return -ENODEV;
	}

	ath_fr_opened = 0;
	return 0;
}

static ssize_t
athfr_read(struct file *file, char *buf, size_t count, loff_t * ppos)
{
	return -ENOTSUPP;
}

static ssize_t
athfr_write(struct file *file, const char *buf, size_t count, loff_t * ppos)
{
	return -ENOTSUPP;
}

static int
athfr_ioctl(struct inode *inode, struct file *file, unsigned int cmd,
		unsigned long arg)
{
	int ret = 0;

	switch (cmd) {
	case ATH_FACTORY_RESET:
		atomic_inc(&ath_fr_status);
		sleep_on(&ath_fr_wq);
		break;

	default:
		ret = -EINVAL;
	}

	return ret;
}

static struct file_operations athfr_fops = {
	read:	athfr_read,
	write:	athfr_write,
	ioctl:	athfr_ioctl,
	open:	athfr_open,
	release:athfr_close
};

static struct miscdevice athfr_miscdev = {
	FACTORY_RESET_MINOR,
	"Factory reset",
	&athfr_fops
};

int sn_init_irq(unsigned int irq, irq_handler_t handler, char *fun_name)
{
	int req;
	/*set gpio is input*/
	ath_gpio_config_input(irq);
	/* configure Jumpstart GPIO as level triggered interrupt */
	ath_gpio_config_int (irq, INT_TYPE_LEVEL, INT_POL_ACTIVE_LOW);
	printk("%s (%s) irq: %d\n", __FILE__, __func__, irq);
    req = request_irq (ATH_GPIO_IRQn(irq), handler, 0,
                       fun_name, NULL);
    if (req != 0) {
        printk (KERN_ERR "unable to request IRQ for irq(%d) (error %d)\n", irq,req);
//         misc_deregister(&athfr_miscdev);
        ath_gpio_intr_shutdown(ATH_GPIO_IRQn(irq));
        return 0;
    }
	return 1;
}

int __init ath_simple_config_init(void)
{
#ifdef CONFIG_CUS100
	u32 mask = 0;
#endif
//	int req, ret;

	//printk("%s (%s) GPIO OE=%x\n", __FILE__, __func__, ath_reg_rd(ATH_GPIO_OE));
	
//	ret = misc_register(&athfr_miscdev);

//	if (ret < 0) {
//		printk("*** ath misc_register failed %d *** \n", ret);
//		return -1;
//	}

	memset(&g_ath_btn, 0, sizeof(g_ath_btn));

#ifdef SENAO_GPIO_BUTTON_WPS /*SUPPORT WPS BUTTON*/
	if(!sn_init_irq(SENAO_GPIO_BUTTON_WPS, button_irq, "SW WPS"))	
		return -1;
#endif

#ifdef SENAO_GPIO_BUTTON_WPS1 /*SUPPORT WPS BUTTON*/
	if(!sn_init_irq(SENAO_GPIO_BUTTON_WPS1, button_irq, "SW WPS1"))	
		return -1;
#endif

#ifdef SENAO_GPIO_BUTTON_RESET /*SUPPORT RESET BUTTON*/
	if(!sn_init_irq(SENAO_GPIO_BUTTON_RESET, button_irq, "SW FACTORY RESET")) 
		return -1;
#endif

#ifdef SENAO_GPIO_BUTTON_USB_EJECT /*SUPPORT USB EJECT BUTTON*/
	if(!sn_init_irq(SENAO_GPIO_BUTTON_USB_EJECT, button_irq, "SW USB EJECT"))
		return -1;
#endif

#ifdef SENAO_GPIO_BUTTON_USB2_EJECT /*SUPPORT USB2 EJECT BUTTON*/
	if(!sn_init_irq(SENAO_GPIO_BUTTON_USB2_EJECT, button_irq, "SW USB2 EJECT"))
		return -1;
#endif

#ifdef SENAO_GPIO_BUTTON_WLAN_ONOFF /*SUPPORT WLAN ONOFF BUTTON*/
	if(!sn_init_irq(SENAO_GPIO_BUTTON_WLAN_ONOFF, button_irq, "SW WLAN ONOFF"))
		return -1;
#endif

/* operate button
 * You can define as you want. */
#ifdef SENAO_GPIO_BUTTON_OP_1
	if(!sn_init_irq(SENAO_GPIO_BUTTON_OP_1, button_irq, "SW Operate 1")) 
		return -1;
#endif

#ifdef SENAO_GPIO_BUTTON_OP_2
	if(!sn_init_irq(SENAO_GPIO_BUTTON_OP_2, button_irq, "SW Operate 2")) 
		return -1;
#endif

/* HW Switch */
#ifdef SENAO_MODESWITCH_GPIO_L
	if(!sn_init_irq(SENAO_MODESWITCH_GPIO_L, button_irq, "Switch L")) 
		return -1;
#endif

#ifdef SENAO_MODESWITCH_GPIO_M
	if(!sn_init_irq(SENAO_MODESWITCH_GPIO_M, button_irq, "Switch M")) 
		return -1;
#endif

#ifdef SENAO_MODESWITCH_GPIO_R
	if(!sn_init_irq(SENAO_MODESWITCH_GPIO_R, button_irq, "Switch R")) 
		return -1;
#endif

#if 0
#ifdef CONFIG_CUS100
	mask = ath_reg_rd(ATH_MISC_INT_MASK);
	ath_reg_wr(ATH_MISC_INT_MASK, mask | (1 << 2));
	ath_gpio_config_int(JUMPSTART_GPIO, INT_TYPE_LEVEL,
				INT_POL_ACTIVE_HIGH);
	ath_gpio_intr_enable(JUMPSTART_GPIO);
	ath_gpio_config_input(JUMPSTART_GPIO);
#else
	ath_gpio_config_input(JUMPSTART_GPIO);
	/* configure Jumpstart GPIO as level triggered interrupt */
	ath_gpio_config_int(JUMPSTART_GPIO, INT_TYPE_LEVEL,
				INT_POL_ACTIVE_LOW);
	printk("%s (%s) JUMPSTART_GPIO: %d\n", __FILE__, __func__,
		JUMPSTART_GPIO);
#ifndef CONFIG_MACH_AR934x
	ath_reg_rmw_clear(ATH_GPIO_FUNCTIONS, (1 << 2));
	ath_reg_rmw_clear(ATH_GPIO_FUNCTIONS, (1 << 16));
	ath_reg_rmw_clear(ATH_GPIO_FUNCTIONS, (1 << 20));
#endif
#endif

	req = request_irq(ATH_GPIO_IRQn(JUMPSTART_GPIO), jumpstart_irq, 0,
			"SW JUMPSTART/FACTORY RESET", NULL);
	if (req != 0) {
		printk("request_irq for jumpstart failed (error %d)\n", req);
		misc_deregister(&athfr_miscdev);
		ath_gpio_intr_shutdown(ATH_GPIO_IRQn(JUMPSTART_GPIO));
		return -1;
	}
#endif //if 0 end

#if !defined(CONFIG_I2S) && defined(AP_USB_LED_GPIO)
	//printk("%s: AP_USB_LED_GPIO=%d\n",__func__,AP_USB_LED_GPIO);
	ath_gpio_config_output(AP_USB_LED_GPIO);
#endif

#if !defined(CONFIG_I2S) && defined(AP_USB2_LED_GPIO)
	//printk("%s: AP_USB2_LED_GPIO=%d\n",__func__,AP_USB2_LED_GPIO);
	ath_gpio_config_output(AP_USB2_LED_GPIO);
#endif

//	init_waitqueue_head(&ath_fr_wq);

	create_simple_config_led_proc_entry();

#if defined(SENAO_MODESWITCH_GPIO_L) || defined(SENAO_MODESWITCH_GPIO_M) || defined(SENAO_MODESWITCH_GPIO_R)
	sn_modeswitch_init = 1;
#endif

	return 0;
}

/*
 * used late_initcall so that misc_register will succeed
 * otherwise, misc driver won't be in a initializated state
 * thereby resulting in misc_register api to fail.
 */
#if !defined(CONFIG_ATH_EMULATION)
late_initcall(ath_simple_config_init);
#endif
