/*
 *   Copyright (C) 2015,2016 by Jonathan Naylor G4KLX
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

#if !defined(DMRRX_H)
#define  DMRRX_H

#include "Config.h"

#if defined(DUPLEX)

#include "DMRSlotRX.h"

class CDMRRX {
public:
  CDMRRX();

  void databit(bool bit, const uint8_t control);

  void setColorCode(uint8_t colorCode);
  void setDelay(uint8_t delay);

  void reset();

private:
  CDMRSlotRX m_slotRX;
  uint8_t    m_control_old;
};

#endif

#endif
