OPENSSL_INCLUDE=$(HOME)/openssl/include
OPENSSL_LIB=$(HOME)/openssl

CC = clang
CFLAGS = -Wall -I. -I$(OPENSSL_INCLUDE) -L$(OPENSSL_LIB) -g

all: etsi_qkd_client.dylib etsi_qkd_server.dylib

etsi_qkd_client.dylib: etsi_qkd_client.c etsi_qkd_common.c qkd_api.c 
	$(LINK.c) -shared -o $@ $^ -lcrypto

etsi_qkd_server.dylib: etsi_qkd_server.c etsi_qkd_common.c qkd_api.c 
	$(LINK.c) -shared -o $@ $^ -lcrypto

clean:
	rm -f etsi_qkd_client.dylib etsi_qkd_server.dylib *.o core
	rm -rf *.dSYM

.PHONY: all clean
