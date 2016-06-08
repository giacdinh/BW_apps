#!/bin/bash
if [ ! -s /odi/log/hw_version.txt ] ; then 
  HW_VER=1
else
  HW_VER=`cat /odi/log/hw_version.txt |cut -c 1`
fi
IMG=`ls /odi/upload/BodyVision*`
FW_VER=`basename ${IMG} | awk -F_  '{printf("%s\n",$3)}'`
HW_V3=3
echo "hardware version: $HW_VER"
echo "firmware version: $FW_VER"
LOGDATE=`date -I | cut -c 1-4,6,7,9,10`
if [ $HW_VER -lt $HW_V3 ] ; then
	echo "Try to update on HW version < 3"
	if [ $FW_VER -le 2 ] ; then
	    echo "Update FW"
	else
	    echo "Don't update and remove all update files"
	    echo "Update effort was stopped. HW:$HW_VER FW:$FW_VER" >> /odi/log/status_$LOGDATE.log
	    rm -rf /odi/upload/*
	    reboot 
	    sleep 30 
	fi
else
	echo "Try to update on HW version >= 3"
        if [ $FW_VER -ge $HW_V3 ] ; then
            echo "FW update"
        else
            echo "Don't update and remove all update files"
	    echo "Update effort was stopped. HW:$HW_VER FW:$FW_VER" >> /odi/log/status_$LOGDATE.log
	    rm -rf /odi/upload/*
	    reboot 
	    sleep 30 
	fi
fi
