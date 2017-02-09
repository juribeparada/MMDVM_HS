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
 
#include "Config.h"
#include "Globals.h"
#include "IO.h"

#if defined(ADF7021)
#include "ADF7021.h"
#endif

uint32_t    m_frequency_rx;
uint32_t    m_frequency_tx;
uint8_t     m_power;

CIO::CIO():
m_started(false),
m_rxBuffer(RX_RINGBUFFER_SIZE),
m_txBuffer(TX_RINGBUFFER_SIZE)
{
  Init();

  LED_pin(HIGH);
  PTT_pin(LOW);
  DSTAR_pin(LOW);
  DMR_pin(LOW);
  YSF_pin(LOW);
  P25_pin(LOW);
  COS_pin(LOW);
  DEB_pin(LOW);

#if !defined(BIDIR_DATA_PIN)
  TXD_pin(LOW);
#endif

  SCLK_pin(LOW);
  SDATA_pin(LOW);
  SLE_pin(LOW);
}

void CIO::process()
{
  uint8_t bit;

  // Switch off the transmitter if needed
  if (m_txBuffer.getData() == 0U && m_tx) {
    setRX();
    m_tx = false;
  }

  if (m_rxBuffer.getData() >= 1U) {
    m_rxBuffer.get(bit);

    if(m_dstarEnable)
      dstarRX.databit(bit);
    else if(m_dmrEnable)
      dmrDMORX.databit(bit);
    else if(m_ysfEnable)
      ysfRX.databit(bit);
    else if(m_p25Enable)
      p25RX.databit(bit);
  }
}

void CIO::interrupt()
{
  uint8_t bit = 0;
  
  if (!m_started)
    return;

  if(m_tx) {
    m_txBuffer.get(bit);

#if defined(BIDIR_DATA_PIN)
    if(bit)
      RXD_pin_write(HIGH);
    else
      RXD_pin_write(LOW);
#else
    if(bit)
      TXD_pin(HIGH);
    else
      TXD_pin(LOW);
#endif

  } else {
    if(RXD_pin())
      bit = 1;
    else
      bit = 0;

    m_rxBuffer.put(bit);
  }
}

void CIO::start()
{ 
  ifConf();
  
  if (m_started)
    return;
  
  startInt();
    
  m_started = true;
  
  setMode();
}

void CIO::write(uint8_t* data, uint16_t length)
{
  if (!m_started)
    return;

  for (uint16_t i = 0U; i < length; i++)
    m_txBuffer.put(data[i]);

  // Switch the transmitter on if needed
  if (!m_tx) {
    setTX();
    m_tx = true;
  }

}

uint16_t CIO::getSpace() const
{
  return m_txBuffer.getSpace();
}

bool CIO::hasTXOverflow()
{
  return m_txBuffer.hasOverflowed();
}

bool CIO::hasRXOverflow()
{
  return m_rxBuffer.hasOverflowed();
}

uint8_t CIO::setFreq(uint32_t frequency_rx, uint32_t frequency_tx)
{
  // power level
  m_power = 0x20;

  if( !( ((frequency_rx >= VHF_MIN)&&(frequency_rx < VHF_MAX)) || ((frequency_tx >= VHF_MIN)&&(frequency_tx < VHF_MAX)) || \
  ((frequency_rx >= UHF_MIN)&&(frequency_rx < UHF_MAX)) || ((frequency_tx >= UHF_MIN)&&(frequency_tx < UHF_MAX)) ) )
    return 4U;

  m_frequency_rx = frequency_rx;
  m_frequency_tx = frequency_tx;

  return 0U;
}

void CIO::setMode()
{
  DSTAR_pin(m_modemState == STATE_DSTAR);
  DMR_pin(m_modemState   == STATE_DMR);
  YSF_pin(m_modemState   == STATE_YSF);
  P25_pin(m_modemState   == STATE_P25);
}

void CIO::setDecode(bool dcd)
{
  if (dcd != m_dcd)
    COS_pin(dcd ? true : false);

  m_dcd = dcd;
}
