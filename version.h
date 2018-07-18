/*
 *   Copyright (C) 2017,2018 by Andy Uribe CA6JAU
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

#if !defined(VERSION_H)
#define  VERSION_H

#include "Config.h"
#include "ADF7021.h"

#define VER_MAJOR       "1"
#define VER_MINOR       "4"
#define VER_REV         "5"
#define VERSION_DATE    "20180718"

#if defined(ZUMSPOT_ADF7021)
#define BOARD_INFO      "ZUMspot"
#elif defined(MMDVM_HS_HAT_REV12)
#define BOARD_INFO      "MMDVM_HS_Hat"
#elif defined(MMDVM_HS_DUAL_HAT_REV10)
#define BOARD_INFO      "MMDVM_HS_Dual_Hat"
#elif defined(NANO_HOTSPOT)
#define BOARD_INFO      "Nano_hotSPOT"
#elif defined(NANO_DV_REV10)
#define BOARD_INFO      "Nano_DV"
#else
#define BOARD_INFO      "MMDVM_HS"
#endif

#if defined(ADF7021_14_7456)
#define TCXO_FREQ       "14.7456"
#endif
#if defined(ADF7021_12_2880)
#define TCXO_FREQ       "12.2880"
#endif

#if defined(ENABLE_ADF7021) && defined(ADF7021_N_VER)
#define RF_CHIP         "ADF7021N"
#elif defined(ENABLE_ADF7021)
#define RF_CHIP         "ADF7021"
#endif

#if defined(DUPLEX)
#define RF_DUAL         "dual "
#else
#define RF_DUAL         ""
#endif

#define FW_VERSION      "v" VER_MAJOR "." VER_MINOR "." VER_REV " " VERSION_DATE

#define DESCRIPTION     BOARD_INFO "-" FW_VERSION " " TCXO_FREQ "MHz " RF_DUAL RF_CHIP " FW by CA6JAU"

#if defined(MADEBYMAKEFILE)
#include "GitVersion.h"
#endif

#if defined(GITVERSION)
#define concat(a, b) a " GitID #" b ""
const char HARDWARE[] = concat(DESCRIPTION, GITVERSION);
#else
#define concat(a, b, c) a " (Build: " b " " c ")"
const char HARDWARE[] = concat(DESCRIPTION, __TIME__, __DATE__);
#endif

#endif

