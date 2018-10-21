#!/bin/sh

PATH=$PATH:/sbin:/bin:/usr/sbin/:/usr/bin
export PATH

if [ ! "$#" = 8 ]; then
    echo "Usage: $0 <PRODUCT_NAME> <TARGET_DIR> <TARGET_APP_DIR> <TARGET_APP_CORE_DIR> <TOP_DIR> <BIN_FORMAT> <MKSQUASHFS> <SYSTEM_IS_LITTLE_ENDIAN>"
    exit 1
fi

#
# Parameter
#
PRODUCT_NAME=$1
TARGET_DIR=$2
TARGET_APP_DIR=$3
TARGET_APP_CORE_DIR=$4
TOP_DIR=$5
BIN_FORMAT=$6
MKSQUASHFS=$7
SYSTEM_IS_LITTLE_ENDIAN=$8


#
# Config Parameter
#
NO_APPSCORE="$CONFIG_NO_APPSCORE"
USE_ONE_FW_IMAGE="$CONFIG_USE_ONE_FW_IMAGE"

#
# Apps squashfs file
# 
APPS_NAME_SQSH=apps$PRODUCT_NAME.sqsh
APPS_NAME_DLF=apps$PRODUCT_NAME.dlf

#
# Appscore squashfs file
#
APP_CORE_IMG=$TARGET_DIR/../IMG/appscore.sqsh

#
# App Image file
#
APP_IMG=$TARGET_DIR/../IMG/$APPS_NAME_SQSH
APP_DLF_IMG=$TARGET_DIR/../IMG/$APPS_NAME_DLF

#
# Create Image Dir
#
mkdir -p $TARGET_DIR/../IMG

#
# Appscore
#
if [ ! "$NO_APPSCORE" = "y" ]; then
#
# Delete Appscore Image
#
rm -rf $APP_CORE_IMG
#
# Big Endian or Little Endian
#
if [ "$SQUASHFS_NO_OPTION" = "y" ]; then
#
# Mksquashfs Appscore.sqsh
#
echo "Compressing the APPS Core Stage into squash format(little endian) no [-le], please wait..."
$MKSQUASHFS $TARGET_APP_CORE_DIR $APP_CORE_IMG	>	/dev/null
elif [ "$SYSTEM_IS_LITTLE_ENDIAN" = "yes" ]; then
#
# Mksquashfs Appscore.sqsh
#
echo "Compressing the APPS Core Stage into squash format(little endian)[-le], please wait..."
$MKSQUASHFS $TARGET_APP_CORE_DIR $APP_CORE_IMG -le  > /dev/null
else
#
# Mksquashfs Appscore.sqsh
#
echo "Compressing the APPS Core Stage into squash format(big endian), please wait..."
$MKSQUASHFS $TARGET_APP_CORE_DIR $APP_CORE_IMG -be  > /dev/null
fi

echo "Finished"
#
# Show
#
ls -al $APP_CORE_IMG

echo "Copy the " $APP_CORE_IMG " to " $TARGET_APP_DIR "."
cp $APP_CORE_IMG $TARGET_APP_DIR
echo "Finished"
fi

#
# Apps Image
#
#
# Delete Apps Image
#
rm -rf $APP_IMG
#
# Big Endian or Little Endian
#
if [ "$SQUASHFS_NO_OPTION" = "yes" ]; then
#
# Mksquashfs Apps.sqsh
#
echo "Compressing the APPS Stage into squash format(little endian) no [-le], please wait..."
$MKSQUASHFS  $TARGET_APP_DIR $APP_IMG	>	/dev/null
elif [ "$SYSTEM_IS_LITTLE_ENDIAN" = "yes" ]; then
#
# Mksquashfs Apps.sqsh
#
echo "Compressing the APPS Stage into squash format(little endian)[-le], please wait..."
$MKSQUASHFS  $TARGET_APP_DIR $APP_IMG -le  > /dev/null
else
#
# Mksquashfs Apps.sqsh
#
echo "Compressing the APPS Stage into squash format(big endian), please wait..."
$MKSQUASHFS  $TARGET_APP_DIR $APP_IMG -be  > /dev/null
fi
echo "Finished"

#
# Add header to the apps.sqsh
#
$TOP_DIR/util/header.x86 -s $APP_IMG -d $APP_DLF_IMG -a -t "apps"

#
# Install
#
install -m 755 $APP_IMG   /tftpboot/$APPS_NAME_SQSH
install -m 755 $APP_DLF_IMG   /tftpboot/$APPS_NAME_DLF

#
# Show file
#
ls -al /tftpboot/$APPS_NAME_SQSH
ls -al /tftpboot/$APPS_NAME_DLF

#
# USE_ONE_FW_IMAGE install apps.sqsh to TARGET_DIR
#
if [ "$USE_ONE_FW_IMAGE" = "y" ]; then
    if [ -e /tftpboot/$APPS_NAME_SQSH ]; then
        install -m 755 /tftpboot/$APPS_NAME_SQSH $TARGET_DIR/apps.sqsh
    fi
else
    rm -rf $TARGET_DIR/apps.sqsh
fi

