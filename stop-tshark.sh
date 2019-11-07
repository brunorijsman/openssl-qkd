#!/bin/bash
if [[ -f tshark.pid ]]; then
    OLD_TSHARK_PID=$(cat tshark.pid)
    if kill ${OLD_TSHARK_PID} >/dev/null 2>&1; then
        echo "Stopped tshark (PID ${OLD_TSHARK_PID})"
    fi
    rm -f tshark.pid
fi