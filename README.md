# Introduction

This is the source code of ZUMspot/MMDVM_HS firmware for personal hotspots (ADF7021 version of the MMDVM firmware), based on Jonathan G4KLX's [MMDVM](https://github.com/g4klx/MMDVM) software. This firmware supports D-Star, DMR, System Fusion, P25 and NXDN digital voice modes and POCSAG 1200 pager protocol.

This software is intended to be run on STM32F103 microcontroller. Also, Arduino with 3.3 V I/O (Arduino Due and Zero) and Teensy (3.1, 3.2, 3.5 or 3.6) are supported. You can build this code using Arduino IDE with Roger Clark's [STM32duino](https://github.com/rogerclarkmelbourne/Arduino_STM32/tree/ZUMspot) package, or using command line tools with ARM GCC tools. The preferred method under Windows is using STM32duino, and under Linux or macOS (command line) is using [STM32F10X_Lib](https://github.com/juribeparada/STM32F10X_Lib). 

This software comes with ABSOLUTELY NO WARRANTY, it is provided "AS IS" with the hope to be useful, use at your own risk. This firmware software is intended to be use into personal hotspots hardware, with a few mili-watts of RF power and short ranges (indoor). Because this is an amateur project doing by developers under their free time, and due to ADF7021 limitations, please DO NOT EXPECT high performance or full compliant to any digital voice standard of this firmware or any board based on this firmware.

This software is licenced under the GPL v2 and is intended for amateur and educational use only. Use of this software for commercial purposes is strictly forbidden.

# Features

- Supported modes: D-Star, DMR, Yaesu Fusion, P25 Phase 1 and NXDN
- Other modes: POCSAG 1200
- Automatic mode detection (scanning)
- G4KLX software suite: [MMDVMHost](https://github.com/g4klx/MMDVMHost), [ircDDBGateway](https://github.com/g4klx/ircDDBGateway), [YSFGateway](https://github.com/g4klx/YSFClients), [P25Gateway](https://github.com/g4klx/P25Clients), [DMRGateway](https://github.com/g4klx/DMRGateway), [NXDNGateway](https://github.com/g4klx/NXDNClients),
[DAPNETGateway](https://github.com/g4klx/DAPNETGateway) and [MMDVMCal](https://github.com/g4klx/MMDVMCal)
- Bands: 144, 220, 430 and 900 MHz (VHF requires external inductor)
- Status LEDs (PTT, COR and digital modes)
- Serial repeater port for Nextion displays
- Support for native USB port in STM32F103 CPU
- RSSI support
- CW ID support
- Full duplex support with two ADF7021

# Known issues

## Common issues for simplex and duplex boards

- High RX BER or not RX, poor TX audio (4FSK modes): adjust frequency offset, specially RXOffset.
- Not instantaneous mode detection (2 modes or more enabled): mode detection could be slow and sometimes you need to hold PTT several seconds, in order to activate the hotspot. There is no solution for that, since ADF7021 works only one mode at once. You can disable mode scanning, enabling just one mode.
- Poor audio with MD380: increase DMR deviation to 55 % or 60 %.
- Bad RX sensitivity: this is not a firmware issue, ADF7021 has a minimum signal detection around -109 dBm in DMR, but RX performance depends on RF board design, external RX noise, frequency, etc. At the moment only original ZUMspot RPi can reach the best RX sensitivity in 70 cm band.
- Not working with RPi 3B+ (USB): be sure your firmware version is >= 1.4.8 and update to new USB bootloader with long reset pulse (make stlink-bl, for example).
- Not working with USB (not RPi 3B+): compile with "LONG_USB_RESET" option disabled, and use old bootloader (make stlink-bl-old, for example).

## Duplex boards

- Very difficult DMR activation ("repeater fail" error): disable mode scanning, select just DMR mode.
- Not DMR activation ("repeater fail" error) with MD380, Ailunce HD1 or some other radios: increase DMR deviation to 55 % or 60 %.
- RX timeout: this is due to TX and RX clock differences, which does not have easy solution. Be sure your firmware version is >= 1.4.7, which minimizes this problem.

# Notes for previous users of MMDVM boards

MMDVM_HS boards do not need deviation calibration like MMDVM boards, but could be necessary some frequency offset calibration (ADF7021 does not have AFC for 4FSK modes).

The following options in MMDVM.ini ([Modem] section) have not any effect for MMDVM_HS boards:

    TXInvert
    RXInvert
    PTTInvert
    RXLevel
    RXDCOffset
    TXDCOffset

The following options in MMDVM.ini ([Modem] section) are very important for MMDVM_HS boards:

    RXOffset: RX frequency offset (HS RX BER improvement)
    TXOffset: TX frequency offset (radio RX improvement)
    TXLevel: default deviation setting (recommended value: 50)
    RFLevel: RF power output (recommended value: 100)
    CWIdTXLevel: CW ID deviation setting (recommended value: 50)
    D-StarTXLevel: D-Star deviation setting (recommended value: 50)
    DMRTXLevel: DMR deviation setting (recommended value: 50)
    YSFTXLevel: YSF deviation setting (recommended value: 50)
    P25TXLevel: P25 deviation setting (recommended value: 50)
    NXDNTXLevel: NXDN deviation setting (recommended value: 50)
    POCSAGTXLevel: POCSAG deviation setting (recommended value: 50)

# Important notes

The ADF7021 (or RF7021SE module) must operate with a 14.7456 MHz TCXO and with at least 2.5 ppm of frequency stability or better. For 800-900 MHz frequency band you will need even a better frequency stability TCXO. You could use also 12.2880 MHz TCXO. Any other TCXO frequency is not supported. Please note that a bad quality TCXO not only affects the frequency offset, also affects clock data rate, which is not possible to fix and will cause BER issues.

Please set TXLevel=50 in MMDVM.ini to configure default deviation levels for all modes. You could modify this value and other TXLevel paramenters to change deviation levels. Use [MMDVMCal](https://github.com/g4klx/MMDVMCal) to check DMR deviation level and TX frequency offset with calibrated test equipment.

The jumper wire to CLKOUT in RF7021SE module is not longer required for lastest MMDVM_HS firmware. But CE pin connection of ADF7021 is required for proper operation of ZUMspot.

Be aware that some Blue Pill STM32F103 board are defectives. If you have trouble with USB, please check this [link](http://wiki.stm32duino.com/index.php?title=Blue_Pill).

VHF (144-148 MHz) support for ZUMSpot is added by an external 18 nH inductor between L1 and L2 pins of ADF7021. This will enable dual band (VHF/UHF) operation.

Dual ADF7021 for full duplex operation (#define DUPLEX in Config.h) will work only with a big RX/TX frequency separation (5 MHz or more in UHF band for example) and proper antenna filtering. At the moment #define LIBRE_KIT_ADF7021 (Config.h) with STM32F103 platform is supported. Please see [BUILD.md](BUILD.md) for pinout details.

If you can't decode any 4FSK modulation (DMR, YSF, P25 or NXDN) with your ZUMspot, the common solution is to adjust RX frequency offset (RXOffset) in your MMDVM.ini file. Please try with steps of +-100 Hz until you get low BER. If you don't have test equipment, the procedure is trial and error. In some cases TXOffset adjustment is also required for proper radio decoding. If you have test equipment, please use [MMDVMCal](https://github.com/g4klx/MMDVMCal).

If you have problems updating firmware using USB bootloader (DFU mode) on Orange Pi or any other system different from RPi, you could compile the dfu tool directly. You could get the source code of a dfu tool [here](https://sourceforge.net/projects/dfu-programmer/files/dfu-programmer/0.7.0/).

# Quick start

Please see [BUILD.md](BUILD.md) for more details, and also [MMDVM](https://groups.yahoo.com/neo/groups/mmdvm/info) Yahoo Groups. You also can check at MMDVM_HS/scripts folder for some automatic tasks.

## Binary firmware installation

Please check the latest firmware [here](https://github.com/juribeparada/MMDVM_HS/releases).

### Pi-Star binary firmware installation

You could use some pi-star commands under SSH console:

- sudo pistar-zumspotflash rpi: ZUMspot RPi board
- sudo pistar-zumspotflash rpi_duplex: ZUMSpot duplex board conected to GPIO
- sudo pistar-zumspotflash usb: ZUMspot USB dongle
- sudo pistar-zumspotflash libre: ZUMspot Libre Kit or generic MMDVM_HS board with USB
- sudo pistar-mmdvmhshatflash hs_hat: MMDVM_HS_Hat board (14.7456MHz TCXO)
- sudo pistar-mmdvmhshatflash hs_dual_hat: HS_DUAL_HAT board (14.7456MHz TCXO)
- sudo pistar-mmdvmhshatflash hs_hat-12mhz: MMDVM_HS_Hat board (12.288MHz TCXO)
- sudo pistar-mmdvmhshatflash hs_dual_hat-12mhz: HS_DUAL_HAT board (12.288MHz TCXO)
- sudo pistar-nanohsflash nano_hs: Nano hotSPOT board
- sudo pistar-nanodvflash pi: NanoDV NPi board
- sudo pistar-nanodvflash usb: NanoDV USB board

### Windows

Download the ZUMspotFW firmware upgrade utility (ZUMspotFW_setup.exe) from the [releases section](https://github.com/juribeparada/MMDVM_HS/releases).

This utility includes firmwares binaries and USB drivers for Windows 7/8/10. If you have problems with the installer, you can download [ZUMspotFW.zip](https://github.com/juribeparada/MMDVM_HS/releases/download/v1.0.2/ZUMspotFW.zip) for a manual installation.

### Linux or macOS

Download the script (*.sh) that matches with your ZUMspot/MMDVM_HS board:

- install_fw_librekit.sh: only for ZUMspot Libre Kit board (KI6ZUM & VE2GZI) or generic MMDVM_HS board with USB interface
- install_fw_rpi.sh: only for ZUMspot RPi board (KI6ZUM & VE2GZI)
- install_fw_usb.sh: only for ZUMspot USB dongle (KI6ZUM)
- install_fw_duplex.sh: only for ZUMspot Duplex for RPi (KI6ZUM)
- install_fw_dualband.sh: only for ZUMspot Dualband for RPi (KI6ZUM)
- install_fw_hshat.sh: only for MMDVM_HS_Hat board (DB9MAT & DF2ET)
- install_fw_hshat-12mhz.sh: only for MMDVM_HS_Hat board with 12.288 MHz TCXO (DB9MAT & DF2ET)
- install_fw_hsdualhat.sh: only for MMDVM_HS_Dual_Hat board (DB9MAT & DF2ET & DO7EN)
- install_fw_hsdualhat-12mhz.sh: only for MMDVM_HS_Dual_Hat board with 12.288 MHz TCXO (DB9MAT & DF2ET & DO7EN)
- install_fw_nanohs.sh: only for Nano hotSPOT board (BI7JTA)
- install_fw_nanodv_npi.sh: only for NanoDV NPi board 1.0 (BG4TGO & BG5HHP)
- install_fw_nanodv_usb.sh: only for NanoDV USB board 1.0 (BG4TGO & BG5HHP)
- install_fw_d2rg_mmdvmhs.sh: only for D2RG MMDVM_HS board (BG3MDO, VE2GZI, CA6JAU)
- install_fw_gen_gpio.sh: only for generic MMDVM_HS board (EA7GIB) with GPIO serial interface
- install_fw_duplex_gpio.sh: only for MMDVM_HS with dual ADF7021 (EA7GIB) or generic dual ADF7021 board with GPIO serial interface
- install_fw_duplex_usb.sh: only for MMDVM_HS with dual ADF7021 (EA7GIB) or generic dual ADF7021 board with USB interface
- install_fw_skybridge_rpi.sh: only for BridgeCom SkyBridge HotSpot

For example, download the ZUMspot RPi upgrade script:

    cd ~
    curl -OL https://raw.github.com/juribeparada/MMDVM_HS/master/scripts/install_fw_rpi.sh

make the script executable:

    chmod +x install_fw_rpi.sh

stop your MMDVMHost process and run (you will need the root password):

    ./install_fw_rpi.sh

and wait to complete the upgrading process.

You could optionally install a beta firmware for testing:

    ./install_fw_rpi.sh beta

## Build from the sources

You could use example files from MMDVM_HS/configs folder and overwrite the Config.h file, in order to compile a firmware with default settings. There are a specific config file for each ZUMspot or any MMDVM_HS compatible boards. In general, there are two possible compilation ways:

- Boards with USB interface:

Boards with STM32_USB_HOST option enabled in Config.h (ZUMspot Libre Kit, ZUMspot USB, GIBSpot USB, NanoDV USB, etc). Those boards need to have the USB bootloader installed. For example:

    make clean
    make bl
    make dfu

You can optionally install a firmware without bootloader, with USB support. Be aware you will need always a ST-Link or serial interface in order to install or update the firmware. For this reason, you should use USB bootloader, unless you have trouble with the bootloader.

    make clean
    make nobl
    make stlink-nobl

- Boards with GPIO interface:

Boards with STM32_USART1_HOST option enabled in Config.h (ZUMspot RPi, MMDVM_HS_Hat, MMDVM_HS_Dual_Hat, Nano hotSPOT, etc). No USB bootloader needed. For example:

    make clean
    make
    make zumspot-pi

Some detailed examples as follows:

### MMDVM_HS_Hat

Please visit Mathis Schmieder GitHub [here](https://github.com/mathisschmieder/MMDVM_HS_Hat/blob/master/README.md) for detailed instructions.

### D2RG MMDVM_HS board

This hotspot needs Raspbian support for SPI-UART converter included in this board. If you don't see /dev/ttySC0, please visit Yuan BG3MDO GitHub [here](https://github.com/bg3mdo/D2RG_MMDVM_HS_ambe_uart_service) for driver installation instructions.

### ZUMspot Libre Kit (under Linux Raspbian)

If you are using Pi-Star, expand filesystem (if you haven't done before):

    sudo pistar-expand
    sudo reboot

Enable RW filesystem if you are using Pi-Star:

    rpi-rw

Install the necessary software tools:

    sudo apt-get update
    sudo apt-get install gcc-arm-none-eabi gdb-arm-none-eabi libstdc++-arm-none-eabi-newlib libnewlib-arm-none-eabi

Download the sources:

    cd ~
    git clone https://github.com/juribeparada/MMDVM_HS
    cd MMDVM_HS/
    git submodule init
    git submodule update

Build the firmware with USB bootloader support (the default Config.h is OK for Libre Kit):

    make bl

If you are using Pi-Star, stop services:

    sudo pistar-watchdog.service stop
    sudo systemctl stop mmdvmhost.timer
    sudo systemctl stop mmdvmhost.service

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

#### Build the firmware and upload to ZUMspot RPi

If you are using Pi-Star, expand filesystem (if you haven't done before):

    sudo pistar-expand
    sudo reboot

Enable RW filesystem if you are using Pi-Star:

    rpi-rw

Install the necessary software tools:

    sudo apt-get update
    sudo apt-get install gcc-arm-none-eabi gdb-arm-none-eabi libstdc++-arm-none-eabi-newlib libnewlib-arm-none-eabi
    
Note: The Pi-Star image contains an executable for stm32flash. The next five steps can be skipped!

    cd ~
    git clone https://git.code.sf.net/p/stm32flash/code stm32flash
    cd stm32flash
    make
    sudo make install

Download the firmware sources:

    cd ~
    git clone https://github.com/juribeparada/MMDVM_HS
    cd MMDVM_HS/
    git submodule init
    git submodule update

Edit Config.h

    nano Config.h
    
and enable:

    #define ZUMSPOT_ADF7021
    #define ENABLE_ADF7021
    #define ADF7021_14_7456
    #define AD7021_GAIN_AUTO
    #define STM32_USART1_HOST
    #define ENABLE_SCAN_MODE
    #define USE_ALTERNATE_NXDN_LEDS
    #define USE_ALTERNATE_POCSAG_LEDS

Build the firmware:

    make

If you are using Pi-Star, stop services:

    sudo pistar-watchdog.service stop
    sudo systemctl stop mmdvmhost.timer
    sudo systemctl stop mmdvmhost.service

Upload the firmware to ZUMspot RPi board:

    sudo make zumspot-pi

### MMDVM_HS_Dual_Hat\MMDVM_HS_Hat gpio firmware update on OrangePI PC/PC2/PC Plus

Install the necessary software tools:

    sudo apt-get update
    sudo apt-get install gcc-arm-none-eabi gdb-arm-none-eabi libstdc++-arm-none-eabi-newlib libnewlib-arm-none-eabi

Download the sources:

    cd ~
    git clone https://github.com/juribeparada/MMDVM_HS
    cd MMDVM_HS/
    git submodule init
    git submodule update

Install the tools for building firmware

    cd MMDVM_HS/scripts
    ./install_buildtools.sh
    
Copy MMDVM_HS/configs/MMDVM_HS_Dual_Hat.h or MMDVM_HS/configs/MMDVM_HS_Hat.h (MMDVM_HS_Hat-12mhz.h) to MMDVM_HS/Config.h and build the firmware:

    make

Stop the MMDVMHost services:

    sudo systemctl stop mmdvmhost.timer
    sudo systemctl stop mmdvmhost.service

Upload the firmware to mmdvm_hs_dual_hat (mmdvm_hs_hat) on OrangePi PC\PC2\PC Plus:

    sudo make mmdvm_hs_dual_hat_opi (sudo make mmdvm_hs_hat_opi)

or

    sudo /usr/local/bin/stm32flash -v -w bin/mmdvm_f1.bin -g 0x0 -R -i 198,-199,199:-198,199 /dev/ttyS3
    
    
