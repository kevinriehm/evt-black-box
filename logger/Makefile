CFLAGS = -g -pg -Wall -Werror


all: angeld

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $+

angeld: angeld.o hmac_sha256.o
	$(CC) $(CFLAGS) -o $@ $+ -lsqlite3 -lm

