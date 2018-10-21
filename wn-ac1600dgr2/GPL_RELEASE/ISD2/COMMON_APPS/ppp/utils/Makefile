CC = $(CROSS)gcc
AR = $(CROSS)ar
LD = $(CROSS)ld
RANLIB= $(CROSS)ranlib
STRIP = $(CROSS)strip

CFLAGS = -Wall -I$(LIBPCAP_DIR)
LDFLAGS = -L/home/mook/Download/libpcap-0.9.8 -lpcap

EXEC = pfc

all: $(EXEC)

pfc: pfc.o
	@echo 'CC $@'
	$(CC) -static -o $@ $< $(LDFLAGS)

%.o: %.c
	@echo 'CC $@'
	@$(CC) $(CFLAGS) -c -o $@ $< 

clean:
	rm -f *.o $(EXEC)

install:
