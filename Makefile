CC = gcc
CFLAGS = -I. -g

diffie-hellman: diffie-hellman.c
	$(CC) -o $@ $< $(CFLAGS)

.PHONY: clean

clean:
	rm -f diffie-hellman *.o core