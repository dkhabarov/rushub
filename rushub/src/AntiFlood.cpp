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
	List_t * Item = NULL;
	while (mList != NULL) {
		Item * Data = mList->remove(mList->mKey, Item);
		delete Data;
		delete mList;
		mList = Item;
	}
}



void AntiFlood::del(Time & now) {
	if (mList) {
		List_t * Item = NULL;
		if (mList->mData && double(now - mList->mData->mTime) > mTime) {
			Item * Data = mList->remove(mList->mKey, Item);
			delete Data;
			delete mList;
			mList = Item;
			del(now);
		} else {
			List_t * list = mList;
			while (list->mNext) {
				Item = list;
				list = list->mNext;
				if (list->mData && double(now - list->mData->mTime) > mTime) {
					Item * Data = mList->remove(list->mKey, Item);
					delete Data;
					list = Item;
				}
			}
		}
	}
}



bool AntiFlood::check(const string & ip, const Time & now) {
	HashType_t hash = mHash(ip);
	Item * Item = NULL;
	if (!mList) {
		Item = new Item();
		mList = new List_t(hash, Item);
		return false;
	}

	Item = mList->find(hash);
	if (!Item) {
		Item = new Item();
		mList->add(hash, Item);
		return false;
	}

	if (Item->miCount < miCount) {
		++Item->miCount;
	} else {
		Item->miCount = 0;
		if (::fabs(double(now - Item->mTime)) < mTime) {
			Item->mTime = now;
			return true;
		}
		Item->mTime = now;
	}
	return false;
}


}; // namespace utils

/**
 * $Id$
 * $HeadURL$
 */
