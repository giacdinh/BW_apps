#!/bin/sh

DATE=`date +'%Y-%m-%d %H:%M:%S'`
LOG=/odi/log/status_`date +%Y%m%d`.log
echo "[INFO] ${DATE} power suspense called" >> ${LOG}
sync
# prepare sleep command
#echo enabled > /sys/devices/platform/imx-uart.2/tty/ttymxc2/power/wakeup
echo enabled > /sys/class/tty/ttymxc2/power/wakeup
sleep 1
# now
echo mem > /sys/power/state
echo "[INFO] ${DATE} set time after power resume"  >> ${LOG}
/sbin/hwclock --hctosys > /dev/null 2>&1
