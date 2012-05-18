CC     = arm-linux-gnueabi-gcc
CFLAGS = -g -Wall `cross-root/bin/sdl-config --cflags` -Icross-root/include \
	`cross-root/bin/freetype-config --cflags` -DGLYPH_TARGET=GLYPH_TARGET_SDL

LIBS = `cross-root/bin/sdl-config --libs` -lm -lfreetype -lSDL_gfx
OBJS = main.o clock.o draw.o draw_specs.o event.o glyph-keeper/glyph.o

OUTPUTS = angel main.c clock.c draw.c event.c FreeSans.ttf

PANDAUSER = evt
PANDAADDR = pandaboard

all: angel

angel: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c angel.h
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: send run

send: $(OUTPUTS)
	scp $(OUTPUTS) $(PANDAUSER)@$(PANDAADDR):~/pandacode

run:
	ssh -X $(PANDAUSER)@$(PANDAADDR) ~/pandacode/angel
