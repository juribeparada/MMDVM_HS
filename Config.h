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

// Enable ADF7021 support:
#define ENABLE_ADF7021

// Bidirectional Data pin (Enable Standard TX/RX Data Interface of ADF7021)
// #define BIDIR_DATA_PIN

// TCXO of the ADF7021:
// For 14.7456 MHz:
#define ADF7021_14_7456
// For 12.2880 MHz:
// #define ADF7021_12_2880
// Original 19.68 MHz, not working, only for experimentation:
// #define ADF7021_19_6800

// Host communication selection:
//#define STM32_USART1_HOST
#define STM32_USB_HOST

// Send RSSI value
// #define SEND_RSSI_DATA

// Enable Nextion LCD serial port repeater:
//#define SERIAL_REPEATER

#endif
