OPENSSL_INCLUDE=$(HOME)/openssl/include
OPENSSL_LIB=$(HOME)/openssl

CC = clang
CFLAGS = -I. -I$(OPENSSL_INCLUDE) -L$(OPENSSL_LIB) -g

diffie-hellman: diffie-hellman.c
	$(CC) -Wall -o $@ $< $(CFLAGS) -lcrypto

.PHONY: clean

clean:
	rm -f diffie-hellman *.o core