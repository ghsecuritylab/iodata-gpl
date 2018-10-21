#!/bin/sh

PATH=$PATH:/sbin:/bin:/usr/sbin/:/usr/bin
export PATH

KEL_DIR=( "/ISD2/MIPS32" "/ISD2/RALINK_KNL" "/ISD2/UBICOM" "/ISD2/ATHEROS_LSDK_KNL")

for ((i=0; i<${#KEL_DIR[@]}; i++)); do
    kernel=`ls "${KEL_DIR[$i]}" 2>/dev/null`
    if [ $? -eq 0 ] ; then
        kernel="${KEL_DIR[$i]}/$kernel";
        break;
    fi
done

if [ "$i" -ge "${#KEL_DIR[@]}" ] ; then
	echo "Please check you have a kernel folder."
	exit 0
else
    cd $kernel
fi

mkdir -p /TARGET_RAMDISK;
rm -rf /TARGET_RAMDISK/MIPS32_STAGE;
ln -sf /ISD2/TARGET_RAMDISK/MIPS32_STAGE /TARGET_RAMDISK/MIPS32_STAGE;
make -f Makefile.ISD clean all install
cd /ISD2/scripts
make -f Makefile.IMG

