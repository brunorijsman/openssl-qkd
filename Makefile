UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
	@echo "Linux"
	CC = gcc
	SHARED_EXT = .so
	SHARED_PATH_ENV = LD_LIBRARY_PATH
else ifeq ($(UNAME_S), Darwin)
	CC = clang
	SHARED_EXT = .dylib
	SHARED_PATH_ENV = DYLD_FALLBACK_LIBRARY_PATH
else
$(error Unsupported platform)
endif

OPENSSL ?= $(HOME)/openssl
OPENSSL_INCLUDE = $(OPENSSL)/include
OPENSSL_LIB = $(OPENSSL)
OPENSSL_BIN = $(OPENSSL)/apps
ENGINE_DIR = /usr/local/lib/engines-3

CFLAGS = -Wall -I. -I$(OPENSSL_INCLUDE) -L$(OPENSSL_LIB) -g -fPIC

CLIENT = etsi_qkd_client$(SHARED_EXT)
SERVER = etsi_qkd_server$(SHARED_EXT)

all: $(CLIENT) $(SERVER) key.pem cert.pem $(ENGINE_DIR)/$(CLIENT) $(ENGINE_DIR)/$(SERVER)

$(CLIENT): etsi_qkd_client.c etsi_qkd_common.c qkd_api.c 
	$(LINK.c) -shared -o $@ $^ -lcrypto

$(SERVER): etsi_qkd_server.c etsi_qkd_common.c qkd_api.c 
	$(LINK.c) -shared -o $@ $^ -lcrypto

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
	mkdir -p $(ENGINE_DIR)
	ln -sf ${CURDIR}/$(CLIENT) $(ENGINE_DIR)/$(CLIENT)

$(ENGINE_DIR)/$(SERVER): $(SERVER)
	mkdir -p $(ENGINE_DIR)
	ln -sf ${CURDIR}/$(SERVER) $(ENGINE_DIR)/$(SERVER)

test: all
	@./stop-server.sh
	@./start-tshark.sh
	@sleep 1
	@./start-server.sh
	@./run-client.sh
	@./stop-server.sh
	@sleep 1
	@./stop-tshark.sh

clean: clean-test
	rm -f $(CLIENT) $(SERVER)
	rm -rf $(ENGINE_DIR)/$(CLIENT) $(ENGINE_DIR)/$(SERVER)
	rm -f key.pem cert.pem
	rm -f *.o core
	rm -f $(ENGINE_DIR)/$(CLIENT)
	rm -f $(ENGINE_DIR)/$(SERVER)
	rm -rf *.dSYM

clean-test:
	./stop-server.sh
	./stop-tshark.sh
	rm -f *.out
	rm -f *.pid
	rm -f *.tcap

.PHONY: all keys test clean clean-test
