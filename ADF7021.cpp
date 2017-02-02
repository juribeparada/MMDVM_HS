/*
 *   Copyright (C) 2016 by Jim McLaughlin KI6ZUM
 *   Copyright (C) 2016, 2017 by Andy Uribe CA6JAU
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

#if defined(ADF7021)

#include "Globals.h"
#include "IO.h"
#include "ADF7021.h"
#include <math.h>

volatile uint32_t  AD7021_control_byte;
volatile int       AD7021_counter;

void Send_AD7021_control()
{
  for(AD7021_counter = 31; AD7021_counter >= 0; AD7021_counter--) {
    if(bitRead(AD7021_control_byte, AD7021_counter) == HIGH)
      io.SDATA_pin(HIGH);   
    else
      io.SDATA_pin(LOW);
     
    io.dlybit();
    io.SCLK_pin(HIGH);
    io.dlybit();
    io.SCLK_pin(LOW);
  }
  
  io.SLE_pin(HIGH);
  io.dlybit();
  io.SLE_pin(LOW);    
  io.SDATA_pin(LOW);
}

void Send_REG0_RX()
{
  uint32_t ADF7021_RX_REG0;
  float divider;
  uint8_t N_divider;
  uint16_t F_divider;
     
  divider = (m_frequency_rx - 100000) / (ADF7021_PFD / 2.0);

  N_divider = floor(divider);
  divider = (divider - N_divider) * 32768;
  F_divider = floor(divider + 0.5);

  ADF7021_RX_REG0  = (uint32_t)0b0000;
  ADF7021_RX_REG0 |= (uint32_t)0b01011   << 27;   // mux regulator/uart enabled/receive
  ADF7021_RX_REG0 |= (uint32_t)N_divider << 19;   //frequency;
  ADF7021_RX_REG0 |= (uint32_t)F_divider << 4;    //frequency;
  
  AD7021_control_byte = ADF7021_RX_REG0;
  Send_AD7021_control();
}  

void Send_REG0_TX()
{
  uint32_t ADF7021_TX_REG0;
  float divider;
  uint8_t N_divider;
  uint16_t F_divider;

  divider = m_frequency_tx / (ADF7021_PFD / 2.0);

  N_divider = floor(divider);
  divider = (divider - N_divider) * 32768;
  F_divider = floor(divider + 0.5);

  ADF7021_TX_REG0  = (uint32_t)0b0000;            // register 0
  ADF7021_TX_REG0 |= (uint32_t)0b01010   << 27;   // mux regulator/uart enabled/transmit
  ADF7021_TX_REG0 |= (uint32_t)N_divider << 19;   //frequency;
  ADF7021_TX_REG0 |= (uint32_t)F_divider << 4;    //frequency;

  AD7021_control_byte = ADF7021_TX_REG0;         
  Send_AD7021_control();
}

void CIO::ifConf()
{
  uint32_t ADF7021_REG2 = 0;
  uint32_t ADF7021_REG3 = 0;
  uint32_t ADF7021_REG4 = 0;
  uint32_t ADF7021_REG13 = 0;

  if (m_dstarEnable) {
    // Dev: 1200 Hz, symb rate = 4800

    ADF7021_REG3 = 0x2A4C4193;
    ADF7021_REG4 = 0x00A82A94;
    ADF7021_REG13 = 0x0000000D;

    ADF7021_REG2 = (uint32_t)0b00         << 28;  // clock normal
    ADF7021_REG2 |= (uint32_t)0b000101010 << 19;  // deviation
    ADF7021_REG2 |= (uint32_t)0b001       << 4;   // modulation (GMSK)
  }
  else if (m_dmrEnable) {
    // Dev: +1 symb 648 Hz, symb rate = 4800
    
    ADF7021_REG3 = 0x2A4C80D3;

    // K=32
    ADF7021_REG4  = (uint32_t)0b0100      << 0;   // register 4
    ADF7021_REG4 |= (uint32_t)0b011       << 4;   // mode, 4FSK
    ADF7021_REG4 |= (uint32_t)0b0         << 7;
    ADF7021_REG4 |= (uint32_t)0b11        << 8;
    ADF7021_REG4 |= (uint32_t)393U        << 10;  // Disc BW
    ADF7021_REG4 |= (uint32_t)65U         << 20;  // Post dem BW
    ADF7021_REG4 |= (uint32_t)0b10        << 30;  // IF filter

    ADF7021_REG13 = 0x0000033D;

    ADF7021_REG2 = (uint32_t)0b10         << 28;  // invert data
    ADF7021_REG2 |= (uint32_t)24U         << 19;  // deviation
    ADF7021_REG2 |= (uint32_t)0b111       << 4;   // modulation (4FSK)
  }
  else if (m_ysfEnable) {
    // Dev: +1 symb 900 Hz, symb rate = 4800

    ADF7021_REG3 = 0x2A4C80D3;

    // K=28
    ADF7021_REG4  = (uint32_t)0b0100      << 0;   // register 4
    ADF7021_REG4 |= (uint32_t)0b011       << 4;   // mode, 4FSK
    ADF7021_REG4 |= (uint32_t)0b0         << 7;
    ADF7021_REG4 |= (uint32_t)0b11        << 8;
    ADF7021_REG4 |= (uint32_t)344U        << 10;  // Disc BW
    ADF7021_REG4 |= (uint32_t)65U         << 20;  // Post dem BW
    ADF7021_REG4 |= (uint32_t)0b10        << 30;  // IF filter

    ADF7021_REG13 = 0x000003BD;

    ADF7021_REG2 = (uint32_t)0b10         << 28;  // invert data
    ADF7021_REG2 |= (uint32_t)32U         << 19;  // deviation
    ADF7021_REG2 |= (uint32_t)0b111       << 4;   // modulation (4FSK)
  }
  else if (m_p25Enable) {
    // Dev: +1 symb 600 Hz, symb rate = 4800

    ADF7021_REG3 = 0x2A4C80D3;

    // K=32
    ADF7021_REG4  = (uint32_t)0b0100      << 0;   // register 4
    ADF7021_REG4 |= (uint32_t)0b011       << 4;   // mode, 4FSK
    ADF7021_REG4 |= (uint32_t)0b0         << 7;
    ADF7021_REG4 |= (uint32_t)0b11        << 8;
    ADF7021_REG4 |= (uint32_t)393U        << 10;  // Disc BW
    ADF7021_REG4 |= (uint32_t)65U         << 20;  // Post dem BW
    ADF7021_REG4 |= (uint32_t)0b10        << 30;  // IF filter

    ADF7021_REG13 = 0x000002DD;

    ADF7021_REG2 = (uint32_t)0b10         << 28;  // invert data
    ADF7021_REG2 |= (uint32_t)22U         << 19;  // deviation
    ADF7021_REG2 |= (uint32_t)0b111       << 4;   // modulation (4FSK)
  }

  // VCO/OSCILLATOR (REG1)
  if( (m_frequency_tx >= VHF_MIN) && (m_frequency_tx < VHF_MAX) )
    AD7021_control_byte = 0x021F5041;             // VHF, external VCO
  else if( (m_frequency_tx >= UHF_MIN)&&(m_frequency_tx < UHF_MAX) )
    AD7021_control_byte = 0x00575041;             // UHF, internal VCO

  Send_AD7021_control();

  // TX/RX CLOCK (3)
  AD7021_control_byte = ADF7021_REG3;
  Send_AD7021_control();

  // DEMOD (4)
  AD7021_control_byte = ADF7021_REG4;
  Send_AD7021_control();

  // IF FILTER (5)
  AD7021_control_byte = 0x000024F5;
  Send_AD7021_control();

  // MODULATION (2)
  ADF7021_REG2 |= (uint32_t)0b0010;               // register 2
  ADF7021_REG2 |= (uint32_t)m_power       << 13;  // power level
  ADF7021_REG2 |= (uint32_t)0b110001      << 7;   // PA  
  AD7021_control_byte = ADF7021_REG2;
  Send_AD7021_control();
 
  // TEST MODE (disabled) (15)
  AD7021_control_byte = 0x000E000F;
  Send_AD7021_control();

  // IF FINE CAL (fine cal, defaults) (6)
  AD7021_control_byte = 0x05080B16;
  Send_AD7021_control();

  // AGC (auto, defaults) (9)
  AD7021_control_byte = 0x000231E9;     // auto
  Send_AD7021_control();

  // AFC (off, defaults) (10)
  AD7021_control_byte = 0x3296472A;     // off
  Send_AD7021_control();

  // SYNC WORD DET (11)
  AD7021_control_byte = 0x0000003B;
  Send_AD7021_control();

  // SWD/THRESHOLD (12)
  AD7021_control_byte = 0x0000010C;
  Send_AD7021_control();

  // 3FSK/4FSK DEMOD (13)
  AD7021_control_byte = ADF7021_REG13;
  Send_AD7021_control();
}

//======================================================================================================================
void CIO::setTX()
{ 
  PTT_pin(HIGH); 
  LED_pin(LOW);
  Send_REG0_TX();
}

//======================================================================================================================
void CIO::setRX()
{ 
  PTT_pin(LOW);
  LED_pin(HIGH);
  delay_rx();
  Send_REG0_RX();
}

#endif
