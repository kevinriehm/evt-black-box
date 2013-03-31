CFLAGS = -g -pg -Wall -Wno-parentheses -Wno-unused-function

CSRC = angel.c aux.c car.c display.c event.c font.c gui.c serial.c
LSRC = pil.l
YSRC = pil.y

LIBS = -lOpenVG -lEGL -X11 -lm -lfreetype
OBJS = $(CSRC:.c=.o) $(LSRC:.l=.yy.o) $(YSRC:.y=.tab.o)


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

