mkdir -p /tmp/linuxigd
LOCKFILE=/tmp/linuxigd/ConfigurationToken.lock
PSFILE=/tmp/linuxigd/ps.txt

if [ "$1" == "lock" ];then
    sleep 300
    $0 unlock
    exit 0
fi

if [ "$1" == "check" ];then
    CONFIG_TOKEN=`cat $LOCKFILE 2>/dev/null`
    #locked just get config_token
    ls $LOCKFILE 1>/dev/null 2>&1
    if [ "$?" == "0" ]; then
        if [ "$2" == "$CONFIG_TOKEN" ]; then
            printf 'pass'
        else
            printf 'fail'
        fi
    else
        printf 'none'
    fi
    exit 0
fi

if [ "$1" == "unlock" ];then
    ps > $PSFILE
    processid=`cat $PSFILE | grep "$0 lock" |awk '{print $1}'`
    kill -9 $processid 1>/dev/null 2>&1
    rm -rf $LOCKFILE
    printf 'unlock'
    exit 0
fi

if [ "$1" == "get" ];then
    #locked just get config_token
    ls $LOCKFILE 1>/dev/null 2>&1
    if [ "$?" == "0" ]; then
        CONFIG_TOKEN=`cat $LOCKFILE`
        printf $CONFIG_TOKEN
    else
        TIMENOW=`date +%s`
        HEX_TIMENOW=`printf "%08x" $TIMENOW`
        HEX_RANDOM=`printf "%04x" $RANDOM;printf "%04x" $RANDOM`
        CONFIG_TOKEN="$HEX_TIMENOW""$HEX_RANDOM"
        printf $CONFIG_TOKEN
        echo $CONFIG_TOKEN > $LOCKFILE
        # we cannot lock here, it will cause system_interact locked.
        #$0 lock 300 &
    fi
    exit 0
fi

