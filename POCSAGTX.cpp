/*
 *   Copyright (C) 2015,2016,2017,2018 by Jonathan Naylor G4KLX
 *   Copyright (C) 2018 by Andy Uribe CA6JAU
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
#include "POCSAGTX.h"
#include "POCSAGDefines.h"

CPOCSAGTX::CPOCSAGTX() :
m_buffer(1000U),
m_poBuffer(),
m_poLen(0U),
m_poPtr(0U),
m_txDelay(POCSAG_PREAMBLE_LENGTH_BYTES),
m_delay(false)
{
}

void CPOCSAGTX::process()
{
  if (m_buffer.getData() == 0U && m_poLen == 0U)
    return;

  if (m_poLen == 0U) {
    if (!m_tx) {
      m_delay = true;
      m_poLen = m_txDelay;
    } else {
      m_delay = false;
      for (uint8_t i = 0U; i < POCSAG_FRAME_LENGTH_BYTES; i++)
        m_poBuffer[m_poLen++] = m_buffer.get();
    }

    m_poPtr = 0U;
  }

  if (m_poLen > 0U) {
    uint16_t space = io.getSpace();
    
    while (space > 8U) {
      if (m_delay) {
        m_poPtr++;
        writeByte(POCSAG_SYNC);
      }
      else
        writeByte(m_poBuffer[m_poPtr++]); 

      space -= 8U;
      
      if (m_poPtr >= m_poLen) {
        m_poPtr = 0U;
        m_poLen = 0U;
        m_delay = false;
        return;
      }
    }
  }
}

uint8_t CPOCSAGTX::writeData(const uint8_t* data, uint8_t length)
{
  if (length != POCSAG_FRAME_LENGTH_BYTES)
    return 4U;

  uint16_t space = m_buffer.getSpace();
  if (space < POCSAG_FRAME_LENGTH_BYTES)
    return 5U;

  for (uint8_t i = 0U; i < POCSAG_FRAME_LENGTH_BYTES; i++)
    m_buffer.put(data[i]);

  return 0U;
}

void CPOCSAGTX::writeByte(uint8_t c)
{
  uint8_t bit;
  uint8_t mask = 0x80U;
  
  for (uint8_t i = 0U; i < 8U; i++, c <<= 1) {
    if ((c & mask) == mask)
      bit = 1U;
    else
      bit = 0U;

    io.write(&bit, 1);
  }
}

void CPOCSAGTX::setTXDelay(uint8_t delay)
{
  m_txDelay = POCSAG_PREAMBLE_LENGTH_BYTES + uint16_t(delay);

  if (m_txDelay > 1200U)
    m_txDelay = 1200U;
}

uint8_t CPOCSAGTX::getSpace() const
{
  return m_buffer.getSpace() / POCSAG_FRAME_LENGTH_BYTES;
}
