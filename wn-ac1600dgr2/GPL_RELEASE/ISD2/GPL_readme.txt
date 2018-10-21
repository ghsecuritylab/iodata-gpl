
Linux OS :Ubuntu 10 or (before)11.04
--> install library:
        sudo apt-get install build-essential bison flex texinfo gettext sharutils tftp xinetd ssh tftpd libncurses5-dev libssl0.9.8 libssl-dev openssl libcurl4-openssl-dev libglib2.0-dev libgtk2.0-dev intltool gawk gcc-4.4 libtool automake autoconf autopoint
---------------------------------------------------------------

0. Uncompress GPL package and install ISD in root directory
 Step 1: tar -zxvf XXX.tar.gz 
 Step 2: cd GPL_RELEASE/
 Step 3: mv ISD2 /

1. Install toolchains in opt directory
 Step 1: cd /ISD2/toolchains/
 Step 2: tar -zxvf buildroot-gccxxxxxxxxx.tar.gz
 Step 3: change folder name to "buildroot-be-gcc434"
 Step 4: mv tooolchain folder to /opt
 Step 5: export PATH=/opt/buildroot-be-gcc434/usr/bin:$PATH

2. Build kerenl or desired open source library in MIPS32 and MIPS32_KNLAPPS directories. After
 building up objects, install those objects into TARGET_RAMDISK directory

 /ISD2/configs/kernel_def_config is kernel config.
 For example: Build up kernel image

 Step 1: mkdir /tftpboot
 Step 2: cd /ISD2/scripts/
 Step 3: sh build_firmware.sh
 (Note: firmwares are generated in /tftpboot directory)

3. Module : Please study /ISD2/MIPS32_KNLAPPS/samplecode, and use Makefile & Mafile.ISD to build driver.

4. Application : Please study /ISD2/MIPS32_APPS/samplecode, and use Makefile & Mafile.ISD to build application.

5. If you want to install application into firmware. 
 Step 1: please copy application to /TARGET_RAMDISK/MIPS32_STAGE/sbin/
 Step 2: cd /ISD2/scripts/;sh build_firmware.sh

6. If you want to auto run your application in firmware.
 Step 1: write script into /TARGET_RAMDISK/MIPS32_STAGE/etc/rc.d/rc.local
 Step 2: cd /ISD2/scripts/;sh build_firmware.sh

