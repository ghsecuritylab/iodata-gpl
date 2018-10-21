#######################################################################
##
## Embedded Target Makefile (ISD2)
## PLATFORM: Atheros AR955x 11n SOC Router, Big Endian
##
#######################################################################

#PRODUCT_NAME = 1221
PLATFORM = MIPS32
BOARD = ATHEROS
CPU = AR955x
ARCH = mips
ARCH_TARGET = mips-be-elf
KERNEL_VERSION = LINUX2.6
KERNEL_VER = 2.6.31
KMODSUF = ko

CONFIG_UCLIBC_VERSION=0.9.31

#TOP_DIR = /ISD2
_TOP_DIR_ := $(dir $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))..
export TOP_DIR := $(realpath $(_TOP_DIR_))

UTIL_DIR = $(TOP_DIR)/util
EXTRA_MOD_DIR = $(TOP_DIR)/extras

TARGET_RAMDISK_DIR = $(TOP_DIR)/TARGET_RAMDISK
TARGET_DIR = $(TARGET_RAMDISK_DIR)/$(PLATFORM)_STAGE
TARGET_APP_DIR = $(TARGET_RAMDISK_DIR)/$(PLATFORM)_APPS_STAGE
TARGET_APP_CORE_DIR = $(TARGET_RAMDISK_DIR)/$(PLATFORM)_APPS_CORE_STAGE
TARGET_FACTORYAPP_DIR = $(TARGET_RAMDISK_DIR)/$(PLATFORM)_FACTORYAPPS_STAGE
TARGET_FACTORYART2_DIR = $(TARGET_RAMDISK_DIR)/$(PLATFORM)_FACTORYART2_STAGE
TARGET_LITTLEAPP_DIR = $(TARGET_RAMDISK_DIR)/$(PLATFORM)_LITTLEAPPS_STAGE
MOD_DIR = $(TARGET_DIR)/lib/modules/$(KERNEL_VER)


BUILD_ZIMAGE =	no
BUILD_BZIMAGE =	no
BUILD_RIMAGE =	no
BUILD_MIPS_UIMAGE    = yes
KERNEL_IMAGE_TYPE = uImage

#KNL_COMP_METHOD = gzip
KNL_COMP_METHOD = lzma
 
RAMDISK_SIZE = 1636

KERNEL_DIR = linux-2.6.31

KERNEL_FULL_DIR = $(TOP_DIR)/ATHEROS_LSDK_KNL/$(KERNEL_DIR)
KIMAGE_DIR = $(KERNEL_FULL_DIR)/images

APPS_FULL_DIR = $(TOP_DIR)/$(PLATFORM)_APPS
COMMONAPPS_FULL_DIR = $(TOP_DIR)/COMMON_APPS
CROSSLIB_DIR = $(TOP_DIR)/crosslibs
ATHEROSAPP_DIR = $(TOP_DIR)/ATHEROS_APPS
AUTOCONF_H_DIR   = $(KERNEL_FULL_DIR)/include/linux
KERNEL_HEADERS_INCLUDE = -I$(KERNEL_FULL_DIR)/include
KERNEL_ZIP = linux-2.6.34.x.gz

MKIMAGE_DIR = $(UTIL_DIR)/mkimage
MKSQUASHFS_DIR= $(UTIL_DIR)/squashfs4.0.x/squashfs-tools/mksquashfs

NCURSES_DIR = ncurses-5.5
ZLIB_DIR = zlib-1.2.3

BUSYBOX_DIR = busybox-1.1.3
BUSYBOX_FULL_DIR = $(TOP_DIR)/MIPS32_KNLAPPS/busybox-1.12.x
IPTABLES_DIR =	iptables-1.4.4
SSL_DIR = openssl-1.0.x
PCAP_DIR= libpcap-0.9.8
MADWIFI_DIR= LSDK-WLAN-7.x
PCRE_DIR=$(CROSSLIB_DIR)/pcre-7.x
SIPHOST_DIR      = /siphost
RVSIPTK_DIR      = $(SIPHOST_DIR)/rvSipTk
# 2013-02-05 Remove LIBUPNP_DIR define in config.make
# LIBUPNP_DIR	= $(CROSSLIB_DIR)/libupnp-1.6.0  
LIBUPNP13_DIR	= $(CROSSLIB_DIR)/libupnp-1.3.x
#
# hostapd include wifi driver directory. (Default)
# You can define your own directory at vendor/product_config.make
#
ATHEROS_APPS_DIR=$(TOP_DIR)/ATHEROS_APPS
HOSTAPD_WLAN_DRIVER_DIR=$(ATHEROS_APPS_DIR)/LSDK-WLAN-9.5.x-scorpion


#####################################
# include the toolchain inforamtion #
# Please do no change the order     #
#####################################
TOOLCHAIN_CONFIG_FILE=$(TOP_DIR)/configs/mips-tool-gcc434.make
include $(TOOLCHAIN_CONFIG_FILE)



BIN_FORMAT=elf32-big

#cfho 2006-0905
#set "-j2" if you have one CPU
#set "-j3" if you have dual(2) core CPUs
#set "-j5" if you have quard(4) core CPUs    
MAKE_FLAGS= -j9

CROSS = $(CROSS_TOOL)-
CC = $(CROSS)gcc
CXX = $(CROSS)g++
LD = $(CROSS)ld
AR = $(CROSS)ar
#RANLIB = $(CROSS)ranlib
STRIP= $(CROSS)strip


############################################
#  System is Big Endian? or little Endian?
#  If system is Big Endian
SYSTEM_IS_LITTLE_ENDIAN = no
############################################
HOSTCC=gcc
CCFLAGS = -Wall -pipe 
LDFLAGS = 
CCFLAGS_ENDIAN=
 

#EXTRA_CFLAGS=-D__NO_CTYPE  -DLinux -fomit-frame-pointer -pipe -D__linux__ -Dunix -DEMBED -march=mips32r2  $(CROSS_INCLUDE) -fPIC -funit-at-a-time -Os
 
EXTRA_CFLAGS=-D__NO_CTYPE  -DLinux -fomit-frame-pointer -pipe -D__linux__ -Dunix -DEMBED $(CROSS_INCLUDE) -fPIC -funit-at-a-time -Os
### Set the PLATFORM, Atheros AR240 ########
#EXTRA_CFLAGS+= -DFOR_ATHEROS_PLATFORM=1 -DFOR_AR7240=1 -DFOR_AR934x=1 -DFOR_AR955x=1
EXTRA_CFLAGS+= -DFOR_ATHEROS_PLATFORM=1 -DFOR_AR934x=1 -DFOR_AR955x=1

### We only run the system as root, no need to call the getuid, getgid, setuid,getgrgid,  ......
EXTRA_CFLAGS += -DRUN_SYSTEM_AS_ROOT=1

#######################################################################
######### Set it to 1 for little endian system, e.g. Atmel 902, RT2880
######### Set it to 0 for big endian system, e.g. IXP425
#ifdef SYSTEM_IS_LITTLE_ENDIAN
ifeq "$(SYSTEM_IS_LITTLE_ENDIAN)" "yes"
EXTRA_CFLAGS+=-DSYSTEM_IS_LITTLE_ENDIAN=1
EXTRA_CFLAGS+=-DSYSTEM_IS_BIG_ENDIAN=0
else
EXTRA_CFLAGS+=-DSYSTEM_IS_LITTLE_ENDIAN=0
EXTRA_CFLAGS+=-DSYSTEM_IS_BIG_ENDIAN=1
endif
EXTRA_CFLAGS+=-DUSE_UCLIB=1
#######################################################################

#use httpd to handle remotecontrol
#EXTRA_CFLAGS+=-DHTTPD_REMOTEPORT_FUNCTION=1

#######################################################################
#EXTRA_CFLAGS+=-DHAS_LOADBALANCE_FUNCTION=1
#######################################################################
#EXTRA_CFLAGS+=-O2

# APPS Build for target
EXTRA_CFLAGS+=-DTARGET=1

EXTRA_LDFLAGS=


CXXFLAGS= -Wall  -Wno-deprecated -pipe -fPIC -fexceptions
KCFLAGS= -D__KERNEL__ -I$(KERNEL_FULL_DIR)/include -I$(KERNEL_FULL_DIR)/include/asm-mips/mach-generic -O2 \
         -fno-strict-aliasing -fno-common -fomit-frame-pointer \
         -I$(KERNEL_FULL_DIR)/include/asm/gcc -G 0 -mno-abicalls \
         -fno-pic -pipe  -finline-limit=100000 -march=mips32r2 -mabi=32 \
         -Wa,--trap -DMODULE -mlong-calls

KMODFLAGS= $(INCLUDE) $(DEFINES) -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -f omit-frame-pointer -fno-strict-aliasing -fno-common  -msoft-float


CP = /bin/cp
MKDIR = /bin/mkdir
LN = /bin/ln
INSTALL =/usr/bin/install
MKDEPEND=$(TOP_DIR)/util/mkdep
LZMA=$(UTIL_DIR)/lzma
FILESIZECHK=$(UTIL_DIR)/filesizechk/filesizechk
FILESIZECHK_TMP=/tmp/filesizechk_tmp.txt

#
# product_config.make
#
PRODUCT_CONFIG_MAKE = $(TOP_DIR)/configs/product_config.make
ifneq ($(realpath $(wildcard $(PRODUCT_CONFIG_MAKE))),)
ifeq ($(wildcard $(PRODUCT_CONFIG_MAKE)),$(PRODUCT_CONFIG_MAKE))
include $(PRODUCT_CONFIG_MAKE)
endif
endif

# talor 2013/07/29, add new included file
#include $(TOP_DIR)/configs/config_extra_inc.make

#
# apcfg and sysutil lib : why define in config.make ?
# Because mips CPU have strange share memory issue.
#
# USE_APPS_SHARED_LIB -
# 1 : use libapcfg.so and libsysUtil.so
# 0 : use apcfg.a and sysUtil.a
#
export USE_APPS_SHARED_LIB=1

ifeq ($(USE_APPS_SHARED_LIB),1)
__SYSUTIL_LIBS = -L$(APPS_FULL_DIR)/sysutil -lsysUtil
else
__SYSUTIL_LIBS = $(APPS_FULL_DIR)/sysutil/sysUtil.a
endif

__SYSUTIL_STATIC_LIBS= $(APPS_FULL_DIR)/sysutil/sysUtil.a

# No using, maybe use CONFIG_USE_LIBRT replace
#ifeq ($(SYSUTIL_SHARED_LIB_USE_RT),1)
#__SYSUTIL_LIBS += -lrt
#__SYSUTIL_STATIC_LIBS += -lrt
#endif

ifeq ($(USE_APPS_SHARED_LIB),1)
__APCFG_LIBS = -L$(APPS_FULL_DIR)/ap_cfg -lapcfg -lpcre $(__SYSUTIL_LIBS)
else
__APCFG_LIBS = $(APPS_FULL_DIR)/ap_cfg/apcfg.a -lpcre $(__SYSUTIL_LIBS)
endif

ifeq ($(CONFIG_HAS_TR_AGENT),y)
__APCFG_LIBS += -lpthread
endif

ifeq ($(CONFIG_USE_LIBSSL),y)
__APCFG_LIBS += -lssl
endif

ifeq ($(CONFIG_USE_LIB_MATRIXSSL),y)
__APCFG_LIBS += -lmatrixssl
endif

export APCFG_LIBS=$(__APCFG_LIBS)
export SYSUTIL_LIBS=$(__SYSUTIL_LIBS)
export SYSUTIL_STATIC_LIBS=$(__SYSUTIL_STATIC_LIBS)
export APCFG_DEP_LIBS=$(APPS_FULL_DIR)/sysutil/sysUtil.a $(APPS_FULL_DIR)/ap_cfg/apcfg.a

