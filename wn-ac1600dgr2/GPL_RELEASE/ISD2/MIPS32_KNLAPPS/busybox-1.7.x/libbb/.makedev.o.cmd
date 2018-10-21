cmd_libbb/makedev.o := mips-linux-gnu3-gcc -Wp,-MD,libbb/.makedev.o.d   -D__NO_CTYPE  -DLinux -fomit-frame-pointer -pipe -D__linux__ -Dunix -DEMBED -funit-at-a-time -I/opt/buildroot-be-gcc434/usr/include -fPIC -funit-at-a-time -Os -DFOR_ATHEROS_PLATFORM=1 -DFOR_AR934x=1 -DFOR_AR955x=1 -DRUN_SYSTEM_AS_ROOT=1 -DSYSTEM_IS_LITTLE_ENDIAN=0 -DSYSTEM_IS_BIG_ENDIAN=1 -DUSE_UCLIB=1 -DTARGET=1 -I/ISD2/configs/h	 -std=gnu99 -Iinclude -Ilibbb -I-I/ISD2/ATHEROS_LSDK_KNL/linux-2.6.31/include  -I/ISD2/MIPS32_KNLAPPS/busybox-1.7.x/libbb -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -D"BB_VER=KBUILD_STR(1.7.5)" -DBB_BT=AUTOCONF_TIMESTAMP -D_FORTIFY_SOURCE=2  -D__NO_CTYPE  -DLinux -fomit-frame-pointer -pipe -D__linux__ -Dunix -DEMBED -funit-at-a-time -I/opt/buildroot-be-gcc434/usr/include -fPIC -funit-at-a-time -Os -DFOR_ATHEROS_PLATFORM=1 -DFOR_AR934x=1 -DFOR_AR955x=1 -DRUN_SYSTEM_AS_ROOT=1 -DSYSTEM_IS_LITTLE_ENDIAN=0 -DSYSTEM_IS_BIG_ENDIAN=1 -DUSE_UCLIB=1 -DTARGET=1 -I/ISD2/configs/h -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations -Os -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Wdeclaration-after-statement -Wno-pointer-sign -D__NO_CTYPE  -DLinux -fomit-frame-pointer -pipe -D__linux__ -Dunix -DEMBED -funit-at-a-time -I/opt/buildroot-be-gcc434/usr/include -fPIC -funit-at-a-time -Os -DFOR_ATHEROS_PLATFORM=1 -DFOR_AR934x=1 -DFOR_AR955x=1 -DRUN_SYSTEM_AS_ROOT=1 -DSYSTEM_IS_LITTLE_ENDIAN=0 -DSYSTEM_IS_BIG_ENDIAN=1 -DUSE_UCLIB=1 -DTARGET=1 -I/ISD2/configs/h    -D__NO_CTYPE  -DLinux -fomit-frame-pointer -pipe -D__linux__ -Dunix -DEMBED -funit-at-a-time -I/opt/buildroot-be-gcc434/usr/include -fPIC -funit-at-a-time -Os -DFOR_ATHEROS_PLATFORM=1 -DFOR_AR934x=1 -DFOR_AR955x=1 -DRUN_SYSTEM_AS_ROOT=1 -DSYSTEM_IS_LITTLE_ENDIAN=0 -DSYSTEM_IS_BIG_ENDIAN=1 -DUSE_UCLIB=1 -DTARGET=1 -I/ISD2/configs/h -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations -Os -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Wdeclaration-after-statement -Wno-pointer-sign -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(makedev)"  -D"KBUILD_MODNAME=KBUILD_STR(makedev)" -c -o libbb/makedev.o libbb/makedev.c

deps_libbb/makedev.o := \
  libbb/makedev.c \
  /opt/buildroot-be-gcc434/usr/include/features.h \
    $(wildcard include/config/c99.h) \
    $(wildcard include/config/c95.h) \
    $(wildcard include/config/ix.h) \
    $(wildcard include/config/ix2.h) \
    $(wildcard include/config/ix199309.h) \
    $(wildcard include/config/ix199506.h) \
    $(wildcard include/config/en.h) \
    $(wildcard include/config/en/extended.h) \
    $(wildcard include/config/x98.h) \
    $(wildcard include/config/en2k.h) \
    $(wildcard include/config/en2k8.h) \
    $(wildcard include/config/gefile.h) \
    $(wildcard include/config/gefile64.h) \
    $(wildcard include/config/e/offset64.h) \
    $(wildcard include/config/.h) \
    $(wildcard include/config/d.h) \
    $(wildcard include/config/c.h) \
    $(wildcard include/config/ile.h) \
    $(wildcard include/config/ntrant.h) \
    $(wildcard include/config/tify/level.h) \
    $(wildcard include/config/i.h) \
    $(wildcard include/config/ern/inlines.h) \
    $(wildcard include/config/ix/implicitly.h) \
  /opt/buildroot-be-gcc434/usr/include/bits/uClibc_config.h \
    $(wildcard include/config/mips/o32/abi//.h) \
    $(wildcard include/config/mips/n32/abi//.h) \
    $(wildcard include/config/mips/n64/abi//.h) \
    $(wildcard include/config/mips/isa/1//.h) \
    $(wildcard include/config/mips/isa/2//.h) \
    $(wildcard include/config/mips/isa/3//.h) \
    $(wildcard include/config/mips/isa/4//.h) \
    $(wildcard include/config/mips/isa/mips32//.h) \
    $(wildcard include/config/mips/isa/mips32r2//.h) \
    $(wildcard include/config/mips/isa/mips64//.h) \
    $(wildcard include/config///.h) \
    $(wildcard include/config//.h) \
    $(wildcard include/config/link//.h) \
  /opt/buildroot-be-gcc434/usr/include/sys/cdefs.h \
    $(wildcard include/config/espaces.h) \
  /opt/buildroot-be-gcc434/usr/include/sys/sysmacros.h \

libbb/makedev.o: $(deps_libbb/makedev.o)

$(deps_libbb/makedev.o):
