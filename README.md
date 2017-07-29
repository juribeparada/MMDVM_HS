# Introduction

This is the source code of ZUMspot, personal hotspot (ADF7021 version of the MMDVM firmware), based on Jonathan G4KLX's MMDVM software. This firmware supports D-Star, DMR, System Fusion and P25 digital modes.

This software is intended to be run on STM32F103 microcontroller. You can build this code using Arduino IDE with STM32duino package, or using "make" with ARM GCC tools. Also, Arduino with 3.3 V I/O (Arduino Due and Zero) and Teensy (3.1, 3.2, 3.5 or 3.6) are supported.

This software is licenced under the GPL v2 and is intended for amateur and educational use only. Use of this software for commercial purposes is strictly forbidden.

# Features

- Supported modes: D-Star, DMR, Yaesu Fusion and P25 Phase 1
- Automatic mode detection (scanning)
- G4KLX software suite: MMDVMHost, ircDDBGateway, YSFGateway, P25Gateway and DMRGateway
- Bands: 144, 220, 430 and 900 MHz (VHF requires external inductor)
- Status LEDs (PTT, COR and digital modes)
- Serial repeater port for Nextion displays
- Support for native USB port in STM32F103 CPU
- RSSI support
- Full duplex support with two ADF7021 (experimental)

# Important notes

The ADF7021 (or RF7021SE module) must operate with a 14.7456 MHz TCXO and with at least 2 ppm of frequency stability. You could use also 12.2880 MHz TCXO, but this frequency configuration has less testing. Any other TCXO frequency is not supported.

The jumper wire to CLKOUT in RF7021SE module is not longer required for lastest MMDVM_HS firmware. But CE pin connection of ADF7021 is required for proper operation of ZUMspot.

Be aware that some Blue Pill STM32F103 board are defectives. If you have trouble with USB, please check this: http://wiki.stm32duino.com/index.php?title=Blue_Pill

VHF (144-148 MHz) support for ZUMSpot is added by an external 18 nH inductor between L1 and L2 pins of ADF7021.

Dual ADF7021 for full duplex operation (#define DUPLEX in Config.h) will work only with a big RX/TX frequency separation (5 MHz or more in UHF band for example) and proper antenna filtering. At the moment #define ADF7021_CARRIER_BOARD (Config.h) with STM32F103 platform is supported. Please see BUILD.md for pinout details.

If you can't decode any 4FSK modulation (DMR, YSF and P25) with your ZUMspot, the common solution is to adjust RX frequency offset (RXOffset) in your MMDVM.ini file. Please try with steps of +-100 Hz until you get low BER. If you don't have test equipment, the only procedure is trial and error. In some cases TXOffset adjustment is also required for proper radio decoding.

# Quick start

Please see BUILD.md for more details, and also MMDVM Yahoo Groups.

## Binary firmware installation

Please check for latest firmware:

    https://github.com/juribeparada/MMDVM_HS/releases

### Linux or macOS

Download the script (*.sh) that match your ZUMspot board, for automatic firmware upgrade process:

- install_fw_rpi.sh: only for ZUMspot RPi board
- install_fw_librekit.sh: only for ZUMspot Libre Kit board or generic ZUMspot board
- install_fw_usb.sh: only for ZUMspot USB dongle
- install_fw_duplex.sh: only for ZUMspot with dual ADF7021

make the script executable (for example for ZUMspot RPi):

    chmod +x install_fw_rpi.sh

stop your MMDVMHost process and run (you will need the root password):

    ./install_fw_rpi.sh

and wait to complete the upgrading process.

## Build from the sources

### ZUMspot Libre Kit (under Linux Raspbian)

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

Upload the firmware to ZUMspot Libre Kit using the USB port (your Libre Kit has the bootloader already installed):

    sudo make dfu devser=/dev/ttyACM0

### ZUMspot RPi

#### Enable serial port in Raspberry Pi 3 or Pi Zero W

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

#### Build de firmware and upload to ZUMspot RPi

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
    #define ADF7021_14_7456
    #define STM32_USART1_HOST
    #define ENABLE_SCAN_MODE

Build the firmware:

    make

Upload the firmware to ZUMspot RPi board:

    sudo make zumspot-pi

