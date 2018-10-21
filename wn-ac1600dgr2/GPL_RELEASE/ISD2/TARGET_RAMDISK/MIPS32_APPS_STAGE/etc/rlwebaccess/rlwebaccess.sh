#!/bin/sh

DAEMON_NAME=rlwebaccess
DAEMON_EXEC=rlwebaccess
BIN_DIR=/sbin
DAEMON_PATH=$BIN_DIR/$DAEMON_EXEC
INI_NAME=rlweb.ini
INI_DIR=/etc/rlwebaccess
INI_PATH=$INI_DIR/$INI_NAME

start_daemon()
{
    echo Starting $EXEC_NAME
    $DAEMON_PATH -cnf $INI_PATH &
}

stop_daemon()
{
    echo Stopping $DAEMON_NAME
    killall -15 $DAEMON_EXEC
    sleep 1
    killall -9 $DAEMON_EXEC
}

case "$1" in
    start)
        start_daemon
        ;;
    stop)
        stop_daemon
    ;;
    restart)
        echo Restart ...
        stop_daemon
        sleep 2
        start_daemon
        ;;
    *)
        echo "Usage: $SCRIPT_NAME {start|stop|restart}" >&2
        exit 1
        ;;
esac
