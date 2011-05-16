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

namespace dcserver {

namespace protocol {


NmdcProtocol::NmdcProtocol() {
	SetClassName("NmdcProtocol");
}



NmdcProtocol::~NmdcProtocol() {
}


int NmdcProtocol::onNewDcConn(DcConn * dcConn) {

	string sMsg;
	dcConn->send(
		appendHubName(
			appendLock(sMsg),
			mDcServer->mDcConfig.mHubName,
			mDcServer->mDcConfig.mTopic
		),
		false,
		false
	);

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnUserConnected.callAll(dcConn)) {
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
		long min = Uptime.Sec() / 60;
		if (iTimeVal != min) {
			iTimeVal = min;
			useCache = false;
			stringstream oss;
			int w, d, h, m;
			Uptime.AsTimeVals(w, d, h, m);
			if (w) {
				oss << w << " " << mDcServer->mDCLang.mTimes[0] << " ";
			}
			if (d) {
				oss << d << " " << mDcServer->mDCLang.mTimes[1] << " ";
			}
			if (h) {
				oss << h << " " << mDcServer->mDCLang.mTimes[2] << " ";
			}
			oss << m << " " << mDcServer->mDCLang.mTimes[3];
			sTimeCache = oss.str();
		}
		if (iShareVal != mDcServer->miTotalShare) {
			iShareVal = mDcServer->miTotalShare;
			useCache = false;
			sShareCache = getNormalShare(iShareVal);
		}
		if (iUsersVal != mDcServer->miTotalUserCount) {
			iUsersVal = mDcServer->miTotalUserCount;
			useCache = false;
		}
		if (!useCache) {
			stringReplace(mDcServer->mDCLang.mFirstMsg, string("HUB"), sCache, string(INTERNALNAME " " INTERNALVERSION));
			stringReplace(sCache, string("uptime"), sCache, sTimeCache);
			stringReplace(sCache, string("users"), sCache, iUsersVal);
			stringReplace(sCache, string("share"), sCache, sShareCache);
		}
		mDcServer->sendToUser(dcConn, sCache.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
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

		DcParser * dcParser = static_cast<DcParser *> (parser);
		if (!dcParser->splitChunks()) {

			// NMDC
			DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(dcParser->chunkString(CHUNK_SR_FROM)));
			if (dcUser && dcUser->mDcConn && conn->ipUdp() == dcUser->getIp()) {
				return dcUser->mDcConn;
			} else {
				if (conn->Log(3)) {
					conn->LogStream() << "Not found user for UDP data" << endl;
				}
			}
		} else {
			if (conn->Log(3)) {
				conn->LogStream() << "Bad UDP cmd syntax" << endl;
			}
		}
	} else {
		if (conn->Log(4)) {
			conn->LogStream() << "Unknown UDP data" << endl;
		}
	}
	return NULL;
}

int NmdcProtocol::doCommand(Parser * parser, Conn * conn) {

	DcParser * dcParser = static_cast<DcParser *> (parser);
	DcConn * dcConn = static_cast<DcConn *> (conn);


	if (checkCommand(dcParser, dcConn) < 0) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnAny.callAll(dcConn, dcParser->mType)) {
			return 1;
		}
	#endif

	if (dcConn->Log(5)) {
		dcConn->LogStream() << "[S]Stage " << dcParser->mType << endl;
	}
	
	switch (dcParser->mType) {

		case NMDC_TYPE_MSEARCH :
			// Fallthrough

		case NMDC_TYPE_MSEARCH_PAS :
			// Fallthrough

		case NMDC_TYPE_SEARCH_PAS :
			// Fallthrough

		case NMDC_TYPE_SEARCH :
			eventSearch(dcParser, dcConn);
			break;

		case NMDC_TYPE_SR_UDP :
			// Fallthrough

		case NMDC_TYPE_SR :
			eventSr(dcParser, dcConn);
			break;

		case NMDC_TYPE_MYNIFO :
			eventMyInfo(dcParser, dcConn);
			break;

		case NMDC_TYPE_SUPPORTS :
			eventSupports(dcParser, dcConn);
			break;

		case NMDC_TYPE_KEY :
			eventKey(dcParser, dcConn);
			break;

		case NMDC_TYPE_VALIDATENICK :
			eventValidateNick(dcParser, dcConn);
			break;

		case NMDC_TYPE_VERSION :
			eventVersion(dcParser, dcConn);
			break;

		case NMDC_TYPE_GETNICKLIST :
			eventGetNickList(dcParser, dcConn);
			break;

		case NMDC_TYPE_CHAT :
			eventChat(dcParser, dcConn);
			break;

		case NMDC_TYPE_TO :
			eventTo(dcParser, dcConn);
			break;

		case NMDC_TYPE_MYPASS :
			eventMyPass(dcParser, dcConn);
			break;

		case NMDC_TYPE_CONNECTTOME :
			eventConnectToMe(dcParser, dcConn);
			break;

		case NMDC_TYPE_RCONNECTTOME :
			eventRevConnectToMe(dcParser, dcConn);
			break;

		case NMDC_TYPE_MCONNECTTOME :
			eventMultiConnectToMe(dcParser, dcConn);
			break;

		case NMDC_TYPE_KICK :
			eventKick(dcParser, dcConn);
			break;

		case NMDC_TYPE_OPFORCEMOVE :
			eventOpForceMove(dcParser, dcConn);
			break;

		case NMDC_TYPE_GETINFO :
			eventGetInfo(dcParser, dcConn);
			break;

		case NMDC_TYPE_MCTO :
			eventMcTo(dcParser, dcConn);
			break;

		case NMDC_TYPE_PING :
			eventPing(dcParser, dcConn);
			break;

		case NMDC_TYPE_USERIP :
			eventUserIp(dcParser, dcConn);
			break;

		case NMDC_TYPE_UNKNOWN :
			eventUnknown(dcParser, dcConn);
			break;

		case NMDC_TYPE_QUIT :
			eventQuit(dcParser, dcConn);
			break;

		case NMDC_TYPE_UNPARSED :
			dcParser->parse();
			return doCommand(dcParser, dcConn);

		default :
			if (ErrLog(1)) {
				LogStream() << "Incoming untreated event: " << dcParser->mType << endl;
			}
			break;

	}

	if (dcConn->Log(5)) {
		dcConn->LogStream() << "[E]Stage " << dcParser->mType << endl;
	}
	return 0;
}

int NmdcProtocol::eventSupports(DcParser * dcparser, DcConn * dcConn) {

	string feature;
	istringstream is(dcparser->mCommand);
	is >> feature;
	dcConn->mFeatures = 0;
	is >> feature;
	while (feature.size()) {
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
		feature.clear();
		is >> feature;
	}
	dcConn->mSupports.assign(dcparser->mCommand, 10, dcparser->mCommand.size() - 10);

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnSupports.callAll(dcConn)) {
			return -3;
		}
	#endif

	static string msg("$Supports UserCommand NoGetINFO NoHello UserIP UserIP2 MCTo"NMDC_SEPARATOR);
	dcConn->send(msg, false, false);

	return 0;
}

int NmdcProtocol::eventKey(DcParser *, DcConn * dcConn) {
	if (badFlag(dcConn, "Key", LOGIN_STATUS_KEY)) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnKey.callAll(dcConn)) {
			//string sLock, sKey;
			//sLock = appendLock(sLock).substr(1, sLock.size() - 1);
			//Lock2Key(sLock, sKey);
		}
	#endif

	dcConn->setLoginStatusFlag(LOGIN_STATUS_KEY); /** User has sent key */
	dcConn->clearTimeOut(HUB_TIME_OUT_KEY);
	dcConn->setTimeOut(HUB_TIME_OUT_VALNICK, mDcServer->mDcConfig.mTimeout[HUB_TIME_OUT_VALNICK], mDcServer->mTime);
	dcConn->setConnectTimeNow();
	return 0;
}

int NmdcProtocol::eventValidateNick(DcParser * dcparser, DcConn * dcConn) {
	if (badFlag(dcConn, "ValidateNick", LOGIN_STATUS_VALNICK)) {
		return -1;
	}

	string &sNick = dcparser->chunkString(CHUNK_1_PARAM);
	size_t iNickLen = sNick.length();

	/** Additional checking the nick length */
	if (iNickLen > 0xFF) {
		if (dcConn->Log(1)) {
			dcConn->LogStream() << "Attempt to attack by long nick" << endl;
		}
		dcConn->closeNow(CLOSE_REASON_NICK_LONG);
		return -1;
	}

	if (dcConn->Log(3)) {
		dcConn->LogStream() << "User " << sNick << " to validate nick" << endl;
	}

	try {
		DcUser * newUser = new DcUser();
		newUser->setNick(sNick);
		if (!dcConn->setUser(newUser)) {
			dcConn->closeNow(CLOSE_REASON_USER_SET);
			return -2;
		}
	} catch(...) {
		if (mDcServer->ErrLog(0)) {
			mDcServer->LogStream() << "Unhandled exception in NmdcProtocol::eventValidateNick" << endl;
		}
		if (dcConn->ErrLog(0)) {
			dcConn->LogStream() << "Error in setUser closing" << endl;
		}
		dcConn->closeNice(9000, CLOSE_REASON_USER_SET);
		return -2;
	}

	/** Checking validate user */
	if (!validateUser(dcConn, sNick)) {
		dcConn->closeNice(9000, CLOSE_REASON_USER_INVALID);
		return -2;
	}

	/** Global user's limit */
	if (mDcServer->mDcConfig.mUsersLimit >= 0 && mDcServer->miTotalUserCount >= mDcServer->mDcConfig.mUsersLimit) {
		if (dcConn->Log(3)) {
			dcConn->LogStream() << "User " << sNick << " was disconnected (user's limit: " << mDcServer->mDcConfig.mUsersLimit << ")" << endl;
		}
		mDcServer->sendToUser(dcConn, mDcServer->mDCLang.mUsersLimit.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_USERS_LIMIT);
		return -3;
	}

	dcConn->setLoginStatusFlag(LOGIN_STATUS_ALOWED);
	++ mDcServer->miTotalUserCount;

	dcConn->setLoginStatusFlag(LOGIN_STATUS_VALNICK | LOGIN_STATUS_NICKLST); /** We Install NICKLST because user can not call user list */
	dcConn->clearTimeOut(HUB_TIME_OUT_VALNICK);

	string sMsg;

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnValidateNick.callAll(dcConn)) {
			dcConn->send(appendGetPass(sMsg)); /** We are sending the query for reception of the password */
			dcConn->setTimeOut(HUB_TIME_OUT_PASS, mDcServer->mDcConfig.mTimeout[HUB_TIME_OUT_PASS], mDcServer->mTime);
			return -4;
		}
	#endif

	if (!checkNickLength(dcConn, iNickLen)) {
		dcConn->closeNice(9000, CLOSE_REASON_NICK_LEN);
	}
	dcConn->setTimeOut(HUB_TIME_OUT_MYINFO, mDcServer->mDcConfig.mTimeout[HUB_TIME_OUT_MYINFO], mDcServer->mTime);
	dcConn->setLoginStatusFlag(LOGIN_STATUS_PASSWD); /** Does not need password */

	dcConn->send(appendHello(sMsg, dcConn->mDcUser->getNick())); /** Protection from change the command */
	return 0;
}

int NmdcProtocol::eventMyPass(DcParser *, DcConn * dcConn) {
	if (!dcConn->mDcUser) { /* Check of existence of the user for current connection */
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Mypass before validatenick" << endl;
		}
		mDcServer->sendToUser(dcConn, mDcServer->mDCLang.mBadLoginSequence.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
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
		bOp = (mDcServer->mCalls.mOnMyPass.callAll(dcConn));
	#endif

	string sMsg;
	dcConn->setLoginStatusFlag(LOGIN_STATUS_PASSWD); /** Password is accepted */
	appendHello(sMsg, dcConn->mDcUser->getNick());
	if (bOp) { /** If entered operator, that sends command LoggedIn ($LogedIn !) */
		sMsg.append("$LogedIn ");
		sMsg.append(dcConn->mDcUser->getNick());
		sMsg.append(NMDC_SEPARATOR);
	}
	dcConn->send(sMsg);
	dcConn->clearTimeOut(HUB_TIME_OUT_PASS);
	dcConn->setTimeOut(HUB_TIME_OUT_MYINFO, mDcServer->mDcConfig.mTimeout[HUB_TIME_OUT_MYINFO], mDcServer->mTime);
	return 0;
}

int NmdcProtocol::eventVersion(DcParser * dcparser, DcConn * dcConn) {
	if (badFlag(dcConn, "Version", LOGIN_STATUS_VERSION)) {
		return -1;
	}

	string & sVersion = dcparser->chunkString(CHUNK_1_PARAM);
	if (dcConn->Log(3)) {
		dcConn->LogStream() << "Version:" << sVersion << endl;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnVersion.callAll(dcConn) && sVersion != "1,0091") {
			dcConn->closeNice(9000, CLOSE_REASON_CMD_VERSION); /** Checking of the version */
		}
	#endif

	dcConn->setLoginStatusFlag(LOGIN_STATUS_VERSION); /** Version was checked */
	dcConn->mVersion = sVersion;
	return 0;
}

int NmdcProtocol::eventGetNickList(DcParser *, DcConn * dcConn) {

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnGetNickList.callAll(dcConn)) {
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
		dcConn->mbSendNickList = true;
		return 0;
	}
	return sendNickList(dcConn);
}

int NmdcProtocol::eventMyInfo(DcParser * dcparser, DcConn * dcConn) {

	const string & sNick = dcparser->chunkString(CHUNK_MI_NICK);

	/** Check existence user, otherwise check support QuickList */
	if (!dcConn->mDcUser) {
		//if (QuickList)
		//	dcConn->mDcUser->msNick = sNick;
		//} else
		{
			if (dcConn->Log(2)) {
				dcConn->LogStream() << "Myinfo without user: " << dcparser->mCommand << endl;
			}
			mDcServer->sendToUser(dcConn, mDcServer->mDCLang.mBadLoginSequence.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
			dcConn->closeNice(9000, CLOSE_REASON_CMD_MYINFO_WITHOUT_USER);
			return -2;
		}
	} else if (sNick != dcConn->mDcUser->getNick()) { /** Проверка ника */
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Bad nick in MyINFO, closing" << endl;
		}
		mDcServer->sendToUser(dcConn, mDcServer->mDCLang.mBadMyinfoNick.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_MYINFO);
		return -1;
	}

	string sOldMyINFO = dcConn->mDcUser->getMyINFO();
	dcConn->mDcUser->setMyINFO(dcparser);

	int iMode = 0;
	#ifndef WITHOUT_PLUGINS
		iMode = mDcServer->mCalls.mOnMyINFO.callAll(dcConn);
	#endif

	if (iMode != 1 && dcConn->mDcUser->getInUserList()) {
		if (sOldMyINFO != dcConn->mDcUser->getMyINFO()) {
			if (dcConn->mDcUser->mHide) {
				dcConn->send(dcparser->mCommand, true); // Send to self only
			} else {
				sendMode(dcConn, dcparser->mCommand, iMode, mDcServer->mDcUserList, true); // Use cache for send to all
				//mDcServer->mDcUserList.sendToAll(dcparser->mCommand, true/*mDcServer->mDcConfig.mDelayedMyinfo*/); // Send to all
			}
		}
	} else if (!dcConn->mDcUser->getInUserList()) {
		dcConn->setLoginStatusFlag(LOGIN_STATUS_MYINFO);
		if (!mDcServer->beforeUserEnter(dcConn)) {
			return -1;
		}
		dcConn->clearTimeOut(HUB_TIME_OUT_MYINFO);
	}
	return 0;
}

int NmdcProtocol::eventChat(DcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser) {
		return -2;
	}
	if (!dcConn->mDcUser->getInUserList()) {
		return -3;
	}

	/** Check chat nick */
	if ((dcparser->chunkString(CHUNK_CH_NICK) != dcConn->mDcUser->getNick()) ) {
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Bad nick in chat, closing" << endl;
		}
		string sMsg = mDcServer->mDCLang.mBadChatNick;
		stringReplace(sMsg, string("nick"), sMsg, dcparser->chunkString(CHUNK_CH_NICK));
		stringReplace(sMsg, string("real_nick"), sMsg, dcConn->mDcUser->getNick());
		mDcServer->sendToUser(dcConn, sMsg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_CHAT);
		return -2;
	}
	int iMode = 0;
	#ifndef WITHOUT_PLUGINS
		iMode = mDcServer->mCalls.mOnChat.callAll(dcConn);
	#endif

	//Hash<unsigned long> hash;
	//unsigned long key = hash(dcparser->mCommand);
	//cout << key << endl;

	/** Sending message */
	sendMode(dcConn, dcparser->mCommand, iMode, mDcServer->mChatList, false); // Don't use cache for send to all
	return 0;
}

int NmdcProtocol::eventTo(DcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser) {
		return -2;
	}
	if (!dcConn->mDcUser->getInUserList()) {
		return -3;
	}
	string & nick = dcparser->chunkString(CHUNK_PM_TO);

	/** Checking the coincidence nicks in command */
	if (dcparser->chunkString(CHUNK_PM_FROM) != dcConn->mDcUser->getNick() || dcparser->chunkString(CHUNK_PM_NICK) != dcConn->mDcUser->getNick()) {
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Bad nick in PM, closing" <<endl;
		}
		dcConn->closeNow(CLOSE_REASON_NICK_PM);
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnTo.callAll(dcConn)) {
			return 0;
		}
	#endif

	/** Search user */
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(nick)); // NMDC
	if (!dcUser) {
		return -2;
	}

	dcUser->send(dcparser->mCommand, true);
	return 0;
}

int NmdcProtocol::eventMcTo(DcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser) {
		return -2;
	}
	if (!dcConn->mDcUser->getInUserList()) {
		return -3;
	}

	string & nick = dcparser->chunkString(CHUNK_MC_TO);

	/** Checking the coincidence nicks in command */
	if (dcparser->chunkString(CHUNK_MC_FROM) != dcConn->mDcUser->getNick()) {
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Bad nick in MCTo, closing" <<endl;
		}
		dcConn->closeNow(CLOSE_REASON_NICK_MCTO);
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnMCTo.callAll(dcConn)) {
			return 0;
		}
	#endif

	/** Search user */
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(nick)); // NMDC
	if (!dcUser) {
		return -2;
	}

	string msg;
	dcUser->send(appendChat(msg, dcConn->mDcUser->getNick(), dcparser->chunkString(CHUNK_MC_MSG)));
	if (dcConn->mDcUser->getNick() != nick) {
		dcConn->send(msg);
	}
	return 0;
}


int NmdcProtocol::eventUserIp(DcParser * dcParser, DcConn * dcConn) {

	if (!dcConn->mDcUser) {
		return -2;
	}
	if (!dcConn->mDcUser->getInUserList()) {
		return -3;
	}

	if (!(dcConn->mFeatures & (SUPPORT_FEATUER_USERIP | SUPPORT_FEATUER_USERIP2))) {
		return -4;
	}

	string param = dcParser->chunkString(CHUNK_1_PARAM);
	string nick, result("$UserIP ");

	size_t pos = param.find("$$");
	size_t cur = 0;
	while (pos != param.npos) {
		nick.assign(param, cur, pos - cur);
		if (nick.size()) {
			// NMDC
			UserBase * userBase = mDcServer->mDcUserList.getUserBaseByNick(nick);
			if (userBase != NULL) {
				result.append(nick).append(" ").append(userBase->getIp()).append("$$");
			}
		}
		cur = pos + 2;
		pos = param.find("$$", pos + 2);
	}

	// last param
	nick.assign(param, cur, param.size() - cur);
	if (nick.size()) {
		// NMDC
		UserBase * userBase = mDcServer->mDcUserList.getUserBaseByNick(nick);
		if (userBase != NULL) {
			result.append(nick).append(" ").append(userBase->getIp());
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
int NmdcProtocol::eventSearch(DcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser || !dcConn->mDcUser->getInUserList()) {
		return -2;
	}

	// TODO: Check overloading of the system

	int iMode = 0;
	#ifndef WITHOUT_PLUGINS
		iMode = mDcServer->mCalls.mOnSearch.callAll(dcConn);
	#endif

	/** Sending cmd */
	string sMsg;

	switch (dcparser->mType) {

		case NMDC_TYPE_SEARCH :
			if (mDcServer->mDcConfig.mCheckSearchIp && dcConn->getIp() != dcparser->chunkString(CHUNK_AS_IP)) {
				sMsg = mDcServer->mDCLang.mBadSearchIp;
				if (dcConn->Log(2)) {
					dcConn->LogStream() << "Bad ip in active search, closing" << endl;
				}
				stringReplace(sMsg, string("ip"), sMsg, dcparser->chunkString(CHUNK_AS_IP));
				stringReplace(sMsg, string("real_ip"), sMsg, dcConn->getIp());
				mDcServer->sendToUser(dcConn, sMsg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
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
				sMsg = mDcServer->mDCLang.mBadSearchIp;
				if (dcConn->Log(2)) {
					dcConn->LogStream() << "Bad ip in active search, closing" << endl;
				}
				stringReplace(sMsg, string("ip"), sMsg, dcparser->chunkString(CHUNK_AS_IP));
				stringReplace(sMsg, string("real_ip"), sMsg, dcConn->getIp());
				mDcServer->sendToUser(dcConn, sMsg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
				dcConn->closeNice(9000, CLOSE_REASON_NICK_SEARCH);
				return -1;
			}
			sMsg = "$Search ";
			sMsg += dcparser->chunkString(CHUNK_AS_ADDR);
			sMsg += ' ';
			sMsg += dcparser->chunkString(CHUNK_AS_QUERY);
			sendMode(dcConn, sMsg, iMode, mDcServer->mDcUserList, true); // Use cache for send to all
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

int NmdcProtocol::eventSr(DcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser || !dcConn->mDcUser->getInUserList()) {
		return -2;
	}

	/** Check same nick in cmd (PROTOCOL NMDC) */
	if (mDcServer->mDcConfig.mCheckSrNick && (dcConn->mDcUser->getNick() != dcparser->chunkString(CHUNK_SR_FROM))) {
		string sMsg = mDcServer->mDCLang.mBadSrNick;
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Bad nick in search response, closing" << endl;
		}
		stringReplace(sMsg, "nick", sMsg, dcparser->chunkString(CHUNK_SR_FROM));
		stringReplace(sMsg, "real_nick", sMsg, dcConn->mDcUser->getNick());
		mDcServer->sendToUser(dcConn, sMsg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_SR);
		return -1;
	}

	DcUser * dcUser = NULL;
	const string & nick = dcparser->chunkString(CHUNK_SR_TO);
	if (nick != "") {
		dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(nick));

		/** Is user exist? */
		if (!dcUser || !dcUser->mDcConn) {
			return -2;
		}
	}

	/** != 0 - error */
	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnSR.callAll(dcConn)) {
			return -3;
		}
	#endif

	/** Sending cmd */
	if (dcUser && (!mDcServer->mDcConfig.mMaxPassiveRes ||
		(dcUser->mDcConn->getSrCounter() <= unsigned(mDcServer->mDcConfig.mMaxPassiveRes))
	)) {
		dcUser->mDcConn->increaseSrCounter();
		string sStr(dcparser->mCommand, 0, dcparser->mChunks[CHUNK_SR_TO].first - 1); /** Remove nick on the end of cmd */
		dcUser->mDcConn->send(sStr, true, false);
	}
	return 0;
}


int NmdcProtocol::eventConnectToMe(DcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser || !dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	if (mDcServer->mDcConfig.mCheckCtmIp && dcConn->getIp() != dcparser->chunkString(CHUNK_CM_IP)) {
		string sMsg = mDcServer->mDCLang.mBadCtmIp;
		stringReplace(sMsg, string("ip"), sMsg, dcparser->chunkString(CHUNK_CM_IP));
		stringReplace(sMsg, string("real_ip"), sMsg, dcConn->getIp());
		mDcServer->sendToUser(dcConn, sMsg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_CTM);
		return -1;
	}

	// NMDC
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(dcparser->chunkString(CHUNK_CM_NICK)));
	if (!dcUser) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnConnectToMe.callAll(dcConn)) {
			return -2;
		}
	#endif

	dcUser->send(dcparser->mCommand, true);
	return 0;
}

int NmdcProtocol::eventRevConnectToMe(DcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser || !dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	/** Checking the nick (PROTOCOL NMDC) */
	if (mDcServer->mDcConfig.mCheckRctmNick && (dcparser->chunkString(CHUNK_RC_NICK) != dcConn->mDcUser->getNick())) {
		string sMsg = mDcServer->mDCLang.mBadRevConNick;
		stringReplace(sMsg, string("nick"), sMsg, dcparser->chunkString(CHUNK_RC_NICK));
		stringReplace(sMsg, string("real_nick"), sMsg, dcConn->mDcUser->getNick());
		mDcServer->sendToUser(dcConn, sMsg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_RCTM);
		return -1;
	}

	/** Searching the user */
	DcUser * other = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(dcparser->chunkString(CHUNK_RC_OTHER))); // NMDC
	if (!other) {
		return -2;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnRevConnectToMe.callAll(dcConn)) {
			return -2;
		}
	#endif

	other->send(dcparser->mCommand, true);
	return 0;
}

int NmdcProtocol::eventMultiConnectToMe(DcParser *, DcConn *) {
	return 0;
}

int NmdcProtocol::eventKick(DcParser * dcparser, DcConn * dcConn) {

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnKick.callAll(dcConn)) {
			return -1;
		}
	#endif

	if (!dcConn->mDcUser || !dcConn->mDcUser->mKick) {
		return -2;
	}

	// NMDC
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(dcparser->chunkString(CHUNK_1_PARAM)));

	/** Is user exist? */
	if (!dcUser || !dcUser->mDcConn) {
		return -3;
	}

	dcUser->mDcConn->closeNice(9000, CLOSE_REASON_CMD_KICK);
	return 0;
}

int NmdcProtocol::eventOpForceMove(DcParser * dcparser, DcConn * dcConn) {

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnOpForceMove.callAll(dcConn)) {
			return -1;
		}
	#endif

	if (!dcConn->mDcUser || !dcConn->mDcUser->mForceMove) {
		return -2;
	}

	// NMDC
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(dcparser->chunkString(CHUNK_FM_NICK)));

	/** Is user exist? */
	if (!dcUser || !dcUser->mDcConn || !dcparser->chunkString(CHUNK_FM_DEST).size()) {
		return -3;
	}

	mDcServer->forceMove(dcUser->mDcConn, dcparser->chunkString(CHUNK_FM_DEST).c_str(), dcparser->chunkString(CHUNK_FM_REASON).c_str());
	return 0;
}

int NmdcProtocol::eventGetInfo(DcParser * dcparser, DcConn * dcConn) {
	if (!dcConn->mDcUser || !dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	// NMDC
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(dcparser->chunkString(CHUNK_GI_OTHER)));
	if (!dcUser) {
		return -2;
	}

	if (dcConn->mDcUser->mTimeEnter < dcUser->mTimeEnter && Time() < (dcUser->mTimeEnter + 60000)) {
		return 0;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnGetINFO.callAll(dcConn)) {
			return -2;
		}
	#endif

	//if(!(dcConn->mFeatures & SUPPORT_FEATUER_NOGETINFO)){
	if (!dcUser->mHide) {
		dcConn->send(string(dcUser->getMyINFO()), true, false);
	}
	return 0;
}

int NmdcProtocol::eventPing(DcParser *, DcConn *) {
	return 0;
}

int NmdcProtocol::eventUnknown(DcParser *, DcConn * dcConn) {

	#ifndef WITHOUT_PLUGINS
	if (!mDcServer->mCalls.mOnUnknown.callAll(dcConn)) {
		dcConn->closeNice(9000, CLOSE_REASON_CMD_UNKNOWN);
		return -2;
	}
	#endif
	return 0;
}

int NmdcProtocol::eventQuit(DcParser *, DcConn * dcConn) {
	dcConn->closeNice(9000, CLOSE_REASON_CMD_QUIT);
	return 0;
}













// $Lock ...|
string & NmdcProtocol::appendLock(string & str) {
	static const char * cmd = "$Lock EXTENDEDPROTOCOL_" INTERNALNAME "_by_setuper_" INTERNALVERSION " Pk=" INTERNALNAME NMDC_SEPARATOR;
	static size_t cmdLen = strlen(cmd);
	return str.append(cmd, cmdLen);
}

// $Hello nick|
string & NmdcProtocol::appendHello(string & str, const string & nick) {
	static const char * cmd = "$Hello ";
	static size_t cmdLen = 7 + NMDC_SEPARATOR_LEN;
	str.reserve(str.size() + nick.size() + cmdLen);
	return str.append(cmd, 7).append(nick).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
}

// $HubIsFull|
string & NmdcProtocol::appendHubIsFull(string & str) {
	static const char * cmd = "$HubIsFull" NMDC_SEPARATOR;
	static size_t cmdLen = 10 + NMDC_SEPARATOR_LEN;
	return str.append(cmd, cmdLen);
}

// $GetPass|
string & NmdcProtocol::appendGetPass(string & str) {
	static const char * cmd = "$GetPass" NMDC_SEPARATOR;
	static size_t cmdLen = 8 + NMDC_SEPARATOR_LEN;
	return str.append(cmd, cmdLen);
}

// $ValidateDenide nick|
string & NmdcProtocol::appendValidateDenide(string & str, const string & nick) {
	static const char * cmd = "$ValidateDenide ";
	static size_t cmdLen = 16 + NMDC_SEPARATOR_LEN;
	str.reserve(str.size() + nick.size() + cmdLen);
	return str.append(cmd, 16).append(nick).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
}

// $HubName hubName - topic|
string & NmdcProtocol::appendHubName(string & str, const string & hubName, const string & topic) {
	static const char * cmd = "$HubName ";
	static const char * cmd2 = " - ";
	static size_t cmdLen = 9 + NMDC_SEPARATOR_LEN;
	static size_t cmdLen2 = 12 + NMDC_SEPARATOR_LEN;
	if (topic.length()) {
		str.reserve(str.size() + hubName.size() + topic.size() + cmdLen2);
		return str.append(cmd, 9).append(hubName).append(cmd2, 3).append(topic).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
	} else {
		str.reserve(str.size() + hubName.size() + cmdLen);
		return str.append(cmd, 9).append(hubName).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
	}
}

// $HubTopic hubTopic|
string & NmdcProtocol::appendHubTopic(string & str, const string & hubTopic) {
	static const char * cmd = "$HubTopic ";
	static size_t cmdLen = 10 + NMDC_SEPARATOR_LEN;
	str.reserve(str.size() + hubTopic.size() + cmdLen);
	return str.append(cmd, 10).append(hubTopic).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
}

// <nick> msg|
string & NmdcProtocol::appendChat(string & str, const string & nick, const string & msg) {
	static const char * cmd = "<";
	static const char * cmd2 = "> ";
	static size_t cmdLen = 3 + NMDC_SEPARATOR_LEN;
	str.reserve(str.size() + nick.size() + msg.size() + cmdLen);
	return str.append(cmd, 1).append(nick).append(cmd2, 2).append(msg).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
}

// $To: to From: from $<nick> msg|
string & NmdcProtocol::appendPm(string & str, const string & to, const string & from, const string & nick, const string & msg) {
	static const char * cmd = "$To: ";
	static const char * cmd2 = " From: ";
	static const char * cmd3 = " $<";
	static const char * cmd4 = "> ";
	static size_t cmdLen = 17 + NMDC_SEPARATOR_LEN;
	str.reserve(str.size() + to.size() + from.size() + nick.size() + msg.size() + cmdLen);
	str.append(cmd, 5).append(to).append(cmd2, 7).append(from).append(cmd3, 3).append(nick);
	return str.append(cmd4, 2).append(msg).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
}

// $To: to From: from $<nick> msg|
void NmdcProtocol::appendPmToAll(string & start, string & end, const string & from, const string & nick, const string & msg) {
	static const char * cmd = "$To: ";
	static const char * cmd2 = " From: ";
	static const char * cmd3 = " $<";
	static const char * cmd4 = "> ";
	static size_t cmdLen = 12 + NMDC_SEPARATOR_LEN;
	start.append(cmd, 5);
	end.reserve(end.size() + from.size() + nick.size() + msg.size() + cmdLen);
	end.append(cmd2, 7).append(from).append(cmd3, 3).append(nick);
	end.append(cmd4, 2).append(msg).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
}

// $Quit nick|
string & NmdcProtocol::appendQuit(string & str, const string & nick) {
	static const char * cmd = "$Quit ";
	static size_t cmdLen = 6 + NMDC_SEPARATOR_LEN;
	str.reserve(str.size() + nick.size() + cmdLen);
	return str.append(cmd, 6).append(nick).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
}

// $OpList nick$$|
string & NmdcProtocol::appendOpList(string & str, const string & nick) {
	static const char * cmd = "$OpList ";
	static const char * cmd2 = "$$"NMDC_SEPARATOR;
	static size_t cmdLen = 10 + NMDC_SEPARATOR_LEN;
	static size_t cmdLen2 = 2 + NMDC_SEPARATOR_LEN;
	str.reserve(str.size() + nick.size() + cmdLen);
	return str.append(cmd, 8).append(nick).append(cmd2, cmdLen2);
}

// $UserIP nick ip$$|
string & NmdcProtocol::appendUserIp(string & str, const string & nick, const string & ip) {
	static const char * cmd = "$UserIP ";
	static const char * cmd2 = " ";
	static const char * cmd3 = "$$"NMDC_SEPARATOR;
	static size_t cmdLen = 11 + NMDC_SEPARATOR_LEN;
	static size_t cmdLen2 = 2 + NMDC_SEPARATOR_LEN;
	if (ip.length()) {
		str.reserve(str.size() + nick.size() + ip.size() + cmdLen);
		str.append(cmd, 8).append(nick).append(cmd2, 1).append(ip).append(cmd3, cmdLen2);
	}
	return str;
}

string & NmdcProtocol::appendForceMove(string & str, const string & address) {
	static const char * cmd = "$forceMove ";
	str.reserve(address.size() + 11 + NMDC_SEPARATOR_LEN);
	return str.append(cmd, 11).append(address).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
}





void NmdcProtocol::sendMode(DcConn * dcConn, const string & str, int iMode, UserList & UL, bool bUseCache) {
	bool bAddSep = false;
	if (str.substr(str.size() - 1, 1) != NMDC_SEPARATOR) {
		bAddSep = true;
	}

	if (iMode == 0) { /** Send to all */
		UL.sendToAll(str, bUseCache, bAddSep);
	} else if (iMode == 3) { /** Send to all except current user */
		if (dcConn->mDcUser->isCanSend()) {
			dcConn->mDcUser->setCanSend(false);
			UL.sendToAll(str, bUseCache, bAddSep);
			dcConn->mDcUser->setCanSend(true);
		}
	} else if (iMode == 4) { /** Send to all except users with ip of the current user */
		DcConn * conn = NULL;
		vector<DcConn *> ul;
		for (DcIpList::iterator mit = mDcServer->mIPListConn->begin(DcConn::ip2Num(dcConn->getIp().c_str())); mit != mDcServer->mIPListConn->end(); ++mit) {
			conn = static_cast<DcConn *> (*mit);
			if(conn->mDcUser && conn->mDcUser->isCanSend()) {
				conn->mDcUser->setCanSend(false);
				ul.push_back(conn);
			}
		}
		UL.sendToAll(str, bUseCache, bAddSep);
		for (vector<DcConn *>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it) {
			(*ul_it)->mDcUser->setCanSend(true);
		}
	}
}


/** Sending the user-list and op-list */
int NmdcProtocol::sendNickList(DcConn * dcConn) {
	try {
		if ((dcConn->getLoginStatusFlag(LOGIN_STATUS_LOGIN_DONE) != LOGIN_STATUS_LOGIN_DONE) && mDcServer->mDcConfig.mNicklistOnLogin) {
			dcConn->mbNickListInProgress = true;
		}

		if (dcConn->mFeatures & SUPPORT_FEATURE_NOHELLO) {
			if (dcConn->Log(3)) {
				dcConn->LogStream() << "Sending MyINFO list" << endl;
			}
			// seperator "|" was added in getInfoList function
			dcConn->send(mDcServer->mDcUserList.getInfoList(true), false, false);
		} else if (dcConn->mFeatures & SUPPORT_FEATUER_NOGETINFO) {
			if (dcConn->Log(3)) {
				dcConn->LogStream() << "Sending MyINFO list and Nicklist" << endl;
			}
			// seperator "|" was not added in getNickList function, because seperator was "$$"
			dcConn->send(mDcServer->mDcUserList.getNickList(), true, false);
			// seperator "|" was added in getInfoList function
			dcConn->send(mDcServer->mDcUserList.getInfoList(true), false, false);
		} else {
			if (dcConn->Log(3)) {
				dcConn->LogStream() << "Sending Nicklist" << endl;
			}
			// seperator "|" was not added in getNickList function, because seperator was "$$"
			dcConn->send(mDcServer->mDcUserList.getNickList(), true, false);
		}
		if (mDcServer->mOpList.size()) {
			if (dcConn->Log(3)) {
				dcConn->LogStream() << "Sending Oplist" << endl;
			}
			// seperator "|" was not added in getNickList function, because seperator was "$$"
			dcConn->send(mDcServer->mOpList.getNickList(), true, false);
		}

		if (dcConn->mDcUser->getInUserList() && dcConn->mDcUser->mInIpList) {
			if (dcConn->Log(3)) {
				dcConn->LogStream() << "Sending Iplist" << endl;
			}
			// seperator "|" was not added in getIpList function, because seperator was "$$"
			dcConn->send(mDcServer->mDcUserList.getIpList(), true);
		} else {
			if (!dcConn->sendBufIsEmpty()) { // buf would not flush, if it was empty
				dcConn->flush(); // newPolitic
			} else {
				static string s(NMDC_SEPARATOR);
				dcConn->send(s);
			}
		}
	} catch(...) {
		if (dcConn->ErrLog(0)) {
			dcConn->LogStream() << "exception in sendNickList" << endl;
		}
		return -1;
	}
	return 0;
}

/** Get normal share size */
string NmdcProtocol::getNormalShare(__int64 iVal) {
	ostringstream os;
	float s = static_cast<float>(iVal);
	int i = 0;
	for (; ((s >= 1024) && (i < 7)); ++i) {
		s /= 1024;
	}
	os << ::std::floor(s * 1000 + 0.5) / 1000 << " " << DcServer::currentDcServer->mDCLang.mUnits[i];
	return os.str();
}



int NmdcProtocol::checkCommand(DcParser * dcParser, DcConn * dcConn) {

	// Checking length of command
	if (dcParser->getCommandLen() > mDcServer->mDcConfig.mMaxCmdLen[dcParser->mType]) {
		if (dcConn->Log(1)) {
			dcConn->LogStream() << "Bad CMD(" << dcParser->mType << ") length: " << dcParser->getCommandLen() << endl;
		}
		dcConn->closeNow(CLOSE_REASON_CMD_LENGTH);
		return -1;
	}

	// Checking null chars
	if (strlen(dcParser->mCommand.data()) < dcParser->mCommand.size()) {
		if (dcConn->Log(1)) {
			dcConn->LogStream() << "Sending null chars, probably attempt an attack" << endl;
		}
		dcConn->closeNow(CLOSE_REASON_CMD_NULL);
		return -2;
	}

	// Check Syntax
	if (dcParser->splitChunks()) {
		// Protection from commands, not belonging to DC protocol
		if (dcParser->mType != NMDC_TYPE_UNKNOWN || mDcServer->mDcConfig.mDisableNoDCCmd) {
			if (dcConn->Log(1)) {
				dcConn->LogStream() << "Wrong syntax in cmd: " << dcParser->mType << endl;
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



bool NmdcProtocol::antiflood(DcConn * dcConn, unsigned int iType) {
	if (mDcServer->antiFlood(dcConn->mTimes1.mCount[iType], dcConn->mTimes1.mTime[iType],
		mDcServer->mDcConfig.mFloodCount[iType], mDcServer->mDcConfig.mFloodTime[iType])
	) {
		#ifndef WITHOUT_PLUGINS
		if (!mDcServer->mCalls.mOnFlood.callAll(dcConn, FLOOD_TYPE_MYNIFO, 1))
		#endif
		{
			mDcServer->sendToUser(dcConn, (mDcServer->mDCLang.mFlood[iType]).c_str(), mDcServer->mDcConfig.mHubBot.c_str());
			dcConn->closeNice(9000, CLOSE_REASON_FLOOD);
			return true;
		}
	}
	if (mDcServer->antiFlood(dcConn->mTimes2.mCount[iType], dcConn->mTimes2.mTime[iType],
		mDcServer->mDcConfig.mFloodCount2[iType], mDcServer->mDcConfig.mFloodTime2[iType])
	) {
		#ifndef WITHOUT_PLUGINS
		if (!mDcServer->mCalls.mOnFlood.callAll(dcConn, FLOOD_TYPE_MYNIFO, 2))
		#endif
		{
			mDcServer->sendToUser(dcConn, (mDcServer->mDCLang.mFlood[iType]).c_str(), mDcServer->mDcConfig.mHubBot.c_str());
			dcConn->closeNice(9000, CLOSE_REASON_FLOOD);
			return true;
		}
	}
	return false;
}


bool NmdcProtocol::validateUser(DcConn * dcConn, const string & sNick) {

	/** Checking for bad symbols in nick */
	static string forbidedChars("$| ");
	if (sNick.npos != sNick.find_first_of(forbidedChars)) {
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Bad nick chars: '" << sNick << "'" << endl;
		}
		mDcServer->sendToUser(
			dcConn,
			mDcServer->mDCLang.mBadChars.c_str(),
			mDcServer->mDcConfig.mHubBot.c_str()
		);
		return false;
	}

	return true;
}



bool NmdcProtocol::checkNickLength(DcConn * dcConn, size_t iLen) {

	if (
		dcConn->miProfile == -1 && (
			iLen > mDcServer->mDcConfig.mMaxNickLen ||
			iLen < mDcServer->mDcConfig.mMinNickLen
		)
	) {

		string sMsg;

		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Bad nick len: " 
				<< iLen << " (" << dcConn->mDcUser->getNick() 
				<< ") [" << mDcServer->mDcConfig.mMinNickLen << ", " 
				<< mDcServer->mDcConfig.mMaxNickLen << "]" << endl;
		}

		stringReplace(mDcServer->mDCLang.mBadNickLen, string("min"), sMsg, (int) mDcServer->mDcConfig.mMinNickLen);
		stringReplace(sMsg, string("max"), sMsg, (int) mDcServer->mDcConfig.mMaxNickLen);

		mDcServer->sendToUser(dcConn, sMsg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());

		return false;
	}
	return true;
}



void NmdcProtocol::addToOps(DcUser * dcUser) {
	if (!dcUser->mInOpList) {
		dcUser->mInOpList = true;
		if (dcUser->getInUserList()) {
			string sMsg;
			mDcServer->mOpList.addWithNick(dcUser->getNick(), dcUser);
			if (dcUser->mHide) {
				dcUser->send(appendOpList(sMsg, dcUser->getNick()), false, true);
			} else {
				mDcServer->mDcUserList.sendToAll(appendOpList(sMsg, dcUser->getNick()), true/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
				mDcServer->mEnterList.sendToAll(sMsg, true/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
			}
		}
	}
}



void NmdcProtocol::delFromOps(DcUser * dcUser) {
	if (dcUser->mInOpList) {
		dcUser->mInOpList = false;
		if (dcUser->getInUserList()) {
			string sMsg1, sMsg2, sMsg3;
			mDcServer->mOpList.removeByNick(dcUser->getNick());
			if (dcUser->mHide) {
				if (dcUser->mDcConn == NULL) {
					return;
				}
				string s = dcUser->getMyINFO();
				dcUser->send(appendQuit(sMsg1, dcUser->getNick()), false, false);
				if (dcUser->mDcConn->mFeatures & SUPPORT_FEATURE_NOHELLO) {
					dcUser->send(s, true, false);
				} else if (dcUser->mDcConn->mFeatures & SUPPORT_FEATUER_NOGETINFO) {
					dcUser->send(appendHello(sMsg2, dcUser->getNick()), false, false);
					dcUser->send(s, true, false);
				} else {
					dcUser->send(appendHello(sMsg2, dcUser->getNick()), false, false);
				}

				if ((dcUser->mDcConn->mFeatures & SUPPORT_FEATUER_USERIP2) || dcUser->mInIpList) {
					dcUser->send(appendUserIp(sMsg3, dcUser->getNick(), dcUser->getIp()));
				} else {
					s.clear();
					dcUser->send(s);
				}
			} else {
				mDcServer->mDcUserList.sendToAll(appendQuit(sMsg1, dcUser->getNick()), true/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
				mDcServer->mEnterList.sendToAll(sMsg1, true/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
				mDcServer->mHelloList.sendToAll(appendHello(sMsg2, dcUser->getNick()), true/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
				mDcServer->mDcUserList.sendToAll(dcUser->getMyINFO(), true/*mDcServer->mDcConfig.mDelayedMyinfo*/);
				mDcServer->mEnterList.sendToAll(dcUser->getMyINFO(), true/*mDcServer->mDcConfig.mDelayedMyinfo*/);
				mDcServer->mIpList.sendToAll(appendUserIp(sMsg3, dcUser->getNick(), dcUser->getIp()), true, false);
			}
		}
	}
}



void NmdcProtocol::addToIpList(DcUser * dcUser) {
	if (!dcUser->mInIpList) {
		dcUser->mInIpList = true;
		if (dcUser->getInUserList()) {
			mDcServer->mIpList.addWithNick(dcUser->getNick(), dcUser);
			dcUser->send(mDcServer->mDcUserList.getIpList(), true);
		}
	}
}



void NmdcProtocol::delFromIpList(DcUser * dcUser) {
	if (dcUser->mInIpList) {
		dcUser->mInIpList = false;
		if (dcUser->getInUserList()) {
			mDcServer->mIpList.removeByNick(dcUser->getNick());
		}
	}
}



void NmdcProtocol::addToHide(DcUser * dcUser) {
	if (!dcUser->mHide) {
		dcUser->mHide = true;
		if (dcUser->isCanSend()) {
			string sMsg;
			dcUser->setCanSend(false);
			mDcServer->mDcUserList.sendToAll(appendQuit(sMsg, dcUser->getNick()), false/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
			mDcServer->mEnterList.sendToAll(sMsg, false/*mDcServer->mDcConfig.mDelayedMyinfo*/, false); // false cache
			dcUser->setCanSend(true);
			mDcServer->mOpList.remake();
			mDcServer->mDcUserList.remake();
		}
	}
}



void NmdcProtocol::delFromHide(DcUser * dcUser) {
	if (dcUser->mHide) {
		dcUser->mHide = false;
		if (dcUser->isCanSend()) {
			string sMsg1, sMsg2, sMsg3;
			if (dcUser->mInOpList) {
				dcUser->setCanSend(false);
				mDcServer->mHelloList.sendToAll(appendHello(sMsg1, dcUser->getNick()), false/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
				sMsg2 = string(dcUser->getMyINFO()).append(NMDC_SEPARATOR);
				mDcServer->mDcUserList.sendToAll(appendOpList(sMsg2, dcUser->getNick()), false/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
				mDcServer->mEnterList.sendToAll(sMsg2, false/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
				mDcServer->mIpList.sendToAll(appendUserIp(sMsg3, dcUser->getNick(), dcUser->getIp()), false, false);
				dcUser->setCanSend(true);
			} else {
				dcUser->setCanSend(false);
				mDcServer->mHelloList.sendToAll(appendHello(sMsg1, dcUser->getNick()), false/*mDcServer->mDcConfig.mDelayedMyinfo*/, false);
				mDcServer->mDcUserList.sendToAll(dcUser->getMyINFO(), false/*mDcServer->mDcConfig.mDelayedMyinfo*/);
				mDcServer->mEnterList.sendToAll(dcUser->getMyINFO(), false/*mDcServer->mDcConfig.mDelayedMyinfo*/);
				mDcServer->mIpList.sendToAll(appendUserIp(sMsg3, dcUser->getNick(), dcUser->getIp()), false, false);
				dcUser->setCanSend(true);
			}
			mDcServer->mOpList.remake();
			mDcServer->mDcUserList.remake();
		}
	}
}

string NmdcProtocol::getSeparator() {
	return NMDC_SEPARATOR;
}

unsigned long NmdcProtocol::getMaxCommandLength() {
	return mDcServer->mDcConfig.mMaxNmdcCommandLength;
}

bool NmdcProtocol::badFlag(DcConn * dcConn, const char * cmd, unsigned int flag) {
	if (dcConn->getLoginStatusFlag(flag)) {
		if (dcConn->Log(1)) {
			dcConn->LogStream() << "Attempt to attack in " << cmd << " (repeated sending)" << endl;
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
