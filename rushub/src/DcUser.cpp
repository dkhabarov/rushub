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


Param::Param(DcUser * dcUser, const char * name) : 
	mName(name),
	mType(TYPE_NONE),
	mMode(MODE_NONE),
	mDcUser(dcUser),
	mAction(NULL)
{
}



Param::~Param() {
}



const string & Param::getName() const {
	return mName;
}



int Param::getType() const {
	return mType;
}



int Param::getMode() const {
	return mMode;
}



const string & Param::getString() const {
	return mValue;
}



int Param::setString(const string & value) {
	return set(value, TYPE_STRING);
}



int Param::setString(string * value) {
	return set(value, TYPE_STRING);
}



const int & Param::getInt() const {
	return mValue;
}



int Param::setInt(int value) {
	return set(value, TYPE_INT);
}



int Param::setInt(int * value) {
	return set(value, TYPE_INT);
}



const bool & Param::getBool() const {
	return mValue;
}



int Param::setBool(bool value) {
	return set(value, TYPE_BOOL);
}



int Param::setBool(bool * value) {
	return set(value, TYPE_BOOL);
}



const double & Param::getDouble() const {
	return mValue;
}



int Param::setDouble(double value) {
	return set(value, TYPE_DOUBLE);
}



int Param::setDouble(double * value) {
	return set(value, TYPE_DOUBLE);
}



const long & Param::getLong() const {
	return mValue;
}



int Param::setLong(long value) {
	return set(value, TYPE_LONG);
}



int Param::setLong(long * value) {
	return set(value, TYPE_LONG);
}



const __int64 & Param::getInt64() const {
	return mValue;
}



int Param::setInt64(__int64 value) {
	return set(value, TYPE_INT64);
}



int Param::setInt64(__int64 * value) {
	return set(value, TYPE_INT64);
}



const string & Param::toString() {
	return utils::toString(mValue, mBuf);
}



DcUser::DcUser(DcConn * dcConn) :
	Obj("DcUser"),
	DcUserBase(),
	mDcServer(NULL),
	mDcConn(dcConn),
	mTimeEnter(true),
	mUidHash(0),
	mInUserList(false),
	mCanSend(false),
	mInfoChanged(true)
{
	mDcConnBase = dcConn;
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
	mDcConnBase = NULL;
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



void DcUser::disconnect() {
	mDcConn->closeNice(9000, CLOSE_REASON_PLUGIN);
}



bool DcUser::hasFeature(const string & feature) const {
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



const string & DcUser::getNmdcTag() {
	// TODO NMDC protocol
	NmdcParser::getTag(this, mTag);
	return mTag;
}



const string & DcUser::getInfo() {
	if (mDcServer->mDcConfig.mAdcOn) {
		if (mInfoChanged || !mInfo.size()) {
			//AdcParser::getInf(this, mInfo);
			// TODO insert into AdcParser

			mInfo = "BINF "; // TODO !
			mInfo.append(getUid());
			for (set<string>::iterator it = mInfoNames.begin(); it != mInfoNames.end(); ++it) {
				const string & name = (*it);
				ParamBase * param = getParam(name.c_str());
				if (param != NULL) {
					mInfo.append(" ").append(name).append(param->toString());
				}
			}
			mInfoChanged = false;
		}
		return mInfo;
	} else {
		if (mInfoChanged) {
			// TODO NMDC protocol
			NmdcParser::getMyInfo(this, mInfo);
			mInfoChanged = false;
		}
		return mInfo;
	}
}



// Only NMDC
/** Set MyINFO string (for plugins). With cmd & nick check */
bool DcUser::setInfo(const string & info) {
	if (mDcServer->mDcConfig.mAdcOn) {
		//AdcParser::parseInf(this, inf);
		// TODO insert into AdcParser
		size_t s = info.find(' ', 9);
		if (s != info.npos) {
			size_t e;
			bool last = true;
			mInfoChanged = true; // !
			mDcServer->mAdcUserList.remake(); // !
			while ((e = info.find(' ', ++s)) != info.npos || last) {
				if (e == info.npos) {
					e = info.size();
					last = false;
				}
				if (s + 2 <= e) { // max 2 (for name)
					string name;
					name.assign(info, s, 2);
					s += 2;
					if (e != s) {
						string value;
						value.assign(info, s, e - s);
						mInfoNames.insert(name); // TODO refactoring
						if ((name == "I4" && value == "0.0.0.0") || (name == "I6" && value == "::")) {
							getParamForce(name.c_str())->setString(getIp());
						} else {
							if (name == "SU") {
								// TODO check syntax
								mFeatures.clear();
								size_t i, j = 0;
								while ((i = value.find(',', j)) != value.npos) {
									mFeatures.insert(value.substr(j, i - j));
									j = i + 1;
								}
								mFeatures.insert(value.substr(j));
							}
							getParamForce(name.c_str())->setString(value);
						}
					} else {
						mInfoNames.erase(name); // TODO refactoring
						removeParam(name.c_str());
					}
				}
				s = e;
			}
		}
		return true;
	} else {
		NmdcParser dcParser;
		if (NmdcParser::checkCmd(dcParser, info, this) != NMDC_TYPE_MYNIFO) {
			return false;
		}
		return setInfo(&dcParser);
	}
}



// Only NMDC
bool DcUser::setInfo(NmdcParser * parser) {
	if (mInfo != parser->mCommand) {
		mInfo = parser->mCommand;

		// Share
		getParamForce(USER_PARAM_SHARE)->setInt64(stringToInt64(parser->chunkString(CHUNK_MI_SIZE)));

		// Email
		getParamForce(USER_PARAM_EMAIL)->setString(parser->chunkString(CHUNK_MI_MAIL));


		string speed = parser->chunkString(CHUNK_MI_SPEED);
		string magicByte;
		size_t size = speed.size();
		if (size != 0) {
			magicByte = speed[--size];
			speed.assign(speed, 0, size);
		}

		getParamForce(USER_PARAM_BYTE)->setString(magicByte);
		getParamForce(USER_PARAM_CONNECTION)->setString(speed);

		// TODO NMDC protocol
		NmdcParser::parseDesc(this, parser->chunkString(CHUNK_MI_DESC));
	}
	return true;
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



bool DcUser::isPassive() const {
	// TODO NMDC protocol only
	ParamBase * mode = getParam(USER_PARAM_MODE);
	unsigned int passive = (mode != NULL && mode->getType() == Param::TYPE_STRING && mode->getString().size()) ? mode->getString()[0] : 0;
	return passive == 80 || passive == 53 || passive == 83;
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
	}
	return 0;
}


}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
