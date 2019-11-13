UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
CC = gcc
SHARED_EXT = .so
SHARED_PATH_ENV = LD_LIBRARY_PATH
MAYBE_SUDO = sudo
else ifeq ($(UNAME_S), Darwin)
CC = clang
SHARED_EXT = .dylib
SHARED_PATH_ENV = DYLD_FALLBACK_LIBRARY_PATH
MAYBE_SUDO = 
else
$(error Unsupported platform)
endif

OPENSSL ?= $(HOME)/openssl
OPENSSL_INCLUDE = $(OPENSSL)/include
OPENSSL_LIB = $(OPENSSL)
OPENSSL_BIN = $(OPENSSL)/apps
ENGINE_DIR = /usr/local/lib/engines-3

CFLAGS = -Wall -Werror -I. -I$(OPENSSL_INCLUDE) -L$(OPENSSL_LIB) -g -fPIC

CLIENT = qkd_engine_client$(SHARED_EXT)
SERVER = qkd_engine_server$(SHARED_EXT)

all: $(CLIENT) $(SERVER) key.pem cert.pem $(ENGINE_DIR)/$(CLIENT) $(ENGINE_DIR)/$(SERVER)

MOCK_API_C = qkd_api_common.c qkd_api_mock.c
MOCK_API_H = qkd_api.h

CLIENT_C = qkd_engine_client.c qkd_engine_common.c qkd_debug.c $(MOCK_API_C)
CLIENT_H = qkd_engine_common.h $(MOCK_API_H)
$(CLIENT): $(CLIENT_C) $(CLIENT_H)
	$(LINK.c) -shared -o $@ $(CLIENT_C) -lcrypto

SERVER_C = qkd_engine_server.c qkd_engine_common.c qkd_debug.c $(MOCK_API_C)
SERVER_H = qkd_engine_common.h $(MOCK_API_H)
$(SERVER): $(SERVER_C) $(SERVER_H)
	$(LINK.c) -shared -o $@ $(SERVER_C) -lcrypto

key.pem cert.pem:
	$(SHARED_PATH_ENV)=${HOME}/openssl \
		$(OPENSSL_BIN)/openssl req \
		-x509 \
		-newkey rsa:2048 \
		-keyout key.pem \
		-out cert.pem \
		-days 365 \
		-nodes \
		-subj /O=Example \
		-config certificate_openssl.cnf

$(ENGINE_DIR)/$(CLIENT): $(CLIENT)
	$(MAYBE_SUDO) mkdir -p $(ENGINE_DIR)
	$(MAYBE_SUDO) ln -sf ${CURDIR}/$(CLIENT) $(ENGINE_DIR)/$(CLIENT)

$(ENGINE_DIR)/$(SERVER): $(SERVER)
	$(MAYBE_SUDO) mkdir -p $(ENGINE_DIR)
	$(MAYBE_SUDO) ln -sf ${CURDIR}/$(SERVER) $(ENGINE_DIR)/$(SERVER)

mock-test:
	./run_mock_test.sh

test: all mock-test

clean: clean-test
	$(MAYBE_SUDO) rm -rf $(ENGINE_DIR)/$(CLIENT) $(ENGINE_DIR)/$(SERVER)
	$(MAYBE_SUDO) rm -f $(ENGINE_DIR)/$(CLIENT)
	$(MAYBE_SUDO) rm -f $(ENGINE_DIR)/$(SERVER)
	rm -f $(CLIENT) $(SERVER)
	rm -f key.pem cert.pem
	rm -f *.o core
	rm -rf *.dSYM

clean-test:
	./stop_server.sh
	./stop_tshark.sh
	rm -f *.out
	rm -f *.pid
	rm -f *.pcap

.PHONY: all keys test mock-test clean clean-test
