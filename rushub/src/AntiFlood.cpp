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

#include "AntiFlood.h"

#include <math.h>

namespace utils {

void AntiFlood::Del(Time &now) {
	if(mList) {
		List_t *Item = NULL;
		if(mList->mData && double(now - mList->mData->mTime) > mTime) {
			sItem * Data = mList->Remove(mList->mKey, Item);
			delete Data;
			delete mList;
			mList = Item;
			Del(now);
		} else {
			List_t * list = mList;
			while(list->mNext) {
				Item = list;
				list = list->mNext;
				if(list->mData && double(now - list->mData->mTime) > mTime) {
					sItem * Data = mList->Remove(list->mKey, Item);
					delete Data;
					list = Item;
				}
			}
		}
	}
}

bool AntiFlood::Check(HashType_t hash, Time now) {
	sItem * Item;
	if(!mList) {
		Item = new sItem();
		mList = new List_t(hash, Item);
		return false;
	}
	Item = mList->Find(hash);
	if(!Item) {
		Item = new sItem();
		mList->Add(hash, Item);
		return false;
	}
	if(Item->miCount < miCount) ++Item->miCount;
	else {
		if(::fabs(double(now - Item->mTime)) < mTime) {
			Item->mTime = now;
			Item->miCount = 0;
			return true;
		}
		Item->mTime = now;
		Item->miCount = 0;
	}
	return false;
}

}; // namespace utils