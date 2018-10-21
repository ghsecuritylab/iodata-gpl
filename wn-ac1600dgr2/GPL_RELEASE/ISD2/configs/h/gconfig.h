/**
 * $Id: config.h,v 1.1.1.1 2003/06/22 02:40:00 cfho Exp $
 * $Author: cfho $
 * $Date: 2003/06/22 02:40:00 $
 * $Locker:  $
 * $Revision: 1.1.1.1 $
 *
 * Send comments to: 
 *
 * Copyright (C) , SENAO INTERNATIONAL CO., LTD All rights reserved.
 * No part of this document may be reproduced in any form or by any 
 * means or used to make any derivative work (such as translation,
 * transformation, or adaptation) without permission from Senao
 * Corporation.
 */

#ifndef _GCONFIG_H_
#define _GCONFIG_H_

/**
 * IMPORTANT NOTE:
 * DO NOT modify any part of this file for board-specific customization. That
 * should be done in the 'board_xxx.h' file, where xxx="name of the board".
 */


/* #include "supportwlan.h" */



#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------------*/
/* Linux Kernel Version Information */
/*-------------------------------------------------------------------------*/
#define KERNEL_VERSION_NUM(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#define LINUX_VER_CODE KERNEL_VERSION_NUM(2,6,31)

#define IPTABLES_VERSION_NUM(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#define IPTABLES_VER_CODE IPTABLES_VERSION_NUM(1,4,4)

/*-------------------------------------------------------------------------*/
/* Silex Utility Kernel Version Information */
/*-------------------------------------------------------------------------*/
#define SVU_VERSION_NUM(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#define SVU_VER_CODE SVU_VERSION_NUM(1,3,0)

/*-------------------------------------------------------------------------*/
/* iobb Version Information */
/*-------------------------------------------------------------------------*/
#define IOBB_VERSION_NUM(a,b) (((a) << 8) + (b))
#define IOBB_VER_CODE IOBB_VERSION_NUM(1,2)

/*-------------------------------------------------------------------------*/
/*                        HW Features                                      */
/*-------------------------------------------------------------------------*/
#define HAS_IXP_NPE         0
#define HAS_MII_CMD         0
#define HAS_PBX             0
#define HAS_DNS_PROXY       1
#define HAS_HW_DSP          0
#define HAS_HW_CPLD         0
#define HAS_HW_GPIO_BUTTONS 1       /* We should either use CPLD or GPIO */
#define HAS_HW_FXS          0
#define HAS_HW_RTC          0
#define HAS_HW_USB          1       /* mount -t usbfs none /proc/bus/usb */ 
#define HAS_SUPPORT_SYSFS   1       /* mount -t sysfs none /sys */ 
#define HAS_MAC_CLONE       1
#define HAS_IGMPPROXY       1
/*#define HAS_POWER_SAVING        1*/
#define HAS_BANDWIDTH_MONITOR   0
#define HAS_SWITCH_5_VLAN       0
#define SUPPORT_ART 		1
#define HAS_GIGABYTE_SWITCH	1

/* 20120206 Jason: add define for 9342 platform */
#define ATHEROS_9342_s17	1  //todo..need mod for 9342..first use in web reload time

/*-------------------------------------------------------------------------*/
/*                        Partition Info                                   */
/*-------------------------------------------------------------------------*/
/********* MTD partition numbers *******************************/
#define BOOTLOADER_PARTITION_NAME           "Bootloader"
#define FACTORY_PARTITION_NAME              "manufacture"
#define KERNEL_PARTITION_NAME               "Kernel"
#define APP_STORAGE_PARTITION_NAME          "apps"
//#define APP_STORAGE_PARTITION_NAME          "app"
#define APPSCORE_PARTITION_NAME             "appscore"
#define COMBINED_APP_PARTITION_NAME         "cb-app"
#define SOUNDS_STORAGE_PARTITION_NAME       "sounds"
//#define MTD_STORAGE_PARTITION_NAME          "apcfg-storage"
#define MTD_STORAGE_PARTITION_NAME          "storage"
#define MTD_VOICE_PARTITION_NAME            "voice"
#define TINY_APP_STORAGE_PARTITION_NAME     "littleapp"
#define FACTORY_APP_STORAGE_PARTITION_NAME  "manufacture"
#define MTD_BACKUP_PARTITION_NAME           "backup"
#define CONFIG_PARTITION_NAME               "Config"
#define AR92XX_RF_PARTITION_NAME            "caldata" /* TODO: define a suitable name for all Atheros RF(used in config_init.c) */
#define APP_DIR                             "/apps"
#define STORAGE_DIR                         "/storage"
#define SOUNDS_DIR                          "/sounds"
#define USBDISK_DEV                         "/dev/sda1"
#define USBDISK_PATH                        "/mnt/usb"
#define FACTORY_DIR                         "/factory"
#define MAXIMUM_DISKSPACE_FOR_FW_UPGRADE    (1024*2)
#define FW_UPGRADE_IN_PROGRESS_FILE         FIRMWARE_UPAGRADE_TAG_FILE
#define MAX_TMPFS_NODES                     "64"

/*-------------------------------------------------------------------------*/
/*                        Interface name                                   */
/*-------------------------------------------------------------------------*/

/**
 * Now, we need to override potential destructive defines that were not correctly
 * setup in the include directive above.
 */

/**
 * The following variables define which logical linux nexwork devices
 * are to be assigned the function of LAN/WAN/WLAN, etc.
 *
 * WAN_DEV: WAN port network device.
 * ETHER_DEV:   LAN (802.1d) ethernet device.
 * WLAN_A_DEV:  802.11a wireless device.
 * WLAN_B_DEV:  802.11b wireless device.
 */
/* define the device interface name */
#define LOOP_DEV        "lo"         /* The loop-back device */
#define BRG_DEV         "br0"        /* There is ALWAYS a bridge device on the ISD */
#define WANBRG_DEV      "br1"        /* There is ALWAYS a bridge device on the ISD */
#define ETH_GMMI_1_DEV  "eth0"       /* There is only one RGMII in AR8327N*/
#define ETHER_A_DEV     "eth0.1"     /* LAN interface, configed by vconfig*/
#define ETHER_B_DEV     "eth0.2"     /* WAN interface, configed by vconfig*/
#define ETHER_C_DEV     "eth0.3"     /* DMZ interface, configed by vconfig*/
//#define ETHER_D_DEV     "eth0.4"     /* LAN interface, configed by vconfig*/
//#define ETHER_E_DEV     "eth0.5"     /* LAN interface, configed by vconfig*/

/* 20111222 Jason: support Japan FLETS PPPoE (move from PCI)*/
#define ETHER_A_DEV_PHY_ID  1     /* LAN interface, configed by vconfig*/
#define ETHER_B_DEV_PHY_ID  5     /* WAN interface, configed by vconfig*/
#define ETHER_C_DEV_PHY_ID  1     /* DMZ interface, configed by vconfig*/

#define PPP_DEV     "ppp0"
#define ATH_DEV     "ath0"
#define ATH2_DEV    "ath3" /* ath0,ath1,ath2:2.4g ,ath3:5g */
#define ATH_REPEATER_DEV  "ath10" /* todo: check and remove */
#define ATH2_REPEATER_DEV "ath11" /* todo: check and remove */
#define WLAN_G_DEV  ATH_DEV
#define MAX_VLAN_DEV_NUM  4
/* 2010-01-26 use WLAN_SSID_NUM to replace MAX_SSID_NUM */
/* #define MAX_SSID_NUM  4 */


#define WDS_DEV_NUM  4  /* support 4 WSD */
#define WDS_DEV     "wds"
#define WDS_0_DEV   "wds0"
#define WDS_1_DEV   "wds1"
#define WDS_2_DEV   "wds2"
#define WDS_3_DEV   "wds3"

#define APCLI_DEV   "apcli0"

//#define SUPPORT_AG7100
#define AGING_TIME  "0x0065"

/* define HAS_LOADBALANCE_FUNCTION 0 */

/* #define SUPPORT_TWO_BOTTON  1*/

#ifdef HAS_LOADBALANCE_FUNCTION

#define WAN1_DEV    ETHER_B_DEV
#define ETHER_DEV   ETHER_A_DEV
#define LAN_DEV		WLAN_G_DEV //ETHER_A_DEV  
#define DMZ_DEV     ETHER_C_DEV  

/* Has Second WAN */
//#define HAS_WAN2_DEV 0

/* Has Third WAN */
//#define HAS_WAN3_DEV

/* Has fourth WAN */
//#define HAS_WAN4_DEV 1

#else

#define WAN1_DEV    ETHER_B_DEV
#define ETHER_DEV   ETHER_A_DEV 
#define LAN_DEV     ETHER_A_DEV  
#define DMZ_DEV     ETHER_C_DEV  

/* Has Second WAN */
/* 20111222 Jason: support Japan FLETS PPPoE (move from PCI)*/
#define HAS_WAN2_DEV 1

#endif

#define WAN_DEV WAN1_DEV

/* Third Party Account */
#define HAS_ODM_PASSWORD_SUPPORT 1 
#define ODM_PASSWORD "aico3d0a0tdagr"
/*-------------------------------------------------------------------------*/
/*                        Normal define                                    */
/*-------------------------------------------------------------------------*/
/* marklin 20070525 : flash size = 256MB */
//#define FLASH_SIZE_256MB 1 //no use; to check

#define TEMP_RAMDISK_SIZE  (1700)

/* TODO!!!!!*/
#define WLAN_PUREG 1 /* cfho 2006-0314, for cli, get back and check it!*/
#define G_TYPE 1
/* END TODO!!!***/

/*************Router *****************/
#define CONFIG_TARGET //for what?? to check! 
#define ROUTER_SUPPORT

/* cfho 2007-1011, it is used by config_init, mmi and httpd */
#define G_MIN_START_YEAR    2009
#define G_MAX_END_YEAR      2037

/* 20120425 Jason: mod default system time */
#define SYSTEM_DEFAULT_START_YEAR  2012
#define SYSTEM_DEFAULT_START_MONTH 01
#define SYSTEM_DEFAULT_START_DAY   01

/* kenny 2007-01-31 ,Firmware Upgrade :display time to web UI */

#define G_WEB_FWD_APP_NEED_TIME 100
#define G_WEB_FWD_KERNEL_NEED_TIME 60
#define G_WEB_FWD_NEED_KERNELAPP_TIME 30
#define WEB_APPLY_TIME 10
#undef TMP_DIR_SIZE

/* cfho 2007-1113, this file is used for firmware upgrade,
    when the fwmanager or web starts to erase the flash, they should
    write the mtd number to this file. When the whole firmware upgarde
    process has been completed (erase, write and check); they should 
    delete this File.
    The config_init will mount the /storage and check this file,
    if it finds this file, it will start the udhcpd, and backup web server
    for firmware upgrade. Once the firmware upgrade has been completed (OK),
    the config_init should delete this file and reboot the board  */
#define FIRMWARE_UPAGRADE_TAG_FILE "/storage/firmware_upg_tag.txt"
#define FIRMWARE_UPAGRADE_CHECKSUM_FILE "/storage/firmware_upg_chksum.txt"

/* web daemon port */
#define HTTP_PRIVATE_PORT  (8080)
#define HTTP_PRIVATE_PORT6 (80)
/*-------------------------------------------------------------------------*/
/*                        Has Functions                                    */
/*-------------------------------------------------------------------------*/
/**
 * Here, we include the file for "board_xxx.h" that defines default values for
 * each specific board.
 */

/**
 * SI-690B is an AP+Router+VOIP box

 */
/*jay 20070620*/
#define HAS_ADVANCED_ROUTE  1
/* cfho 2007-0329, support L7 filtering in iptables, and WEB UI.
    we do not use L7 filtering because the IXP425 is not fast enough */
#define HAS_L7_FILTERING 0 
/* cfho 2007-0514, support PPTP server? */
#define HAS_PPTP_SERVER 0

/*jaykung20070704 support multiple ssid*/
#define HAS_MULTIPLE_SSID   1

/*jaykung 20070705 mac schedule function */
#define HAS_SCHEDULE_MACACL 0
/*jaykung 20070705 scroll in help datebase*/
#define HAS_SCROLL_FUNCTION 0

/* cfho 2007-1008 support PPTP */
#define HAS_WAN_PPTP   0
#define HAS_WAN_WLAN   0
#define HAS_WAN_PPPOE  1
#define HAS_DMZ_IF_FUNCTION 0
#define DMZ_ELEMENTS 5

/* cfho 2008-0409 Maximum NAT session for ip conntrack table.
   Must be muplier of 8 */
#define MAX_NAT_SESSION_NUM 30000

#define CT_TCP_TIMEOUT_ESTABLISHED 4 HOURS
#define CT_IP_CONNTRACK_ICMP_TIMEOUT 30

#define SUPPORT_LLMNR	1
#define HAS_HTTP_SYSTEM_NAME	1
/* 2010/10/13 Jerry : It needs update RT2860.dat manually at unConfigured of WPS status for wireless driver 2.5 (SDK 3500) */
//#define SYSCONF_UPDATE_WLAN_DAT 1

/**************************/
/* DDNS Setting           */
/**************************/
/* 20120418 Jason: AP mode support DDNS (move from WN-AG450DGR) */
#define SUPPORT_IODATA_DDNS_IN_BRIDGE_MODE  1
/* auto generate DDNS username, password, hostname ,(450DGR is 1) */
#define SUPPORT_IODATA_GENERATE_DDNS        1

/* auto use upnpc to add port 1723 redirect ,(450DGR is 1) */
#define SUPPORT_AUTO_PORT_1723_REDIRECT_IN_AP_MODE 1

/* Sliex USB mode */
/* 1 : Normal */
/* 2 : WN-AC1600DGR special mode only printer server. */
#define SUPPORT_SX_USB_MODE         1

/* sysconfd bind port 10088 */
#define SYSCONFD_BIND_LOCAL_PORT     1
/* HW Switch can choice op mode. */
#define SUPPORT_SWITCH_MODE_BY_BUTTON  1
/* WN-GXXXR LED Style at platform */
#define WN_GXXXR_LED_STYLE          1
/* IODATA_WPS_LED_STYLE_A: Normal mode - OFF  */
/* IODATA_WPS_LED_STYLE_B: Normal mode - Follow WLAN Status (first use in WN-AG300DGR) */
#define IODATA_WPS_LED_STYLE_B		1
/* IODATA_STATUS_LED_STYLE_A: Normal mode  */
/* IODATA_STATUS_LED_STYLE_B: ECO operation that does not blink off "all: LAN wired (first use in WN-AG300DGR) */
#define IODATA_STATUS_LED_STYLE_B		1
/* WN-GXXXR Button Style at platform  */
#define WN_GXXXR_BUTTON_STYLE       1
/* Status LED */
#define IO_STYLE_STATUS_LED         23
/** Auto Bridge Mode  */
#define HAS_AUTO_BRIDGE_MODE	1

/* ECO LED Style
 * A: 2 Power LED = 1 blue , 1 orange  
 * B: 1 Power LED = ECO will blink
 * C: 1 ECO LED = ECO will LED_ON
 * D: 1 Power & 1 ECO LED = ECO will ECO_LED_ON  */
#define IODATA_ECO_LED_STYLE_D 1

/* 20120511 Jason: Beacon minimum ,limit by driver /LSDK-WLAN-9.2.x/os/linux/src/ieee80211_wireless.c ,
	it #define IEEE80211_BINTVAL_IWMIN       40     // min beacon interval  (first use in WN-AG300DGR) */
#define WLAN_BEACON_MIN_VALUE	40	//other platform and LSDK-wLAN-7.x is (20-1000)

/* 20120511 Jason: RTS Threshold maximum ,limit by driver, (1-2346) (first use in WN-AG300DGR) */
#define WLAN_RTS_MAX_VALUE	2347 //other platform and LSDK-wLAN-7.x is (1-2347)

/* Upload file will close some applications */
#define HTTPD_UPLOADFILE_WITH_CLOSE_MODULE  0

/************************************************/
/* Config for sysconfd                          */
/************************************************/
#define SYSCONFD_FLUSH_CACHED_MEMORY_THRESHOLD (20*1024*1024)
/*-------------------------------------------------------------------------*/
/*                        Specific Function Definition                     */
/*-------------------------------------------------------------------------*/
#define HAS_FANCY_PING 1

#define HAS_SYSTEM_SCHEDULE_FUNCTION  1
#define HAS_SCHEDULE_FUNCTION   0

#if HAS_ADVANCED_ROUTE
#define HAS_DYNAMIC_ROUTE 0
#define HAS_STATIC_ROUTE  1
#endif

/**************************/
/* Radio related settings */
/**************************/
#define RADIO_CARD_NUM  2
#define MAX_RADIO_NUM   RADIO_CARD_NUM

/* WLAN related settings */
#define HAS_SUPPORT_ATHEROS_WLAN 1

#if HAS_SUPPORT_ATHEROS_WLAN
#define ATH_WLAN_SSID_NUM  1
#define ATH_WLAN_DEV       "ath"
#define WLAN_SSID_NUM  1
#define WLAN_DEV       "ath"
#define WLAN_DEV_1     "ath0"
#endif

#if HAS_MULTIPLE_SSID
#if HAS_SUPPORT_ATHEROS_WLAN
/* second SSID */
#define HAS_ATH1_2_DEV   1
#define HAS_WLAN2_DEV	 1
/* Third SSID */
#define HAS_ATH1_3_DEV   1
#define HAS_WLAN3_DEV	 1
/* Fourth SSID */
#define HAS_ATH1_4_DEV   0
#define HAS_WLAN4_DEV	 0

#if HAS_ATH1_2_DEV
#undef  ATH_WLAN_SSID_NUM
#define ATH_WLAN_SSID_NUM     2 /* Atheros WLAN device numbers */
#undef  WLAN_SSID_NUM
#define WLAN_SSID_NUM 2
#define WLAN_DEV_2     "ath1"
#endif

#if HAS_ATH1_3_DEV
#undef  ATH_WLAN_SSID_NUM
#define ATH_WLAN_SSID_NUM     3 /* Atheros WLAN device numbers */
#undef  WLAN_SSID_NUM
#define WLAN_SSID_NUM 3
#define WLAN_DEV_3     "ath2"
#endif

#if HAS_ATH1_4_DEV
#undef  ATH_WLAN_SSID_NUM
#define ATH_WLAN_SSID_NUM     4 /* Atheros WLAN device numbers */
#undef  WLAN_SSID_NUM
#define WLAN_SSID_NUM 4
#define WLAN_DEV_4     "ath3"
#endif

#endif /*HAS_SUPPORT_ATHEROS_WLAN*/

#define HAS_SUPPORT_ATHEROS_RADIO2_WLAN 1

#if HAS_SUPPORT_ATHEROS_RADIO2_WLAN
#define ATH2_WLAN_SSID_NUM  1
#else
#define ATH2_WLAN_SSID_NUM  0
#endif

#if HAS_SUPPORT_ATHEROS_RADIO2_WLAN
/* second SSID */
#define HAS_ATH2_2_DEV   0
/* Third SSID */
#define HAS_ATH2_3_DEV   0
/* Fourth SSID */
#define HAS_ATH2_4_DEV   0

#if HAS_ATH2_2_DEV
#undef  ATH2_WLAN_SSID_NUM
#define ATH2_WLAN_SSID_NUM     2 /* Atheros WLAN device numbers */
#endif

#if HAS_ATH2_3_DEV
#undef  ATH2_WLAN_SSID_NUM
#define ATH2_WLAN_SSID_NUM     3 /* Atheros WLAN device numbers */
#endif

#if HAS_ATH2_4_DEV
#undef  ATH2_WLAN_SSID_NUM
#define ATH2_WLAN_SSID_NUM     4 /* Atheros WLAN device numbers */
#endif

#endif /*HAS_SUPPORT_ATHEROS_RADIO2_WLAN*/

#endif /* HAS_MULTIPLE_SSID */

/* 20120321 Jason: supports limitation of connected STAs number */
#define ATH1_SSID1_MAXIMUM_STA_NUM	127 /* Default driver limit value */

#ifdef ETHER_DEV 
#define HAS_ETHER_DEV 1 
#else        
#define HAS_ETHER_DEV 0
#endif

#ifdef WAN_DEV 
#define HAS_WAN_DEV 1 
#else        
#define HAS_WAN_DEV 0
#endif

#define WAN_DEV_NUM 1

#if HAS_WAN_DEV
#define HAS_MULTI_WAN_DEV 1
#endif

#if HAS_MULTI_WAN_DEV

#ifdef HAS_WAN2_DEV
#define WAN2_DEV        ETHER_B_DEV
#undef  WAN_DEV_NUM
#define WAN_DEV_NUM     2 /* WAN device numbers */
#endif

#ifdef HAS_WAN3_DEV
#define WAN3_DEV        ETHER_C_DEV
#undef  WAN_DEV_NUM
#define WAN_DEV_NUM     3 /* WAN device numbers */
#ifndef HAS_WAN2_DEV
#error "define HAS_WAN2_DEV needed"
#endif
#endif

#ifdef HAS_WAN4_DEV
#define WAN4_DEV        ETHER_D_DEV
#undef  WAN_DEV_NUM
#define WAN_DEV_NUM     4 /* WAN device numbers */
#ifndef HAS_WAN3_DEV
#error "define HAS_WAN3_DEV needed"
#endif
#endif

#define WAN_MAX_NUM   WAN_DEV_NUM


#endif
#ifdef WLAN_G_DEV 
#define HAS_WLAN_G_DEV 1 
#else        
#define HAS_WLAN_G_DEV 0
#endif



#ifdef WLAN_A_DEV
#define HAS_WLAN_A_DEV 1
#else
#define HAS_WLAN_A_DEV 0
#endif

#ifdef WLAN_B_DEV
#define HAS_WLAN_B_DEV 1
#else
#define HAS_WLAN_B_DEV 0
#endif

#define PCIE_DEV_STATUS_PATH "/proc/bus/pci/devices"
#define PCIE_DEV_LIST_PATH "/tmp/pcie_dev_list"
#define ATHEROS_VID_DID "168c0030"

#ifndef USE_UDHCPD
#define USE_ISC_DHCPD
#endif

#ifdef ROUTER_SUPPORT
#define HAS_ROUTER_SUPPORT 1
#define HAVE_ROUTER_FUNCTION 1
#define HAS_WAN_DEV        1
#define HAS_DNS_PROXY      1
#define PPTPD_SUPPORT      1
#define HAS_MEDIA_SUPPORT  0
#define WLAN_ACL_APCFG     1
#define IMQ_SUPPORT        1
#else
#define HAS_ROUTER_SUPPORT 0
#endif

#ifdef PRINTER_SUPPORT
#define HAS_PRINTER_SUPPORT 1
#else
#define HAS_PRINTER_SUPPORT 0
#endif

#ifdef PPTPD_SUPPORT
#define HAS_PPTPD_SUPPORT 1
#else
#define HAS_PPTPD_SUPPORT 0
#endif

#ifdef RADIUS_SUPPORT
#define HAS_RADIUS_SUPPORT 1
#else
#define HAS_RADIUS_SUPPORT 0
#endif

#ifdef MEDIA_SUPPORT
#define HAS_MEDIA_SUPPORT 1
#else
#define HAS_MEDIA_SUPPORT 0
#endif

#ifdef IMQ_SUPPORT
#define HAS_IMQ_SUPPORT 1
#else
#define HAS_IMQ_SUPPORT 0
#endif

#define HAS_WAN_STATUS_LED  0

#define HAS_QOS_HWPORT     0
#define HAS_QOS_PRIO_QUEUE 1
#define HAS_QOS_IP_LIMIT_QUEUE 0
#define QOS_BANDWIDTH_RATE_MAX  40000
#define NETPKT_PASSME_UNIT 100

#define HAS_APCLI_WPS      0
#define HAS_MULTI_OPMODE   1

/* 20120105 Jason: add for NTP (move from WN-AG450) */
#define HAS_NTP_USED_INDEX 1

#define HAS_MANUAL_TIME_SETTING 1
/* 20111222 Jason: support netpkthandler (move from WN-AG450) */
#define HAS_SPECIAL_PASS_THROUGH	1
/* 20120412 Kilink: display time seconds */
#define SUPPORT_DETAILED_TIMEINFO 		1

/* 20120416 Kilink: Support Isolation */
#define HAS_DUALBAND_ISOLATION		1	//EBTABLE control 2.4G/5G isolation define
#define HAS_IODATA_ISOLATION		1	//special case in dualband isolation
#if HAS_IODATA_ISOLATION
#define ISOLATION_DEV "ath1"	//GameXXXX
#endif

#define HAS_KERNEL_PPTP 0
/* 20120118 Jason: generate wpa2psk as default WLAN secutiry (move from WN-G150R)*/
#define USE_WPA2PSK_AS_DEFAULT_SECURITY 1

/* 20120105 Jason: default SSID format: AirPort + mac address (move from WN-AG450)*/
#define HAS_WLAN_IODATA_SSID      1	
/* Default IODATA SSID NAME */
#define WLAN_IODATA_1_1_DEF_SSID "AirPort%05d"
#define WLAN_IODATA_1_2_DEF_SSID "Guest%05d"
#define WLAN_IODATA_1_3_DEF_SSID "Copy%05d"
#define WLAN_IODATA_1_4_DEF_SSID "AirPort%05d"
#define WLAN_IODATA_2_1_DEF_SSID "Stream%05d"
#define WLAN_IODATA_2_2_DEF_SSID "AirPort%05d"
#define WLAN_IODATA_2_3_DEF_SSID "AirPort%05d"
#define WLAN_IODATA_2_4_DEF_SSID "AirPort%05d"

/* 20111222 Jason: support Japan FLETS PPPoE (move from PCI)*/
#define SUPPORT_DOMAIN_ROUTING 1
#define SUPPORT_JP_STATIC_ROUTING 1

/* 20120105 Jason: IODATA paththrough control by wizard (move from WN-AG450 ,150R doesn't have) */
#define HAS_IPV6_PATHTHROUGH_WIZARD_CONTROL 1

/* 20120118 Jason : Support ECO Schedule and Advanced function  */
#define HAS_IODATA_ADVANCED_ECO_SETTINGS 	1

/* 20120419 Kilink : Support WLAN Low Rate function (move from WN-AG450) */
#define ECO_WITH_WLAN_LOW_RATE      1


/* 20120116 Norkay: */
/* BadSiteFilter : Becuase etherLAN without VLAN, we add etherLAN device to WLAN.
 * netpkthandler/bsf.c BSF_handler_driver() will add VLAN tag from etherLAN for badsitefilter check packet. */
//#define ETHER_LAN_HASNT_SUPPORT_VLAN 1

/* 20130307 Kilink : WEP auth only show auto mode */
#define HAS_WEB_WEP_ONLY_AUTO   1

/*20120119 Jason: WEB only show NTP server ,hide set time by user (move from WN-AG450DGR)*/
#define HAS_NTPSERVER_SUPPORT_ONLY  1

/* 20120120 Kilink : Add new BSTxt_V2 */
#define SUPPORT_BS_V2 1

/*20120523 Kilink : It can control VPN only MSCHAP 128 and password display*/
#define HAS_WEB_VPN_ONLY_MSCHAP_128      1

/*20130408 Kilink : It can control display Band select*/
#define HAS_WEB_SUPPORT_BAND    0
/*20130417 Kilink : It can control display WDS setting*/
#define HAS_WEB_SUPPORT_WDS_SETTING    0

/* 20130426 leoanrd: temp define for changing AC bandwidth do reboot, this would be deleted at latrer dirver version. */
#define AC_BAND_CHANGE_AND_REBOOT   0
/* -------- [Start] 20120130 Jason ------------ */
/* Add for AP mode setting ,move from (450DGR) */

/* AP Mode support dhcp client  */
#define HAS_DHCP_CLIENT_IN_AP_MODE  1
/* AP Mode Bridge Wan & Wireless & Lan */
#define HAS_BRIDGE_WAN_IN_AP_MODE   1

#define HAS_MULTI_MODE_LAN_IP 		1
/* AP mode support NTP client */
#define SUPPORT_AP_MODE_USE_NTP     1
/* -------- [END] 20120130 Jason ------------ */


/* 20120120 Kilink : Support MTU for WN-G300R minimun 576*/
#define HAS_WEB_NEW_PPPOE_MTU_MINIMAL 1
/* 20120120 Kilink : Support hide internet multicast for WN-G300R*/
#define HAS_WEB_HIDE_MULTICAST 0

/*Jason 20100701 support WEB_NETUSB information*/
#define HAS_WEB_NETUSB	1
/*Jason 20110411 support USB HOST function(NAS/Print Server)*/
#define HAS_WEB_NETUSB_HOST_FUNCTION	1
/*20130419 Jason: Add only support (None/Print Server)...remove net.usb and NAS */
#define HAS_WEB_NETUSB_HOST_ONLY_PRINT_SERVER	0

/*Jason 20110106 support internet connection auto selector (ezinet.htm)*/
#define SUPPORT_INTERNET_AUTO_SELECTOR		1
#if SUPPORT_INTERNET_AUTO_SELECTOR
/*Jason 20110104 support 404 error web page to redirect to ezinet.htm*/
#define SUPPORT_404_ERROR_HTML		1
#define HTTP_404_ERROR_SERV_PORT 10404
/*Jason 20110104 use in wansetting.c to check wan status when wan reload(get wanip) */
#define SUPPORT_404_ERROR_HTML_CHECK_INTERNET 1
#endif

/*20130902 Kilink : support mobile wlbasic page count down page.*/
#define HAS_WEB_MOBILE_WLBAISC_PAGE 1

/*20130905 Kilink : support FW upgrade add new words on stainfo.*/
#define HAS_WEB_FW_UPGRADE_ADD_WORDS    1

/* 20120201 Jason: support disable WPS when SSID1 encrypt is WEP or disable SSID broadcast (for WPS2.0). */
/* 				   Modify web and CGI */
#define HAS_SUPPORT_WEB_WPS20	1

/* 2012-02-04 Norkay, remove if( || pIp ) check.
 * AR9341 ifconfig down interface then ifconfig up interface will let interface do initial again. 
 * It will let system hang 2 seconds. 
 * So we try to let WAN interface doesn't ifconfig down if WAN MAC is the same.(without CLONEMAC) 
 * TODO: Other platform also doesn't need ifconfig down ??? 
 * For protect, we should try everyplatform. If so, we can remove this definition. */
#define WANIFUP_HASNT_IFCONFIG_DOWN_DEV 1

/* -------- [Start] 20120207 Jason ----(Support Notification function)-------- */
/*Leonard 20110303 support notification function (move from WN-AG450DGR)*/
#define HAS_NOTIFICATION_ENABLE 1
/* 2011/03/01 Yolin : IODATA auto firmware check function */
#define FW_NOTIFY_FOR_IODATA	1
/* yolin 2011-0303, Used by systemmonitor to check new firmware */
//http://iosupdate.iodata.jp/lib/auto/wn/wn-ac1600dgr/wn-ac1600dgr.xml
#define FW_CHECK_URL "http://iosupdate.iodata.jp/lib/auto/"
#define FW_FOLDER_NAME "wn/wn-ac1600dgr2/"
#define FW_XML_FILE_NAME "wn-ac1600dgr2.xml"
#define FW_XML_TEMP_FILE "/tmp/fwcheck.xml"
#define FW_CHECK_PERIOD 864000  /* 10 dyas = 10*24*60*60 = 864000 */
/* -------- [END] 20120207 Jason ---------------------------------------------- */

/* Wireless STA access MGMT UI setting */
#define HAS_DENY_ADMIN_ACCESS 1
#if HAS_DENY_ADMIN_ACCESS
/* 20120207 Jason: Add Deny dev name */
#define WLAN_DENY_DEV "ath1"
#endif

/* 2012xxxx Sam: detect whether wifi%d or ath&d atheros wlan interface exist and reboot if doesn't */
#define HAS_RADIO_INTF_FAILED_DETECT	1
/* 20120628 Jason: IO request detect but not reset */
#define HAS_RADIO_INTF_FAILED_DETECT_WITHOUT_RESET 1

/* 20120210 Jason: use in sysconfig, WEP key only config key selected */
#define WEP_KEY_ONLY_CONFIG_SELECTED_KEY_IDX	1

/* URL filter can set allow or Deny separate. */
#define HAS_URL_FILTER_SEPARATE_RULE 1

/* Zero Configuration Networking (Zeroconfig), is a set of techniques that automatically creates a usable
   IP network without configuration or special servers. */
#define HAS_ZERO_CONFIG_FUNCTION 1

/* Support ECO Button */
#define HAS_SUPPORT_ECO_BUTTON        1

/* 20120427 Jason: hide web pppoe flet checkbox */
#define HAS_WEB_HIDE_PPPOE_FLET_SQUARE 	1

/* 20120613 Kilink: add online document on menu tree*/
#define HAS_ONLINE_DOCUMENT 1

/* 21030117 Leonard: support duplicate ap settings. */
#define HAS_SUPPORT_DUPLICATE_AP_SETTINGS 1
#if HAS_SUPPORT_DUPLICATE_AP_SETTINGS
#define DUP_RADIO "1"   // radio1
#define DUP_SSID  "3"   // ssid3
#define DUP_REAL_SSID  "ath2"
#endif

/* 21030701 Leonard: support to use un-sequentail wlan interface. */
#define HAS_SUPPORT_UNSEQUENTIAL_SSID_ENABLE    1

#define INCREASE_SYSTEM_RELOAD_TIME_FOR_HTTPD   "60"

/* 20120514 Jason: support VPN ------- Start -----------(move from WN-G300DGR) */
#if defined(HAS_VPN_PPTP)
#ifndef NUM_VPN_PPTP
#define NUM_VPN_PPTP 1
#endif
#else
#define NUM_VPN_PPTP 0
#endif /* HAS_VPN_PPTP */

#if defined(HAS_VPN_L2TP)
#ifndef NUM_VPN_L2TP
#define NUM_VPN_L2TP 1
#endif
#else
#define NUM_VPN_L2TP 0
#endif /* HAS_VPN_L2TP */

#if defined(HAS_VPN_IPSEC)
#ifndef NUM_VPN_IPSEC
#define NUM_VPN_IPSEC 5
#endif
#define NUM_IPSEC_CONNECTION NUM_VPN_IPSEC
/* IPSec kernel module has two interface in default */
#define IPSEC_DEV_1 "ipsec0"
#define IPSEC_DEV_2 "ipsec1"
#define IPSEC_DEV   IPSEC_DEV_1
#else
#define NUM_VPN_IPSEC 0
#endif /* HAS_VPN_IPSEC */

#define VPN_MAX_NUM NUM_VPN_PPTP + NUM_VPN_L2TP + NUM_VPN_IPSEC
/* 20120514 Jason: support VPN ------- [End] ----------- */

/* 20120518 Jason: AC1600DGR request 5G pin code should same to 2.4G (follow 450DGR) - (add new define to do this funciton) */
#define SUPPORT_WPS_ALL_RADIO_SAME_PINCODE 1
#define SUPPORT_WPS_GENERATE_NEW_PIN 1     /* To support generate new pin in web */



/* BRG_DEV fix MAC address
 * The value should be 6 or 7
 * 6    : LAN MAC address
 * 7    : WAN MAC address  */
#define SUPPORT_BRG_DEV_FIX_MAC_ADDRESS_AS_X  6

/* Allow broadcast packet in any case. */
#define SUPPORT_BROADCAST_PACKET_IN_ANY_CASE  1

/* 2012-07-06, AVOID_DETECT_ETHER_PHY_UNSTABLE
 * Avoid ether phy get unstable value, so we skip 1 secs(1 time) change! */
#define AVOID_DETECT_ETHER_PHY_UNSTABLE       1

/* AutoBridge : Some customer or project don't want to enable lan dhcp client when jump to AP mode. */
#define AUTO_BRIDGE_NOT_SUPPORT_AUTOCHANGE_DYNAMIC 1

/* To support generate new pin in web */
/*#define SUPPORT_WPS_GENERATE_NEW_PIN 0*/

/* 20121029 Jason: Hide UPnP page in AP mode */
#define HAS_WEB_HIDE_UPNP_APMODE 1

/* 20130305 Jason: Support CSRF */
#define HAS_WEB_CSRF_PROTECT        1
/*-------------------------------------------------------------------------*/
/*                        Vendor (Unique)                                  */
/*-------------------------------------------------------------------------*/
#define FOR_IODATA       1
#define FOR_WN_AC1600DGR  1
/*-------------------------------------------------------------------------*/
/*                        Vendor Specials                                  */
/*-------------------------------------------------------------------------*/
/* 20120716 Jason: only use new token, but no apcfgapi */
#define SUPPORT_MULTI_TOKEN_NO_APCFGAPI 1

/* 20110518 Jason: Use for sending trap in different vendor */
//#define ENTERPRISE_TRAP_OID ".1.3.6.1.4.1.14125.100" //to check!!!
/*  tim 2012-10/15, support snmpd for wan port   */
/*#define HAS_SNMPD_WAN_SUPPORT   0*/
#define DEF_REGULAR_DOMAIN      1
#define HAS_HOTEL_FUNCTION      0
/*regular domain
0:1-11  1:1-13 2:10-11 3:10-13 4:14 5:1-14 6:3-9 7:5-13
*/

#define FAKE_UBOOT_VERSION  "1.0.0"
/* 20111222 Jason: support netpkthandler (move from WN-AG450, 150R define:1000) */
#define	NPK_MAX_RECORD_GET_PACKET_NUM	(4000)

/*-------------------------------------------------------------------------*/
/*                       Atheros Max Txpower                               */
/*-------------------------------------------------------------------------*/
#define ATH_MAX_TX_POWER	21
/*-------------------------------------------------------------------------*/
/*                        Factory Test                                  */
/*-------------------------------------------------------------------------*/
#define SUPPORT_MANUFAC_APPS        1	/* factory_apps_init */
#define SUPPORT_GIGABIT_ETHERNET_TEST	1
#define HAS_ATH_PCIE_CHECK  1

#define FACTORY_ATH_5G_PCIE_ID      "0100"

#define SUPPORT_USB_TO_LAN_DONGLE       1
#define	ART_IN_FACTORY					1
#define	ART2_VERSION_4_9_93		1

/*---------------------------------------------*/
/*           APCFG_DEF_NUMBER VALUE            */
/*---------------------------------------------*/
#define NUM_PORT_QOS                            16
#define NUM_LAN_MAC_FILTERS                     32
#define NUM_IP_FILTERS                          32
#define NUM_URL_FILTER                          20
#define NUM_DHCPD_MACTOIP                       10
#define NUM_STATICROUTING                       20
/* 20111222 Jason: support Japan FLETS PPPoE (move from PCI)*/
#define NUM_JP_STATICROUTING					4

#define NUM_MAC_FILTERS                         32
#define NUM_ATH2_MAC_FILTERS                    32
#define NUM_PORT_FORWARD                        32
#define NUM_NAT_VSERVER                         32
#define NUM_TRIGGER_PORT                        16
#define NAT_VSERVER_NAME_MAX                    25
#define NUM_SCHEDULE_LIST                       10 /* 1~10:ECO */
#define NUM_ECO_SCHEDULE 						10 /* ECO MAX Support 10 schedule */
#define NUM_SPEC_APP_PUBLIC_PORT_LIST           10 /* 5 Pairs */
#define NUM_NTP_SERVER							1

/* 20111222 Jason: support Japan FLETS PPPoE (move from PCI)*/
#define NUM_DOMAIN_ROUTING 						4

/*---------------------------------------------*/
/*           APCFG_DEF_VALUE                  */
/*-------------------------------------------*/
#define APCFG_HAS_WARNINGS  1

#define VENDOR_NAME "IO-DATA"
#define VENDOR_MODE "Wireless Network Broadband Router"
#define MODEL_NAME  "WN-AC1600DGR"
#define MODEL_NAME_FORHOSTNAME  "WN-AC1600DGR"
#define WEB_TITLE_NAME  "I-O DATA Wireless Router"

#define MODEL_SUBNAME  "v1002"  //to do ..remove?

#define AP_WSC_MANUFACTURE     VENDOR_NAME" Technologies, Inc."
#define AP_WSC_MODEL_NAME      VENDOR_NAME" Wireless AP Router"
#define AP_WSC_DEVICE_NAME     VENDOR_NAME" 802.11n AP Router"
#define CLI_WSC_MODEL_NAME     VENDOR_NAME" Wireless Client"
#define CLI_WSC_DEVICE_NAME    VENDOR_NAME" Client"
#define CLI_WSC_DEVICE_NAME_R  VENDOR_NAME" EX-Registrar"

/*---------------------------------------------*/
/*           PnP-x Definition                  */
/*---------------------------------------------*/
#define PNPX_VENDOR_URN                 "www-iodata-jp"
#define PNPX_COMPATIBLE_ID				"GenericUmPass" /* Specified compatible ID by customer. */
#define PNPX_HARDWARE_ID				"WN-AC1600DGR" /* Specified hardware ID by customer. */
#define PNPX_VEN_ID                     "06F3"
#define PNPX_DEV_ID                     "100E"
#define PNPX_MODEL_ID                  	"18de43f8-677c-4d3a-8920-b94722aca686" //creat by COMMON_APPS/uuidgen/uuidgen -m b94722aca686

#define LANMAC_HASCHANGED_TAG "/storage/lanMACChanged.tag"

#define SYSLOG_MESSAGE_SIZE 16000

/*joey 2009-1009 define log level for system_log*/
/*0: Nornal, 1: Advance, 2: Detail*/
#define DEFAULT_LOG_LEVEL 1

#define DDNS_NOTIFY_FILE    "/tmp/ddns.status"

#define DDNS_USERAGENT_NAME "WNAC1600DGR/1.2"

#define DLNA_FRIENDLY_NAME  "WNAC1600DGR"
/***************************************************************************/

/*-------------------------------------------------------------------------*/
/*                        Web OPMode List                                  */
/*-------------------------------------------------------------------------*/
//#define WEB_OPMODE_LIST "0011101"
#define WEB_OPMODE_LIST "0000101"
/* opmode list
   Definition: [SYS_OPM_ARRP][SYS_OPM_WDSB][SYS_OPM_CBRT][SYS_OPM_CB][SYS_OPM_AP][SYS_OPM_APRP][SYS_OPM_AR]
   Example:
        "0000011" : AR, APRP
        "0001101" : AR, AP, CB
*/
/***************************************************************************/

/*-------------------------------------------------------------------------*/
/*                        Web Function List                                */
/*-------------------------------------------------------------------------*/
#define WEB_HAS_DUAL_PPPOE				0
#define WEB_HAS_PASS_THROUGH			1
#define WEB_HAS_PORTBASED_QOS			0
#define WEB_HAS_WLAN_CONNECTION_CONTROL	1

/* 20090929 Nelson: support release WPS configuration */
#define WEB_SUPPORT_DTM_TEST_SETTINGS   1
/* 20090317 jerry chen: get vendor information from sysProductInfo.h */
#define WEB_GET_VENDOR_INFO_FROM_SYS	1

/* 20120112 Jason : IODATA WAN MENU is defferent to other ODM, it for WEB display (move from WN-AG450) */
#define HAS_RADIO_IN_WAN_SETTING	1

/*-------------------------------------------------------------------------*/
/*                        RMGMT Settings                                   */
/*-------------------------------------------------------------------------*/
#define RMGMT_INTERFACE_BRG						BRG_DEV
#define RMGMT_INTERFACE_LAN						ETHER_A_DEV
#define RMGMT_INTERFACE_WLAN					WLAN_G_DEV
#define RMGMT_INTERFACE_WLAN2                   ATH2_DEV

/*-------------------------------------------------------------------------*/
/*                        Kernel Modules Specials                          */
/*-------------------------------------------------------------------------*/
#define HASNOT_SUPPORT_IP_CONNTRACK_APPLICATEION_MODULE 1


/* marklin 20090303 : configure mtd partition */
#if LINUX_VER_CODE > KERNEL_VERSION_NUM(2,6,0)
#define UBOOT_ENV_PARTITION "/dev/mtd2"
#define RF_DATA_PARTITION "/dev/mtd8"
#define BACKUP_PARTITION "/dev/mtd6"
#else
#define UBOOT_ENV_PARTITION "/dev/mtd/2"
#define RF_DATA_PARTITION "/dev/mtd/8"
#define BACKUP_PARTITION "/dev/mtd/6"
#endif



#define APPS_PATH   "/apps.sqsh"


#ifdef __KERNEL__

/* cfho 2009-0730, TODO, need to check where should we define the HAVE_EXTERNAL_FLASH_MAP, hardcode here */
/*define it in kernel/ar_7100_flash*/
/* #define HAVE_EXTERNAL_FLASH_MAP 1 */
#ifdef HAVE_EXTERNAL_FLASH_MAP
#endif

#ifdef __cplusplus
}
#endif
#endif
#define tr(x) x


#endif
