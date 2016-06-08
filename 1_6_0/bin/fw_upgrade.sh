#!/bin/sh
echo "Firmware upgrade script version 1.0.22"
if [[ -z $1 ]] ; then
    echo "Please provide SU image name"
    exit 0
fi

IMG=$1
#check to see if /odi/data (mmcblk0p9) has enough space
USE=`df |grep "/odi/data" | awk '{print $4}'`
echo "Available space for upgrade process ${USE} kbytes"

if [ ${USE} -gt 1000000 ]; then
    umount /mnt > /dev/null 2>&1
    umount /media/upgrade > /dev/null 2>&1

    VER=`basename ${IMG} .tar | awk -F_  '{printf("%s.%s.%s\n",$2,$3,$4)}'`
    echo "Updating version to: ${VER}"

    # unpack SU_img.tar.gz to /odi/data to do rsa check
    rm -rf /odi/upload/upgrade
    cd /odi/upload
    mkdir -p upgrade
    echo "Unpacking update img ${IMG}..."
    tar xf /odi/${IMG} -C upgrade

    cd upgrade

    # RSA check upgrade image
    RSA=`/usr/local/bin/hashfile v SU_image.tar.gz Signature | cut -c18-24`
    if [ "$RSA" = "success" ]; then
        if [ -f /odi/upload/upgrade/pre_update.sh ];then
           /odi/upload/upgrade/pre_update.sh
        fi

        SCR=`ls boot.scr 2> /dev/null`
        if [ "$?" = "0" ];then
            echo "Updating boot.scr..."
            mount /dev/mmcblk0p1 /mnt
            cp boot.scr /mnt
            sync
            umount /mnt
        fi

        UCODE=`ls *.hex 2> /dev/null`
        if [ "$?" = "0" ];then
           echo "Updating micro code..."
           /usr/local/bin/mC_update ${UCODE} > /odi/log/u-code-update 2>&1
        fi

        mkdir /media/upgrade > /dev/null 2>&1
        #finding destination partition
        mount /dev/mmcblk0p1 /mnt        #Mount boot flag partition
        ls /mnt/part_b > /dev/null 2>&1
        PART_A=$?
        if [ "${PART_A}" = "0" ]; then
            echo "Boot from partition B then upgrade will be set for PART_A (mmcblk0p2)"
            mount -t ext3 /dev/mmcblk0p2 /media/upgrade
        else
            echo "Boot from partition A then upgrade will be set for PART_B (mmcblk0p3)"
            mount -t ext3 /dev/mmcblk0p3 /media/upgrade
        fi

        #start untar, unzip to upgrade partition
        rm -rf /media/upgrade/*
        rm -rf /media/upgrade/.*
        tar zxf /odi/upload/upgrade/SU_image.tar.gz -C /media/upgrade
        sync
        umount /media/upgrade

        if [ -f /odi/upload/upgrade/post_update.sh ];then
           /odi/upload/upgrade/post_update.sh
        fi

        rm -f /mnt/part*
        if [ "${PART_A}" = "0" ]; then
            echo part_a > /mnt/part_a
        else
            echo part_b > /mnt/part_b
        fi
    else
        echo "RSA check failed. Invalid SU image was used"
    fi
    #Clean up temp space
    rm -f /odi/${IMG}
    rm -rf /odi/upload/upgrade

    init 6 
else
    echo "There isn't enough temp space to unzip upgrade image"
fi
