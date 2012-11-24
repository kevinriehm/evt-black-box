CC     = arm-linux-gnueabi-gcc
CFLAGS = -g -Wall -Icross-root/include `cross-root/bin/curl-config --cflags` \
	$(EXTRACFLAGS)

CSRC = main.c data.c display.c event.c gui.c hmac_sha256.c log.c scheduler.c \
	serial.c
LSRC = pil.l
YSRC = pil.y

LIBS := `cross-root/bin/curl-config --libs` -lm
OBJS := $(CSRC:.c=.o) $(LSRC:.l=.yy.o) $(YSRC:.y=.tab.o)

OUTPUTS = angel $(SRC) FreeSans.ttf

PANDAUSER = evt
PANDAADDR = pandaboard


.PHONY: send run

all: angel
	echo $(OBJS)

angel: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c angel.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Lex files
%.yy.c: %.l %.tab.h
	$(LEX) -t $< > $@

# Yacc files
%.tab.c %.tab.h: %.y
	$(YACC) -d -p $* -b $* $<

send: $(OUTPUTS)
	scp $(OUTPUTS) $(PANDAUSER)@$(PANDAADDR):~/pandacode

run:
	ssh -X $(PANDAUSER)@$(PANDAADDR) ~/pandacode/angel
