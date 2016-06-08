#!/bin/bash

DEV=$1

DISK_SIZE=`fdisk -s /dev/$1`
for n in /dev/${DEV}*; do sudo umount $n; done > /dev/null 2>&1

if [ $DISK_SIZE -gt 16000000 ] ; then
    echo "System disk size: $DISK_SIZE run 32G script"
    sfdisk /dev/${DEV} < kernel_layout.32 > /dev/null 2>&1
else
    echo "System disk size: $DISK_SIZE run 16G script"
    sfdisk /dev/${DEV} < kernel_layout > /dev/null 2>&1
fi

echo "[Making filesystems...]"
echo "mkfs boot"
mkfs.vfat -F 32 -n boot /dev/${DEV}1 &> /dev/null
echo "mkfs rfs1"
mkfs.ext3 -L rfs1 /dev/${DEV}2 &> /dev/null
echo "mkfs rfs2"
mkfs.ext3 -L rfs2 /dev/${DEV}3 &> /dev/null
echo "mkfs conf"
mkfs.ext3 -L conf /dev/${DEV}5 &> /dev/null
echo "mkfs log"
mkfs.ext3 -L log /dev/${DEV}6 &> /dev/null
echo "mkfs data"
mkfs.ext3 -L data /dev/${DEV}7&> /dev/null
echo "[Filesystem create done]"
