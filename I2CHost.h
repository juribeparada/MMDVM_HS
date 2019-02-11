/*
 *   Copyright (C) 2019 by Andy Uribe CA6JAU
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

#if !defined(I2CHost_H)
#define  I2CHost_H

#include "Config.h"

#if defined(STM32_I2C_HOST)

#define I2C_CLK_FREQ       100000U
#define I2C_TX_FIFO_SIZE   512U
#define I2C_RX_FIFO_SIZE   512U

class CI2CHost {
public:
  CI2CHost();

  void    Init(void);
  void    I2C_EVHandler(void);
  uint8_t AvailI2C(void);
  uint8_t ReadI2C(void);
  void    WriteI2C(const uint8_t* data, uint16_t length);

private:
  void     I2C2_ClearFlag(void);
  uint16_t txFIFO_level(void);
  uint16_t rxFIFO_level(void);
  uint8_t  txFIFO_put(uint8_t next);

  volatile uint8_t  txFIFO[I2C_TX_FIFO_SIZE];
  volatile uint8_t  rxFIFO[I2C_RX_FIFO_SIZE];
  volatile uint16_t txFIFO_head, txFIFO_tail;
  volatile uint16_t rxFIFO_head, rxFIFO_tail;

};

#endif

#endif
