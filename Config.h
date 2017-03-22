/*
 *   Copyright (C) 2017 by Andy Uribe CA6JAU
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

#if !defined(CONFIG_H)
#define  CONFIG_H

// Select one board:
// 1) Initial release of Pi HAT:
// #define PI_HAT_7021_REV_02
// 2) ZUM-Spot USB v0.1 and Pi HAT v0.3:
// #define PI_HAT_7021_REV_03
// 3) Board with modified RF7021SE and Blue Pill STM32F103 (initial tests)
#define ADF7021_CARRIER_BOARD

// Enable ADF7021 support:
#define ENABLE_ADF7021

// Support for ADF7021-N version 
// #define ADF7021_N_VER

// Bidirectional Data pin (Enable Standard TX/RX Data Interface of ADF7021):
#define BIDIR_DATA_PIN

// TCXO of the ADF7021:
// For 14.7456 MHz:
#define ADF7021_14_7456
// For 12.2880 MHz:
// #define ADF7021_12_2880
// Original 19.68 MHz, not working, only for experimentation:
// #define ADF7021_19_6800

// AFC is enabled by default in D-Star

// Enable AFC support for DMR, YSF and P25 (experimental)
// #define ADF7021_ENABLE_4FSK_AFC
// Configure AFC with positive initial frequency offset
// #define ADF7021_AFC_POS

// Host communication selection:
// #define STM32_USART1_HOST
#define STM32_USB_HOST

// Enable Half Deviation mode in YSF (experimental)
// #define ADF7021_YSF_HALF_DEV

// Send RSSI value:
// #define SEND_RSSI_DATA

// Enable Nextion LCD serial port repeater:
// #define SERIAL_REPEATER

#endif
