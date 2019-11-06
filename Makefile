UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux)
	@echo "Linux"
	CC = gcc
	SHARED_EXT = .so
else ifeq ($(UNAME_S), Darwin)
	CC = clang
	SHARED_EXT = .dylib
else
$(error Unsupported platform)
endif

OPENSLL ?= $(HOME)/openssl
OPENSSL_INCLUDE = $(OPENSLL)/include
OPENSSL_LIB = $(OPENSLL)

CFLAGS = -Wall -I. -I$(OPENSSL_INCLUDE) -L$(OPENSSL_LIB) -g -fPIC

all: etsi_qkd_client$(SHARED_EXT) etsi_qkd_server$(SHARED_EXT) 

etsi_qkd_client$(SHARED_EXT): etsi_qkd_client.c etsi_qkd_common.c qkd_api.c 
	$(LINK.c) -shared -o $@ $^ -lcrypto

etsi_qkd_server$(SHARED_EXT): etsi_qkd_server.c etsi_qkd_common.c qkd_api.c 
	$(LINK.c) -shared -o $@ $^ -lcrypto

key_a key_b:
	date > key_a
	date > key_b

clean:
	rm -f etsi_qkd_client$(SHARED_EXT) etsi_qkd_server$(SHARED_EXT) *.o core
	rm -f
	rm -rf *.dSYM

.PHONY: all keys clean
