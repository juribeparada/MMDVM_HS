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
#include "DMRSlotRX.h"
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

CDMRSlotRX::CDMRSlotRX() :
m_slot(false),
m_patternBuffer(0x00U),
m_buffer(),
m_dataPtr(0U),
m_syncPtr1(0U),
m_startPtr1(0U),
m_endPtr1(NOENDPTR),
m_control1(CONTROL_NONE),
m_syncCount1(0U),
m_state1(DMRRXS_NONE),
m_n1(0U),
m_type1(0U),
m_syncPtr2(0U),
m_startPtr2(0U),
m_endPtr2(NOENDPTR),
m_control2(CONTROL_NONE),
m_syncCount2(0U),
m_state2(DMRRXS_NONE),
m_n2(0U),
m_type2(0U),
m_delayPtr(0U),
m_colorCode(0U),
m_delay(0U)
{
}

void CDMRSlotRX::start(bool slot)
{
  m_slot = slot;
  m_delayPtr = 0U;
}

void CDMRSlotRX::reset()
{
  m_dataPtr   = 0U;
  m_delayPtr  = 0U;
  m_patternBuffer = 0U;

  m_syncPtr1   = 0U;
  m_control1   = CONTROL_NONE;
  m_syncCount1 = 0U;
  m_state1     = DMRRXS_NONE;
  m_startPtr1  = 0U;
  m_endPtr1    = NOENDPTR;
  m_type1      = 0U;
  m_n1         = 0U;

  
  m_syncPtr2   = 0U;
  m_control2   = CONTROL_NONE;
  m_syncCount2 = 0U;
  m_state2     = DMRRXS_NONE;
  m_startPtr2  = 0U;
  m_endPtr2    = NOENDPTR;
  m_type2      = 0U;
  m_n2         = 0U;
}

bool CDMRSlotRX::databit(bool bit)
{
  uint16_t min, max;

  m_delayPtr++;
  if (m_delayPtr < m_delay)
    return (m_state1 != DMRRXS_NONE) || (m_state2 != DMRRXS_NONE);

  WRITE_BIT1(m_buffer, m_dataPtr, bit);

  m_patternBuffer <<= 1;
  if (bit)
    m_patternBuffer |= 0x01U;
    
  if (m_state1 == DMRRXS_NONE || m_state2 == DMRRXS_NONE) {
    correlateSync();
  } else {
    if(m_slot) {
      min = m_syncPtr2 + DMR_BUFFER_LENGTH_BITS - 2;
      max = m_syncPtr2 + 2;
    } else {
      min = m_syncPtr1 + DMR_BUFFER_LENGTH_BITS - 2;
      max = m_syncPtr1 + 2;
    }

    if (min >= DMR_BUFFER_LENGTH_BITS)
      min -= DMR_BUFFER_LENGTH_BITS;
    if (max >= DMR_BUFFER_LENGTH_BITS)
      max -= DMR_BUFFER_LENGTH_BITS;

    if (min < max) {
      if (m_dataPtr >= min && m_dataPtr <= max)
        correlateSync();
    } else {
      if (m_dataPtr >= min || m_dataPtr <= max)
        correlateSync();
    }
  }

  if(m_slot)
    procSlot2();
  else
    procSlot1();

  m_dataPtr++;

  if (m_dataPtr >= DMR_BUFFER_LENGTH_BITS)
    m_dataPtr = 0U;

  return (m_state1 != DMRRXS_NONE) || (m_state2 != DMRRXS_NONE);;
}

void CDMRSlotRX::procSlot1()
{
  if (m_dataPtr == m_endPtr1) {
    frame1[0U] = m_control1;

    bitsToBytes(m_startPtr1, DMR_FRAME_LENGTH_BYTES, frame1 + 1U);

    if (m_control1 == CONTROL_DATA) {
      // Data sync
      uint8_t colorCode;
      uint8_t dataType;
      CDMRSlotType slotType;
      slotType.decode(frame1 + 1U, colorCode, dataType);

      if (colorCode == m_colorCode) {
        m_syncCount1 = 0U;
        m_n1         = 0U;

        frame1[0U] |= dataType;

        switch (dataType) {
          case DT_DATA_HEADER:
            DEBUG2("DMRSlot1RX: data header found pos", m_syncPtr1);
            writeRSSIData1();
            m_state1 = DMRRXS_DATA;
            m_type1  = 0x00U;
            break;
          case DT_RATE_12_DATA:
          case DT_RATE_34_DATA:
          case DT_RATE_1_DATA:
            if (m_state1 == DMRRXS_DATA) {
              DEBUG2("DMRSlot1RX: data payload found pos", m_syncPtr1);
              writeRSSIData1();
              m_type1 = dataType;
            }
            break;
          case DT_VOICE_LC_HEADER:
            DEBUG2("DMRSlot1RX: voice header found pos", m_syncPtr1);
            writeRSSIData1();
            m_state1 = DMRRXS_VOICE;
            break;
          case DT_VOICE_PI_HEADER:
            if (m_state1 == DMRRXS_VOICE) {
              DEBUG2("DMRSlot1RX: voice pi header found pos", m_syncPtr1);
              writeRSSIData1();
            }
            m_state1 = DMRRXS_VOICE;
            break;
          case DT_TERMINATOR_WITH_LC:
            if (m_state1 == DMRRXS_VOICE) {
              DEBUG2("DMRSlot1RX: voice terminator found pos", m_syncPtr1);
              writeRSSIData1();
              m_state1  = DMRRXS_NONE;
              m_endPtr1 = NOENDPTR;
            }
            break;
          default:    // DT_CSBK
            DEBUG2("DMRSlot1RX: csbk found pos", m_syncPtr1);
            writeRSSIData1();
            m_state1  = DMRRXS_NONE;
            m_endPtr1 = NOENDPTR;
            break;
        }
      }
    } else if (m_control1 == CONTROL_VOICE) {
      // Voice sync
      DEBUG2("DMRSlot1RX: voice sync found pos", m_syncPtr1);
      writeRSSIData1();
      m_state1     = DMRRXS_VOICE;
      m_syncCount1 = 0U;
      m_n1         = 0U;
    } else {
      if (m_state1 != DMRRXS_NONE) {
        m_syncCount1++;
        if (m_syncCount1 >= MAX_SYNC_LOST_FRAMES) {
          serial.writeDMRLost(0U);
          m_state1  = DMRRXS_NONE;
          m_endPtr1 = NOENDPTR;
        }
      }

      if (m_state1 == DMRRXS_VOICE) {
        if (m_n1 >= 5U) {
          frame1[0U] = CONTROL_VOICE;
          m_n1 = 0U;
        } else {
          frame1[0U] = ++m_n1;
        }

        serial.writeDMRData(0U, frame1, DMR_FRAME_LENGTH_BYTES + 1U);
      } else if (m_state1 == DMRRXS_DATA) {
        if (m_type1 != 0x00U) {
          frame1[0U] = CONTROL_DATA | m_type1;
          writeRSSIData1();
        }
      }
    }
    
    // End of this slot, reset some items for the next slot.
    m_control1 = CONTROL_NONE;
  }
}

void CDMRSlotRX::procSlot2()
{
  if (m_dataPtr == m_endPtr2) {
    frame2[0U] = m_control2;

    bitsToBytes(m_startPtr2, DMR_FRAME_LENGTH_BYTES, frame2 + 1U);

    if (m_control2 == CONTROL_DATA) {
      // Data sync
      uint8_t colorCode;
      uint8_t dataType;
      CDMRSlotType slotType;
      slotType.decode(frame2 + 1U, colorCode, dataType);

      if (colorCode == m_colorCode) {
        m_syncCount2 = 0U;
        m_n2         = 0U;

        frame2[0U] |= dataType;

        switch (dataType) {
          case DT_DATA_HEADER:
            DEBUG2("DMRSlot2RX: data header found pos", m_syncPtr2);
            writeRSSIData2();
            m_state2 = DMRRXS_DATA;
            m_type2  = 0x00U;
            break;
          case DT_RATE_12_DATA:
          case DT_RATE_34_DATA:
          case DT_RATE_1_DATA:
            if (m_state2 == DMRRXS_DATA) {
              DEBUG2("DMRSlot2RX: data payload found pos", m_syncPtr2);
              writeRSSIData2();
              m_type2 = dataType;
            }
            break;
          case DT_VOICE_LC_HEADER:
            DEBUG2("DMRSlot2RX: voice header found pos", m_syncPtr2);
            writeRSSIData2();
            m_state2 = DMRRXS_VOICE;
            break;
          case DT_VOICE_PI_HEADER:
            if (m_state2 == DMRRXS_VOICE) {
              DEBUG2("DMRSlot2RX: voice pi header found pos", m_syncPtr2);
              writeRSSIData2();
            }
            m_state2 = DMRRXS_VOICE;
            break;
          case DT_TERMINATOR_WITH_LC:
            if (m_state2 == DMRRXS_VOICE) {
              DEBUG2("DMRSlot2RX: voice terminator found pos", m_syncPtr2);
              writeRSSIData2();
              m_state2  = DMRRXS_NONE;
              m_endPtr2 = NOENDPTR;
            }
            break;
          default:    // DT_CSBK
            DEBUG2("DMRSlot2RX: csbk found pos", m_syncPtr2);
            writeRSSIData2();
            m_state2  = DMRRXS_NONE;
            m_endPtr2 = NOENDPTR;
            break;
        }
      }
    } else if (m_control2 == CONTROL_VOICE) {
      // Voice sync
      DEBUG2("DMRSlot2RX: voice sync found pos", m_syncPtr2);
      writeRSSIData2();
      m_state2     = DMRRXS_VOICE;
      m_syncCount2 = 0U;
      m_n2         = 0U;
    } else {
      if (m_state2 != DMRRXS_NONE) {
        m_syncCount2++;
        if (m_syncCount2 >= MAX_SYNC_LOST_FRAMES) {
          serial.writeDMRLost(1U);
          m_state2  = DMRRXS_NONE;
          m_endPtr2 = NOENDPTR;
        }
      }

      if (m_state2 == DMRRXS_VOICE) {
        if (m_n2 >= 5U) {
          frame2[0U] = CONTROL_VOICE;
          m_n2 = 0U;
        } else {
          frame2[0U] = ++m_n2;
        }

        serial.writeDMRData(1U, frame2, DMR_FRAME_LENGTH_BYTES + 1U);
      } else if (m_state2 == DMRRXS_DATA) {
        if (m_type2 != 0x00U) {
          frame2[0U] = CONTROL_DATA | m_type2;
          writeRSSIData2();
        }
      }
    }
    
    // End of this slot, reset some items for the next slot.
    m_control2 = CONTROL_NONE;
  }
}

void CDMRSlotRX::correlateSync()
{
  uint16_t syncPtr;
  uint16_t startPtr;
  uint16_t endPtr;
  uint8_t  control;

  if (countBits64((m_patternBuffer & DMR_SYNC_BITS_MASK) ^ DMR_MS_DATA_SYNC_BITS) <= MAX_SYNC_BYTES_ERRS) {
    control = CONTROL_DATA;
    syncPtr = m_dataPtr;

    startPtr = m_dataPtr + DMR_BUFFER_LENGTH_BITS - DMR_SLOT_TYPE_LENGTH_BITS / 2U - DMR_INFO_LENGTH_BITS / 2U - DMR_SYNC_LENGTH_BITS + 1;
    if (startPtr >= DMR_BUFFER_LENGTH_BITS)
      startPtr -= DMR_BUFFER_LENGTH_BITS;

    endPtr = m_dataPtr + DMR_SLOT_TYPE_LENGTH_BITS / 2U + DMR_INFO_LENGTH_BITS / 2U;
    if (endPtr >= DMR_BUFFER_LENGTH_BITS)
      endPtr -= DMR_BUFFER_LENGTH_BITS;

    if(m_slot) {
      m_syncPtr2 = syncPtr;
      m_startPtr2 = startPtr;
      m_endPtr2 = endPtr;
      m_control2 = control;
    } else {
      m_syncPtr1 = syncPtr;
      m_startPtr1 = startPtr;
      m_endPtr1 = endPtr;
      m_control1 = control;
    }
    //DEBUG5("SYNC corr MS Data found slot/pos/start/end:", m_slot ? 2U : 1U, m_dataPtr, startPtr, endPtr);
  } else if (countBits64((m_patternBuffer & DMR_SYNC_BITS_MASK) ^ DMR_MS_VOICE_SYNC_BITS) <= MAX_SYNC_BYTES_ERRS) {
    control  = CONTROL_VOICE;
    syncPtr  = m_dataPtr;

    startPtr = m_dataPtr + DMR_BUFFER_LENGTH_BITS - DMR_SLOT_TYPE_LENGTH_BITS / 2U - DMR_INFO_LENGTH_BITS / 2U - DMR_SYNC_LENGTH_BITS + 1;
    if (startPtr >= DMR_BUFFER_LENGTH_BITS)
      startPtr -= DMR_BUFFER_LENGTH_BITS;

    endPtr   = m_dataPtr + DMR_SLOT_TYPE_LENGTH_BITS / 2U + DMR_INFO_LENGTH_BITS / 2U;
    if (endPtr >= DMR_BUFFER_LENGTH_BITS)
      endPtr -= DMR_BUFFER_LENGTH_BITS;

    if(m_slot) {
      m_syncPtr2 = syncPtr;
      m_startPtr2 = startPtr;
      m_endPtr2 = endPtr;
      m_control2 = control;
    } else {
      m_syncPtr1 = syncPtr;
      m_startPtr1 = startPtr;
      m_endPtr1 = endPtr;
      m_control1 = control;
    }
    //DEBUG5("SYNC corr MS Voice found slot/pos/start/end: ", m_slot ? 2U : 1U, m_dataPtr, startPtr, endPtr);
  }
}

void CDMRSlotRX::bitsToBytes(uint16_t start, uint8_t count, uint8_t* buffer)
{
  for (uint8_t i = 0U; i < count; i++) {
    buffer[i]  = 0U;
    buffer[i] |= READ_BIT1(m_buffer, start) << 7;
    start++;
    if (start >= DMR_BUFFER_LENGTH_BITS)
      start -= DMR_BUFFER_LENGTH_BITS;
    buffer[i] |= READ_BIT1(m_buffer, start) << 6;
    start++;
    if (start >= DMR_BUFFER_LENGTH_BITS)
      start -= DMR_BUFFER_LENGTH_BITS;
    buffer[i] |= READ_BIT1(m_buffer, start) << 5;
    start++;
    if (start >= DMR_BUFFER_LENGTH_BITS)
      start -= DMR_BUFFER_LENGTH_BITS;
    buffer[i] |= READ_BIT1(m_buffer, start) << 4;
    start++;
    if (start >= DMR_BUFFER_LENGTH_BITS)
      start -= DMR_BUFFER_LENGTH_BITS;
    buffer[i] |= READ_BIT1(m_buffer, start) << 3;
    start++;
    if (start >= DMR_BUFFER_LENGTH_BITS)
      start -= DMR_BUFFER_LENGTH_BITS;
    buffer[i] |= READ_BIT1(m_buffer, start) << 2;
    start++;
    if (start >= DMR_BUFFER_LENGTH_BITS)
      start -= DMR_BUFFER_LENGTH_BITS;
    buffer[i] |= READ_BIT1(m_buffer, start) << 1;
    start++;
    if (start >= DMR_BUFFER_LENGTH_BITS)
      start -= DMR_BUFFER_LENGTH_BITS;
    buffer[i] |= READ_BIT1(m_buffer, start) << 0;
    start++;
    if (start >= DMR_BUFFER_LENGTH_BITS)
      start -= DMR_BUFFER_LENGTH_BITS;
  }
}

void CDMRSlotRX::setColorCode(uint8_t colorCode)
{
  m_colorCode = colorCode;
}

void CDMRSlotRX::setDelay(uint8_t delay)
{
  m_delay = delay / 5;
}

void CDMRSlotRX::writeRSSIData1()
{
#if defined(SEND_RSSI_DATA)
  uint16_t rssi = io.readRSSI();
  
  frame1[34U] = (rssi >> 8) & 0xFFU;
  frame1[35U] = (rssi >> 0) & 0xFFU;
  
  serial.writeDMRData(0U, frame1, DMR_FRAME_LENGTH_BYTES + 3U);
#else
  serial.writeDMRData(0U, frame1, DMR_FRAME_LENGTH_BYTES + 1U);
#endif
}

void CDMRSlotRX::writeRSSIData2()
{
#if defined(SEND_RSSI_DATA)
  uint16_t rssi = io.readRSSI();
  
  frame2[34U] = (rssi >> 8) & 0xFFU;
  frame2[35U] = (rssi >> 0) & 0xFFU;
  
  serial.writeDMRData(1U, frame2, DMR_FRAME_LENGTH_BYTES + 3U);
#else
  serial.writeDMRData(1U, frame2, DMR_FRAME_LENGTH_BYTES + 1U);
#endif
}

#endif
