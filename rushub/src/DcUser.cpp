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

#include "DcUser.h"
#include "DcConn.h"
#include "DcServer.h"

namespace dcserver {


DcUser::DcUser() :
	Obj("DcUser"),
	DcUserBase(),
	mDcServer(NULL),
	mDcConn(NULL),
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


DcUser::DcUser(const string & sNick) :
	Obj("DcUser"),
	DcUserBase(),
	mDcServer(NULL),
	mDcConn(NULL),
	mInOpList(false),
	mInIpList(false),
	mHide(false),
	mForceMove(false),
	mKick(false),
	mInUserList(false),
	mCanSend(false)
{
	mDcConnBase = NULL;
	msNick = sNick;
}


DcUser::~DcUser() {
	mDcConn = NULL;
	mDcConnBase = NULL;
	mDcServer = NULL;
}



void DcUser::setInUserList(bool inUserList) {
	mInUserList = inUserList;
}



void DcUser::setCanSend(bool canSend) {
	mCanSend = (mInUserList && mDcConn && mDcConn->isOk() && canSend);
}



void DcUser::send(const string & data, bool addSep, bool flush) {
	if (mDcConn) {
		mDcConn->send(data, addSep, flush);
	}
}



/** Get IP address of user */
const string & DcUser::getIp() const {
	return mIp;
}



/** Get nick */
const string & DcUser::getNick() const {
	return msNick;
}



const string & DcUser::Nick() const {
	return msNick;
}



const string & DcUser::MyINFO() const {
	return myInfo.getMyInfo();
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



bool DcUser::Hide() const {
	return mHide;
}



bool DcUser::getForceMove() const {
	return mForceMove;
}



bool DcUser::getKick() const {
	return mKick;
}



int DcUser::getProfile() const {
	// 30 profile for bot
	return mDcConn != NULL ? mDcConn->miProfile : 30;
}


void DcUser::SetIp(const string & ip) {
	if (ip.size()) {
		mIp = ip;
	}
}



/** Set/unset user in OpList (for plugins) */
void DcUser::setInOpList(bool inOpList) {
	if (inOpList) {
		mDcServer->mDcProtocol.addToOps(this);
	} else {
		mDcServer->mDcProtocol.delFromOps(this);
	}
}


/** Set/unset user in IpList (for plugins) */
void DcUser::setInIpList(bool inIpList) {
	if (inIpList) {
		mDcServer->mDcProtocol.addToIpList(this);
	} else {
		mDcServer->mDcProtocol.delFromIpList(this);
	}
}


/** Set/unset user in HideList (for plugins) */
void DcUser::setHide(bool hide) {
	if (hide) {
		mDcServer->mDcProtocol.addToHide(this);
	} else {
		mDcServer->mDcProtocol.delFromHide(this);
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
const string & DcUser::getMyINFO(/*bool real = false */) const {
	return myInfo.getMyInfo();
}

/** Set MyINFO string (for plugins). With cmd & nick check */
bool DcUser::setMyINFO(const string & newMyInfo) {

	// TODO remove DcParser class from this function

	DcParser dcParser;
	if (DcParser::checkCmd(dcParser, newMyInfo, this) != NMDC_TYPE_MYNIFO) {
		return false;
	}
	myInfo.setMyInfo(newMyInfo, &dcParser, mDcServer->miTotalShare);
	return true;
}

bool DcUser::setMyINFO(DcParser * parser) {
	myInfo.setMyInfo(parser->mCommand, parser, mDcServer->miTotalShare);
	return true;
}



/** Get description (for plugins) */
const string & DcUser::getDesc(/*bool real = false */) const {
	return myInfo.getDescription();
}

/** Get e-mail (for plugins) */
const string & DcUser::getEmail(/*bool real = false */) const {
	return myInfo.getEmail();
}

/** Get connection (for plugins) */
const string & DcUser::getConnection(/*bool real = false */) const {
	return myInfo.getConnection();
}

/** Get byte (for plugins) */
unsigned DcUser::getByte(/*bool real = false */) const {
	return myInfo.getMagicByte();
}

/** Get share (for plugins) */
__int64 DcUser::getShare(/*bool real = false */) const {
	// !!!
	return myInfo.getShare();
}

bool DcUser::IsPassive() const {
	// !!!
	return myInfo.dcTag.IsPassive();
}

const string & DcUser::getTag(/*bool real = false */) const {
	return myInfo.dcTag.getTag();
}

const string & DcUser::getClient(/*bool real = false */) const {
	return myInfo.dcTag.getClientName();
}

const string & DcUser::getVersion(/*bool real = false */) const {
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

unsigned int DcUser::getTagNil(/*bool real = false */) const {
	return myInfo.dcTag.getNil();
}


}; // namespace dcserver
