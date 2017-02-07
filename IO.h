/*
 *   Copyright (C) 2015, 2016 by Jonathan Naylor G4KLX
 *   Copyright (C) 2016, 2017 by Andy Uribe CA6JAU
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

#if !defined(CIO_H)
#define  CIO_H

#include "Globals.h"
#include "BitRB.h"

#define LOW      0
#define HIGH     1

#define VHF_MIN  144000000
#define VHF_MAX  148000000
#define UHF_MIN  430000000
#define UHF_MAX  450000000

extern uint32_t  m_frequency_rx;
extern uint32_t  m_frequency_tx;
extern uint8_t   m_power;

class CIO {

public:
  CIO();

  // Platform API
  void  Init(void);
  void  SCLK_pin(bool on);
  void  SDATA_pin(bool on);
  void  SLE_pin(bool on);
  bool  RXD_pin();
  void  TXD_pin(bool on);
  void  PTT_pin(bool on);
  void  LED_pin(bool on);
  void  DEB_pin(bool on);
  void  DSTAR_pin(bool on);
  void  DMR_pin(bool on);
  void  YSF_pin(bool on);
  void  P25_pin(bool on);
  void  COS_pin(bool on);
  void  interrupt(void);

  // IO API
  void  write(uint8_t* data, uint16_t length);
  uint16_t  getSpace() const;
  void  process();
  bool  hasTXOverflow();
  bool  hasRXOverflow();
  uint8_t  setFreq(uint32_t frequency_rx, uint32_t frequency_tx);
  void  setMode();
  void  setDecode(bool dcd);

  // RF interface API
  void  setTX();
  void  setRX();
  void  ifConf();
  void  start();
  void  startInt();

  // Misc functions
  void  dlybit(void);
  void  delay_rx(void);
     
private:
  bool      m_started;
  CBitRB    m_rxBuffer;
  CBitRB    m_txBuffer;

};

#endif
