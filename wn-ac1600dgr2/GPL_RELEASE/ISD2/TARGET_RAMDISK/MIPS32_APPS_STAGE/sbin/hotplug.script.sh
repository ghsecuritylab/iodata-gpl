#!/bin/sh

RC_MINIDLNA=/tmp/sxnasplus/rc.minidlna.sh

prehotplug()
{
	$RC_MINIDLNA stop
}

posthotplug()
{
	$RC_MINIDLNA start
}

case "$1" in
'pre-hotplug')
	prehotplug ;;
'hotplug')
	posthotplug ;;
*)
	;;
esac
