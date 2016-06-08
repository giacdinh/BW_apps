#!/bin/bash
NETUP=`dmesg |grep "ready" | tail -1 |grep "becomes ready"`

if [ -z "$NETUP" ]; then
        echo "0" > /odi/log/net_state
else
        echo "1" > /odi/log/net_state
fi
