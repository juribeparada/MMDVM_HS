/*
 *   Copyright (C) 2009-2016 by Jonathan Naylor G4KLX
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
#include "DMRDMORX.h"
#include "DMRSlotType.h"
#include "Utils.h"

const uint8_t MAX_SYNC_BYTES_ERRS   = 3U;

const uint8_t MAX_SYNC_LOST_FRAMES  = 13U;

const uint16_t NOENDPTR = 9999U;

const uint8_t CONTROL_NONE  = 0x00U;
const uint8_t CONTROL_VOICE = 0x20U;
const uint8_t CONTROL_DATA  = 0x40U;

const uint8_t BIT_MASK_TABLE[] = {0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U};

#define WRITE_BIT1(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE[(i)&7])
#define READ_BIT1(p,i)    ((p[(i)>>3] & BIT_MASK_TABLE[(i)&7]) >> (7 - ((i)&7)))

CDMRDMORX::CDMRDMORX() :
m_patternBuffer(0x00U),
m_buffer(),
m_dataPtr(0U),
m_syncPtr(0U),
m_startPtr(0U),
m_endPtr(NOENDPTR),
m_control(CONTROL_NONE),
m_syncCount(0U),
m_colorCode(0U),
m_state(DMORXS_NONE),
m_n(0U),
m_type(0U)
{
}

void CDMRDMORX::reset()
{
  m_syncPtr   = 0U;
  m_control   = CONTROL_NONE;
  m_syncCount = 0U;
  m_state     = DMORXS_NONE;
  m_startPtr  = 0U;
  m_endPtr    = NOENDPTR;
}

void CDMRDMORX::databit(bool bit)
{
  WRITE_BIT1(m_buffer, m_dataPtr, bit);

  m_patternBuffer <<= 1;
  if (bit)
    m_patternBuffer |= 0x01U;

  if (m_state == DMORXS_NONE) {
    correlateSync();
  } else {

    uint16_t min  = m_syncPtr + DMO_BUFFER_LENGTH_BITS - 2;
    uint16_t max  = m_syncPtr + 2;

    if (min >= DMO_BUFFER_LENGTH_BITS)
      min -= DMO_BUFFER_LENGTH_BITS;
    if (max >= DMO_BUFFER_LENGTH_BITS)
      max -= DMO_BUFFER_LENGTH_BITS;

    if (min < max) {
      if (m_dataPtr >= min && m_dataPtr <= max)
        correlateSync();
    } else {
      if (m_dataPtr >= min || m_dataPtr <= max)
        correlateSync();
    }
  }

  if (m_dataPtr == m_endPtr) {
    frame[0U] = m_control;

    bitsToBytes(m_startPtr, DMR_FRAME_LENGTH_BYTES, frame + 1U);

    if (m_control == CONTROL_DATA) {
      // Data sync
      uint8_t colorCode;
      uint8_t dataType;
      CDMRSlotType slotType;
      slotType.decode(frame + 1U, colorCode, dataType);

      if (colorCode == m_colorCode) {
        m_syncCount = 0U;
        m_n         = 0U;

        frame[0U] |= dataType;

        switch (dataType) {
          case DT_DATA_HEADER:
            DEBUG2("DMRDMORX: data header found pos", m_syncPtr);
            writeRSSIData(frame);
            m_state = DMORXS_DATA;
            m_type  = 0x00U;
            break;
          case DT_RATE_12_DATA:
          case DT_RATE_34_DATA:
          case DT_RATE_1_DATA:
            if (m_state == DMORXS_DATA) {
              DEBUG2("DMRDMORX: data payload found pos", m_syncPtr);
              writeRSSIData(frame);
              m_type = dataType;
            }
            break;
          case DT_VOICE_LC_HEADER:
            DEBUG2("DMRDMORX: voice header found pos", m_syncPtr);
            writeRSSIData(frame);
            m_state = DMORXS_VOICE;
            break;
          case DT_VOICE_PI_HEADER:
            if (m_state == DMORXS_VOICE) {
              DEBUG2("DMRDMORX: voice pi header found pos", m_syncPtr);
              writeRSSIData(frame);
            }
            m_state = DMORXS_VOICE;
            break;
          case DT_TERMINATOR_WITH_LC:
            if (m_state == DMORXS_VOICE) {
              DEBUG2("DMRDMORX: voice terminator found pos", m_syncPtr);
              writeRSSIData(frame);
              reset();
            }
            break;
          default:    // DT_CSBK
            DEBUG2("DMRDMORX: csbk found pos", m_syncPtr);
            writeRSSIData(frame);
            reset();
            break;
        }
      }
    } else if (m_control == CONTROL_VOICE) {
      // Voice sync
      DEBUG2("DMRDMORX: voice sync found pos", m_syncPtr);
      writeRSSIData(frame);

      m_state     = DMORXS_VOICE;
      m_syncCount = 0U;
      m_n         = 0U;
    } else {
      if (m_state != DMORXS_NONE) {
        m_syncCount++;
        if (m_syncCount >= MAX_SYNC_LOST_FRAMES) {
          serial.writeDMRLost(true);
          reset();
        }
      }

      if (m_state == DMORXS_VOICE) {
        if (m_n >= 5U) {
          frame[0U] = CONTROL_VOICE;
          m_n = 0U;
        } else {
          frame[0U] = ++m_n;
        }
        serial.writeDMRData(true, frame, DMR_FRAME_LENGTH_BYTES + 1U);
      } else if (m_state == DMORXS_DATA) {
        if (m_type != 0x00U) {
          frame[0U] = CONTROL_DATA | m_type;
          writeRSSIData(frame);
        }
      }
    }

    // End of this slot, reset some items for the next slot.
    m_control = CONTROL_NONE;
  }

  m_dataPtr++;

  if (m_dataPtr >= DMO_BUFFER_LENGTH_BITS)
    m_dataPtr = 0U;

  io.setDecode(m_state != DMORXS_NONE);
}

void CDMRDMORX::correlateSync()
{
  if ( (countBits64((m_patternBuffer & DMR_SYNC_BITS_MASK) ^ DMR_MS_DATA_SYNC_BITS) <= MAX_SYNC_BYTES_ERRS) || \
    (countBits64((m_patternBuffer & DMR_SYNC_BITS_MASK) ^ DMR_S2_DATA_SYNC_BITS) <= MAX_SYNC_BYTES_ERRS) ) {
    m_control = CONTROL_DATA;
    m_syncPtr = m_dataPtr;

    m_startPtr = m_dataPtr + DMO_BUFFER_LENGTH_BITS - DMR_SLOT_TYPE_LENGTH_BITS / 2U - DMR_INFO_LENGTH_BITS / 2U - DMR_SYNC_LENGTH_BITS + 1;
    if (m_startPtr >= DMO_BUFFER_LENGTH_BITS)
      m_startPtr -= DMO_BUFFER_LENGTH_BITS;

    m_endPtr = m_dataPtr + DMR_SLOT_TYPE_LENGTH_BITS / 2U + DMR_INFO_LENGTH_BITS / 2U;
    if (m_endPtr >= DMO_BUFFER_LENGTH_BITS)
      m_endPtr -= DMO_BUFFER_LENGTH_BITS;

    m_modeTimerCnt = 0;
    //DEBUG4("SYNC MS Data found pos/start/end:", m_dataPtr, m_startPtr, m_endPtr);
  } else if ( (countBits64((m_patternBuffer & DMR_SYNC_BITS_MASK) ^ DMR_MS_VOICE_SYNC_BITS) <= MAX_SYNC_BYTES_ERRS) || \
    (countBits64((m_patternBuffer & DMR_SYNC_BITS_MASK) ^ DMR_S2_VOICE_SYNC_BITS) <= MAX_SYNC_BYTES_ERRS) ) {
    m_control  = CONTROL_VOICE;
    m_syncPtr  = m_dataPtr;

    m_startPtr = m_dataPtr + DMO_BUFFER_LENGTH_BITS - DMR_SLOT_TYPE_LENGTH_BITS / 2U - DMR_INFO_LENGTH_BITS / 2U - DMR_SYNC_LENGTH_BITS + 1;
    if (m_startPtr >= DMO_BUFFER_LENGTH_BITS)
      m_startPtr -= DMO_BUFFER_LENGTH_BITS;

    m_endPtr   = m_dataPtr + DMR_SLOT_TYPE_LENGTH_BITS / 2U + DMR_INFO_LENGTH_BITS / 2U;
    if (m_endPtr >= DMO_BUFFER_LENGTH_BITS)
      m_endPtr -= DMO_BUFFER_LENGTH_BITS;

    m_modeTimerCnt = 0;
    //DEBUG4("SYNC MS Voice found pos/start/end: ", m_dataPtr, m_startPtr, m_endPtr);
  }
}

void CDMRDMORX::bitsToBytes(uint16_t start, uint8_t count, uint8_t* buffer)
{
  for (uint8_t i = 0U; i < count; i++) {

  buffer[i]  = 0U;
  buffer[i] |= READ_BIT1(m_buffer, start) << 7;
  start++;
  if (start >= DMO_BUFFER_LENGTH_BITS)
    start -= DMO_BUFFER_LENGTH_BITS;
  buffer[i] |= READ_BIT1(m_buffer, start) << 6;
  start++;
  if (start >= DMO_BUFFER_LENGTH_BITS)
    start -= DMO_BUFFER_LENGTH_BITS;
  buffer[i] |= READ_BIT1(m_buffer, start) << 5;
  start++;
  if (start >= DMO_BUFFER_LENGTH_BITS)
    start -= DMO_BUFFER_LENGTH_BITS;
  buffer[i] |= READ_BIT1(m_buffer, start) << 4;
  start++;
  if (start >= DMO_BUFFER_LENGTH_BITS)
    start -= DMO_BUFFER_LENGTH_BITS;
  buffer[i] |= READ_BIT1(m_buffer, start) << 3;
  start++;
  if (start >= DMO_BUFFER_LENGTH_BITS)
    start -= DMO_BUFFER_LENGTH_BITS;
  buffer[i] |= READ_BIT1(m_buffer, start) << 2;
  start++;
  if (start >= DMO_BUFFER_LENGTH_BITS)
    start -= DMO_BUFFER_LENGTH_BITS;
  buffer[i] |= READ_BIT1(m_buffer, start) << 1;
  start++;
  if (start >= DMO_BUFFER_LENGTH_BITS)
    start -= DMO_BUFFER_LENGTH_BITS;
  buffer[i] |= READ_BIT1(m_buffer, start) << 0;
  start++;
  if (start >= DMO_BUFFER_LENGTH_BITS)
    start -= DMO_BUFFER_LENGTH_BITS;
  }
}

void CDMRDMORX::setColorCode(uint8_t colorCode)
{
  m_colorCode = colorCode;
}

void CDMRDMORX::writeRSSIData(uint8_t* frame)
{
#if defined(SEND_RSSI_DATA)
  uint16_t rssi = io.readRSSI();
  
  frame[34U] = (rssi >> 8) & 0xFFU;
  frame[35U] = (rssi >> 0) & 0xFFU;
  
  serial.writeDMRData(true, frame, DMR_FRAME_LENGTH_BYTES + 3U);
#else
  serial.writeDMRData(true, frame, DMR_FRAME_LENGTH_BYTES + 1U);
#endif
}

