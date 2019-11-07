#! /bin/bash
if [[ -f server.pid ]]; then
    OLD_SERVER_PID=$(cat server.pid)
    if kill ${OLD_SERVER_PID} >/dev/null 2>&1; then
        echo "Killed server (PID ${OLD_SERVER_PID})"
    fi
    rm -f server.pid
fi