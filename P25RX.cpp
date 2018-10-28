/*
 *   Copyright (C) 2016,2017 by Jonathan Naylor G4KLX
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
#include "P25RX.h"
#include "Utils.h"

const uint8_t SYNC_BIT_START_ERRS = 2U;
const uint8_t SYNC_BIT_RUN_ERRS   = 4U;

const unsigned int MAX_SYNC_FRAMES = 3U + 1U;

const uint8_t BIT_MASK_TABLE[] = {0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U};

#define WRITE_BIT1(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE[(i)&7])

const uint16_t NOENDPTR = 9999U;

CP25RX::CP25RX() :
m_state(P25RXS_NONE),
m_bitBuffer(0x00U),
m_outBuffer(),
m_buffer(NULL),
m_bufferPtr(0U),
m_endPtr(NOENDPTR),
m_lostCount(0U),
m_duid(0U)
{
  m_buffer = m_outBuffer + 1U;
}

void CP25RX::reset()
{
  m_state     = P25RXS_NONE;
  m_bitBuffer = 0x00U;
  m_bufferPtr = 0U;
  m_endPtr    = NOENDPTR;
  m_lostCount = 0U;
  m_duid      = 0U;
}

void CP25RX::databit(bool bit)
{
  switch (m_state) {
    case P25RXS_HDR:
      processHdr(bit);
      break;
    case P25RXS_LDU:
      processLdu(bit);
      break;
    default:
      processNone(bit);
      break;
  }
}

void CP25RX::processNone(bool bit)
{
  m_bitBuffer <<= 1;
  if (bit)
    m_bitBuffer |= 0x01U;

  // Fuzzy matching of the data sync bit sequence
  if (countBits64((m_bitBuffer & P25_SYNC_BITS_MASK) ^ P25_SYNC_BITS) <= SYNC_BIT_START_ERRS) {
    DEBUG1("P25RX: sync found in None");
    for (uint8_t i = 0U; i < P25_SYNC_LENGTH_BYTES; i++)
      m_buffer[i] = P25_SYNC_BYTES[i];

    m_lostCount = MAX_SYNC_FRAMES;
    m_bufferPtr = P25_SYNC_LENGTH_BITS;
    m_state     = P25RXS_HDR;

    io.setDecode(true);
  }
}

void CP25RX::processHdr(bool bit)
{
  m_bitBuffer <<= 1;
  if (bit)
    m_bitBuffer |= 0x01U;

  WRITE_BIT1(m_buffer, m_bufferPtr, bit);

  m_bufferPtr++;
  if (m_bufferPtr > P25_LDU_FRAME_LENGTH_BITS)
    reset();

  if (m_bufferPtr == P25_SYNC_LENGTH_BITS + 16U) {
    // FIXME: we should check and correct for errors in NID first!
    m_duid = m_buffer[7U] & 0x0F;

    if (m_duid != P25_DUID_HDU && m_duid != P25_DUID_TSDU &&
        m_duid != P25_DUID_TDU && m_duid != P25_DUID_TDULC) {
      m_lostCount = MAX_SYNC_FRAMES;
      m_state     = P25RXS_LDU;
      return;
    }

    setEndPtr();
    DEBUG2("P25RX: DUID", m_duid);
  }

  // Search for end of header frame
  if (m_bufferPtr == m_endPtr) {
    m_outBuffer[0U] = 0x01U;
    serial.writeP25Hdr(m_outBuffer, (m_endPtr / 8U) + 1U);

    m_lostCount = MAX_SYNC_FRAMES;
    m_bufferPtr = 0U;
    m_state     = P25RXS_LDU;
  }
}

void CP25RX::processLdu(bool bit)
{
  m_bitBuffer <<= 1;
  if (bit)
    m_bitBuffer |= 0x01U;

  WRITE_BIT1(m_buffer, m_bufferPtr, bit);

  m_bufferPtr++;
  if (m_bufferPtr > P25_LDU_FRAME_LENGTH_BITS)
    reset();

  // Only search for a sync in the right place +-2 bits
  if (m_bufferPtr >= (P25_SYNC_LENGTH_BITS - 2U) && m_bufferPtr <= (P25_SYNC_LENGTH_BITS + 2U)) {
    // Fuzzy matching of the data sync bit sequence
    if (countBits64((m_bitBuffer & P25_SYNC_BITS_MASK) ^ P25_SYNC_BITS) <= SYNC_BIT_RUN_ERRS) {
      DEBUG1("P25RX: found sync in LDU");
      m_lostCount = MAX_SYNC_FRAMES;
      m_bufferPtr = P25_SYNC_LENGTH_BITS;
    }
  }

  if (m_bufferPtr == P25_SYNC_LENGTH_BITS + 16U) {
    // We use DUID here only to detect TDU for EOT
    // FIXME: we should check and correct for errors in NID first!
    m_duid = m_buffer[7U] & 0x0F;
    setEndPtr();
    DEBUG2("P25RX: DUID", m_duid);
  }

  // Send a data frame to the host if the required number of bits have been received
  if (m_bufferPtr == P25_LDU_FRAME_LENGTH_BITS) {
    m_lostCount--;
    // We've not seen a data sync for too long, signal RXLOST and change to RX_NONE
    if (m_lostCount == 0U) {
      DEBUG1("P25RX: sync timed out, lost lock");
      io.setDecode(false);
      serial.writeP25Lost();
      reset();
    } else {
      // Write data to host
      m_outBuffer[0U] = m_lostCount == (MAX_SYNC_FRAMES - 1U) ? 0x01U : 0x00U;
      writeRSSILdu(m_outBuffer);

      // Start the next frame
      ::memset(m_outBuffer, 0x00U, P25_LDU_FRAME_LENGTH_BYTES + 3U);
      m_bufferPtr = 0U;
    }

    // Check if we found a TDU to avoid a false "lost lock"
    if (m_duid == P25_DUID_TDU || m_duid == P25_DUID_TDULC) {
      reset();
    }
  }
}

void CP25RX::writeRSSILdu(uint8_t* ldu)
{
#if defined(SEND_RSSI_DATA)
  uint16_t rssi = io.readRSSI();

  ldu[217U] = (rssi >> 8) & 0xFFU;
  ldu[218U] = (rssi >> 0) & 0xFFU;

  serial.writeP25Ldu(ldu, P25_LDU_FRAME_LENGTH_BYTES + 3U);
#else
  serial.writeP25Ldu(ldu, P25_LDU_FRAME_LENGTH_BYTES + 1U);
#endif
}

void CP25RX::setEndPtr()
{
  switch (m_duid) {
    case P25_DUID_HDU:
      DEBUG1("P25RX: sync found in HDU");
      m_endPtr = P25_HDR_FRAME_LENGTH_BITS;
      break;
    case P25_DUID_TDU:
      DEBUG1("P25RX: sync found in TDU");
      m_endPtr = P25_TERM_FRAME_LENGTH_BITS;
      break;
    case P25_DUID_TSDU:
      DEBUG1("P25RX: sync found in TSDU");
      m_endPtr = P25_TSDU_FRAME_LENGTH_BITS;
      break;
    case P25_DUID_PDU:
      // FIXME: not sure about PDU lengths since they have arbitrary length...
      DEBUG1("P25RX: sync found in PDU");
      m_endPtr = P25_PDU_HDR_FRAME_LENGTH_BITS;
      break;
    case P25_DUID_TDULC:
      DEBUG1("P25RX: sync found in TDULC");
      m_endPtr = P25_TERMLC_FRAME_LENGTH_BITS;
      break;
    default:
      m_endPtr = P25_LDU_FRAME_LENGTH_BITS;
      break;
  }
}
