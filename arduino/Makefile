ARDUINOROOT = /usr/share/arduino
ARDUINOCORE = corelib

CORECSRC   = $(wildcard $(ARDUINOCORE)/*.c)
CORECXXSRC = $(wildcard $(ARDUINOCORE)/*.cpp)
COREOBJS   = $(CORECSRC:.c=.o) $(CORECXXSRC:.cpp=.o)

COMMONFLAGS = -I$(ARDUINOCORE) -I/usr/lib/avr/include \
	-I$(ARDUINOROOT)/hardware/arduino/variants/$(VARIANT) -mmcu=$(MCU) \
	-DF_CPU=$(F_CPU) -Os -ffunction-sections -fdata-sections -Lcorelib -L. \
	-Wl,--gc-sections

DAEMONTRGT   = daemon.hex
DAEMONHEAD   = com.h
DAEMONCXXSRC = daemon.cpp com.cpp
DAEMONOBJS   = $(DAEMONCXXSRC:.cpp=.o)

LIBS = -larduinocore -ltinygps

AR       = avr-ar
CC       = avr-gcc
CFLAGS   = $(COMMONFLAGS)
CXX      = avr-g++
CXXFLAGS = $(COMMONFLAGS)
OBJCOPY  = avr-objcopy

# Board-specific definitions
MCU        = atmega2560
F_CPU      = 16000000L
BAUD       = 115200
PROGRAMMER = stk500v2
PORT       = $(shell ls /dev/ttyACM* | head -n 1)
VARIANT    = mega

SENDTARGET = $(DAEMONTRGT)


.PHONY: reset send

all: $(DAEMONTRGT)

%.o: $(ARDUINOCORE)/libarduinocore.a libtinygps.a

%.hex: %.o
	$(CXX) $(CXXFLAGS) -o $@.tmp $< $(LIBS)
	$(OBJCOPY) -O ihex $@.tmp $@
	$(RM) $@.tmp

$(DAEMONTRGT): $(DAEMONOBJS) $(DAEMONHEAD)
	$(CXX) $(CXXFLAGS) -o $@.tmp $(DAEMONOBJS) $(LIBS)
	$(OBJCOPY) -O ihex $@.tmp $@
	$(RM) $@.tmp

$(ARDUINOCORE)/libarduinocore.a: $(COREOBJS)
	$(AR) rcs $@ $^

libtinygps.a: TinyGPS.o
	$(AR) rcs $@ $^

reset:
	stty -F $(PORT) hup
	stty -F $(PORT) -hup

send: $(SENDTARGET) reset
	avrdude -q -q -V -p $(MCU) -c $(PROGRAMMER) -b $(BAUD) -P $(PORT) -U flash:w:-:i < $<
