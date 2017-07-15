# Introduction

This is the source code of ZUMSpot, personal hotspot (ADF7021 version of the MMDVM firmware), based on Jonathan G4KLX's MMDVM software. This firmware supports D-Star, DMR, System Fusion and P25 digital modes.

This software is intended to be run on STM32F103 microcontroller. You can build this code using Arduino IDE with STM32duino package, or using "make" with ARM GCC tools. Also, Arduino with 3.3 V I/O (Arduino Due and Zero) and Teensy (3.1, 3.2, 3.5 or 3.6) are supported.

This software is licenced under the GPL v2 and is intended for amateur and educational use only. Use of this software for commercial purposes is strictly forbidden.

# Quick start

Please see BUILD.md for more details, and also MMDVM Yahoo Groups.

## ZUMSpot Libre Kit (under Linux Raspbian)

Install the necessary software tools:

    sudo apt-get update
    sudo apt-get install gcc-arm-none-eabi gdb-arm-none-eabi libstdc++-arm-none-eabi-newlib libnewlib-arm-none-eabi

Download the sources:

    cd ~
    git clone https://github.com/juribeparada/MMDVM_HS
    cd MMDVM_HS/
    git clone https://github.com/juribeparada/STM32F10X_Lib

Build the firmware with USB bootloader support (the default Config.h is OK for Libre Kit):

    make bl

Upload the firmware to ZUMSpot Libre Kit using the USB port (your Libre Kit has the bootloader already installed):

    sudo make dfu devser=/dev/ttyACM0

## ZUMSpot RPi

### Enable serial port in Raspberry Pi 3 or Pi Zero W

This this necessary only if you are installing a fresh copy of Raspbian OS.

Edit /boot/cmdline.txt:

    sudo nano /boot/cmdline.txt
    (remove the text: console=serial0,115200)

Disable services:

    sudo systemctl disable serial-getty@ttyAMA0.service
    sudo systemctl disable bluetooth.service

Edit /boot/config.txt

    sudo nano /boot/config.txt

and add the following lines at the end of /boot/config.txt:

    enable_uart=1
    dtoverlay=pi3-disable-bt

Reboot your RPi:

    sudo reboot

### Build de firmware and upload to ZUMSpot RPi

Install the necessary software tools:

    sudo apt-get update
    sudo apt-get install gcc-arm-none-eabi gdb-arm-none-eabi libstdc++-arm-none-eabi-newlib libnewlib-arm-none-eabi
    
    cd ~
    git clone https://git.code.sf.net/p/stm32flash/code stm32flash
    cd stm32flash
    make
    sudo make install

Download the firmware sources:

    cd ~
    git clone https://github.com/juribeparada/MMDVM_HS
    cd MMDVM_HS/
    git clone https://github.com/juribeparada/STM32F10X_Lib

Edit Config.h

    nano Config.h
    
and enable:

    #define PI_HAT_7021_REV_03
    #define ENABLE_ADF7021
    #define BIDIR_DATA_PIN
    #define ADF7021_14_7456
    #define STM32_USART1_HOST
    #define ENABLE_SCAN_MODE

Build the firmware:

    make

Upload the firmware to ZUMSpot RPi board:

    sudo make zumspot-pi

