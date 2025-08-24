/*
 *   Copyright (C) 2016,2017,2018,2020,2021 by Jonathan Naylor G4KLX
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

#if !defined(M17DEFINES_H)
#define  M17DEFINES_H

const unsigned int M17_RADIO_SYMBOL_LENGTH = 5U;      // At 24 kHz sample rate

const unsigned int M17_FRAME_LENGTH_BITS    = 384U;
const unsigned int M17_FRAME_LENGTH_BYTES   = M17_FRAME_LENGTH_BITS / 8U;

const unsigned int M17_SYNC_LENGTH_BITS    = 16U;
const unsigned int M17_SYNC_LENGTH_BYTES   = M17_SYNC_LENGTH_BITS / 8U;

const uint8_t M17_LINK_SETUP_SYNC_BYTES[] = {0x55U, 0xF7U};
const uint8_t M17_STREAM_SYNC_BYTES[]     = {0xFFU, 0x5DU};
const uint8_t M17_PACKET_SYNC_BYTES[]     = {0x75U, 0xFFU};
const uint8_t M17_EOT_SYNC_BYTES[]        = {0x55U, 0x5DU};

const uint8_t M17_SYNC_BYTES_LENGTH = 2U;

const uint16_t M17_LINK_SETUP_SYNC_BITS = 0x55F7U;
const uint16_t M17_STREAM_SYNC_BITS     = 0xFF5DU;
const uint16_t M17_PACKET_SYNC_BITS     = 0x75FFU;
const uint16_t M17_EOT_SYNC_BITS        = 0x555DU;

#endif

