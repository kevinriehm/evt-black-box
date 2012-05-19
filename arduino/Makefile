ARDUINOROOT = /usr/share/arduino
ARDUINOCORE = corelib

CORECSRC   = $(wildcard $(ARDUINOCORE)/*.c)
CORECXXSRC = $(wildcard $(ARDUINOCORE)/*.cpp)
COREOBJS   = $(CORECSRC:.c=.o) $(CORECXXSRC:.cpp=.o)

# Board-specific definitions
MCU     = atmega2560
F_CPU   = 16000000L
VARIANT = mega

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

SENDTARGET = serial_test


.PHONY: send
.SUFFIXES: .c .cpp .o

all: serial_test.hex

serial_test.hex: serial_test.o $(ARDUINOCORE)/libarduinocore.a
	$(CXX) $(CXXFLAGS) -o $@.tmp $^ -larduinocore
	$(OBJCOPY) -O ihex $@.tmp $@
	$(RM) $@.tmp

$(ARDUINOCORE)/libarduinocore.a: $(COREOBJS)
	$(AR) rcs $@ $^

send: $(SENDTARGET)
	
