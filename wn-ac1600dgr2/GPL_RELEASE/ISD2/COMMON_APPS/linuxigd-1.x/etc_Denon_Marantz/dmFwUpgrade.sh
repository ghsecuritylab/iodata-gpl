#!/bin/sh

fwtempfile="/mnt/kernel.dlf"
wgetfile="/tmp/wgetpid"

urlfile=$1
progressfile=$2
statusfile=$3
checksum=$4
mtdpath=$5
vid=$6
pid=$7

if [ "$urlfile" = "GetUpdateProgress" ];then
    fwprg=`test -f $progressfile && cat $progressfile |grep % |tail -1`;
    if [ "$fwprg" = "" ];then
        printf "0"
    else
        fwprg=${fwprg%%%*};
        fwprg=${fwprg##* };
        printf $fwprg
    fi
    exit 0
fi

if [ "$urlfile" = "CancelFirmwareUpdate" ];then
    if [ ! -e $wgetfile ];then
        #cannot cancel fw upgrade when dlownload complete.
        printf "fail"
        exit 1
    fi
    wgetpid=`cat $wgetfile |awk '{printf $5}'`
    wgetpid=${wgetpid%.}
    kill -9 $wgetpid
    echo "0%" > $progressfile
    rm -rf $fwtempfile
    rm -rf $wgetfile
    echo 'UPDATE_CANCELLED' > $statusfile
    killall dmFwUpgrade.sh
    exit 0
fi

#firmware upgrade stat
#UPDATE_INITIAL
#UPDATE_IN_PROGRESS
#UPDATE_DONE
#UPDATE_DOWNLOAD_FAILED
#UPDATE_CANCELLED

echo 'UPDATE_INITIAL' > $statusfile

wget -O $fwtempfile -o $progressfile -b --no-check-certificate -i $urlfile > $wgetfile
for i in 1 2 3;
do
    sleep $i
    cat $progressfile |grep "Saving to: \`$fwtempfile'"
    if [ $? -ne 0 ];then
        if [ $i = "3" ];then
            echo 'UPDATE_DOWNLOAD_FAILED' > $statusfile
            exit 1
        fi
    else
        break
    fi
done

echo 'UPDATE_IN_PROGRESS' > $statusfile

network_failue_times=0
file_info_compare_before=""
file_info_compare_after=""
while true;
do
    file_info_compare_before=`ls $progressfile`
    #Use file info to check download progress
    if [ "$file_info_compare_before" != "$file_info_compare_after" ];then
        file_info_compare_after=$file_info_compare_before
    else
        network_failue_times=$(($network_failue_times+1))
    fi

    #failure 30 times --> 60 secs
    if [ $network_failue_times -ge 30 ];then
        $0 CancelFirmwareUpdate
        echo 'UPDATE_DOWNLOAD_FAILED' > $statusfile
        exit 1
    fi

    sleep 2;
    cat $progressfile |grep "\`$fwtempfile' saved"
    if [ $? -eq 0 ];then
        break;
    fi
done

md5read=`md5sum $fwtempfile |awk '{printf $1}'`
if [ $md5read = $checksum ];then
    header -x $fwtempfile -r $vid -p $pid |grep "Return OK"
    if [ $? -ne 0 ];then
        echo 'UPDATE_DOWNLOAD_FAILED' > $statusfile
        echo "0%" > $progressfile
        rm -rf $fwtempfile
        rm -rf $wgetfile
        exit 1
    fi
else
    echo 'UPDATE_DOWNLOAD_FAILED' > $statusfile
    echo "0%" > $progressfile
    rm -rf $fwtempfile
    rm -rf $wgetfile
    exit 1
fi

# start flashw we remove $wgetfile to avoid cancel flashw
rm -rf $wgetfile

# finish download and check the FW file
echo 'UPDATE_DONE' > $statusfile
# wait upnp to notify status
sleep 5

sysconf_cli inform fw_closeModules
sleep 1

eraseall $mtdpath

# never eraseall fail
#if [ $? -ne 0 ];then
#    echo 'UPDATE_CANCELLED' > $statusfile
#    rm -rf $fwtempfile.bin
#    exit 1
#fi

flashw -e -f $fwtempfile.bin $mtdpath

# never flashw fail
#if [ $? -ne 0 ];then
#    echo 'UPDATE_CANCELLED' > $statusfile
#    rm -rf $fwtempfile.bin
#    exit 1
#fi

# complete fw upgrade
sysconf_cli inform reboot
exit 0

