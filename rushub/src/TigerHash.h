/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2012 by Setuper
 * E-Mail: setuper at gmail dot com (setuper@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TIGER_HASH_H
#define TIGER_HASH_H

#include "stdinc.h"

#include <string.h>
#include <iostream>

using namespace std;

namespace dcserver {

class TigerHash {

public:

	/** Hash size */
	static const size_t BITS = 192;
	static const size_t BYTES = BITS / 8;

public:

	TigerHash();

	~TigerHash();

	/** Get result */
	uint8_t * getResult() const;

	/** Calculates the Tiger hash of the data */
	void update(const void * data, size_t len);

	/** Call once all data has been processed */
	uint8_t * finalize();

private:

	/** 64 byte blocks for the compress function */
	uint8_t tmp[64];

	/** State / final hash value */
	uint64_t res[3];

	/** Total number of bytes compressed */
	uint64_t pos;

	/** S boxes */
	static uint64_t table[];

private:

	void tigerCompress(const uint64_t * data, uint64_t state[3]) const;

};

}; // namespace dcserver

#endif // TIGER_HASH_H

/**
 * $Id$
 * $HeadURL$
 */
