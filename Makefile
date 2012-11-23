CC     = arm-linux-gnueabi-gcc
CFLAGS = -g -Wall `cross-root/bin/sdl-config --cflags` -Icross-root/include \
	`cross-root/bin/freetype-config --cflags` `cross-root/bin/curl-config --cflags` \
	-DGLYPH_TARGET=GLYPH_TARGET_SDL $(EXTRACFLAGS)

CSRC = main.c analog_sensor.c clock.c data.c draw.c draw_specs.c event.c \
	hmac_sha256.c log.c serial.c
LSRC = pil.l
YSRC = pil.y

LIBS := `cross-root/bin/sdl-config --libs` `cross-root/bin/curl-config --libs` \
	-lm -lfreetype -lSDL_gfx
OBJS := $(CSRC:.c=.o) $(LSRC:.l=.yy.o) $(YSRC:.y=.tab.o) glyph-keeper/glyph.o

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
