#!/bin/bash
today=$(date +"%Y%m%d")
tail -f /odi/log/status_$today.log
