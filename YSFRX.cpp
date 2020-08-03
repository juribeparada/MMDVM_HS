/*
 *   Copyright (C) 2009-2017 by Jonathan Naylor G4KLX
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
#include "YSFRX.h"
#include "Utils.h"

const uint8_t SYNC_BIT_START_ERRS = 2U;
const uint8_t SYNC_BIT_RUN_ERRS   = 4U;

const unsigned int MAX_SYNC_FRAMES = 1U + 1U;

const uint8_t BIT_MASK_TABLE[] = {0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U};

#define WRITE_BIT1(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE[(i)&7])

CYSFRX::CYSFRX() :
m_state(YSFRXS_NONE),
m_bitBuffer(0x00U),
m_outBuffer(),
m_buffer(NULL),
m_bufferPtr(0U),
m_lostCount(0U)
{
  m_buffer = m_outBuffer + 1U;
}

void CYSFRX::reset()
{
  m_state     = YSFRXS_NONE;
  m_bitBuffer = 0x00U;
  m_bufferPtr = 0U;
  m_lostCount = 0U;
}

void CYSFRX::databit(bool bit)
{
  if (m_state == YSFRXS_NONE)
    processNone(bit);
  else
    processData(bit);
}

void CYSFRX::processNone(bool bit)
{
  m_bitBuffer <<= 1;
  if (bit)
    m_bitBuffer |= 0x01U;

  // Fuzzy matching of the data sync bit sequence
  if (countBits64((m_bitBuffer & YSF_SYNC_BITS_MASK) ^ YSF_SYNC_BITS) <= SYNC_BIT_START_ERRS) {
    DEBUG1("YSFRX: sync found in None");
    for (uint8_t i = 0U; i < YSF_SYNC_LENGTH_BYTES; i++)
      m_buffer[i] = YSF_SYNC_BYTES[i];

    m_lostCount = MAX_SYNC_FRAMES;
    m_bufferPtr = YSF_SYNC_LENGTH_BITS;
    m_state     = YSFRXS_DATA;

    io.setDecode(true);
  }
  
}

void CYSFRX::processData(bool bit)
{
  m_bitBuffer <<= 1;
  if (bit)
    m_bitBuffer |= 0x01U;

  WRITE_BIT1(m_buffer, m_bufferPtr, bit);

  m_bufferPtr++;
  if (m_bufferPtr > YSF_FRAME_LENGTH_BITS)
    reset();

  // Only search for a sync in the right place +-2 bits
  if (m_bufferPtr >= (YSF_SYNC_LENGTH_BITS - 2U) && m_bufferPtr <= (YSF_SYNC_LENGTH_BITS + 2U)) {
    // Fuzzy matching of the data sync bit sequence
    if (countBits64((m_bitBuffer & YSF_SYNC_BITS_MASK) ^ YSF_SYNC_BITS) <= SYNC_BIT_RUN_ERRS) {
      DEBUG2("YSFRX: found sync in Data, pos", m_bufferPtr - YSF_SYNC_LENGTH_BITS);
      m_lostCount = MAX_SYNC_FRAMES;
      m_bufferPtr = YSF_SYNC_LENGTH_BITS;
    }
  }

  // Send a data frame to the host if the required number of bits have been received
  if (m_bufferPtr == YSF_FRAME_LENGTH_BITS) {
    m_lostCount--;
    // We've not seen a data sync for too long, signal RXLOST and change to RX_NONE
    if (m_lostCount == 0U) {
      DEBUG1("YSFRX: sync timed out, lost lock");
      io.setDecode(false);
      serial.writeYSFLost();
      reset();
    } else {
      // Write data to host
      m_outBuffer[0U] = m_lostCount == (MAX_SYNC_FRAMES - 1U) ? 0x01U : 0x00U;
      writeRSSIData(m_outBuffer);

      // Start the next frame
      ::memset(m_outBuffer, 0x00U, YSF_FRAME_LENGTH_BYTES + 3U);
      m_bufferPtr = 0U;
    }
  }
}

void CYSFRX::writeRSSIData(uint8_t* data)
{
#if defined(SEND_RSSI_DATA)
  uint16_t rssi = io.readRSSI();

  data[121U] = (rssi >> 8) & 0xFFU;
  data[122U] = (rssi >> 0) & 0xFFU;

  serial.writeYSFData(data, YSF_FRAME_LENGTH_BYTES + 3U);
#else
  serial.writeYSFData(data, YSF_FRAME_LENGTH_BYTES + 1U);
#endif
}
