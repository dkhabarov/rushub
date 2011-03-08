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

#include "DcIpList.h"
#include "DcConn.h"

namespace dcserver {

DcIpList::DcIpList() : Obj("DcIpList"), mFlush(false), mAddSep(false) {
}

DcIpList::~DcIpList() {
	IpList * ipList = NULL;
	for (IpTable::iterator it = mIpTable.begin(); it != mIpTable.end(); ++it) {
		ipList = (*it);
		if (ipList) {
			delete ipList;
		}
		ipList = NULL;
	}
}

bool DcIpList::Add(DcConn * conn) {
	IpList * ipList = mIpTable.Find(conn->getNetIp());
	if (ipList == NULL) {
		mIpTable.Add(conn->getNetIp(), new IpList((tSocket)(*conn), conn));
	} else {
		ipList->Add((tSocket)(*conn), conn);
	}
	return true;
}

bool DcIpList::Remove(DcConn * dcConn) {
	IpList * ipList = NULL, * ipLists = mIpTable.Find(dcConn->getNetIp());
	if (ipLists == NULL) {
		return false;
	}
	ipList = ipLists;
	Conn * conn = ipLists->Remove((tSocket)(*dcConn), ipList);
	if (ipList != ipLists) {
		if (ipList) {
			mIpTable.Update(dcConn->getNetIp(), ipList);
		} else {
			mIpTable.Remove(dcConn->getNetIp()); /** Removing the list from hash-table */
		}
		delete ipLists; /** removing old start element in the list */
		ipLists = NULL;
	}
	return conn != NULL;
}

void DcIpList::sendToIp(const char * ip, string & data, unsigned long profile, bool addSep, bool flush) {
	sendToIp(Conn::ip2Num(ip), data, profile, addSep, flush);
}

void DcIpList::SendToIPWithNick(const char * ip, string & start, string & end, unsigned long profile, bool addSep, bool flush) {
	SendToIPWithNick(Conn::ip2Num(ip), start, end, profile, addSep, flush);
}

void DcIpList::sendToIp(unsigned long ip, string & data, unsigned long profile, bool addSep, bool flush) {
	miProfile = profile;
	msData1 = data;
	mFlush = flush;
	mAddSep = addSep;
	IpList * ipList = mIpTable.Find(ip);
	while (ipList != NULL) {
		send(ipList->mData);
		ipList = ipList->mNext;
	}
}

void DcIpList::SendToIPWithNick(unsigned long ip, string & start, string & end, unsigned long profile, bool addSep, bool flush) {
	miProfile = profile;
	msData1 = start;
	msData2 = end;
	mFlush = flush;
	mAddSep = addSep;
	IpList * ipList = mIpTable.Find(ip);
	while (ipList != NULL) {
		SendWithNick(ipList->mData);
		ipList = ipList->mNext;
	}
}

int DcIpList::send(DcConn * dcConn) {
	if (!dcConn || !dcConn->mbIpRecv) {
		return 0;
	}
	if (miProfile) {
		int profile = dcConn->miProfile + 1;
		if (profile < 0) {
			profile = -profile;
		}
		if (profile > 31) {
			profile = (profile % 32) - 1;
		}
		if (miProfile & (1 << profile)) {
			return dcConn->send(msData1, mAddSep, mFlush);
		}
	} else {
		return dcConn->send(msData1, mAddSep, mFlush);
	}
	return 0;
}

int DcIpList::SendWithNick(DcConn * dcConn) {
	if (!dcConn || !dcConn->mDcUser || !dcConn->mbIpRecv) {
		return 0;
	}
	string str(msData1);
	str.append(dcConn->mDcUser->msNick);
	if (miProfile) {
		int profile = dcConn->miProfile + 1;
		if (profile < 0) {
			profile = -profile;
		}
		if (profile > 31) {
			profile = (profile % 32) - 1;
		}
		if (miProfile & (1 << profile)) {
			string str(msData1);
			str.append(dcConn->mDcUser->msNick);
			return dcConn->send(str.append(msData2), mAddSep, mFlush);
		}
	} else {
		return dcConn->send(str.append(msData2), mAddSep, mFlush);
	}
	return 0;
}

}; // namespace dcserver
