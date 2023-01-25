/*
 *   Copyright (C) 2015,2016,2017,2018 by Jonathan Naylor G4KLX
 *   Copyright (C) 2018,2019 by Andy Uribe CA6JAU
 *   Copyright (C) 2019 by Florian Wolters DF2ET
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

#if !defined(POCSAGTX_H_)
#define POCSAGTX_H_


class CPOCSAGTX {
public:
	CPOCSAGTX();

	uint8_t writeData(const uint8_t *data, uint8_t length);

	void setTXDelay(uint8_t delay);

	uint8_t setCal(const uint8_t *data, uint8_t length);

	void createCal();

	uint8_t getSpace() const;

	void process();

	bool busy();

private:
	CSerialRB m_buffer;
	uint8_t m_poBuffer[200U];
	uint16_t m_poLen;
	uint16_t m_poPtr;
	uint16_t m_txDelay;
	bool m_delay;
	bool m_cal;

	void writeByte(uint8_t c);

};

#endif	/* !POCSAGTX_H_ */
