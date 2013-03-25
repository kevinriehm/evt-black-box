ROOT = cross-root

CC     = arm-linux-gnueabi-gcc
CFLAGS = -g -Wall -Wno-parentheses -Wno-unused-function -Icross-root/include \
	$(EXTRACFLAGS)

CSRC = main.c aux.c display.c event.c gui.c libs.c
LSRC = pil.l
YSRC = pil.y

LIBS = -ldl
OBJS = $(CSRC:.c=.o) $(LSRC:.l=.yy.o) $(YSRC:.y=.tab.o)

OUTPUTS = angel $(SRC) FreeSans.ttf

PANDAUSER = evt
PANDAADDR = pandaboard


.PHONY: send run

all: angel

angel: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
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

