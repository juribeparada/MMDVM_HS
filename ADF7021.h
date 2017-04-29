/*
 *   Copyright (C) 2016 by Jim McLaughlin KI6ZUM
 *   Copyright (C) 2016, 2017 by Andy Uribe CA6JAU
 *
 *   Some of the code is based on work of Guus Van Dooren PE1PLM:
 *   https://github.com/ki6zum/gmsk-dstar/blob/master/firmware/dvmega/dvmega.ino
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
 
#if !defined(ADF7021_H)
#define  ADF7021_H

#include "Config.h"

#if defined(ENABLE_ADF7021)

/*
- Most of the registers values are obteined from ADI eval software:
http://www.analog.com/en/products/rf-microwave/integrated-transceivers-transmitters-receivers/low-power-rf-transceivers/adf7021.html
- or ADF7021 datasheet formulas:
www.analog.com/media/en/technical-documentation/data-sheets/ADF7021.pdf
*/

/***** Test modes ****/
// Enable SWD pin to access the demodulator output signal
// See application note AN-852 and ADF7021 datasheet, page 60
// #define TEST_DAC

// Transmit the carrier frequency
// #define TEST_TX

/****** Support for 14.7456 MHz TCXO (modified RF7021SE boards) ******/
#if defined(ADF7021_14_7456)

// R = 4
// DEMOD_CLK = 2.4576 MHz (DSTAR)
// DEMOD_CLK = 4.9152 MHz (DMR, YSF_L, P25)
// DEMOD_CLK = 7.3728 MHz (YSF_H)
#define ADF7021_PFD              3686400.0

// PLL (REG 01)
#define ADF7021_REG1_VHF1        0x021F5041
#define ADF7021_REG1_VHF2        0x021F5041
#define ADF7021_REG1_UHF1        0x00575041
#define ADF7021_REG1_UHF2        0x00535041

// Deviation of modulator (REG 02)
#define ADF7021_DEV_DSTAR        43U
#define ADF7021_DEV_DMR          23U
#define ADF7021_DEV_YSF_L        18U
#define ADF7021_DEV_YSF_H        36U
#define ADF7021_DEV_P25          22U

// TX/RX CLOCK register (REG 03)
#define ADF7021_REG3_DSTAR       0x2A4C4193
#if defined(TEST_DAC)
#define ADF7021_REG3_DMR         0x2A4C04D3
#define ADF7021_REG3_YSF_L       0x2A4C04D3
#define ADF7021_REG3_YSF_H       0x2A4C0493
#define ADF7021_REG3_P25         0x2A4C04D3
#else
#define ADF7021_REG3_DMR         0x2A4C80D3
#define ADF7021_REG3_YSF_L       0x2A4C80D3
#define ADF7021_REG3_YSF_H       0x2A4CC093
#define ADF7021_REG3_P25         0x2A4C80D3
#endif

// Discriminator bandwith, demodulator (REG 04)
// Bug in ADI evaluation software, use datasheet formula (4FSK)
#define ADF7021_DISC_BW_DSTAR    522U // K=85
#define ADF7021_DISC_BW_DMR      393U // K=32
#define ADF7021_DISC_BW_YSF_L    393U // K=32
#define ADF7021_DISC_BW_YSF_H    516U // K=28
#define ADF7021_DISC_BW_P25      394U // K=32

// Post demodulator bandwith (REG 04)
#define ADF7021_POST_BW_DSTAR    10U
#define ADF7021_POST_BW_DMR      150U
#define ADF7021_POST_BW_YSF      20U
#define ADF7021_POST_BW_P25      6U

// IF filter (REG 05)
#define ADF7021_REG5             0x000024F5

// IF CAL (fine cal, defaults) (REG 06)
#define ADF7021_REG6             0x05070E16

// AFC configuration (REG 10)
#define ADF7021_REG10_DSTAR      0x0C96473A

#if defined(ADF7021_ENABLE_4FSK_AFC)
#define ADF7021_REG10_DMR        0x01FE473A
#define ADF7021_REG10_YSF        0x01FE473A
#define ADF7021_REG10_P25        0x01FE473A
#if defined(ADF7021_AFC_POS)
#define AFC_OFFSET_DMR           -250
#define AFC_OFFSET_YSF           -250
#define AFC_OFFSET_P25           -250
#else
#define AFC_OFFSET_DMR           250
#define AFC_OFFSET_YSF           250
#define AFC_OFFSET_P25           250
#endif
#else
#define ADF7021_REG10_DMR        0x049E472A
#define ADF7021_REG10_YSF        0x049E472A
#define ADF7021_REG10_P25        0x049E472A
#define AFC_OFFSET_DMR           0
#define AFC_OFFSET_YSF           0
#define AFC_OFFSET_P25           0
#endif

/****** Support for 12.2880 MHz TCXO ******/
#elif defined(ADF7021_12_2880)

// R = 2
// DEMOD_CLK = 2.4576 MHz (DSTAR)
// DEMOD_CLK = 4.0960 MHz (DMR, YSF_L, P25)
// DEMOD_CLK = 6.1440 MHz (YSF_H)
#define ADF7021_PFD              6144000.0

// PLL (REG 01)
#define ADF7021_REG1_VHF1        0x021F5021
#define ADF7021_REG1_VHF2        0x021F5021
#define ADF7021_REG1_UHF1        0x00575021
#define ADF7021_REG1_UHF2        0x00535021

// Deviation of modulator (REG 02)
#define ADF7021_DEV_DSTAR        26U
#define ADF7021_DEV_DMR          14U
#define ADF7021_DEV_YSF_L        11U
#define ADF7021_DEV_YSF_H        21U
#define ADF7021_DEV_P25          14U

// TX/RX CLOCK register (REG 03)
#define ADF7021_REG3_DSTAR       0x29EC4153
#if defined(TEST_DAC)
#define ADF7021_REG3_DMR         0x29EC0493
#define ADF7021_REG3_YSF_L       0x29EC0493
#define ADF7021_REG3_YSF_H       0x29EC0493
#define ADF7021_REG3_P25         0x29EC0493
#else
#define ADF7021_REG3_DMR         0x29ECA093
#define ADF7021_REG3_YSF_L       0x29ECA093
#define ADF7021_REG3_YSF_H       0x29ECA093
#define ADF7021_REG3_P25         0x29ECA093
#endif

// Discriminator bandwith, demodulator (REG 04)
// Bug in ADI evaluation software, use datasheet formula (4FSK)
#define ADF7021_DISC_BW_DSTAR    522U // K=85
#define ADF7021_DISC_BW_DMR      491U // K=32
#define ADF7021_DISC_BW_YSF_L    493U // K=32
#define ADF7021_DISC_BW_YSF_H    430U // K=28
#define ADF7021_DISC_BW_P25      493U // K=32

// Post demodulator bandwith (REG 04)
#define ADF7021_POST_BW_DSTAR    10U
#define ADF7021_POST_BW_DMR      150U
#define ADF7021_POST_BW_YSF      20U
#define ADF7021_POST_BW_P25      6U

// IF filter (REG 05)
#define ADF7021_REG5             0x00001ED5

// IF CAL (fine cal, defaults) (REG 06)
#define ADF7021_REG6             0x0505EBB6

// AFC (REG 10)
#define ADF7021_REG10_DSTAR      0x0C96557A

#if defined(ADF7021_ENABLE_4FSK_AFC)
#define ADF7021_REG10_DMR        0x01FE557A
#define ADF7021_REG10_YSF        0x01FE557A
#define ADF7021_REG10_P25        0x01FE557A
#if defined(ADF7021_AFC_POS)
#define AFC_OFFSET_DMR           -250
#define AFC_OFFSET_YSF           -250
#define AFC_OFFSET_P25           -250
#else
#define AFC_OFFSET_DMR           250
#define AFC_OFFSET_YSF           250
#define AFC_OFFSET_P25           250
#endif
#else
#define ADF7021_REG10_DMR        0x049E556A
#define ADF7021_REG10_YSF        0x049E556A
#define ADF7021_REG10_P25        0x049E556A
#define AFC_OFFSET_DMR           0
#define AFC_OFFSET_YSF           0
#define AFC_OFFSET_P25           0
#endif

#endif

// Slicer threshold for 4FSK demodulator (REG 13)
#if defined(ADF7021_N_VER)

#define ADF7021_SLICER_TH_DSTAR  0U
#define ADF7021_SLICER_TH_DMR    51U
#define ADF7021_SLICER_TH_YSF_L  32U
#define ADF7021_SLICER_TH_YSF_H  63U
#define ADF7021_SLICER_TH_P25    43U

#else

#define ADF7021_SLICER_TH_DSTAR  0U
#define ADF7021_SLICER_TH_DMR    57U
#define ADF7021_SLICER_TH_YSF_L  38U
#define ADF7021_SLICER_TH_YSF_H  75U
#define ADF7021_SLICER_TH_P25    52U

#endif

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

void Send_AD7021_control(bool doSle = true);

#endif

#endif
