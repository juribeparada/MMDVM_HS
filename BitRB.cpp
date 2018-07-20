/*
TX fifo control - Copyright (C) KI6ZUM 2015
Copyright (C) 2015,2016 by Jonathan Naylor G4KLX
Copyright (C) 2016,2017,2018 by Andy Uribe CA6JAU

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
Boston, MA  02110-1301, USA.
*/

#include "BitRB.h"

const uint8_t BIT_MASK_TABLE[] = {0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U};

#define WRITE_BIT1(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE[(i)&7])
#define READ_BIT1(p,i)    ((p[(i)>>3] & BIT_MASK_TABLE[(i)&7]) >> (7 - ((i)&7)))

CBitRB::CBitRB(uint16_t length) :
m_length(length),
m_bits(NULL),
m_control(NULL),
m_head(0U),
m_tail(0U),
m_full(false),
m_overflow(false)
{
  m_bits    = new uint8_t[length / 8U];
  m_control = new uint8_t[length / 8U];
}

uint16_t CBitRB::getSpace() const
{
  uint16_t n = 0U;

  if (m_tail == m_head)
    n = m_full ? 0U : m_length;
  else if (m_tail < m_head)
    n = m_length - m_head + m_tail;
  else
    n = m_tail - m_head;

  if (n > m_length)
    n = 0U;

  return n;
}

uint16_t CBitRB::getData() const
{
  if (m_tail == m_head)
    return m_full ? m_length : 0U;
  else if (m_tail < m_head)
    return m_head - m_tail;
  else
    return m_length - m_tail + m_head;
}

bool CBitRB::put(uint8_t bit, uint8_t control)
{
  if (m_full) {
    m_overflow = true;
    return false;
  }

  WRITE_BIT1(m_bits, m_head, bit);
  WRITE_BIT1(m_control, m_head, control);

  m_head++;
  if (m_head >= m_length)
    m_head = 0U;

  if (m_head == m_tail)
    m_full = true;

  return true;
}

bool CBitRB::get(uint8_t& bit, uint8_t& control)
{
  if (m_head == m_tail && !m_full)
    return false;

  bit     = READ_BIT1(m_bits, m_tail);
  control = READ_BIT1(m_control, m_tail);

  m_full = false;

  m_tail++;
  if (m_tail >= m_length)
    m_tail = 0U;

  return true;
}

bool CBitRB::hasOverflowed()
{
  bool overflow = m_overflow;

  m_overflow = false;

  return overflow;
}

