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

#include "DcIpList.h"
#include "DcConn.h"

#include <string.h>

namespace dcserver {


DcIpList::DcIpList() : 
	Obj("DcIpList"),
	HashTable<IpList *> (1024)
{
}



DcIpList::~DcIpList() {
	IpList * ipList = NULL;
	for (HashTable<IpList *>::iterator it = IpTable::begin(); it != IpTable::end(); ++it) {
		ipList = (*it);
		if (ipList) {
			delete ipList;
		}
		ipList = NULL;
	}
}



bool DcIpList::add(DcConn * dcConn) {
	unsigned long hash = mHash(dcConn->getIp());
	IpList * ipList = IpTable::find(hash);
	if (ipList == NULL) {
		IpTable::add(hash, new IpList(static_cast<tSocket> (*dcConn), dcConn));
	} else {
		ipList->add(static_cast<tSocket> (*dcConn), dcConn);
	}
	return true;
}



bool DcIpList::remove(DcConn * dcConn) {
	unsigned long hash = mHash(dcConn->getIp());
	IpList * ipList = NULL, * ipLists = IpTable::find(hash);
	if (ipLists == NULL) {
		return false;
	}
	ipList = ipLists;
	Conn * conn = ipLists->remove(static_cast<tSocket> (*dcConn), ipList);
	if (ipList != ipLists) {
		if (ipList) {
			IpTable::update(hash, ipList);
		} else {
			IpTable::remove(hash); // Removing the list from hash-table
		}
		delete ipLists; // Removing old start element in the list
		ipLists = NULL;
	}
	return conn != NULL;
}



void DcIpList::sendToIp(const string & ip, const string & data, unsigned long profile, bool flush /*= true*/) {
	unsigned long ipHash = mHash(ip);
	IpList * ipList = IpTable::find(ipHash);
	while (ipList != NULL) {
		DcConn * dcConn = ipList->mData;
		if (dcConn && dcConn->getIp() == ip && dcConn->mIpRecv) {
			if (!profile || checkProfile(dcConn, profile)) {
				dcConn->send(data, true, flush);
			}
		}
		ipList = ipList->mNext;
	}
}



void DcIpList::sendToIpChat(const string & ip, const string & data, const string & nick, unsigned long profile, bool flush /*= true*/) {
	unsigned long ipHash = mHash(ip);
	IpList * ipList = IpTable::find(ipHash);
	while (ipList != NULL) {
		DcConn * dcConn = ipList->mData;
		if (dcConn && dcConn->getIp() == ip && dcConn->mIpRecv && !dcConn->mDcUser->getNick().empty()) {
			if (!profile || checkProfile(dcConn, profile)) {
				dcConn->mDcUser->sendToChat(data, nick, flush);
			}
		}
		ipList = ipList->mNext;
	}
}



void DcIpList::sendToIpPm(const string & ip, const string & data, const string & nick, const string & from, unsigned long profile, bool flush /*= true*/) {
	unsigned long ipHash = mHash(ip);
	IpList * ipList = IpTable::find(ipHash);
	while (ipList != NULL) {
		DcConn * dcConn = ipList->mData;
		if (dcConn && dcConn->getIp() == ip && dcConn->mIpRecv && !dcConn->mDcUser->getNick().empty()) {
			if (!profile || checkProfile(dcConn, profile)) {
				dcConn->mDcUser->sendToPm(data, nick, from, flush);
			}
		}
		ipList = ipList->mNext;
	}
}



bool DcIpList::checkProfile(DcConn * dcConn, unsigned long profile) {
	int p = dcConn->mDcUser->getParamForce(USER_PARAM_PROFILE)->getInt() + 1;
	if (p < 0) {
		p = -p;
	}
	if (p > 31) {
		p = (p % 32) - 1;
	}
	if (profile & static_cast<unsigned long> (1 << p)) {
		return true;
	}
	return false;
}


} // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
