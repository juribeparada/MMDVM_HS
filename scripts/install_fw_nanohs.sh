#!/bin/bash

#   Copyright (C) 2018 by Andy Uribe CA6JAU

#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

# Configure latest version
FW_VERSION="v1.3.7"

# Change USB-serial port name ONLY in macOS
MAC_DEV_USB_SER="/dev/cu.usbmodem1441"
	
# Download latest firmware for Nano hotSPOT
curl -OL https://github.com/juribeparada/MMDVM_HS/releases/download/$FW_VERSION/nano_hotspot_fw.bin

# Download STM32F10X_Lib (only for binary tools)
if [ ! -d "./STM32F10X_Lib/utils" ]; then
  git clone https://github.com/juribeparada/STM32F10X_Lib
fi

# Configure vars depending on OS
if [ $(uname -s) == "Linux" ]; then
	DEV_USB_SER="/dev/ttyACM0"
	if [ $(uname -m) == "x86_64" ]; then
		echo "Linux 64-bit detected"
		DFU_RST="./STM32F10X_Lib/utils/linux64/upload-reset"
		DFU_UTIL="./STM32F10X_Lib/utils/linux64/dfu-util"
		ST_FLASH="./STM32F10X_Lib/utils/linux64/st-flash"
		STM32FLASH="./STM32F10X_Lib/utils/linux64/stm32flash"
	elif [ $(uname -m) == "armv7l" ]; then
		echo "Raspberry Pi 3 detected"
		DFU_RST="./STM32F10X_Lib/utils/rpi32/upload-reset"
		DFU_UTIL="./STM32F10X_Lib/utils/rpi32/dfu-util"
		ST_FLASH="./STM32F10X_Lib/utils/rpi32/st-flash"
		STM32FLASH="./STM32F10X_Lib/utils/rpi32/stm32flash"
	elif [ $(uname -m) == "armv6l" ]; then
		echo "Raspberry Pi 2 or Pi Zero W detected"
		DFU_RST="./STM32F10X_Lib/utils/rpi32/upload-reset"
		DFU_UTIL="./STM32F10X_Lib/utils/rpi32/dfu-util"
		ST_FLASH="./STM32F10X_Lib/utils/rpi32/st-flash"
		STM32FLASH="./STM32F10X_Lib/utils/rpi32/stm32flash"
	else
		echo "Linux 32-bit detected"
		DFU_RST="./STM32F10X_Lib/utils/linux/upload-reset"
		DFU_UTIL="./STM32F10X_Lib/utils/linux/dfu-util"
		ST_FLASH="./STM32F10X_Lib/utils/linux/st-flash"
		STM32FLASH="./STM32F10X_Lib/utils/linux/stm32flash"
	fi
fi

if [ $(uname -s) == "Darwin" ]; then
	echo "macOS detected"
	DEV_USB_SER=$MAC_DEV_USB_SER
	DFU_RST="./STM32F10X_Lib/utils/macosx/upload-reset"
	DFU_UTIL="./STM32F10X_Lib/utils/macosx/dfu-util"
	ST_FLASH="./STM32F10X_Lib/utils/macosx/st-flash"
	STM32FLASH="./STM32F10X_Lib/utils/macosx/stm32flash"
fi

# Stop MMDVMHost process to free serial port
sudo killall MMDVMHost >/dev/null 2>&1

# Upload the firmware
eval sudo $STM32FLASH -v -w nano_hotspot_fw.bin -g 0x0 -R -i 200,-3,3:-200,3 /dev/ttyAMA0

