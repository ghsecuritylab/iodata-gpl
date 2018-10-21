.PHONY: all clean depend

CC = $(CROSS)gcc
AR = $(CROSS)ar
LD = $(CROSS)ld
RANLIB= $(CROSS)ranlib
CFLAGS = -Wall -pipe $(EXTRA_CFLAGS)
LDFLAGS=$(EXTRA_LDFLAGS)


LIBS =


INCLUDES =  -I./include -I$(KERNEL_FULL_DIR)/include

all: depend brctl

brctl: $(OBJS) brctl.o 
	@echo 'CC $@'
	@$(CC) $(CFLAGS) $(INCLUDES) -o $@ $? $(LIBS); \
	cp brctl /tftpboot

%.o: %.c
	@echo 'CC $@'
	@$(CC) $(CFLAGS) -Wall $(INCLUDES) -c -o $@ $< 

clean:
	rm -f apps_init *.elf *.gdb *.o *~ *.a .depend;

depend: .depend

.depend:
	CC=$(CC) ../../util/mkdep $(CFLAGS) $(INCLUDES) `ls *.c`

ifneq ($(wildcard .depend),)
include .depend
endif

install: 
	mkdir -p $(PREFIX)/sbin; \
	install brctl $(PREFIX)/sbin; 
	
	
