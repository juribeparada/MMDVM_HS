/*
 *   Copyright (C) 2009-2015 by Jonathan Naylor G4KLX
 *   Copyright (C) 2018 by Andy Uribe CA6JAU
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

#if !defined(YSFDEFINES_H)
#define  YSFDEFINES_H

const unsigned int YSF_FRAME_LENGTH_BYTES = 120U;
const unsigned int YSF_FRAME_LENGTH_BITS  = YSF_FRAME_LENGTH_BYTES * 8U;

const unsigned int YSF_SYNC_LENGTH_BYTES  = 5U;
const unsigned int YSF_SYNC_LENGTH_BITS   = YSF_SYNC_LENGTH_BYTES * 8U;

const unsigned int YSF_FICH_LENGTH_BITS   = 200U;

const uint8_t YSF_SYNC_BYTES[] = {0xD4U, 0x71U, 0xC9U, 0x63U, 0x4DU};
const uint8_t YSF_SYNC_BYTES_LENGTH  = 5U;

const uint64_t YSF_SYNC_BITS      = 0x000000D471C9634DU;
const uint64_t YSF_SYNC_BITS_MASK = 0x000000FFFFFFFFFFU;

#endif

