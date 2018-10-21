#!/bin/sh

PATH=$PATH:/sbin:/bin:/usr/sbin/:/usr/bin
export PATH

if [ ! "$#" = 4 ] && [ ! "$#" = 5 ]; then
    echo "Usage: $0 <PRODUCT_NAME> <KNL_IMAGE> <KNL_IMAGE_NAME> <TOP_DIR> <CONFIG_FLASH_SECTOR_SIZE>"
    exit 1
fi

#
# Parameter
#
PRODUCT_NAME=$1
KNL_IMG=$2
KNL_IMG_NAME=$3
TOP_DIR=$4
CONFIG_FLASH_SECTOR_SIZE=$5
#
# Config Parameter
#
HAS_FW1_FW2="$CONFIG_HAS_FW1_FW2"
USE_COMBINED_FW="$CONFIG_USE_COMBINED_FW_IMAGE"

#
# Two firmware
#
if [ "$HAS_FW1_FW2" = "y" ]; then
HEADERTYPE="apps"
else
HEADERTYPE="kernel"
fi

#
# USE_COMBINED_FW : Apps.sqsh combine to Kernel Image 
# 
if [ "$USE_COMBINED_FW" = "y" ]; then
#
# Apps squashfs file
# 
APPS_NAME_SQSH=apps$PRODUCT_NAME.sqsh
APP_IMG=$TOP_DIR/TARGET_RAMDISK/IMG/$APPS_NAME_SQSH
#
# Sector Size, default 64K : 0x10000
# 
if [ -z $CONFIG_FLASH_SECTOR_SIZE ]; then
FLASH_SECTOR_SIZE=65536
else
FLASH_SECTOR_SIZE=$CONFIG_FLASH_SECTOR_SIZE
fi
#
# Do combine
# 
EXTERN_COMMAND="-z -c $APP_IMG -b $FLASH_SECTOR_SIZE"
else
EXTERN_COMMAND=""
fi


#
# Kernel Image file
#
KNL_NAME_DLF=kernel$PRODUCT_NAME.dlf
KNL_TYPE_IMG=$TOP_DIR/TARGET_RAMDISK/IMG/$KNL_IMG_NAME
KNL_DLF_IMG=$TOP_DIR/TARGET_RAMDISK/IMG/$KNL_NAME_DLF

#
# Create Image Dir
#
mkdir -p $TOP_DIR/TARGET_RAMDISK/IMG

echo "Adding header to kernel image, please wait..."
rm -rf $KNL_DLF_IMG

#
# install KNL_IMG to /tftpboot/KNL_IMG_NAME
#
install -m 755 $KNL_IMG /tftpboot/$KNL_IMG_NAME$PRODUCT_NAME

#
# Add header to the apps.sqsh
#
$TOP_DIR/util/header.x86 -s /tftpboot/$KNL_IMG_NAME$PRODUCT_NAME -d $KNL_DLF_IMG -a -t $HEADERTYPE $EXTERN_COMMAND

#
# Install KNL_DLF_IMG to /tftpboot/KNL_NAME_DLF
#
install -m 755 $KNL_DLF_IMG   /tftpboot/$KNL_NAME_DLF

#
# Show file
#
ls -al /tftpboot/$KNL_IMG_NAME$PRODUCT_NAME
ls -al /tftpboot/$KNL_NAME_DLF
