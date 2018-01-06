/*
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

#if !defined(VERSION_H)
#define  VERSION_H

#include "Config.h"

#define VER_MAJOR       "1"
#define VER_MINOR       "1"
#define VER_REV         "0"
#define VERSION_DATE    "20180106"

#if defined(ZUMSPOT_ADF7021)
#define BOARD_INFO      "ZUMspot"
#elif defined(MMDVM_HS_HAT_REV12)
#define BOARD_INFO      "MMDVM_HS_Hat"
#else
#define BOARD_INFO      "MMDVM_HS"
#endif

#if defined(ENABLE_ADF7021)
#define RF_CHIP         "ADF7021"
#endif

#if defined(DUPLEX)
#define RF_DUAL         "dual "
#else
#define RF_DUAL         ""
#endif

#define FW_VERSION      "v" VER_MAJOR "." VER_MINOR "." VER_REV " " VERSION_DATE

#define DESCRIPTION     BOARD_INFO "-" FW_VERSION " " RF_DUAL RF_CHIP " FW by CA6JAU"

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

