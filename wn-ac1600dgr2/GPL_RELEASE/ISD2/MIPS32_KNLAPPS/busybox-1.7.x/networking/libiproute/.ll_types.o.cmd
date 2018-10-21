cmd_networking/libiproute/ll_types.o := mips-linux-gnu3-gcc -Wp,-MD,networking/libiproute/.ll_types.o.d   -D__NO_CTYPE  -DLinux -fomit-frame-pointer -pipe -D__linux__ -Dunix -DEMBED -funit-at-a-time -I/opt/buildroot-be-gcc434/usr/include -fPIC -funit-at-a-time -Os -DFOR_ATHEROS_PLATFORM=1 -DFOR_AR934x=1 -DFOR_AR955x=1 -DRUN_SYSTEM_AS_ROOT=1 -DSYSTEM_IS_LITTLE_ENDIAN=0 -DSYSTEM_IS_BIG_ENDIAN=1 -DUSE_UCLIB=1 -DTARGET=1 -I/ISD2/configs/h	 -std=gnu99 -Iinclude -Ilibbb -I-I/ISD2/ATHEROS_LSDK_KNL/linux-2.6.31/include  -I/ISD2/MIPS32_KNLAPPS/busybox-1.7.x/libbb -include include/autoconf.h -D_GNU_SOURCE -DNDEBUG  -D"BB_VER=KBUILD_STR(1.7.5)" -DBB_BT=AUTOCONF_TIMESTAMP -D_FORTIFY_SOURCE=2  -D__NO_CTYPE  -DLinux -fomit-frame-pointer -pipe -D__linux__ -Dunix -DEMBED -funit-at-a-time -I/opt/buildroot-be-gcc434/usr/include -fPIC -funit-at-a-time -Os -DFOR_ATHEROS_PLATFORM=1 -DFOR_AR934x=1 -DFOR_AR955x=1 -DRUN_SYSTEM_AS_ROOT=1 -DSYSTEM_IS_LITTLE_ENDIAN=0 -DSYSTEM_IS_BIG_ENDIAN=1 -DUSE_UCLIB=1 -DTARGET=1 -I/ISD2/configs/h -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations -Os -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Wdeclaration-after-statement -Wno-pointer-sign -D__NO_CTYPE  -DLinux -fomit-frame-pointer -pipe -D__linux__ -Dunix -DEMBED -funit-at-a-time -I/opt/buildroot-be-gcc434/usr/include -fPIC -funit-at-a-time -Os -DFOR_ATHEROS_PLATFORM=1 -DFOR_AR934x=1 -DFOR_AR955x=1 -DRUN_SYSTEM_AS_ROOT=1 -DSYSTEM_IS_LITTLE_ENDIAN=0 -DSYSTEM_IS_BIG_ENDIAN=1 -DUSE_UCLIB=1 -DTARGET=1 -I/ISD2/configs/h    -D__NO_CTYPE  -DLinux -fomit-frame-pointer -pipe -D__linux__ -Dunix -DEMBED -funit-at-a-time -I/opt/buildroot-be-gcc434/usr/include -fPIC -funit-at-a-time -Os -DFOR_ATHEROS_PLATFORM=1 -DFOR_AR934x=1 -DFOR_AR955x=1 -DRUN_SYSTEM_AS_ROOT=1 -DSYSTEM_IS_LITTLE_ENDIAN=0 -DSYSTEM_IS_BIG_ENDIAN=1 -DUSE_UCLIB=1 -DTARGET=1 -I/ISD2/configs/h -Wall -Wshadow -Wwrite-strings -Wundef -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations -Os -fno-builtin-strlen -finline-limit=0 -fomit-frame-pointer -ffunction-sections -fdata-sections -fno-guess-branch-probability -funsigned-char -static-libgcc -falign-functions=1 -falign-jumps=1 -falign-labels=1 -falign-loops=1 -Wdeclaration-after-statement -Wno-pointer-sign -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(ll_types)"  -D"KBUILD_MODNAME=KBUILD_STR(ll_types)" -c -o networking/libiproute/ll_types.o networking/libiproute/ll_types.c

deps_networking/libiproute/ll_types.o := \
  networking/libiproute/ll_types.c \
  /opt/buildroot-be-gcc434/usr/include/arpa/inet.h \
    $(wildcard include/config/c.h) \
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
  /opt/buildroot-be-gcc434/usr/include/netinet/in.h \
  /opt/buildroot-be-gcc434/usr/include/stdint.h \
  /opt/buildroot-be-gcc434/usr/include/bits/wchar.h \
  /opt/buildroot-be-gcc434/usr/include/bits/wordsize.h \
  /opt/buildroot-be-gcc434/usr/include/sys/socket.h \
  /opt/buildroot-be-gcc434/usr/include/sys/uio.h \
  /opt/buildroot-be-gcc434/usr/include/sys/types.h \
  /opt/buildroot-be-gcc434/usr/include/bits/types.h \
  /opt/toolchains/buildroot-be-gcc434-2013-0107/usr/bin/../lib/gcc/mips-unknown-linux-uclibc/4.3.4/include/stddef.h \
  /opt/buildroot-be-gcc434/usr/include/bits/typesizes.h \
  /opt/buildroot-be-gcc434/usr/include/bits/pthreadtypes.h \
  /opt/buildroot-be-gcc434/usr/include/bits/sched.h \
  /opt/buildroot-be-gcc434/usr/include/time.h \
  /opt/buildroot-be-gcc434/usr/include/endian.h \
  /opt/buildroot-be-gcc434/usr/include/bits/endian.h \
  /opt/buildroot-be-gcc434/usr/include/sys/select.h \
  /opt/buildroot-be-gcc434/usr/include/bits/select.h \
  /opt/buildroot-be-gcc434/usr/include/bits/sigset.h \
  /opt/buildroot-be-gcc434/usr/include/bits/time.h \
  /opt/buildroot-be-gcc434/usr/include/sys/sysmacros.h \
  /opt/buildroot-be-gcc434/usr/include/bits/uio.h \
  /opt/buildroot-be-gcc434/usr/include/bits/socket.h \
  /opt/toolchains/buildroot-be-gcc434-2013-0107/usr/bin/../lib/gcc/mips-unknown-linux-uclibc/4.3.4/include-fixed/limits.h \
  /opt/toolchains/buildroot-be-gcc434-2013-0107/usr/bin/../lib/gcc/mips-unknown-linux-uclibc/4.3.4/include-fixed/syslimits.h \
  /opt/buildroot-be-gcc434/usr/include/limits.h \
  /opt/buildroot-be-gcc434/usr/include/bits/posix1_lim.h \
  /opt/buildroot-be-gcc434/usr/include/bits/local_lim.h \
  /opt/buildroot-be-gcc434/usr/include/linux/limits.h \
  /opt/buildroot-be-gcc434/usr/include/bits/uClibc_local_lim.h \
  /opt/buildroot-be-gcc434/usr/include/bits/posix2_lim.h \
  /opt/buildroot-be-gcc434/usr/include/bits/xopen_lim.h \
  /opt/buildroot-be-gcc434/usr/include/bits/stdio_lim.h \
  /opt/buildroot-be-gcc434/usr/include/bits/sockaddr.h \
  /opt/buildroot-be-gcc434/usr/include/asm/socket.h \
  /opt/buildroot-be-gcc434/usr/include/asm/sockios.h \
  /opt/buildroot-be-gcc434/usr/include/asm/ioctl.h \
  /opt/buildroot-be-gcc434/usr/include/asm-generic/ioctl.h \
  /opt/buildroot-be-gcc434/usr/include/bits/in.h \
  /opt/buildroot-be-gcc434/usr/include/bits/byteswap.h \
  /opt/buildroot-be-gcc434/usr/include/bits/byteswap-common.h \
  /opt/buildroot-be-gcc434/usr/include/linux/if_arp.h \
  /opt/buildroot-be-gcc434/usr/include/linux/netdevice.h \
  /opt/buildroot-be-gcc434/usr/include/linux/if.h \
  /opt/buildroot-be-gcc434/usr/include/linux/types.h \
  /opt/buildroot-be-gcc434/usr/include/asm/types.h \
  /opt/buildroot-be-gcc434/usr/include/asm-generic/int-ll64.h \
  /opt/buildroot-be-gcc434/usr/include/asm/bitsperlong.h \
  /opt/buildroot-be-gcc434/usr/include/asm-generic/bitsperlong.h \
    $(wildcard include/config/64bit.h) \
  /opt/buildroot-be-gcc434/usr/include/linux/posix_types.h \
  /opt/buildroot-be-gcc434/usr/include/linux/stddef.h \
  /opt/buildroot-be-gcc434/usr/include/asm/posix_types.h \
  /opt/buildroot-be-gcc434/usr/include/asm/sgidefs.h \
  /opt/buildroot-be-gcc434/usr/include/linux/socket.h \
  /opt/buildroot-be-gcc434/usr/include/linux/hdlc/ioctl.h \
  /opt/buildroot-be-gcc434/usr/include/linux/if_ether.h \
  /opt/buildroot-be-gcc434/usr/include/linux/if_packet.h \
  include/libbb.h \
    $(wildcard include/config/selinux.h) \
    $(wildcard include/config/locale/support.h) \
    $(wildcard include/config/feature/shadowpasswds.h) \
    $(wildcard include/config/lfs.h) \
    $(wildcard include/config/feature/buffers/go/on/stack.h) \
    $(wildcard include/config/buffer.h) \
    $(wildcard include/config/ubuffer.h) \
    $(wildcard include/config/feature/buffers/go/in/bss.h) \
    $(wildcard include/config/feature/ipv6.h) \
    $(wildcard include/config/ture/ipv6.h) \
    $(wildcard include/config/feature/prefer/applets.h) \
    $(wildcard include/config/busybox/exec/path.h) \
    $(wildcard include/config/getopt/long.h) \
    $(wildcard include/config/feature/pidfile.h) \
    $(wildcard include/config/feature/syslog.h) \
    $(wildcard include/config/route.h) \
    $(wildcard include/config/gunzip.h) \
    $(wildcard include/config/ktop.h) \
    $(wildcard include/config/ioctl/hex2str/error.h) \
    $(wildcard include/config/feature/editing.h) \
    $(wildcard include/config/feature/editing/history.h) \
    $(wildcard include/config/ture/editing/savehistory.h) \
    $(wildcard include/config/feature/editing/savehistory.h) \
    $(wildcard include/config/feature/tab/completion.h) \
    $(wildcard include/config/feature/username/completion.h) \
    $(wildcard include/config/feature/editing/vi.h) \
    $(wildcard include/config/inux.h) \
    $(wildcard include/config/feature/devfs.h) \
  include/platform.h \
    $(wildcard include/config/werror.h) \
    $(wildcard include/config//nommu.h) \
    $(wildcard include/config//mmu.h) \
  /opt/buildroot-be-gcc434/usr/include/byteswap.h \
  /opt/toolchains/buildroot-be-gcc434-2013-0107/usr/bin/../lib/gcc/mips-unknown-linux-uclibc/4.3.4/include/stdbool.h \
  /opt/buildroot-be-gcc434/usr/include/sys/mount.h \
  /opt/buildroot-be-gcc434/usr/include/sys/ioctl.h \
  /opt/buildroot-be-gcc434/usr/include/bits/ioctls.h \
  /opt/buildroot-be-gcc434/usr/include/asm/ioctls.h \
  /opt/buildroot-be-gcc434/usr/include/bits/ioctl-types.h \
  /opt/buildroot-be-gcc434/usr/include/sys/ttydefaults.h \
  /opt/buildroot-be-gcc434/usr/include/ctype.h \
  /opt/buildroot-be-gcc434/usr/include/bits/uClibc_touplow.h \
  /opt/buildroot-be-gcc434/usr/include/dirent.h \
  /opt/buildroot-be-gcc434/usr/include/bits/dirent.h \
  /opt/buildroot-be-gcc434/usr/include/errno.h \
  /opt/buildroot-be-gcc434/usr/include/bits/errno.h \
  /opt/buildroot-be-gcc434/usr/include/linux/errno.h \
  /opt/buildroot-be-gcc434/usr/include/asm/errno.h \
  /opt/buildroot-be-gcc434/usr/include/asm-generic/errno-base.h \
  /opt/buildroot-be-gcc434/usr/include/sys/syscall.h \
  /opt/buildroot-be-gcc434/usr/include/bits/sysnum.h \
  /opt/buildroot-be-gcc434/usr/include/fcntl.h \
  /opt/buildroot-be-gcc434/usr/include/bits/fcntl.h \
  /opt/buildroot-be-gcc434/usr/include/sgidefs.h \
  /opt/buildroot-be-gcc434/usr/include/sys/stat.h \
  /opt/buildroot-be-gcc434/usr/include/bits/stat.h \
  /opt/buildroot-be-gcc434/usr/include/inttypes.h \
  /opt/buildroot-be-gcc434/usr/include/mntent.h \
  /opt/buildroot-be-gcc434/usr/include/stdio.h \
  /opt/buildroot-be-gcc434/usr/include/paths.h \
  /opt/buildroot-be-gcc434/usr/include/netdb.h \
    $(wildcard include/config/3/ascii/rules.h) \
  /opt/buildroot-be-gcc434/usr/include/rpc/netdb.h \
  /opt/buildroot-be-gcc434/usr/include/bits/siginfo.h \
  /opt/buildroot-be-gcc434/usr/include/bits/netdb.h \
  /opt/buildroot-be-gcc434/usr/include/setjmp.h \
  /opt/buildroot-be-gcc434/usr/include/bits/setjmp.h \
  /opt/buildroot-be-gcc434/usr/include/signal.h \
  /opt/buildroot-be-gcc434/usr/include/bits/signum.h \
  /opt/buildroot-be-gcc434/usr/include/bits/sigaction.h \
  /opt/buildroot-be-gcc434/usr/include/bits/sigcontext.h \
  /opt/buildroot-be-gcc434/usr/include/bits/sigstack.h \
  /opt/buildroot-be-gcc434/usr/include/ucontext.h \
  /opt/buildroot-be-gcc434/usr/include/sys/ucontext.h \
  /opt/buildroot-be-gcc434/usr/include/bits/sigthread.h \
  /opt/buildroot-be-gcc434/usr/include/bits/uClibc_stdio.h \
  /opt/buildroot-be-gcc434/usr/include/wchar.h \
  /opt/buildroot-be-gcc434/usr/include/bits/uClibc_mutex.h \
  /opt/buildroot-be-gcc434/usr/include/pthread.h \
  /opt/buildroot-be-gcc434/usr/include/sched.h \
  /opt/buildroot-be-gcc434/usr/include/bits/uClibc_clk_tck.h \
  /opt/buildroot-be-gcc434/usr/include/bits/initspin.h \
  /opt/buildroot-be-gcc434/usr/include/bits/uClibc_pthread.h \
  /opt/toolchains/buildroot-be-gcc434-2013-0107/usr/bin/../lib/gcc/mips-unknown-linux-uclibc/4.3.4/include/stdarg.h \
  /opt/buildroot-be-gcc434/usr/include/stdlib.h \
  /opt/buildroot-be-gcc434/usr/include/bits/waitflags.h \
  /opt/buildroot-be-gcc434/usr/include/bits/waitstatus.h \
  /opt/buildroot-be-gcc434/usr/include/alloca.h \
  /opt/buildroot-be-gcc434/usr/include/string.h \
  /opt/buildroot-be-gcc434/usr/include/sys/mman.h \
  /opt/buildroot-be-gcc434/usr/include/bits/mman.h \
  /opt/buildroot-be-gcc434/usr/include/sys/statfs.h \
  /opt/buildroot-be-gcc434/usr/include/bits/statfs.h \
  /opt/buildroot-be-gcc434/usr/include/sys/time.h \
  /opt/buildroot-be-gcc434/usr/include/sys/wait.h \
  /opt/buildroot-be-gcc434/usr/include/sys/resource.h \
  /opt/buildroot-be-gcc434/usr/include/bits/resource.h \
  /opt/buildroot-be-gcc434/usr/include/termios.h \
  /opt/buildroot-be-gcc434/usr/include/bits/termios.h \
  /opt/buildroot-be-gcc434/usr/include/unistd.h \
  /opt/buildroot-be-gcc434/usr/include/bits/posix_opt.h \
  /opt/buildroot-be-gcc434/usr/include/bits/environments.h \
  /opt/buildroot-be-gcc434/usr/include/bits/confname.h \
  /opt/buildroot-be-gcc434/usr/include/bits/getopt.h \
  /opt/buildroot-be-gcc434/usr/include/utime.h \
  include/pwd_.h \
    $(wildcard include/config/use/bb/pwd/grp.h) \
  /opt/buildroot-be-gcc434/usr/include/pwd.h \
  include/grp_.h \
  /opt/buildroot-be-gcc434/usr/include/grp.h \
  /opt/buildroot-be-gcc434/usr/include/sys/param.h \
  /opt/buildroot-be-gcc434/usr/include/linux/param.h \
  /opt/buildroot-be-gcc434/usr/include/asm/param.h \
  include/xatonum.h \
  networking/libiproute/rt_names.h \

networking/libiproute/ll_types.o: $(deps_networking/libiproute/ll_types.o)

$(deps_networking/libiproute/ll_types.o):
