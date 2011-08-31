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
	mCanSend(false),
	magicByte(0),
	share(0),
	nil(TAGNIL_NO),
	passive(false),
	unregHubs(0),
	regHubs(0),
	opHubs(0),
	slots(0),
	limit(0),
	open(0),
	bandwidth(0),
	download(0)
{
	mDcConnBase = NULL;
}



DcUser::~DcUser() {
	HashMap<string *>::iterator it = mParams.begin(), it_e = mParams.end();
	while (it != mParams.end()) {
		string * param = (*it);
		++it;
		delete param;
	}
	mDcConn = NULL;
	mDcConnBase = NULL;
	mDcServer = NULL;
}



const string & DcUser::uid() const {
	return mUid;
}



const string & DcUser::myInfoString() const {
	return myInfo;
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
	mUidHash = static_cast<unsigned long> (UserList::uidToLowerHash(uid));
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


//////////////////////////////////////////////////////////////////////////////////////


/** Get MyINFO */
const string & DcUser::getMyInfo() const {
	return myInfo;
}

/** Set MyINFO string (for plugins). With cmd & nick check */
bool DcUser::setMyInfo(const string & newMyInfo) {

	// TODO remove NmdcParser class from this function
	NmdcParser dcParser;
	if (NmdcParser::checkCmd(dcParser, newMyInfo, this) != NMDC_TYPE_MYNIFO) {
		return false;
	}
	setMyInfo(&dcParser);
	return true;
}

bool DcUser::setMyInfo(NmdcParser * parser) {
	if (myInfo != parser->mCommand) {
		myInfo = parser->mCommand;

		mDcServer->miTotalShare -= share;
		share = stringToInt64(parser->chunkString(CHUNK_MI_SIZE));
		mDcServer->miTotalShare += share;

		email = parser->chunkString(CHUNK_MI_MAIL);
		connection = parser->chunkString(CHUNK_MI_SPEED);

		size_t connSize = connection.size();
		if (connSize != 0) {
			magicByte = unsigned(connection[--connSize]);
			connection.assign(connection, 0, connSize);
		}

		//string * description = new string(parser->chunkString(CHUNK_MI_DESC));
		description = parser->chunkString(CHUNK_MI_DESC);
		parse(description);
	}
	return true;
}

/** Get share (for plugins) */
__int64 DcUser::getShare() const {
	return share;
}

bool DcUser::isPassive() const {
	return passive;
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



const string & DcUser::getDesc() const {
	return description;
}

const string & DcUser::getEmail() const {
	return email;
}

const string & DcUser::getConnection() const {
	return connection;
}

unsigned int DcUser::getByte() const {
	return magicByte;
}

unsigned int DcUser::getTagNil() const {
	return nil;
}

const string & DcUser::getTag() const {
	return tag;
}

const string & DcUser::getClient() const {
	return clientName;
}

const string & DcUser::getClientVersion() const {
	return clientVersion;
}

unsigned DcUser::getUnregHubs() const {
	return unregHubs;
}

unsigned DcUser::getRegHubs() const {
	return regHubs;
}

unsigned DcUser::getOpHubs() const {
	return opHubs;
}

unsigned DcUser::getSlots() const {
	return slots;
}

unsigned DcUser::getLimit() const {
	return limit;
}

unsigned DcUser::getOpen() const {
	return open;
}

unsigned DcUser::getBandwidth() const {
	return bandwidth;
}

unsigned DcUser::getDownload() const {
	return download;
}

const string & DcUser::getFraction() const {
	return fraction;
}

const string & DcUser::getMode() const {
	return mode;
}



const string * DcUser::getParam(unsigned int key) const {
	return mParams.find(key);
}


void DcUser::parse(string & description) {

	unsigned int OldNil = nil;
	nil = TAGNIL_NO; // Set null value for all params

	size_t l = description.size();
	if (l) { // optimization
		size_t i = description.find_last_of('<');
		if (i != description.npos && description[--l] == '>') {
			nil |= TAGNIL_TAG;
			string sOldTag = tag;
			++i;
			tag.assign(description, i, l - i);
			description.assign(description, 0, --i);
			if (tag.compare(sOldTag) != 0) { // optimization
				parseTag();
			} else {
				nil |= OldNil;
			}
		}
	}
}



void DcUser::parseTag() {

	/* Get clientName and clientVersion */

	nil |= TAGNIL_CLIENT;

	size_t clientPos = tag.find(',');
	size_t tagSize = tag.size();
	if (clientPos == tag.npos) {
		clientPos = tagSize;
	}

	size_t v = tag.find("V:");
	if (v != tag.npos) {
		nil |= TAGNIL_VERSION;
		clientVersion.assign(tag, v + 2, clientPos - v - 2);
		clientName.assign(tag, 0, v);
	} else {
		size_t s = tag.find(' ');
		if (s != tag.npos && s < clientPos) {
			++s;
			if (atof(tag.substr(s, clientPos - s).c_str())) {
				nil |= TAGNIL_VERSION;
				clientVersion.assign(tag, s, clientPos - s);
				clientName.assign(tag, 0, --s);
			} else {
				clientName.assign(tag, 0, clientPos);
			}
		} else {
			clientName.assign(tag, 0, clientPos);
		}
	}
	trim(clientName);
	trim(clientVersion);

	/* Get mode */
	size_t m = tag.find("M:");
	if (m != tag.npos) {
		nil |= TAGNIL_MODE;
		m += 2;
		size_t mPos = tag.find(',', m);
		if (mPos == tag.npos) {
			mPos = tagSize;
		}
		mode.assign(tag, m, mPos - m);
		if (mPos > m) {
			unsigned int p = mode[0];
			if(p == 80 || p == 53 || p == 83) {
				passive = true;
			}
		}
	}
	string tmp;

	/* hubs */
	size_t h = tag.find("H:");
	if (h != tag.npos) {
		h += 2;
		size_t unregPos = tag.find('/', h);
		if (unregPos == tag.npos) {
			unregPos = tag.find(',', h);
			if (unregPos == tag.npos) {
				unregPos = tagSize;
			}
		} else {
			size_t regPos = tag.find('/', ++unregPos);
			if (regPos == tag.npos) {
				regPos = tag.find(',', unregPos);
				if (regPos == tag.npos) {
					regPos = tagSize;
				}
			} else {
				size_t opPos = tag.find('/', ++regPos);
				if (opPos == tag.npos) {
					opPos = tag.find(',', regPos);
					if (opPos == tag.npos) {
						opPos = tagSize;
					}
				}
				opHubs = atoi(tmp.assign(tag, regPos, opPos - regPos).c_str());
				if (tmp.size()) {
					nil |= TAGNIL_UNREG;
				}
			}
			regHubs = atoi(tmp.assign(tag, unregPos, regPos - unregPos - 1).c_str());
			if (tmp.size()) {
				nil |= TAGNIL_REG;
			}
		}
		unregHubs = atoi(tmp.assign(tag, h, unregPos - h - 1).c_str());
		if (tmp.size()) {
			nil |= TAGNIL_OP;
		}
	}

	/* slots and limits */
	findIntParam("S:", slots, TAGNIL_SLOT);
	findIntParam("L:", limit, TAGNIL_LIMIT);
	findIntParam("O:", open, TAGNIL_OPEN);
	findIntParam("B:", bandwidth, TAGNIL_BANDWIDTH);
	findIntParam("D:", download, TAGNIL_DOWNLOAD);

	size_t f = tag.find("F:");
	if (f != tag.npos) {
		nil |= TAGNIL_FRACTION;
		f += 2;
		size_t fPos = tag.find(',', f);
		if(fPos == tag.npos) {
			fPos = tagSize;
		}
		fraction.assign(tag, f, fPos - f);
	}
}

void DcUser::findIntParam(const char * find, unsigned int & param, TagNil tagNil) {

	size_t pos = tag.find(find);
	if (pos != tag.npos) {

		pos += 2;
		size_t sepPos = tag.find(',', pos);

		if (sepPos == tag.npos) {
			sepPos = tag.size();
		}

		nil |= tagNil;

		string tmp;
		param = atoi(tmp.assign(tag, pos, sepPos - pos).c_str());
	}
}

// =====================================================================


}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
