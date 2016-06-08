#!/bin/bash

if [ $# != 3 ];then
   echo "Usage sdinstall.sh dev serial SU_image"
   echo "  ex: ./sdinstall.sh sdf 1214100080 BodyVision_1.2.1"
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
sudo ./create_partitions.sh ${DEV}

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
sudo rm -rf tem
mkdir tem
tar xf $3 -C tem
mount /dev/${DEV}2 mnt
tar zxf ./tem/SU_image.tar.gz -C ./mnt

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
