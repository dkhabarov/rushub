/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2013 by Setuper
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


#ifndef ENCODER_H
#define ENCODER_H

#include "stdinc.h"

#include <string.h>
#include <iostream>

using namespace std;

namespace utils {

class Encoder {

public:

	static string & toBase32(const uint8_t * src, size_t len, string & tgt);
	static string toBase32(const uint8_t * src, size_t len) {
		string tmp;
		return toBase32(src, len, tmp);
	}
	static void fromBase32(const char * src, uint8_t * dst, size_t len);
	static bool isBase32(const char * src);

private:

	static const int8_t base32Table[];
	static const signed char base32Alphabet[];

}; // class Encoder

} // namespace utils

#endif // ENCODER_H

/**
 * $Id$
 * $HeadURL$
 */
