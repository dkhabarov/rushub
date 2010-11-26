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

#ifndef CDCIPLIST_H
#define CDCIPLIST_H

#include "tchashtable.h"
#include "cdcconn.h"

using namespace nUtils;
using namespace std;

namespace nDCServer {

class cDCIPList : public cObj {

protected:

	typedef tcList<tSocket, cDCConn*> tItem;
	typedef tcHashTable<tItem*> tIPList;
	tIPList mIPList;
	bool mbFlush, mbAddSep;
	unsigned long miProfile;
	string msData1, msData2;

public:
	cDCIPList();
	virtual ~cDCIPList();

	bool Add(cDCConn*);
	bool Remove(cDCConn*);

	void SendToIP(unsigned long iIP, string &sData, unsigned long iProfile = 0, bool bAddSep = false, bool bFlush = true);
	void SendToIP(const char *sIP, string &sData, unsigned long iProfile = 0, bool bAddSep = false, bool bFlush = true);
	void SendToIPWithNick(unsigned long iIP, string &sStart, string &sEnd, unsigned long iProfile = 0, bool bAddSep = false, bool bFlush = true);
	void SendToIPWithNick(const char *sIP, string &sStart, string &sEnd, unsigned long iProfile = 0, bool bAddSep = false, bool bFlush = true);

	bool AutoResize() {
		unsigned iSize, iCapacity, iNewSize;
		if(mIPList.AutoResize(iSize, iCapacity, iNewSize) && Log(3)) {
			LogStream() << "Autoresizing: miSize = " << iSize << 
			", miCapacity = " << iCapacity << " -> " + iNewSize << endl;
			return true;
		}
		return false;
	}

	class iterator {
	public:
		tItem * mItem; /** Pointer on element of the array */
		iterator() : mItem(NULL){}
		iterator & operator = (const iterator &it){ mItem = it.mItem; return *this; }
		iterator(const iterator &it){ (*this) = it; }
		bool operator == (const iterator &it){ return mItem == it.mItem; }
		bool operator != (const iterator &it){ return mItem != it.mItem; }
		iterator & operator ++() {
			if(mItem != NULL) mItem = mItem->mNext;
			return *this;
		}
		cConn* operator *(){ return mItem->mData; }
	}; // iterator

	iterator begin(const char* sIP) {
		return begin(cConn::Ip2Num(sIP));
	}
	iterator begin(unsigned long iIP) {
		iterator it;
		it.mItem = mIPList.Find(iIP);
		return it;
	}
	iterator end() {
		return iterator();
	}

protected:

	int Send(cDCConn * Conn);
	int SendWithNick(cDCConn * Conn);

}; // cDCIPList

}; // nDCServer

#endif // CDCIPLIST_H
