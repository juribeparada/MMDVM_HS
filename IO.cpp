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

// TODO: move to IO class as members
volatile bool totx_request = false;
volatile bool torx_request = false;
volatile bool even = true;
static uint32_t last_clk = 2;

CIO::CIO():
m_started(false),
m_rxBuffer(RX_RINGBUFFER_SIZE),
m_txBuffer(TX_RINGBUFFER_SIZE),
m_LoDevYSF(false),
m_ledCount(0U),
m_scanEnable(false),
m_modeTimerCnt(0U),
m_scanPauseCnt(0U),
m_scanPos(0U),
m_ledValue(true),
m_watchdog(0U)
{
  Init();
  
  CE_pin(HIGH);
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
  uint32_t scantime;
  
  m_ledCount++;
  
  if (m_started) {
    // Two seconds timeout
    if (m_watchdog >= 19200U) {
      if (m_modemState == STATE_DSTAR || m_modemState == STATE_DMR || m_modemState == STATE_YSF ||  m_modemState == STATE_P25) {
        m_modemState = STATE_IDLE;
        setMode(m_modemState);
      }

      m_watchdog = 0U;
    }

    if (m_ledCount >= 24000U) {
      m_ledCount = 0U;
      m_ledValue = !m_ledValue;
      LED_pin(m_ledValue);
    }
  } else {
    if (m_ledCount >= 240000U) {
      m_ledCount = 0U;
      m_ledValue = !m_ledValue;
      LED_pin(m_ledValue);
    }
    return;
  }

  // Switch off the transmitter if needed
  if (m_txBuffer.getData() == 0U && m_tx) {
    setRX(false);
    torx_request = true;
    while(torx_request) { asm volatile ("nop"); }
  }
  
  if(m_modemState_prev == STATE_DSTAR)
    scantime = SCAN_TIME;
  else if(m_modemState_prev == STATE_DMR)
    scantime = SCAN_TIME*2;
  else if(m_modemState_prev == STATE_YSF)
    scantime = SCAN_TIME;
  else if(m_modemState_prev == STATE_P25)
    scantime = SCAN_TIME;
  else
    scantime = SCAN_TIME;

  if(m_modeTimerCnt >= scantime) {
    m_modeTimerCnt = 0;
    if( (m_modemState == STATE_IDLE) && (m_scanPauseCnt == 0) && m_scanEnable) {
      m_scanPos = (m_scanPos + 1) % m_TotalModes;
      setMode(m_Modes[m_scanPos]);
      io.ifConf(m_Modes[m_scanPos], true);
    }
  }

  if (m_rxBuffer.getData() >= 1U) {
    m_rxBuffer.get(bit);
    
    switch (m_modemState_prev) {
      case STATE_DSTAR:
        dstarRX.databit(bit);
        break;
      case STATE_DMR:
        dmrDMORX.databit(bit);
        break;
      case STATE_YSF:
        ysfRX.databit(bit);
        break;
      case STATE_P25:
        p25RX.databit(bit);
        break;
      default:
        break;
    }

  }
  
}

void CIO::interrupt()
{
  uint8_t bit = 0;
  
  if (!m_started)
    return;

  uint8_t clk = CLK_pin();

  // this is to prevent activation by spurious interrupts
  // which seem to happen if you send out an control word
  // needs investigation
  // this workaround will fail if only rising or falling edge
  // is used to trigger the interrupt !!!!
  // TODO: figure out why sending the control word seems to issue interrupts
  // possibly this is a design problem of the RF7021 board or too long wires
  // on the breadboard build 
  // but normally this will not hurt too much
  if (clk == last_clk) {
    return;
  } else {
    last_clk = clk;
  }


  // we set the TX bit at TXD low, sampling of ADF7021 happens at rising clock
  if (m_tx && clk == 0) {
	
    m_txBuffer.get(bit);
    even = !even; 

    // use this for tracking issues
    // P25_pin(even);

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
    // wait a brief period before raising SLE
    if (totx_request == true) { 
      asm volatile("nop          \n\t"
                   "nop          \n\t"
                   "nop          \n\t"
                   );

      // SLE Pulse, should be moved out of here into class method 
      // according to datasheet in 4FSK we have to deliver this before 1/4 tbit == 26uS 
      SLE_pin(HIGH);
      asm volatile("nop          \n\t"
                   "nop          \n\t"
                   "nop          \n\t"
                   );
      SLE_pin(LOW);
      SDATA_pin(LOW);

      // now do housekeeping
      totx_request = false;
      // first tranmittted bit is always the odd bit
      even = false;
    }  
  }  
  // we sample the RX bit at rising TXD clock edge, so TXD must be 1 and we are not in tx mode
  if (!m_tx && clk == 1) {
    if(RXD_pin())
      bit = 1;
    else
      bit = 0;

    m_rxBuffer.put(bit);
  }
  if (torx_request == true && even == false && m_tx && clk == 0) { 
      // that is absolutely crucial in 4FSK, see datasheet:
      // enable sle after 1/4 tBit == 26uS when sending MSB (even == false) and clock is low 
      delay_us(26);

      // SLE Pulse, should be moved out of here into class method 
      SLE_pin(HIGH);
      asm volatile("nop          \n\t"
                   "nop          \n\t"
                   "nop          \n\t"
                   );
      SLE_pin(LOW);
      SDATA_pin(LOW);

      // now do housekeeping
      m_tx = false;
      torx_request = false;
      //last tranmittted bit is always the even bit
      // since the current bit is a transitional "don't care" bit, never transmitted
      even = true;
  }  


  m_watchdog++;
  m_modeTimerCnt++;

  if(m_scanPauseCnt >= SCAN_PAUSE)
    m_scanPauseCnt = 0;
  
  if(m_scanPauseCnt != 0)
    m_scanPauseCnt++;

}

void CIO::start()
{ 
  m_TotalModes = 0;
  
  if(m_dstarEnable) {
    m_Modes[m_TotalModes] = STATE_DSTAR;
    m_TotalModes++;
  }
  if(m_dmrEnable) {
    m_Modes[m_TotalModes] = STATE_DMR;
    m_TotalModes++;
  }
  if(m_ysfEnable) {
    m_Modes[m_TotalModes] = STATE_YSF;
    m_TotalModes++;
  }
  if(m_p25Enable) {
    m_Modes[m_TotalModes] = STATE_P25;
    m_TotalModes++;
  }
  
#if defined(ENABLE_SCAN_MODE)
  if(m_TotalModes > 1)
    m_scanEnable = true;
  else {
    m_scanEnable = false;
    setMode(m_modemState);
  }
#else
  m_scanEnable = false;
  setMode(m_modemState);
#endif

  if (m_started)
    return;
    
  startInt();
    
  m_started = true;
  
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
    totx_request = true;
    // not sure if needed, would be better if handled in interrupt (like torx_request)
    // or move into setTX()
    while(CLK_pin());
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

  if( !( ((frequency_rx >= VHF1_MIN)&&(frequency_rx < VHF1_MAX)) || ((frequency_tx >= VHF1_MIN)&&(frequency_tx < VHF1_MAX)) || \
  ((frequency_rx >= UHF1_MIN)&&(frequency_rx < UHF1_MAX)) || ((frequency_tx >= UHF1_MIN)&&(frequency_tx < UHF1_MAX)) || \
  ((frequency_rx >= VHF2_MIN)&&(frequency_rx < VHF2_MAX)) || ((frequency_tx >= VHF2_MIN)&&(frequency_tx < VHF2_MAX)) || \
  ((frequency_rx >= UHF2_MIN)&&(frequency_rx < UHF2_MAX)) || ((frequency_tx >= UHF2_MIN)&&(frequency_tx < UHF2_MAX)) ) )
    return 4U;

  m_frequency_rx = frequency_rx;
  m_frequency_tx = frequency_tx;

  return 0U;
}

void CIO::setMode(MMDVM_STATE modemState)
{
  DSTAR_pin(modemState == STATE_DSTAR);
  DMR_pin(modemState   == STATE_DMR);
  YSF_pin(modemState   == STATE_YSF);
  P25_pin(modemState   == STATE_P25);
}

void CIO::setDecode(bool dcd)
{
  if (dcd != m_dcd) {
    m_scanPauseCnt = 1;
    COS_pin(dcd ? true : false);
  }

  m_dcd = dcd;
}

void CIO::resetWatchdog()
{
  m_watchdog = 0U;
}

