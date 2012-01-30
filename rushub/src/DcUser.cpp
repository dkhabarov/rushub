/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2012 by Setuper
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


DcUser::DcUser(int type, DcConn * dcConn) :
	Obj("DcUser"),
	DcUserBase(type),
	mDcServer(NULL),
	mDcConn(dcConn),
	mTimeEnter(true),
	mUidHash(0),
	mInUserList(false),
	mCanSend(false),
	mInfoChanged(true)
{
	if (dcConn != NULL) {
		mDcServer = dcConn->server();
		getParamForce(USER_PARAM_IP_CONN,     false)->set(&mDcConn->mIpConn,     Param::TYPE_STRING, Param::MODE_NOT_MODIFY);
		getParamForce(USER_PARAM_MAC_ADDRESS, false)->set(&mDcConn->mMacAddress, Param::TYPE_STRING, Param::MODE_NOT_MODIFY);
		getParamForce(USER_PARAM_PORT,        false)->set(&mDcConn->mPort,       Param::TYPE_INT,    Param::MODE_NOT_MODIFY);
		getParamForce(USER_PARAM_PORT_CONN,   false)->set(&mDcConn->mPortConn,   Param::TYPE_INT,    Param::MODE_NOT_MODIFY);
		getParamForce(USER_PARAM_ENTER_TIME,  false)->set(mTimeEnter.sec(),      Param::TYPE_LONG,   Param::MODE_NOT_MODIFY);
	}
	getParamForce(USER_PARAM_IP, false)->set(dcConn != NULL ? &mIp : &mDcConn->mIp, Param::TYPE_STRING, Param::MODE_NOT_MODIFY);
	getParamForce(USER_PARAM_IN_USER_LIST, false)->set(&mInUserList, Param::TYPE_BOOL, Param::MODE_NOT_MODIFY);
	getParamForce(USER_PARAM_PROFILE,      false)->set(-1,           Param::TYPE_INT,  Param::MODE_NOT_CHANGE_TYPE | Param::MODE_NOT_REMOVE);
}



DcUser::~DcUser() {
	HashMap<Param *, string>::iterator it = mParamList.begin(), it_e = mParamList.end();
	while (it != it_e) {
		delete (*it++);
	}
	mDcConn = NULL;
	mDcServer = NULL;
}



void DcUser::send(const string & data, bool addSep /*= false*/, bool flush /*= true*/) {
	if (mDcConn) {
		mDcConn->send(data, addSep, flush);
	}
}



void DcUser::send(const char * data, size_t len, bool addSep /*= false*/, bool flush /*= true*/) {
	if (mDcConn) {
		mDcConn->send(data, len, addSep, flush);
	}
}



void DcUser::sendToChat(const string & data, const string & uid, bool flush /*= true*/) {
	if (mDcConn) {
		mDcConn->dcProtocol()->sendToChat(mDcConn, data, uid, flush);
	}
}



void DcUser::sendToPm(const string & data, const string & uid, const string & from, bool flush /*= true*/) {
	if (mDcConn) {
		mDcConn->dcProtocol()->sendToPm(mDcConn, data, uid, from, flush);
	}
}



void DcUser::disconnect() {
	if (mDcConn) {
		mDcConn->closeNice(9000, CLOSE_REASON_PLUGIN);
	}
}



bool DcUser::hasFeature(int feature) const {
	return mFeatures.find(feature) != mFeatures.end();
}



ParamBase * DcUser::getParam(const char * name) const {
	return mParamList.find(name);
}



ParamBase * DcUser::getParamForce(const char * name) {
	return getParamForce(name, true);
}



Param * DcUser::getParamForce(const char * name, bool setRules) {
	Param * param = mParamList.find(name);
	if (param == NULL) {
		param = new Param(this, name);
		mParamList.add(name, param);
		if (setRules) {
			if (strcmp(name, USER_PARAM_CAN_KICK) == 0) {
				param->set(false, Param::TYPE_BOOL, Param::MODE_NOT_CHANGE_TYPE);
			} else if (strcmp(name, USER_PARAM_CAN_REDIRECT) == 0) {
				param->set(false, Param::TYPE_BOOL, Param::MODE_NOT_CHANGE_TYPE);
			} else if (strcmp(name, USER_PARAM_CAN_HIDE) == 0) {
				param->set(false, Param::TYPE_BOOL, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetHide);
			} else if (strcmp(name, USER_PARAM_IN_IP_LIST) == 0) {
				param->set(false, Param::TYPE_BOOL, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInIpList);
			} else if (strcmp(name, USER_PARAM_IN_OP_LIST) == 0) {
				param->set(false, Param::TYPE_BOOL, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInOpList);
			} else if (strcmp(name, USER_PARAM_SHARE) == 0) {
				__int64 n = 0;
				param->set(n, Param::TYPE_INT64, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetShare);
			} else if (strcmp(name, USER_PARAM_EMAIL) == 0) {
				param->set(string(""), Param::TYPE_STRING, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInfo);
			} else if (strcmp(name, USER_PARAM_DESC) == 0) {
				param->set(string(""), Param::TYPE_STRING, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInfo);
			} else if (strcmp(name, USER_PARAM_BYTE) == 0) {
				param->set(string(""), Param::TYPE_STRING, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInfo);
			} else if (strcmp(name, USER_PARAM_CONNECTION) == 0) {
				param->set(string(""), Param::TYPE_STRING, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInfo);
			} else if (strcmp(name, USER_PARAM_CLIENT_NAME) == 0) {
				param->set(string(""), Param::TYPE_STRING, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInfo);
			} else if (strcmp(name, USER_PARAM_CLIENT_VERSION) == 0) {
				param->set(string(""), Param::TYPE_STRING, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInfo);
			} else if (strcmp(name, USER_PARAM_MODE) == 0) {
				param->set(string(""), Param::TYPE_STRING, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInfo);
			} else if (strcmp(name, USER_PARAM_UNREG_HUBS) == 0) {
				param->set(int(0), Param::TYPE_INT, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInfo);
			} else if (strcmp(name, USER_PARAM_REG_HUBS) == 0) {
				param->set(int(0), Param::TYPE_INT, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInfo);
			} else if (strcmp(name, USER_PARAM_OP_HUBS) == 0) {
				param->set(int(0), Param::TYPE_INT, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInfo);
			} else if (strcmp(name, USER_PARAM_SLOTS) == 0) {
				param->set(int(0), Param::TYPE_INT, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInfo);
			} else if (strcmp(name, USER_PARAM_LIMIT) == 0) {
				param->set(int(0), Param::TYPE_INT, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInfo);
			} else if (strcmp(name, USER_PARAM_OPEN) == 0) {
				param->set(int(0), Param::TYPE_INT, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInfo);
			} else if (strcmp(name, USER_PARAM_BANDWIDTH) == 0) {
				param->set(int(0), Param::TYPE_INT, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInfo);
			} else if (strcmp(name, USER_PARAM_DOWNLOAD) == 0) {
				param->set(int(0), Param::TYPE_INT, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInfo);
			} else if (strcmp(name, USER_PARAM_FRACTION) == 0) {
				param->set(string(""), Param::TYPE_STRING, Param::MODE_NOT_CHANGE_TYPE, &DcUser::onSetInfo);
			}
		}
	}
	return param;
}



bool DcUser::removeParam(const char * name) {
	Param * param = mParamList.find(name);
	if (param == NULL || param->getMode() & (Param::MODE_NOT_MODIFY | Param::MODE_NOT_REMOVE)) {
		return false;
	}

	// TODO remove it!
	if (strcmp(name, USER_PARAM_SHARE) == 0) {
		__int64 n = 0;
		param->setInt64(n); // for remove from total share
	}

	mParamList.remove(name);
	delete param;
	return true;
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



const string & DcUser::getInfo() {
	if (mInfoChanged) {
		if (mDcServer->mDcConfig.mAdcOn) {
			AdcParser::formingInfo(this, mInfo); // ADC
		} else {
			NmdcParser::formingInfo(this, mInfo); // NMDC
		}
		mInfoChanged = false;
	}	
	return mInfo;
}



/// Set Info string
bool DcUser::setInfo(const string & info) {

	// Protocol dependence
	if (mDcServer->mDcConfig.mAdcOn) { // ADC
		AdcParser::parseInfo(this, info);
	} else { // NMDC
		NmdcParser::parseInfo(this, info);
	}
	return true;
}



bool DcUser::parseCommand(const char * cmd) {
	if (mDcConn) {
		return mDcConn->parseCommand(cmd);
	}
	return false;
}



const char * DcUser::getCommand() {
	if (mDcConn) {
		return mDcConn->getCommand();
	}
	return NULL;
}



// NMDC protocol only
const string & DcUser::getNmdcTag() {
	NmdcParser::getTag(this, mNmdcTag);
	return mNmdcTag;
}



// NMDC protocol only
bool DcUser::isPassive() const {
	// TODO: refactoring!
	ParamBase * mode = getParam(USER_PARAM_MODE);
	unsigned int passive = (mode != NULL && mode->getType() == Param::TYPE_STRING && mode->getString().size()) ? mode->getString()[0] : 0;
	return passive == 80 || passive == 53 || passive == 83;
}



/// Get IP address of user
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



int DcUser::getProfile() const {
	return getParam(USER_PARAM_PROFILE)->getInt();
}



bool DcUser::isHide() const {
	return isTrueBoolParam(USER_PARAM_CAN_HIDE);
}



bool DcUser::isCanSend() const {
	return mCanSend;
}



void DcUser::setCanSend(bool canSend) {
	mCanSend = (mInUserList && mDcConn && mDcConn->isOk() && canSend);
}



void DcUser::setInUserList(bool inUserList) {
	mInUserList = inUserList;
}



bool DcUser::isTrueBoolParam(const char * name) const {
	ParamBase * param = getParam(name);
	return param != NULL && param->getType() == Param::TYPE_BOOL && param->getBool();
}



int DcUser::onSetShare(const string & old, const string & now) {
	mDcServer->miTotalShare -= stringToInt64(old);
	mDcServer->miTotalShare += stringToInt64(now);
	onSetInfo(old, now);
	return 0;
}



/** Set/unset user in OpList (for plugins) */
int DcUser::onSetInOpList(const string & old, const string & now) {
	if (now != "0") {
		if (old == "0") {
			// TODO NMDC protocol
			mDcServer->mNmdcProtocol.addToOps(this);
		}
	} else {
		if (old != "0") {
			// TODO NMDC protocol
			mDcServer->mNmdcProtocol.delFromOps(this);
		}
	}
	return 0;
}



int DcUser::onSetInIpList(const string & old, const string & now) {
	if (now != "0") {
		if (old == "0") {
			// TODO NMDC protocol
			mDcServer->mNmdcProtocol.addToIpList(this);
		}
	} else {
		if (old != "0") {
			// TODO NMDC protocol
			mDcServer->mNmdcProtocol.delFromIpList(this);
		}
	}
	return 0;
}



int DcUser::onSetHide(const string & old, const string & now) {
	if (now != "0") {
		if (old == "0") {
			// TODO NMDC protocol
			mDcServer->mNmdcProtocol.addToHide(this);
		}
	} else {
		if (old != "0") {
			// TODO NMDC protocol
			mDcServer->mNmdcProtocol.delFromHide(this);
		}
	}
	return 0;
}



int DcUser::onSetInfo(const string & old, const string & now) {
	if (old != now) {
		mInfoChanged = true;
		mDcServer->mDcUserList.remake(); // !
	}
	return 0;
}


}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
