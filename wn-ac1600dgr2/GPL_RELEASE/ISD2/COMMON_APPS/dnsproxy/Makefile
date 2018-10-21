CC = $(CROSS)gcc
AR = $(CROSS)ar
LD = $(CROSS)ld
RANLIB = $(CROSS)ranlib
CFLAGS = -Wall -pipe $(EXTRA_CFLAGS)
LDFLAGS= $(EXTRA_LDFLAGS)

OBJS = dnsproxy.o 


INCLUDES = -I./include

all: depend dnsproxy 


%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

dnsproxy: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $@.o $(LDLIBS)
	cp $@ /tftpboot
	ls -al /tftpboot/$@

clean:
	rm -rf dnsproxy *.elf *.gdb *.o *~ *.a .depend;


depend: .depend

.depend:
	CC=$(CC) ../../util/mkdep $(CFLAGS) $(INCLUDES) `ls *.c`

ifneq ($(wildcard .depend),)
include .depend
endif

install: 
	mkdir -p $(PREFIX)/sbin; \
	mkdir -p $(PREFIX)/usr/bin; \
	install dnsproxy $(PREFIX)/sbin; 
	
	
