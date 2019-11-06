#!/bin/bash

export DYLD_FALLBACK_LIBRARY_PATH="${HOME}/openssl/:."
export OPENSSL_CONF="${HOME}/ripe-quantum-hackathon-nov-2019/etsi-qkd-engine/openssl_server.cnf"
export OPENSSL_ENGINES="/usr/local/lib/engines-3"
