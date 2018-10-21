#!/bin/sh

IFNAME="br0"

SXPSPD=/sbin/sxpspd
SXPSPD_PID=/var/run/sxpspd.pid

sxpsplus_start() {
	echo "Initialize SX PrintServer Plus"

	mkdir -p /dev/usb
	if [ ! -c /dev/usb/lp0 ]; then
		mknod /dev/usb/lp0 c 180 0
	fi
	if [ -x ${SXPSPD} ]; then
		EXEC="${SXPSPD} -i ${IFNAME} -D"
		echo "Starting SX PrintServer Plus Daemon: ${EXEC}"
		${EXEC}
	fi
}

sxpsplus_stop() {
	if [ -f ${SXPSPD_PID} ]; then
		PID=`cat ${SXPSPD_PID}`
		rm -rf ${SXPSPD_PID}
	fi
	if [ -n ${PID} ]; then
		echo "Stop SX PrintServer Plus"
		kill ${PID} 2>/dev/null
	fi
}

case "$1" in
'start')
	sxpsplus_start ;;
'stop')
	sxpsplus_stop ;;
*)
	echo "usage $0 start|stop"
esac

