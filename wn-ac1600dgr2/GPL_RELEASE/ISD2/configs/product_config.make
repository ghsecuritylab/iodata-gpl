#######################################################################
##
## Embedded Target Makefile (ISD2)
## Platform: Atheros AR955x
## Product: WN-AC1600DGR_SCO
## Author: Sam Huang
##
#######################################################################

PRODUCT_NAME=WN-AC1600DGR_SCO
CONFIG_WN-AC1600DGR_SCO=y
CONFIG_NO_BRAND=n
CONFIG_ATHEROS_PLATFORM=y
CONFIG_ATHEROS_NEW_CONFIG_TERM=y

#######################################################################
#
# Firmware type
# 
#######################################################################
export CONFIG_USE_COMBINED_FW_IMAGE=y
##export CONFIG_USE_ONE_FW_IMAGE=n
##export CONFIG_HAS_FW1_FW2=y
##export CONFIG_NO_APPSCORE=y
##CONFIG_HAS_BACKUP_IMAGE=y


CONFIG_VLAN_SUPPORT=y
CONFIG_WSC_UPNP_SUPPORT=y
CONFIG_HAS_PNPX_FUNCTION=y
#######################################################################
#
# Kernel setting (overwrite the setting in config.make) 
# 
#######################################################################
# Jerry: Move settings to /ISD2/configs/config_mips74kc_AR955x_linux2.6.31.make
# KERNEL_VER =  2.6.31
# KERNEL_DIR =  linux-2.6.31
# KERNEL_FULL_DIR = $(TOP_DIR)/ATHEROS_LSDK_KNL/$(KERNEL_DIR)
# KIMAGE_DIR = $(KERNEL_FULL_DIR)/images
# KERNEL_HEADERS_INCLUDE = -I$(KERNEL_FULL_DIR)/include
# KERNEL_ZIP =  linux-2.6.31.gz
# AUTOCONF_H_DIR   = $(KERNEL_FULL_DIR)/include/linux
# KCFLAGS= -D__KERNEL__ -I$(KERNEL_FULL_DIR)/include -Os \
         -fno-strict-aliasing -fno-common -fomit-frame-pointer \
         -I$(KERNEL_FULL_DIR)/include/asm/gcc -G 0 -mno-abicalls \
         -fno-pic -pipe  -finline-limit=100000 -march=mips32r2 -mabi=32 \
         -Wa,--trap -DMODULE -mlong-calls

#######################################################################
#
# System Related
# 
#######################################################################
CONFIG_HAS_INITRAMFS=y
CONFIG_HAS_APPSCORE_ON_MTD=n

# Applications (hostapd, wpa supplication, use openssl lib)
CONFIG_USE_OPENSSL_SHARED_LIB=y

# openssl only support wap_supplicant
CONFIG_OPENSSL_ONLY_SUPPORT_WPA_SUPPLICANT=n

# To control LED
CONFIG_HAS_ATHEROS_GPIO=y

# Flash sector size
## undefined(default: 64KB) ##
# CONFIG_FLASH_SECTOR_SIZE=4096   ###this value is depending on flash sectore size
                                ###please refer to HW spec
HOSTAPD_WLAN_DRIVER_DIR=$(ATHEROS_APPS_DIR)/LSDK-WLAN-10.x-11ac
#######################################################################
#
# Ethernet Switch 
# 
#######################################################################
CONFIG_HAS_ETHERNET_SWITCH=y
CONFIG_HAS_ATH_ETHERNET_SWITCH=y
CONFIG_HAS_ATH_ETHERNET_SWITCH_S17=n
CONFIG_HAS_ATH_ETHERNET_SWITCH_S27=n
CONFIG_HAS_ATH_ETHERNET_SWITCH_S17_HWACCEL=y

CONFIG_AR8327_JUMBO_FRAME=y

### new add for Atheros Switch ###

#samhuang porting
export BOARD_TYPE=ap136
ETH_NAME=AR955x_eth
ENETDIR=$(ATHEROS_APPS_DIR)/$(ETH_NAME)
# for ap136 evb board,samhuang porting 
# export ETH_CONFIG=_s17
export ETH_CHIP_CONFIG=_AR833X
export ATH_CFG_SGMII=0
#for ag300dgr
export ETH_CONFIG=_s17_hwaccel
export ATH_S17_FULL_FEATURE=1

# For S17 H/W Accelerator
#ifeq ($(ETH_CONFIG),_s17_hwaccel) #{
#export BUILD_CONFIG=_routing
#endif #}

#samhuang porting
export GMAC_QCA955x=1
export ATH_GMAC_DRIVER=1
export MAC06_EXCHANGE_EN=1
#######################################################################
# Lib Related
#######################################################################
# Applications use libmath
export CONFIG_USE_LIBMATH=y
# Applications use libz
export CONFIG_USE_LIBZ=y
# Applications use librt
export CONFIG_USE_LIBRT=y
# Applications use libssl
export CONFIG_USE_LIBSSL=n
# Applications use libusb
export CONFIG_USE_LIBUSB=n
# Use I2C device
export CONFIG_USE_I2C=n
# Use RTC device
export CONFIG_USE_RTC=n
export CONFIG_USE_LIB_GCC=y
# Application use gmp
export CONFIG_USE_LIBGMP=y
################################################
# Atheros S17 SWITCH init(vlan, hnat, igmpsnoop)
################################################
CONFIG_ATH_S17_PROPRIETARY_INIT=y

#CONFIG_ATH_S17_HNAT=n

#CONFIG_ATH_S17_IGMPSNOOP=n
# igmpsnooping releated,  0011 1111 0011 1111 0010 0001 0011 1111
CONFIG_ATH_S17_GLOFW_CTRL1_REG_VALUE=0x3f3f213f
CONFIG_ATH_S17_GLOFW_CTRL1_REG_VALUE_DEFAULT=0x007f7f7f # S17_BROAD_DPALL | S17_MULTI_FLOOD_DPALL | S17_UNI_FLOOD_DPALL : 0111 1111 0111 1111 0111 1111
CONFIG_ATH_S17_ARL_CTRL_REG_VALUE=0x50f8002b # igmpsnooping releated:  0101 0000 1111 1000 0000 0000 0010 1011
CONFIG_ATH_S17_FRAME_ACK_CTL1_OFFSET_VALUE=0x01060606 # igmpsnooping ,enable IGMP JOIN LEAVE on port 4 ~ 6,: enable IGMP v3/MLD V2, bit 24: 0001 0000 0110 0000 0110 0000 0110
CONFIG_ATH_S17_FRAME_ACK_CTL0_OFFSET_VALUE=0x06060606 # igmpsnooping releated, enable IGMP JOIN LEAVE on port 0 ~ 3: 0110 0000 0110 0000 0110 0000 0110

CONFIG_ATH_S17_PWS_REG_VALUE=0x80000000 #Power on strapping, aka PWS ##0xc1000000

#CONFIG_ATH_S17_IP_VALIDATION=y
CONFIG_ATH_S17_NORMALIZE_CONTROL0=0x0200
CONFIG_ATH_S17_NORMALIZE_CONTROL0_VALUE=0x0000a076
CONFIG_ATH_S17_NORMALIZE_CONTROL1=0x0204
CONFIG_ATH_S17_NORMALIZE_CONTROL1_VALUE=0x00000007

CONFIG_ATH_S17_P0PAD_MODE_REG_VALUE=0x07a00000  # PORT0 PAD Mode
CONFIG_ATH_S17_P6PAD_MODE_REG_VALUE=0x01000000  # PORT6 PAD Mode
CONFIG_ATH_S17_P0STATUS_REG_VALUE=0x7e          # PORT0_STATUS register
CONFIG_ATH_S17_P6STATUS_REG_VALUE=0x7e          # PORT6_STATUS register

CONFIG_ATH_S17_GLOFW_CTRL0_REG_VALUE=0x000004f0 # value is provided by vendor
CONFIG_ATH_S17_P0LOOKUP_CTRL_REG_VALUE=0x0014017e # value is provided by vendor
CONFIG_ATH_S17_P1LOOKUP_CTRL_REG_VALUE=0x0014017d # value is provided by vendor
CONFIG_ATH_S17_P2LOOKUP_CTRL_REG_VALUE=0x0014017b # value is provided by vendor
CONFIG_ATH_S17_P3LOOKUP_CTRL_REG_VALUE=0x00140177 # value is provided by vendor
CONFIG_ATH_S17_P4LOOKUP_CTRL_REG_VALUE=0x0014016f # value is provided by vendor
CONFIG_ATH_S17_P5LOOKUP_CTRL_REG_VALUE=0x0014015f # value is provided by vendor
CONFIG_ATH_S17_P6LOOKUP_CTRL_REG_VALUE=0x0014013f # value is provided by vendor
#CONFIG_ATH_S17_VLAN_PVID1=0x00010001
#CONFIG_ATH_S17_VLAN_PVID2=0x00020001
CONFIG_ATH_S17_P0VLAN_CTRL0_REG_VALUE=0x00010001
CONFIG_ATH_S17_P1VLAN_CTRL0_REG_VALUE=0x00010001
CONFIG_ATH_S17_P2VLAN_CTRL0_REG_VALUE=0x00010001
CONFIG_ATH_S17_P3VLAN_CTRL0_REG_VALUE=0x00010001
CONFIG_ATH_S17_P4VLAN_CTRL0_REG_VALUE=0x00010001
CONFIG_ATH_S17_P6VLAN_CTRL0_REG_VALUE=0x00010001
CONFIG_ATH_S17_P5VLAN_CTRL0_REG_VALUE=0x00020001
CONFIG_ATH_S17_P0VLAN_CTRL1_REG_VALUE=0x00002040
CONFIG_ATH_S17_P1VLAN_CTRL1_REG_VALUE=0x00001040
CONFIG_ATH_S17_P2VLAN_CTRL1_REG_VALUE=0x00001040
CONFIG_ATH_S17_P3VLAN_CTRL1_REG_VALUE=0x00001040
CONFIG_ATH_S17_P4VLAN_CTRL1_REG_VALUE=0x00001040
CONFIG_ATH_S17_P5VLAN_CTRL1_REG_VALUE=0x00001040
CONFIG_ATH_S17_P6VLAN_CTRL1_REG_VALUE=0x00001040
CONFIG_ATH_S17_VTU_FUNC0_REG_VLAN_PVID1_VALUE=0x001bd560
CONFIG_ATH_S17_VTU_FUNC0_REG_VLAN_PVID2_VALUE=0x001b7fe0
CONFIG_ATH_S17_VTU_FUNC1_REG_VLAN_PVID1_VALUE=0x80010002
CONFIG_ATH_S17_VTU_FUNC1_REG_VLAN_PVID2_VALUE=0x80020002

CONFIG_ATH_S17_NORMALIZE_CONTROL0=0X0200
CONFIG_ATH_S17_NORMALIZE_CONTROL0_VALUE=0x0000A076
CONFIG_ATH_S17_NORMALIZE_CONTROL1=0X0204
CONFIG_ATH_S17_NORMALIZE_CONTROL1_VALUE=0x00000007

export ATH_GMAC_LEN_PER_TX_DS=1536
#samhuang porting, for ap136 evb board
export ATH_GMAC0_TX_CNT=128
export ATH_GMAC0_RX_CNT=128
export ATH_GMAC1_TX_CNT=128
export ATH_GMAC1_RX_CNT=128
#for ag300dgr
#ATH_GMAC0_TX_CNT=5
#ATH_GMAC0_RX_CNT=232
#ATH_GMAC1_TX_CNT=5
#ATH_GMAC1_RX_CNT=232

export ATH_GMAC_LOCATION=0xbfff0000 #### fake, we don't use ###
export ATH_GMAC_TXQUEUELEN=1024

export ATH_GMAC_HW_QOS=0
export ATH_GMAC_RX_TASKLET=1
export ATH_GMAC_RX_PKT_CNT=100
#ATH_GMAC_RX_PKT_CNT=16 #for ag300dgr
export ATH_GMAC0_RXFCTL=1
export ATH_GMAC0_TXFCTL=1
export ATH_GMAC1_RXFCTL=1
export ATH_GMAC1_TXFCTL=1
export ATH_GMAC_GE0_IS_CONNECTED=1
#for ap136 evb board, samhuang porting
export ATH_GMAC_GE1_IS_CONNECTED=0
#for ag300dgr
#ATH_GMAC_GE1_IS_CONNECTED=0
export ATH_SWITCH_ONLY_MODE=0
export ATH_GMAC_DESC_SRAM=0

#samhuang porting
export ATH_GMAC0_MII=ATHR_RGMII
#export ATH_GMAC1_MII=ATHR_SGMII
#sgmii defines
#export ATH_CFG_SGMII=1
#export ATH_CONFIG_SWAP_SGMII=0
#export ATH_S17_PHY0_WAN=1
#export ATH_SGMII_FORCED=1
export ATH_SUPPORT_VOW_DCS=1

export ATH_GMAC_RXTIMER=1
export ATH_GMAC_RXTIMER_FREQ=500
#####################################################################
##
## RADIO Setting
## cfho 2008-0722
## The sysconfd can config up to two radios (ra+ath, 2 ra, or 2 ath)
########################################################################
CONFIG_HAS_RADIO_SETTING=y

#######################################################################
#
# Atheros WLAN (AR95xx) for AP136 (AR955x+AR95xx)
#
#######################################################################
ATHEROS_WLAN_DRIVER_VER=10
CONFIG_ATHEROS_RXCHAINMASK=7
CONFIG_ATHEROS_TXCHAINMASK=7
CONFIG_ATHEROS_SUPPORT_DEBUG_DOMAIN=n
CONFIG_ATHEROS_SUPPORT_HIGH_POWER=n
CONFIG_ATHEROS_SUPPORT_AR9280=y
CONFIG_ATHEROS_RF_DATA_ON_FLASH=y
CONFIG_ATHEROS_AH_CAL_LOCATIONS="0xbfff0000"
CONFIG_ATHEROS_CAL_NUM_OF_RADIOS=1
CONFIG_ATHEROS_HAS_XAT=n
CONFIG_HAS_WLAN_STA_FUNCTION=n
CONFIG_HAS_ATHEROS_WLAN=y
CONFIG_HAS_ATHEROS_WLAN_A_BAND=y
CONFIG_HAS_ATHEROS_AR10K=y
CONFIG_HAS_SINGLE_RADIO_REPEATER=n
#This will use 3 LED (using GPIO of RT2880) for indicating both the WLAN traffic and RSSI of client.
#CONFIG_ATHEROS_HAS_RSSI_LED_INDICATION=y
#CONFIG_ATHEROS_TX99=n
### ATH_SUPPORT_TXPOWER_ADJUST ?? todo: need this func. ###
#CONFIG_ATHEROS_SUPPORT_TXPOWER=y
### select region/domain ###
#CONFIG_ATHEROS_SUPPORT_SELECT_RD=y

CONFIG_ATHEROS_SUPPORT_WDS_IOCTL=y
CONFIG_ATHEROS_RSN_IE_PATCH=y
CONFIG_ATHEROS_SCAN_PATCH=y
CONFIG_ATHEROS_SUPPORT_11N=y

### SENAO 802.11ac settings ###
CONFIG_ATHEROS_SUPPORT_11AC=y

CONFIG_AUTO_CHANNEL=y
### use senao autochannelselection medthod  ###
CONFIG_USE_SENAO_AUTO_CHANNEL_SELECTION=n
#CONFIG_HAS_NEWAUTOCHANNEL_ALGTHM=y
#CONFIG_HAS_40BANDWIDTH_NEWAUTOCHANNEL=y

### use atheros ACS module to do autochanneling  ###
CONFIG_USE_ATHEROS_AUTO_CHANNEL_SELECTION=y

### SENAO DFS MODIFY ###
CONFIG_ATHEROS_SENAO_MODIFY_DFS_W52=y

CONFIG_ATHEROS_FLAGS_EXT2=y
#Enable/Disable radio, use flags ext2
CONFIG_ATHEROS_SUPPORT_RADIO_ONOFF=y
#CONFIG_ATHEROS_SUPPORT_WLANLED_ONOFF=n
#CONFIG_ATHEROS_DISABLE_TURBO_CMD=n
CONFIG_ATHEROS_FIXED_40CHWIDTH=y

#20120321 Jason :add ssid sta limit
CONFIG_SUPPORT_MAXIMUM_STA_NUM=y

CONFIG_PROBE_PCIE_WLAN01_AND_REBOOT=n
####################### Wi-Fi CERTIFIED (11N test) ####################
#Support to disable beacon for AP
#CONFIG_ATHEROS_DISABLE_BEACON=y
#Support WPS RF band in dual band
#CONFIG_HAS_HOSTAPD_WPS_DUAL_RF=n


## Adonn 08-1017: modify default reg domain from FCC1_FCCA(0x10) to FCC3_FCCA(0x3A)
## FCC1_FCCA: USA
## FCC3_FCCA: USA & Canada w/5470 band, 11h, DFS enabled
CONFIG_ATHEROS_MODIFY_DEFAULT_RD=y

#Jay 20081022 Rate Control#
#if using ONOE, wds only can work in b mode#
ATHEROS_RATECTRL_ATHEROS=y
ATHEROS_RATECTRL_ONOE=n

#jaykung WPS related
# Wi-Fi Protected Setup (WPS)
CONFIG_ATHEROS_SUPPORT_WPS=y
CONFIG_ATHEROS_SUPPORT_WPS2=y
CONFIG_ATHEROS_SUPPORT_WPS_STRICT=n
CONFIG_ATHEROS_HOSTAPD_WPS_PBC_IN_M1=y
CONFIG_ATHEROS_DISABLE_WPS2_AT_STA=y
# Enable UPnP support for external WPS Registrars
CONFIG_ATHEROS_SUPPORT_WPS_UPNP=y
#Enealbe WPS_LED control
CONFIG_ATHEROS_SUPPORT_WPS_LED=y
# jerry: Save WPS event status in file
#CONFIG_SAVE_HOSTAPD_WPS_STATUS_IN_FILE=y


CONFIG_ATHEROS_SUPPORT_MULTIPLE_SSID=y

CONFIG_ATHEROS_SUPPORT_WPS_LED_SIMPLE_CONFIG=y
CONFIG_ATHEROS_WDT_TRIGGER=y
#CONFIG_ATHEROS_SENAO_MODIFY_MAC=y
CONFIG_ATHEROS_FAKE_DFS_TEST=y
#CONFIG_ATHEROS_SENAO_MODIFY_SCAN=y

#jaykung turn CONFIG_ATHEROS_SUPPORT_COUNTRYCODE on to set countrycode base
CONFIG_ATHEROS_SUPPORT_COUNTRYCODE=y
CONFIG_ATHEROS_SUPPORT_COUNTRYCODE_TRADITION=y
##turn CONFIG_ATHEROS_SUPPORT_COUNTRYCODE on to set regular domain to atheros driver
CONFIG_ATHEROS_SUPPORT_REGDOMAIN=n

CONFIG_ATHEROS_SUPPORT_SET_MACADDR=n

#jaykung
#iolsation between ssid
CONFIG_ATHEROS_SUPPORT_BSSID_ISOLATION=y
#user can set max aid number
#CONFIG_ATHEROS_SUPPORT_USER_DEFINE_AID_NUM=y
#jaykung WPS related
# Wi-Fi Protected Setup (WPS)
#CONFIG_ATHEROS_SUPPORT_WPS=y
# Enable UPnP support for external WPS Registrars
#CONFIG_ATHEROS_SUPPORT_WPS_UPNP=y
#Enalbe WPS_LED control
#CONFIG_ATHEROS_SUPPORT_WPS_LED=y
CONFIG_ATHEROS_DETAIL_CLIENT_LIST=y
#CONFIG_ATHEROS_DFS_CHANNEL_LOG=n
#CONFIG_HAS_ONLY_LAN_FUNCTION=y
CONFIG_ATHEROS_SUPPORT_FOLLOWUP_1ST_BSSID_MACADDR=n

#######################################################################
#
# Atheros WLAN (AR955x) 
#
#######################################################################
#
# UMAC build option
#
export BUILD_UMAC=1

# Set Phy Err Diagnostics (Radar detection) to be enabled for AP builds
 #enable for AP136 evb board,samhuang porting
export ATH_CAP_PHYERR_DIAG=1

#samhuang porting
export AR5416_G_MODE=1
export AR9100=0

export AH_DEBUG=0

#samhuang porting
export AH_SUPPORT_AR5210=0

export AH_SUPPORT_AR5212=0
export AH_SUPPORT_5111=0
export AH_SUPPORT_5112=0
export AH_SUPPORT_2413=0
export AH_SUPPORT_5111=0
export AH_SUPPORT_5112=0
export AH_SUPPORT_2413=0
export AH_SUPPORT_5413=0
export AH_SUPPORT_2316=0
export AH_SUPPORT_2317=0
export AH_SUPPORT_2425=0
export AH_SUPPORT_HOWL=0
export AH_SUPPORT_SOWL=0
#samhuang porting
export AH_SUPPORT_AR5416=1
#samhuang porting
#export AH_SUPPORT_AR9300=1

#export AH_SUPPORT_EEPROM_AR9330=1 
#export AH_SUPPORT_HORNET=0
#samhuang porting
export AH_SUPPORT_KITE_ANY=1
export AH_SUPPORT_KITE_10=1
export AH_SUPPORT_KITE_12=1
export AH_SUPPORT_KIWI_10=1
export AH_SUPPORT_KIWI_ANY=1
export AH_SUPPORT_KIWI_11=1
 
export AH_SUPPORT_K2=0
#samhuang porting
export AH_SUPPORT_EEPROM_AR9287=1
export ATH_SUPPORT_VLAN=1

export ATH_SUPPORT_IQUE=1
#export ATH_SUPPORT_IQUE_EXT=1
export ATH_CHAINMASK_SELECT=0 
#export ATH_RXBUF=128
#samhuang porting
export ATH_RXBUF=512
export ATH_TXBUF=512

export ATH_CAP_AMSDU=1
export IEEE80211_MCAST_ENHANCEMENT=1
export ATH_RB=0 
export AH_SUPPORT_OWL=1
# samhuang porting
# 2014-05-02 Disable AR5416_INT_MITIGATION will stable when burn DUT (BT) 
# export AR5416_INT_MITIGATION=1

export ATH_DEBUG=0
### 0 for v10.0.91
export UMAC_SUPPORT_TX_FRAG=1
export ATH_TX_COMPACT=1
export ATH_11AC_TXCOMPACT=1

export ATH_SUPPORT_MCAST2UCAST=1
export ATH_SUPPORT_GREEN_AP=1
export ATH_SUPPORT_DYN_TX_CHAINMASK=1
#samhuang porting
export AH_DESC_NOPACK=1

export ATH_SUPPORT_PAPRD=1
#export ATH_TRAFFIC_FAST_RECOVER=1
#export UMAC_SUPPORT_OPMODE_APONLY=1
#export UMAC_SUPPORT_STATS_APONLY=1
#export ATH_SUPPORT_STATS_APONLY=1

#samhuang porting
export AH_SUPPORT_SCORPION=1
export AH_SUPPORT_WASP=0

export ATH_EXT_AP=1

export ATH_SUPPORT_QUICK_KICKOUT=1

# S/W retry mechanism (in addition to H/W retries) +
# TID queue pause/unpause and ps-poll handling in LMAC
export ATH_SWRETRY=1
# ATH_SWRETRY_MODIFY_DSTMASK enables HW optimization to filter pkts on
# failures.
export ATH_SWRETRY_MODIFY_DSTMASK=0

# 11ac offload
export REMOVE_PKT_LOG=0
export WDI_EVENT_ENABLE=1
export ATH_SUPPORT_TxBF=0
export ATH_SUPPORT_WEP_MBSSID=1
export ATH_SUPPORT_DESC_CACHABLE=1
export ATH_TXQ_BH_LOCK=1
export UMAC_SUPPORT_APONLY=1
### new from v10.1.45
export ATH_WDS_SUPPORT_APONLY=1
export ATH_SUPPORT_WIFIPOS=0
#export UMAC_SUPPORT_TXDATAPATH_NODESTATS=0
#export ADF_OS_DEBUG=0

# 0: 128B - default, 1: 256B, 2: 64B
export ATH_OL_11AC_DMA_BURST=1
export ATH_OL_11AC_MA_AGGR_DELIM=0

# if 11ac offload enable
export UMAC_SUPPORT_RESMGR_SM=1
export UMAC_SUPPORT_RESMGR=1
export UMAC_SUPPORT_RESMGR_OC_SCHED=1
export QCA_OL_11AC_FAST_PATH=1
export ATH_VAP_PAUSE_SUPPORT=1
export ATH_PERF_PWR_OFFLOAD=1
export ATH_TGT_TYPE=AR9888
export ATH_HIF_TYPE=pci
export FORCE_LEGACY_PCI_INTERRUPTS=1
export LOAD_ARRAY_FW=1
# end of 11ac offload enable

export BIG_ENDIAN_HOST=1
export ATH_SUPPORT_KEYPLUMB_WAR=1
export ATH_SUPPORT_TIDSTUCK_WAR=1
export LMAC_SUPPORT_POWERSAVE_QUEUE=1
# ~11ac offload

export ATH_SUPPORT_SPECTRAL=0
#turn on ap-only code
#ifneq ($(ATH_SUPPORT_SPECTRAL),1)
#export UMAC_SUPPORT_APONLY=1
#endif
#export BUILD_WPA2=y

#ifeq ($(BUILD_WPA2),y)
#export ATH_WPS_IE=1
#export MADWIFIPATH=$(TOPDIR)/wlan/linux
#export MADWIFIINC=$(TOPDIR)/wlan
#else
#export ATH_WPS_IE=0
#endif
#samhuang porting
export ATH_WPS_IE=1


# Set the GPIO PIN for WPS LED
#export WPS_LED_GPIO_PIN=21

#export ATH_WDS_INTEROP=0

# No 5G support
export ATH_NO_5G_SUPPORT=0

#samhuang porting
export CONFIG_MIPS_74K_KERNEL_OPTIMIZATION=1
export ATH_SUPPORT_CWM=1
export ATH_CAP_CWM=1

#######################################################################
#
# Atheros AR93xx
#######################################################################
# Select the support required for this board
# please refer hal/linux/Makefile.inc for detail
#samhuang porting
#export KERNELVER=2.6.31
export KERNELVER=2.6.31
export KERNELARCH=mips
export JUMPSTART_GPIO=16

export ATH_SUPPORT_RX_PROC_QUOTA=0


export ATH_SUPPORT_VOWEXT=0
export ATH_VOW_EXT_STATS=0
#export ATH_SUPPORT_VOW_DCS=1
export DIRECT_ATTACH=1
# Periodically measure performance stats such as throughput, PER
# But enabling this option can affect peak performance slightly.
export UMAC_SUPPORT_PERIODIC_PERFSTATS=0

# Measure channel utilization
# Enabling this option can affect peak performance slightly.
export UMAC_SUPPORT_CHANUTIL_MEASUREMENT=1

#enable BSS load support
export UMAC_SUPPORT_BSSLOAD=1
#enable radio measurement support for voice enterprise
### 0 for v10.0.91
export UMAC_SUPPORT_RRM=1
#enable wireless network management support for voice enterprise
export UMAC_SUPPORT_WNM=0
#enable WMM admission control support for voice enterprise
export UMAC_SUPPORT_ADMCTL=1
#enable quiet period support
export UMAC_SUPPORT_QUIET=1
#enable UAPSD RATECTL
export ATH_SUPPORT_UAPSD_RATE_CONTROL=1
# Randomness generation support from hardware
export ATH_GEN_RANDOMNESS=1

# IEEE 802.11v Proxy ARP
export UMAC_SUPPORT_PROXY_ARP=1

# Hotspot 2.0 DGAF Disable support
export UMAC_SUPPORT_DGAF_DISABLE=1

# Hotspot 2.0 L2 Traffic Inspection and Filtering support
export UMAC_SUPPORT_HS20_L2TIF=1
export ATH_SUPPORT_HS20=1

export ATH_HW_TXQ_STUCK_WAR=1
# Disable STA mode support to gain some CPU cycles
#export UMAC_SUPPORT_STA_MODE=0
#~samhuang porting

# PERE_IP_HDR_ALIGNMENT_WAR=1 will cause 5G can't communicate each other
# PERE_IP_HDR_ALIGNMENT_WAR=0, 5G sometime will hang at "XXX TARGET ASSERTED XXX"
# export PERE_IP_HDR_ALIGNMENT_WAR=1


#export AH_CAL_IN_FLASH=1
#export AH_CAL_RADIOS=1
#export AH_CAL_LOCATIONS=0xbfff0000

export BUS=dual
#export BUS=AHB
export AP_TYPE=dual
#export AP_TYPE=single
export AH_CAL_IN_FLASH_PCI=1
#export AH_CAL_RADIOS_PCI=1
export AH_CAL_LOCATIONS_PCI=0xbfff4000
export AH_CAL_IN_FLASH_AHB=1
export AH_CAL_RADIOS_AHB=1
export AH_CAL_LOCATIONS_AHB=0xbfff0000

#samhuang porting
export ATH_SUPPORT_DFS=1
#export AP_USB_LED_GPIO=13
#export AP_USB2_LED_GPIO=4
#export MADWIFITARGET=mipsisa32-be-elf
#export FUSIONTARGET=mipsisa32-be-elf
#export TARGETARCH=mipsisa32-be-elf
export MADWIFITARGET=mips-be-elf
export FUSIONTARGET=mips-be-elf
export TARGETARCH=mips-be-elf

export ATH_SUPPORT_LED=1
export GPIO_PIN_FUNC_0=0
export GPIO_PIN_FUNC_1=0
export GPIO_PIN_FUNC_2=0

# 2012-02-01 sysgpio.h/SENAO_GPIO_LED_WLAN instead of product_config.make/CONFIG_ATH_WLAN_LED_1
# GPIO PIN of WLAN LED
# export CONFIG_ATH_WLAN_LED_1=14

# 20120321 SamHuang: add ksocket into wlan module
export BUILD_KSOCKET=1
export SENAO_SUPPORT_SYSLOG=1
#######################################################################
#
# Wireless LAN (RT2860)
# marklin 20080114 ; reference linux-2.6.21/.config
#######################################################################
CONFIG_NOT_SUPPORT_RALINK_WLAN=y
#######################L2_CONTROL####################################
CONFIG_SEPARATE_ISOLATION_BSSID=y
#####################################################################
CONFIG_COUNT_BY_EACH_STA=y
#######################################################################
# jaykung
# if CONFIG_SENAO_CONSTANT_CFG=y, driver will load parameters from 
# /apps/ralink_ap.cfg or /apps/ralink_sta.cfg after DAT file is loaded
#######################################################################
#CONFIG_SENAO_CONSTANT_CFG=y
## ap
CONFIG_HAS_LOCAL_NTP_SERVER=y
#######################################################################
##
## Wireless LAN (wpa_supplicant)
## cfho 2008-0731
########################################################################
CONFIG_WPA_SUPPLICANT_SUPPORT_MADWIFI=y
CONFIG_WPA_SUPPLICANT_SUPPORT_RALINK=n

#Austin: support WPS in client mode
CONFIG_ATHEROS_SUPPORT_STA_WPS=y

#CONFIG_USE_WPA_SUPPLICANT_TO_SCAN=y
#CONFIG_USE_WPA_SUPPLICANT_FIXED_MAC=n

#######################################################################
##
## Wireless LAN (ART)
## Jay 20100428
########################################################################
CONFIG_ATH_HAS_ART2=y
CONFIG_ART_PB90=y
CONFIG_ATH_CAL_OFFSET=0x1000
#CONFIG_ATH_CAL_OFFSET=0

#######################################################################
# VPN settings
#######################################################################
CONFIG_HAS_VPN=y
CONFIG_HAS_VPN_IPSEC=n
CONFIG_HAS_VPN_L2TP=n
CONFIG_HAS_VPN_PPTP=y
CONFIG_HAS_VPN_PPTP_MPPE=y
CONFIG_HAS_VPN_REMOTE_WITHIN_LAN=y

#######################################################################
#
# Multiple WAN settings
#
#######################################################################
CONFIG_HAS_MULTIPLE_WAN=y
CONFIG_HAS_WAN_JP_FLETS=y
CONFIG_HAS_DOMAIN_ROUTING=y
CONFIG_HAS_JP_STATIC_ROUTING=y

##############################
CONFIG_ATH_BOARD_TYPE=AP136 ##scorpion_pci_2 series
##############################

#######################################################################
#
# System config daemon modules, Ap_cfg, httpd
# 
#######################################################################
### Good TCP performance require more memory.  ###
### Make sure you are working on a 32MB+ board ###
CONFIG_GOOD_TCP_PERFORMANCE=y
##################################################
CONFIG_NEW_WLAN_SETTINGS=y
CONFIG_POWER_SAVING_WLAN=n
CONFIG_AP_PROFILE=y
#CONFIG_MULTIDMZ_SUPPORT=n
#CONFIG_HAS_INSTALL_WIZARD=y
CONFIG_HAS_QOS_ENGENIUS=y
CONFIG_HAS_QOS_SITECOM=n
CONFIG_HAS_QOS_TC=y
CONFIG_STATIC_ROUTE_ENGENIUS=y
CONFIG_USE_PRODUCT_ID=y
CONFIG_HAS_LLTD=y
CONFIG_SCHEDULE_ENGENIUS=y
CONFIG_HASNT_NORMAL_SCHEDULE=y
CONFIG_SCHEDULE_COREGA=n
CONFIG_SCHEDULE_ZYXEL=n
#CONFIG_HAS_NETBIOS_FUNCTION=n
CONFIG_HAS_NETBIOSD=n
CONFIG_PACKET_HANDLER=y
CONFIG_NETPKTHANDLE_SUPPORT_BADSITE_FILTER=y
CONFIG_PACKET_HANDLER_PASSTHROUGH=y
CONFIG_PACKET_HANDLER_PASSME=n
CONFIG_PACKET_HANDLER_RATECTRL=n
CONFIG_PACKET_HANDLER_PASSTHROUGH4=y
CONFIG_NETPKTHANDLE_HAS_ETH_IGMP_SNOOPING=y
CONFIG_NETPKTHANDLE_WLAN_COPY_SKB=y
CONFIG_PPPOE_JAPAN=n
CONFIG_WAN_MANAGER=y
CONFIG_AP_BRIDGE_HAS_DHCP_SERVER=n
CONFIG_HAS_MACFILTER_IMPORT=n

CONFIG_HAS_SUPPORT_ONE_WEP=y

CONFIG_HAS_AIRPORT_FINDER=y
CONFIG_HAS_AIRPORT_QUICK_COPY=y
CONFIG_HAS_AIRPORT_QUICK_COPY_V2=n
CONFIG_HAS_AIRPORT_QUICK_COPY_HG=n
CONFIG_HAS_AIRPORT_CHILD_FILTER=n
CONFIG_HAS_EBTABLE=y
CONFIG_HAS_WLAN2LAN_ACL=y

CONFIG_IP_NF_IP_ALIGNMENT=y

#recode associated client number for each bssid
CONFIG_HAS_EACH_SSID_CLINUM=y
CONFIG_HAS_HW_NAT=y
CONFIG_HW_NAT_ACL_RULE=y
########## debug ##########
CONFIG_NO_PRINTF_APCFG=n
CONFIG_NO_PRINTF_SYSCONFIG=n

# ez-ipupdate has iobb DDNS
CONFIG_HAS_IOBB_DDNS=y

#20120518 Jason: udhcpc always brocast discovery packet
CONFIG_HAS_LAN_DHCPC_BACKOFF_FOREVER=y

#CONFIG_SUPPORT_REAL_MULTI_WLAN_PLATFORM=y
#20120605 Jason: add for dnsmasq - dynamic routing table funciton(Multiple PPPoE)
CONFIG_HAS_DNSMASQ_DNSREPLY_TO_SYSCONF_CLI=y
#######################################################################
#
# IPv6 Settings (sysconfig ipv6 setting)
# 
#######################################################################
#CONFIG_HAS_TC_IPROUTE=y

#######################################################################
#
# Unix Util (Mount, swapon, swapoff )
# 
#######################################################################
CONFIG_UNIT_UTIL_HAS_NO_SWAPONOFF=y

#######################################################################
#
# Language File
# 
#######################################################################
HAS_MULTIPLE_LANGUAGE=n
HAS_RUSSIAN_LANGUAGE=n
HAS_JAPANESE_LANGUAGE=y

CONFIG_APCFG_USE_STORAGE2=n
CONFIG_HAS_FWM=y
CONFIG_USE_SIMPLE_TIMEZONE_TABLE=y

CONFIG_HTTPD_REMOTEPORT=y

CONFIG_WEB_FOR_ATHEROS=y
#######################################################################
##
## Modifcations in net-snmp
##
########################################################################
#
#CONFIG_SNMPD_SUPPORT=y
#CONFIG_SNMP_TRAP_SUPPORT=y
#CONFIG_NETSNMP_DISABLE_SNMPV3=y
#SNMP_MIB_MODULES_INCLUDED_ENTERPRISE=senao/entSystem senao/entSNMP senao/entLAN senao/entMacFilter
#SNMP_ENTERPRISE_OID=14125
#SNMP_ENTERPRISE_SYSOID=.1.3.6.1.4.1.14125.100.1.3

#######################################################################
#
# Modifcations in busybox (udhcpd)
# 
#######################################################################
#CONFIG_DHCPD_HAS_HOSTNAME=y
#CONFIG_DHCPD_HAS_CLIENT_IFNAME=y
#######################################################################
##
## Modifcations in kernel modules
##
########################################################################
#CONFIG_NOT_SUPPORT_NAT_MODULE=y
#######################################################################
##
## Modifcations in iptables
##
#######################################################################
IPTABLES_PF_EXT_LIB_LIST=conntrack mac IMQ LOG MASQUERADE REJECT mark iprange tcpmss tcp udp state \
		icmp standard MARK physdev policy tos TCPMSS TOS length limit multiport string DNAT SNAT
#######################################################################
#
# Web
# 
#######################################################################
#MRTG is Multi Router Traffic Grapher. Include mrtg, gd, libpng#
CONFIG_HAS_MRTG=n
CONFIG_USE_WEB_SCR_MINI=y


CONFIG_HTTPD_HAS_XML_AGENT=y

CONFIG_HAS_ATHEROS_5G_SETTING=n
CONFIG_HAS_MOBILE_PAGE_SUPPORT=y

# jerry chen: Allow users to make port forwarding changes through UPnP
CONFIG_UPNPD_PORTFORWARDING=y
#Support WEB upgrade DDWRT Software
CONFIG_HAS_SUPPORT_DDWRT=n
CONFIG_HAS_SUPPORT_WLAN_WPA_AES=y

CONFIG_WEB_NONE_LOGO_IMG=y
CONFIG_WEB_NONE_BOTTOM_IMG=y
CONFIG_HAS_BAD_SITE_FILTER=y

# 20120507 jason:add to support bootloader upgrade on WEB
CONFIG_BOOTLOADER_UPGRADE_ON_WEB=y

# 20120305 jason: support 5G WEB
CONFIG_HAS_RADIO_MIX_SETTINGS=y
CONFIG_HAS_ATHEROS_RADIO2_WLAN=y
CONFIG_HAS_SUPPORT_ATH_RADIO2_MACFILTER=y

WEB_INSTALL_FILE = \
win32/web_html/pictures/logo_iodata.gif:picture/logo.gif \
win32/web_html/pictures/title_iodata_bs.gif:picture/title.gif \
win32/web_html/pictures/bs_error_logo.gif:picture/bs_error_logo.gif \
win32/web_html/pictures/iobb.jpg:picture/iobb.jpg \
win32/web_html/pictures/netusb.gif:picture/netusb.gif \
win32/web_html/pictures/iodata2.gif:picture/iodata.gif \
win32/web_html/pictures/unknow.gif:picture/unknow.gif \
win32/web_html/pictures/manual.jpg:picture/manual.jpg \
win32/web_html/file/bstxt_v2.txt:language/bstxt_v2.txt \
win32/web_html/file/iobbtxt.txt:language/iobbtxt.txt \
win32/web_html/file/jquery.js:language/jquery.js \
win32/web_html/file/iobbtxt_mobile.txt:language/iobbtxt_mobile.txt

#######################################################################
#
# Silex module settings
# 
#######################################################################
CONFIG_HAS_SXUPTP_MODULE=y
CONFIG_HAS_SXPSPLUS_FUNC=y
CONFIG_HAS_SXNASPLUS_FUNC=y
CONFIG_HAS_SXDLNA_FUNC=y
CONFIG_HAS_IO_REMOTE_ACCESS_FUNC=y

#######################################################################
# Linuxigd (libupnpd)
#######################################################################
CONFIG_UPNPD_MAX_PORTMAPPING_LIMIT=100

#######################################################################
# Hostapd (hostapd)
#######################################################################
# manual config EAPOL Key (msg 1/4) timeout to fix Nintendo Wii delay response EAPOL Key (msg 2/4)
CONFIG_HOSTAPD_EAPOL_KEY_TIMEOUT_FIRST=300


#######################################################################
#
# factory configuration
# 
#######################################################################
CONFIG_FACTORY_SUPPORT_LED_TESTING=y
#20120103 George: add for WLR-4000v1002
CONFIG_SUPPORT_EXIA_LAN_TEST=y
CONFIG_HAS_SINGLE_ETH_PORT=y
CONFIG_HAS_INTERNAL_SWITCH=n
CONFIG_SUPPORT_SIMPLE_CONFIG_BUTTON=y
#######################################################################
#
# Ralink Ethernet driver "raeth" settings
# 
#######################################################################
#CONFIG_ETHDRV_MODULE=n
#CONFIG_RAETH=y
#CONFIG_RT_3052_ESW=y
#CONFIG_RT3052_ASIC=y
#CONFIG_LAN_WAN_SUPPORT=y
#CONFIG_WAN_AT_P4=y
#CONFIG_RALINK_RT3052=y
#CONFIG_RALINK_RT3052_MP=y
#CONFIG_MTD_PHYSMAP_START=0xBFC00000
#CONFIG_RAETH_NAPI=n
#CONFIG_RAETH_QOS=n
#CONFIG_RAETH_MCAST=n
#CONFIG_RAETH_MCAST_OPTIMIZE=n
#CONFIG_RAETH_MCAST_VARIABLE=y
#CONFIG_RAETH_CHECKSUM_OFFLOAD=n
#CONFIG_RAETH_ROUTER=n
#CONFIG_ICPLUS_PHY=n
#CONFIG_MAC_TO_MAC_MODE=n
#CONFIG_RALINK_RT2880=n
#CONFIG_ESW_DOUBLE_VLAN_TAG=n
#CONFIG_P5_RGMII_TO_MAC_MODE=n
#CONFIG_PSEUDO_SUPPORT=n
#CONFIG_RA_NAT_NONE=n
#CONFIG_RAETH_JUMBOFRAME=n
#CONFIG_RALINK_VISTA_BASIC=n
#CONFIG_RAETH_NETLINK=n
# pppd don't output any messages
CONFIG_HAS_PPPD_NO_MESSAGES=n
# build pppd with strict features
CONFIG_HAS_PPPD_STRICT_FEATURES=n
#######################################################################
##
## Atheros Ethernet driver ag7240 settings
##
########################################################################
#
#CONFIG_AG7240_SWITCH=y
CONFIG_WAN_AT_P5=y



ifeq ($(CONFIG_HAS_APPSCORE_ON_MTD),y)
APPS_CORE +=$(TARGET_APP_DIR)/sbin/appscore_init_ver.txt:/sbin/appscore_init_ver.txt
endif


