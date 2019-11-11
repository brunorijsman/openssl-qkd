#! /bin/bash
#
# stop_tshark.sh
#
# Stop tshark (WireShark for the Terminal) which was started using script start_tshark.sh.
# 
# (c) 2019 Bruno Rijsman, All Rights Reserved.
# See LICENSE for licensing information.
#
if [[ -f tshark.pid ]]; then
    OLD_TSHARK_PID=$(cat tshark.pid)
    if kill ${OLD_TSHARK_PID} >/dev/null 2>&1; then
        echo "Stopped tshark (PID ${OLD_TSHARK_PID})"
    fi
    rm -f tshark.pid
fi