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


int cDCUser::GetProfile() const {
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
void cDCUser::SetOpList(bool bInOpList) {
	if (bInOpList) {
		mDCServer->AddToOps(this);
	} else {
		mDCServer->DelFromOps(this);
	}
}


/** Set/unset user in IpList (for plugins) */
void cDCUser::SetIpList(bool bInIpList) {
	if (bInIpList) {
		mDCServer->AddToIpList(this);
	} else {
		mDCServer->DelFromIpList(this);
	}
}


/** Set/unset user in HideList (for plugins) */
void cDCUser::SetHide(bool bHide) {
	if (bHide) {
		mDCServer->AddToHide(this);
	} else {
		mDCServer->DelFromHide(this);
	}
}


/** Set/unset ForceMove flag */
void cDCUser::SetForceMove(bool bForceMove) {
	mbForceMove = bForceMove;
}


/** Set/unset Kick flag */
void cDCUser::SetKick(bool bKick) {
	mbKick = bKick;
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
const string & cDCUser::GetDesc(/*bool real = false */) const {
	return myInfo.getDescription();
}

/** Get e-mail (for plugins) */
const string & cDCUser::GetEmail(/*bool real = false */) const {
	return myInfo.getEmail();
}

/** Get connection (for plugins) */
const string & cDCUser::GetConnection(/*bool real = false */) const {
	return myInfo.getConnection();
}

/** Get byte (for plugins) */
unsigned cDCUser::GetByte(/*bool real = false */) const {
	return myInfo.getMagicByte();
}

/** Get share (for plugins) */
__int64 cDCUser::GetShare(/*bool real = false */) const {
	// !!!
	return myInfo.getShare();
}

bool cDCUser::IsPassive() const {
	// !!!
	return myInfo.dcTag.IsPassive();
}

const string & cDCUser::GetTag(/*bool real = false */) const {
	return myInfo.dcTag.getTag();
}

const string & cDCUser::GetClient(/*bool real = false */) const {
	return myInfo.dcTag.getClientName();
}

const string & cDCUser::GetVersion(/*bool real = false */) const {
	return myInfo.dcTag.getClientVersion();
}

unsigned cDCUser::GetUnRegHubs(/*bool real = false */) const {
	return myInfo.dcTag.getUnregHubs();
}

unsigned cDCUser::GetRegHubs(/*bool real = false */) const {
	return myInfo.dcTag.getRegHubs();
}

unsigned cDCUser::GetOpHubs(/*bool real = false */) const {
	return myInfo.dcTag.getOpHubs();
}

unsigned cDCUser::GetSlots(/*bool real = false */) const {
	return myInfo.dcTag.getSlots();
}

unsigned cDCUser::GetLimit(/*bool real = false */) const {
	return myInfo.dcTag.getLimit();
}

unsigned cDCUser::GetOpen(/*bool real = false */) const {
	return myInfo.dcTag.getOpen();
}

unsigned cDCUser::GetBandwidth(/*bool real = false */) const {
	return myInfo.dcTag.getBandwidth();
}

unsigned cDCUser::GetDownload(/*bool real = false */) const {
	return myInfo.dcTag.getDownload();
}

const string & cDCUser::GetFraction(/*bool real = false */) const {
	return myInfo.dcTag.getFraction();
}

const string & cDCUser::GetMode(/*bool real = false */) const {
	return myInfo.dcTag.getMode();
}

unsigned int cDCUser::getTagNil(/*bool real = false */) const {
	return myInfo.dcTag.getNil();
}


}; // nDCServer
