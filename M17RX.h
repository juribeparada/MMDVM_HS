/*
 *   Copyright (C) 2015-2018,2020,2021 by Jonathan Naylor G4KLX
 *   Copyright (C) 2016-2018 by Andy Uribe CA6JAU
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

#if !defined(M17RX_H)
#define  M17RX_H

#include "M17Defines.h"

enum M17RX_STATE {
  M17RXS_NONE,
  M17RXS_LINK_SETUP,
  M17RXS_STREAM,
  M17RXS_PACKET
};

class CM17RX {
public:
  CM17RX();

  void databit(bool bit);

  void reset();

private:
  M17RX_STATE m_state;
  uint16_t    m_bitBuffer;
  uint8_t     m_outBuffer[M17_FRAME_LENGTH_BYTES + 3U];
  uint8_t*    m_buffer;
  uint16_t    m_bufferPtr;
  uint16_t    m_lostCount;

  void processNone(bool bit);
  void processData(bool bit);
  void writeRSSILinkSetup(uint8_t* data);
  void writeRSSIStream(uint8_t* data);
  void writeRSSIPacket(uint8_t* data);
};

#endif

