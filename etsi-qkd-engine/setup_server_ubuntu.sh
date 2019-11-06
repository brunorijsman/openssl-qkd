#!/bin/bash

export OPENSSL_ENGINES="/usr/local/lib/engines-3"

if [[ ! -L ${OPENSSL_ENGINES}/etsi_qkd_server.so ]]; then
    ln -s etsi_qkd_server.dylib ${OPENSSL_ENGINES}/etsi_qkd_server.so 
fi

if [[ ! -L ${OPENSSL_ENGINES}/etsi_qkd_client.so ]]; then
    ln -s etsi_qkd_client.dylib ${OPENSSL_ENGINES}/etsi_qkd_client.so
fi

export LD_LIBRARY_PATH="$HOME/openssl/:."
export OPENSSL_CONF="/workspaces/ripe-quantum-hackathon-nov-2019/etsi-qkd-engine/openssl_server.cnf"
