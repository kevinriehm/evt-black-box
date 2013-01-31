ROOT = cross-root

CC     = arm-linux-gnueabi-gcc
CFLAGS = -g -Wall -Wno-parentheses -Wno-unused-function -Icross-root/include \
	`$(ROOT)/bin/curl-config --cflags 2> /dev/null` $(EXTRACFLAGS)

CSRC = main.c data.c display.c event.c gui.c hmac_sha256.c log.c scheduler.c \
	serial.c
LSRC = pil.l
YSRC = pil.y

LIBS := `$(ROOT)/bin/curl-config --libs 2> /dev/null` -lm -lrt -lpthread -lX11 \
	-lEGL -lOpenVG
OBJS := $(CSRC:.c=.o) $(LSRC:.l=.yy.o) $(YSRC:.y=.tab.o)

OUTPUTS = angel $(SRC) FreeSans.ttf

PANDAUSER = evt
PANDAADDR = pandaboard


.PHONY: send run

all: angel angeld

angel: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c angel.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Lex files
%.yy.c: %.l %.tab.h
	$(LEX) -t $< | sed s/yyin/pilin/g > $@

# Yacc files
%.tab.c %.tab.h: %.y
	$(YACC) -v -d -p $* -b $* $<

send: $(OUTPUTS)
	scp $(OUTPUTS) $(PANDAUSER)@$(PANDAADDR):~/pandacode

run:
	ssh -X $(PANDAUSER)@$(PANDAADDR) ~/pandacode/angel

angeld:
	$(MAKE) -C monitor angeld

