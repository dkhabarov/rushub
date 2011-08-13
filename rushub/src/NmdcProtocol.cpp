/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
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

#include "NmdcProtocol.h"
#include "DcServer.h" // for mDcServer
#include "DcConn.h" // for DcConn

#include <string.h>

namespace dcserver {

namespace protocol {


NmdcProtocol::NmdcProtocol() :
	mDcServer(NULL)
{
	setClassName("NmdcProtocol");

	events[NMDC_TYPE_MSEARCH] = &NmdcProtocol::eventSearch;
	events[NMDC_TYPE_MSEARCH_PAS] = &NmdcProtocol::eventSearch;
	events[NMDC_TYPE_SEARCH_PAS] = &NmdcProtocol::eventSearch;
	events[NMDC_TYPE_SEARCH] = &NmdcProtocol::eventSearch;
	events[NMDC_TYPE_SR] = &NmdcProtocol::eventSr;
	events[NMDC_TYPE_SR_UDP] = &NmdcProtocol::eventSr;
	events[NMDC_TYPE_MYNIFO] = &NmdcProtocol::eventMyInfo;
	events[NMDC_TYPE_SUPPORTS] = &NmdcProtocol::eventSupports;
	events[NMDC_TYPE_KEY] = &NmdcProtocol::eventKey;
	events[NMDC_TYPE_VALIDATENICK] = &NmdcProtocol::eventValidateNick;
	events[NMDC_TYPE_VERSION] = &NmdcProtocol::eventVersion;
	events[NMDC_TYPE_GETNICKLIST] = &NmdcProtocol::eventGetNickList;
	events[NMDC_TYPE_CHAT] = &NmdcProtocol::eventChat;
	events[NMDC_TYPE_TO] = &NmdcProtocol::eventTo;
	events[NMDC_TYPE_QUIT] = &NmdcProtocol::eventQuit;
	events[NMDC_TYPE_MYPASS] = &NmdcProtocol::eventMyPass;
	events[NMDC_TYPE_CONNECTTOME] = &NmdcProtocol::eventConnectToMe;
	events[NMDC_TYPE_RCONNECTTOME] = &NmdcProtocol::eventRevConnectToMe;
	events[NMDC_TYPE_MCONNECTTOME] = &NmdcProtocol::eventMultiConnectToMe;
	events[NMDC_TYPE_KICK] = &NmdcProtocol::eventKick;
	events[NMDC_TYPE_OPFORCEMOVE] = &NmdcProtocol::eventOpForceMove;
	events[NMDC_TYPE_GETINFO] = &NmdcProtocol::eventGetInfo;
	events[NMDC_TYPE_MCTO] = &NmdcProtocol::eventMcTo;
	events[NMDC_TYPE_USERIP] = &NmdcProtocol::eventUserIp;
	events[NMDC_TYPE_PING] = &NmdcProtocol::eventPing;
	events[NMDC_TYPE_UNKNOWN] = &NmdcProtocol::eventUnknown;

}



NmdcProtocol::~NmdcProtocol() {
}



const char * NmdcProtocol::getSeparator() const {
	return NMDC_SEPARATOR;
}



size_t NmdcProtocol::getSeparatorLen() const {
	return NMDC_SEPARATOR_LEN;
}



unsigned long NmdcProtocol::getMaxCommandLength() const {
	return mDcServer->mDcConfig.mMaxNmdcCommandLength;
}



int NmdcProtocol::onNewConn(Conn * conn) {

	DcConn * dcConn = static_cast<DcConn *> (conn);
	string msg;
	dcConn->send(
		appendHubName(
			appendLock(msg),
			mDcServer->mDcConfig.mHubName,
			mDcServer->mDcConfig.mTopic
		),
		false,
		false
	);

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnUserConnected.callAll(dcConn->mDcUser)) {
			dcConn->flush();
		} else
	#endif
	{
		static __int64 iShareVal = -1;
		static int iUsersVal = -1;
		static long iTimeVal = -1;
		static string sTimeCache, sShareCache, sCache;
		bool useCache = true;
		Time Uptime(mDcServer->mTime);
		Uptime -= mDcServer->mStartTime;
		long min = Uptime.sec() / 60;
		if (iTimeVal != min) {
			iTimeVal = min;
			useCache = false;
			stringstream oss;
			int w, d, h, m;
			Uptime.asTimeVals(w, d, h, m);
			if (w) {
				oss << w << " " << mDcServer->mDcLang.mTimes[0] << " ";
			}
			if (d) {
				oss << d << " " << mDcServer->mDcLang.mTimes[1] << " ";
			}
			if (h) {
				oss << h << " " << mDcServer->mDcLang.mTimes[2] << " ";
			}
			oss << m << " " << mDcServer->mDcLang.mTimes[3];
			sTimeCache = oss.str();
		}
		if (iShareVal != mDcServer->miTotalShare) {
			iShareVal = mDcServer->miTotalShare;
			useCache = false;
			getNormalShare(iShareVal, sShareCache);
		}
		if (iUsersVal != mDcServer->miTotalUserCount) {
			iUsersVal = mDcServer->miTotalUserCount;
			useCache = false;
		}
		if (!useCache) {
			stringReplace(mDcServer->mDcLang.mFirstMsg, "HUB", sCache, INTERNALNAME " " INTERNALVERSION);
			stringReplace(sCache, "uptime", sCache, sTimeCache);
			stringReplace(sCache, "users", sCache, iUsersVal);
			stringReplace(sCache, "share", sCache, sShareCache);
		}
		mDcServer->sendToUser(dcConn->mDcUser, sCache.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
	}
	dcConn->setTimeOut(HUB_TIME_OUT_LOGIN, mDcServer->mDcConfig.mTimeout[HUB_TIME_OUT_LOGIN], mDcServer->mTime); /** Timeout for enter */
	dcConn->setTimeOut(HUB_TIME_OUT_KEY, mDcServer->mDcConfig.mTimeout[HUB_TIME_OUT_KEY], mDcServer->mTime);
	dcConn->flush();
	return 0;
}



Conn * NmdcProtocol::getConnForUdpData(Conn * conn, Parser * parser) {

	// only SR command
	if (parser->mType == NMDC_TYPE_SR) {
		parser->mType = NMDC_TYPE_SR_UDP; // Set type for parse

		NmdcParser * dcParser = static_cast<NmdcParser *> (parser);
		if (!dcParser->splitChunks()) {

			// NMDC
			DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByUid(dcParser->chunkString(CHUNK_SR_FROM)));
			if (dcUser && dcUser->mDcConn && conn->getIpUdp() == dcUser->getIp()) {
				return dcUser->mDcConn;
			} else {
				if (conn->log(3)) {
					conn->logStream() << "Not found user for UDP data" << endl;
				}
			}
		} else {
			if (conn->log(3)) {
				conn->logStream() << "Bad UDP cmd syntax" << endl;
			}
		}
	} else {
		if (conn->log(4)) {
			conn->logStream() << "Unknown UDP data" << endl;
		}
	}
	return NULL;
}



void NmdcProtocol::onFlush(Conn * conn) {
	DcConn * dcConn = static_cast<DcConn *> (conn);
	if (dcConn->mNickListInProgress) {
		dcConn->setLoginStatusFlag(LOGIN_STATUS_NICKLST);
		dcConn->mNickListInProgress = false;
		if (!dcConn->isOk() || !dcConn->isWritable()) {
			if (dcConn->log(2)) {
				dcConn->logStream() << "Connection closed during nicklist" << endl;
			}
		} else {
			if (dcConn->log(3)) {
				dcConn->logStream() << "Enter after nicklist" << endl;
			}
			mDcServer->doUserEnter(dcConn);
		}
	}
}



int NmdcProtocol::doCommand(Parser * parser, Conn * conn) {

	NmdcParser * dcParser = static_cast<NmdcParser *> (parser);
	DcConn * dcConn = static_cast<DcConn *> (conn);

	if (checkCommand(dcParser, dcConn) < 0) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnAny.callAll(dcConn->mDcUser, dcParser->mType)) {
			return 1;
		}
	#endif

	#ifdef _DEBUG
		if (dcParser->mType == TYPE_UNPARSED) {
			throw "Unparsed command";
		}
	#endif

	if (dcConn->log(5)) {
		dcConn->logStream() << "[S]Stage " << dcParser->mType << endl;
	}

	(this->*(this->events[dcParser->mType])) (dcParser, dcConn);

	if (dcConn->log(5)) {
		dcConn->logStream() << "[E]Stage " << dcParser->mType << endl;
	}
	return 0;
}



int NmdcProtocol::eventSupports(NmdcParser * dcparser, DcConn * dcConn) {

	string feature;
	size_t posNext, posPrev = 10;
	dcConn->mFeatures = 0;
	while((posNext = dcparser->mCommand.find(' ', posPrev)) != feature.npos) {
		feature.assign(dcparser->mCommand, posPrev, posNext - posPrev);
		posPrev = posNext + 1;
		if (feature == "UserCommand") {
			dcConn->mFeatures |= SUPPORT_FEATUER_USERCOMMAND;
		} else if (feature == "NoGetINFO") {
			dcConn->mFeatures |= SUPPORT_FEATUER_NOGETINFO;
		} else if (feature == "NoHello") {
			dcConn->mFeatures |= SUPPORT_FEATURE_NOHELLO;
		} else if (feature == "UserIP2") {
			dcConn->mFeatures |= SUPPORT_FEATUER_USERIP2;
		} else if (feature == "UserIP") {
			dcConn->mFeatures |= SUPPORT_FEATUER_USERIP;
		} else if (feature == "TTHSearch") {
			dcConn->mFeatures |= SUPPORT_FEATUER_TTHSEARCH;
		} else if (feature == "QuickList") {
			dcConn->mFeatures |= SUPPORT_FEATUER_QUICKLIST;
		}
	}
	dcConn->mDcUser->mSupports.assign(dcparser->mCommand, 10, dcparser->mCommand.size() - 10);

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnSupports.callAll(dcConn->mDcUser)) {
			return -3;
		}
	#endif

	const char * support = "$Supports UserCommand NoGetINFO NoHello UserIP UserIP2 MCTo";
	dcConn->send(support, strlen(support), true, false);

	return 0;
}



int NmdcProtocol::eventKey(NmdcParser *, DcConn * dcConn) {

	if (badFlag(dcConn, "Key", LOGIN_STATUS_KEY)) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnKey.callAll(dcConn->mDcUser)) {
			//string lock, key;
			//appendLock(lock);
			//Lock2Key(lock.assign(lock, 1, lock.size() - 1), key);
		}
	#endif

	dcConn->setLoginStatusFlag(LOGIN_STATUS_KEY); /** User has sent key */
	dcConn->clearTimeOut(HUB_TIME_OUT_KEY);
	dcConn->setTimeOut(HUB_TIME_OUT_VALNICK, mDcServer->mDcConfig.mTimeout[HUB_TIME_OUT_VALNICK], mDcServer->mTime);
	return 0;
}



int NmdcProtocol::eventValidateNick(NmdcParser * dcparser, DcConn * dcConn) {

	if (badFlag(dcConn, "ValidateNick", LOGIN_STATUS_VALNICK)) {
		return -1;
	}

	const string & nick = dcparser->chunkString(CHUNK_1_PARAM);
	size_t iNickLen = nick.size();

	/** Additional checking the nick length */
	if (iNickLen > 0xFF) {
		if (dcConn->log(1)) {
			dcConn->logStream() << "Attempt to attack by long nick" << endl;
		}
		dcConn->closeNow(CLOSE_REASON_NICK_LONG);
		return -1;
	}

	if (dcConn->log(3)) {
		dcConn->logStream() << "User " << nick << " to validate nick" << endl;
	}

	// Set nick
	dcConn->mDcUser->setUid(nick);
	

	/** Checking validate user */
	if (!validateUser(dcConn, nick)) {
		dcConn->closeNice(9000, CLOSE_REASON_USER_INVALID);
		return -2;
	}

	/** Global user's limit */
	if (mDcServer->mDcConfig.mUsersLimit >= 0 && mDcServer->miTotalUserCount >= mDcServer->mDcConfig.mUsersLimit) {
		if (dcConn->log(3)) {
			dcConn->logStream() << "User " << nick << " was disconnected (user's limit: " << mDcServer->mDcConfig.mUsersLimit << ")" << endl;
		}
		mDcServer->sendToUser(dcConn->mDcUser, mDcServer->mDcLang.mUsersLimit.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_USERS_LIMIT);
		return -3;
	}

	dcConn->setLoginStatusFlag(LOGIN_STATUS_VALNICK | LOGIN_STATUS_NICKLST); /** We Install NICKLST because user can not call user list */
	dcConn->clearTimeOut(HUB_TIME_OUT_VALNICK);

	string msg;

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnValidateNick.callAll(dcConn->mDcUser)) {
			dcConn->send(appendGetPass(msg)); /** We are sending the query for reception of the password */
			dcConn->setTimeOut(HUB_TIME_OUT_PASS, mDcServer->mDcConfig.mTimeout[HUB_TIME_OUT_PASS], mDcServer->mTime);
			return -4;
		}
	#endif

	if (!iNickLen || !checkNickLength(dcConn, iNickLen)) {
		dcConn->closeNice(9000, CLOSE_REASON_NICK_LEN);
	}
	dcConn->setTimeOut(HUB_TIME_OUT_MYINFO, mDcServer->mDcConfig.mTimeout[HUB_TIME_OUT_MYINFO], mDcServer->mTime);
	dcConn->setLoginStatusFlag(LOGIN_STATUS_PASSWD); /** Does not need password */

	dcConn->send(appendHello(msg, dcConn->mDcUser->getUid())); /** Protection from change the command */
	return 0;
}



int NmdcProtocol::eventMyPass(NmdcParser *, DcConn * dcConn) {

	if (dcConn->mDcUser->getUid().empty()) { /* Check of existence of the user for current connection */
		if (dcConn->log(2)) {
			dcConn->logStream() << "Mypass before validatenick" << endl;
		}
		mDcServer->sendToUser(dcConn->mDcUser, mDcServer->mDcLang.mBadLoginSequence.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_CMD_PASSWORD_ERR);
		return -1;
	}
	if (badFlag(dcConn, "MyPass", LOGIN_STATUS_PASSWD)) {
		return -1;
	}

	// Checking the accepted password, otherwise send $BadPass|
	// or $Hello Nick|$LogedIn Nick|
	int bOp = 0;
	#ifndef WITHOUT_PLUGINS
		bOp = (mDcServer->mCalls.mOnMyPass.callAll(dcConn->mDcUser));
	#endif

	string msg;
	dcConn->setLoginStatusFlag(LOGIN_STATUS_PASSWD); /** Password is accepted */
	appendHello(msg, dcConn->mDcUser->getUid());
	if (bOp) { /** If entered operator, that sends command LoggedIn ($LogedIn !) */
		msg.append("$LogedIn ", 9);
		msg.append(dcConn->mDcUser->getUid());
		msg.append(getSeparator());
	}
	dcConn->send(msg);
	dcConn->clearTimeOut(HUB_TIME_OUT_PASS);
	dcConn->setTimeOut(HUB_TIME_OUT_MYINFO, mDcServer->mDcConfig.mTimeout[HUB_TIME_OUT_MYINFO], mDcServer->mTime);
	return 0;
}



int NmdcProtocol::eventVersion(NmdcParser * dcparser, DcConn * dcConn) {

	if (badFlag(dcConn, "Version", LOGIN_STATUS_VERSION)) {
		return -1;
	}

	string & sVersion = dcparser->chunkString(CHUNK_1_PARAM);
	if (dcConn->log(3)) {
		dcConn->logStream() << "Version:" << sVersion << endl;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnVersion.callAll(dcConn->mDcUser) && sVersion != "1,0091") {
			dcConn->closeNice(9000, CLOSE_REASON_CMD_VERSION); /** Checking of the version */
		}
	#endif

	dcConn->setLoginStatusFlag(LOGIN_STATUS_VERSION); /** Version was checked */
	dcConn->mDcUser->mVersion = sVersion;
	return 0;
}



int NmdcProtocol::eventGetNickList(NmdcParser *, DcConn * dcConn) {

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnGetNickList.callAll(dcConn->mDcUser)) {
			return 3;
		}
	#endif

	if (!dcConn->getLoginStatusFlag(LOGIN_STATUS_MYINFO) && mDcServer->mDcConfig.mNicklistOnLogin) {
		if (mDcServer->mDcConfig.mDelayedLogin) {
			int LSFlag = dcConn->getLoginStatusFlag(LOGIN_STATUS_LOGIN_DONE);
			if (LSFlag & LOGIN_STATUS_NICKLST) {
				LSFlag -= LOGIN_STATUS_NICKLST;
			}
			dcConn->resetLoginStatusFlag(LSFlag);
		}
		dcConn->mSendNickList = true;
		return 0;
	}
	return sendNickList(dcConn);
}



int NmdcProtocol::eventMyInfo(NmdcParser * dcparser, DcConn * dcConn) {

	const string & nick = dcparser->chunkString(CHUNK_MI_NICK);

	/** Check existence user, otherwise check support QuickList */
	if (nick.empty()) {
		//if (QuickList)
		//	dcConn->mDcUser->setUid(nick);
		//} else
		{
			if (dcConn->log(2)) {
				dcConn->logStream() << "Myinfo without user: " << dcparser->mCommand << endl;
			}
			mDcServer->sendToUser(dcConn->mDcUser, mDcServer->mDcLang.mBadLoginSequence.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
			dcConn->closeNice(9000, CLOSE_REASON_CMD_MYINFO_WITHOUT_USER);
			return -2;
		}
	} else if (nick != dcConn->mDcUser->getUid()) { /** Проверка ника */
		if (dcConn->log(2)) {
			dcConn->logStream() << "Bad nick in MyINFO, closing" << endl;
		}
		mDcServer->sendToUser(dcConn->mDcUser, mDcServer->mDcLang.mBadMyinfoNick.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_MYINFO);
		return -1;
	}

	string sOldMyINFO = dcConn->mDcUser->getMyInfo();
	dcConn->mDcUser->setMyInfo(dcparser);

	int iMode = 0;
	#ifndef WITHOUT_PLUGINS
		iMode = mDcServer->mCalls.mOnMyINFO.callAll(dcConn->mDcUser);
	#endif

	if (iMode != 1 && dcConn->mDcUser->getInUserList()) {
		if (sOldMyINFO != dcConn->mDcUser->getMyInfo()) {
			if (dcConn->mDcUser->getHide()) {
				dcConn->send(dcConn->mDcUser->getMyInfo(), true); // Send to self only
			} else {
				sendMode(dcConn, dcConn->mDcUser->getMyInfo(), iMode, mDcServer->mDcUserList, true); // Use cache for send to all
			}
		}
	} else if (!dcConn->mDcUser->getInUserList()) {
		dcConn->setLoginStatusFlag(LOGIN_STATUS_MYINFO);

		unsigned iWantedMask = LOGIN_STATUS_LOGIN_DONE;
		if (mDcServer->mDcConfig.mDelayedLogin && dcConn->mSendNickList) {
			iWantedMask = LOGIN_STATUS_LOGIN_DONE - LOGIN_STATUS_NICKLST;
		}
		if (iWantedMask != dcConn->getLoginStatusFlag(iWantedMask)) {
			if (dcConn->log(2)) {
				dcConn->logStream() << "Invalid sequence of the sent commands (" 
					<< dcConn->getLoginStatusFlag(iWantedMask) << "), wanted: " 
					<< iWantedMask << endl;
			}
			dcConn->closeNow(CLOSE_REASON_CMD_SEQUENCE);
			return -1;
		}

		if (!mDcServer->beforeUserEnter(dcConn)) {
			return -1;
		}
		dcConn->clearTimeOut(HUB_TIME_OUT_MYINFO);
	}
	return 0;
}



int NmdcProtocol::eventChat(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	/** Check chat nick */
	if ((dcparser->chunkString(CHUNK_CH_NICK) != dcConn->mDcUser->getUid()) ) {
		if (dcConn->log(2)) {
			dcConn->logStream() << "Bad nick in chat, closing" << endl;
		}
		string msg;
		stringReplace(mDcServer->mDcLang.mBadChatNick, "nick", msg, dcparser->chunkString(CHUNK_CH_NICK));
		stringReplace(msg, "real_nick", msg, dcConn->mDcUser->getUid());
		mDcServer->sendToUser(dcConn->mDcUser, msg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_CHAT);
		return -2;
	}
	int mode = 0;
	#ifndef WITHOUT_PLUGINS
		mode = mDcServer->mCalls.mOnChat.callAll(dcConn->mDcUser);
	#endif

	//Hash<unsigned long> hash;
	//unsigned long key = hash(dcparser->mCommand);
	//cout << key << endl;

	/** Sending message */
	sendMode(dcConn, dcparser->mCommand, mode, mDcServer->mChatList, false); // Don't use cache for send to all
	return 0;
}



int NmdcProtocol::eventTo(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	const string & nick = dcparser->chunkString(CHUNK_PM_TO);

	/** Checking the coincidence nicks in command */
	if (dcparser->chunkString(CHUNK_PM_FROM) != dcConn->mDcUser->getUid() || dcparser->chunkString(CHUNK_PM_NICK) != dcConn->mDcUser->getUid()) {
		if (dcConn->log(2)) {
			dcConn->logStream() << "Bad nick in PM, closing" <<endl;
		}
		dcConn->closeNow(CLOSE_REASON_NICK_PM);
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnTo.callAll(dcConn->mDcUser)) {
			return 0;
		}
	#endif

	// Search user (PROTOCOL NMDC)
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByUid(nick));
	if (!dcUser) {
		return -2;
	}

	dcUser->send(dcparser->mCommand, true);
	return 0;
}



int NmdcProtocol::eventMcTo(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	const string & nick = dcparser->chunkString(CHUNK_MC_TO);

	/** Checking the coincidence nicks in command */
	if (dcparser->chunkString(CHUNK_MC_FROM) != dcConn->mDcUser->getUid()) {
		if (dcConn->log(2)) {
			dcConn->logStream() << "Bad nick in MCTo, closing" <<endl;
		}
		dcConn->closeNow(CLOSE_REASON_NICK_MCTO);
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnMCTo.callAll(dcConn->mDcUser)) {
			return 0;
		}
	#endif

	// Search user (PROTOCOL NMDC)
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByUid(nick));
	if (!dcUser) {
		return -2;
	}

	string msg;
	dcUser->send(appendChat(msg, dcConn->mDcUser->getUid(), dcparser->chunkString(CHUNK_MC_MSG)));
	if (dcConn->mDcUser->getUid() != nick) {
		dcConn->send(msg);
	}
	return 0;
}



int NmdcProtocol::eventUserIp(NmdcParser * dcParser, DcConn * dcConn) {

	if (!dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	if (!(dcConn->mFeatures & (SUPPORT_FEATUER_USERIP | SUPPORT_FEATUER_USERIP2))) {
		return -2;
	}

	const string & param = dcParser->chunkString(CHUNK_1_PARAM);
	string nick, result("$UserIP ");

	size_t pos = param.find("$$");
	size_t cur = 0;
	while (pos != param.npos) {
		nick.assign(param, cur, pos - cur);
		if (nick.size()) {
			// PROTOCOL NMDC
			UserBase * userBase = mDcServer->mDcUserList.getUserBaseByUid(nick);
			if (userBase != NULL) {
				result.append(nick).append(" ", 1).append(userBase->ip()).append("$$", 2);
			}
		}
		cur = pos + 2;
		pos = param.find("$$", pos + 2);
	}

	// last param
	nick.assign(param, cur, param.size() - cur);
	if (nick.size()) {
		// PROTOCOL NMDC
		UserBase * userBase = mDcServer->mDcUserList.getUserBaseByUid(nick);
		if (userBase != NULL) {
			result.append(nick).append(" ", 1).append(userBase->ip());
		}
	}


	dcConn->send(result, true);
	return 0;
}



/**
	NMDC_TYPE_SEARCH
	NMDC_TYPE_SEARCH_PAS
	NMDC_TYPE_MSEARCH
	NMDC_TYPE_MSEARCH_PAS
*/
int NmdcProtocol::eventSearch(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	// TODO: Check overloading of the system

	int iMode = 0;
	#ifndef WITHOUT_PLUGINS
		iMode = mDcServer->mCalls.mOnSearch.callAll(dcConn->mDcUser);
	#endif

	/** Sending cmd */
	string msg;

	switch (dcparser->mType) {

		case NMDC_TYPE_SEARCH :
			if (mDcServer->mDcConfig.mCheckSearchIp && dcConn->getIp() != dcparser->chunkString(CHUNK_AS_IP)) {
				if (dcConn->log(2)) {
					dcConn->logStream() << "Bad ip in active search, closing" << endl;
				}
				stringReplace(mDcServer->mDcLang.mBadSearchIp, "ip", msg, dcparser->chunkString(CHUNK_AS_IP));
				stringReplace(msg, "real_ip", msg, dcConn->getIp());
				mDcServer->sendToUser(dcConn->mDcUser, msg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
				dcConn->closeNice(9000, CLOSE_REASON_NICK_SEARCH);
				return -1;
			}
			sendMode(dcConn, dcparser->mCommand, iMode, mDcServer->mDcUserList, true); // Use cache for send to all
			break;

		case NMDC_TYPE_SEARCH_PAS :
			dcConn->emptySrCounter(); /** Zeroizing result counter of the passive search */
			sendMode(dcConn, dcparser->mCommand, iMode, mDcServer->mActiveList, true); // Use cache for send to all
			break;

		case NMDC_TYPE_MSEARCH :
			if (mDcServer->mDcConfig.mCheckSearchIp && (dcConn->getIp() != dcparser->chunkString(CHUNK_AS_IP))) {
				if (dcConn->log(2)) {
					dcConn->logStream() << "Bad ip in active search, closing" << endl;
				}
				stringReplace(mDcServer->mDcLang.mBadSearchIp, "ip", msg, dcparser->chunkString(CHUNK_AS_IP));
				stringReplace(msg, "real_ip", msg, dcConn->getIp());
				mDcServer->sendToUser(dcConn->mDcUser, msg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
				dcConn->closeNice(9000, CLOSE_REASON_NICK_SEARCH);
				return -1;
			}
			msg = "$Search ";
			msg.append(dcparser->chunkString(CHUNK_AS_ADDR));
			msg.append(" ", 1);
			msg.append(dcparser->chunkString(CHUNK_AS_QUERY));
			sendMode(dcConn, msg, iMode, mDcServer->mDcUserList, true); // Use cache for send to all
			break;

		case NMDC_TYPE_MSEARCH_PAS :
			dcConn->emptySrCounter(); /** Zeroizing result counter of the passive search */
			sendMode(dcConn, dcparser->mCommand, iMode, mDcServer->mActiveList, true); // Use cache for send to all
			break;

		default :
			return -4;

	}
	return 0;
}



int NmdcProtocol::eventSr(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	// Check same nick in cmd (PROTOCOL NMDC)
	if (mDcServer->mDcConfig.mCheckSrNick && (dcConn->mDcUser->getUid() != dcparser->chunkString(CHUNK_SR_FROM))) {
		if (dcConn->log(2)) {
			dcConn->logStream() << "Bad nick in search response, closing" << endl;
		}
		string msg;
		stringReplace(mDcServer->mDcLang.mBadSrNick, "nick", msg, dcparser->chunkString(CHUNK_SR_FROM));
		stringReplace(msg, "real_nick", msg, dcConn->mDcUser->getUid());
		mDcServer->sendToUser(dcConn->mDcUser, msg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_SR);
		return -1;
	}

	DcUser * dcUser = NULL;
	const string & nick = dcparser->chunkString(CHUNK_SR_TO);
	if (nick != "") {
		dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByUid(nick));

		/** Is user exist? */
		if (!dcUser || !dcUser->mDcConn) {
			return -2;
		}
	}

	/** != 0 - error */
	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnSR.callAll(dcConn->mDcUser)) {
			return -3;
		}
	#endif

	/** Sending cmd */
	if (dcUser && (!mDcServer->mDcConfig.mMaxPassiveRes ||
		(dcUser->mDcConn->getSrCounter() <= unsigned(mDcServer->mDcConfig.mMaxPassiveRes))
	)) {
		dcUser->mDcConn->increaseSrCounter();
		string str(dcparser->mCommand, 0, dcparser->mChunks[CHUNK_SR_TO].first - 1); /** Remove nick on the end of cmd */
		dcUser->mDcConn->send(str, true, false);
	}
	return 0;
}



int NmdcProtocol::eventConnectToMe(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	if (mDcServer->mDcConfig.mCheckCtmIp && dcConn->getIp() != dcparser->chunkString(CHUNK_CM_IP)) {
		string msg;
		stringReplace(mDcServer->mDcLang.mBadCtmIp, "ip", msg, dcparser->chunkString(CHUNK_CM_IP));
		stringReplace(msg, "real_ip", msg, dcConn->getIp());
		mDcServer->sendToUser(dcConn->mDcUser, msg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_CTM);
		return -1;
	}

	// PROTOCOL NMDC
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByUid(dcparser->chunkString(CHUNK_CM_NICK)));
	if (!dcUser) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnConnectToMe.callAll(dcConn->mDcUser)) {
			return -2;
		}
	#endif

	dcUser->send(dcparser->mCommand, true);
	return 0;
}



int NmdcProtocol::eventRevConnectToMe(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	// Checking the nick (PROTOCOL NMDC)
	if (mDcServer->mDcConfig.mCheckRctmNick && (dcparser->chunkString(CHUNK_RC_NICK) != dcConn->mDcUser->getUid())) {
		string msg;
		stringReplace(mDcServer->mDcLang.mBadRevConNick, "nick", msg, dcparser->chunkString(CHUNK_RC_NICK));
		stringReplace(msg, "real_nick", msg, dcConn->mDcUser->getUid());
		mDcServer->sendToUser(dcConn->mDcUser, msg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_RCTM);
		return -1;
	}

	// Searching the user (PROTOCOL NMDC)
	DcUser * other = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByUid(dcparser->chunkString(CHUNK_RC_OTHER)));
	if (!other) {
		return -2;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnRevConnectToMe.callAll(dcConn->mDcUser)) {
			return -2;
		}
	#endif

	other->send(dcparser->mCommand, true);
	return 0;
}



int NmdcProtocol::eventMultiConnectToMe(NmdcParser *, DcConn * dcConn) {

	if (!dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	return 0;
}



int NmdcProtocol::eventKick(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnKick.callAll(dcConn->mDcUser)) {
			return -1;
		}
	#endif

	if (!dcConn->mDcUser->getKick()) {
		return -2;
	}

	// PROTOCOL NMDC
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByUid(dcparser->chunkString(CHUNK_1_PARAM)));

	// Is user exist?
	if (!dcUser || !dcUser->mDcConn) {
		return -3;
	}

	dcUser->mDcConn->closeNice(9000, CLOSE_REASON_CMD_KICK);
	return 0;
}



int NmdcProtocol::eventOpForceMove(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnOpForceMove.callAll(dcConn->mDcUser)) {
			return -1;
		}
	#endif

	if (!dcConn->mDcUser->getForceMove()) {
		return -2;
	}

	// PROTOCOL NMDC
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByUid(dcparser->chunkString(CHUNK_FM_NICK)));

	// Is user exist?
	if (!dcUser || !dcUser->mDcConn || !dcparser->chunkString(CHUNK_FM_DEST).size()) {
		return -3;
	}

	mDcServer->forceMove(dcUser, dcparser->chunkString(CHUNK_FM_DEST).c_str(), dcparser->chunkString(CHUNK_FM_REASON).c_str());
	return 0;
}



int NmdcProtocol::eventGetInfo(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	// PROTOCOL NMDC
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByUid(dcparser->chunkString(CHUNK_GI_OTHER)));
	if (!dcUser) {
		return -2;
	}

	if (dcConn->mDcUser->mTimeEnter < dcUser->mTimeEnter && Time().get() < (dcUser->mTimeEnter + 60000)) {
		return 0;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnGetINFO.callAll(dcConn->mDcUser)) {
			return -2;
		}
	#endif

	//if(!(dcConn->mFeatures & SUPPORT_FEATUER_NOGETINFO)){
	if (!dcUser->getHide()) {
		dcConn->send(dcUser->getMyInfo(), true, false);
	}
	return 0;
}



int NmdcProtocol::eventQuit(NmdcParser *, DcConn * dcConn) {
	dcConn->closeNice(9000, CLOSE_REASON_CMD_QUIT);
	return 0;
}



int NmdcProtocol::eventPing(NmdcParser *, DcConn *) {
	return 0;
}



int NmdcProtocol::eventUnknown(NmdcParser *, DcConn * dcConn) {

	#ifndef WITHOUT_PLUGINS
	if (!mDcServer->mCalls.mOnUnknown.callAll(dcConn->mDcUser)) {
		dcConn->closeNice(9000, CLOSE_REASON_CMD_UNKNOWN);
		return -2;
	}
	#endif
	return 0;
}














// $Lock ...|
string & NmdcProtocol::appendLock(string & str) {
	str.append("$Lock EXTENDEDPROTOCOL_" INTERNALNAME "_by_setuper_" INTERNALVERSION " Pk=" INTERNALNAME);
	return str.append(getSeparator());
}

// $Hello nick|
string & NmdcProtocol::appendHello(string & str, const string & nick) {
	str.reserve(str.size() + nick.size() + 7 + getSeparatorLen());
	return str.append("$Hello ").append(nick).append(getSeparator());
}

// $HubIsFull|
string & NmdcProtocol::appendHubIsFull(string & str) {
	return str.append("$HubIsFull").append(getSeparator());
}

// $GetPass|
string & NmdcProtocol::appendGetPass(string & str) {
	return str.append("$GetPass").append(getSeparator());
}

// $ValidateDenide nick|
string & NmdcProtocol::appendValidateDenied(string & str, const string & nick) {
	str.reserve(str.size() + nick.size() + 16 + getSeparatorLen());
	return str.append("$ValidateDenide ").append(nick).append(getSeparator());
}

// $HubName hubName - topic|
string & NmdcProtocol::appendHubName(string & str, const string & hubName, const string & topic) {
	if (topic.size()) {
		str.reserve(str.size() + hubName.size() + topic.size() + 12 + getSeparatorLen());
		return str.append("$HubName ").append(hubName).append(" - ", 3).append(topic).append(getSeparator());
	} else {
		str.reserve(str.size() + hubName.size() + 9 + getSeparatorLen());
		return str.append("$HubName ").append(hubName).append(getSeparator());
	}
}

// $HubTopic hubTopic|
string & NmdcProtocol::appendHubTopic(string & str, const string & hubTopic) {
	str.reserve(str.size() + hubTopic.size() + 10 + getSeparatorLen());
	return str.append("$HubTopic ").append(hubTopic).append(getSeparator());
}

// <nick> msg|
string & NmdcProtocol::appendChat(string & str, const string & nick, const string & msg) {
	str.reserve(str.size() + nick.size() + msg.size() + 3 + getSeparatorLen());
	return str.append("<").append(nick).append("> ").append(msg).append(getSeparator());
}

// $To: to From: from $<nick> msg|
string & NmdcProtocol::appendPm(string & str, const string & to, const string & from, const string & nick, const string & msg) {
	str.reserve(str.size() + to.size() + from.size() + nick.size() + msg.size() + 17 + getSeparatorLen());
	str.append("$To: ").append(to).append(" From: ").append(from).append(" $<").append(nick);
	return str.append("> ").append(msg).append(getSeparator());
}

// $To: to From: from $<nick> msg|
void NmdcProtocol::appendPmToAll(string & start, string & end, const string & from, const string & nick, const string & msg) {
	start.append("$To: ");
	end.reserve(end.size() + from.size() + nick.size() + msg.size() + 12 + getSeparatorLen());
	end.append(" From: ").append(from).append(" $<").append(nick);
	end.append("> ").append(msg).append(getSeparator());
}

// $Quit nick|
string & NmdcProtocol::appendQuit(string & str, const string & nick) {
	str.reserve(str.size() + nick.size() + 6 + getSeparatorLen());
	return str.append("$Quit ").append(nick).append(getSeparator());
}

// $OpList nick$$|
string & NmdcProtocol::appendOpList(string & str, const string & nick) {
	str.reserve(str.size() + nick.size() + 10 + getSeparatorLen());
	return str.append("$OpList ").append(nick).append("$$").append(getSeparator());
}

// $UserIP nick ip$$|
string & NmdcProtocol::appendUserIp(string & str, const string & nick, const string & ip) {
	if (ip.size()) {
		str.reserve(str.size() + nick.size() + ip.size() + 11 + getSeparatorLen());
		str.append("$UserIP ").append(nick).append(" ").append(ip).append("$$").append(getSeparator());
	}
	return str;
}

// $ForceMove address|
string & NmdcProtocol::appendForceMove(string & str, const string & address) {
	str.reserve(address.size() + 11 + getSeparatorLen());
	return str.append("$ForceMove ").append(address).append(getSeparator());
}





void NmdcProtocol::sendMode(DcConn * dcConn, const string & str, int mode, UserList & userList, bool useCache) {
	if (mode == 0) { // Send to all
		userList.sendToAll(str, useCache, true);
	} else if (mode == 3) { // Send to all except current user
		if (dcConn->mDcUser->isCanSend()) {
			dcConn->mDcUser->setCanSend(false);
			userList.sendToAll(str, useCache, true);
			dcConn->mDcUser->setCanSend(true);
		}
	} else if (mode == 4) { // Send to all except users with ip of the current user
		DcConn * conn = NULL;
		vector<DcConn *> ul;
		for (DcIpList::iterator mit = mDcServer->mIpListConn->begin(dcConn->getIp().c_str()); mit != mDcServer->mIpListConn->end(); ++mit) {
			conn = static_cast<DcConn *> (*mit);
			if(conn->mDcUser && conn->mDcUser->isCanSend() && dcConn->getIp() == conn->getIp()) {
				conn->mDcUser->setCanSend(false);
				ul.push_back(conn);
			}
		}
		userList.sendToAll(str, useCache, true);
		for (vector<DcConn *>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it) {
			(*ul_it)->mDcUser->setCanSend(true);
		}
	}
}



/// Sending the user-list and op-list
int NmdcProtocol::sendNickList(DcConn * dcConn) {
	try {
		if ((dcConn->getLoginStatusFlag(LOGIN_STATUS_LOGIN_DONE) != LOGIN_STATUS_LOGIN_DONE) && mDcServer->mDcConfig.mNicklistOnLogin) {
			dcConn->mNickListInProgress = true;
		}

		if (dcConn->mFeatures & SUPPORT_FEATURE_NOHELLO) {
			if (dcConn->log(3)) {
				dcConn->logStream() << "Sending MyINFO list" << endl;
			}
			// seperator "|" was added in getInfoList function
			dcConn->send(mDcServer->mDcUserList.getList(USER_LIST_MYINFO), false, false);
		} else if (dcConn->mFeatures & SUPPORT_FEATUER_NOGETINFO) {
			if (dcConn->log(3)) {
				dcConn->logStream() << "Sending MyINFO list and Nicklist" << endl;
			}
			// seperator "|" was not added in getNickList function, because seperator was "$$"
			dcConn->send(mDcServer->mDcUserList.getList(USER_LIST_NICK), true, false);
			// seperator "|" was added in getInfoList function
			dcConn->send(mDcServer->mDcUserList.getList(USER_LIST_MYINFO), false, false);
		} else {
			if (dcConn->log(3)) {
				dcConn->logStream() << "Sending Nicklist" << endl;
			}
			// seperator "|" was not added in getNickList function, because seperator was "$$"
			dcConn->send(mDcServer->mDcUserList.getList(USER_LIST_NICK), true, false);
		}
		if (mDcServer->mOpList.size()) {
			if (dcConn->log(3)) {
				dcConn->logStream() << "Sending Oplist" << endl;
			}
			// seperator "|" was not added in getNickList function, because seperator was "$$"
			dcConn->send(mDcServer->mOpList.getList(), true, false);
		}

		if (dcConn->mDcUser->getInUserList() && dcConn->mDcUser->getInIpList()) {
			if (dcConn->log(3)) {
				dcConn->logStream() << "Sending Iplist" << endl;
			}
			// seperator "|" was not added in getIpList function, because seperator was "$$"
			dcConn->send(mDcServer->mDcUserList.getList(USER_LIST_IP), true);
		} else {
			if (!dcConn->sendBufIsEmpty()) { // buf would not flush, if it was empty
				dcConn->flush(); // newPolitic
			} else {
				dcConn->send("", 0 , true, true);
			}
		}
	} catch(...) {
		if (dcConn->errLog(0)) {
			dcConn->logStream() << "exception in sendNickList" << endl;
		}
		return -1;
	}
	return 0;
}

/** Get normal share size */
void NmdcProtocol::getNormalShare(__int64 share, string & normalShare) {
	ostringstream os;
	float s(static_cast<float>(share));
	int i(0);
	for (; ((s >= 1024) && (i < 7)); ++i) {
		s /= 1024;
	}
	os << ::std::floor(s * 1000 + 0.5) / 1000 << " " << DcServer::currentDcServer->mDcLang.mUnits[i];
	normalShare = os.str();
}



int NmdcProtocol::checkCommand(NmdcParser * dcParser, DcConn * dcConn) {

	// Checking length of command
	if (dcParser->getCommandLen() > mDcServer->mDcConfig.mMaxCmdLen[dcParser->mType]) {
		if (dcConn->log(1)) {
			dcConn->logStream() << "Bad CMD(" << dcParser->mType << ") length: " << dcParser->getCommandLen() << endl;
		}
		dcConn->closeNow(CLOSE_REASON_CMD_LENGTH);
		return -1;
	}

	// Checking null chars
	if (strlen(dcParser->mCommand.data()) < dcParser->mCommand.size()) {
		if (dcConn->log(1)) {
			dcConn->logStream() << "Sending null chars, probably attempt an attack" << endl;
		}
		dcConn->closeNow(CLOSE_REASON_CMD_NULL);
		return -2;
	}

	// Check Syntax
	if (dcParser->splitChunks()) {
		// Protection from commands, not belonging to DC protocol
		if (dcParser->mType != NMDC_TYPE_UNKNOWN || mDcServer->mDcConfig.mDisableNoDCCmd) {
			if (dcConn->log(1)) {
				dcConn->logStream() << "Wrong syntax in cmd: " << dcParser->mType << endl;
			}
			dcConn->closeNice(9000, CLOSE_REASON_CMD_SYNTAX);
			return -3;
		}
	}

	// Check flood
	if (antiflood(dcConn, dcParser->mType)) {
		return -4;
	}

	return 0;
}



bool NmdcProtocol::antiflood(DcConn * dcConn, unsigned int type) {
	if (mDcServer->antiFlood(dcConn->mTimes1.mCount[type], dcConn->mTimes1.mTime[type],
		mDcServer->mDcConfig.mFloodCount[type], mDcServer->mDcConfig.mFloodTime[type])
	) {
		#ifndef WITHOUT_PLUGINS
		if (!mDcServer->mCalls.mOnFlood.callAll(dcConn->mDcUser, (int) type, 1))
		#endif
		{
			mDcServer->sendToUser(dcConn->mDcUser, (mDcServer->mDcLang.mFlood[type]).c_str(), mDcServer->mDcConfig.mHubBot.c_str());
			dcConn->closeNice(9000, CLOSE_REASON_FLOOD);
			return true;
		}
	}
	if (mDcServer->antiFlood(dcConn->mTimes2.mCount[type], dcConn->mTimes2.mTime[type],
		mDcServer->mDcConfig.mFloodCount2[type], mDcServer->mDcConfig.mFloodTime2[type])
	) {
		#ifndef WITHOUT_PLUGINS
		if (!mDcServer->mCalls.mOnFlood.callAll(dcConn->mDcUser, (int) type, 2))
		#endif
		{
			mDcServer->sendToUser(dcConn->mDcUser, (mDcServer->mDcLang.mFlood[type]).c_str(), mDcServer->mDcConfig.mHubBot.c_str());
			dcConn->closeNice(9000, CLOSE_REASON_FLOOD);
			return true;
		}
	}
	return false;
}


bool NmdcProtocol::validateUser(DcConn * dcConn, const string & nick) {

	/** Checking for bad symbols in nick */
	if (nick.npos != nick.find_first_of("$| ")) {
		if (dcConn->log(2)) {
			dcConn->logStream() << "Bad nick chars: '" << nick << "'" << endl;
		}
		mDcServer->sendToUser(
			dcConn->mDcUser,
			mDcServer->mDcLang.mBadChars.c_str(),
			mDcServer->mDcConfig.mHubBot.c_str()
		);
		return false;
	}

	return true;
}



bool NmdcProtocol::checkNickLength(DcConn * dcConn, size_t len) {

	if (
		dcConn->mDcUser->getProfile() == -1 && (
			len > mDcServer->mDcConfig.mMaxNickLen ||
			len < mDcServer->mDcConfig.mMinNickLen
		)
	) {

		string msg;

		if (dcConn->log(2)) {
			dcConn->logStream() << "Bad nick len: " 
				<< len << " (" << dcConn->mDcUser->getUid() 
				<< ") [" << mDcServer->mDcConfig.mMinNickLen << ", " 
				<< mDcServer->mDcConfig.mMaxNickLen << "]" << endl;
		}

		stringReplace(mDcServer->mDcLang.mBadNickLen, "min", msg, (int) mDcServer->mDcConfig.mMinNickLen);
		stringReplace(msg, "max", msg, (int) mDcServer->mDcConfig.mMaxNickLen);

		mDcServer->sendToUser(dcConn->mDcUser, msg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());

		return false;
	}
	return true;
}



void NmdcProtocol::nickList(string & list, UserBase * userBase) {
	// $NickList nick1$$nick2$$
	// $OpList nick1$$nick2$$
	if (!userBase->hide() && !userBase->uid().empty()) {
		list.append(userBase->uid());
		list.append("$$");
	}
}



void NmdcProtocol::myInfoList(string & list, UserBase * userBase) {
	// $MyINFO nick1 ...|$MyINFO nick2 ...|
	if (!userBase->hide()) {
		list.append(userBase->myInfoString());
		list.append(NMDC_SEPARATOR);
	}
}



void NmdcProtocol::ipList(string & list, UserBase * userBase) {
	// $UserIP nick1 ip1$$nick2 ip2$$
	if (!userBase->hide() && userBase->ip().size() && !userBase->uid().empty()) {
		list.append(userBase->uid());
		list.append(" ", 1);
		list.append(userBase->ip());
		list.append("$$");
	}
}



void NmdcProtocol::addToOps(DcUser * dcUser) {
	if (dcUser->getInUserList()) {
		string msg;
		mDcServer->mOpList.add(dcUser->getUidHash(), dcUser);
		if (dcUser->getHide()) {
			dcUser->send(appendOpList(msg, dcUser->getUid()), false, true);
		} else {
			mDcServer->mDcUserList.sendToAll(appendOpList(msg, dcUser->getUid()), true/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
			mDcServer->mEnterList.sendToAll(msg, true/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
		}
	}
}



void NmdcProtocol::delFromOps(DcUser * dcUser) {
	if (dcUser->getInUserList()) {
		string sMsg1, sMsg2, sMsg3;
		mDcServer->mOpList.remove(dcUser->getUidHash());
		if (dcUser->getHide()) {
			if (dcUser->mDcConn == NULL) {
				return;
			}
			dcUser->send(appendQuit(sMsg1, dcUser->getUid()), false, false);
			if (dcUser->mDcConn->mFeatures & SUPPORT_FEATURE_NOHELLO) {
				dcUser->send(dcUser->getMyInfo(), true, false);
			} else if (dcUser->mDcConn->mFeatures & SUPPORT_FEATUER_NOGETINFO) {
				dcUser->send(appendHello(sMsg2, dcUser->getUid()), false, false);
				dcUser->send(dcUser->getMyInfo(), true, false);
			} else {
				dcUser->send(appendHello(sMsg2, dcUser->getUid()), false, false);
			}

			if ((dcUser->mDcConn->mFeatures & SUPPORT_FEATUER_USERIP2) || dcUser->getInIpList()) {
				dcUser->send(appendUserIp(sMsg3, dcUser->getUid(), dcUser->getIp()));
			} else {
				dcUser->send("", 0, false, true);
			}
		} else {
			mDcServer->mDcUserList.sendToAll(appendQuit(sMsg1, dcUser->getUid()), true/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
			mDcServer->mEnterList.sendToAll(sMsg1, true/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
			mDcServer->mHelloList.sendToAll(appendHello(sMsg2, dcUser->getUid()), true/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
			mDcServer->mDcUserList.sendToAll(dcUser->getMyInfo(), true/*mDcServer->mDcConfig.mDelayedMyinfo*/);
			mDcServer->mEnterList.sendToAll(dcUser->getMyInfo(), true/*mDcServer->mDcConfig.mDelayedMyinfo*/);
			mDcServer->mIpList.sendToAll(appendUserIp(sMsg3, dcUser->getUid(), dcUser->getIp()), true, false);
		}
	}
}



void NmdcProtocol::addToIpList(DcUser * dcUser) {
	if (dcUser->getInUserList()) {
		mDcServer->mIpList.add(dcUser->getUidHash(), dcUser);
		dcUser->send(mDcServer->mDcUserList.getList(USER_LIST_IP), true);
	}
}



void NmdcProtocol::delFromIpList(DcUser * dcUser) {
	if (dcUser->getInUserList()) {
		mDcServer->mIpList.remove(dcUser->getUidHash());
	}
}



void NmdcProtocol::addToHide(DcUser * dcUser) {
	if (dcUser->isCanSend()) {
		string msg;
		dcUser->setCanSend(false);
		mDcServer->mDcUserList.sendToAll(appendQuit(msg, dcUser->getUid()), false/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
		mDcServer->mEnterList.sendToAll(msg, false/*mDcServer->mDcConfig.mDelayedMyinfo*/, false); // false cache
		dcUser->setCanSend(true);
		mDcServer->mOpList.remake();
		mDcServer->mDcUserList.remake();
	}
}



void NmdcProtocol::delFromHide(DcUser * dcUser) {
	if (dcUser->isCanSend()) {
		string sMsg1, sMsg2, sMsg3;
		if (dcUser->getInOpList()) {
			dcUser->setCanSend(false);
			mDcServer->mHelloList.sendToAll(appendHello(sMsg1, dcUser->getUid()), false/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
			mDcServer->mDcUserList.sendToAll(dcUser->getMyInfo(), true, true);
			mDcServer->mDcUserList.sendToAll(appendOpList(sMsg2, dcUser->getUid()), false/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
			mDcServer->mEnterList.sendToAll(dcUser->getMyInfo(), true, true);
			mDcServer->mEnterList.sendToAll(sMsg2, false/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
			mDcServer->mIpList.sendToAll(appendUserIp(sMsg3, dcUser->getUid(), dcUser->getIp()), false, false);
			dcUser->setCanSend(true);
		} else {
			dcUser->setCanSend(false);
			mDcServer->mHelloList.sendToAll(appendHello(sMsg1, dcUser->getUid()), false/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
			mDcServer->mDcUserList.sendToAll(dcUser->getMyInfo(), false/*mDcServer->mDcConfig.mDelayedMyinfo*/);
			mDcServer->mEnterList.sendToAll(dcUser->getMyInfo(), false/*mDcServer->mDcConfig.mDelayedMyinfo*/);
			mDcServer->mIpList.sendToAll(appendUserIp(sMsg3, dcUser->getUid(), dcUser->getIp()), false, false);
			dcUser->setCanSend(true);
		}
		mDcServer->mOpList.remake();
		mDcServer->mDcUserList.remake();
	}
}



bool NmdcProtocol::badFlag(DcConn * dcConn, const char * cmd, unsigned int flag) {
	if (dcConn->getLoginStatusFlag(flag)) {
		if (dcConn->log(1)) {
			dcConn->logStream() << "Attempt to attack in " << cmd << " (repeated sending)" << endl;
		}
		dcConn->closeNow(CLOSE_REASON_CMD_REPEAT);
		return true;
	}
	return false;
}


}; // namespace protocol

}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
