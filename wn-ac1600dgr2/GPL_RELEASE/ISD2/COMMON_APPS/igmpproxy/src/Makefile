CC=$(CROSS)gcc
#BINDIR=../bin
BINDIR=/usr/local/bin
ETCDIR=/etc
MANDIR=/usr/share/man

EXEC=igmpproxy
IGMP_PROXY_OBJ= igmpproxy.o config.o confread.o request.o udpsock.o mcgroup.o rttable.o \
                igmp.o ifvc.o callout.o kern.o syslog.o lib.o mroute-api.o

# CFLAGS=-g
#CFLAGS=-O
CFLAGS=$(EXTRA_CFLAGS)

#default : build.h igmpproxy

all: clean  $(EXECS)  

clean:
	rm -rf *.o *.asm build.h igmpproxy  

install: 
	cp igmpproxy ${BINDIR}
	cp ../doc/igmpproxy.8 ${MANDIR}/man8
	cp ../doc/igmpproxy.conf.5 ${MANDIR}/man5
	if [ ! -e ${ETCDIR}/igmpproxy.conf ]; then cp igmpproxy.conf ${ETCDIR}; fi

 
%.o: %.c
	@echo 'CC $@'
	@$(CC) $(CFLAGS) -c -o $@ $<


all: $(EXEC)

igmpproxy: build.h $(IGMP_PROXY_OBJ)
	@echo 'CC $@'
	@$(CC) $(LDFLAGS) -o $@ $(IGMP_PROXY_OBJ)


build.h	:  
	echo '#define BUILD "' `date +%y%m%d` '"' >build.h


