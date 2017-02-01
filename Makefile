#  Copyright (C) 2016 by Andy Uribe CA6JAU
#  Copyright (C) 2016 by Jim McLaughlin KI6ZUM

#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.

#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.

#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

# GNU ARM Embedded Toolchain
CC=arm-none-eabi-gcc
CXX=arm-none-eabi-g++
LD=arm-none-eabi-ld
AR=arm-none-eabi-ar
AS=arm-none-eabi-as
CP=arm-none-eabi-objcopy
OD=arm-none-eabi-objdump
NM=arm-none-eabi-nm
SIZE=arm-none-eabi-size
A2L=arm-none-eabi-addr2line

# Directory Structure
BINDIR=bin

# Find source files
# "SystemRoot" is only defined in Windows
ifdef SYSTEMROOT
	ASOURCES=$(shell dir /S /B *.s)
	CSOURCES=$(shell dir /S /B *.c)
	CXXSOURCES=$(shell dir /S /B *.cpp)
	CLEANCMD=del /S *.o *.hex *.bin *.elf
	MDBIN=md $@
else ifdef SystemRoot
	ASOURCES=$(shell dir /S /B *.s)
	CSOURCES=$(shell dir /S /B *.c)
	CXXSOURCES=$(shell dir /S /B *.cpp)
	CLEANCMD=del /S *.o *.hex *.bin *.elf
	MDBIN=md $@
else
	ASOURCES=$(shell find . -name '*.s')
	CSOURCES=$(shell find . -name '*.c')
	CXXSOURCES=$(shell find . -name '*.cpp')
	CLEANCMD=rm -f $(OBJECTS) $(BINDIR)/$(BINELF) $(BINDIR)/$(BINHEX) $(BINDIR)/$(BINBIN)
	MDBIN=mkdir $@
endif

# Default reference oscillator frequencies
ifndef $(OSC)
	OSC=8000000
endif

# Find header directories
INC= . STM32F10X_Lib/CMSIS/ STM32F10X_Lib/Device/ STM32F10X_Lib/STM32F10x_StdPeriph_Driver/inc/ STM32F10X_Lib/usb/inc/
INCLUDES=$(INC:%=-I%)

# Find libraries
INCLUDES_LIBS=
LINK_LIBS=

# Create object list
OBJECTS=$(ASOURCES:%.s=%.o)
OBJECTS+=$(CSOURCES:%.c=%.o)
OBJECTS+=$(CXXSOURCES:%.cpp=%.o)

# Define output files ELF & IHEX
BINELF=outp.elf
BINHEX=outp.hex
BINBIN=outp.bin

# MCU FLAGS
MCFLAGS=-mcpu=cortex-m3 -mthumb -Wall

# COMPILE FLAGS
DEFS_HS=-DUSE_STDPERIPH_DRIVER -DSTM32F10X_MD -DHSE_VALUE=$(OSC)

CFLAGS=-c $(MCFLAGS) $(INCLUDES)
CXXFLAGS=-c $(MCFLAGS) $(INCLUDES)

# LINKER FLAGS
LDSCRIPT=stm32f10x_link.ld
LDFLAGS =-T $(LDSCRIPT) $(MCFLAGS) --specs=nosys.specs $(INCLUDES_LIBS) $(LINK_LIBS)

# Build Rules
.PHONY: all release hs debug clean

all: hs

hs: CFLAGS+=$(DEFS_HS) -Os -ffunction-sections -fdata-sections -fno-builtin -Wno-implicit -DCUSTOM_NEW -DNO_EXCEPTIONS
hs: CXXFLAGS+=$(DEFS_HS) -Os -fno-exceptions -ffunction-sections -fdata-sections -fno-builtin -fno-rtti -DCUSTOM_NEW -DNO_EXCEPTIONS
hs: LDFLAGS+=-Os --specs=nano.specs
hs: release

debug: CFLAGS+=-g
debug: CXXFLAGS+=-g
debug: LDFLAGS+=-g
debug: release

release: $(BINDIR)
release: $(BINDIR)/$(BINHEX)
release: $(BINDIR)/$(BINBIN)

$(BINDIR):
	$(MDBIN)

$(BINDIR)/$(BINHEX): $(BINDIR)/$(BINELF)
	$(CP) -O ihex $< $@
	@echo "Objcopy from ELF to IHEX complete!\n"
	
$(BINDIR)/$(BINBIN): $(BINDIR)/$(BINELF)
	$(CP) -O binary $< $@
	@echo "Objcopy from ELF to BINARY complete!\n"

$(BINDIR)/$(BINELF): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	@echo "Linking complete!\n"
	$(SIZE) $(BINDIR)/$(BINELF)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@
	@echo "Compiled "$<"!\n"

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@
	@echo "Compiled "$<"!\n"

%.o: %.s
	$(CC) $(CFLAGS) $< -o $@
	@echo "Assambled "$<"!\n"

clean:
	$(CLEANCMD)
	
stlink:
ifneq ($(wildcard /usr/bin/openocd),)
	/usr/bin/openocd -f /usr/share/openocd/scripts/interface/stlink-v2-1.cfg -f /usr/share/openocd/scripts/target/stm32f1x.cfg -c "program bin/$(BINELF) verify reset exit"
endif

ifneq ($(wildcard /usr/local/bin/openocd),)
	/usr/local/bin/openocd -f /usr/local/share/openocd/scripts/interface/stlink-v2-1.cfg -f /usr/local/share/openocd/scripts/target/stm32f1x.cfg -c "program bin/$(BINELF) verify reset exit"
endif

ifneq ($(wildcard /opt/openocd/bin/openocd),)
	/opt/openocd/bin/openocd -f /opt/openocd/scripts/interface/stlink-v2-1.cfg -f /opt/openocd/share/openocd/scripts/target/stm32f1x.cfg -c "program bin/$(BINELF) verify reset exit"
endif

serial:
ifneq ($(wildcard /usr/local/bin/stm32flash),)
	/usr/local/bin/stm32flash -w bin/$(BINBIN) -g 0x0 $(devser)
endif
