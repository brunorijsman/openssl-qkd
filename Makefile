UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
	@echo "Linux"
	CC = gcc
	SHARED_EXT = .so
	SHARED_PATH_ENV = LD_LIBRARY_PATH
	ENGINE_DIR = /usr/local/lib/engines-3
else ifeq ($(UNAME_S), Darwin)
	CC = clang
	SHARED_EXT = .dylib
	SHARED_PATH_ENV = DYLD_FALLBACK_LIBRARY_PATH
	ENGINE_DIR = /usr/local/lib/engine
else
$(error Unsupported platform)
endif

OPENSLL ?= $(HOME)/openssl
OPENSSL_INCLUDE = $(OPENSLL)/include
OPENSSL_LIB = $(OPENSLL)
OPENSSL_BIN = $(OPENSLL)/apps

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
		-config server_cert_ssl_config.cnf

$(ENGINE_DIR)/$(CLIENT): $(CLIENT)
	mkdir -p $(ENGINE_DIR)
	cp $(CLIENT) $(ENGINE_DIR)

$(ENGINE_DIR)/$(SERVER): $(SERVER)
	mkdir -p $(ENGINE_DIR)
	cp $(SERVER) $(ENGINE_DIR)

clean:
	rm -f $(CLIENT) $(SERVER)
	rm -rf $(ENGINE_DIR)/$(CLIENT) $(ENGINE_DIR)/$(SERVER)
	rm -f key.pem cert.pem
	rm -f *.o core
	rm -rf *.dSYM

.PHONY: all keys clean
