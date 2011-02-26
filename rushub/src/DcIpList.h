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

#ifndef DC_IP_LIST_H
#define DC_IP_LIST_H

#include "HashTable.h"
#include "DcConn.h"

using namespace ::std;
using namespace ::utils;

namespace dcserver {

class DcIpList : public Obj {

protected:

	typedef List<tSocket, DcConn *> IpList;
	typedef HashTable<IpList *> IpTable;
	IpTable mIpTable;
	bool mFlush, mAddSep;
	unsigned long miProfile;
	string msData1, msData2;

public:

	DcIpList();
	virtual ~DcIpList();

	bool Add(DcConn*);
	bool Remove(DcConn*);

	void sendToIp(unsigned long iIP, string &sData, unsigned long iProfile = 0, bool bAddSep = false, bool bFlush = true);
	void sendToIp(const char *sIP, string &sData, unsigned long iProfile = 0, bool bAddSep = false, bool bFlush = true);
	void SendToIPWithNick(unsigned long iIP, string &sStart, string &sEnd, unsigned long iProfile = 0, bool bAddSep = false, bool bFlush = true);
	void SendToIPWithNick(const char *sIP, string &sStart, string &sEnd, unsigned long iProfile = 0, bool bAddSep = false, bool bFlush = true);

	bool AutoResize() {
		unsigned size, capacity, newSize;
		if(mIpTable.AutoResize(size, capacity, newSize) && Log(3)) {
			LogStream() << "Autoresizing: size = " << size << 
			", capacity = " << capacity << " -> " + newSize << endl;
			return true;
		}
		return false;
	}

	class iterator {

	public:

		IpList * mIpList; /** Pointer on element of the array */
		iterator() : mIpList(NULL){}
		iterator & operator = (const iterator &it){ mIpList = it.mIpList; return *this; }
		iterator(const iterator &it){ (*this) = it; }
		bool operator == (const iterator & it){ return mIpList == it.mIpList; }
		bool operator != (const iterator & it){ return mIpList != it.mIpList; }
		iterator & operator ++() {
			if(mIpList != NULL) mIpList = mIpList->mNext;
			return *this;
		}
		Conn* operator *(){ return mIpList->mData; }

	}; // iterator

	iterator begin(const char * sIP) {
		return begin(Conn::Ip2Num(sIP));
	}
	iterator begin(unsigned long iIP) {
		iterator it;
		it.mIpList = mIpTable.Find(iIP);
		return it;
	}
	iterator end() {
		return iterator();
	}

protected:

	int send(DcConn * conn);
	int SendWithNick(DcConn * conn);

}; // class DcIpList

}; // namespace dcserver

#endif // DC_IP_LIST_H
