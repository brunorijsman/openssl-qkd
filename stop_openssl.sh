#! /bin/bash
#
# stop_server.sh
#
# Sometimes, if the QKD server or client crashes, the OpenSSL process is not properly terminated
# and continues to run in the background. This causes subsequent test runs to fail since the
# lingering instance of OpenSSL is still bound to TCP ports. This script kills any lingering
# instance of OpenSSL.
# 
# (c) 2019 Bruno Rijsman, All Rights Reserved.
# See LICENSE for licensing information.
#
echo -n "Stopping lingering OpenSSL... "
PID=$(lsof -nP -i4TCP:8999 | grep LISTEN | awk -F ' ' '{print $2}')
if [[ -z "$PID" ]]; then
    echo "OK, did not find lingering OpenSSL"
else
    if kill ${PID} >/dev/null 2>&1; then
        echo "OK, stopped openssl (PID ${OLD_SERVER_PID})"
    else
        echo "FAIL, kill failed (PID ${OLD_SERVER_PID})"
    fi
fi