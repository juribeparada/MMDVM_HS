#!/bin/bash

#   Copyright (C) 2017,2018 by Andy Uribe CA6JAU

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

cd ~/MMDVM_HS/
make clean
git pull

# Building ZUMspot Libre Kit
cp ~/MMDVM_HS/configs/ZUMspot_Libre.h ~/MMDVM_HS/Config.h
make bl
mv ~/MMDVM_HS/bin/mmdvm_f1bl.bin ~/MMDVM_HS/bin/zumspot_libre_fw.bin
make clean

# Building ZUMspot RPi
cp ~/MMDVM_HS/configs/ZUMspot_RPi.h ~/MMDVM_HS/Config.h
make
mv ~/MMDVM_HS/bin/mmdvm_f1.bin ~/MMDVM_HS/bin/zumspot_rpi_fw.bin
make clean

# Building ZUMspot USB
cp ~/MMDVM_HS/configs/ZUMspot_USB.h ~/MMDVM_HS/Config.h
make bl
mv ~/MMDVM_HS/bin/mmdvm_f1bl.bin ~/MMDVM_HS/bin/zumspot_usb_fw.bin
make clean

# Building ZUMspot Duplex
cp ~/MMDVM_HS/configs/ZUMspot_duplex.h ~/MMDVM_HS/Config.h
make bl
mv ~/MMDVM_HS/bin/mmdvm_f1bl.bin ~/MMDVM_HS/bin/zumspot_duplex_fw.bin
make clean

# Building MMDVM_HS_Hat
cp ~/MMDVM_HS/configs/MMDVM_HS_Hat.h ~/MMDVM_HS/Config.h
make
mv ~/MMDVM_HS/bin/mmdvm_f1.bin ~/MMDVM_HS/bin/mmdvm_hs_hat_fw.bin
make clean

# Building MMDVM_HS_Hat (12.288 MHz TCXO)
cp ~/MMDVM_HS/configs/MMDVM_HS_Hat-12mhz.h ~/MMDVM_HS/Config.h
make
mv ~/MMDVM_HS/bin/mmdvm_f1.bin ~/MMDVM_HS/bin/mmdvm_hs_hat_fw-12mhz.bin
make clean

# Building MMDVM_HS_Dual_Hat
cp ~/MMDVM_HS/configs/MMDVM_HS_Dual_Hat.h ~/MMDVM_HS/Config.h
make
mv ~/MMDVM_HS/bin/mmdvm_f1.bin ~/MMDVM_HS/bin/mmdvm_hs_dual_hat_fw.bin
make clean

# Building Nano hotSPOT
cp ~/MMDVM_HS/configs/Nano_hotSPOT.h ~/MMDVM_HS/Config.h
make
mv ~/MMDVM_HS/bin/mmdvm_f1.bin ~/MMDVM_HS/bin/nano_hotspot_fw.bin
make clean

# Building Nano DV
cp ~/MMDVM_HS/configs/NanoDV.h ~/MMDVM_HS/Config.h
make
mv ~/MMDVM_HS/bin/mmdvm_f1.bin ~/MMDVM_HS/bin/nano_dv_fw.bin
make clean

# Building Generic Simplex GPIO
cp ~/MMDVM_HS/configs/generic_gpio.h ~/MMDVM_HS/Config.h
make
mv ~/MMDVM_HS/bin/mmdvm_f1.bin ~/MMDVM_HS/bin/generic_gpio_fw.bin
make clean

# Building Generic Duplex GPIO
cp ~/MMDVM_HS/configs/generic_duplex_gpio.h ~/MMDVM_HS/Config.h
make
mv ~/MMDVM_HS/bin/mmdvm_f1.bin ~/MMDVM_HS/bin/generic_duplex_gpio_fw.bin
make clean

cp ~/MMDVM_HS/configs/ZUMspot_Libre.h ~/MMDVM_HS/Config.h
