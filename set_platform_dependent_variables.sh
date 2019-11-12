#! /bin/bash
#
# set_platform_dependent_variables.sh
#
# A bash script that sets environment variables to account for the differences between the two
# supported development environments: Apple MacOS and Ubuntu Linux.
# 
# (c) 2019 Bruno Rijsman, All Rights Reserved.
# See LICENSE for licensing information.
#

# Keep this consistent with the equivalent code in Makefile

# Development environment dependencies
export OPENSSL=${HOME}/openssl

# Helper variables
export OPENSSL_INCLUDE=${OPENSSL}/include
export OPENSSL_LIB=${OPENSSL}
export OPENSSL_BIN=${OPENSSL}/apps
export ENGINE_DIR=/usr/local/lib/engines-3

# Platform dependencies
UNAME_S=$(uname -s)
if [[ ${UNAME_S} == "Linux" ]]; then
    export SHARED_EXT=.so
    export LD_LIBRARY_PATH="${ENGINE_DIR}:${OPENSSL}:${LD_LIBRARY_PATH}"
    export MAYBE_SUDO="sudo"
    export LOOPBACK="lo"
elif [[ ${UNAME_S} == "Darwin" ]]; then
    export SHARED_EXT=.dylib
    export DYLD_FALLBACK_LIBRARY_PATH="${ENGINE_DIR}:${OPENSSL}:${DYLD_FALLBACK_LIBRARY_PATH}"
    export MAYBE_SUDO=""
    export LOOPBACK="lo0"
else
    echo "Unsupported platform" 1>&2
    exit 1
fi

