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

// Select one board (STM32F103 based boards)
// 1) ZUMspot USB and ZUMspot RPi:
#define PI_HAT_7021_REV_03
// 2) ZUMspot Libre Kit or board with modified RF7021SE and Blue Pill STM32F103:
// #define ADF7021_CARRIER_BOARD

// Enable ADF7021 support:
#define ENABLE_ADF7021

// Enable full duplex support with dual ADF7021:
// #define DUPLEX

// TCXO of the ADF7021
// For 14.7456 MHz:
#define ADF7021_14_7456
// For 12.2880 MHz:
// #define ADF7021_12_2880

// Host communication selection:
// #define STM32_USART1_HOST
#define STM32_USB_HOST

// Enable mode detection:
#define ENABLE_SCAN_MODE

// Send RSSI value:
#define SEND_RSSI_DATA

// Enable Nextion LCD serial port repeater on USART2 (ZUMspot Libre Kit and ZUMspot RPi):
// #define SERIAL_REPEATER

// Enable Nextion LCD serial port repeater on USART1 (Do not use with STM32_USART1_HOST enabled):
// #define SERIAL_REPEATER_USART1

// Enable P25 Wide modulation:
// #define ENABLE_P25_WIDE

#endif
