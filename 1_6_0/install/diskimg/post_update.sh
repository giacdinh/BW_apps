#!/bin/bash

cp /odi/upload/upgrade/extras/m2mqtt* /odi/conf
cp /odi/upload/upgrade/extras/config.conf /odi/conf
/odi/upload/upgrade/extras/mC_update /odi/upload/upgrade/extras/release_uC_hex > /odi/log/u-code-update 2>&1
