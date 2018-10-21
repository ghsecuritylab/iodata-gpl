#!/bin/sh
if [ "$1" == "-h" ] || [ "$1" == "" ];then
    echo "-r [IP4] [filename]    -- Download file, IP4 default 200"
    echo "-r [IP4] -p [filename] -- Upload file, IP4 default 200, please touch filename in /tftpboot and chmod 777 first."
    echo "-r [IP4] -z            -- Reload httpd daemon, IP4 default 200"
    echo "-d [1:sys/2:apcfg/3:sysUtil] -- Debug"
    echo "-s [LAN_MAC] [WAN_MAC] -- For modify lan/wan mac by setconfig"
    exit 1
fi;

# for modify LAN/WAN mac
if [ "$1" == "-s" ] ; then
    setconfig -a 1
    if [ "$1" != "null" ] ; then
        old_lanmac=`setconfig -g 6`
        echo "old lan mac: $old_lanmac"
        setconfig -a 2 -s 6 -d $2
        if [ $? -eq 0 ] ; then #set ok
            modlan=1
        fi
    fi
    if [ "$3" != "null" ] ; then
        old_wanmac=`setconfig -g 7`
        echo "old wan mac: $old_wanmac"
        setconfig -a 2 -s 7 -d $3
        if [ $? -eq 0 ] ; then #set ok
            modwan=1
        fi
    fi
    echo "====="
    setconfig -a 5
    if [ $modlan ] ; then
        new_lanmac=`setconfig -g 6`
        echo "new lan mac: $new_lanmac"
    fi
    if [ $modwan ] ; then
        new_wanmac=`setconfig -g 7`
        echo "new wan mac: $new_wanmac"
    fi
    exit 0;
fi

# for senao debug
if [ "$1" == "-d" ];then
    shift
    ans1=$1
    if [ "$ans1" == "" ];then
    echo "init....do starteth.sh..."
    starteth.sh 2>/dev/null
    mkdir -p /debug_sys
    cd /debug_sys;
    echo "1: sysconfig debug."
    echo "2: apcfg debug."
    echo "3: sysUtil debug."
    read ans1
    fi
    if [ "$ans1" = "1" ]; then
        echo "dl.sh sysconfd"
        dl.sh sysconfd
        echo "dl.sh sysconf_cli"
        dl.sh sysconf_cli
    elif [ "$ans1" = "2" ]; then
        echo "dl.sh apcfg_init"
        dl.sh apcfg_init
        echo "dl.sh libapcfg.so"
        dl.sh libapcfg.so
    elif [ "$ans1" = "3" ]; then
        echo "dl.sh libsysUtil.so"
        dl.sh libsysUtil.so
    fi

    if [ $? -ne 0 ];then
        echo "fail"
        exit 1;
    else
        echo "file downloaded to /debug_sys"
    fi
    exit 0;
fi;

# easy to download file
ic=`ifconfig br0 2>/dev/null|grep "inet addr"`;
if [ $? -ne 0 ];then
    i1=192;i2=168;i3=99;i4=8;
else
    tp=${ic%\.* Bcast*};
    i3=${tp#*:*\.*\.};
    tp=${tp%\.*};
    i2=${tp#*:*\.};
    tp=${tp%\.*};
    i1=${tp#*:};
    if [ "$i3" == "99" ];then
        i4=8;
    else
        i4=200;
    fi;
fi;

while [ "$1" != "" ];
do
    # get IP if input '-r'
    if [ "$1" == "-r" ];then
        shift;
        i4=$1;
        shift;
    fi;

    # put file
    if [ "$1" == "-p" ];then
        shift;
        tftp -p -r $1 $i1.$i2.$i3.$i4;
        exit 0
    fi;

    # for web debug
    if [ "$1" == "-z" ];then
        shift;
        if [ "$1" == "" ];then
            file="httpd";
        else
            file=$1;
        fi;
        echo "kill processmanager, and Reload httpd!"
        cd /mnt/;killall -9 processmanager;killall -9 $file;tftp -g -r $file $i1.$i2.$i3.$i4;chmod 755 $file;/mnt/$file -dddd&
        exit 0
    fi;

    # download file
    rm -rf $1;
    tftp -g -r $1 $i1.$i2.$i3.$i4;
    chmod 777 $1;
    shift;
done;
exit 0


