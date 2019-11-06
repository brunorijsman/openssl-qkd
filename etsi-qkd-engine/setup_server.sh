#!/bin/bash

if [[ ! -L /usr/local/lib/engine/etsi_qkd_server.dylib ]]; then
    ln -s etsi_qkd_server.dylib /usr/local/lib/engine/etsi_qkd_server.dylib 
fi

if [[ ! -L /usr/local/lib/engine/etsi_qkd_client.dylib ]]; then
    ln -s etsi_qkd_client.dylib /usr/local/lib/engine/etsi_qkd_client.dylib 
fi

export DYLD_FALLBACK_LIBRARY_PATH="${HOME}/openssl/:."
export OPENSSL_CONF="${HOME}/ripe-quantum-hackathon-nov-2019/etsi-qkd-engine/openssl_server.cnf"
export OPENSSL_ENGINES="/usr/local/lib/engine"
