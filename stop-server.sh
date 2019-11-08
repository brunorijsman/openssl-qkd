#! /bin/bash
#
# stop-server.sh
#
# A bash script for stopping the OpenSSL demonstration web server that was started using script
# start-server.sh.
# 
# (c) 2019 Bruno Rijsman, All Rights Reserved.
# See LICENSE for licensing information.
#
if [[ -f server.pid ]]; then
    OLD_SERVER_PID=$(cat server.pid)
    if kill ${OLD_SERVER_PID} >/dev/null 2>&1; then
        echo "Stopped server (PID ${OLD_SERVER_PID})"
    fi
    rm -f server.pid
fi