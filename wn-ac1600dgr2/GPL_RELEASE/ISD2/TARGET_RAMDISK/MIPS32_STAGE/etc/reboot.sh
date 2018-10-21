#!/bin/sh
echo 8 > /proc/sys/kernel/printk
sync
sync
sysconf_cli reboot
#/bin/umount -a -r
sleep 2
/sbin/reboot



