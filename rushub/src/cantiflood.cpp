/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2009-2010 by Setuper
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

#include "cantiflood.h"
#include <math.h>

namespace nUtils
{

void cAntiFlood::Del(cTime &now)
{
  if(mList) {
    List_t *Item = NULL;
    if(mList->mData && double(now - mList->mData->mTime) > mTime) {
      sItem * Data = mList->Remove(mList->mKey, Item);
      delete Data;
      delete mList;
      mList = Item;
      Del(now);
    } else {
      List_t *List = mList;
      while(List->mNext) {
        Item = List;
        List = List->mNext;
        if(List->mData && double(now - List->mData->mTime) > mTime) {
          sItem * Data = mList->Remove(List->mKey, Item);
          delete Data;
          List = Item;
        }
      }
    }
  }
}

bool cAntiFlood::Check(HashType_t Hash)
{
  sItem * Item;
  if(!mList) {
    Item = new sItem();
    mList = new List_t(Hash, Item);
    return false;
  }
  Item = mList->Find(Hash);
  if(!Item) {
    Item = new sItem();
    mList->Add(Hash, Item);
    return false;
  }
  bool bRet = false;
  if(Item->miCount < miCount) ++Item->miCount;
  else {
    cTime now;
    if(::fabs(double(now - Item->mTime)) < mTime) bRet = true;
    Item->mTime = now;
    Item->miCount = 0;
  }
  return bRet;
}

}; // nDCServer
