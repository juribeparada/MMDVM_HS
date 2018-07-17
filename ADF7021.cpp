/*
 *   Copyright (C) 2016 by Jim McLaughlin KI6ZUM
 *   Copyright (C) 2016,2017,2018 by Andy Uribe CA6JAU
 *   Copyright (C) 2017 by Danilo DB4PLE 
 * 
 *   Some of the code is based on work of Guus Van Dooren PE1PLM:
 *   https://github.com/ki6zum/gmsk-dstar/blob/master/firmware/dvmega/dvmega.ino
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

#if defined(ENABLE_ADF7021)

#include "Globals.h"
#include "IO.h"
#include "ADF7021.h"
#include <math.h>

volatile bool totx_request = false;
volatile bool torx_request = false;
volatile bool even = true;
static uint32_t last_clk = 2U;

volatile uint32_t  AD7021_control_word;

uint32_t           ADF7021_RX_REG0;
uint32_t           ADF7021_TX_REG0;
uint32_t           ADF7021_REG1;
uint32_t           div2;
uint32_t           f_div;

uint16_t           m_dstarDev;
uint16_t           m_dmrDev;
uint16_t           m_ysfDev;
uint16_t           m_p25Dev;
uint16_t           m_nxdnDev;
uint16_t           m_pocsagDev;

static void Send_AD7021_control_shift()
{
  int AD7021_counter;

  for(AD7021_counter = 31; AD7021_counter >= 0; AD7021_counter--) {
    if(bitRead(AD7021_control_word, AD7021_counter) == HIGH)
      io.SDATA_pin(HIGH);
    else
      io.SDATA_pin(LOW);

    io.dlybit();
    io.SCLK_pin(HIGH);
    io.dlybit();
    io.SCLK_pin(LOW);
  }
  // to keep SDATA signal at defined level when idle (not required)
  io.SDATA_pin(LOW);
}

static void Send_AD7021_control_slePulse()
{
  io.SLE_pin(HIGH);
  io.dlybit();
  io.SLE_pin(LOW);
}

void Send_AD7021_control(bool doSle)
{
  Send_AD7021_control_shift();

  if (doSle) {
    Send_AD7021_control_slePulse();
  }
}

#if defined(DUPLEX)
static  void Send_AD7021_control_sle2Pulse()
{
  io.SLE2_pin(HIGH);
  io.dlybit();
  io.SLE2_pin(LOW);
}

void Send_AD7021_control2(bool doSle)
{
  Send_AD7021_control_shift();

  if (doSle) {
    Send_AD7021_control_sle2Pulse();
  }
}
#endif

#if defined(SEND_RSSI_DATA)
uint16_t CIO::readRSSI()
{
  uint32_t AD7021_RB;
  uint16_t RB_word = 0U;
  int AD7021_counter;
  uint8_t RB_code, gain_code, gain_corr;

  // Register 7, readback enable, ADC RSSI mode
  AD7021_RB = 0x0147;
  
  // Send control register
  for(AD7021_counter = 8; AD7021_counter >= 0; AD7021_counter--) {
    if(bitRead(AD7021_RB, AD7021_counter) == HIGH)
      SDATA_pin(HIGH);
    else
      SDATA_pin(LOW);

    dlybit();
    SCLK_pin(HIGH);
    dlybit();
    SCLK_pin(LOW);
  }

  SDATA_pin(LOW);
  SLE_pin(HIGH);
  dlybit();
  
  // Read SREAD pin
  for(AD7021_counter = 17; AD7021_counter >= 0; AD7021_counter--) {
    SCLK_pin(HIGH);
    dlybit();

    if( (AD7021_counter != 17) && (AD7021_counter != 0) )
      RB_word |= ( (SREAD_pin() & 0x01) << (AD7021_counter-1) );

    SCLK_pin(LOW);
    dlybit();

  }

  SLE_pin(LOW);
  
  // Process RSSI code
  RB_code = RB_word & 0x7f;
  gain_code = (RB_word >> 7) & 0x0f;
  
  switch(gain_code) {
    case 0b1010:
      gain_corr = 0U;
      break;
    case 0b0110:
      gain_corr = 24U;
      break;
    case 0b0101:
      gain_corr = 38U;
      break;
    case 0b0100:
      gain_corr = 58U;
      break;
    case 0b0000:
      gain_corr = 86U;
      break;
    default:
      gain_corr = 0U;
      break;
  }

  return ( 130 - (RB_code + gain_corr)/2 );

}
#endif

void CIO::ifConf(MMDVM_STATE modemState, bool reset)
{
  float    divider;

  uint32_t ADF7021_REG2  = 0U;
  uint32_t ADF7021_REG3  = 0U;
  uint32_t ADF7021_REG4  = 0U;
  uint32_t ADF7021_REG10 = 0U;
  uint32_t ADF7021_REG13 = 0U;
  int32_t AFC_OFFSET = 0;

  uint32_t frequency_tx_tmp, frequency_rx_tmp;

  if (modemState != STATE_CWID && modemState != STATE_POCSAG)
    m_modemState_prev = modemState;

  // Change frequency for POCSAG mode, store a backup of DV frequencies
  if (modemState == STATE_POCSAG) {
    frequency_tx_tmp = m_frequency_tx;
    frequency_rx_tmp = m_frequency_rx;
    m_frequency_tx   = m_pocsag_freq_tx;
    m_frequency_rx   = m_pocsag_freq_tx;
  }

  // Toggle CE pin for ADF7021 reset
  if(reset) {
    CE_pin(LOW);
    delay_reset();
    CE_pin(HIGH);
    delay_reset();
  }

  // Check frequency band
  if( (m_frequency_tx >= VHF1_MIN) && (m_frequency_tx < VHF1_MAX) ) {
    ADF7021_REG1 = ADF7021_REG1_VHF1;         // VHF1, external VCO
    div2 = 1U;
  }
  else if( (m_frequency_tx >= VHF2_MIN) && (m_frequency_tx < VHF2_MAX) ) {
    ADF7021_REG1 = ADF7021_REG1_VHF2;         // VHF1, external VCO
    div2 = 1U;
  }
  else if( (m_frequency_tx >= UHF1_MIN)&&(m_frequency_tx < UHF1_MAX) ) {
    ADF7021_REG1 = ADF7021_REG1_UHF1;         // UHF1, internal VCO
    div2 = 1U;
  }
  else if( (m_frequency_tx >= UHF2_MIN)&&(m_frequency_tx < UHF2_MAX) ) {
    ADF7021_REG1 = ADF7021_REG1_UHF2;         // UHF2, internal VCO
    div2 = 2U;
  }
  else {
    ADF7021_REG1 = ADF7021_REG1_UHF1;         // UHF1, internal VCO
    div2 = 1U;
  }

  if(div2 == 1U)
    f_div = 2U;
  else
    f_div = 1U;

  switch (modemState) {
    case STATE_DSTAR:
    case STATE_POCSAG:
      AFC_OFFSET = 0;
      break;
    case STATE_DMR:
    case STATE_CWID:
      AFC_OFFSET = AFC_OFFSET_DMR;
      break;
    case STATE_YSF:
      AFC_OFFSET = AFC_OFFSET_YSF;
      break;
    case STATE_P25:
      AFC_OFFSET = AFC_OFFSET_P25;
      break;
    case STATE_NXDN:
      AFC_OFFSET = AFC_OFFSET_NXDN;
      break;
    default:
      break;
  }

  if( div2 == 1U )
    divider = (m_frequency_rx - 100000 + AFC_OFFSET) / (ADF7021_PFD / 2U);
  else
    divider = (m_frequency_rx - 100000 + (2*AFC_OFFSET)) / ADF7021_PFD;

  m_RX_N_divider = floor(divider);
  divider = (divider - m_RX_N_divider) * 32768;
  m_RX_F_divider = floor(divider + 0.5);

  ADF7021_RX_REG0  = (uint32_t) 0b0000;
  
#if defined(BIDIR_DATA_PIN)
  ADF7021_RX_REG0 |= (uint32_t) 0b01001   << 27;   // mux regulator/receive
#else
  ADF7021_RX_REG0 |= (uint32_t) 0b01011   << 27;   // mux regulator/uart-spi enabled/receive
#endif

  ADF7021_RX_REG0 |= (uint32_t) m_RX_N_divider << 19;   // frequency;
  ADF7021_RX_REG0 |= (uint32_t) m_RX_F_divider << 4;    // frequency;
  
  if( div2 == 1U )
    divider = m_frequency_tx / (ADF7021_PFD / 2U);
  else
    divider = m_frequency_tx / ADF7021_PFD;

  m_TX_N_divider = floor(divider);
  divider = (divider - m_TX_N_divider) * 32768;
  m_TX_F_divider = floor(divider + 0.5);

  ADF7021_TX_REG0  = (uint32_t) 0b0000;            // register 0

#if defined(BIDIR_DATA_PIN)
  ADF7021_TX_REG0 |= (uint32_t) 0b01000   << 27;   // mux regulator/transmit
#else
  ADF7021_TX_REG0 |= (uint32_t) 0b01010   << 27;   // mux regulator/uart-spi enabled/transmit
#endif

  ADF7021_TX_REG0 |= (uint32_t) m_TX_N_divider << 19;   // frequency;
  ADF7021_TX_REG0 |= (uint32_t) m_TX_F_divider << 4;    // frequency;

#if defined(TEST_TX)
  modemState = STATE_DSTAR;
#endif

  switch (modemState) {
    case STATE_CWID:
      // CW ID base configuration: DMR
      // Dev: +1 symb (variable), symb rate = 4800
    
      ADF7021_REG3 = ADF7021_REG3_DMR;
      ADF7021_REG10 = ADF7021_REG10_DMR;

      // K=32
      ADF7021_REG4  = (uint32_t) 0b0100                    << 0;   // register 4
      ADF7021_REG4 |= (uint32_t) 0b011                     << 4;   // mode, 4FSK
      ADF7021_REG4 |= (uint32_t) 0b0                       << 7;
      ADF7021_REG4 |= (uint32_t) 0b11                      << 8;
      ADF7021_REG4 |= (uint32_t) ADF7021_DISC_BW_DMR       << 10;  // Disc BW
      ADF7021_REG4 |= (uint32_t) ADF7021_POST_BW_DMR       << 20;  // Post dem BW
      ADF7021_REG4 |= (uint32_t) 0b10                      << 30;  // IF filter (25 kHz)

      ADF7021_REG13 = (uint32_t) 0b1101                    << 0;   // register 13
      ADF7021_REG13 |= (uint32_t) ADF7021_SLICER_TH_DMR    << 4;   // slicer threshold

      ADF7021_REG2 = (uint32_t) 0b10                       << 28;  // invert data (and RC alpha = 0.5)
      ADF7021_REG2 |= (uint32_t) (m_cwIdTXLevel / div2)    << 19;  // deviation
      ADF7021_REG2 |= (uint32_t) 0b111                     << 4;   // modulation (RC 4FSK)
      break;

    case STATE_POCSAG:
      // Dev: 4500 Hz, symb rate = 1200

      ADF7021_REG3 = ADF7021_REG3_POCSAG;
      ADF7021_REG10 = ADF7021_REG10_POCSAG;

      ADF7021_REG4  = (uint32_t) 0b0100                     << 0;   // register 4
      ADF7021_REG4 |= (uint32_t) 0b000                      << 4;   // 2FSK linear demodulator
      ADF7021_REG4 |= (uint32_t) 0b1                        << 7;
      ADF7021_REG4 |= (uint32_t) 0b10                       << 8;
      ADF7021_REG4 |= (uint32_t) ADF7021_DISC_BW_POCSAG     << 10;  // Disc BW
      ADF7021_REG4 |= (uint32_t) ADF7021_POST_BW_POCSAG     << 20;  // Post dem BW
      ADF7021_REG4 |= (uint32_t) 0b10                       << 30;  // IF filter (25 kHz)

      // Register 13 not used with 2FSK
      ADF7021_REG13 = (uint32_t) 0b1101                     << 0;   // register 13

      ADF7021_REG2  = (uint32_t) 0b10                       << 28;  // inverted data, clock normal
      ADF7021_REG2 |= (uint32_t) (m_pocsagDev / div2)       << 19;  // deviation
      ADF7021_REG2 |= (uint32_t) 0b000                      << 4;   // modulation (2FSK)
      break;

    case STATE_DSTAR:
      // Dev: 1200 Hz, symb rate = 4800

      ADF7021_REG3 = ADF7021_REG3_DSTAR;
      ADF7021_REG10 = ADF7021_REG10_DSTAR;
    
      // K=32
      ADF7021_REG4  = (uint32_t) 0b0100                    << 0;   // register 4
      ADF7021_REG4 |= (uint32_t) 0b001                     << 4;   // mode, GMSK
      ADF7021_REG4 |= (uint32_t) 0b1                       << 7;
      ADF7021_REG4 |= (uint32_t) 0b10                      << 8;
      ADF7021_REG4 |= (uint32_t) ADF7021_DISC_BW_DSTAR     << 10;  // Disc BW
      ADF7021_REG4 |= (uint32_t) ADF7021_POST_BW_DSTAR     << 20;  // Post dem BW
      ADF7021_REG4 |= (uint32_t) 0b00                      << 30;  // IF filter (12.5 kHz)

      ADF7021_REG13 = (uint32_t) 0b1101                    << 0;   // register 13
      ADF7021_REG13 |= (uint32_t) ADF7021_SLICER_TH_DSTAR  << 4;   // slicer threshold

      ADF7021_REG2 = (uint32_t) 0b00                       << 28;  // clock normal
      ADF7021_REG2 |= (uint32_t) (m_dstarDev / div2)       << 19;  // deviation
      ADF7021_REG2 |= (uint32_t) 0b001                     << 4;   // modulation (GMSK)
      break;
      
    case STATE_DMR:
      // Dev: +1 symb 648 Hz, symb rate = 4800
    
      ADF7021_REG3 = ADF7021_REG3_DMR;
      ADF7021_REG10 = ADF7021_REG10_DMR;

      // K=32
      ADF7021_REG4  = (uint32_t) 0b0100                    << 0;   // register 4
      ADF7021_REG4 |= (uint32_t) 0b011                     << 4;   // mode, 4FSK
      ADF7021_REG4 |= (uint32_t) 0b0                       << 7;
      ADF7021_REG4 |= (uint32_t) 0b11                      << 8;
      ADF7021_REG4 |= (uint32_t) ADF7021_DISC_BW_DMR       << 10;  // Disc BW
      ADF7021_REG4 |= (uint32_t) ADF7021_POST_BW_DMR       << 20;  // Post dem BW
      ADF7021_REG4 |= (uint32_t) 0b10                      << 30;  // IF filter (25 kHz)

      ADF7021_REG13 = (uint32_t) 0b1101                    << 0;   // register 13
      ADF7021_REG13 |= (uint32_t) ADF7021_SLICER_TH_DMR    << 4;   // slicer threshold

      ADF7021_REG2 = (uint32_t) 0b10                       << 28;  // invert data (and RC alpha = 0.5)
      ADF7021_REG2 |= (uint32_t) (m_dmrDev / div2)         << 19;  // deviation
#if defined(ADF7021_DISABLE_RC_4FSK)
      ADF7021_REG2 |= (uint32_t) 0b011                     << 4;   // modulation (4FSK)
#else
      ADF7021_REG2 |= (uint32_t) 0b111                     << 4;   // modulation (RC 4FSK)
#endif
      break;
      
    case STATE_YSF:
      // Dev: +1 symb 900 Hz, symb rate = 4800

      ADF7021_REG3 = (m_LoDevYSF ? ADF7021_REG3_YSF_L : ADF7021_REG3_YSF_H);
      ADF7021_REG10 = ADF7021_REG10_YSF;

      // K=28
      ADF7021_REG4  = (uint32_t) 0b0100                    << 0;   // register 4
      ADF7021_REG4 |= (uint32_t) 0b011                     << 4;   // mode, 4FSK
      ADF7021_REG4 |= (uint32_t) 0b0                       << 7;
      ADF7021_REG4 |= (uint32_t) 0b11                      << 8;
      ADF7021_REG4 |= (uint32_t) (m_LoDevYSF ? ADF7021_DISC_BW_YSF_L : ADF7021_DISC_BW_YSF_H) << 10;  // Disc BW
      ADF7021_REG4 |= (uint32_t) ADF7021_POST_BW_YSF       << 20;  // Post dem BW
      ADF7021_REG4 |= (uint32_t) 0b10                      << 30;  // IF filter (25 kHz)

      ADF7021_REG13 = (uint32_t) 0b1101                    << 0;   // register 13
      ADF7021_REG13 |= (uint32_t) (m_LoDevYSF ? ADF7021_SLICER_TH_YSF_L : ADF7021_SLICER_TH_YSF_H) << 4;   // slicer threshold

      ADF7021_REG2 = (uint32_t) 0b10                       << 28;  // invert data (and RC alpha = 0.5)
      ADF7021_REG2 |= (uint32_t) (m_ysfDev / div2)         << 19;  // deviation
#if defined(ADF7021_DISABLE_RC_4FSK)
      ADF7021_REG2 |= (uint32_t) 0b011                     << 4;   // modulation (4FSK)
#else
      ADF7021_REG2 |= (uint32_t) 0b111                     << 4;   // modulation (RC 4FSK)
#endif
      break;
      
    case STATE_P25:
      // Dev: +1 symb 600 Hz, symb rate = 4800

      ADF7021_REG3 = ADF7021_REG3_P25;
      ADF7021_REG10 = ADF7021_REG10_P25;

      // K=32
      ADF7021_REG4  = (uint32_t) 0b0100                    << 0;   // register 4
      ADF7021_REG4 |= (uint32_t) 0b011                     << 4;   // mode, 4FSK
      ADF7021_REG4 |= (uint32_t) 0b0                       << 7;
      ADF7021_REG4 |= (uint32_t) 0b11                      << 8;
      ADF7021_REG4 |= (uint32_t) ADF7021_DISC_BW_P25       << 10;  // Disc BW
      ADF7021_REG4 |= (uint32_t) ADF7021_POST_BW_P25       << 20;  // Post dem BW
      ADF7021_REG4 |= (uint32_t) 0b00                      << 30;  // IF filter (12.5 kHz)

      ADF7021_REG13 = (uint32_t) 0b1101                    << 0;   // register 13
      ADF7021_REG13 |= (uint32_t) ADF7021_SLICER_TH_P25    << 4;   // slicer threshold

      ADF7021_REG2 = (uint32_t) 0b10                       << 28;  // invert data (and RC alpha = 0.5)
      ADF7021_REG2 |= (uint32_t) (m_p25Dev / div2)         << 19;  // deviation
#if defined(ENABLE_P25_WIDE) || defined(ADF7021_DISABLE_RC_4FSK)
      ADF7021_REG2 |= (uint32_t) 0b011                     << 4;   // modulation (4FSK)
#else
      ADF7021_REG2 |= (uint32_t) 0b111                     << 4;   // modulation (RC 4FSK)
#endif
      break;
      
    case STATE_NXDN:
      // Dev: +1 symb 350 Hz, symb rate = 2400

      ADF7021_REG3 = ADF7021_REG3_NXDN;
      ADF7021_REG10 = ADF7021_REG10_NXDN;

      // K=32
      ADF7021_REG4  = (uint32_t) 0b0100                    << 0;   // register 4
      ADF7021_REG4 |= (uint32_t) 0b011                     << 4;   // mode, 4FSK
      ADF7021_REG4 |= (uint32_t) 0b0                       << 7;
      ADF7021_REG4 |= (uint32_t) 0b11                      << 8;
      ADF7021_REG4 |= (uint32_t) ADF7021_DISC_BW_NXDN      << 10;  // Disc BW
      ADF7021_REG4 |= (uint32_t) ADF7021_POST_BW_NXDN      << 20;  // Post dem BW
      ADF7021_REG4 |= (uint32_t) 0b00                      << 30;  // IF filter (12.5 kHz)

      ADF7021_REG13 = (uint32_t) 0b1101                    << 0;   // register 13
      ADF7021_REG13 |= (uint32_t) ADF7021_SLICER_TH_NXDN   << 4;   // slicer threshold

      ADF7021_REG2 = (uint32_t) 0b10                       << 28;  // invert data (and RC alpha = 0.5)
      ADF7021_REG2 |= (uint32_t) (m_nxdnDev / div2)        << 19;  // deviation
#if defined(ADF7021_DISABLE_RC_4FSK)
      ADF7021_REG2 |= (uint32_t) 0b011                     << 4;   // modulation (4FSK)
#else
      ADF7021_REG2 |= (uint32_t) 0b111                     << 4;   // modulation (RC 4FSK)
#endif
      break;
      
    default:
      break;
  }

  // VCO/OSCILLATOR (REG1)
  AD7021_control_word = ADF7021_REG1;
  Send_AD7021_control();

  // TX/RX CLOCK (3)
  AD7021_control_word = ADF7021_REG3;
  Send_AD7021_control();

  // DEMOD (4)
  AD7021_control_word = ADF7021_REG4;
  Send_AD7021_control();
  
  // IF fine cal (6)
  AD7021_control_word = ADF7021_REG6;
  Send_AD7021_control();

  // IF coarse cal (5)
  AD7021_control_word = ADF7021_REG5;
  Send_AD7021_control();
  
  // Delay for filter calibration
  delay_IFcal();

  // Frequency RX (0)
  setRX();

  // MODULATION (2)
  ADF7021_REG2 |= (uint32_t) 0b0010;               // register 2
  ADF7021_REG2 |= (uint32_t) m_power       << 13;  // power level
  ADF7021_REG2 |= (uint32_t) 0b110001      << 7;   // PA  
  AD7021_control_word = ADF7021_REG2;
  Send_AD7021_control();
 
  // TEST DAC (14)
#if defined(TEST_DAC)
  AD7021_control_word = 0x0000001E;
#else
  AD7021_control_word = 0x0000000E;
#endif
  Send_AD7021_control();

  // AGC (auto, defaults) (9)
#if defined(AD7021_GAIN_AUTO)
  AD7021_control_word = 0x000231E9; // AGC ON, normal operation
#elif defined(AD7021_GAIN_AUTO_LIN)
  AD7021_control_word = 0x100231E9; // AGC ON, LNA high linearity
#elif defined(AD7021_GAIN_LOW)
  AD7021_control_word = 0x120631E9; // AGC OFF, low gain, LNA high linearity
#elif defined(AD7021_GAIN_HIGH)
  AD7021_control_word = 0x00A631E9; // AGC OFF, high gain
#endif
  Send_AD7021_control();

  // AFC (10)
  AD7021_control_word = ADF7021_REG10;
  Send_AD7021_control();

  // SYNC WORD DET (11)
  AD7021_control_word = 0x0000003B;
  Send_AD7021_control();

  // SWD/THRESHOLD (12)
  AD7021_control_word = 0x0000010C;
  Send_AD7021_control();

  // 3FSK/4FSK DEMOD (13)
  AD7021_control_word = ADF7021_REG13;
  Send_AD7021_control();
  
#if defined(TEST_TX)
  PTT_pin(HIGH); 
  AD7021_control_word = ADF7021_TX_REG0;
  Send_AD7021_control();
  // TEST MODE (TX carrier only) (15)
  AD7021_control_word = 0x000E010F;
#else
  // TEST MODE (disabled) (15)
  AD7021_control_word = 0x000E000F;
#endif
  Send_AD7021_control();

  // Restore normal DV frequencies
  if (modemState == STATE_POCSAG) {
    m_frequency_tx = frequency_tx_tmp;
    m_frequency_rx = frequency_rx_tmp;
  }

#if defined(DUPLEX)
if(m_duplex && (modemState != STATE_CWID && modemState != STATE_POCSAG))
  ifConf2(modemState);
#endif
}

#if defined(DUPLEX)
void CIO::ifConf2(MMDVM_STATE modemState)
{
  uint32_t ADF7021_REG2  = 0U;
  uint32_t ADF7021_REG3  = 0U;
  uint32_t ADF7021_REG4  = 0U;
  uint32_t ADF7021_REG10 = 0U;
  uint32_t ADF7021_REG13 = 0U;

  switch (modemState) {
    case STATE_DSTAR:
      // Dev: 1200 Hz, symb rate = 4800

      ADF7021_REG3 = ADF7021_REG3_DSTAR;
      ADF7021_REG10 = ADF7021_REG10_DSTAR;
    
      // K=32
      ADF7021_REG4  = (uint32_t) 0b0100                    << 0;   // register 4
      ADF7021_REG4 |= (uint32_t) 0b001                     << 4;   // mode, GMSK
      ADF7021_REG4 |= (uint32_t) 0b1                       << 7;
      ADF7021_REG4 |= (uint32_t) 0b10                      << 8;
      ADF7021_REG4 |= (uint32_t) ADF7021_DISC_BW_DSTAR     << 10;  // Disc BW
      ADF7021_REG4 |= (uint32_t) ADF7021_POST_BW_DSTAR     << 20;  // Post dem BW
      ADF7021_REG4 |= (uint32_t) 0b00                      << 30;  // IF filter (12.5 kHz)

      ADF7021_REG13 = (uint32_t) 0b1101                    << 0;   // register 13
      ADF7021_REG13 |= (uint32_t) ADF7021_SLICER_TH_DSTAR  << 4;   // slicer threshold

      ADF7021_REG2 = (uint32_t) 0b00                       << 28;  // clock normal
      ADF7021_REG2 |= (uint32_t) (m_dstarDev / div2)<< 19;  // deviation
      ADF7021_REG2 |= (uint32_t) 0b001                     << 4;   // modulation (GMSK)
      break;
      
    case STATE_DMR:
      // Dev: +1 symb 648 Hz, symb rate = 4800
    
      ADF7021_REG3 = ADF7021_REG3_DMR;
      ADF7021_REG10 = ADF7021_REG10_DMR;

      // K=32
      ADF7021_REG4  = (uint32_t) 0b0100                    << 0;   // register 4
      ADF7021_REG4 |= (uint32_t) 0b011                     << 4;   // mode, 4FSK
      ADF7021_REG4 |= (uint32_t) 0b0                       << 7;
      ADF7021_REG4 |= (uint32_t) 0b11                      << 8;
      ADF7021_REG4 |= (uint32_t) ADF7021_DISC_BW_DMR       << 10;  // Disc BW
      ADF7021_REG4 |= (uint32_t) ADF7021_POST_BW_DMR       << 20;  // Post dem BW
      ADF7021_REG4 |= (uint32_t) 0b10                      << 30;  // IF filter (25 kHz)

      ADF7021_REG13 = (uint32_t) 0b1101                    << 0;   // register 13
      ADF7021_REG13 |= (uint32_t) ADF7021_SLICER_TH_DMR    << 4;   // slicer threshold

      ADF7021_REG2 = (uint32_t) 0b10                       << 28;  // invert data (and RC alpha = 0.5)
      ADF7021_REG2 |= (uint32_t) (m_dmrDev / div2)  << 19;  // deviation
      ADF7021_REG2 |= (uint32_t) 0b111                     << 4;   // modulation (RC 4FSK)
      break;
      
    case STATE_YSF:
      // Dev: +1 symb 900 Hz, symb rate = 4800

      ADF7021_REG3 = (m_LoDevYSF ? ADF7021_REG3_YSF_L : ADF7021_REG3_YSF_H);
      ADF7021_REG10 = ADF7021_REG10_YSF;

      // K=28
      ADF7021_REG4  = (uint32_t) 0b0100                    << 0;   // register 4
      ADF7021_REG4 |= (uint32_t) 0b011                     << 4;   // mode, 4FSK
      ADF7021_REG4 |= (uint32_t) 0b0                       << 7;
      ADF7021_REG4 |= (uint32_t) 0b11                      << 8;
      ADF7021_REG4 |= (uint32_t) (m_LoDevYSF ? ADF7021_DISC_BW_YSF_L : ADF7021_DISC_BW_YSF_H) << 10;  // Disc BW
      ADF7021_REG4 |= (uint32_t) ADF7021_POST_BW_YSF       << 20;  // Post dem BW
      ADF7021_REG4 |= (uint32_t) 0b10                      << 30;  // IF filter (25 kHz)

      ADF7021_REG13 = (uint32_t) 0b1101                    << 0;   // register 13
      ADF7021_REG13 |= (uint32_t) (m_LoDevYSF ? ADF7021_SLICER_TH_YSF_L : ADF7021_SLICER_TH_YSF_H) << 4;   // slicer threshold

      ADF7021_REG2 = (uint32_t) 0b10                       << 28;  // invert data (and RC alpha = 0.5)
      ADF7021_REG2 |= (uint32_t) (m_ysfDev / div2)  << 19;  // deviation
      ADF7021_REG2 |= (uint32_t) 0b111                     << 4;   // modulation (RC 4FSK)
      break;
      
    case STATE_P25:
      // Dev: +1 symb 600 Hz, symb rate = 4800

      ADF7021_REG3 = ADF7021_REG3_P25;
      ADF7021_REG10 = ADF7021_REG10_P25;

      // K=32
      ADF7021_REG4  = (uint32_t) 0b0100                    << 0;   // register 4
      ADF7021_REG4 |= (uint32_t) 0b011                     << 4;   // mode, 4FSK
      ADF7021_REG4 |= (uint32_t) 0b0                       << 7;
      ADF7021_REG4 |= (uint32_t) 0b11                      << 8;
      ADF7021_REG4 |= (uint32_t) ADF7021_DISC_BW_P25       << 10;  // Disc BW
      ADF7021_REG4 |= (uint32_t) ADF7021_POST_BW_P25       << 20;  // Post dem BW
      ADF7021_REG4 |= (uint32_t) 0b00                      << 30;  // IF filter (12.5 kHz)

      ADF7021_REG13 = (uint32_t) 0b1101                    << 0;   // register 13
      ADF7021_REG13 |= (uint32_t) ADF7021_SLICER_TH_P25    << 4;   // slicer threshold

      ADF7021_REG2 = (uint32_t) 0b10                       << 28;  // invert data (and RC alpha = 0.5)
      ADF7021_REG2 |= (uint32_t) (m_p25Dev / div2)  << 19;  // deviation
      ADF7021_REG2 |= (uint32_t) 0b111                     << 4;   // modulation (RC 4FSK)
      break;

    case STATE_NXDN:
      // Dev: +1 symb 350 Hz, symb rate = 2400

      ADF7021_REG3 = ADF7021_REG3_NXDN;
      ADF7021_REG10 = ADF7021_REG10_NXDN;

      // K=32
      ADF7021_REG4  = (uint32_t) 0b0100                    << 0;   // register 4
      ADF7021_REG4 |= (uint32_t) 0b011                     << 4;   // mode, 4FSK
      ADF7021_REG4 |= (uint32_t) 0b0                       << 7;
      ADF7021_REG4 |= (uint32_t) 0b11                      << 8;
      ADF7021_REG4 |= (uint32_t) ADF7021_DISC_BW_NXDN      << 10;  // Disc BW
      ADF7021_REG4 |= (uint32_t) ADF7021_POST_BW_NXDN      << 20;  // Post dem BW
      ADF7021_REG4 |= (uint32_t) 0b00                      << 30;  // IF filter (12.5 kHz)

      ADF7021_REG13 = (uint32_t) 0b1101                    << 0;   // register 13
      ADF7021_REG13 |= (uint32_t) ADF7021_SLICER_TH_NXDN   << 4;   // slicer threshold

      ADF7021_REG2 = (uint32_t) 0b10                       << 28;  // invert data (and RC alpha = 0.5)
      ADF7021_REG2 |= (uint32_t) (m_nxdnDev / div2) << 19;  // deviation
      ADF7021_REG2 |= (uint32_t) 0b111                     << 4;   // modulation (RC 4FSK)
      break;

    default:
      break;
  }

  // VCO/OSCILLATOR (1)
  AD7021_control_word = ADF7021_REG1;
  Send_AD7021_control2();

  // TX/RX CLOCK (3)
  AD7021_control_word = ADF7021_REG3;
  Send_AD7021_control2();

  // DEMOD (4)
  AD7021_control_word = ADF7021_REG4;
  Send_AD7021_control2();

  // IF fine cal (6)
  AD7021_control_word = ADF7021_REG6;
  Send_AD7021_control2();

  // IF coarse cal (5)
  AD7021_control_word = ADF7021_REG5;
  Send_AD7021_control2();
  
  // Delay for coarse IF filter calibration
  delay_IFcal();

  // Frequency RX (0) and set to RX only
  AD7021_control_word = ADF7021_RX_REG0;
  Send_AD7021_control2();

  // MODULATION (2)
  ADF7021_REG2 |= (uint32_t) 0b0010;                  // register 2
  ADF7021_REG2 |= (uint32_t) (m_power & 0x3F) << 13;  // power level
  ADF7021_REG2 |= (uint32_t) 0b110001         << 7;   // PA  
  AD7021_control_word = ADF7021_REG2;
  Send_AD7021_control2();
 
  // TEST DAC (14)
  AD7021_control_word = 0x0000000E;
  Send_AD7021_control2();

  // AGC (auto, defaults) (9)
  AD7021_control_word = 0x000231E9;
  Send_AD7021_control2();

  // AFC (10)
  AD7021_control_word = ADF7021_REG10;
  Send_AD7021_control2();

  // SYNC WORD DET (11)
  AD7021_control_word = 0x0000003B;
  Send_AD7021_control2();

  // SWD/THRESHOLD (12)
  AD7021_control_word = 0x0000010C;
  Send_AD7021_control2();

  // 3FSK/4FSK DEMOD (13)
  AD7021_control_word = ADF7021_REG13;
  Send_AD7021_control2();
  
  // TEST MODE (disabled) (15)
  AD7021_control_word = 0x000E000F;
  Send_AD7021_control2();
}
#endif

void CIO::interrupt()
{
  uint8_t bit = 0U;
  
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
  if (m_tx && clk == 0U) {

    m_txBuffer.get(bit, m_control);
    even = !even;

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
      even = ADF7021_EVEN_BIT;
    }
  }
  
  // we sample the RX bit at rising TXD clock edge, so TXD must be 1 and we are not in tx mode
  if (!m_tx && clk == 1U && !m_duplex) {
    if(RXD_pin())
      bit = 1U;
    else
      bit = 0U;

    m_rxBuffer.put(bit, m_control);
  }
  
  if (torx_request == true && even == ADF7021_EVEN_BIT && m_tx && clk == 0U) { 
      // that is absolutely crucial in 4FSK, see datasheet:
      // enable sle after 1/4 tBit == 26uS when sending MSB (even == false) and clock is low 
      delay_us(26U);

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
      // last tranmittted bit is always the even bit
      // since the current bit is a transitional "don't care" bit, never transmitted
      even = !ADF7021_EVEN_BIT;
  }  

  m_watchdog++;
  m_modeTimerCnt++;

  if(m_scanPauseCnt >= SCAN_PAUSE)
    m_scanPauseCnt = 0U;
  
  if(m_scanPauseCnt != 0U)
    m_scanPauseCnt++;
}

#if defined(DUPLEX)
void CIO::interrupt2()
{
  uint8_t bit = 0U;
  
  if(m_duplex) {
    if(RXD2_pin())
      bit = 1U;
    else
      bit = 0U;

    m_rxBuffer.put(bit, m_control);
  }
}
#endif

void CIO::setTX()
{ 
  // PTT pin on (doing it earlier helps to measure timing impact)
  PTT_pin(HIGH); 

  // Send register 0 for TX operation, but do not activate yet. 
  // This is done in the interrupt at the correct time
  AD7021_control_word = ADF7021_TX_REG0;
  Send_AD7021_control(false);

#if defined(BIDIR_DATA_PIN)
  Data_dir_out(true);  // Data pin output mode
#endif

  totx_request = true;
  while(CLK_pin());
}

void CIO::setRX(bool doSle)
{ 
  // PTT pin off (doing it earlier helps to measure timing impact)
  PTT_pin(LOW);

  // Send register 0 for RX operation, but do not activate yet. 
  // This is done in the interrupt at the correct time
  AD7021_control_word = ADF7021_RX_REG0;
  Send_AD7021_control(doSle);
  
#if defined(BIDIR_DATA_PIN)
  Data_dir_out(false);  // Data pin input mode
#endif

  if(!doSle) {
    torx_request = true;
    while(torx_request) { asm volatile ("nop"); }
  }
}

void CIO::setPower(uint8_t power)
{
  m_power = power >> 2;
}

void CIO::setDeviations(uint8_t dstarTXLevel, uint8_t dmrTXLevel, uint8_t ysfTXLevel, uint8_t p25TXLevel, uint8_t nxdnTXLevel, uint8_t pocsagTXLevel, bool ysfLoDev)
{
  m_dstarDev = uint16_t((ADF7021_DEV_DSTAR * uint16_t(dstarTXLevel)) / 128U);
  m_dmrDev = uint16_t((ADF7021_DEV_DMR * uint16_t(dmrTXLevel)) / 128U);

  if (ysfLoDev)
    m_ysfDev = uint16_t((ADF7021_DEV_YSF_L * uint16_t(ysfTXLevel)) / 128U);
  else
    m_ysfDev = uint16_t((ADF7021_DEV_YSF_H * uint16_t(ysfTXLevel)) / 128U);

  m_p25Dev = uint16_t((ADF7021_DEV_P25 * uint16_t(p25TXLevel)) / 128U);
  m_nxdnDev = uint16_t((ADF7021_DEV_NXDN * uint16_t(nxdnTXLevel)) / 128U);
  m_pocsagDev = uint16_t((ADF7021_DEV_POCSAG * uint16_t(pocsagTXLevel)) / 128U);
}

void CIO::updateCal()
{
  uint32_t ADF7021_REG2;
  float    divider;

  // Check frequency band
  if( (m_frequency_tx >= VHF1_MIN) && (m_frequency_tx < VHF1_MAX) ) {
    ADF7021_REG1 = ADF7021_REG1_VHF1;         // VHF1, external VCO
    div2 = 1U;
  }
  else if( (m_frequency_tx >= VHF2_MIN) && (m_frequency_tx < VHF2_MAX) ) {
    ADF7021_REG1 = ADF7021_REG1_VHF2;         // VHF1, external VCO
    div2 = 1U;
  }
  else if( (m_frequency_tx >= UHF1_MIN)&&(m_frequency_tx < UHF1_MAX) ) {
    ADF7021_REG1 = ADF7021_REG1_UHF1;         // UHF1, internal VCO
    div2 = 1U;
  }
  else if( (m_frequency_tx >= UHF2_MIN)&&(m_frequency_tx < UHF2_MAX) ) {
    ADF7021_REG1 = ADF7021_REG1_UHF2;         // UHF2, internal VCO
    div2 = 2U;
  }
  else {
    ADF7021_REG1 = ADF7021_REG1_UHF1;         // UHF1, internal VCO
    div2 = 1U;
  }

  if(div2 == 1U)
    f_div = 2U;
  else
    f_div = 1U;

  // VCO/OSCILLATOR (REG1)
  AD7021_control_word = ADF7021_REG1;
  Send_AD7021_control();

  ADF7021_REG2  = (uint32_t) 0b10              << 28;  // invert data (and RC alpha = 0.5)
  ADF7021_REG2 |= (uint32_t) (m_dmrDev / div2) << 19;  // deviation
  ADF7021_REG2 |= (uint32_t) 0b111             << 4;   // modulation (RC 4FSK)
  ADF7021_REG2 |= (uint32_t) 0b0010;                   // register 2
  ADF7021_REG2 |= (uint32_t) m_power           << 13;  // power level
  ADF7021_REG2 |= (uint32_t) 0b110001          << 7;   // PA

  AD7021_control_word = ADF7021_REG2;
  Send_AD7021_control();

  if( div2 == 1U )
    divider = m_frequency_tx / (ADF7021_PFD / 2U);
  else
    divider = m_frequency_tx / ADF7021_PFD;

  m_TX_N_divider = floor(divider);
  divider = (divider - m_TX_N_divider) * 32768;
  m_TX_F_divider = floor(divider + 0.5);

  ADF7021_TX_REG0  = (uint32_t) 0b0000;            // register 0

#if defined(BIDIR_DATA_PIN)
  ADF7021_TX_REG0 |= (uint32_t) 0b01000   << 27;   // mux regulator/transmit
#else
  ADF7021_TX_REG0 |= (uint32_t) 0b01010   << 27;   // mux regulator/uart-spi enabled/transmit
#endif

  ADF7021_TX_REG0 |= (uint32_t) m_TX_N_divider << 19;   // frequency;
  ADF7021_TX_REG0 |= (uint32_t) m_TX_F_divider << 4;    // frequency;

  if (m_tx)
    setTX();
  else
    setRX();
}

#if defined(ENABLE_DEBUG)

uint32_t CIO::RXfreq()
{
  return (uint32_t)((float)(ADF7021_PFD / f_div) * ((float)((32768 * m_RX_N_divider) + m_RX_F_divider) / 32768.0)) + 100000;
}

uint32_t CIO::TXfreq()
{
  return (uint32_t)((float)(ADF7021_PFD / f_div) * ((float)((32768 * m_TX_N_divider) + m_TX_F_divider) / 32768.0));
}

uint16_t CIO::devDSTAR()
{
  return (uint16_t)((ADF7021_PFD * m_dstarDev) / (f_div * 65536));
}

uint16_t CIO::devDMR()
{
  return (uint16_t)((ADF7021_PFD * m_dmrDev) / (f_div * 65536));
}

uint16_t CIO::devYSF()
{
  return (uint16_t)((ADF7021_PFD * m_ysfDev) / (f_div * 65536));
}

uint16_t CIO::devP25()
{
  return (uint16_t)((ADF7021_PFD * m_p25Dev) / (f_div * 65536));
}

uint16_t CIO::devNXDN()
{
  return (uint16_t)((ADF7021_PFD * m_nxdnDev) / (f_div * 65536));
}

uint16_t CIO::devPOCSAG()
{
  return (uint16_t)((ADF7021_PFD * m_pocsagDev) / (f_div * 65536));
}

void CIO::printConf()
{
  DEBUG1("MMDVM_HS FW configuration:");
  DEBUG2I("TX freq (Hz):", TXfreq());
  DEBUG2I("RX freq (Hz):", RXfreq());
  DEBUG2("Power set:", m_power);
  DEBUG2("D-Star dev (Hz):", devDSTAR());
  DEBUG2("DMR +1 sym dev (Hz):", devDMR());
  DEBUG2("YSF +1 sym dev (Hz):", devYSF());
  DEBUG2("P25 +1 sym dev (Hz):", devP25());
  DEBUG2("NXDN +1 sym dev (Hz):", devNXDN());
  DEBUG2("POCSAG dev (Hz):", devPOCSAG());
}

#endif

#endif
