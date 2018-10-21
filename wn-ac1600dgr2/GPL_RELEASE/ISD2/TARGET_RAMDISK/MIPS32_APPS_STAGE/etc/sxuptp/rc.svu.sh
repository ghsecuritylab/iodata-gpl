#!/bin/sh

# network interface name net.USB will be activated on
IFNAME=br0

# maximum number of USB interface connections
MAXCONN=15

KMOD_DIR=/lib/modules/sxuptp
DEV_ALLOW=/proc/sxuptp/devices.allow
DEV_DENY=/proc/sxuptp/devices.deny

INSMOD=/sbin/insmod
RMMOD=/sbin/rmmod
JCPD=/usr/sbin/jcpd
SXDEVD=/usr/sbin/sxdevd

install_device_filter() {
	# Please comment out/uncomment one pair (${DEV_ALLOW} and ${DEV_DENY})
	# of the following filter rules to enable USB devices to be controlled
	# by other processes.

	# rule for use with SX NAS Plus
	# cat /etc/sxuptp/storage_filter.allow > ${DEV_ALLOW}
	# cat /etc/sxuptp/storage_filter.deny  > ${DEV_DENY}

	# rule for binding all USB devices to net.USB
	echo "" > ${DEV_ALLOW}
	echo "" > ${DEV_DENY}
}

svu_start() {
	echo "Loading net.USB modules..."

	# load core modules
	${INSMOD} ${KMOD_DIR}/wq/sxuptp_wq.ko
	${INSMOD} ${KMOD_DIR}/daemon/sxuptp.ko \
		netif=${IFNAME} maxconn=${MAXCONN}

	# enable USB Device Filter
	# ${INSMOD} ${KMOD_DIR}/daemon/sxuptp_devfilter.ko

	# install filter rules
	# install_device_filter

	# load generic USB device driver
	${INSMOD} ${KMOD_DIR}/drivers/sxuptp_driver.ko

	echo "Starting sxdev daemon: ${SXDEVD}"
	${SXDEVD}

	echo "Starting JCP Daemon: ${JCPD}"
	${JCPD}
}

svu_stop() {
	echo "Stopping JCP Daemon..."
	killall jcpd

	echo "Stopping sxdev daemon..."
	killall sxdevd

	echo "Unloading net.USB modules..."

	# unload generic USB device driver
	${RMMOD} sxuptp_driver

	# disable USB Device Filter
	# ${RMMOD} sxuptp_devfilter

	# unload core modules
	${RMMOD} sxuptp
	${RMMOD} sxuptp_wq
}

case "$1" in
'start')
	svu_start ;;
'stop')
	svu_stop ;;
*)
	echo "usage $0 start|stop"
esac
