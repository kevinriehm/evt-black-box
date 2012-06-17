CC     = arm-linux-gnueabi-gcc
CFLAGS = -g -Wall `cross-root/bin/sdl-config --cflags` -Icross-root/include \
	`cross-root/bin/freetype-config --cflags` `cross-root/bin/curl-config --cflags` \
	-DGLYPH_TARGET=GLYPH_TARGET_SDL $(EXTRACFLAGS)

SRC = main.c analog_sensor.c clock.c draw.c draw_specs.c event.c hmac_sha256.c serial.c

LIBS := `cross-root/bin/sdl-config --libs` `cross-root/bin/curl-config --libs` \
	-lm -lfreetype -lSDL_gfx
OBJS := $(SRC:.c=.o) glyph-keeper/glyph.o

OUTPUTS = angel $(SRC) FreeSans.ttf

PANDAUSER = evt
PANDAADDR = pandaboard


.PHONY: send run

all: angel

angel: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c angel.h
	$(CC) $(CFLAGS) -c -o $@ $<

send: $(OUTPUTS)
	scp $(OUTPUTS) $(PANDAUSER)@$(PANDAADDR):~/pandacode

run:
	ssh -X $(PANDAUSER)@$(PANDAADDR) ~/pandacode/angel
