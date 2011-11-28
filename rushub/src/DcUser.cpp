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
	mInUserList(false),
	mCanSend(false)
{
	mDcConnBase = NULL;
}



DcUser::~DcUser() {
	HashMap<string *>::iterator _it = mParams.begin(), _it_e = mParams.end();
	while (_it != _it_e) {
		string * param = (*_it++);
		delete param;
	}

	HashMap<Param *>::iterator it = mParamList.begin(), it_e = mParamList.end();
	while (it != it_e) {
		Param * param = (*it++);
		delete param;
	}

	mDcConn = NULL;
	mDcConnBase = NULL;
	mDcServer = NULL;
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



void DcUser::disconnect() {
	mDcConn->closeNice(9000, CLOSE_REASON_PLUGIN);
}



const string * DcUser::getParam(unsigned int key) const {
	return mParams.find(key);
}



void DcUser::setParam(unsigned int key, const char * value) {
	if (value != NULL) {
		updateParam(key, value);
	} else {
		mParams.remove(key);
	}
}



const string & DcUser::getStringParam(unsigned int key) const {
	switch (key) {
		case USER_STRING_PARAM_DATA :
			return mData;

		case USER_STRING_PARAM_SUPPORTS :
			return mSupports;

		case USER_STRING_PARAM_NMDC_VERSION :
			return mNmdcVersion;

		case USER_STRING_PARAM_MAC_ADDRESS :
			return mDcConn->getMacAddress();

		case USER_STRING_PARAM_IP :
			return getIp();

		case USER_STRING_PARAM_IP_CONN :
			return mDcConn->getIpConn();

		case USER_STRING_PARAM_UID :
			return getUid();

		default :
			throw "Invalid key";

	}
}



void DcUser::setStringParam(unsigned int key, const string & value) {
	setStringParam(key, value.c_str());
}



void DcUser::setStringParam(unsigned int key, const char * value) {
	switch (key) {
		case USER_STRING_PARAM_DATA :
			mData = value;
			break;

		case USER_STRING_PARAM_SUPPORTS :
			value += 10;
			mSupports = value;
			break;

		case USER_STRING_PARAM_NMDC_VERSION :
			mNmdcVersion = value;
			break;

		default :
			throw "Invalid key";

	}
}



bool DcUser::getBoolParam(unsigned int key) const {
	switch (key) {
		case USER_BOOL_PARAM_CAN_KICK :
			return mCanKick;

		case USER_BOOL_PARAM_CAN_FORCE_MOVE :
			return mCanForceMove;

		case USER_BOOL_PARAM_IN_USER_LIST :
			return isInUserList();

		case USER_BOOL_PARAM_IN_OP_LIST :
			return isInOpList();

		case USER_BOOL_PARAM_IN_IP_LIST :
			return isInIpList();

		case USER_BOOL_PARAM_HIDE :
			return isHide();

		default :
			throw "Invalid key";

	}
}



void DcUser::setBoolParam(unsigned int key, bool value) {
	switch (key) {
		case USER_BOOL_PARAM_CAN_KICK :
			mCanKick = value;
			break;

		case USER_BOOL_PARAM_CAN_FORCE_MOVE :
			mCanForceMove = value;
			break;

		case USER_BOOL_PARAM_IN_USER_LIST :
			setInUserList(value);
			break;

		case USER_BOOL_PARAM_IN_OP_LIST :
			setInOpList(value);
			break;

		case USER_BOOL_PARAM_IN_IP_LIST :
			setInIpList(value);
			break;

		case USER_BOOL_PARAM_HIDE :
			setHide(value);
			break;

		default :
			throw "Invalid key";

	}
}



int DcUser::getIntParam(unsigned int key) const {
	switch (key) {
		case USER_INT_PARAM_PROFILE :
			return getProfile();

		case USER_INT_PARAM_PORT :
			return getPort();

		case USER_INT_PARAM_PORT_CONN :
			return getPortConn();

		default :
			throw "Invalid key";

	}
}



void DcUser::setIntParam(unsigned int key, int value) {
	switch (key) {
		case USER_BOOL_PARAM_CAN_KICK :
			setProfile(value);
			break;

		default :
			throw "Invalid key";

	}
}



const string & DcUser::getUid() const {
	return mUid;
}



void DcUser::setUid(const string & uid) {
	mUid = uid;

	// Calc uid hash
	mUidHash = static_cast<unsigned long> (UserList::uidToLowerHash(uid));
}



unsigned long DcUser::getUidHash() const {
	return mUidHash;
}


void DcUser::appendParam(string & dst, const char * prefix, unsigned long key) {
	const string * param = getParam(key);
	if (param != NULL) {
		dst.append(prefix).append(*param);
	}
}



const string & DcUser::getInfo() {
	return myInfo;
}



// Only NMDC
/** Set MyINFO string (for plugins). With cmd & nick check */
bool DcUser::setInfo(const string & newMyInfo) {

	// TODO remove NmdcParser class from this function
	NmdcParser dcParser;
	if (NmdcParser::checkCmd(dcParser, newMyInfo, this) != NMDC_TYPE_MYNIFO) {
		return false;
	}
	return setInfo(&dcParser);
}



// Only NMDC
bool DcUser::setInfo(NmdcParser * parser) {
	if (myInfo != parser->mCommand) {
		myInfo = parser->mCommand;

		const string * oldShare = getParam(USER_PARAM_SHARE);
		if (oldShare != NULL) {
			mDcServer->miTotalShare -= stringToInt64(*oldShare);
		}
		string & newShare = updateParam(USER_PARAM_SHARE, parser->chunkString(CHUNK_MI_SIZE).c_str());
		mDcServer->miTotalShare += stringToInt64(newShare);

		updateParam(USER_PARAM_EMAIL, parser->chunkString(CHUNK_MI_MAIL).c_str());

		size_t size = parser->chunkString(CHUNK_MI_SPEED).size();
		if (size != 0) {
			string & connection = updateParam(USER_PARAM_CONNECTION, parser->chunkString(CHUNK_MI_SPEED).c_str());
			string & magicByte = updateParam(USER_PARAM_BYTE, "");
			magicByte += connection[--size];
			connection.assign(connection, 0, size);
		}

		string & description = updateParam(USER_PARAM_DESC, parser->chunkString(CHUNK_MI_DESC).c_str());
		parseDesc(description);
	}
	return true;
}



const string & DcUser::getInf() const {
	return mInf;
}



void DcUser::setInf(const string & inf) {
	mInf = inf;
}



/** Get IP address of user */
const string & DcUser::getIp() const {
	if (mDcConn != NULL) {
		return mDcConn->getIp();
	}
	return mIp; // TODO remove this param (now for bot only)
}



void DcUser::setIp(const string & ip) {
	if (ip.size()) {
		mIp = ip;
	}
}



/// Get real clients port
int DcUser::getPort() const {
	return mDcConn->getPort();
}



/// Get connection port
int DcUser::getPortConn() const {
	return mDcConn->getPortConn();
}



int DcUser::getProfile() const {
	return mProfile;
}


void DcUser::setProfile(int profile) {
	mProfile = profile;
}



bool DcUser::isCanSend() const {
	return mCanSend;
}



void DcUser::setCanSend(bool canSend) {
	mCanSend = (mInUserList && mDcConn && mDcConn->isOk() && canSend);
}



bool DcUser::isInUserList() const {
	return mInUserList;
}



void DcUser::setInUserList(bool inUserList) {
	mInUserList = inUserList;
}



bool DcUser::isInOpList() const {
	return mInOpList;
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



bool DcUser::isInIpList() const {
	return mInIpList;
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



bool DcUser::isHide() const {
	return mHide;
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



bool DcUser::isPassive() const {
	const string * mode = getParam(USER_PARAM_MODE);
	unsigned int passive = (mode != NULL && mode->size()) ? mode->operator [](0) : 0;
	return passive == 80 || passive == 53 || passive == 83;
}



long DcUser::getConnectTime() const {
	return mTimeEnter.sec();
}



string & DcUser::updateParam(unsigned long key, const char * value) {
	string * param = mParams.find(key);
	if (param != NULL) {
		*param = value;
		return *param;
	}
	string * val = new string(value);
	mParams.add(key, val);
	return *val;
}



void DcUser::parseDesc(string & description) {

	size_t size = description.size();
	if (size) { // optimization
		size_t i = description.find_last_of('<');
		if (i != description.npos && description[--size] == '>') {
			const string * oldTag = getParam(USER_PARAM_TAG);
			string & tag = updateParam(USER_PARAM_TAG, "");
			++i;
			tag.assign(description, i, size - i);
			description.assign(description, 0, --i);
			if (oldTag == NULL || tag.compare(*oldTag) != 0) { // optimization
				parseTag();
			}
		}
	}
}



void DcUser::parseTag() {

	#ifdef _DEBUG
		if (getParam(USER_PARAM_TAG) == NULL) {
			throw "tag is NULL";
		}
	#endif

	const string & tag = *getParam(USER_PARAM_TAG);
	size_t clientPos = tag.find(',');
	size_t tagSize = tag.size();
	if (clientPos == tag.npos) {
		clientPos = tagSize;
	}

	// Get clientName and clientVersion
	string & clientVersion = updateParam(USER_PARAM_CLIENT_VERSION, "");
	string & clientName = updateParam(USER_PARAM_CLIENT_NAME, "");

	size_t v = tag.find("V:");
	if (v != tag.npos) {
		clientVersion.assign(tag, v + 2, clientPos - v - 2);
		clientName.assign(tag, 0, v);
	} else {
		size_t s = tag.find(' ');
		if (s != tag.npos && s < clientPos) {
			++s;
			if (atof(tag.substr(s, clientPos - s).c_str())) {
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
		
	// hubs
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
				string & opHubs = updateParam(USER_PARAM_OP_HUBS, "");
				opHubs.assign(tag, regPos, opPos - regPos);
			}
			string & regHubs = updateParam(USER_PARAM_REG_HUBS, "");
			regHubs.assign(tag, unregPos, regPos - unregPos - 1);
		}
		string & unregHubs = updateParam(USER_PARAM_UNREG_HUBS, "");
		unregHubs.assign(tag, h, unregPos - h - 1);
	}

	// slots and limits
	findParam(tag, "M:", USER_PARAM_MODE);
	findParam(tag, "S:", USER_PARAM_SLOTS);
	findParam(tag, "L:", USER_PARAM_LIMIT);
	findParam(tag, "O:", USER_PARAM_OPEN);
	findParam(tag, "B:", USER_PARAM_BANDWIDTH);
	findParam(tag, "D:", USER_PARAM_DOWNLOAD);
	findParam(tag, "F:", USER_PARAM_FRACTION);
}



void DcUser::findParam(const string & tag, const char * find, unsigned long key) {
	size_t pos = tag.find(find);
	if (pos != tag.npos) {
		pos += 2;
		size_t sepPos = tag.find(',', pos);
		if (sepPos == tag.npos) {
			sepPos = tag.size();
		}
		string & param = updateParam(key, "");
		param.assign(tag, pos, sepPos - pos);
	}
}


}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
