#! /bin/bash
source set-platform-dependent-variables.sh
rm -f server.out
echo "server started on $(date +'%Y-%m-%dT%H:%M:%S.%s')" >server.out
export OPENSSL_CONF=server_openssl.cnf
echo $OPENSSL_CONF
${OPENSSL_BIN}/openssl s_server \
    -key key.pem \
    -cert cert.pem \
    -accept 44330 \
    -www \
    -engine etsi_qkd_server \
    >>server.out 2>&1 & \
    echo $! >server.pid
echo "Started server in background (PID `cat server.pid`)"
