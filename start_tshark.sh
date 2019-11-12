#! /bin/bash
#
# start_tshark.sh
#
# Start tshark (WireShark for the Terminal) running in the background and capture TLS traffic
# on port 44330 into a text file (tshark.out) and into a pcap file (tshark.pcap) that can be viewed
# using WireShark.
# 
# (c) 2019 Bruno Rijsman, All Rights Reserved.
# See LICENSE for licensing information.
#
echo -n "Starting tshark in background... "
source set_platform_dependent_variables.sh
rm -f tshark.out
echo "tshark started on $(date +'%Y-%m-%dT%H:%M:%S.%s')" >tshark.out
${MAYBE_SUDO} tshark \
    -i ${LOOPBACK} \
    -w tshark.pcap \
    -V \
    -d tcp.port==44330,tls \
    "tcp port 44330" \
    >>tshark.out 2>&1 & \
    echo $! >tshark.pid
echo "OK (PID `cat tshark.pid`)"
