INSTALL_DIR = /opt/staevt.com/angel

CFLAGS = -g -pg -Wall -Werror


all: angeld

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $+

angeld: angeld.o hmac_sha256.o
	$(CC) $(CFLAGS) -o $@ $+ -lsqlite3 -lm

install: all
	mkdir -p /opt/bin
	mkdir -p $(INSTALL_DIR)
	
	cp angeld $(INSTALL_DIR)
	ln -s -f $(INSTALL_DIR)/angeld /opt/bin/angeld

