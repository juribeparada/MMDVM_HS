# Building instructions

This is a detailed guide for building the firmware of MMDVM_HS from the source code. For quick instructions, please see [README.md](README.md). This software runs on STM32F103 microcontroller. Also, Arduino with 3.3 V I/O (Arduino Due and Zero) and Teensy (3.1, 3.2, 3.5 or 3.6) are supported. You can build this code using Arduino IDE with Roger Clark's [STM32duino](https://github.com/rogerclarkmelbourne/Arduino_STM32/tree/ZUMspot) package, or using command line tools with ARM GCC tools. The preferred method under Windows is using STM32duino, and under Linux or macOS (command line) is using [STM32F10X_Lib](https://github.com/juribeparada/STM32F10X_Lib).

# Index

- ZUMspot RPi
- ZUMspot Libre Kit
- ZUMspot USB
- MMDVM_HS_Hat
- Makefile options
- Config.h options
- Pinout definitions
- Hidden options
 
# ZUMspot RPi

Download latest Raspbian image and install to a micro SD

* See: https://www.raspberrypi.org/documentation/installation/installing-images/
* Configure your SD before booting. Useful article: https://styxit.com/2017/03/14/headless-raspberry-setup.html

Boot your Raspberry Pi. Run raspi-config and configure according to your preferences:

    sudo raspi-config

Select at least:

* Expand filesystem
* Change default password
* Enable "Wait for Network at Boot"
* Disable Desktop GUI if you don't plan to use it

### Enable serial port in Raspberry Pi 3 or Pi Zero W

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

### Enable serial port in Raspberry Pi 2

Edit /boot/cmdline.txt:

    sudo nano /boot/cmdline.txt
    (remove the text: console=serial0,115200)

Disable the service:

    sudo systemctl disable serial-getty@ttyAMA0.service

Reboot your RPi:

    sudo reboot

### Build the firmware and upload to ZUMspot RPi

If you are using Pi-Star, expand filesystem (if you haven't done before):

    sudo pistar-expand
    sudo reboot

Enable RW filesystem if you are using Pi-Star:

    rpi-rw

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
    git submodule init
    git submodule update

(Please do not download any different code inside MMDVM_HS folder)

Edit Config.h

    nano Config.h
    
and enable:

    #define ZUMSPOT_ADF7021
    #define ENABLE_ADF7021
    #define ADF7021_14_7456
    #define STM32_USART1_HOST
    #define ENABLE_SCAN_MODE

Build the firmware:

    make

If you are using Pi-Star, stop services:

    sudo pistar-watchdog.service stop
    sudo systemctl stop mmdvmhost.timer
    sudo systemctl stop mmdvmhost.service

Upload the firmware to ZUMspot RPi board:

    sudo make zumspot-pi

Install MMDVMHost:

    cd ~
    git clone https://github.com/g4klx/MMDVMHost/
    cd MMDVMHost/
    make

Edit MMDVM.ini according your preferences

    nano MMDVM.ini
    (use Port=/dev/ttyAMA0 in [Modem])

Execute MMDVMHost:

    ./MMDVMHost MMDVM.ini

# ZUMspot Libre Kit

## Windows with Arduino IDE

Download and install the Arduino IDE:

    https://www.arduino.cc/en/Main/Software

Run Arduino IDE. On the Tools menu, select the Boards manager, and install the "Arduino SAM" from the list of available boards.

Download STM32duino (Arduino for STM32) from this URL:

    https://github.com/rogerclarkmelbourne/Arduino_STM32/tree/ZUMspot

Unzip and change the extracted folder name "Arduino_STM32-ZUMspot" to "Arduino_STM32"

Copy Arduino_STM32 folder in:

    My Documents/Arduino/hardware

Install the USB bootloader to STM32F103. Follow the instructions:

    https://github.com/rogerclarkmelbourne/Arduino_STM32/wiki/stm32duino-bootloader

Connect the ZUMspot Libre Kit to your PC. Install the USB Mapple driver using the bat file (you may also check http://wiki.stm32duino.com/index.php?title=Windows_driver_installation):

    My Documents/Arduino/hardware/Arduino_STM32/drivers/win/install_drivers.bat

You have to be sure that Windows detect your ZUMspot as an USB serial device COMx (please see Windows Device Manager).

Download the source (zip file) of MMDVM_HS from:

    https://github.com/juribeparada/MMDVM_HS

Do not download or install the STM32F103 library (STM32F10X_Lib) this is not necessary under STM32duino.

Unzip MMDVM_HS-master.zip and change the folder name to "MMDVM_HS". The path name to this folder can't have spaces.

Start the Arduino IDE. Open the MMDVM_HS.ino file in the MMDVM_HS folder.

Under the menu "Tools" select "Board" and then select:

    Board: Generic STM32F103C Series
    Variant: STM32F103C8 (20k RAM, 64k Flash)
    CPU Speed: 72 MHz (Normal)
    Upload method: STM32duino bootloader (you have transfered the USB bootloader before)
    Serial port: COMx (Maple Mini)

Edit Config.h:

    #define LIBRE_KIT_ADF7021
    #define ENABLE_ADF7021
    #define ADF7021_14_7456
    #define STM32_USB_HOST
    #define ENABLE_SCAN_MODE

Click the Upload button in the IDE and wait for the transfer.

Once the transfer is completed, press the RESET button of the board or disconnect and connect the USB cable. You will see the LED (PC13) of the blue pill blinking. Once you connect with MMDVMHost, the LED will blink fast.

For further help with STM32duino and STM32F103 blue pill boards, please see the STM32duino [forum](http://www.stm32duino.com).

## Windows with command line

Download the source code (zip file) of MMDVM_HS from:

    https://github.com/juribeparada/MMDVM_HS

Unzip MMDVM_HS-master.zip and change the folder name to "MMDVM_HS". The path name to this folder can't have spaces.

Download the ST libraries STM32F10X_Lib-master.zip from:

    https://github.com/juribeparada/STM32F10X_Lib/

Extract the STM32F10X_Lib-master folder into the same folder as the MMDVM_HS. Change the folder name to "STM32F10X_Lib".

Download the GNU make utility:

    http://gnuwin32.sourceforge.net/packages/make.htm
    
Download the binaries zip file and extract make.exe and put it in the same directory MMDVM_HS. Download the dependencies zip file and extract libintl3.dll and libiconv2.dll and put them in the same directory MMDVM_HS.

Download the GNU ARM embedded toolchain from here:

    https://launchpad.net/gcc-arm-embedded/+download

Currently the direct link to the installer is here:
    https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q3-update/+download/gcc-arm-none-eabi-5_4-2016q3-20160926-win32.exe

Download STM32duino (Arduino for STM32) from this URL (only for USB drivers):

    https://github.com/rogerclarkmelbourne/Arduino_STM32/tree/ZUMspot

Unzip and copy Arduino_STM32-ZUMspot folder in (for example):

    C:\Arduino_STM32-ZUMspot

Connect the ZUMspot Libre Kit to your PC. Install the USB Mapple driver using the bat file (you may also check http://wiki.stm32duino.com/index.php?title=Windows_driver_installation): 

    C:\Arduino_STM32-ZUMspot\drivers\win\install_drivers.bat
    
Once the USB driver is installed, you may delete "C:\Arduino_STM32-ZUMspot" folder.

Launch the "GCC Command Prompt" from "GNU Tools for ARM Embedded Processors" (Start Menu) and
cd to the folder where you put the MMDVM_HS folder.

Edit Config.h according your preferences. The default Config.h is OK for ZUMSpot Libre Kit.

Build the firmware:

    make clean
    make bl
    
Press the reset button of ZUMspot and upload the firmware:

    make dfu

## Linux Raspbian

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

(Please do not download any different code inside MMDVM_HS folder)

Edit Config.h:

    nano Config.h

and enable:

    #define LIBRE_KIT_ADF7021
    #define ENABLE_ADF7021
    #define ADF7021_14_7456
    #define STM32_USB_HOST
    #define ENABLE_SCAN_MODE

Build the firmware with bootloader support:

    make bl

If you are using Pi-Star, stop services:

    sudo pistar-watchdog.service stop
    sudo systemctl stop mmdvmhost.timer
    sudo systemctl stop mmdvmhost.service

Upload bootloader and firmware to ZUMspot Libre Kit, using serial port first (you are using an USB-serial converter with device name /dev/ttyUSB0). Move BOOT0 jumper to 1, next press and release RESET and execute:

    sudo make serial-bl devser=/dev/ttyUSB0

Move BOOT0 jumper to 0.

For following firmware updates, you could use the USB port directly:

    sudo make dfu devser=/dev/ttyACM0

Install MMDVMHost:

    cd ~
    git clone https://github.com/g4klx/MMDVMHost/
    cd MMDVMHost/
    make

Edit MMDVM.ini according your preferences

    nano MMDVM.ini
    (use Port=/dev/ttyACM0 in [Modem])

Execute MMDVMHost:

    ./MMDVMHost MMDVM.ini
    
# ZUMspot USB

## Windows

Download and install the Arduino IDE:

    https://www.arduino.cc/en/Main/Software

Run Arduino IDE. On the Tools menu, select the Boards manager, and install the "Arduino SAM" from the list of available boards.

Download STM32duino (Arduino for STM32) from this URL:

    https://github.com/rogerclarkmelbourne/Arduino_STM32/tree/ZUMspot

Unzip and change the extracted folder name "Arduino_STM32-ZUMspot" to "Arduino_STM32"

Copy Arduino_STM32 folder in:

    My Documents/Arduino/hardware

Connect the ZUMspot USB to your PC. Install the USB Mapple driver using the bat file: 

    My Documents/Arduino/hardware/Arduino_STM32/drivers/win/install_drivers.bat
    (you may also check: http://wiki.stm32duino.com/index.php?title=Windows_driver_installation)

You have to be sure that Windows detect your ZUMspot as an USB serial device COMx (please
see Windows Device Manager)

Download the source (zip file) of MMDVM_HS from:

    https://github.com/juribeparada/MMDVM_HS

Do not download or install the STM32F103 library, STM32F10X_Lib, this is not necessary
under STM32duino.

Unzip MMDVM_HS-master.zip and change the folder name to "MMDVM_HS". The path name to this
folder can't have spaces.

Start the Arduino IDE. Open the MMDVM_HS.ino file in the MMDVM_HS folder.

Under the menu "Tools" select "Board" and then select:

    Board: Generic STM32F103C Series
    Variant: STM32F103C8 (20k RAM, 64k Flash)
    CPU Speed: 72 MHz (Normal)
    Upload method: STM32duino bootloader (you have transfered the USB bootloader before)
    Serial port: COMx (Maple Mini)

Edit Config.h:

    #define ZUMSPOT_ADF7021
    #define ENABLE_ADF7021
    #define ADF7021_14_7456
    #define STM32_USB_HOST
    #define ENABLE_SCAN_MODE

Click the Upload button in the IDE and wait for the transfer.

Once the transfer is completed, press the RESET button of the board or disconnect and
connect the USB cable.

## Linux Raspbian

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

(Please do not download any different code inside MMDVM_HS folder)

Edit Config.h

    nano Config.h

and enable:

    #define ZUMSPOT_ADF7021
    #define ENABLE_ADF7021
    #define ADF7021_14_7456
    #define STM32_USB_HOST
    #define ENABLE_SCAN_MODE

Build the firmware with bootloader support:

    make bl

If you are using Pi-Star, stop services:

    sudo pistar-watchdog.service stop
    sudo systemctl stop mmdvmhost.timer
    sudo systemctl stop mmdvmhost.service

Upload the firmware to ZUMspot USB:

    sudo make dfu devser=/dev/ttyACM0

Install MMDVMHost:

    cd ~
    git clone https://github.com/g4klx/MMDVMHost/
    cd MMDVMHost/
    make

Edit MMDVM.ini according your preferences

    nano MMDVM.ini
    (use Port=/dev/ttyACM0 in [Modem])

Execute MMDVMHost:

    ./MMDVMHost MMDVM.ini
    
# MMDVM_HS_Hat

Please check here for detailed instructions:

    https://github.com/mathisschmieder/MMDVM_HS_Hat/blob/master/README.md

# Makefile options

- make clean: delete all objects files *.o, for starting a new firmware building.

- make: it builds a standard firmware (without USB bootloader support).

- make bl: it builds a firmware with USB bootloader support.

- make zumspot-pi: upload the firmware to a ZUMspot RPi version (using internal RPi serial port)

- make mmdvm_hs_hat: upload the firmware to MMDVM_HS_Hat board (using internal RPi serial port)

- make nano-hotspot: upload the firmware to Nano hotSPOT board (using internal serial port)

- make nano-dv: upload the firmware to NanoDV board (using internal serial port)

- make d2rg_mmdvm_hs: upload the firmware to D2RG MMDVM_HS board (using internal serial port)

- make skybridge: upload the firmware to SkyBridge HotSpot board (using internal serial port)

- make dfu [devser=/dev/ttyXXX]: upload firmware using USB bootloader. "devser" is optional, and it corresponds to the USB serial port device name. This option permits to perform a reset to enter to booloader mode (DFU). If you don't use "devser", you have to press the reset button of the ZUMspot just before using this command.

- make serial devser=/dev/ttyXXX: upload standard firmware using serial port bootloader method.

- make serial-bl devser=/dev/ttyXXX: upload firmware with USB bootloader support using serial port bootloader method.

- make serial-nobl devser=/dev/ttyXXX: upload firmware with USB support using serial port bootloader method, but without USB bootloader installation.

- make serial-bl-old devser=/dev/ttyXXX: same as "make serial-bl" but using bootloader with short reset pulse.

- make stlink: upload standard firmware using ST-Link interface.

- make stlink-bl: upload firmware with USB bootloader support using ST-Link interface.

- make stlink-nobl: upload firmware with USB support using ST-Link interface, but without USB bootloader.

- make stlink-bl-old: same as "make stlink-bl" but using bootloader with short reset pulse.

- make ocd: upload standard firmware using ST-Link interface. This method uses a local openocd installation.

- make ocd-nobl: upload firmware with USB support using ST-Link interface, but without USB bootloader. This method uses a local openocd installation.

- make ocd-bl: upload firmware with USB bootloader support using ST-Link interface. This method uses a local openocd installation.

- make ocd-bl-old: same as "make ocd-bl" but using bootloader with short reset pulse.

## Common Makefile commands

Serial programming (first programming, transfer the USB bootloader):

    make clean
    make bl
    sudo make serial-bl devser=/dev/ttyUSB0

USB programming (you have already transfered the USB bootloader):

    make clean
    make bl
    sudo make dfu (reset ZUMspot before) or
    sudo make dfu devser=/dev/ttyACM0 (/dev/ttyACM0 is the device name of ZUMspot USB under Raspbian)

ZUMspot RPi (no USB support needed):

    make clean
    make
    sudo make zumspot-pi

# Config.h options

- #define ZUMSPOT_ADF7021: enable pinouts support for ZUMspot RPi or ZUMspot USB. You have to enable this option if you have one of these products.

- #define LIBRE_KIT_ADF7021: enable this option if you have a ZUMspot Libre Kit (Board with modified RF7021SE and Blue Pill STM32F103).

- #define MMDVM_HS_HAT_REV12: enable this option if you have a MMDVM_HS_Hat board for RPi.

- #define MMDVM_HS_DUAL_HAT_REV10: enable this option if you have a MMDVM_HS_Dual_Hat board for RPi/USB.

- #define NANO_HOTSPOT: enable this option if you have a Nano hotSPOT (BI7JTA).

- #define NANO_DV_REV10: enable this option if you have a Nano DV (BG4TGO & BG5HHP).

- #define ENABLE_ADF7021: add support for ADF7021 (all boards, enabled by default).

- #define DUPLEX: enable duplex mode with dual ADF7021. It is still under development.

- #define ADF7021_14_7456: select this option if your board uses a 14.7456 MHz (enabled by default).

- #define ADF7021_12_2880: select this option if your board uses a 12.2880 MHz.

- #define STM32_USART1_HOST: enable direct serial host communication with ZUMspot (using USART1 PA9 and PA10 pins). Disable STM32_USB_HOST if you enable this option. Enable this if you have a ZUMspot RPi. You don't need to enable this option if you will transfer the bootloader.

- #define STM32_USB_HOST: enable USB host communication with ZUMspot (using STM32F103 USB interface). Disable STM32_USART1_HOST if you enable this option. Enable this if you have a ZUMspot USB or ZUMspot Libre Kit.

- #define ENABLE_SCAN_MODE: enable automatic mode detection in ZUMspot. This is based on scanning over all enabled modes, and you could have some detection delay. Enabled by default.

- #define SEND_RSSI_DATA: enable RSSI reports to MMDVMHost. It is already converted to dBm.

- #define SERIAL_REPEATER: enable a second serial port (USART2, pins PA2 and PA3) for Nextion LCD display.

- #define SERIAL_REPEATER_USART1: enable USART1 (pins PA9 and PA10) for Nextion LCD display. Do not use with STM32_USART1_HOST enabled, only with USB host communication.

- #define ENABLE_P25_WIDE: enable support for Motorola Wide P25 mod/demod in XTS3000 radios. Using this mode improves RX BER. You need to enable this mode in your radio for each conventional personalities.

- #define QUIET_MODE_LEDS: disable mode LEDs blink during scan mode.

# Pinout definitions

## Pinout definitions for ZUMspot Libre Kit

This is the carrier board or any board with RF7021SE + STM32F103.

Main RF7021SE board:

    CE             PC14
    SLE            PB8
    SREAD          PB7
    SDATA          PB6
    SCLK           PB5
    DATA           PB4 (TxRxData)*
    DCLK           PB3 (TxRxCLK)*
    CLKOUT         PA15 (jumper wire in RF7021SE, not needed with BIDIR_DATA_PIN enabled)
    PAC            PB14 (PTT LED)
    VCC            3.3 V
    GND            Ground

Second RF7021SE board (duplex mode, experimental):

    SLE            PA6
    DATA           PA4 (TxRxData)*
    DCLK           PA5 (TxRxCLK)*
    PAC            NC
    CLKOUT         NC
    VCC            3.3 V
    GND            Ground

    SDATA, SREAD, SCLK and CE are shared with the main ADF7021.

Serial ports:

    TXD            PA9 (serial port host communication)
    RXD            PA10 (serial port host communication)
    DISP_TXD       PA2 (Nextion LCD serial repeater)
    DISP_RXD       PA3 (Nextion LCD serial repeater)

Status LEDs:

    COS_LED        PB15
    PTT_LED        PB14
    NXDN_LED       PA8
    P25_LED        PB0
    YSF_LED        PB1
    DMR_LED        PB13
    DSTAR_LED      PB12

Misc pins:

    PIN_LED        PC13 (status led)
    PIN_DEB        PB9 (debugging pin)

You could install a serie resistor (10 - 100 ohms) in each TxRxData and TxRxCLK lines, for reducing EMI.

## Pinout definitions for Arduino Due/Zero + RF7021SE

Use Arduino IDE with SAM support for building the code.

Main RF7021SE board:

    CE            12
    SLE            6
    SREAD          5
    SDATA          4   // 2 in Arduino Zero Pro
    SCLK           3 
    DATA           7 (TxRxData)*
    DCLK           8 (TxRxCLK)*
    CLKOUT         2   // 4 in Arduino Zero Pro  (jumper wire in RF7021SE, not needed with BIDIR_DATA_PIN enabled)
    PAC            9 (PTT LED)
    VCC            3.3 V
    GND            Ground

Serial ports:

    USB Arduino Programming Port (host communication)

Status LEDs:

    COS_LED       10
    PTT_LED        9
    NXDN_LED      18
    P25_LED       17
    YSF_LED       16
    DMR_LED       15
    DSTAR_LED     14

Misc pins:

    PIN_LED       13
    PIN_DEB       11

You could install a serie resistor (10 - 100 ohms) in each TxRxData and TxRxCLK lines, for reducing EMI.

## Pinout definitions for Teensy (3.1, 3.2, 3.5 or 3.6) + RF7021SE:

Use Teensyduino + Arduino IDE for building the code.

Main RF7021SE board:

    CE             6
    SLE            5
    SREAD          4
    SDATA          3
    SCLK           2
    DATA           7 (TxRxData)*
    DCLK           8 (TxRxCLK)*
    CLKOUT         22 (jumper wire in RF7021SE, not needed with BIDIR_DATA_PIN enabled)
    PAC            14 (PTT LED)
    VCC            3.3 V
    GND            Ground

Serial ports:

    Teensy USB Port (host communication)
    DISP_TXD       1 (Nextion LCD serial repeater)
    DISP_RXD       0 (Nextion LCD serial repeater)

Status LEDs:

    COS_LED       15
    PTT_LED       14
    NXDN_LED      20
    P25_LED       19
    YSF_LED       18
    DMR_LED       17
    DSTAR_LED     16

Misc pins:

    PIN_LED       13
    PIN_DEB       23

You could install a serie resistor (10 - 100 ohms) in each TxRxData and TxRxCLK lines, for reducing EMI.

# Hidden functions

You could enable two test modes, if you edit ADF7021.h file before compilation. Please always comment these two #defines for normal operation.

- #define TEST_DAC: Enable SWD pin to access the demodulator output signal. See application note AN-852 and ADF7021 datasheet, page 60.

- #define TEST_TX: Transmit the carrier frequency. This works only with D-Star mode enabled in MMDVM.ini. This test mode will transmit the carrier frequency defined with TXFrequency in MMDVM.ini. This could be useful to determine the frequency offset of your ZUMspot (with test equipment).

Also in ADF7021.h:

- #define ADF7021_N_VER: enable support for narrow band version of ADF7021 (ADF7021N). Disabled by default, in general all boards will have just ADF7021.

- #define ADF7021_ENABLE_4FSK_AFC: enable AFC support for DMR, YSF and P25. This is experimental, depending on your frequency offset this option will improve or not your BER reception.

- #define ADF7021_AFC_POS: enable this option if you can not receive any signal after enable the ADF7021_ENABLE_4FSK_AFC option.

- #define ADF7021_DISABLE_RC_4FSK: disable TX Raised Cosine filter for 4FSK modulation in ADF7021. Default TX pulse shaping filter for 4FSK is not optimum for DMR, YSF and P25. Activating this option might improve audio in 4FSK digital modes.

In Globals.h:

- #define BIDIR_DATA_PIN: enable Standard TX/RX Data Interface of ADF7021 (enabled by default, needed for scanning mode detection feature).

In IOSTM.cpp:

- #define PI_HAT_7021_REV_02: enable pinouts for first revision of ZUMspot RPi. In general is not used.
