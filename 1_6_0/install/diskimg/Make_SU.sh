#!/bin/bash

VERSION=`cat ../../src/version.h | awk '{print $3}' | sed 's/"//g' | sed 's/\./_/g'`
echo Clean up first
rm -rf RFS Signature BodyVision* SU_image.tar.gz

mkdir RFS
tar zxf ../RFS-3.0.35.tar.gz -C RFS

cp -rf RFS/home/root/.gstreamer-0.10 RFS/.

/bin/cp ../../bin/* RFS/usr/local/bin
/bin/cp ../../etc/odi-* RFS/etc/init.d
/bin/cp ../../etc/shadow RFS/etc
/bin/cp ../../etc/fstab RFS/etc
/bin/cp ../../etc/rcS RFS/etc/default
/bin/cp ../binx/* RFS/usr/local/bin
/bin/cp ../images/uImage RFS/boot
/bin/cp ../MicrocodeUpdater/mC_update RFS/usr/local/bin

/bin/cp -rf ../../usr/* RFS/usr

echo Installing gstreamer libraries
LIB_DIR=RFS/usr/lib/
LIB_FILES="../gstreamer/libgstaudio-0.10.* ../gstreamer/libffi.* ../gstreamer/libgstapp-0.10.so.0.25.0"

GST_LIB_DIR=RFS/usr/lib/gstreamer-0.10/
GST_LIB_FILES="../gstreamer/libgstcoreelements.* ../gstreamer/libgstmatroska.* ../gstreamer/libgstmultifile.* ../gstreamer/libgstjpeg.* ../gstreamer/libmfw_gst_mp3enc.* ../gstreamer/libgstvideorate.* ../gstreamer/libgstaudiorate.* ../gstreamer/libgstinterpipe.* ../gstreamer/libgstpretrigger.*"
/bin/cp $LIB_FILES $LIB_DIR
/bin/cp $GST_LIB_FILES $GST_LIB_DIR

#update gstreamer registry
/bin/cp ../gstreamer/registry.arm.bin RFS/.gstreamer-0.10/.
/bin/cp ../gstreamer/registry.arm.bin RFS/home/root/.gstreamer-0.10/.
/bin/cp -rf ../../lib/* RFS/lib

chmod +x RFS/usr/local/bin/*.sh

echo Creating SU image...

LIST="SU_image.tar.gz Signature pre_update.sh post_update.sh"
UCODE=`ls *.hex 2> /dev/null`
if [ "$UCODE" != "" ];then
   LIST="$LIST $UCODE"
fi

SCR=`ls boot.scr 2> /dev/null`
if [ "$?" = "0" ];then
   LIST="$LIST $SCR"
fi

cd RFS > /dev/null 2>&1
tar -Pzcf ../SU_image.tar.gz * .gst*
cd ..

./hashfile.host c SU_image.tar.gz Signature

echo Creating update file...
tar cvf BodyVision_$VERSION.tar $LIST extras --exclude .svn
md5sum BodyVision_$VERSION.tar > md5_$VERSION
