/*
 *   Copyright (C) 2016 by Jonathan Naylor G4KLX
 *   Copyright (C) 2016,2017 by Andy Uribe CA6JAU
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

#include "Config.h"
#include "Globals.h"

#include "SerialPort.h"

#if defined(ARDUINO)

void CSerialPort::beginInt(uint8_t n, int speed)
{
  switch (n) {
    case 1U:
    #if defined(STM32_USART1_HOST) && defined(__STM32F1__)
      Serial1.begin(speed);
    #else
      Serial.begin(speed);
    #endif
      break;
    case 3U:
    #if defined(SERIAL_REPEATER) && defined(__STM32F1__)
      Serial2.begin(speed);
    #elif defined(SERIAL_REPEATER_USART1) && defined(__STM32F1__)
      Serial1.begin(speed);
    #elif defined(SERIAL_REPEATER) && (defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))
      Serial1.begin(speed);
    #endif
      break;
    default:
      break;
  }
}

int CSerialPort::availableInt(uint8_t n)
{
  switch (n) {
    case 1U:
    #if defined(STM32_USART1_HOST) && defined(__STM32F1__)
      return Serial1.available();
    #else
      return Serial.available();
    #endif
    case 3U:
    #if defined(SERIAL_REPEATER) && defined(__STM32F1__)
      return Serial2.available();
    #elif defined(SERIAL_REPEATER_USART1) && defined(__STM32F1__)
      return Serial1.available();
    #elif defined(SERIAL_REPEATER) && (defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))
      return Serial1.available();
    #endif
    default:
      return 0;
  }
}

uint8_t CSerialPort::readInt(uint8_t n)
{
  switch (n) {
    case 1U:
    #if defined(STM32_USART1_HOST) && defined(__STM32F1__)
      return Serial1.read();
    #else
      return Serial.read();
    #endif
    case 3U:
    #if defined(SERIAL_REPEATER) && defined(__STM32F1__)
      return Serial2.read();
    #elif defined(SERIAL_REPEATER_USART1) && defined(__STM32F1__)
      return Serial1.read();
    #elif defined(SERIAL_REPEATER) && (defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))
      return Serial1.read();
    #endif
    default:
      return 0U;
  }
}

void CSerialPort::writeInt(uint8_t n, const uint8_t* data, uint16_t length, bool flush)
{
  switch (n) {
    case 1U:
    #if defined(STM32_USART1_HOST) && defined(__STM32F1__)
      Serial1.write(data, length);
    #else
      Serial.write(data, length);
      if (flush)
        Serial.flush();
    #endif
      break;
    case 3U:
    #if defined(SERIAL_REPEATER) && defined(__STM32F1__)
      Serial2.write(data, length);
    #elif defined(SERIAL_REPEATER_USART1) && defined(__STM32F1__)
      Serial1.write(data, length);
    #elif defined(SERIAL_REPEATER) && (defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__))
      Serial1.write(data, length);
    #endif
      break;
    default:
      break;
  }
}

#endif

