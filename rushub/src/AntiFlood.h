/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2011 by Setuper
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

#ifndef ANTIFLOOD_H
#define ANTIFLOOD_H

#include "Times.h"
#include "HashTable.h"

namespace utils {

/**
	\class AntiFlood
	\brief Antiflood
*/
class AntiFlood {

public:

	AntiFlood(unsigned & iCount, double & time);
	~AntiFlood();

	AntiFlood & operator = (const AntiFlood &) {
		return *this;
	}

	bool check(const string & ip, Time now);
	void del(Time & now);

private:

	struct sItem {
		Time mTime;
		unsigned miCount;
		sItem() : miCount(0) {
		}
	};

	typedef unsigned long HashType_t;
	typedef List<HashType_t, sItem *> List_t;
	List_t * mList;


	Hash<HashType_t> mHash;
	unsigned & miCount;
	double & mTime;

}; // AntiFlood

}; // namespace utils

#endif // ANTIFLOOD_H

/**
 * $Id$
 * $HeadURL$
 */
