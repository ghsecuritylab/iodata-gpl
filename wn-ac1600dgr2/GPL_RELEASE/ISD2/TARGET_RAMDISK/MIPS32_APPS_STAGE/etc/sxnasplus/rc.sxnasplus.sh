#!/bin/sh

IFNAME=br0

SXNAS_CONF=sxnas.conf
SXNAS_USER_CONF=sxnas.user.conf

CONF_BASE=/etc/sxnasplus
CONF_TEMP=/tmp/sxnasplus

SXNAS_FSSD=/sbin/sxnas-fssd
SXNAS_NSD=/sbin/sxnas-nsd

sxnasplus_start() {
	echo "Initialize SX NAS Plus"

	if [ ! -d  ${CONF_TEMP} ]; then
		mkdir -p ${CONF_TEMP} -m 0700
	fi

	# copy config file
	if [ -f ${CONF_BASE}/${SXNAS_CONF} ]; then
		if [ ! -f ${CONF_TEMP}/${SXNAS_CONF} ]; then
			cp ${CONF_BASE}/${SXNAS_CONF} ${CONF_TEMP}/${SXNAS_CONF}
			chmod 0600 ${CONF_TEMP}/${SXNAS_CONF}
		fi
	fi

	# copy config file
	if [ -f ${CONF_BASE}/${SXNAS_USER_CONF} ]; then
		if [ ! -f ${CONF_TEMP}/${SXNAS_USER_CONF} ]; then
			cp ${CONF_BASE}/${SXNAS_USER_CONF} ${CONF_TEMP}/${SXNAS_USER_CONF}
			chmod 0600 ${CONF_TEMP}/${SXNAS_USER_CONF}
		fi
	fi

	if [ -x ${SXNAS_FSSD} ]; then
		echo "Starting SX NAS Plus FSS Daemon:            ${SXNAS_FSSD}"
		${SXNAS_FSSD} -f ${CONF_TEMP}/${SXNAS_CONF}
	fi

	if [ -x ${SXNAS_NSD} ]; then
		echo "Starting SX NAS Plus NS Daemon:             ${SXNAS_NSD}"
		${SXNAS_NSD} -f ${CONF_TEMP}/${SXNAS_CONF} -i ${IFNAME} -D
	fi
}

sxnasplus_stop() {
	echo "Stop SX NAS Plus"
	killall sxnas-fssd
	killall sxnas-nsd
}

case "$1" in
'start')
	sxnasplus_start
	;;
'stop')
	sxnasplus_stop
	;;
*)
	echo "usage $0 start|stop"
	;;
esac
