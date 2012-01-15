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

#include "AntiFlood.h"

#include <math.h>


namespace utils {



AntiFlood::AntiFlood(unsigned & iCount, double & time) :
	mList(NULL),
	miCount(iCount),
	mTime(time)
{
}



AntiFlood::~AntiFlood() {
	List_t * list = NULL;
	while (mList != NULL) {
		Item * Data = mList->remove(mList->mKey, list);
		delete Data;
		delete mList;
		mList = list;
	}
}



void AntiFlood::del(Time & now) {
	if (mList) {
		List_t * lst = NULL;
		if (mList->mData && double(now - mList->mData->mTime) > mTime) {
			Item * Data = mList->remove(mList->mKey, lst);
			delete Data;
			delete mList;
			mList = lst;
			del(now);
		} else {
			List_t * list = mList;
			while (list->mNext) {
				lst = list;
				list = list->mNext;
				if (list->mData && double(now - list->mData->mTime) > mTime) {
					Item * Data = mList->remove(list->mKey, lst);
					delete Data;
					list = lst;
				}
			}
		}
	}
}



bool AntiFlood::check(const string & ip, const Time & now) {
	HashType_t hash = mHash(ip);
	Item * item = NULL;
	if (!mList) {
		item = new Item();
		mList = new List_t(hash, item);
		return false;
	}

	item = mList->find(hash);
	if (!item) {
		item = new Item();
		mList->add(hash, item);
		return false;
	}

	if (item->miCount < miCount) {
		++item->miCount;
	} else {
		item->miCount = 0;
		if (::fabs(double(now - item->mTime)) < mTime) {
			item->mTime = now;
			return true;
		}
		item->mTime = now;
	}
	return false;
}


}; // namespace utils

/**
 * $Id$
 * $HeadURL$
 */
