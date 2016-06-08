#!/bin/sh

#Some variables
RELEASE_DIR=$PWD
ROOT=../RFS
LIB_DIR=${ROOT}/usr/lib/
LIB_FILES="libgstaudio-0.10.* libffi.* libgstapp-0.10.so.0.25.0"

GST_LIB_DIR=${ROOT}/usr/lib/gstreamer-0.10/
GST_LIB_FILES="libgstcoreelements.* libgstmatroska.* libgstmultifile.* libgstjpeg.* libmfw_gst_mp3enc.* libgstvideorate.* libgstaudiorate.* libgstdelay.* libgstinterpipe.* libgstpretrigger.*"

#Install release content
echo "Installing..."
cd $RELEASE_DIR

#Install /usr/lib/ content
cp $LIB_FILES $LIB_DIR

#Install /usr/lib/gstreamer-0.10/ content
cp $GST_LIB_FILES $GST_LIB_DIR
