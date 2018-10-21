CC = $(CROSS)gcc
AR = $(CROSS)ar
LD = $(CROSS)ld
RANLIB= $(CROSS)ranlib
CFLAGS = -Wall -pipe $(EXTRA_CFLAGS)
LDFLAGS=$(EXTRA_LDFLAGS)

#OBJS =  gen_uuid.o pack.o unpack.o
UUID_OBJS = gen_uuid.o pack.o unpack.o

LIBS=  


INCLUDES = -I./

all: clean uuidgen

uuidgen: $(UUID_OBJS)
	@echo 'CC $@'
	@$(CC) $(CFLAGS) $(INCLUDES) -o $@ $? $(LIBS); \
	cp uuidgen /tftpboot

%.o: %.c
	@echo 'CC $@'
	@$(CC) $(CFLAGS)  $(INCLUDES) -c -o $@ $< 

clean:
	rm -f $(EXECS) *.elf *.gdb *.o *~ *.a uuidgen

install: 
	mkdir -p $(PREFIX)/sbin; \
	mkdir -p $(PREFIX)/usr/bin; \
	install uuidgen $(PREFIX)/sbin; 
	
	
