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

#if (defined _DEBUG) && (!defined UNITTESTS_OFF)

#include "UnitTest++.h"
#include "TigerHash.h"
#include "Encoder.h"

using namespace utils;
using namespace dcserver;

SUITE(TestTigerHash) {


TEST(TestTigerHash_1) {

	string pid("BZUJ2ZJWXMYHCCCYB5OGRLWPAUQRWTTTBSSGTKY"); // end with: A, I, Q, Y (other equal these)

	uint8_t buf[TigerHash::BYTES];
	Encoder::fromBase32(pid.c_str(), buf, sizeof(buf));

	TigerHash th;
	th.update((uint8_t *) buf, sizeof(buf));

	CHECK_EQUAL("QEVJTDTMHUVWRZWWAD7YNVYK2C2SNHLWFRJMHGY", Encoder::toBase32(th.finalize(), TigerHash::BYTES));
}

TEST(TestTigerHash_2) {

	string pid("BZUJ2ZJWXMYHCCCYB5OGRLWPAUQRWTTTBSSGTKZ");

	uint8_t buf[TigerHash::BYTES];
	Encoder::fromBase32(pid.c_str(), buf, sizeof(buf));

	TigerHash th;
	th.update((uint8_t *) buf, sizeof(buf));

	CHECK_EQUAL("QEVJTDTMHUVWRZWWAD7YNVYK2C2SNHLWFRJMHGY", Encoder::toBase32(th.finalize(), TigerHash::BYTES));
}

}; // SUITE(TestTigerHash)

#endif // _DEBUG

/**
 * $Id$
 * $HeadURL$
 */
