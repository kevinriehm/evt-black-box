ARDUINOROOT = /usr/share/arduino
ARDUINOCORE = corelib

CORECSRC   := $(wildcard $(ARDUINOCORE)/*.c)
CORECXXSRC := $(wildcard $(ARDUINOCORE)/*.cpp)
COREOBJS   := $(CORECSRC:.c=.o) $(CORECXXSRC:.cpp=.o)

COMMONFLAGS = -I$(ARDUINOCORE) -I/usr/lib/avr/include \
	-I$(ARDUINOROOT)/hardware/arduino/variants/$(VARIANT) -mmcu=$(MCU) \
	-DF_CPU=$(F_CPU) -Os -ffunction-sections -fdata-sections -Lcorelib \
	-Wl,--gc-sections

AR       = avr-ar
CC       = avr-gcc
CFLAGS   = $(COMMONFLAGS)
CXX      = avr-g++
CXXFLAGS = $(COMMONFLAGS)
OBJCOPY  = avr-objcopy

PANDAUSER = evt
PANDAADDR = pandaboard

# Board-specific definitions
MCU        = atmega2560
F_CPU      = 16000000L
BAUD       = 115200
PROGRAMMER = stk500v2
PORT      := $(shell ssh $(PANDAUSER)@$(PANDAADDR) ls /dev/ttyACM* | head -n 1)
VARIANT    = mega

SENDTARGET := sensors.hex


.PHONY: reset send

all: serial_test.hex potenti_test.hex

%.hex: %.o $(ARDUINOCORE)/libarduinocore.a
	$(CXX) $(CXXFLAGS) -o $@.tmp $^ -larduinocore
	$(OBJCOPY) -O ihex $@.tmp $@
	$(RM) $@.tmp

$(ARDUINOCORE)/libarduinocore.a: $(COREOBJS)
	$(AR) rcs $@ $^

reset:
	ssh $(PANDAUSER)@$(PANDAADDR) stty -F $(PORT) hup
	ssh $(PANDAUSER)@$(PANDAADDR) stty -F $(PORT) -hup

send: $(SENDTARGET) reset
	ssh $(PANDAUSER)@$(PANDAADDR) avrdude -q -q -V -p $(MCU) -c $(PROGRAMMER) -b $(BAUD) -P $(PORT) -U flash:w:-:i < $<
