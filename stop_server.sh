#! /bin/bash
#
# stop_server.sh
#
# A bash script for stopping the OpenSSL demonstration web server that was started using script
# start_server.sh.
# 
# (c) 2019 Bruno Rijsman, All Rights Reserved.
# See LICENSE for licensing information.
#
echo -n "Stopping server in background... "
if [[ -f server.pid ]]; then
    OLD_SERVER_PID=$(cat server.pid)
    if kill ${OLD_SERVER_PID} >/dev/null 2>&1; then
        echo "OK, stopped server (PID ${OLD_SERVER_PID})"
    else
        echo "FAIL, kill failed (PID ${OLD_SERVER_PID})"
    fi
    rm -f server.pid
else
    echo "OK, server was not running"
fi