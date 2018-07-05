/*
 *   Copyright (C) 2015,2016 by Jonathan Naylor G4KLX
 *   Copyright (C) 2016,2017,2018 by Andy Uribe CA6JAU
 *   Copyright (C) 2017 by Danilo DB4PLE 
 
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

#include "Config.h"
#include "Globals.h"
#include "BitRB.h"

#define LOW      0
#define HIGH     1

#define VHF1_MIN  144000000
#define VHF1_MAX  148000000
#define VHF2_MIN  219000000
#define VHF2_MAX  225000000
#define UHF1_MIN  420000000
#define UHF1_MAX  475000000
#define UHF2_MIN  842000000
#define UHF2_MAX  950000000

#define SCAN_TIME  1920
#define SCAN_PAUSE 20000

extern uint32_t  m_frequency_rx;
extern uint32_t  m_frequency_tx;
extern uint32_t  m_pocsag_freq_tx;
extern uint8_t   m_power;

class CIO {

public:
  CIO();

  // Platform API
  void      Init(void);
  void      SCLK_pin(bool on);
  void      SDATA_pin(bool on);
  bool      SREAD_pin(void);
  void      SLE_pin(bool on);
#if defined(DUPLEX)
  void      SLE2_pin(bool on);
  bool      RXD2_pin(void);
#endif
  void      CE_pin(bool on);
  bool      RXD_pin(void);
  bool      CLK_pin(void);
  
#if defined(BIDIR_DATA_PIN)
  void      RXD_pin_write(bool on);
#endif

  void      TXD_pin(bool on);
  void      PTT_pin(bool on);
  void      LED_pin(bool on);
  void      DEB_pin(bool on);
  void      DSTAR_pin(bool on);
  void      DMR_pin(bool on);
  void      YSF_pin(bool on);
  void      P25_pin(bool on);
  void      NXDN_pin(bool on);
  void      POCSAG_pin(bool on);
  void      COS_pin(bool on);
  void      interrupt(void);
#if defined(DUPLEX)
  void      interrupt2(void);
#endif
  
#if defined(BIDIR_DATA_PIN)
  void      Data_dir_out(bool dir);
#endif

  // IO API
  void      write(uint8_t* data, uint16_t length, const uint8_t* control = NULL);
  uint16_t  getSpace(void) const;
  void      process(void);
  bool      hasTXOverflow(void);
  bool      hasRXOverflow(void);
  uint8_t   setFreq(uint32_t frequency_rx, uint32_t frequency_tx, uint8_t rf_power, uint32_t pocsag_freq_tx);
  void      setPower(uint8_t power);
  void      setMode(MMDVM_STATE modemState);
  void      setDecode(bool dcd);
  void      setLoDevYSF(bool ysfLoDev);
  void      resetWatchdog(void);
  uint32_t  getWatchdog(void);
  void      selfTest(void);

  // RF interface API
  void      setTX(void);
  void      setRX(bool doSle = true);
  void      ifConf(MMDVM_STATE modemState, bool reset);
#if defined(DUPLEX)
  void      ifConf2(MMDVM_STATE modemState);
#endif
  void      start(void);
  void      startInt(void);
  void      setDeviations(uint8_t dstarTXLevel, uint8_t dmrTXLevel, uint8_t ysfTXLevel, uint8_t p25TXLevel, uint8_t nxdnTXLevel, uint8_t pocsagTXLevel, bool ysfLoDev);
  void      updateCal(void);

#if defined(SEND_RSSI_DATA)
  uint16_t  readRSSI(void);
#endif

  // Misc functions
  void      dlybit(void);
  void      delay_IFcal(void);
  void      delay_reset(void);
  void      delay_us(uint32_t us);

#if defined(ENABLE_DEBUG)
  uint32_t  RXfreq(void);
  uint32_t  TXfreq(void);
  uint16_t  devDSTAR(void);
  uint16_t  devDMR(void);
  uint16_t  devYSF(void);
  uint16_t  devP25(void);
  uint16_t  devNXDN(void);
  uint16_t  devPOCSAG(void);
  void      printConf();
#endif

private:
  uint8_t            m_RX_N_divider;
  uint16_t           m_RX_F_divider;
  uint8_t            m_TX_N_divider;
  uint16_t           m_TX_F_divider;
  
  bool               m_started;
  CBitRB             m_rxBuffer;
  CBitRB             m_txBuffer;
  bool               m_LoDevYSF;
  uint32_t           m_ledCount;
  bool               m_scanEnable;
  uint32_t           m_scanPauseCnt;
  uint8_t            m_scanPos;
  uint8_t            m_TotalModes;
  MMDVM_STATE        m_Modes[5];
  bool               m_ledValue;
  volatile uint32_t  m_watchdog;

};

#endif
