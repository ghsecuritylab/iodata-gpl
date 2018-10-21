PREFIX=/usr
LIBUPNP_PREFIX=/usr
#LIBUPNP_DIR=/usr


CC = $(CROSS)gcc
AR = $(CROSS)ar
LD = $(CROSS)ld
RANLIB= $(CROSS)ranlib

#INCLUDES= -I$(LIBUPNP_DIR)/upnp/inc -I../include
#LIBS= -lpthread -lupnp -lixml -lthreadutil -L$(LIBUPNP_PREFIX)/lib -L../libs
#LIBUPNP_DIR=/ISD/crosslibs/libupnp-1.x
LIBS= $(CROSS_LIB_DIR)/libupnp.a \
	$(CROSS_LIB_DIR)/libixml.a \
	$(CROSS_LIB_DIR)/libthreadutil.a \
		-lpthread 
#\  	    	-L$(LIBUPNP_DIR)/lib -L../libs

FILES= main.o gatedevice.o pmlist.o util.o config.o mygettimeofday.o

ifeq ($(CONFIG_HAS_UPNP_API), y)
FILES += X_MBridgeControl.o
endif

ifeq ($(CONFIG_HAS_MBRIDGE_CONTROL), y)
FILES += MBridgeControl.o
endif

CFLAGS += -Wall -O2

ifdef HAVE_LIBIPTC
ifdef LIBUPNP_DIR
LIBS += -L$(LIBUPNP_DIR)/lib
INCLUDES += -I$(LIBUPNP_DIR)/include
endif

LIBS += -liptc
INCLUDES += -DHAVE_LIBIPTC
FILES += iptc.o
endif


ifeq ($(CONFIG_HAS_MBRIDGE_CONTROL),y)
ETC_VENDOR_DIR=./etc_Denon_Marantz
INCLUDES += -I../../configs/h -I../../MIPS32_APPS/h -I../../MIPS32_APPS/ap_cfg/include
endif

all: upnpd

upnpd: $(FILES)
	$(CC) $(CFLAGS) $(FILES) $(LIBS) -o $@
	@echo "make $@ finished on `date`"

%.o:	%.c
	@echo 'CC $@'
	@$(CC) $(CFLAGS) $(INCLUDES) -c $<

clean:
	rm -f *.o upnpd

install: upnpd
	install -d /etc/linuxigd
	install etc/gatedesc.xml /etc/linuxigd
	install etc/gateconnSCPD.xml  /etc/linuxigd
	install etc/gateicfgSCPD.xml /etc/linuxigd
	install etc/dummy.xml /etc/linuxigd
	install etc/VuzeOfflineDownloaderService.xml /etc/linuxigd
	install etc/X_MBridgeControl.xml /etc/linuxigd
	install upnpd $(PREFIX)/sbin
	install upnpd.8 $(PREFIX)/share/man/man8
	if [ ! -f /etc/upnpd.conf ]; then install etc/upnpd.conf /etc; fi
ifneq ($(ETC_VENDOR_DIR),)
	install -m755 $(ETC_VENDOR_DIR)/*.xml $(TARGET_APP_DIR)/etc/linuxigd;
	install -m755 $(ETC_VENDOR_DIR)/*.sh $(TARGET_APP_DIR)/sbin/;
endif
