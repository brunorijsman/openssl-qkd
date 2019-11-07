#! /bin/bash
echo -n "Running client... "
source set-platform-dependent-variables.sh
rm -f client.out
echo "client started on $(date +'%Y-%m-%dT%H:%M:%S.%s')" >client.out
export OPENSSL_CONF=client_openssl.cnf
echo "GET /" | \
    ${OPENSSL_BIN}/openssl s_client \
    -tls1_2 \
    -cipher 'DHE-RSA-AES128-GCM-SHA256' \
    -connect localhost:44330 \
    -CAfile cert.pem \
    -msg \
    >>client.out 2>&1
if [[ $? -eq 0 ]]; then
    echo "OK"
else
    echo "FAILED"
fi