/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
 * Copyright (C) 2009-2013 by Setuper
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

using namespace ::utils;

namespace dcserver {

namespace protocol {


NmdcProtocol::NmdcProtocol() : 
	DcProtocol()
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



/// Protocol type
int NmdcProtocol::getType() const {
	return DC_PROTOCOL_TYPE_NMDC;
}



const char * NmdcProtocol::getSeparator() const {
	return NMDC_SEPARATOR;
}



size_t NmdcProtocol::getSeparatorLen() const {
	return NMDC_SEPARATOR_LEN;
}



unsigned int NmdcProtocol::getMaxCommandLength() const {
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
		} else
	#endif
	{
		// Send First Message
		bool flush = false;
		mDcServer->sendToUser(dcConn->mDcUser, getFirstMsg(flush), mDcServer->mDcConfig.mHubBot.c_str());
	}
	dcConn->setLoginTimeOut(mDcServer->mDcConfig.mTimeoutLogon, mDcServer->mTime); // Timeout for enter
	dcConn->flush();
	return 0;
}



Conn * NmdcProtocol::getConnForUdpData(Conn * conn, Parser * parser) {

	// only SR command
	if (parser->mType == NMDC_TYPE_SR) {
		parser->mType = NMDC_TYPE_SR_UDP; // Set type for parse

		NmdcParser * dcParser = static_cast<NmdcParser *> (parser);
		if (!dcParser->splitChunks()) {

			// UDP SR PROTOCOL NMDC SPEC
			DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(dcParser->chunkString(CHUNK_SR_FROM)));
			if (dcUser && dcUser->mDcConn && conn->getIpUdp() == dcUser->getIp()) {
				return dcUser->mDcConn;
			} else {
				LOG_CLASS(conn, LEVEL_DEBUG, "Not found user for UDP data");
			}
		} else {
			LOG_CLASS(conn, LEVEL_DEBUG, "Bad UDP cmd syntax");
		}
	} else {
		LOG_CLASS(conn, LEVEL_TRACE, "Unknown UDP data");
	}
	return NULL;
}



void NmdcProtocol::onFlush(Conn * conn) {
	DcConn * dcConn = static_cast<DcConn *> (conn);
	if (dcConn->mNickListInProgress) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Enter after nicklist");
		dcConn->mNickListInProgress = false;
		dcConn->setState(STATE_NICKLST);
		mDcServer->doUserEnter(dcConn);
	}
}



int NmdcProtocol::doCommand(Parser * parser, Conn * conn) {

	NmdcParser * nmdcParser = static_cast<NmdcParser *> (parser);
	DcConn * dcConn = static_cast<DcConn *> (conn);

	LOG(LEVEL_TRACE, "doCommand");

	if (checkCommand(nmdcParser, dcConn) < 0) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnAny.callAll(dcConn->mDcUser, nmdcParser->mType)) {
			return 1;
		}
	#endif

	#if defined(_DEBUG) || defined (DEBUG)
		if (nmdcParser->mType == TYPE_UNPARSED) {
			throw "Unparsed command";
		}
	#endif

	LOG_CLASS(dcConn, LEVEL_TRACE, "[S]Stage " << nmdcParser->mType);

	(this->*(this->events[nmdcParser->mType])) (nmdcParser, dcConn);

	LOG_CLASS(dcConn, LEVEL_TRACE, "[E]Stage " << nmdcParser->mType);
	return 0;
}



int NmdcProtocol::eventSupports(NmdcParser * dcparser, DcConn * dcConn) {

	string feature;
	size_t posNext, posPrev = 10;
	dcConn->mFeatures = 0;
	dcparser->mCommand += ' ';
	while((posNext = dcparser->mCommand.find(' ', posPrev)) != string::npos) {
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
		} else if (feature == "ZPipe0" || feature == "ZPipe") {
			dcConn->mFeatures |= SUPPORT_FEATUER_ZPIPE;
		} else if (feature == "QuickList") {
			dcConn->mFeatures |= SUPPORT_FEATUER_QUICKLIST;
		}
	}

	if (dcparser->mCommand.size() > 10) {
		dcConn->mDcUser->getParamForce(USER_PARAM_SUPPORTS)->setString(dcparser->mCommand.substr(10));
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnSupports.callAll(dcConn->mDcUser)) {
			return -3;
		}
	#endif

	dcConn->send(
		"$Supports UserCommand NoGetINFO NoHello UserIP UserIP2 MCTo",
		59,
		true,
		false
	);

	return 0;
}



int NmdcProtocol::eventKey(NmdcParser * dcparser, DcConn * dcConn) {

	if (!checkState(dcConn, "Key", STATE_PROTOCOL)) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnKey.callAll(dcConn->mDcUser)) {
			string lock, key;
			appendLock(lock);
			size_t pos = lock.find(' ', 6);
			if (pos != string::npos && pos > 6) {
				lock2key(lock.assign(lock, 6, pos - 6), key);
			}
			
			const string & userKey = dcparser->chunkString(CHUNK_1_PARAM);
			if (key != userKey) {
				LOG_CLASS(dcConn, LEVEL_DEBUG, "nmdcKey: " << key);
				mDcServer->sendToUser(dcConn->mDcUser, mDcServer->mDcLang.mBadNmdcKey, mDcServer->mDcConfig.mHubBot.c_str());
				dcConn->closeNice(9000, CLOSE_REASON_CMD_KEY_INVALID);
				return -2;
			}
		}
	#endif

	dcConn->setState(STATE_PROTOCOL);
	return 0;
}



int NmdcProtocol::eventValidateNick(NmdcParser * dcparser, DcConn * dcConn) {

	if (!checkState(dcConn, "ValidateNick", STATE_VALNICK)) {
		return -1;
	}

	const string & nick = dcparser->chunkString(CHUNK_1_PARAM);
	size_t iNickLen = nick.size();

	// Additional checking the nick length
	if (iNickLen > 0xFF) {
		LOG_CLASS(dcConn, LEVEL_WARN, "Attempt to attack by long nick");
		dcConn->closeNow(CLOSE_REASON_NICK_LONG);
		return -1;
	}

	LOG_CLASS(dcConn, LEVEL_DEBUG, "User " << nick << " to validate nick");

	// Set nick
	dcConn->mDcUser->setNick(nick);
	

	// Checking validate user
	if (!validateUser(dcConn, nick)) {
		dcConn->closeNice(9000, CLOSE_REASON_USER_INVALID);
		return -2;
	}

	// Global user's limit
	if (mDcServer->mDcConfig.mUsersLimit >= 0 && mDcServer->getUsersCount() >= mDcServer->mDcConfig.mUsersLimit) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "User " << nick << " was disconnected (user's limit: " << mDcServer->mDcConfig.mUsersLimit << ")");
		mDcServer->sendToUser(dcConn->mDcUser, mDcServer->mDcLang.mUsersLimit, mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_USERS_LIMIT);
		return -3;
	}

	dcConn->setState(STATE_VALNICK | STATE_NICKLST); // We Install NICKLST because user can not call user list

	string msg;

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnValidateNick.callAll(dcConn->mDcUser)) {
			dcConn->send(appendGetPass(msg)); // We are sending the query for reception of the password
			return -4;
		}
	#endif

	if (!iNickLen || !checkNickLength(dcConn, iNickLen)) {
		dcConn->closeNice(9000, CLOSE_REASON_NICK_LEN);
	}
	dcConn->setState(STATE_VERIFY); // Does not need password

	dcConn->send(appendHello(msg, dcConn->mDcUser->getNick())); // Protection from change the command
	return 0;
}



int NmdcProtocol::eventMyPass(NmdcParser *, DcConn * dcConn) {

	if (dcConn->mDcUser->getNick().empty()) { // Check of existence of the user for current connection
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Mypass before validatenick");
		mDcServer->sendToUser(dcConn->mDcUser, mDcServer->mDcLang.mBadLoginSequence, mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_CMD_PASSWORD_ERR);
		return -1;
	}
	if (!checkState(dcConn, "MyPass", STATE_VERIFY)) {
		return -1;
	}

	// Checking the accepted password, otherwise send $BadPass|
	// or $Hello Nick|$LogedIn Nick|
	int bOp = 0;
	#ifndef WITHOUT_PLUGINS
		bOp = (mDcServer->mCalls.mOnMyPass.callAll(dcConn->mDcUser));
	#endif

	string msg;
	dcConn->setState(STATE_VERIFY); // Password is accepted
	appendHello(msg, dcConn->mDcUser->getNick());
	if (bOp) { // If entered operator, that sends command LoggedIn ($LogedIn !)
		msg.append(STR_LEN("$LogedIn "));
		msg.append(dcConn->mDcUser->getNick());
		msg.append(STR_LEN(NMDC_SEPARATOR));
	}
	dcConn->send(msg);
	return 0;
}



int NmdcProtocol::eventVersion(NmdcParser * dcparser, DcConn * dcConn) {

	if (!checkState(dcConn, "Version", STATE_VERSION)) {
		return -1;
	}

	string & version = dcparser->chunkString(CHUNK_1_PARAM);
	LOG_CLASS(dcConn, LEVEL_DEBUG, "Version:" << version);

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnVersion.callAll(dcConn->mDcUser) && version != "1,0091" && version != "1.0091") {
			dcConn->closeNice(9000, CLOSE_REASON_CMD_VERSION); /** Checking of the version */
		}
	#endif

	dcConn->setState(STATE_VERSION); /** Version was checked */
	dcConn->mDcUser->getParamForce(USER_PARAM_VERSION)->setString(version);
	return 0;
}



int NmdcProtocol::eventGetNickList(NmdcParser *, DcConn * dcConn) {

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnGetNickList.callAll(dcConn->mDcUser)) {
			return 3;
		}
	#endif

	if (!dcConn->isState(STATE_INFO) && mDcServer->mDcConfig.mNicklistOnLogin) {
		if (mDcServer->mDcConfig.mDelayedLogin) {
			unsigned int state = dcConn->getState();
			if (state & STATE_NICKLST) {
				state -= STATE_NICKLST;
			}
			dcConn->resetState(state);
		}
		dcConn->mSendNickList = true;
		return 0;
	}
	return sendNickList(dcConn);
}



int NmdcProtocol::eventMyInfo(NmdcParser * dcparser, DcConn * dcConn) {

	const string & nick = dcparser->chunkString(CHUNK_MI_NICK);

	// Check existence user, otherwise check support QuickList
	if (nick.empty()) {
		//if (QuickList)
		//	dcConn->mDcUser->setNick(nick);
		//} else
		{
			LOG_CLASS(dcConn, LEVEL_DEBUG, "Myinfo without user: " << dcparser->mCommand);
			mDcServer->sendToUser(dcConn->mDcUser, mDcServer->mDcLang.mBadLoginSequence, mDcServer->mDcConfig.mHubBot.c_str());
			dcConn->closeNice(9000, CLOSE_REASON_CMD_MYINFO_WITHOUT_USER);
			return -2;
		}
	} else if (nick != dcConn->mDcUser->getNick()) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Bad nick in MyINFO, closing");
		mDcServer->sendToUser(dcConn->mDcUser, mDcServer->mDcLang.mBadMyinfoNick, mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_MYINFO);
		return -1;
	}

	string oldMyInfo = dcConn->mDcUser->getInfo();
	dcConn->mDcUser->setInfo(dcparser->mCommand);

	int mode = 0;
	#ifndef WITHOUT_PLUGINS
		mode = mDcServer->mCalls.mOnMyINFO.callAll(dcConn->mDcUser);
	#endif

	if (mode != 1 && dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		if (oldMyInfo != dcConn->mDcUser->getInfo()) {
			if (dcConn->mDcUser->isTrueBoolParam(USER_PARAM_CAN_HIDE)) {
				dcConn->send(dcConn->mDcUser->getInfo(), true); // Send to self only
			} else {
				sendMode(dcConn, dcConn->mDcUser->getInfo(), mode, mDcServer->mDcUserList, false); // Use cache for send to all
			}
		}
	} else if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		dcConn->setState(STATE_INFO);

		unsigned iWantedMask = STATE_NORMAL;
		if (mDcServer->mDcConfig.mDelayedLogin && dcConn->mSendNickList) {
			iWantedMask = STATE_NORMAL - STATE_NICKLST;
		}
		if (!dcConn->isState(iWantedMask)) {
			LOG_CLASS(dcConn, LEVEL_DEBUG, "Invalid sequence of the sent commands (" 
					<< dcConn->getState() << "), wanted: " << iWantedMask);
			dcConn->closeNow(CLOSE_REASON_CMD_SEQUENCE);
			return -1;
		}

		if (!mDcServer->beforeUserEnter(dcConn)) {
			return -1;
		}
	}
	return 0;
}



int NmdcProtocol::eventChat(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		return -1;
	}

	// Check chat nick
	if ((dcparser->chunkString(CHUNK_CH_NICK) != dcConn->mDcUser->getNick()) ) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Bad nick in chat, closing");
		string msg;
		stringReplace(mDcServer->mDcLang.mBadChatNick, string(STR_LEN("nick")), msg, dcparser->chunkString(CHUNK_CH_NICK));
		stringReplace(msg, string(STR_LEN("real_nick")), msg, dcConn->mDcUser->getNick());
		mDcServer->sendToUser(dcConn->mDcUser, msg, mDcServer->mDcConfig.mHubBot.c_str());
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

	// Sending message
	sendMode(dcConn, dcparser->mCommand, mode, mDcServer->mChatList, true); // Don't use cache for send to all
	return 0;
}



int NmdcProtocol::eventTo(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		return -1;
	}

	const string & nick = dcparser->chunkString(CHUNK_PM_TO);

	// Checking the coincidence nicks in command
	if (dcparser->chunkString(CHUNK_PM_FROM) != dcConn->mDcUser->getNick() || dcparser->chunkString(CHUNK_PM_NICK) != dcConn->mDcUser->getNick()) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Bad nick in PM, closing");
		dcConn->closeNow(CLOSE_REASON_NICK_PM);
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnTo.callAll(dcConn->mDcUser)) {
			return 0;
		}
	#endif

	// Check TO user (PROTOCOL NMDC SPEC)
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(nick));
	if (!dcUser) {
		return -2;
	}

	dcUser->send(dcparser->mCommand, true);
	return 0;
}



int NmdcProtocol::eventMcTo(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		return -1;
	}

	const string & nick = dcparser->chunkString(CHUNK_MC_TO);

	// Checking the coincidence nicks in command
	if (dcparser->chunkString(CHUNK_MC_FROM) != dcConn->mDcUser->getNick()) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Bad nick in MCTo, closing");
		dcConn->closeNow(CLOSE_REASON_NICK_MCTO);
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnMCTo.callAll(dcConn->mDcUser)) {
			return 0;
		}
	#endif

	// Check MCTO user (PROTOCOL NMDC SPEC)
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(nick));
	if (!dcUser) {
		return -2;
	}

	const string & msg = dcparser->chunkString(CHUNK_MC_MSG);
	dcUser->sendToChat(msg, dcConn->mDcUser->getNick());
	if (dcConn->mDcUser->getNick() != nick) {
		dcConn->mDcUser->sendToChat(msg, dcConn->mDcUser->getNick());
	}
	return 0;
}



int NmdcProtocol::eventUserIp(NmdcParser * dcParser, DcConn * dcConn) {

	if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		return -1;
	}

	if (!(dcConn->mFeatures & (SUPPORT_FEATUER_USERIP | SUPPORT_FEATUER_USERIP2))) {
		return -2;
	}

	const string & param = dcParser->chunkString(CHUNK_1_PARAM);
	string nick, result(STR_LEN("$UserIP "));

	size_t pos = param.find("$$");
	size_t cur = 0;
	while (pos != string::npos) {
		nick.assign(param, cur, pos - cur);
		if (nick.size()) {
			// UserIP PROTOCOL NMDC SPEC
			UserBase * userBase = mDcServer->mDcUserList.getUserBaseByNick(nick);
			if (userBase != NULL) {
				result.append(nick).append(STR_LEN(" ")).append(userBase->getIp()).append(STR_LEN("$$"));
			}
		}
		cur = pos + 2;
		pos = param.find("$$", pos + 2);
	}

	// last param
	nick.assign(param, cur, param.size() - cur);
	if (nick.size()) {
		// UserIP PROTOCOL NMDC SPEC
		UserBase * userBase = mDcServer->mDcUserList.getUserBaseByNick(nick);
		if (userBase != NULL) {
			result.append(nick).append(STR_LEN(" ")).append(userBase->getIp());
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

	if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		return -1;
	}

	if (mDcServer->getSystemLoad() == SYSTEM_LOAD_SYSTEM_DOWN) {
		LOG_CLASS(dcConn, LEVEL_WARN, "System down, search is impossible");
		return -2;
	}

	int mode = 0;
	#ifndef WITHOUT_PLUGINS
		mode = mDcServer->mCalls.mOnSearch.callAll(dcConn->mDcUser);
	#endif

	// Sending cmd
	string msg;

	switch (dcparser->mType) {

		case NMDC_TYPE_SEARCH :
			if (mDcServer->mDcConfig.mCheckSearchIp && dcConn->getIp() != dcparser->chunkString(CHUNK_AS_IP)) {
				LOG_CLASS(dcConn, LEVEL_DEBUG, "Bad ip in active search, closing");
				stringReplace(mDcServer->mDcLang.mBadSearchIp, string(STR_LEN("ip")), msg, dcparser->chunkString(CHUNK_AS_IP));
				stringReplace(msg, string(STR_LEN("real_ip")), msg, dcConn->getIp());
				mDcServer->sendToUser(dcConn->mDcUser, msg, mDcServer->mDcConfig.mHubBot.c_str());
				dcConn->closeNice(9000, CLOSE_REASON_NICK_SEARCH);
				return -1;
			}
			sendMode(dcConn, dcparser->mCommand, mode, mDcServer->mDcUserList, false); // Use cache for send to all
			break;

		case NMDC_TYPE_SEARCH_PAS :
			dcConn->emptySrCounter(); /** Zeroizing result counter of the passive search */
			sendMode(dcConn, dcparser->mCommand, mode, mDcServer->mActiveList, false); // Use cache for send to all
			break;

		case NMDC_TYPE_MSEARCH :
			if (mDcServer->mDcConfig.mCheckSearchIp && (dcConn->getIp() != dcparser->chunkString(CHUNK_AS_IP))) {
				LOG_CLASS(dcConn, LEVEL_DEBUG, "Bad ip in active search, closing");
				stringReplace(mDcServer->mDcLang.mBadSearchIp, string(STR_LEN("ip")), msg, dcparser->chunkString(CHUNK_AS_IP));
				stringReplace(msg, string(STR_LEN("real_ip")), msg, dcConn->getIp());
				mDcServer->sendToUser(dcConn->mDcUser, msg, mDcServer->mDcConfig.mHubBot.c_str());
				dcConn->closeNice(9000, CLOSE_REASON_NICK_SEARCH);
				return -1;
			}
			msg.reserve(9 + dcparser->chunkString(CHUNK_AS_ADDR).size() + dcparser->chunkString(CHUNK_AS_QUERY).size());
			msg.assign(STR_LEN("$Search "));
			msg.append(dcparser->chunkString(CHUNK_AS_ADDR));
			msg.append(STR_LEN(" "));
			msg.append(dcparser->chunkString(CHUNK_AS_QUERY));
			sendMode(dcConn, msg, mode, mDcServer->mDcUserList, false); // Use cache for send to all
			break;

		case NMDC_TYPE_MSEARCH_PAS :
			dcConn->emptySrCounter(); /** Zeroizing result counter of the passive search */
			sendMode(dcConn, dcparser->mCommand, mode, mDcServer->mActiveList, false); // Use cache for send to all
			break;

		default :
			return -4;

	}
	return 0;
}



int NmdcProtocol::eventSr(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		return -1;
	}

	// Check same nick in cmd (PROTOCOL NMDC SPEC)
	if (mDcServer->mDcConfig.mCheckSrNick && (dcConn->mDcUser->getNick() != dcparser->chunkString(CHUNK_SR_FROM))) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Bad nick in search response, closing");
		string msg;
		stringReplace(mDcServer->mDcLang.mBadSrNick, string(STR_LEN("nick")), msg, dcparser->chunkString(CHUNK_SR_FROM));
		stringReplace(msg, string(STR_LEN("real_nick")), msg, dcConn->mDcUser->getNick());
		mDcServer->sendToUser(dcConn->mDcUser, msg, mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_SR);
		return -1;
	}

	DcUser * dcUser = NULL;
	const string & nick = dcparser->chunkString(CHUNK_SR_TO);
	if (nick.size()) {
		dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(nick));

		// Is user exist?
		if (!dcUser || !dcUser->mDcConn) {
			return -2;
		}
	}

	// != 0 - error
	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnSR.callAll(dcConn->mDcUser)) {
			return -3;
		}
	#endif

	// Sending cmd
	if (dcUser && (!mDcServer->mDcConfig.mMaxPassiveRes ||
		(dcUser->mDcConn->getSrCounter() <= unsigned(mDcServer->mDcConfig.mMaxPassiveRes))
	)) {
		dcUser->mDcConn->increaseSrCounter();
		string str(dcparser->mCommand, 0, dcparser->getStartChunk(CHUNK_SR_TO) - 1); /** Remove nick on the end of cmd */
		dcUser->mDcConn->send(str, true, false);
	}
	return 0;
}



int NmdcProtocol::eventConnectToMe(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		return -1;
	}

	if (mDcServer->mDcConfig.mCheckCtmIp && dcConn->getIp() != dcparser->chunkString(CHUNK_CM_IP)) {
		string msg;
		stringReplace(mDcServer->mDcLang.mBadCtmIp, string(STR_LEN("ip")), msg, dcparser->chunkString(CHUNK_CM_IP));
		stringReplace(msg, string(STR_LEN("real_ip")), msg, dcConn->getIp());
		mDcServer->sendToUser(dcConn->mDcUser, msg, mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_CTM);
		return -1;
	}

	// Check CTM user PROTOCOL NMDC SPEC
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(dcparser->chunkString(CHUNK_CM_NICK)));
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

	if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		return -1;
	}

	// Check RCTM nick (PROTOCOL NMDC SPEC)
	if (mDcServer->mDcConfig.mCheckRctmNick && (dcparser->chunkString(CHUNK_RC_NICK) != dcConn->mDcUser->getNick())) {
		string msg;
		stringReplace(mDcServer->mDcLang.mBadRevConNick, string(STR_LEN("nick")), msg, dcparser->chunkString(CHUNK_RC_NICK));
		stringReplace(msg, string(STR_LEN("real_nick")), msg, dcConn->mDcUser->getNick());
		mDcServer->sendToUser(dcConn->mDcUser, msg, mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_RCTM);
		return -1;
	}

	// Check RCTM user (PROTOCOL NMDC)
	DcUser * other = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(dcparser->chunkString(CHUNK_RC_OTHER)));
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

	if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		return -1;
	}

	return 0;
}



int NmdcProtocol::eventKick(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnKick.callAll(dcConn->mDcUser)) {
			return -1;
		}
	#endif

	if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_CAN_KICK)) {
		return -2;
	}

	// Check kick user PROTOCOL NMDC SPEC
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(dcparser->chunkString(CHUNK_1_PARAM)));

	// Is user exist?
	if (!dcUser || !dcUser->mDcConn) {
		return -3;
	}

	dcUser->mDcConn->closeNice(9000, CLOSE_REASON_CMD_KICK);
	return 0;
}



int NmdcProtocol::eventOpForceMove(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnOpForceMove.callAll(dcConn->mDcUser)) {
			return -1;
		}
	#endif

	if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_CAN_REDIRECT)) {
		return -2;
	}

	// Check redirect user PROTOCOL NMDC SPEC
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(dcparser->chunkString(CHUNK_FM_NICK)));

	// Is user exist?
	if (!dcUser || !dcUser->mDcConn || dcparser->chunkString(CHUNK_FM_DEST).empty()) {
		return -3;
	}

	mDcServer->forceMove(dcUser, dcparser->chunkString(CHUNK_FM_DEST).c_str(), dcparser->chunkString(CHUNK_FM_REASON).c_str());
	return 0;
}



int NmdcProtocol::eventGetInfo(NmdcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		return -1;
	}

	// Check GetINFO user PROTOCOL NMDC SPEC
	DcUser * dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(dcparser->chunkString(CHUNK_GI_OTHER)));
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
	if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_CAN_HIDE)) {
		dcConn->send(dcUser->getInfo(), true, false);
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
	str.append(STR_LEN("$Lock EXTENDEDPROTOCOL_" INTERNALNAME "_by_setuper_" INTERNALVERSION " Pk=" INTERNALNAME));
	return str.append(STR_LEN(NMDC_SEPARATOR));
}



// $Hello nick|
string & NmdcProtocol::appendHello(string & str, const string & nick) {
	str.reserve(str.size() + nick.size() + 8);
	return str.append(STR_LEN("$Hello ")).append(nick).append(STR_LEN(NMDC_SEPARATOR));
}



// $HubIsFull|
string & NmdcProtocol::appendHubIsFull(string & str) {
	return str.append(STR_LEN("$HubIsFull")).append(STR_LEN(NMDC_SEPARATOR));
}



// $GetPass|
string & NmdcProtocol::appendGetPass(string & str) {
	return str.append(STR_LEN("$GetPass")).append(STR_LEN(NMDC_SEPARATOR));
}



// $ValidateDenide nick|
string & NmdcProtocol::appendValidateDenied(string & str, const string & nick) {
	str.reserve(str.size() + nick.size() + 17);
	return str.append(STR_LEN("$ValidateDenide ")).append(nick).append(STR_LEN(NMDC_SEPARATOR));
}



// $HubName hubName - topic|
string & NmdcProtocol::appendHubName(string & str, const string & hubName, const string & topic) {
	if (topic.size()) {
		str.reserve(str.size() + hubName.size() + topic.size() + 13);
		return str.append(STR_LEN("$HubName ")).append(hubName).append(STR_LEN(" - ")).append(topic).append(STR_LEN(NMDC_SEPARATOR));
	} else {
		str.reserve(str.size() + hubName.size() + 10);
		return str.append(STR_LEN("$HubName ")).append(hubName).append(STR_LEN(NMDC_SEPARATOR));
	}
}



// $HubTopic hubTopic|
string & NmdcProtocol::appendHubTopic(string & str, const string & hubTopic) {
	str.reserve(str.size() + hubTopic.size() + 11);
	return str.append(STR_LEN("$HubTopic ")).append(hubTopic).append(STR_LEN(NMDC_SEPARATOR));
}



// $Quit nick|
string & NmdcProtocol::appendQuit(string & str, const string & nick) {
	str.reserve(str.size() + nick.size() + 7);
	return str.append(STR_LEN("$Quit ")).append(nick).append(STR_LEN(NMDC_SEPARATOR));
}



// $OpList nick$$|
string & NmdcProtocol::appendOpList(string & str, const string & nick) {
	str.reserve(str.size() + nick.size() + 11);
	return str.append(STR_LEN("$OpList ")).append(nick).append(STR_LEN("$$")).append(STR_LEN(NMDC_SEPARATOR));
}



// $UserIP nick ip$$|
string & NmdcProtocol::appendUserIp(string & str, const string & nick, const string & ip) {
	if (ip.size()) {
		str.reserve(str.size() + nick.size() + ip.size() + 12);
		str.append(STR_LEN("$UserIP ")).append(nick).append(STR_LEN(" ")).append(ip).append(STR_LEN("$$")).append(STR_LEN(NMDC_SEPARATOR));
	}
	return str;
}



// $ForceMove address|
string & NmdcProtocol::appendForceMove(string & str, const string & address) {
	str.reserve(address.size() + 12);
	return str.append(STR_LEN("$ForceMove ")).append(address).append(STR_LEN(NMDC_SEPARATOR));
}



// str|
string & NmdcProtocol::appendChat(string & str, const string & msg) {
	str.reserve(msg.size() + 1); // msg.size() + 1
	return str.append(msg).append(STR_LEN(NMDC_SEPARATOR));
}



// <nick> str|
string & NmdcProtocol::appendChat(string & str, const string & msg, const string & nick) {
	str.reserve(nick.size() + msg.size() + 4); // 1 + nick.size() + 2 + msg.size() + 1
	return str.append(STR_LEN("<")).append(nick).append(STR_LEN("> ")).append(msg).append(STR_LEN(NMDC_SEPARATOR));
}



// "$To: "
// " From: from $<nick> str|"
void NmdcProtocol::appendPm(string & start, string & end, const string & msg, const string & nick, const string & from) {
	start.append(STR_LEN("$To: "));
	end.reserve(from.size() + nick.size() + msg.size() + 13); // 7 + from.size() + 3 + nick.size() + 2 + msg.size() + 1
	end.append(STR_LEN(" From: ")).append(from).append(STR_LEN(" $<")).append(nick).append(STR_LEN("> "));
	end.append(msg).append(STR_LEN(NMDC_SEPARATOR));
}



// <nick> msg|
void NmdcProtocol::sendToChat(DcConn * dcConn, const string & data, const string & nick, bool flush /*= true*/) {
	dcConn->reserve(nick.size() + data.size() + 4); // 1 + nick.size() + 2 + data.size() + 1
	dcConn->send(STR_LEN("<"), false, false);
	dcConn->send(nick, false, false);
	dcConn->send(STR_LEN("> "), false, false);
	dcConn->send(data, true, flush);
}



// <nick> msg|
void NmdcProtocol::sendToChatAll(DcConn * dcConn, const string & data, const string & nick, bool flush /*= true*/) {
	sendToChat(dcConn, data, nick, flush);
}



// $To: to From: from $<nick> msg|
void NmdcProtocol::sendToPm(DcConn * dcConn, const string & data, const string & nick, const string & from, bool flush /*= true*/) {
	size_t len = from.size() + nick.size() + data.size() + 6; // from.size() + 3 + nick.size() + 2 + data.size() + 1
	const string userNick = dcConn->mDcUser->getNick();
	if (!userNick.empty()) {
		dcConn->reserve(len + 12 + userNick.size()); // 5 + userNick.size() + 7 + len
		dcConn->send(STR_LEN("$To: "), false, false);
		dcConn->send(userNick, false, false);
		dcConn->send(STR_LEN(" From: "), false, false);
	} else {
		dcConn->reserve(len + 21);
		dcConn->send(STR_LEN("$To: <unknown> From: "), false, false);
	}
	dcConn->send(from, false, false);
	dcConn->send(STR_LEN(" $<"), false, false);
	dcConn->send(nick, false, false);
	dcConn->send(STR_LEN("> "), false, false);
	dcConn->send(data, true, flush);
}



void NmdcProtocol::sendError(DcConn * dcConn, const string & errorText, int errorCode) {
	sendToChat(dcConn, errorText, mDcServer->mDcConfig.mHubBot.c_str(), true);
	if (errorCode == ERROR_CODE_NICK_INVALID) { // TODO: ADC error code
		string msg;
		dcConn->send(appendValidateDenied(msg, dcConn->mDcUser->getNick()));
	}
}



/// Action after add in user list
void NmdcProtocol::onAddInUserList(DcUser * dcUser) {
	unsigned long nickHash = dcUser->getNickHash();
	if (!(dcUser->mDcConn->mFeatures & SUPPORT_FEATURE_NOHELLO)) {
		mDcServer->mHelloList.add(nickHash, dcUser);
	}
	if (!dcUser->isPassive()) {
		mDcServer->mActiveList.add(nickHash, dcUser);
	}
	if (dcUser->isTrueBoolParam(USER_PARAM_IN_IP_LIST)) {
		mDcServer->mIpList.add(nickHash, dcUser);
	}

	if (dcUser->isTrueBoolParam(USER_PARAM_IN_OP_LIST)) {
		mDcServer->mOpList.add(nickHash, dcUser);
	}
}



void NmdcProtocol::sendMode(DcConn * dcConn, const string & str, int mode, UserList & userList, bool flush) {
	if (mode == 0) { // Send to all
		userList.sendToAll(str, true, flush);
	} else if (mode == 3) { // Send to all except current user
		if (dcConn->mDcUser->isCanSend()) {
			dcConn->mDcUser->setCanSend(false);
			userList.sendToAll(str, true, flush);
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
		userList.sendToAll(str, true, flush);
		for (vector<DcConn *>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it) {
			(*ul_it)->mDcUser->setCanSend(true);
		}
	}
}



void NmdcProtocol::forceMove(DcConn * dcConn, const char * address, const char * reason /*= NULL*/) {
	string msg, force;

	stringReplace(mDcServer->mDcLang.mForceMove, string(STR_LEN("address")), force, address);
	stringReplace(force, string(STR_LEN("reason")), force, reason != NULL ? reason : "");

	if (dcConn->mDcUser != NULL) {
		dcConn->mDcUser->sendToPm(force, mDcServer->mDcConfig.mHubBot, mDcServer->mDcConfig.mHubBot, false);
		dcConn->mDcUser->sendToChat(force, mDcServer->mDcConfig.mHubBot, false);
	}
	dcConn->send(appendForceMove(msg, address));
	dcConn->closeNice(9000, CLOSE_REASON_CMD_FORCE_MOVE);
}



/// Sending the user-list and op-list
int NmdcProtocol::sendNickList(DcConn * dcConn) {
	if (!dcConn->isState(STATE_NORMAL) && mDcServer->mDcConfig.mNicklistOnLogin) {
		dcConn->mNickListInProgress = true;
	}

	if (dcConn->mFeatures & SUPPORT_FEATURE_NOHELLO) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Sending MyINFO list");
		// seperator "|" was added in getInfoList function
		if (mDcServer->mDcConfig.mCompressionType == 1) {
			dcConn->sendZpipe(mDcServer->mDcUserList.getList(USER_LIST_MYINFO), false);
		} else {
			dcConn->send(mDcServer->mDcUserList.getList(USER_LIST_MYINFO), false, false);
		}
	} else if (dcConn->mFeatures & SUPPORT_FEATUER_NOGETINFO) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Sending MyINFO list and Nicklist");
		// seperator "|" was not added in getNickList function, because seperator was "$$"
		dcConn->send(mDcServer->mDcUserList.getList(USER_LIST_NICK), true, false);
		// seperator "|" was added in getInfoList function
		dcConn->send(mDcServer->mDcUserList.getList(USER_LIST_MYINFO), false, false);
	} else {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Sending Nicklist");
		// seperator "|" was not added in getNickList function, because seperator was "$$"
		dcConn->send(mDcServer->mDcUserList.getList(USER_LIST_NICK), true, false);
	}
	if (mDcServer->mOpList.size()) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Sending Oplist");
		// seperator "|" was not added in getNickList function, because seperator was "$$"
		dcConn->send(mDcServer->mOpList.getList(), true, false);
	}

	if (dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST) && dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_IP_LIST)) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Sending Iplist");
		// seperator "|" was not added in getIpList function, because seperator was "$$"
		dcConn->send(mDcServer->mDcUserList.getList(USER_LIST_IP), true, true);
	} else {
		dcConn->send("", 0 , true, true); // for flush (don't replace to flush function!)
	}
	return 0;
}



int NmdcProtocol::checkCommand(NmdcParser * nmdcParser, DcConn * dcConn) {

	// Checking length of command
	if (nmdcParser->getCommandLen() > mDcServer->mDcConfig.mMaxCmdLen[nmdcParser->mType]) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Bad CMD(" << nmdcParser->mType << ") length: " << nmdcParser->getCommandLen());
		dcConn->closeNow(CLOSE_REASON_CMD_LENGTH);
		return -1;
	}

	// Checking null chars
	if (nmdcParser->mCommand.find('\0') != string::npos) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Sending null chars, probably attempt an attack");
		dcConn->closeNow(CLOSE_REASON_CMD_NULL);
		return -2;
	}

	// Check Syntax
	if (nmdcParser->splitChunks()) {
		// Protection from commands, not belonging to DC protocol
		if (nmdcParser->mType != NMDC_TYPE_UNKNOWN || mDcServer->mDcConfig.mDisableNoDCCmd) {
			LOG_CLASS(dcConn, LEVEL_DEBUG, "Wrong syntax in cmd: " << nmdcParser->mType);
			dcConn->closeNice(9000, CLOSE_REASON_CMD_SYNTAX);
			return -3;
		}
	}

	// Check flood
	if (antiflood(dcConn, nmdcParser->mType)) {
		return -4;
	}

	return 0;
}



bool NmdcProtocol::antiflood(DcConn * dcConn, int type) {
	DcConn::Antiflood & antiflood = dcConn->mAntiflood[type];

	if (mDcServer->antiFlood(antiflood.mCount, antiflood.mTime,
		mDcServer->mDcConfig.mFloodCount[type], mDcServer->mDcConfig.mFloodTime[type])
	) {
		#ifndef WITHOUT_PLUGINS
		if (!mDcServer->mCalls.mOnFlood.callAll(dcConn->mDcUser, static_cast<int> (type), 1))
		#endif
		{
			mDcServer->sendToUser(dcConn->mDcUser, mDcServer->mDcLang.mFlood[type], mDcServer->mDcConfig.mHubBot.c_str());
			dcConn->closeNice(9000, CLOSE_REASON_FLOOD);
			return true;
		}
	}

	if (mDcServer->antiFlood(antiflood.mCount2, antiflood.mTime2,
		mDcServer->mDcConfig.mFloodCount2[type], mDcServer->mDcConfig.mFloodTime2[type])
	) {
		#ifndef WITHOUT_PLUGINS
		if (!mDcServer->mCalls.mOnFlood.callAll(dcConn->mDcUser, static_cast<int> (type), 2))
		#endif
		{
			mDcServer->sendToUser(dcConn->mDcUser, mDcServer->mDcLang.mFlood[type], mDcServer->mDcConfig.mHubBot.c_str());
			dcConn->closeNice(9000, CLOSE_REASON_FLOOD);
			return true;
		}
	}
	return false;
}



bool NmdcProtocol::validateUser(DcConn * dcConn, const string & nick) {

	// Checking for bad symbols in nick
	if (string::npos != nick.find_first_of("$| ")) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Bad nick chars: '" << nick << "'");
		mDcServer->sendToUser(
			dcConn->mDcUser,
			mDcServer->mDcLang.mBadChars,
			mDcServer->mDcConfig.mHubBot.c_str()
		);
		return false;
	}

	return true;
}



bool NmdcProtocol::checkNickLength(DcConn * dcConn, size_t len) {

	if (
		dcConn->mDcUser->getParamForce(USER_PARAM_PROFILE)->getInt() == -1 && (
			len > mDcServer->mDcConfig.mMaxNickLen ||
			len < mDcServer->mDcConfig.mMinNickLen
		)
	) {

		string msg;

		LOG_CLASS(dcConn, LEVEL_DEBUG, "Bad nick len: " 
			<< len << " (" << dcConn->mDcUser->getNick() 
			<< ") [" << mDcServer->mDcConfig.mMinNickLen << ", " 
			<< mDcServer->mDcConfig.mMaxNickLen << "]");

		stringReplace(mDcServer->mDcLang.mBadNickLen, string(STR_LEN("min")), msg, static_cast<int> (mDcServer->mDcConfig.mMinNickLen));
		stringReplace(msg, string(STR_LEN("max")), msg, static_cast<int> (mDcServer->mDcConfig.mMaxNickLen));

		mDcServer->sendToUser(dcConn->mDcUser, msg, mDcServer->mDcConfig.mHubBot.c_str());

		return false;
	}
	return true;
}



void NmdcProtocol::nickList(string & list, UserBase * userBase) {
	// $NickList nick1$$nick2$$
	// $OpList nick1$$nick2$$
	if (!userBase->isHide() && !userBase->getNick().empty()) {
		list.append(userBase->getNick());
		list.append(STR_LEN("$$"));
	}
}



void NmdcProtocol::myInfoList(string & list, UserBase * userBase) {
	// $MyINFO nick1 ...|$MyINFO nick2 ...|
	if (!userBase->isHide()) {
		list.append(userBase->getInfo());
		list.append(STR_LEN(NMDC_SEPARATOR));
	}
}



void NmdcProtocol::ipList(string & list, UserBase * userBase) {
	// $UserIP nick1 ip1$$nick2 ip2$$
	if (!userBase->isHide() && userBase->getIp().size() && !userBase->getNick().empty()) {
		list.append(userBase->getNick());
		list.append(STR_LEN(" "));
		list.append(userBase->getIp());
		list.append(STR_LEN("$$"));
	}
}



void NmdcProtocol::addToOps(DcUser * dcUser) {
	if (dcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		string msg;
		mDcServer->mOpList.add(dcUser->getNickHash(), dcUser);
		if (dcUser->isTrueBoolParam(USER_PARAM_CAN_HIDE)) {
			dcUser->send(appendOpList(msg, dcUser->getNick()), false, true);
		} else {
			mDcServer->mDcUserList.sendToAll(appendOpList(msg, dcUser->getNick()), false, false/*mDcServer->mDcConfig.mDelayedMyinfo*/);
			mDcServer->mEnterList.sendToAll(msg, false, false/*mDcServer->mDcConfig.mDelayedMyinfo*/);
		}
	}
}



void NmdcProtocol::delFromOps(DcUser * dcUser) {
	if (dcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		string sMsg1, sMsg2, sMsg3;
		mDcServer->mOpList.remove(dcUser->getNickHash());
		if (dcUser->isTrueBoolParam(USER_PARAM_CAN_HIDE)) {
			if (dcUser->mDcConn == NULL) {
				return;
			}
			dcUser->send(appendQuit(sMsg1, dcUser->getNick()), false, false);
			if (dcUser->mDcConn->mFeatures & SUPPORT_FEATURE_NOHELLO) {
				dcUser->send(dcUser->getInfo(), true, false);
			} else if (dcUser->mDcConn->mFeatures & SUPPORT_FEATUER_NOGETINFO) {
				dcUser->send(appendHello(sMsg2, dcUser->getNick()), false, false);
				dcUser->send(dcUser->getInfo(), true, false);
			} else {
				dcUser->send(appendHello(sMsg2, dcUser->getNick()), false, false);
			}

			if ((dcUser->mDcConn->mFeatures & SUPPORT_FEATUER_USERIP2) || dcUser->isTrueBoolParam(USER_PARAM_IN_IP_LIST)) {
				dcUser->send(appendUserIp(sMsg3, dcUser->getNick(), dcUser->getIp()));
			} else {
				dcUser->send("", 0, false, true);
			}
		} else {
			mDcServer->mDcUserList.sendToAll(appendQuit(sMsg1, dcUser->getNick()), false, false/*mDcServer->mDcConfig.mDelayedMyinfo*/);
			mDcServer->mEnterList.sendToAll(sMsg1, false, false/*mDcServer->mDcConfig.mDelayedMyinfo*/);
			mDcServer->mHelloList.sendToAll(appendHello(sMsg2, dcUser->getNick()), false, false/*mDcServer->mDcConfig.mDelayedMyinfo*/);
			mDcServer->mDcUserList.sendToAll(dcUser->getInfo(), true, false/*mDcServer->mDcConfig.mDelayedMyinfo*/);
			mDcServer->mEnterList.sendToAll(dcUser->getInfo(), true, false/*mDcServer->mDcConfig.mDelayedMyinfo*/);
			mDcServer->mIpList.sendToAll(appendUserIp(sMsg3, dcUser->getNick(), dcUser->getIp()), false, false);
		}
	}
}



void NmdcProtocol::addToIpList(DcUser * dcUser) {
	if (dcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		mDcServer->mIpList.add(dcUser->getNickHash(), dcUser);
		// TODO: send bot list
		dcUser->send(mDcServer->mDcUserList.getList(USER_LIST_IP), true);
	}
}



void NmdcProtocol::delFromIpList(DcUser * dcUser) {
	if (dcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		mDcServer->mIpList.remove(dcUser->getNickHash());
	}
}



void NmdcProtocol::addToHide(DcUser * dcUser) {
	if (dcUser->isCanSend()) {
		string msg;
		dcUser->setCanSend(false);
		mDcServer->mDcUserList.sendToAll(appendQuit(msg, dcUser->getNick()), false, true/*mDcServer->mDcConfig.mDelayedMyinfo*/);
		mDcServer->mEnterList.sendToAll(msg, false, true/*mDcServer->mDcConfig.mDelayedMyinfo*/); // false cache
		dcUser->setCanSend(true);
		mDcServer->mOpList.remake();
		mDcServer->mDcUserList.remake();
	}
}



void NmdcProtocol::delFromHide(DcUser * dcUser) {
	if (dcUser->isCanSend()) {
		string sMsg1, sMsg2, sMsg3;
		if (dcUser->isTrueBoolParam(USER_PARAM_IN_OP_LIST)) {
			dcUser->setCanSend(false);
			mDcServer->mHelloList.sendToAll(appendHello(sMsg1, dcUser->getNick()), false, true/*mDcServer->mDcConfig.mDelayedMyinfo*/);
			mDcServer->mDcUserList.sendToAll(dcUser->getInfo(), true, false);
			mDcServer->mDcUserList.sendToAll(appendOpList(sMsg2, dcUser->getNick()), false, true/*mDcServer->mDcConfig.mDelayedMyinfo*/);
			mDcServer->mEnterList.sendToAll(dcUser->getInfo(), true, false);
			mDcServer->mEnterList.sendToAll(sMsg2, false, true/*mDcServer->mDcConfig.mDelayedMyinfo*/);
			mDcServer->mIpList.sendToAll(appendUserIp(sMsg3, dcUser->getNick(), dcUser->getIp()), false, true);
			dcUser->setCanSend(true);
		} else {
			dcUser->setCanSend(false);
			mDcServer->mHelloList.sendToAll(appendHello(sMsg1, dcUser->getNick()), false, true/*mDcServer->mDcConfig.mDelayedMyinfo*/);
			mDcServer->mDcUserList.sendToAll(dcUser->getInfo(), true, true/*mDcServer->mDcConfig.mDelayedMyinfo*/);
			mDcServer->mEnterList.sendToAll(dcUser->getInfo(), true, true/*mDcServer->mDcConfig.mDelayedMyinfo*/);
			mDcServer->mIpList.sendToAll(appendUserIp(sMsg3, dcUser->getNick(), dcUser->getIp()), false, true);
			dcUser->setCanSend(true);
		}
		mDcServer->mOpList.remake();
		mDcServer->mDcUserList.remake();
	}
}



/// DCN Escape
void NmdcProtocol::dcnEscape(const char * buf, size_t len, string & dest) {
	dest.clear();
	unsigned char c;
	char b[11];
	while (len-- > 0) {
		c = *(buf++);
		switch (c) {
			case 0:
			case 5:
			case 36:
			case 96:
			case 124:
			case 126:
				sprintf(b, "/%%DCN%03d%%/", c);
				dest += b;
				break;
			default:
				dest += c;
				break;
		}
	}
}



/// DCN UnEscape
void NmdcProtocol::dcnUnescape(const string & src, char * dest, size_t & len) {
	string start = "/%DCN", end = "%/";
	unsigned char c;
	size_t pos = src.find(start), pos2 = 0;
	len = 0;
	while ((pos != string::npos) && (len < src.size())) {
		if (pos > pos2) {
			memcpy(dest + len, src.c_str() + pos2, pos - pos2);
			len += pos - pos2;
		}
		pos2 = src.find(end, pos);
		if ((pos2 != string::npos) && 
				(pos2 - pos <= start.size() + 3)) {
			c = static_cast<unsigned char> (atoi(src.substr(pos + start.size(), 3).c_str()));
			dest[len++] = c;
			pos2 += end.size();
		}
		pos = src.find(start, pos + 1);
	}
	if (pos2 < src.size()) {
		memcpy(dest + len, src.c_str() + pos2, src.size() - pos2 + 1);
		len += src.size() - pos2;
	}
}



/// lock2key
void NmdcProtocol::lock2key(const string & lock, string & key) {
	size_t count = 0, len = lock.size();
	char *k = 0, *l = new char[len + 1];
	dcnUnescape(lock, l, len);

	k = new char[len + 1];

	k[0] = l[0] ^ l[len - 1] ^ l[len - 2] ^ 5;
	while (++count < len) {
		k[count] = l[count] ^ l[count - 1];
	}
	k[len] = 0;

	count = 0;
	--count;
	while (++count < len) {
		k[count] = ((k[count] << 4)) | ((k[count] >> 4));
	}

	dcnEscape(k, len, key);
	delete [] k;
	delete [] l;
}


} // namespace protocol

} // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
