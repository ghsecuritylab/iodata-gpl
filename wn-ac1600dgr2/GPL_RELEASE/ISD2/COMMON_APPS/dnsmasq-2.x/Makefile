PREFIX ?= /usr/local
BINDIR ?= ${PREFIX}/sbin
MANDIR ?= ${PREFIX}/share/man
LOCALEDIR ?= ${PREFIX}/share/locale
CC = $(CROSS)gcc
AR = $(CROSS)ar
LD = $(CROSS)ld
RANLIB= $(CROSS)ranlib

SRC = src
PO = po
MAN = man

CFLAGS?= -O2

CFLAGS+=$(EXTRA_CFLAGS)

all :   dnsmasq

dnsmasq :
	$(MAKE) I18N=-DNO_GETTEXT -f ../bld/Makefile -C $(SRC) dnsmasq \
	CROSS=$(CROSS) CC=$(CC)  CFLAGS="$(CFLAGS)"

clean :
	rm -f *~ $(SRC)/*.mo contrib/*/*~ */*~ $(SRC)/*.pot 
	rm -f $(SRC)/*.o $(SRC)/dnsmasq.a $(SRC)/dnsmasq core */core

install : all install-common

install-common :
	install -m 755 $(SRC)/dnsmasq $(DESTDIR)$(BINDIR)

all-i18n :
	$(MAKE) I18N=-DLOCALEDIR='\"$(LOCALEDIR)\"' -f ../bld/Makefile -C $(SRC) dnsmasq
	cd $(PO); for f in *.po; do \
		$(MAKE) -f ../bld/Makefile -C ../$(SRC) $${f%.po}.mo; \
	done

install-i18n : all-i18n install-common
	cd $(SRC); ../bld/install-mo $(DESTDIR)$(LOCALEDIR)
	cd $(MAN); ../bld/install-man $(DESTDIR)$(MANDIR)

merge :
	$(MAKE) I18N=-DLOCALEDIR='\"$(LOCALEDIR)\"' -f ../bld/Makefile -C $(SRC) dnsmasq.pot
	cd $(PO); for f in *.po; do \
		msgmerge -U $$f ../$(SRC)/dnsmasq.pot; \
	done


