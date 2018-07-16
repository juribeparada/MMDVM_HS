/*
 *   Copyright (C) 2015,2016,2017 by Jonathan Naylor G4KLX
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

#if !defined(DMRSLOTRX_H)
#define  DMRSLOTRX_H

#include "Config.h"

#if defined(DUPLEX)

#include "DMRDefines.h"

const uint16_t DMR_BUFFER_LENGTH_BITS = 576U;

enum DMRRX_STATE {
  DMRRXS_NONE,
  DMRRXS_VOICE,
  DMRRXS_DATA
};

class CDMRSlotRX {
public:
  CDMRSlotRX();

  void start(bool slot);

  bool databit(bool bit);

  void setColorCode(uint8_t colorCode);
  void setDelay(uint8_t delay);

  void reset();

private:
  bool        m_slot;
  uint64_t    m_patternBuffer;
  uint8_t     m_buffer[DMR_BUFFER_LENGTH_BITS / 8U];  // 72 bytes
  uint16_t    m_dataPtr;

  uint8_t     frame1[DMR_FRAME_LENGTH_BYTES + 3U];
  uint16_t    m_syncPtr1;
  uint16_t    m_startPtr1;
  uint16_t    m_endPtr1;
  uint8_t     m_control1;
  uint8_t     m_syncCount1;
  DMRRX_STATE m_state1;
  uint8_t     m_n1;
  uint8_t     m_type1;

  uint8_t     frame2[DMR_FRAME_LENGTH_BYTES + 3U];
  uint16_t    m_syncPtr2;
  uint16_t    m_startPtr2;
  uint16_t    m_endPtr2;
  uint8_t     m_control2;
  uint8_t     m_syncCount2;
  DMRRX_STATE m_state2;
  uint8_t     m_n2;
  uint8_t     m_type2;

  uint16_t    m_delayPtr;
  uint8_t     m_colorCode;
  uint16_t    m_delay;

  void procSlot1();
  void procSlot2();
  void correlateSync();
  void bitsToBytes(uint16_t start, uint8_t count, uint8_t* buffer);
  void writeRSSIData1();
  void writeRSSIData2();
};

#endif

#endif
