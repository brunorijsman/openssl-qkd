OPENSSL_INCLUDE=$(HOME)/openssl/include
OPENSSL_LIB=$(HOME)/openssl

CC = clang
CFLAGS = -Wall -I. -I$(OPENSSL_INCLUDE) -L$(OPENSSL_LIB) -g

all: libexample_engine.so test_example_engine

example.dylib: example_engine.c qkd_api.c 
	$(LINK.c) -shared -o $@ $^ -lcrypto

test_example_engine: test_example_engine.c example.dylib
	$(CC) -o $@ $< $(CFLAGS) -lcrypto example.dylib

clean:
	rm -f example.dylib test_example_engine *.o core
	rm -rf *.dSYM

.PHONY: all clean
