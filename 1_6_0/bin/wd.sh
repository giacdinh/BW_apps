#!/bin/bash

rm -f /tmp/wd_mon /tmp/wd_upload
sync
sleep 1
MON=`ps |grep "bin/monitor" | wc -l`
UPLOAD=`ps |grep "file_upload" | wc -l`

echo $MON > /tmp/wd_mon
echo $UPLOAD > /tmp/wd_upload
