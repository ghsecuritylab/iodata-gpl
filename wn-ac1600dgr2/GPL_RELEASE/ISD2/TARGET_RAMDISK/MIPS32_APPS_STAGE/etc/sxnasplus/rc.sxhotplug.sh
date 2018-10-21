#!/bin/sh

MOUNT_SYSFS=0
INSMOD_USB_STORAGE=0
KERNELVERSION=2.6.21

# Kernel Hotplug Setting
PROC_HOTPLUG=/proc/sys/kernel/hotplug
HOTPLUG=/sbin/hotplug

# Hotplug Daemon Setting
HOTPLUG_DAEMON=/sbin/sxhotplugd
HOTPLUG_CONFIG=sxhotplugd.conf
HOTPLUG_CONF_DEV=hotplug.dev.conf

CONFIG_BASE=/etc/sxnasplus
CONFIG_TEMP=/tmp/sxnasplus

DEVICE_PATH=/tmp/dev
MOUNTPOINT_PATH=/tmp/mnt/shared

SYSFS_PATH=/sys
USB_STORAGE_MOD=usb-storage.ko
USB_STORAGE=/lib/modules/${KERNELVERSION}/kernel/drivers/usb/storage/${USB_STORAGE_MOD}

hotplug_start() {
	echo "Initialize Hotplug"

	if [ ${MOUNT_SYSFS} -eq 1 ]; then
		echo "Mount SysFS"
		if [ ! -d  ${SYSFS_PATH} ]; then
			mkdir -p ${SYSFS_PATH}
		fi
		mount -t sysfs sysfs ${SYSFS_PATH}
	fi

	if [ ! -d  ${CONFIG_TEMP} ]; then
		mkdir -p ${CONFIG_TEMP} -m 0700
	fi

	if [ ! -d  ${DEVICE_PATH} ]; then
		mkdir -p ${DEVICE_PATH} -m 0755
	fi

	if [ ! -d  ${MOUNTPOINT_PATH} ]; then
		mkdir -p ${MOUNTPOINT_PATH} -m 0755
	fi

	# copy config file
	if [ -f ${CONFIG_BASE}/${HOTPLUG_CONFIG} ]; then
		if [ ! -f ${CONFIG_TEMP}/${HOTPLUG_CONFIG} ]; then
			cp ${CONFIG_BASE}/${HOTPLUG_CONFIG} ${CONFIG_TEMP}/${HOTPLUG_CONFIG}
			chmod 0600 ${CONFIG_TEMP}/${HOTPLUG_CONFIG}
		fi
	fi

	# copy config file
	if [ -f ${CONFIG_BASE}/${HOTPLUG_CONF_DEV} ]; then
		if [ ! -f ${CONFIG_TEMP}/${HOTPLUG_CONF_DEV} ]; then
			cp ${CONFIG_BASE}/${HOTPLUG_CONF_DEV} ${CONFIG_TEMP}/${HOTPLUG_CONF_DEV}
			chmod 0600 ${CONFIG_TEMP}/${HOTPLUG_CONF_DEV}
		fi
	fi

#	if [ -f ${PROC_HOTPLUG} ]; then
#		if [ -f ${HOTPLUG} ]; then
#			echo "${HOTPLUG}" > ${PROC_HOTPLUG}
#		fi
#	fi

	if [ -x ${HOTPLUG_DAEMON} ]; then
		echo "Starting HotPlug:                       ${HOTPLUG_DAEMON}"
		${HOTPLUG_DAEMON} -f ${CONFIG_TEMP}/${HOTPLUG_CONFIG}
	fi

	if [ ${INSMOD_USB_STORAGE} -eq 1 ]; then
		echo "insmod USB Storage Module"
		insmod ${USB_STORAGE}
	fi
}

hotplug_stop() {
	echo "Unmount All Hotplug Device"
	sxunmount all

	echo "Stop Hotplug Daemon"
	killall sxhotplugd 2>/dev/null

	if [ ${INSMOD_USB_STORAGE} -eq 1 ]; then
		echo "rmmod USB Storage Module"
		rmmod ${USB_STORAGE_MOD}
	fi
}

case "$1" in
'start')
	hotplug_start ;;
'stop')
	hotplug_stop ;;
*)
	echo "usage $0 start|stop" ;;
esac
