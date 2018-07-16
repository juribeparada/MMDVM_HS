/*
 *   Copyright (C) 2009-2017 by Jonathan Naylor G4KLX
 *   Copyright (C) 2017,2018 by Andy Uribe CA6JAU
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

#if defined(DUPLEX)

#include "Globals.h"
#include "DMRIdleRX.h"
#include "DMRSlotType.h"
#include "Utils.h"

const uint8_t MAX_SYNC_BYTES_ERRS = 2U;

const uint16_t NOENDPTR = 9999U;

const uint8_t CONTROL_IDLE = 0x80U;
const uint8_t CONTROL_DATA = 0x40U;

const uint8_t BIT_MASK_TABLE[] = {0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U};

#define WRITE_BIT1(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE[(i)&7])
#define READ_BIT1(p,i)    ((p[(i)>>3] & BIT_MASK_TABLE[(i)&7]) >> (7 - ((i)&7)))

CDMRIdleRX::CDMRIdleRX() :
m_patternBuffer(0U),
m_buffer(),
m_dataPtr(0U),
m_endPtr(NOENDPTR),
m_colorCode(0U)
{
}

void CDMRIdleRX::reset()
{
  m_dataPtr   = 0U;
  m_endPtr    = NOENDPTR;
}

void CDMRIdleRX::databit(bool bit)
{
  WRITE_BIT1(m_buffer, m_dataPtr, bit);

  m_patternBuffer <<= 1;
  if (bit)
    m_patternBuffer |= 0x01U;

  if (countBits64((m_patternBuffer & DMR_SYNC_BITS_MASK) ^ DMR_MS_DATA_SYNC_BITS) <= MAX_SYNC_BYTES_ERRS) {
    m_endPtr = m_dataPtr + DMR_SLOT_TYPE_LENGTH_BITS / 2U + DMR_INFO_LENGTH_BITS / 2U;
    if (m_endPtr >= DMR_IDLE_LENGTH_BITS)
      m_endPtr -= DMR_IDLE_LENGTH_BITS;
    // DEBUG3("SYNC MS Data found pos/end:", m_dataPtr, m_endPtr);
  }

  if (m_dataPtr == m_endPtr) {
    uint16_t ptr = m_endPtr + DMR_IDLE_LENGTH_BITS - DMR_FRAME_LENGTH_BITS + 1;
    if (ptr >= DMR_IDLE_LENGTH_BITS)
      ptr -= DMR_IDLE_LENGTH_BITS;

    uint8_t frame[DMR_FRAME_LENGTH_BYTES + 1U];
    bitsToBytes(ptr, DMR_FRAME_LENGTH_BYTES, frame + 1U);

    uint8_t colorCode;
    uint8_t dataType;
    CDMRSlotType slotType;
    slotType.decode(frame + 1U, colorCode, dataType);

    if (colorCode == m_colorCode && dataType == DT_CSBK) {
      frame[0U] = CONTROL_IDLE | CONTROL_DATA | DT_CSBK;
      serial.writeDMRData(false, frame, DMR_FRAME_LENGTH_BYTES + 1U);
    }

    m_endPtr  = NOENDPTR;
  }

  m_dataPtr++;
  if (m_dataPtr >= DMR_IDLE_LENGTH_BITS)
    m_dataPtr = 0U;
}

void CDMRIdleRX::bitsToBytes(uint16_t start, uint8_t count, uint8_t* buffer)
{
  for (uint8_t i = 0U; i < count; i++) {
    buffer[i]  = 0U;
    buffer[i] |= READ_BIT1(m_buffer, start) << 7;
    start++;
    if (start >= DMR_IDLE_LENGTH_BITS)
      start -= DMR_IDLE_LENGTH_BITS;
    buffer[i] |= READ_BIT1(m_buffer, start) << 6;
    start++;
    if (start >= DMR_IDLE_LENGTH_BITS)
      start -= DMR_IDLE_LENGTH_BITS;
    buffer[i] |= READ_BIT1(m_buffer, start) << 5;
    start++;
    if (start >= DMR_IDLE_LENGTH_BITS)
      start -= DMR_IDLE_LENGTH_BITS;
    buffer[i] |= READ_BIT1(m_buffer, start) << 4;
    start++;
    if (start >= DMR_IDLE_LENGTH_BITS)
      start -= DMR_IDLE_LENGTH_BITS;
    buffer[i] |= READ_BIT1(m_buffer, start) << 3;
    start++;
    if (start >= DMR_IDLE_LENGTH_BITS)
      start -= DMR_IDLE_LENGTH_BITS;
    buffer[i] |= READ_BIT1(m_buffer, start) << 2;
    start++;
    if (start >= DMR_IDLE_LENGTH_BITS)
      start -= DMR_IDLE_LENGTH_BITS;
    buffer[i] |= READ_BIT1(m_buffer, start) << 1;
    start++;
    if (start >= DMR_IDLE_LENGTH_BITS)
      start -= DMR_IDLE_LENGTH_BITS;
    buffer[i] |= READ_BIT1(m_buffer, start) << 0;
    start++;
    if (start >= DMR_IDLE_LENGTH_BITS)
      start -= DMR_IDLE_LENGTH_BITS;
  }
}

void CDMRIdleRX::setColorCode(uint8_t colorCode)
{
  m_colorCode = colorCode;
}

#endif
