#######################################################################
##
## Embedded Target Makefile (ISD2)
## Platform: Atheros AR7740 dual radio platform
## Author: Jimmy Ho
## The following definations are for the mips-linux-gnu3 v4.3.4 (from openwrt)
## BIG ENDIAN
#######################################################################

CROSS_TOOL = mips-linux-gnu3
CROSS_TOP = /opt/buildroot-be-gcc434
CROSS_DIR = $(CROSS_TOP)/usr
export CROSS_BIN_DIR = $(CROSS_DIR)/bin
export CROSS_LIB_DIR = $(CROSS_DIR)/lib
export CROSS_INCLUDE_DIR = $(CROSS_DIR)/include
export CROSS_INCLUDE = -funit-at-a-time -I$(CROSS_INCLUDE_DIR)
export PKG_CONFIG_PATH=$(CROSS_LIB_DIR)/pkgconfig
export PKG_CONFIG_LIBDIR=$(CROSS_LIB_DIR)
