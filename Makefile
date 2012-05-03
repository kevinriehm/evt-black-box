CC     = arm-linux-gnueabi-gcc
CFLAGS = -g `cross-root/bin/sdl-config --cflags`

LIBS = `cross-root/bin/sdl-config --libs` -lfreetype -lSDL_ttf
OBJS = main.o clock.o draw.o draw_specs.o event.o

OUTPUTS = angel main.c clock.c draw.c event.c FreeSans.ttf

PANDAUSER = evt
PANDAADDR = `avahi-resolve-host-name -4 pandaboard.local | cut -f 2`

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
