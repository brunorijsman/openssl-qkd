#! /bin/bash
source set-platform-dependent-variables.sh
rm -f server.out
${OPENSSL_BIN}/openssl s_server \
    -key key.pem \
    -cert cert.pem \
    -accept 44330 \
    -www \
    >server.out 2>&1 & \
    echo $! >server.pid
echo "Started server in background (PID `cat server.pid`)"
