#!/bin/sh

MINIDLNA=/usr/sbin/minidlna
MINIDLNA_TMP=/tmp/minidlna

AUTO_CONFIG=${MINIDLNA_TMP}/minidlna.conf
DBDIR_CONFIG=${MINIDLNA_TMP}/dbdir.conf
MAIN_CONFIG=${MINIDLNA_TMP}/main.conf

PID_FILE=/var/run/minidlna.pid

#DEFAULT_CACHE_SIZE=10240 # KB
DB_PATH=${MINIDLNA_TMP}/cache
VENDOR_DB_PATH=.iodata/minidlna/cache

VSZ_LIMIT=24576

update_db_path()
{
	ORDER=0
	while :
	do
		ORDER=`expr ${ORDER} + 1`
		MNT_INFO=`sxhotpluginfo -s ${ORDER}`
		if [ $? -ne 0 ]; then
			break
		fi

		# parse MNT_PATH and MNT_SIZE
		eval "${MNT_INFO}"
		if [ $? -ne 0 ]; then
			continue
		fi

#		if [ ${MNT_SIZE} -lt ${DEFAULT_CACHE_SIZE} ]; then
#			break
#		fi

		# check if the partition is writable
		mkdir -p ${MNT_PATH}/${VENDOR_DB_PATH} || continue
		touch ${MNT_PATH}/${VENDOR_DB_PATH} || continue

		DB_PATH=${MNT_PATH}/${VENDOR_DB_PATH}
		break
	done
}

minidlna_start()
{
	echo "Starting MiniDLNA: ${MINIDLNA}"

	if [ ! -f ${MAIN_CONFIG} ]; then
		echo "  Failed: configuration file (${MAIN_CONFIG}) not found"
		return 1
	fi

	update_db_path
	if [ -z "${DB_PATH}" ]; then
		echo "  Failed: writable volumes are not found"
		return 1
	fi

	mkdir -p ${DB_PATH}
	echo "  db_dir = ${DB_PATH}"

	cp ${MAIN_CONFIG} ${AUTO_CONFIG}
	echo "db_dir=${DB_PATH}" >> ${AUTO_CONFIG}
	echo "db_dir=${DB_PATH}" > ${DBDIR_CONFIG}

	${MINIDLNA} -f ${AUTO_CONFIG} -R
}

minidlna_stop()
{
	echo "Stopping MiniDLNA..."
	if [ ! -f ${PID_FILE} ]; then
		return 0
	fi

	killall -9 minidlna
	rm -f ${PID_FILE}

	eval `cat ${DBDIR_CONFIG}`
	if [ -n "${db_dir}" ]; then
		echo "  Deleting ${db_dir} ..."
		rm -rf ${db_dir}
	fi
}

minidlna_rescan()
{
	minidlna_stop
	minidlna_start
}

minidlna_restart()
{
	echo "Restarting MiniDLNA..."

	if [ ! -f ${AUTO_CONFIG} ]; then
		echo "  Failed: configuration file (${AUTO_CONFIG}) has not"
		     "been generated"
		return 1
	fi

	killall -9 minidlna
	rm -f ${PID_FILE}

	${MINIDLNA} -f ${AUTO_CONFIG}
}

ulimit -v ${VSZ_LIMIT}

case "$1" in
'start')
	minidlna_start ;;
'stop')
	minidlna_stop ;;
'rescan')
	minidlna_rescan ;;
'restart')
	minidlna_restart ;;
*)
	echo "usage $0 rescan|restart|start|stop"
esac
