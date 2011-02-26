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

bool DcIpList::Add(DcConn* conn) {
	IpList * ipList = mIpTable.Find(conn->getNetIp());
	if (ipList == NULL) {
		mIpTable.Add(conn->getNetIp(), new IpList((tSocket)(*conn), conn));
	} else {
		ipList->Add((tSocket)(*conn), conn);
	}
	return true;
}

bool DcIpList::Remove(DcConn* dcConn) {
	IpList * ipList = NULL, * ipLists = mIpTable.Find(dcConn->getNetIp());
	if(ipLists == NULL) return false;
	ipList = ipLists;
	Conn * conn = ipLists->Remove((tSocket)(*dcConn), ipList);
	if(ipList != ipLists) {
		if(ipList) mIpTable.Update(dcConn->getNetIp(), ipList);
		else mIpTable.Remove(dcConn->getNetIp()); /** Removing the list from hash-table */
		delete ipLists; /** removing old start element in the list */
		ipLists = NULL;
	}
	if(conn == NULL) return false;
	return true;
}

void DcIpList::sendToIp(const char *sIP, string &sData, unsigned long iProfile, bool bAddSep, bool bFlush) {
	sendToIp(Conn::Ip2Num(sIP), sData, iProfile, bAddSep, bFlush);
}

void DcIpList::SendToIPWithNick(const char *sIP, string &sStart, string &sEnd, unsigned long iProfile, bool bAddSep, bool bFlush) {
	SendToIPWithNick(Conn::Ip2Num(sIP), sStart, sEnd, iProfile, bAddSep, bFlush);
}

void DcIpList::sendToIp(unsigned long iIP, string &sData, unsigned long iProfile, bool bAddSep, bool bFlush) {
	miProfile = iProfile;
	msData1 = sData;
	mFlush = bFlush;
	mAddSep = bAddSep;
	IpList * ipList = mIpTable.Find(iIP);
	while(ipList != NULL) {
		send(ipList->mData);
		ipList = ipList->mNext;
	}
}

void DcIpList::SendToIPWithNick(unsigned long iIP, string &sStart, string &sEnd, unsigned long iProfile, bool bAddSep, bool bFlush) {
	miProfile = iProfile;
	msData1 = sStart;
	msData2 = sEnd;
	mFlush = bFlush;
	mAddSep = bAddSep;
	IpList * ipList = mIpTable.Find(iIP);
	while(ipList != NULL) {
		SendWithNick(ipList->mData);
		ipList = ipList->mNext;
	}
}

int DcIpList::send(DcConn * conn) {
	if(!conn || !conn->mbIpRecv) return 0;
	if(miProfile) {
		int iProfile = conn->miProfile + 1;
		if(iProfile < 0) iProfile = -iProfile;
		if(iProfile > 31) iProfile = (iProfile % 32) - 1;
		if(miProfile & (1 << iProfile))
			return conn->send(msData1, mAddSep, mFlush);
	} else {
		return conn->send(msData1, mAddSep, mFlush);
	}
	return 0;
}

int DcIpList::SendWithNick(DcConn * conn) {
	if(!conn || !conn->mDCUser || !conn->mbIpRecv) return 0;
	string sStr(msData1);
	sStr.append(conn->mDCUser->msNick);
	if(miProfile) {
		int iProfile = conn->miProfile + 1;
		if(iProfile < 0) iProfile = -iProfile;
		if(iProfile > 31) iProfile = (iProfile % 32) - 1;
		if(miProfile & (1 << iProfile)) {
			string sStr(msData1);
			sStr.append(conn->mDCUser->msNick);
			return conn->send(sStr.append(msData2), mAddSep, mFlush);
		}
	} else {
		return conn->send(sStr.append(msData2), mAddSep, mFlush);
	}
	return 0;
}

}; // namespace dcserver
