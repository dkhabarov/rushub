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

#include "cdcuser.h"
#include "cdcconn.h"
#include "cdcserver.h"

namespace nDCServer {


cDCUser::cDCUser() :
	cObj("cDCUser"),
	cDCUserBase(),
	mDCServer(NULL),
	mDCConn(NULL),
	mbInUserList(false),
	mbInOpList(false),
	mbInIpList(false),
	mbHide(false),
	mbForceMove(false),
	mbKick(false)
{
	mDCConnBase = NULL;
}


cDCUser::cDCUser(const string & sNick) :
	cObj("cDCUser"),
	cDCUserBase(),
	mDCServer(NULL),
	mDCConn(NULL),
	mbInUserList(false),
	mbInOpList(false),
	mbInIpList(false),
	mbHide(false),
	mbForceMove(false),
	mbKick(false)
{
	mDCConnBase = NULL;
	msNick = sNick;
}


cDCUser::~cDCUser() {
	mDCConn = NULL;
	mDCConnBase = NULL;
	mDCServer = NULL;
}


bool cDCUser::CanSend() {
	return mbInUserList && mDCConn && mDCConn->mbOk;
}


void cDCUser::Send(const string & sData, bool bAddSep, bool bFlush) {
	if(mDCConn) {
		mDCConn->Send(sData, bAddSep, bFlush);
	}
}


int cDCUser::getProfile() const {
	if (mDCConn) {
		return mDCConn->miProfile;
	} else {
		return 30; // 30 profile for bot
	}
}


void cDCUser::SetIp(const string & sIP) {
	if (sIP.size() && cDCConn::CheckIp(sIP)) {
		msIp = sIP;
	}
}



/** Set/unset user in OpList (for plugins) */
void cDCUser::setInOpList(bool inOpList) {
	if (inOpList) {
		mDCServer->AddToOps(this);
	} else {
		mDCServer->DelFromOps(this);
	}
}


/** Set/unset user in IpList (for plugins) */
void cDCUser::setInIpList(bool inIpList) {
	if (inIpList) {
		mDCServer->AddToIpList(this);
	} else {
		mDCServer->DelFromIpList(this);
	}
}


/** Set/unset user in HideList (for plugins) */
void cDCUser::setHide(bool hide) {
	if (hide) {
		mDCServer->AddToHide(this);
	} else {
		mDCServer->DelFromHide(this);
	}
}


/** Set/unset ForceMove flag */
void cDCUser::setForceMove(bool forceMove) {
	mbForceMove = forceMove;
}


/** Set/unset Kick flag */
void cDCUser::setKick(bool kick) {
	mbKick = kick;
}


/** Get MyINFO */
const string & cDCUser::GetMyINFO(/*bool real = false */) const {
	return myInfo.getMyInfo();
}

/** Set MyINFO string (for plugins). With cmd & nick check */
bool cDCUser::SetMyINFO(const string & newMyInfo, const string & nick) {
	if (mDCServer->CheckCmd(newMyInfo) != eDC_MYNIFO || 
		mDCServer->mDCParser.ChunkString(eCH_MI_NICK) != nick)
	{
		return false;
	}
	myInfo.setMyInfo(newMyInfo, &mDCServer->mDCParser, mDCServer->miTotalShare);
	return true;
}

bool cDCUser::SetMyINFO(cDCParserBase * parser) {
	myInfo.setMyInfo(parser->msStr, parser, mDCServer->miTotalShare);
	return true;
}



/** Get description (for plugins) */
const string & cDCUser::getDesc(/*bool real = false */) const {
	return myInfo.getDescription();
}

/** Get e-mail (for plugins) */
const string & cDCUser::getEmail(/*bool real = false */) const {
	return myInfo.getEmail();
}

/** Get connection (for plugins) */
const string & cDCUser::getConnection(/*bool real = false */) const {
	return myInfo.getConnection();
}

/** Get byte (for plugins) */
unsigned cDCUser::getByte(/*bool real = false */) const {
	return myInfo.getMagicByte();
}

/** Get share (for plugins) */
__int64 cDCUser::getShare(/*bool real = false */) const {
	// !!!
	return myInfo.getShare();
}

bool cDCUser::IsPassive() const {
	// !!!
	return myInfo.dcTag.IsPassive();
}

const string & cDCUser::getTag(/*bool real = false */) const {
	return myInfo.dcTag.getTag();
}

const string & cDCUser::getClient(/*bool real = false */) const {
	return myInfo.dcTag.getClientName();
}

const string & cDCUser::getVersion(/*bool real = false */) const {
	return myInfo.dcTag.getClientVersion();
}

unsigned cDCUser::getUnregHubs(/*bool real = false */) const {
	return myInfo.dcTag.getUnregHubs();
}

unsigned cDCUser::getRegHubs(/*bool real = false */) const {
	return myInfo.dcTag.getRegHubs();
}

unsigned cDCUser::getOpHubs(/*bool real = false */) const {
	return myInfo.dcTag.getOpHubs();
}

unsigned cDCUser::getSlots(/*bool real = false */) const {
	return myInfo.dcTag.getSlots();
}

unsigned cDCUser::getLimit(/*bool real = false */) const {
	return myInfo.dcTag.getLimit();
}

unsigned cDCUser::getOpen(/*bool real = false */) const {
	return myInfo.dcTag.getOpen();
}

unsigned cDCUser::getBandwidth(/*bool real = false */) const {
	return myInfo.dcTag.getBandwidth();
}

unsigned cDCUser::getDownload(/*bool real = false */) const {
	return myInfo.dcTag.getDownload();
}

const string & cDCUser::getFraction(/*bool real = false */) const {
	return myInfo.dcTag.getFraction();
}

const string & cDCUser::getMode(/*bool real = false */) const {
	return myInfo.dcTag.getMode();
}

unsigned int cDCUser::getTagNil(/*bool real = false */) const {
	return myInfo.dcTag.getNil();
}


}; // nDCServer
