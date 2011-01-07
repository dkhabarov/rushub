/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2009-2011 by Setuper
 * E-Mail: setuper at gmail dot com (setuper@gmail.com)

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CANTIFLOOD_H
#define CANTIFLOOD_H

#include "ctime.h"
#include "tchashtable.h"

namespace nUtils {

/**
	\class cAntiFlood
	\brief Antiflood
*/
class cAntiFlood {

public:

	struct sItem {
		cTime mTime;
		unsigned miCount;
		sItem() : miCount(0) {}
	};

	typedef unsigned long HashType_t;
	typedef tcList<HashType_t, sItem*> List_t;
	List_t * mList;

private:
	unsigned & miCount;
	double & mTime;

public:

	cAntiFlood(unsigned &iCount, double &Time) : mList(NULL), miCount(iCount), mTime(Time) {}
	~cAntiFlood(){}
	bool Check(HashType_t Hash, cTime now);
	void Del(cTime &now);

}; // cAntiFlood

}; // nDCServer

#endif // CANTIFLOOD_H
