#!/bin/bash

if [ $# != 2 ];then
   echo "Usage sdinstall.sh dev serial"
   echo "  ex: ./sdinstall.sh sdf 1214100080"
   exit
fi

DEV=$1

DVR_ID=$2
DVR_ID_cert=$2-cert.pem
DVR_ID_key=$2-rsa-key.pem

if [ ! -e certs/${DVR_ID_cert} ];then
   echo ${DVR_ID_cert} file not found
   exit
fi

echo "Make partitions"
./create_partitions.sh ${DEV}

umount mnt > /dev/null 2>&1
rm -rf mnt
mkdir mnt
echo "Installing uboot"
dd if=../images/u-boot.imx of=/dev/${DEV} bs=512 seek=2; sync
echo "Copy kernel and boot"
mount /dev/${DEV}1 ./mnt
cp ../bootcmd/boot.scr mnt/boot.scr
echo part_a > mnt/part_a
sync
umount mnt

echo "Installing root file system..."
mount /dev/${DEV}2 mnt
tar zxf ../RFS-3.0.35.tar.gz -C mnt

/bin/cp ../../etc/odi* mnt/etc/init.d
/bin/cp ../../etc/fstab mnt/etc
/bin/cp ../../etc/rcS mnt/etc/default
/bin/cp ../../etc/shadow mnt/etc
/bin/cp ../../bin/* mnt/usr/local/bin
/bin/cp ../binx/* mnt/usr/local/bin
/bin/cp ../MicrocodeUpdater/mC_update mnt/usr/local/bin
/bin/cp ../images/uImage mnt/boot

/bin/chmod +x mnt/usr/local/bin/*

echo Installing gstreamer libraries
LIB_DIR=mnt/usr/lib/
LIB_FILES="../gstreamer/libgstaudio-0.10.*"

GST_LIB_DIR=mnt/usr/lib/gstreamer-0.10/
GST_LIB_FILES="../gstreamer/libgstcoreelements.* ../gstreamer/libgstmatroska.* ../gstreamer/libgstmultifile.* ../gstreamer/libgstjpeg.* ../gstreamer/libmfw_gst_mp3enc.*"
/bin/cp $LIB_FILES $LIB_DIR
/bin/cp $GST_LIB_FILES $GST_LIB_DIR

sync
umount mnt

echo "Copy configuration files"
mount /dev/${DEV}5 ./mnt
rsync -a ../../conf/* ./mnt
echo Copying pem files $DVR_ID_cert,$DVR_ID_key and setting DVR_ID number
cp certs/${DVR_ID_cert} mnt/MobileHD-cert.pem
cp certs/${DVR_ID_key} mnt/MobileHD-key.pem
sed -i "/dvr_id>/c\      <dvr_id>${DVR_ID}</dvr_id>" mnt/config.xml
sync
umount mnt

mount /dev/${DEV}7 ./mnt
mkdir mnt/upload
sync
umount mnt
rm -rf mnt
