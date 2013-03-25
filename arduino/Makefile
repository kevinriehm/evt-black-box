ARDUINOROOT = /usr/share/arduino
ARDUINOCORE = corelib

# Board-specific definitions

ifeq '$(BOARD)' 'mega2560'
MCU        = atmega2560
F_CPU      = 16000000L
BAUD       = 115200
PROGRAMMER = stk500v2
PORT       = $(shell ls /dev/ttyACM* | head -n 1)
VARIANT    = mega
endif

ifeq '$(BOARD)' 'uno'
MCU        = atmega328p
F_CPU      = 16000000L
BAUD       = 115200
PROGRAMMER = arduino
PORT       = $(shell ls /dev/ttyUSB* | head -n 1)
VARIANT    = standard
endif

ifndef MCU
$(error BOARD not set to a valid value (one of mega2560, uno))
endif

# Source code and flags

CORECSRC   = $(wildcard $(ARDUINOCORE)/*.c)
CORECXXSRC = $(wildcard $(ARDUINOCORE)/*.cpp)
COREOBJS   = $(CORECSRC:.c=.o) $(CORECXXSRC:.cpp=.o)

COMMONFLAGS = -I$(ARDUINOCORE) -I/usr/lib/avr/include -I. \
	-I$(ARDUINOROOT)/hardware/arduino/variants/$(VARIANT) -mmcu=$(MCU) \
	-DF_CPU=$(F_CPU) -Os -ffunction-sections -fdata-sections -Lcorelib -L. \
	-Wl,--gc-sections -Wno-write-strings

DAEMONTRGT   = daemon.hex
DAEMONHEAD   = adc.h com.h gps.h i2c.h pot.h spi.h twi.h Wire.h
DAEMONCSRC   = twi.c
DAEMONCXXSRC = adc.cpp com.cpp daemon.cpp gps.cpp i2c.cpp pot.cpp spi.cpp \
	Wire.cpp
DAEMONOBJS   = $(DAEMONCSRC:.c=.o) $(DAEMONCXXSRC:.cpp=.o)

LIBS = -larduinocore -ltinygps

# Compiler and tools

AR       = avr-ar
CC       = avr-gcc
CFLAGS   = $(COMMONFLAGS)
CXX      = avr-g++
CXXFLAGS = $(COMMONFLAGS)
OBJCOPY  = avr-objcopy

SENDTARGET = $(DAEMONTRGT)


# Targets

.PHONY: reset send

all: $(DAEMONTRGT)

%.o: $(ARDUINOCORE)/libarduinocore.a libtinygps.a

%.hex: %.o
	$(CXX) $(CXXFLAGS) -o $@.tmp $< $(LIBS)
	$(OBJCOPY) -O ihex $@.tmp $@
	$(RM) $@.tmp

$(DAEMONTRGT): $(DAEMONOBJS) $(DAEMONHEAD) $(ARDUINOCORE)/libarduinocore.a libtinygps.a
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

