/*
 *   Copyright (C) 2009-2017 by Jonathan Naylor G4KLX
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

#include "Config.h"

#if defined(DUPLEX)

#include "Globals.h"
#include "DMRIdleRX.h"
#include "DMRSlotType.h"
#include "Utils.h"

const uint8_t MAX_SYNC_SYMBOLS_ERRS = 2U;
const uint8_t MAX_SYNC_BYTES_ERRS   = 3U;

const uint16_t NOENDPTR = 9999U;

const uint8_t CONTROL_IDLE = 0x80U;
const uint8_t CONTROL_DATA = 0x40U;

CDMRIdleRX::CDMRIdleRX() :
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
  m_buffer[m_dataPtr] = bit;
  
  m_patternBuffer <<= 1;
  if (bit)
    m_patternBuffer |= 0x01U;
    
  if (countBits64((m_patternBuffer & DMR_SYNC_BITS_MASK) ^ DMR_MS_DATA_SYNC_BITS) <= MAX_SYNC_BYTES_ERRS) {
    m_endPtr    = m_dataPtr + DMR_SLOT_TYPE_LENGTH_BITS / 2U + DMR_INFO_LENGTH_BITS / 2U;
    if (m_endPtr >= DMR_FRAME_LENGTH_BITS)
      m_endPtr -= DMR_FRAME_LENGTH_BITS;
          
    // DEBUG3("SYNC MS Data found pos/end:", m_dataPtr, m_endPtr);
  }

  if (m_dataPtr == m_endPtr) {
    uint16_t ptr = m_endPtr + 1;

    if (ptr >= DMR_FRAME_LENGTH_BITS)
	  ptr -= DMR_FRAME_LENGTH_BITS;

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
  if (m_dataPtr >= DMR_FRAME_LENGTH_BITS)
    m_dataPtr = 0U;
}

void CDMRIdleRX::bitsToBytes(uint16_t start, uint8_t count, uint8_t* buffer)
{
  for (uint8_t i = 0U; i < count; i++) {
    buffer[i]  = 0U;
    buffer[i] |= ((m_buffer[start++] & 0x01) << 7);
    if (start >= DMR_FRAME_LENGTH_BITS)
      start -= DMR_FRAME_LENGTH_BITS;
    buffer[i] |= ((m_buffer[start++] & 0x01) << 6);
    if (start >= DMR_FRAME_LENGTH_BITS)
      start -= DMR_FRAME_LENGTH_BITS;
    buffer[i] |= ((m_buffer[start++] & 0x01) << 5);
    if (start >= DMR_FRAME_LENGTH_BITS)
      start -= DMR_FRAME_LENGTH_BITS;
    buffer[i] |= ((m_buffer[start++] & 0x01) << 4);
    if (start >= DMR_FRAME_LENGTH_BITS)
      start -= DMR_FRAME_LENGTH_BITS;
    buffer[i] |= ((m_buffer[start++] & 0x01) << 3);
    if (start >= DMR_FRAME_LENGTH_BITS)
      start -= DMR_FRAME_LENGTH_BITS;
    buffer[i] |= ((m_buffer[start++] & 0x01) << 2);
    if (start >= DMR_FRAME_LENGTH_BITS)
      start -= DMR_FRAME_LENGTH_BITS;
    buffer[i] |= ((m_buffer[start++] & 0x01) << 1);
    if (start >= DMR_FRAME_LENGTH_BITS)
      start -= DMR_FRAME_LENGTH_BITS;
    buffer[i] |= ((m_buffer[start++] & 0x01) << 0);
    if (start >= DMR_FRAME_LENGTH_BITS)
      start -= DMR_FRAME_LENGTH_BITS;
  }
}

void CDMRIdleRX::setColorCode(uint8_t colorCode)
{
  m_colorCode = colorCode;
}

#endif
