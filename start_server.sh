#! /bin/bash
#
# start_server.sh
#
# A bash script for starting the OpenSSL demonstration web server using the ETSI QKD API for QKD
# key agreement. The server servers HTTP GET requests from the OpenSSL demonstration web client
# (see run_client.sh). The output of the server is written to server.out.
# 
# (c) 2019 Bruno Rijsman, All Rights Reserved.
# See LICENSE for licensing information.
#
echo -n "Starting server in background... "
source set_platform_dependent_variables.sh
rm -f server.out
echo "server started on $(date +'%Y-%m-%dT%H:%M:%S.%s')" >server.out
export OPENSSL_CONF=server_openssl.cnf
${OPENSSL_BIN}/openssl s_server \
    -key key.pem \
    -cert cert.pem \
    -accept 44330 \
    -www \
    >>server.out 2>&1 & \
    echo $! >server.pid
echo "OK (PID `cat server.pid`)"
