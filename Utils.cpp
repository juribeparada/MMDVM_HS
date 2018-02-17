/*
 *   Copyright (C) 2015 by Jonathan Naylor G4KLX
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

#include "Utils.h"

const uint8_t BITS_TABLE[] = {
#   define B2(n) n,     n+1,     n+1,     n+2
#   define B4(n) B2(n), B2(n+1), B2(n+1), B2(n+2)
#   define B6(n) B4(n), B4(n+1), B4(n+1), B4(n+2)
    B6(0), B6(1), B6(1), B6(2)
};

uint8_t countBits8(uint8_t bits)
{
  return BITS_TABLE[bits];
}

uint8_t countBits32(uint32_t bits)
{
  uint8_t* p = (uint8_t*)&bits;
  uint8_t n = 0U;
  n += BITS_TABLE[p[0U]];
  n += BITS_TABLE[p[1U]];
  n += BITS_TABLE[p[2U]];
  n += BITS_TABLE[p[3U]];
  return n;
}

uint8_t countBits64(uint64_t bits)
{
  uint8_t* p = (uint8_t*)&bits;
  uint8_t n = 0U;
  n += BITS_TABLE[p[0U]];
  n += BITS_TABLE[p[1U]];
  n += BITS_TABLE[p[2U]];
  n += BITS_TABLE[p[3U]];
  n += BITS_TABLE[p[4U]];
  n += BITS_TABLE[p[5U]];
  n += BITS_TABLE[p[6U]];
  n += BITS_TABLE[p[7U]];
  return n;
}

#if defined(ENABLE_DEBUG)
// Simple functions to convert from int to string
// Example from: https://stackoverflow.com/questions/8257714/how-to-convert-an-int-to-string-in-c
static uint8_t *i2str_helper(uint8_t *dest, uint32_t n, int32_t x) {
  if (n == 0) {
    return NULL;
  }
  if (x <= -10) {
    dest = i2str_helper(dest, n - 1, x / 10);
    if (dest == NULL) return NULL;
  }
  *dest = (uint8_t) ('0' - x % 10);
  return dest + 1;
}

uint8_t *i2str(uint8_t *dest, uint32_t n, int32_t x) {
  uint8_t *p = dest;
  if (n == 0) {
    return NULL;
  }
  n--;
  x = -x;
  p = i2str_helper(p, n, x);
  if (p == NULL) return NULL;
  *p = 0;
  return dest;
}
#endif
