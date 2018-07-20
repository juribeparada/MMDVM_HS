/*
 *   Copyright (C) 2009-2016 by Jonathan Naylor G4KLX
 *   Copyright (C) 2016 by Colin Durbridge G4EML
 *   Copyright (C) 2016,2017,2018 by Andy Uribe CA6JAU
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

// PR FILL pattern
const uint8_t PR_FILL[] =
        {0x63U, 0xEAU, 0x00U, 0x76U, 0x6CU, 0x76U, 0xC4U, 0x52U, 0xC8U, 0x78U,
         0x09U, 0x2DU, 0xB8U, 0x79U, 0x27U, 0x57U, 0x9BU, 0x31U, 0xBCU, 0x3EU,
         0xEAU, 0x45U, 0xC3U, 0x30U, 0x49U, 0x17U, 0x93U, 0xAEU, 0x8BU, 0x6DU,
         0xA4U, 0xA5U, 0xADU, 0xA2U, 0xF1U, 0x35U, 0xB5U, 0x3CU, 0x1EU};

const uint8_t DMR_SYNC = 0x5FU;

CDMRDMOTX::CDMRDMOTX() :
m_fifo(875U),
m_poBuffer(),
m_poLen(0U),
m_poPtr(0U),
m_txDelay(240U),      // 200ms
m_delay(false),
m_cal(false)
{
}

void CDMRDMOTX::process()
{
  if (m_cal) {
    if (m_poLen == 0U) {
      m_delay = false;
      createCal();
    }
  }
  else {
    if (m_poLen == 0U && m_fifo.getData() > 0U) {
      if (!m_tx) {
        m_delay = true;
        m_poLen = m_txDelay;
      } else {
        m_delay = false;

        for (unsigned int i = 0U; i < DMR_FRAME_LENGTH_BYTES; i++)
          m_poBuffer[i] = m_fifo.get();

        for (unsigned int i = 0U; i < 39U; i++)
          m_poBuffer[i + DMR_FRAME_LENGTH_BYTES] = PR_FILL[i];

        m_poLen = 72U;
      }
      m_poPtr = 0U;
    }
  }

  if (m_poLen > 0U) {
    uint16_t space = io.getSpace();

    while (space > 8U) {
      if (m_delay) {
        m_poPtr++;
        writeByte(DMR_SYNC);
      } else
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

uint8_t CDMRDMOTX::writeData(const uint8_t* data, uint8_t length)
{
  if (length != (DMR_FRAME_LENGTH_BYTES + 1U))
    return 4U;

  uint16_t space = m_fifo.getSpace();
  if (space < DMR_FRAME_LENGTH_BYTES)
    return 5U;

  for (uint8_t i = 0U; i < DMR_FRAME_LENGTH_BYTES; i++)
    m_fifo.put(data[i + 1U]);

  return 0U;
}

void CDMRDMOTX::writeByte(uint8_t c)
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

void CDMRDMOTX::setCal(bool start)
{
  m_cal = start ? true : false;
}

void CDMRDMOTX::createCal()
{
  // 1.2 kHz sine wave generation
  if (m_calState == STATE_DMRCAL) {
    for (unsigned int i = 0U; i < DMR_FRAME_LENGTH_BYTES; i++) {
      m_poBuffer[i]   = 0x5FU;              // +3, +3, -3, -3 pattern for deviation cal.
    }

    m_poLen = DMR_FRAME_LENGTH_BYTES;
  }

  m_poPtr = 0U;
}

uint16_t CDMRDMOTX::getSpace() const
{
  return m_fifo.getSpace() / (DMR_FRAME_LENGTH_BYTES + 2U);
}

void CDMRDMOTX::setTXDelay(uint8_t delay)
{
  m_txDelay = 600U + uint16_t(delay) * 12U;        // 500ms + tx delay
}
