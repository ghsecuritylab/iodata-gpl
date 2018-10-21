# udhcp makefile
PRODUCT_CONFIG_MAKE = ../../configs/product_config.make
ifeq ($(wildcard $(PRODUCT_CONFIG_MAKE)),$(PRODUCT_CONFIG_MAKE))
include $(PRODUCT_CONFIG_MAKE)
endif
# USE busybox libbb/lib.a
UDHCP_WITHOUT_BUSYBOX = y

DESTDIR     = 
prefix      = /usr
SBINDIR     = /sbin
USRSBINDIR  = $(DESTDIR)${prefix}/sbin
USRBINDIR   = $(DESTDIR)${prefix}/bin
USRSHAREDIR = $(DESTDIR)${prefix}/share
USRETCDIR   = $(DESTDIR)${prefix}/etc

BUSYBOX_DIR = ../../MIPS32_KNLAPPS/busybox-1.7.x

CFLAGS = $(EXTRA_CFLAGS)
LDFLAGS = $(EXTRA_LDFLAGS)

CFLAGS += -D_GNU_SOURCE -DNDEBUG  -D"BB_VER=KBUILD_STR(1.7.5)" -DBB_BT=AUTOCONF_TIMESTAMP \
	-D_FORTIFY_SOURCE=2 -D"KBUILD_STR(s)=\#s"

ifeq ($(CONFIG_DHCPD_HAS_HOSTNAME),y)
CFLAGS +=-DHAS_DHCP_HOST_FUNCTION=1
endif

ifeq ($(CONFIG_DHCPD_HAS_CLIENT_IFNAME),y)
CFLAGS +=-DHAS_DHCPD_IF_INFOMATION_FUNCTION=1
endif

ifeq ($(CONFIG_HAS_TR_AGENT),y)
CFLAGS += -DHAS_TR_AGENT=1
endif

ifeq ($(CONFIG_HAS_NETBIOS_FUNCTION),y)
CFLAGS += -DHAS_NETBIOS_FUNCTION=1
endif

ifeq ($(CONFIG_DHCPD_HAS_USER_MANUAL_BOOTPFLAG),y)
CFLAGS += -DHAS_USER_MANUAL_BOOTPFLAG=1
endif

ifeq ($(CONFIG_DHCPD_REQUEST_INIC_CLIENTINFO),y)
CFLAGS += -DDHCPD_REQUEST_INIC_CLIENTINFO=1
endif

ifeq ($(CONFIG_HAS_ARP_PROXY),y)
CFLAGS +=-DHAS_ARP_PROXY=1
endif

# Uncomment this to get a shared binary. Call as udhcpd for the server,
# and udhcpc for the client
#COMBINED_BINARY=1

# Uncomment this for extra output and to compile with debugging symbols
#UDHCP_DEBUG=1

# Uncomment this to output messages to syslog, otherwise, messages go to stdout
#CFLAGS += -DUDHCP_SYSLOG

#CROSS_COMPILE=arm-uclibc-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
INSTALL = install

OBJS_SHARED = common.o options.o packet.o signalpipe.o socket.o
DHCPD_OBJS = dhcpd.o arpping.o files.o leases.o serverpacket.o static_leases.o
DHCPC_OBJS = dhcpc.o clientpacket.o clientsocket.o script.o 

ifeq ($(UDHCP_WITHOUT_BUSYBOX),y)
CFLAGS += -DUDHCP_WITHOUT_BUSYBOX
OBJS_SHARED +=  pidfile.o
else
LIBS += $(BUSYBOX_DIR)/libbb/lib.a
INCLUDE = -I$(BUSYBOX_DIR)/include -I$(BUSYBOX_DIR)/libbb \
        -I$(BUSYBOX_DIR)/ -include $(BUSYBOX_DIR)/include/autoconf.h
endif

ifdef COMBINED_BINARY
EXEC1 = udhcpd
OBJS1 = $(DHCPD_OBJS) $(DHCPC_OBJS) $(OBJS_SHARED) frontend.o
CFLAGS += -DCOMBINED_BINARY
else
EXEC1 = udhcpd
OBJS1 = $(DHCPD_OBJS) $(OBJS_SHARED)

EXEC2 = udhcpc
OBJS2 = $(DHCPC_OBJS) $(OBJS_SHARED)
endif

EXEC3 = dumpleases
OBJS3 = dumpleases.o

BOOT_PROGRAM = udhcpc
DAEMON = udhcpd
COMMAND = dumpleases

ifdef UDHCP_SYSLOG
CFLAGS += -DUDHCP_SYSLOG
endif

CFLAGS += -W -Wall -Wstrict-prototypes -D_GNU_SOURCE

ifdef UDHCP_DEBUG
CFLAGS += -g -DUDHCP_DEBUG
STRIP=true
else
CFLAGS += -Os -fomit-frame-pointer
STRIP=$(CROSS_COMPILE)strip
endif

#all: $(EXEC1) $(EXEC2) $(EXEC3)
#	$(STRIP) --remove-section=.note --remove-section=.comment $(EXEC1) $(EXEC2) $(EXEC3)
all: $(EXEC1) $(EXEC3)
	$(STRIP) --remove-section=.note --remove-section=.comment $(EXEC1) $(EXEC3)

$(OBJS1) $(OBJS2) $(OBJS3): *.h Makefile
$(EXEC1) $(EXEC2) $(EXEC3): Makefile

.c.o:
	@echo 'CC $@'
	@$(CC) -c $(CFLAGS) $(INCLUDE) $<
	
$(EXEC1): $(OBJS1)
	@echo 'CC $@'
	@$(LD) $(LDFLAGS) $(OBJS1) -o $(EXEC1) $(LIBS)

$(EXEC2): $(OBJS2)
	@echo 'CC $@'
	@$(LD) $(LDFLAGS) $(OBJS2) -o $(EXEC2) $(LIBS)

$(EXEC3): $(OBJS3)
	@echo 'CC $@'
	@$(LD) $(LDFLAGS) $(OBJS3) -o $(EXEC3)


install: all
	mkdir -p $(USRSBINDIR) $(USRBINDIR) 
	$(INSTALL) -m 755 $(DAEMON) $(USRSBINDIR)
	$(INSTALL) -m 755 $(COMMAND) $(USRBINDIR)
ifdef COMBINED_BINARY
	ln -sf $(DAEMON) $(USRSBINDIR)/$(BOOT_PROGRAM)
else
	$(INSTALL) -m 755 $(BOOT_PROGRAM) $(USRSBINDIR)
endif
	mkdir -p $(USRSHAREDIR)/udhcpc
	mkdir -p $(USRETCDIR)
	for name in bound deconfig nak renew script ; do \
		$(INSTALL) -m 755 samples/sample.$$name \
			$(USRSHAREDIR)/udhcpc/default.$$name ; \
	done

clean:
	-rm -f udhcpd udhcpc dumpleases *.o core
