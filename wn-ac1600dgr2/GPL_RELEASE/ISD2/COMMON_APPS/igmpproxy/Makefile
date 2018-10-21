CC=$(CROSS)gcc
CFLAGS=$(EXTRA_CFLAGS)
EXECS = src/igmpproxy
OBJS = igmpproxy.o config.o confread.o request.o udpsock.o mcgroup.o rttable.o \
			igmp.o ifvc.o callout.o kern.o syslog.o lib.o mroute-api.o
#OBJS = $(addprefix ./src/, $(OBJS__))
MYINCLUDES = -I../src
LDLIBS=


all: clean   $(EXECS)  

%.o: %.c
	@echo 'CC $@'
	@$(CC) $(CFLAGS) $(MYINCLUDES) -c -o $@ $<

$(EXECS): $(OBJS)
	@echo 'CC $@'
	@$(CC) $(LDFLAGS) -o $@ $@.o $(LDLIBS)


clean:
	rm -f $(EXECS) *.elf *.gdb *.o src/*.o

install: 
	mkdir -p $(PREFIX)/sbin; \
	install testtime $(PREFIX)/sbin
