/*
 *   Copyright (C) 2016 by Jim McLaughlin KI6ZUM
 *   Copyright (C) 2016,2017,2018 by Andy Uribe CA6JAU
 *   Copyright (C) 2017 by Danilo DB4PLE 
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

// Enable SWD pin to access the demodulator output signal:
// (See application note AN-852 and ADF7021 datasheet, page 60)
// #define TEST_DAC

// Transmit the carrier frequency:
// #define TEST_TX

/*********************/

// Disable TX Raised Cosine filter for 4FSK modulation in ADF7021:
// #define ADF7021_DISABLE_RC_4FSK

// Support for ADF7021-N version: 
// #define ADF7021_N_VER

// Enable AFC support for DMR, YSF and P25 (experimental):
// (AFC is already enabled by default in D-Star)
// #define ADF7021_ENABLE_4FSK_AFC

// Configure AFC with positive initial frequency offset:
// #define ADF7021_AFC_POS

/****** Support for 14.7456 MHz TCXO (modified RF7021SE boards) ******/
#if defined(ADF7021_14_7456)

// R = 4
// DEMOD_CLK = 2.4576 MHz (DSTAR)
// DEMOD_CLK = 4.9152 MHz (DMR, YSF_L, P25)
// DEMOD_CLK = 7.3728 MHz (YSF_H)
// DEMOD CLK = 3.6864 MHz (NXDN)
// DEMOD_CLK = 7.3728 MHz (POCSAG)
#define ADF7021_PFD              3686400.0

// PLL (REG 01)
#define ADF7021_REG1_VHF1        0x021F5041
#define ADF7021_REG1_VHF2        0x021F5041
#define ADF7021_REG1_UHF1        0x00575041
#define ADF7021_REG1_UHF2        0x00535041

// Deviation of modulator (REG 02)
#define ADF7021_DEV_DSTAR        43U
#define ADF7021_DEV_DMR          23U
#define ADF7021_DEV_YSF_L        16U
#define ADF7021_DEV_YSF_H        32U
#if defined(ENABLE_P25_WIDE)
#define ADF7021_DEV_P25          32U
#else
#define ADF7021_DEV_P25          22U
#endif
#define ADF7021_DEV_NXDN         13U
#define ADF7021_DEV_POCSAG       160U

// TX/RX CLOCK register (REG 03)
#define ADF7021_REG3_DSTAR       0x2A4C4193
#if defined(TEST_DAC)
#define ADF7021_REG3_DMR         0x2A4C04D3
#define ADF7021_REG3_YSF_L       0x2A4C04D3
#define ADF7021_REG3_YSF_H       0x2A4C0493
#define ADF7021_REG3_P25         0x2A4C04D3
#define ADF7021_REG3_NXDN        0x2A4C04D3
#else
#define ADF7021_REG3_DMR         0x2A4C80D3
#define ADF7021_REG3_YSF_L       0x2A4C80D3
#define ADF7021_REG3_YSF_H       0x2A4CC093
#define ADF7021_REG3_P25         0x2A4C80D3
#define ADF7021_REG3_NXDN        0x2A4CC113
#endif
#define ADF7021_REG3_POCSAG      0x2A4F0093

// Discriminator bandwith, demodulator (REG 04)
// Bug in ADI evaluation software, use datasheet formula (4FSK)
#define ADF7021_DISC_BW_DSTAR    522U // K=85
#define ADF7021_DISC_BW_DMR      393U // K=32
#define ADF7021_DISC_BW_YSF_L    393U // K=32
#define ADF7021_DISC_BW_YSF_H    516U // K=28
#define ADF7021_DISC_BW_P25      394U // K=32
#define ADF7021_DISC_BW_NXDN     295U // K=32
#define ADF7021_DISC_BW_POCSAG   406U // K=22

// Post demodulator bandwith (REG 04)
#define ADF7021_POST_BW_DSTAR    10U
#define ADF7021_POST_BW_DMR      80U
#define ADF7021_POST_BW_YSF      20U
#define ADF7021_POST_BW_P25      6U
#define ADF7021_POST_BW_NXDN     7U
#define ADF7021_POST_BW_POCSAG   1U

// IF filter (REG 05)
#define ADF7021_REG5             0x000024F5

// IF CAL (fine cal, defaults) (REG 06)
#define ADF7021_REG6             0x05070E16

// AFC configuration (REG 10)
#define ADF7021_REG10_DSTAR      0x0C96473A
#define ADF7021_REG10_POCSAG     0x1496473A

#if defined(ADF7021_ENABLE_4FSK_AFC)
#define ADF7021_REG10_DMR        0x01FE473A
#define ADF7021_REG10_YSF        0x01FE473A
#define ADF7021_REG10_P25        0x01FE473A
#define ADF7021_REG10_NXDN       0x01FE473A
#if defined(ADF7021_AFC_POS)
#define AFC_OFFSET_DMR           -250
#define AFC_OFFSET_YSF           -250
#define AFC_OFFSET_P25           -250
#define AFC_OFFSET_NXDN          -250
#else
#define AFC_OFFSET_DMR           250
#define AFC_OFFSET_YSF           250
#define AFC_OFFSET_P25           250
#define AFC_OFFSET_NXDN          250
#endif
#else
#define ADF7021_REG10_DMR        0x049E472A
#define ADF7021_REG10_YSF        0x049E472A
#define ADF7021_REG10_P25        0x049E472A
#define ADF7021_REG10_NXDN       0x049E472A
#define AFC_OFFSET_DMR           0
#define AFC_OFFSET_YSF           0
#define AFC_OFFSET_P25           0
#define AFC_OFFSET_NXDN          0
#endif

/****** Support for 12.2880 MHz TCXO ******/
#elif defined(ADF7021_12_2880)

// R = 2
// DEMOD_CLK = 2.4576 MHz (DSTAR)
// DEMOD_CLK = 6.1440 MHz (DMR, YSF_H, YSF_L, P25)
// DEMOD_CLK = 3.0720 MHz (NXDN)
// DEMOD_CLK = 6.1440 MHz (POCSAG)
#define ADF7021_PFD              6144000.0

// PLL (REG 01)
#define ADF7021_REG1_VHF1        0x021F5021
#define ADF7021_REG1_VHF2        0x021F5021
#define ADF7021_REG1_UHF1        0x00575021
#define ADF7021_REG1_UHF2        0x00535021

// Deviation of modulator (REG 02)
#define ADF7021_DEV_DSTAR        26U
#define ADF7021_DEV_DMR          14U
#define ADF7021_DEV_YSF_L        10U
#define ADF7021_DEV_YSF_H        19U
#if defined(ENABLE_P25_WIDE)
#define ADF7021_DEV_P25          19U
#else
#define ADF7021_DEV_P25          13U
#endif
#define ADF7021_DEV_NXDN         8U
#define ADF7021_DEV_POCSAG       96U

// TX/RX CLOCK register (REG 03)
#define ADF7021_REG3_DSTAR       0x29EC4153
#if defined(TEST_DAC)
#define ADF7021_REG3_DMR         0x29EC0493
#define ADF7021_REG3_YSF_L       0x29EC0493
#define ADF7021_REG3_YSF_H       0x29EC0493
#define ADF7021_REG3_P25         0x29EC0493
#define ADF7021_REG3_NXDN        0x29EC0493
#else
#define ADF7021_REG3_DMR         0x29ECA093
#define ADF7021_REG3_YSF_L       0x29ECA093
#define ADF7021_REG3_YSF_H       0x29ECA093
#define ADF7021_REG3_P25         0x29ECA093
#define ADF7021_REG3_NXDN        0x29ECA113
#endif
#define ADF7021_REG3_POCSAG      0x29EE8093

// Discriminator bandwith, demodulator (REG 04)
// Bug in ADI evaluation software, use datasheet formula (4FSK)
#define ADF7021_DISC_BW_DSTAR    522U // K=85
#define ADF7021_DISC_BW_DMR      491U // K=32
#define ADF7021_DISC_BW_YSF_L    491U // K=32
#define ADF7021_DISC_BW_YSF_H    430U // K=28
#define ADF7021_DISC_BW_P25      493U // K=32
#define ADF7021_DISC_BW_NXDN     246U // K=32
#define ADF7021_DISC_BW_POCSAG   338U // K=22

// Post demodulator bandwith (REG 04)
#define ADF7021_POST_BW_DSTAR    10U
#define ADF7021_POST_BW_DMR      80U
#define ADF7021_POST_BW_YSF      20U
#define ADF7021_POST_BW_P25      6U
#define ADF7021_POST_BW_NXDN     8U
#define ADF7021_POST_BW_POCSAG   1U

// IF filter (REG 05)
#define ADF7021_REG5             0x00001ED5

// IF CAL (fine cal, defaults) (REG 06)
#define ADF7021_REG6             0x0505EBB6

// AFC (REG 10)
#define ADF7021_REG10_DSTAR      0x0C96557A
#define ADF7021_REG10_POCSAG     0x1496557A

#if defined(ADF7021_ENABLE_4FSK_AFC)
#define ADF7021_REG10_DMR        0x01FE557A
#define ADF7021_REG10_YSF        0x01FE557A
#define ADF7021_REG10_P25        0x01FE557A
#define ADF7021_REG10_NXDN       0x01FE557A
#if defined(ADF7021_AFC_POS)
#define AFC_OFFSET_DMR           -250
#define AFC_OFFSET_YSF           -250
#define AFC_OFFSET_P25           -250
#define AFC_OFFSET_NXDN          -250
#else
#define AFC_OFFSET_DMR           250
#define AFC_OFFSET_YSF           250
#define AFC_OFFSET_P25           250
#define AFC_OFFSET_NXDN          250
#endif
#else
#define ADF7021_REG10_DMR        0x049E556A
#define ADF7021_REG10_YSF        0x049E556A
#define ADF7021_REG10_P25        0x049E556A
#define ADF7021_REG10_NXDN       0x049E556A
#define AFC_OFFSET_DMR           0
#define AFC_OFFSET_YSF           0
#define AFC_OFFSET_P25           0
#define AFC_OFFSET_NXDN          0
#endif

#endif

// Slicer threshold for 4FSK demodulator (REG 13)
#if defined(ADF7021_N_VER)

#define ADF7021_SLICER_TH_DSTAR  0U
#define ADF7021_SLICER_TH_DMR    51U
#define ADF7021_SLICER_TH_YSF_L  35U
#define ADF7021_SLICER_TH_YSF_H  69U
#define ADF7021_SLICER_TH_P25    43U
#define ADF7021_SLICER_TH_NXDN   26U

#else

#define ADF7021_SLICER_TH_DSTAR  0U
#define ADF7021_SLICER_TH_DMR    57U
#define ADF7021_SLICER_TH_YSF_L  38U
#define ADF7021_SLICER_TH_YSF_H  75U
#define ADF7021_SLICER_TH_P25    47U
#define ADF7021_SLICER_TH_NXDN   26U

#endif

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)

void Send_AD7021_control(bool doSle = true);
#if defined(DUPLEX)
void Send_AD7021_control2(bool doSle = true);
#endif

#if defined(ADF7021_DISABLE_RC_4FSK)
#define ADF7021_EVEN_BIT  true
#else
#define ADF7021_EVEN_BIT  false
#endif

#endif

#endif
