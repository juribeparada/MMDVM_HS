/*
 *   Copyright (C) 2009-2016 by Jonathan Naylor G4KLX
 *   Copyright (C) 2016 by Colin Durbridge G4EML
 *   Copyright (C) 2016, 2017 by Andy Uribe CA6JAU
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

// #define WANT_DEBUG

#include "Config.h"
#include "Globals.h"
#include "DMRSlotType.h"

// The PR FILL and Data Sync pattern.
const uint8_t IDLE_DATA[] =
        {0x53U, 0xC2U, 0x5EU, 0xABU, 0xA8U, 0x67U, 0x1DU, 0xC7U, 0x38U, 0x3BU, 0xD9U,
         0x36U, 0x00U, 0x0DU, 0xFFU, 0x57U, 0xD7U, 0x5DU, 0xF5U, 0xD0U, 0x03U, 0xF6U,
         0xE4U, 0x65U, 0x17U, 0x1BU, 0x48U, 0xCAU, 0x6DU, 0x4FU, 0xC6U, 0x10U, 0xB4U};
         
const uint8_t EMPTY_SHORT_LC[] =
      {0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};

const uint8_t DMR_SYNC = 0x5FU;

CDMRDMOTX::CDMRDMOTX() :
m_fifo(),
m_poBuffer(),
m_poLen(0U),
m_poPtr(0U),
m_txDelay(240U),      // 200ms
m_count(0U)
{
}

void CDMRDMOTX::process()
{
  if (m_poLen == 0U && m_fifo.getData() > 0U) {
    if (!m_tx) {
      m_delay = true;
      m_poLen = m_txDelay;
    } else {
      m_delay = false;
      
      createCACH(m_poBuffer + 0U, 0U);

      for (unsigned int i = 0U; i < DMR_FRAME_LENGTH_BYTES; i++)
        m_poBuffer[i + 3U] = m_fifo.get();

      createCACH(m_poBuffer + 36U, 1U);

      for (unsigned int i = 0U; i < DMR_FRAME_LENGTH_BYTES; i++)
        m_poBuffer[i + 39U] = m_idle[i];
        
      m_poLen = 72U;
    }

    m_poPtr = 0U;
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

uint16_t CDMRDMOTX::getSpace() const
{
  return m_fifo.getSpace() / (DMR_FRAME_LENGTH_BYTES + 2U);
}

void CDMRDMOTX::setTXDelay(uint8_t delay)
{
  m_txDelay = 600U + uint16_t(delay) * 12U;        // 500ms + tx delay
}


void CDMRDMOTX::createCACH(uint8_t* buffer, uint8_t slotIndex)
{
  if (m_cachPtr >= 12U)
    m_cachPtr = 0U;

  ::memcpy(buffer, EMPTY_SHORT_LC + m_cachPtr, 3U);

  bool at  = true;
  bool tc  = slotIndex == 1U;
  bool ls0 = true;            // For 1 and 2
  bool ls1 = true;

  if (m_cachPtr == 0U)          // For 0
    ls1 = false;
  else if (m_cachPtr == 9U)     // For 3
    ls0 = false;

  bool h0 = at ^ tc ^ ls1;
  bool h1 = tc ^ ls1 ^ ls0;
  bool h2 = at ^ tc       ^ ls0;

  buffer[0U] |= at ? 0x80U : 0x00U;
  buffer[0U] |= tc ? 0x08U : 0x00U;
  buffer[1U] |= ls1 ? 0x80U : 0x00U;
  buffer[1U] |= ls0 ? 0x08U : 0x00U;
  buffer[1U] |= h0 ? 0x02U : 0x00U;
  buffer[2U] |= h1 ? 0x20U : 0x00U;
  buffer[2U] |= h2 ? 0x02U : 0x00U;

  m_cachPtr += 3U;
}

void CDMRDMOTX::setColorCode(uint8_t colorCode)
{
  ::memcpy(m_idle, IDLE_DATA, DMR_FRAME_LENGTH_BYTES);

  CDMRSlotType slotType;
  slotType.encode(colorCode, DT_IDLE, m_idle);
}

