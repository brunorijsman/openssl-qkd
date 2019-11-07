#! /bin/bash

# Keep this consistent with the equivalent code in Makefile

# Development environment dependencies
export OPENSSL=${HOME}/openssl

# Helper variables
export OPENSSL_INCLUDE=${OPENSSL}/include
export OPENSSL_LIB=${OPENSSL}
export OPENSSL_BIN=${OPENSSL}/apps

# Platform dependencies
UNAME_S=$(uname -s)
if [[ ${UNAME_S} == "Linux" ]]; then
    export SHARED_EXT=.so
    export LD_LIBRARY_PATH="${OPENSSL}:${LD_LIBRARY_PATH}"    
    export ENGINE_DIR=/usr/local/lib/engines-3
elif [[ ${UNAME_S} == "Darwin" ]]; then
    export SHARED_EXT=.dylib
    export DYLD_FALLBACK_LIBRARY_PATH=${OPENSSL}
    export ENGINE_DIR=/usr/local/lib/engine
else
    echo "Unsupported platform" 1>&2
    exit 1
fi

