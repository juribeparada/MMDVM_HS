/*
 *   Copyright (C) 2015,2016 by Jonathan Naylor G4KLX
 *   Copyright (C) 2016 by Colin Durbridge G4EML
 *   Copyright (C) 2016,2017 by Andy Uribe CA6JAU
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

// Global variables
MMDVM_STATE m_modemState = STATE_IDLE;
MMDVM_STATE m_calState = STATE_IDLE;
MMDVM_STATE m_modemState_prev = STATE_IDLE;

bool m_cwid_state = false;
bool m_pocsag_state = false;

uint8_t m_cwIdTXLevel = 30;

uint32_t m_modeTimerCnt;

bool m_dstarEnable  = true;
bool m_dmrEnable    = true;
bool m_ysfEnable    = true;
bool m_p25Enable    = true;
bool m_nxdnEnable   = true;
bool m_pocsagEnable = true;

bool m_duplex = false;

bool m_tx  = false;
bool m_dcd = false;

CDStarRX   dstarRX;
CDStarTX   dstarTX;

uint8_t    m_control;

#if defined(DUPLEX)
CDMRIdleRX dmrIdleRX;
CDMRRX     dmrRX;
CDMRTX     dmrTX;
#endif

CDMRDMORX  dmrDMORX;
CDMRDMOTX  dmrDMOTX;

CYSFRX     ysfRX;
CYSFTX     ysfTX;

CP25RX     p25RX;
CP25TX     p25TX;

CNXDNRX    nxdnRX;
CNXDNTX    nxdnTX;

CPOCSAGTX  pocsagTX;

CCalDMR    calDMR;

#if defined(SEND_RSSI_DATA)
CCalRSSI   calRSSI;
#endif

CCWIdTX    cwIdTX;

CSerialPort serial;
CIO io;

void setup()
{
  serial.start();
}

void loop()
{
  serial.process();
  io.process();

  // The following is for transmitting
  if (m_dstarEnable && m_modemState == STATE_DSTAR)
    dstarTX.process();

  if (m_dmrEnable && m_modemState == STATE_DMR && m_calState == STATE_IDLE) {
#if defined(DUPLEX)
    if (m_duplex)
      dmrTX.process();
    else
      dmrDMOTX.process();
#else
    dmrDMOTX.process();
#endif
  }

  if (m_ysfEnable && m_modemState == STATE_YSF)
    ysfTX.process();

  if (m_p25Enable && m_modemState == STATE_P25)
    p25TX.process();

  if (m_nxdnEnable && m_modemState == STATE_NXDN)
    nxdnTX.process();

  if (m_pocsagEnable && (m_modemState == STATE_POCSAG || pocsagTX.busy()))
    pocsagTX.process();

  if (m_calState == STATE_DMRCAL || m_calState == STATE_DMRDMO1K)
    calDMR.process();

#if defined(SEND_RSSI_DATA)
  if (m_calState == STATE_RSSICAL)
    calRSSI.process();
#endif

  if (m_modemState == STATE_IDLE)
    cwIdTX.process();
}
