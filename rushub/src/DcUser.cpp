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

#include "DcUser.h"
#include "DcConn.h"
#include "DcServer.h"

namespace dcserver {


DcUser::DcUser() :
	Obj("DcUser"),
	DcUserBase(),
	mDcServer(NULL),
	mDcConn(NULL),
	mTimeEnter(true),
	mUidHash(0),
	mProfile(-1),
	mInOpList(false),
	mInIpList(false),
	mHide(false),
	mForceMove(false),
	mKick(false),
	mInUserList(false),
	mCanSend(false)
{
	mDcConnBase = NULL;
}



DcUser::~DcUser() {
	mDcConn = NULL;
	mDcConnBase = NULL;
	mDcServer = NULL;
}



const string & DcUser::uid() const {
	return mUid;
}



const string & DcUser::myInfoString() const {
	return myInfo.getMyInfo();
}



/** Get IP address of user */
const string & DcUser::ip() const {
	return mIp;
}



bool DcUser::hide() const {
	return mHide;
}



int DcUser::getProfile() const {
	return mProfile;
}



void DcUser::send(const string & data, bool addSep, bool flush) {
	if (mDcConn) {
		mDcConn->send(data, addSep, flush);
	}
}



void DcUser::send(const char * data, size_t len, bool addSep, bool flush) {
	if (mDcConn) {
		mDcConn->send(data, len, addSep, flush);
	}
}






void DcUser::setUid(const string & uid) {
	mUid = uid;

	// Calc uid hash
	mUidHash = static_cast<unsigned long> (UserList::nick2Key(uid));
}



void DcUser::setInUserList(bool inUserList) {
	mInUserList = inUserList;
}



void DcUser::setCanSend(bool canSend) {
	mCanSend = (mInUserList && mDcConn && mDcConn->isOk() && canSend);
}




/** Get nick */
const string & DcUser::getUid() const {
	return mUid;
}



unsigned long DcUser::getUidHash() const {
	return mUidHash;
}





bool DcUser::getInUserList() const {
	return mInUserList;
}



bool DcUser::getInOpList() const {
	return mInOpList;
}



bool DcUser::getInIpList() const {
	return mInIpList;
}



bool DcUser::getHide() const {
	return mHide;
}



bool DcUser::getForceMove() const {
	return mForceMove;
}



bool DcUser::getKick() const {
	return mKick;
}



void DcUser::setProfile(int profile) {
	mProfile = profile;
}



void DcUser::setIp(const string & ip) {
	if (ip.size()) {
		mIp = ip;
	}
}



/** Set/unset user in OpList (for plugins) */
void DcUser::setInOpList(bool inOpList) {
	if (inOpList) {
		if (!mInOpList) {
			mDcServer->mNmdcProtocol.addToOps(this); // refactoring to DcProtocol pointer
			mInOpList = true;
		}
	} else {
		if (mInOpList) {
			mDcServer->mNmdcProtocol.delFromOps(this); // refactoring to DcProtocol pointer
			mInOpList = false;
		}
	}
}


/** Set/unset user in IpList (for plugins) */
void DcUser::setInIpList(bool inIpList) {
	if (inIpList) {
		if (!mInIpList) {
			mDcServer->mNmdcProtocol.addToIpList(this); // refactoring to DcProtocol pointer
			mInIpList = true;
		}
	} else {
		if (mInIpList) {
			mDcServer->mNmdcProtocol.delFromIpList(this); // refactoring to DcProtocol pointer
			mInIpList = false;
		}
	}
}


/** Set/unset user in HideList (for plugins) */
void DcUser::setHide(bool hide) {
	if (hide) {
		if (!mHide) {
			mDcServer->mNmdcProtocol.addToHide(this); // refactoring to DcProtocol pointer
			mHide = true;
		}
	} else {
		if (mHide) {
			mDcServer->mNmdcProtocol.delFromHide(this); // refactoring to DcProtocol pointer
			mHide = false;
		}
	}
}


/** Set/unset forceMove flag */
void DcUser::setForceMove(bool forceMove) {
	mForceMove = forceMove;
}


/** Set/unset Kick flag */
void DcUser::setKick(bool kick) {
	mKick = kick;
}


/** Get MyINFO */
const string & DcUser::getMyInfo(/*bool real = false */) const {
	return myInfo.getMyInfo();
}

/** Set MyINFO string (for plugins). With cmd & nick check */
bool DcUser::setMyInfo(const string & newMyInfo) {

	// TODO remove DcParser class from this function

	DcParser dcParser;
	if (DcParser::checkCmd(dcParser, newMyInfo, this) != NMDC_TYPE_MYNIFO) {
		return false;
	}
	myInfo.setMyInfo(newMyInfo, &dcParser, mDcServer->miTotalShare);
	return true;
}

bool DcUser::setMyInfo(DcParser * parser) {
	myInfo.setMyInfo(parser->mCommand, parser, mDcServer->miTotalShare);
	return true;
}

/** Get share (for plugins) */
__int64 DcUser::getShare(/*bool real = false */) const {
	// !!!
	return myInfo.getShare();
}

bool DcUser::isPassive() const {
	// !!!
	return myInfo.dcTag.isPassive();
}



void DcUser::disconnect() {
	mDcConn->closeNice(9000, CLOSE_REASON_PLUGIN);
}

/// Get string of IP
const string & DcUser::getIp() const {
	if (mDcConn != NULL) {
		return mDcConn->getIp();
	}
	return mIp; // TODO remove this param (now for bot only)
}

/// Get string of server ip (host)
const string & DcUser::getIpConn() const{
	return mDcConn->getIpConn();
}

/// Get real clients port
int DcUser::getPort() const {
	return mDcConn->getPort();
}

/// Get connection port
int DcUser::getPortConn() const {
	return mDcConn->getPortConn();
}

/// Get mac address
const string & DcUser::getMacAddress() const {
	return mDcConn->getMacAddress();
}




// Used in plugins only
// =====================================================================
const string & DcUser::getSupports() const {
	return mSupports;
}

const string & DcUser::getVersion() const {
	return mVersion;
}

long DcUser::getConnectTime() const {
	return mTimeEnter.sec();
}

const string & DcUser::getData() const {
	return mData;
}

void DcUser::setData(const string & data) {
	mData = data;
}

const string & DcUser::getDesc(/*bool real = false */) const {
	return myInfo.getDescription();
}

const string & DcUser::getEmail(/*bool real = false */) const {
	return myInfo.getEmail();
}

const string & DcUser::getConnection(/*bool real = false */) const {
	return myInfo.getConnection();
}

unsigned DcUser::getByte(/*bool real = false */) const {
	return myInfo.getMagicByte();
}

unsigned int DcUser::getTagNil(/*bool real = false */) const {
	return myInfo.dcTag.getNil();
}

const string & DcUser::getTag(/*bool real = false */) const {
	return myInfo.dcTag.getTag();
}

const string & DcUser::getClient(/*bool real = false */) const {
	return myInfo.dcTag.getClientName();
}

const string & DcUser::getClientVersion(/*bool real = false */) const {
	return myInfo.dcTag.getClientVersion();
}

unsigned DcUser::getUnregHubs(/*bool real = false */) const {
	return myInfo.dcTag.getUnregHubs();
}

unsigned DcUser::getRegHubs(/*bool real = false */) const {
	return myInfo.dcTag.getRegHubs();
}

unsigned DcUser::getOpHubs(/*bool real = false */) const {
	return myInfo.dcTag.getOpHubs();
}

unsigned DcUser::getSlots(/*bool real = false */) const {
	return myInfo.dcTag.getSlots();
}

unsigned DcUser::getLimit(/*bool real = false */) const {
	return myInfo.dcTag.getLimit();
}

unsigned DcUser::getOpen(/*bool real = false */) const {
	return myInfo.dcTag.getOpen();
}

unsigned DcUser::getBandwidth(/*bool real = false */) const {
	return myInfo.dcTag.getBandwidth();
}

unsigned DcUser::getDownload(/*bool real = false */) const {
	return myInfo.dcTag.getDownload();
}

const string & DcUser::getFraction(/*bool real = false */) const {
	return myInfo.dcTag.getFraction();
}

const string & DcUser::getMode(/*bool real = false */) const {
	return myInfo.dcTag.getMode();
}

// =====================================================================


}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
