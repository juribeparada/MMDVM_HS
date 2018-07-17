/*
 *   Copyright (C) 2017,2018 by Andy Uribe CA6JAU, Florian Wolters DF2ET
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
// 1) ZUMspot RPi or ZUMspot USB:
// #define ZUMSPOT_ADF7021
// 2) Libre Kit board or any homebrew hotspot with modified RF7021SE and Blue Pill STM32F103:
// #define LIBRE_KIT_ADF7021
// 3) MMDVM_HS_Hat revisions 1.1, 1.2 and 1.4 (DB9MAT & DF2ET)
// #define MMDVM_HS_HAT_REV12
// 4) MMDVM_HS_Dual_Hat revisions 1.0 (DB9MAT & DF2ET & DO7EN)
#define MMDVM_HS_DUAL_HAT_REV10
// 5) Nano hotSPOT (BI7JTA)
// #define NANO_HOTSPOT
// 6) Nano DV revisions 1.0 (BG4TGO & BG5HHP)
// #define NANO_DV_REV10

// Enable ADF7021 support:
#define ENABLE_ADF7021

// Enable full duplex support with dual ADF7021 (valid for homebrew hotspots only):
#define DUPLEX

// TCXO of the ADF7021
// For 14.7456 MHz:
// #define ADF7021_14_7456
// For 12.2880 MHz:
#define ADF7021_12_2880

// Configure receiver gain for ADF7021
// AGC automatic, default settings:
#define AD7021_GAIN_AUTO
// AGC automatic with high LNA linearity:
// #define AD7021_GAIN_AUTO_LIN
// AGC OFF, lowest gain: 
// #define AD7021_GAIN_LOW
// AGC OFF, highest gain:
// #define AD7021_GAIN_HIGH

// Host communication selection:
#define STM32_USART1_HOST
// #define STM32_USB_HOST

// Enable mode detection:
#define ENABLE_SCAN_MODE

// Send RSSI value:
#define SEND_RSSI_DATA

// Enable Nextion LCD serial port repeater on USART2 (ZUMspot Libre Kit and ZUMspot RPi):
#define SERIAL_REPEATER

// Enable Nextion LCD serial port repeater on USART1 (Do not use with STM32_USART1_HOST enabled):
// #define SERIAL_REPEATER_USART1

// Enable P25 Wide modulation:
// #define ENABLE_P25_WIDE

// Disable mode LEDs blink during scan mode:
// #define QUIET_MODE_LEDS

// Engage a constant or descreet Service LED mode once repeater is running 
// #define CONSTANT_SRV_LED
// #define CONSTANT_SRV_LED_INVERTED
// #define DISCREET_SRV_LED
// #define DISCREET_SRV_LED_INVERTED

// Use the YSF and P25 LEDs for NXDN
// #define USE_ALTERNATE_NXDN_LEDS

// Use the D-Star and DMR LEDs for POCSAG
#define USE_ALTERNATE_POCSAG_LEDS

// Enable modem debug messages
// #define ENABLE_DEBUG

#endif
