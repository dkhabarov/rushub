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

#ifndef DC_IP_LIST_H
#define DC_IP_LIST_H

#include "HashTable.h"
#include "DcConn.h"

using namespace ::std;
using namespace ::utils;

namespace dcserver {

typedef List<tSocket, DcConn *> IpList;



/// List which contain all IP of the DC server
class DcIpList : public Obj, public HashTable<IpList *> {

public:

	Hash<unsigned long> mHash;

public:

	DcIpList();
	virtual ~DcIpList();

	bool add(DcConn *);
	bool remove(DcConn *);

	void sendToIp(const string & ip, const string & data, unsigned long profiles = 0, bool flush = true);
	void sendToIpChat(const string & ip, const string & data, const string & nick, unsigned long profiles = 0, bool flush = true);
	void sendToIpPm(const string & ip, const string & data, const string & nick, const string & from, unsigned long profiles = 0, bool flush = true);

	virtual void onResize(size_t & currentSize, size_t & oldCapacity, size_t & newCapacity) {
		LOG(LEVEL_DEBUG, "Autoresizing: size = " << currentSize << 
			", capacity = " << oldCapacity << " -> " << newCapacity);
	}

	/// Iterator for IP list
	class iterator {

	public:

		IpList * mIpList; /** Pointer on element of the array */
		iterator() : mIpList(NULL){
		}
		iterator & operator = (const iterator & it) {
			mIpList = it.mIpList;
			return *this;
		}
		iterator(const iterator & it) : mIpList(NULL) {
			(*this) = it;
		}
		inline bool operator == (const iterator & it) {
			return mIpList == it.mIpList;
		}
		inline bool operator != (const iterator & it) {
			return mIpList != it.mIpList;
		}
		iterator & operator ++() {
			if (mIpList != NULL) {
				mIpList = mIpList->mNext;
			}
			return *this;
		}
		inline Conn * operator * () {
			return mIpList->mData;
		}

	}; // iterator

	iterator begin(const char * ip) {
		iterator it;
		it.mIpList = find(mHash(ip));
		return it;
	}
	iterator end() {
		return iterator();
	}

private:

	typedef HashTable<IpList *> IpTable;

private:

	bool checkProfile(DcConn *, unsigned long profiles);

}; // class DcIpList

} // namespace dcserver

#endif // DC_IP_LIST_H

/**
 * $Id$
 * $HeadURL$
 */
