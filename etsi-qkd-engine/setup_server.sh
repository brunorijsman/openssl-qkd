#!/bin/bash

export OPENSSL_ENGINES="/usr/local/lib/engine"

if [[ ! -L {$OPENSSL_ENGINES}/etsi_qkd_server.dylib ]]; then
    ln -s etsi_qkd_server.dylib {$OPENSSL_ENGINES}/etsi_qkd_server.dylib 
fi

if [[ ! -L {$OPENSSL_ENGINES}/etsi_qkd_client.dylib ]]; then
    ln -s etsi_qkd_client.dylib {$OPENSSL_ENGINES}/etsi_qkd_client.dylib 
fi

export DYLD_FALLBACK_LIBRARY_PATH="${HOME}/openssl/:."
export OPENSSL_CONF="${HOME}/ripe-quantum-hackathon-nov-2019/etsi-qkd-engine/openssl_server.cnf"
