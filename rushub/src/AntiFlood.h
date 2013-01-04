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

#ifndef ANTIFLOOD_H
#define ANTIFLOOD_H

#include "Times.h"
#include "HashTable.h"

namespace utils {

/// Main antiflood class
class AntiFlood {

public:

	AntiFlood(unsigned & iCount, double & time);
	~AntiFlood();

	int check(const string & ip, const Time & now);
	void del(Time & now);

private:

	/// Main antiflood item
	struct Item {
		Time mTime;
		unsigned miCount;
		bool mMoreOne;
		Item() : mTime(true), miCount(0), mMoreOne(false) {
		}
	};

	typedef unsigned long HashType_t;
	typedef List<HashType_t, Item *> List_t;
	List_t * mList;


	Hash<HashType_t> mHash;
	unsigned & miCount;
	double & mTime;

	AntiFlood & operator = (const AntiFlood &);

}; // class AntiFlood

} // namespace utils

#endif // ANTIFLOOD_H

/**
 * $Id$
 * $HeadURL$
 */
