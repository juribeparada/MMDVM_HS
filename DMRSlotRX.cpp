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

#define  WANT_DEBUG

#include "Config.h"

#if defined(DUPLEX)

#include "Globals.h"
#include "DMRSlotRX.h"
#include "DMRSlotType.h"
#include "Utils.h"

const uint16_t SCAN_START = 400U;
const uint16_t SCAN_END   = 490U;

const uint8_t MAX_SYNC_BYTES_ERRS   = 1U;

const uint8_t MAX_SYNC_LOST_FRAMES  = 13U;

const uint16_t NOENDPTR = 9999U;

const uint8_t CONTROL_NONE  = 0x00U;
const uint8_t CONTROL_VOICE = 0x20U;
const uint8_t CONTROL_DATA  = 0x40U;

CDMRSlotRX::CDMRSlotRX(bool slot) :
m_slot(slot),
m_patternBuffer(0x00U),
m_buffer(),
m_dataPtr(0U),
m_syncPtr(0U),
m_startPtr(0U),
m_endPtr(NOENDPTR),
m_delayPtr(0U),
m_control(CONTROL_NONE),
m_syncCount(0U),
m_colorCode(0U),
m_delay(0U),
m_state(DMRRXS_NONE),
m_n(0U),
m_type(0U)
{
}

void CDMRSlotRX::start()
{
  m_dataPtr  = 0U;
  m_delayPtr = 0U;
  m_control  = CONTROL_NONE;
}

void CDMRSlotRX::reset()
{
  m_syncPtr   = 0U;
  m_dataPtr   = 0U;
  m_delayPtr  = 0U;
  m_control   = CONTROL_NONE;
  m_syncCount = 0U;
  m_state     = DMRRXS_NONE;
  m_startPtr  = 0U;
  m_endPtr    = NOENDPTR;
}

bool CDMRSlotRX::databit(bool bit)
{
  m_delayPtr++;
  if (m_delayPtr < m_delay)
    return m_state != DMRRXS_NONE;

  // Ensure that the buffer doesn't overflow
  if (m_dataPtr > m_endPtr || m_dataPtr >= 900U)
    return m_state != DMRRXS_NONE;

  m_buffer[m_dataPtr] = bit;
  
  m_patternBuffer <<= 1;
  if (bit)
    m_patternBuffer |= 0x01U;

  if (m_state == DMRRXS_NONE) {
    if (m_dataPtr >= SCAN_START && m_dataPtr <= SCAN_END)
      correlateSync(true);
  } else {

    uint16_t min = m_syncPtr - 1U;
    uint16_t max = m_syncPtr + 1U;
    if (m_dataPtr >= min && m_dataPtr <= max)
      correlateSync(false);
  }

  if (m_dataPtr == m_endPtr) {
    uint8_t frame[DMR_FRAME_LENGTH_BYTES + 3U];
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
            DEBUG3("DMRSlotRX: data header found slot/pos", m_slot ? 2U : 1U, m_syncPtr);
            writeRSSIData(frame);
            m_state = DMRRXS_DATA;
            m_type  = 0x00U;
            break;
          case DT_RATE_12_DATA:
          case DT_RATE_34_DATA:
          case DT_RATE_1_DATA:
            if (m_state == DMRRXS_DATA) {
              DEBUG3("DMRSlotRX: data payload found slot/pos", m_slot ? 2U : 1U, m_syncPtr);
              writeRSSIData(frame);
              m_type = dataType;
            }
            break;
          case DT_VOICE_LC_HEADER:
            DEBUG3("DMRSlotRX: voice header found slot/pos", m_slot ? 2U : 1U, m_syncPtr);
            writeRSSIData(frame);
            m_state = DMRRXS_VOICE;
            break;
          case DT_VOICE_PI_HEADER:
            if (m_state == DMRRXS_VOICE) {
              DEBUG3("DMRSlotRX: voice pi header found slot/pos", m_slot ? 2U : 1U, m_syncPtr);
              writeRSSIData(frame);
            }
            m_state = DMRRXS_VOICE;
            break;
          case DT_TERMINATOR_WITH_LC:
            if (m_state == DMRRXS_VOICE) {
              DEBUG3("DMRSlotRX: voice terminator found slot/pos", m_slot ? 2U : 1U, m_syncPtr);
              writeRSSIData(frame);
              m_state  = DMRRXS_NONE;
              m_endPtr = NOENDPTR;
            }
            break;
          default:    // DT_CSBK
            DEBUG3("DMRSlotRX: csbk found slot/pos", m_slot ? 2U : 1U, m_syncPtr);
            writeRSSIData(frame);
            m_state  = DMRRXS_NONE;
            m_endPtr = NOENDPTR;
            break;
        }
      }
    } else if (m_control == CONTROL_VOICE) {
      // Voice sync
      DEBUG3("DMRSlotRX: voice sync found slot/pos", m_slot ? 2U : 1U, m_syncPtr);
      writeRSSIData(frame);
      m_state     = DMRRXS_VOICE;
      m_syncCount = 0U;
      m_n         = 0U;
    } else {
      if (m_state != DMRRXS_NONE) {
        m_syncCount++;
        if (m_syncCount >= MAX_SYNC_LOST_FRAMES) {
          serial.writeDMRLost(m_slot);
          m_state  = DMRRXS_NONE;
          m_endPtr = NOENDPTR;
        }
      }

      if (m_state == DMRRXS_VOICE) {
        if (m_n >= 5U) {
          frame[0U] = CONTROL_VOICE;
          m_n = 0U;
        } else {
          frame[0U] = ++m_n;
        }

        serial.writeDMRData(m_slot, frame, DMR_FRAME_LENGTH_BYTES + 1U);
      } else if (m_state == DMRRXS_DATA) {
        if (m_type != 0x00U) {
          frame[0U] = CONTROL_DATA | m_type;
          writeRSSIData(frame);
        }
      }
    }
  }

  m_dataPtr++;

  return m_state != DMRRXS_NONE;
}

void CDMRSlotRX::correlateSync(bool first)
{  
  if (countBits64((m_patternBuffer & DMR_SYNC_BITS_MASK) ^ DMR_MS_DATA_SYNC_BITS) <= MAX_SYNC_BYTES_ERRS) {

  m_control = CONTROL_DATA;
  m_syncPtr = m_dataPtr;
  m_startPtr = m_dataPtr - DMR_SLOT_TYPE_LENGTH_BITS / 2U - DMR_INFO_LENGTH_BITS / 2U - DMR_SYNC_LENGTH_BITS + 1;
  m_endPtr   = m_dataPtr + DMR_SLOT_TYPE_LENGTH_BITS / 2U + DMR_INFO_LENGTH_BITS / 2U;
  DEBUG4("SYNC MS Data found pos/start/end:", m_dataPtr, m_startPtr, m_endPtr);
  
  } else if (countBits64((m_patternBuffer & DMR_SYNC_BITS_MASK) ^ DMR_MS_VOICE_SYNC_BITS) <= MAX_SYNC_BYTES_ERRS) {

  m_control  = CONTROL_VOICE;
  m_syncPtr  = m_dataPtr;
  m_startPtr = m_dataPtr - DMR_SLOT_TYPE_LENGTH_BITS / 2U - DMR_INFO_LENGTH_BITS / 2U - DMR_SYNC_LENGTH_BITS + 1;
  m_endPtr   = m_dataPtr + DMR_SLOT_TYPE_LENGTH_BITS / 2U + DMR_INFO_LENGTH_BITS / 2U;
  DEBUG4("SYNC MS Voice found pos/start/end: ", m_dataPtr, m_startPtr, m_endPtr);
  }
}

void CDMRSlotRX::bitsToBytes(uint16_t start, uint8_t count, uint8_t* buffer)
{
  for (uint8_t i = 0U; i < count; i++) {
    buffer[i]  = 0U;
    buffer[i] |= ((m_buffer[start + 0U] & 0x01) << 7);
    buffer[i] |= ((m_buffer[start + 1U] & 0x01) << 6);
    buffer[i] |= ((m_buffer[start + 2U] & 0x01) << 5);
    buffer[i] |= ((m_buffer[start + 3U] & 0x01) << 4);
    buffer[i] |= ((m_buffer[start + 4U] & 0x01) << 3);
    buffer[i] |= ((m_buffer[start + 5U] & 0x01) << 2);
    buffer[i] |= ((m_buffer[start + 6U] & 0x01) << 1);
    buffer[i] |= ((m_buffer[start + 7U] & 0x01) << 0);

    start += 8U;
  }
}

void CDMRSlotRX::setColorCode(uint8_t colorCode)
{
  m_colorCode = colorCode;
}

void CDMRSlotRX::setDelay(uint8_t delay)
{
  m_delay = delay;
}

void CDMRSlotRX::writeRSSIData(uint8_t* frame)
{
#if defined(SEND_RSSI_DATA)
  uint16_t rssi = io.readRSSI();
  
  frame[34U] = (rssi >> 8) & 0xFFU;
  frame[35U] = (rssi >> 0) & 0xFFU;
  
  serial.writeDMRData(m_slot, frame, DMR_FRAME_LENGTH_BYTES + 3U);
#else
  serial.writeDMRData(m_slot, frame, DMR_FRAME_LENGTH_BYTES + 1U);
#endif
}

#endif
