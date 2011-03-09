/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz

 * modified: 27 Aug 2009
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

#include "DcProtocol.h"
#include "DcServer.h" // for mDcServer
#include "DcConn.h" // for DcConn

namespace dcserver {

namespace protocol {

#define BADFLAG(CMD, FLAG) \
if (dcConn->GetLSFlag(FLAG)) { \
	if (dcConn->Log(1)) { \
		dcConn->LogStream() << "Attempt to attack in " CMD " (repeated sending)" << endl; \
	} \
	dcConn->closeNow(CLOSE_REASON_CMD_REPEAT); \
	return -1; \
}



DcProtocol::DcProtocol() {
	SetClassName("DcProtocol");
}



DcProtocol::~DcProtocol() {
}



int DcProtocol::DoCmd(Parser * parser, Conn * conn) {
	DcConn * dcConn = (DcConn *)conn;
	DcParser * dcparser = (DcParser *)parser;

	if (checkCommand(dcparser, dcConn) < 0) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnAny.CallAll(dcConn, dcparser)) {
			return 1;
		}
	#endif

	if (dcConn->Log(5)) {
		dcConn->LogStream() << "[S]Stage " << dcparser->miType << endl;
	}
	
	switch (parser->miType) {

		case NMDC_TYPE_MSEARCH :
			// Fallthrough

		case NMDC_TYPE_MSEARCH_PAS :
			// Fallthrough

		case NMDC_TYPE_SEARCH_PAS :
			// Fallthrough

		case NMDC_TYPE_SEARCH :
			DC_Search(dcparser, dcConn);
			break;

		case NMDC_TYPE_SR :
			DC_SR(dcparser, dcConn);
			break;

		case NMDC_TYPE_MYNIFO :
			DC_MyINFO(dcparser, dcConn);
			break;

		case NMDC_TYPE_SUPPORTS :
			DC_Supports(dcparser, dcConn);
			break;

		case NMDC_TYPE_KEY :
			DC_Key(dcparser, dcConn);
			break;

		case NMDC_TYPE_VALIDATENICK :
			DC_ValidateNick(dcparser, dcConn);
			break;

		case NMDC_TYPE_VERSION :
			DC_Version(dcparser, dcConn);
			break;

		case NMDC_TYPE_GETNICKLIST :
			DC_GetNickList(dcparser, dcConn);
			break;

		case NMDC_TYPE_CHAT :
			DC_Chat(dcparser, dcConn);
			break;

		case NMDC_TYPE_TO :
			DC_To(dcparser, dcConn);
			break;

		case NMDC_TYPE_MYPASS :
			DC_MyPass(dcparser, dcConn);
			break;

		case NMDC_TYPE_CONNECTTOME :
			DC_ConnectToMe(dcparser, dcConn);
			break;

		case NMDC_TYPE_RCONNECTTOME :
			DC_RevConnectToMe(dcparser, dcConn);
			break;

		case NMDC_TYPE_MCONNECTTOME :
			DC_MultiConnectToMe(dcparser, dcConn);
			break;

		case NMDC_TYPE_KICK :
			DC_Kick(dcparser, dcConn);
			break;

		case NMDC_TYPE_OPFORCEMOVE :
			DC_OpForceMove(dcparser, dcConn);
			break;

		case NMDC_TYPE_GETINFO :
			DC_GetINFO(dcparser, dcConn);
			break;

		case NMDC_TYPE_MCTO :
			DC_MCTo(dcparser, dcConn);
			break;

		case NMDC_TYPE_PING :
			DC_Ping(dcparser, dcConn);
			break;

		case NMDC_TYPE_UNKNOWN :
			DC_Unknown(dcparser, dcConn);
			break;

		case NMDC_TYPE_QUIT :
			DC_Quit(dcparser, dcConn);
			break;

		case NMDC_TYPE_UNPARSED :
			dcparser->Parse();
			return DoCmd(parser, dcConn);

		default :
			if (ErrLog(1)) {
				LogStream() << "Incoming untreated event: " << parser->miType << endl;
			}
			break;

	}

	if (dcConn->Log(5)) {
		dcConn->LogStream() << "[E]Stage " << dcparser->miType << endl;
	}
	return 0;
}

int DcProtocol::DC_Supports(DcParser * dcparser, DcConn * dcConn) {

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
			dcConn->mFeatures |= SUPPORT_FEATUER_NOHELLO;
		} else if (feature == "UserIP2") {
			dcConn->mFeatures |= SUPPORT_FEATUER_USERIP2;
		} else if (feature == "TTHSearch") {
			dcConn->mFeatures |= SUPPORT_FEATUER_TTHSEARCH;
		} else if (feature == "QuickList") {
			dcConn->mFeatures |= SUPPORT_FEATUER_QUICKLIST;
		}
		feature.clear();
		is >> feature;
	}
	dcConn->msSupports.assign(dcparser->mCommand, 10, dcparser->mCommand.size() - 10);

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnSupports.CallAll(dcConn, dcparser)) {
			return -3;
		}
	#endif

	static string msg("$Supports UserCommand NoGetINFO NoHello UserIP2 MCTo"NMDC_SEPARATOR);
	dcConn->send(msg, false, false);

	return 0;
}

int DcProtocol::DC_Key(DcParser * dcparser, DcConn * dcConn) {
	BADFLAG("Key", LOGIN_STATUS_KEY);

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnKey.CallAll(dcConn, dcparser)) {
			//string sLock, sKey;
			//sLock = Append_DC_Lock(sLock).substr(1, sLock.size() - 1);
			//Lock2Key(sLock, sKey);
		}
	#endif

	dcConn->SetLSFlag(LOGIN_STATUS_KEY); /** User has sent key */
	dcConn->ClearTimeOut(HUB_TIME_OUT_KEY);
	dcConn->SetTimeOut(HUB_TIME_OUT_VALNICK, mDcServer->mDcConfig.mTimeout[HUB_TIME_OUT_VALNICK], mDcServer->mTime);
	dcConn->mTimes.mKey.Get();
	return 0;
}

int DcProtocol::DC_ValidateNick(DcParser * dcparser, DcConn * dcConn) {
	BADFLAG("ValidateNick", LOGIN_STATUS_VALNICK);

	string &sNick = dcparser->chunkString(CHUNK_1_PARAM);
	unsigned iNickLen = sNick.length();

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
		DcUser *NewUser = new DcUser(sNick);
		if (!dcConn->SetUser(NewUser)) {
			dcConn->closeNow(CLOSE_REASON_USER_SET);
			return -2;
		}
	} catch(...) {
		if (mDcServer->ErrLog(0)) {
			mDcServer->LogStream() << "Unhandled exception in DcProtocol::DC_ValidateNick" << endl;
		}
		if (dcConn->ErrLog(0)) {
			dcConn->LogStream() << "Error in SetUser closing" << endl;
		}
		dcConn->closeNice(9000, CLOSE_REASON_USER_SET);
		return -2;
	}

	/** Checking validate user */
	if (!mDcServer->ValidateUser(dcConn, sNick)) {
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

	dcConn->SetLSFlag(LOGIN_STATUS_ALOWED);
	++mDcServer->miTotalUserCount;

	dcConn->SetLSFlag(LOGIN_STATUS_VALNICK | LOGIN_STATUS_NICKLST); /** We Install NICKLST because user can not call user list */
	dcConn->ClearTimeOut(HUB_TIME_OUT_VALNICK);

	string sMsg;

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnValidateNick.CallAll(dcConn, dcparser)) {
			dcConn->send(Append_DC_GetPass(sMsg)); /** We are sending the query for reception of the password */
			dcConn->SetTimeOut(HUB_TIME_OUT_PASS, mDcServer->mDcConfig.mTimeout[HUB_TIME_OUT_PASS], mDcServer->mTime);
			return -4;
		}
	#endif

	if (!mDcServer->CheckNickLength(dcConn, iNickLen)) {
		dcConn->closeNice(9000, CLOSE_REASON_NICK_LEN);
	}
	dcConn->SetTimeOut(HUB_TIME_OUT_MYINFO, mDcServer->mDcConfig.mTimeout[HUB_TIME_OUT_MYINFO], mDcServer->mTime);
	dcConn->SetLSFlag(LOGIN_STATUS_PASSWD); /** Does not need password */

	dcConn->send(Append_DC_Hello(sMsg, dcConn->mDcUser->msNick)); /** Protection from change the command */
	return 0;
}

int DcProtocol::DC_MyPass(DcParser * dcparser, DcConn * dcConn) {
	if (!dcConn->mDcUser) { /* Check of existence of the user for current connection */
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Mypass before validatenick" << endl;
		}
		mDcServer->sendToUser(dcConn, mDcServer->mDCLang.mBadLoginSequence.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_CMD_PASSWORD_ERR);
		return -1;
	}
	BADFLAG("MyPass", LOGIN_STATUS_PASSWD);

	// Checking the accepted password, otherwise send $BadPass|
	// or $Hello Nick|$LogedIn Nick|
	int bOp = 0;
	#ifndef WITHOUT_PLUGINS
		bOp = (mDcServer->mCalls.mOnMyPass.CallAll(dcConn, dcparser));
	#endif

	if (!mDcServer->CheckNickLength(dcConn, dcConn->mDcUser->msNick.length())) {
		dcConn->closeNice(9000, CLOSE_REASON_NICK_LEN);
	}
	string sMsg;
	dcConn->SetLSFlag(LOGIN_STATUS_PASSWD); /** Password is accepted */
	Append_DC_Hello(sMsg, dcConn->mDcUser->msNick);
	if (bOp) { /** If entered operator, that sends command LoggedIn ($LogedIn !) */
		sMsg.append("$LogedIn ");
		sMsg.append(dcConn->mDcUser->msNick);
		sMsg.append(NMDC_SEPARATOR);
	}
	dcConn->send(sMsg);
	dcConn->ClearTimeOut(HUB_TIME_OUT_PASS);
	dcConn->SetTimeOut(HUB_TIME_OUT_MYINFO, mDcServer->mDcConfig.mTimeout[HUB_TIME_OUT_MYINFO], mDcServer->mTime);
	return 0;
}

int DcProtocol::DC_Version(DcParser * dcparser, DcConn * dcConn) {
	BADFLAG("Version", LOGIN_STATUS_VERSION);

	string & sVersion = dcparser->chunkString(CHUNK_1_PARAM);
	if (dcConn->Log(3)) {
		dcConn->LogStream() << "Version:" << sVersion << endl;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnVersion.CallAll(dcConn, dcparser) && sVersion != "1,0091") {
			dcConn->closeNice(9000, CLOSE_REASON_CMD_VERSION); /** Checking of the version */
		}
	#endif

	dcConn->SetLSFlag(LOGIN_STATUS_VERSION); /** Version was checked */
	dcConn->mVersion = sVersion;
	return 0;
}

int DcProtocol::DC_GetNickList(DcParser * dcparser, DcConn * dcConn) {

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnGetNickList.CallAll(dcConn, dcparser)) {
			return 3;
		}
	#endif

	if (!dcConn->GetLSFlag(LOGIN_STATUS_MYINFO) && mDcServer->mDcConfig.mNicklistOnLogin) {
		if (mDcServer->mDcConfig.mDelayedLogin) {
			int LSFlag = dcConn->GetLSFlag(LOGIN_STATUS_LOGIN_DONE);
			if (LSFlag & LOGIN_STATUS_NICKLST) {
				LSFlag -= LOGIN_STATUS_NICKLST;
			}
			dcConn->ReSetLSFlag(LSFlag);
		}
		dcConn->mbSendNickList = true;
		return 0;
	}
	return SendNickList(dcConn);
}

int DcProtocol::DC_MyINFO(DcParser * dcparser, DcConn * dcConn) {

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
	} else if (sNick != dcConn->mDcUser->msNick) { /** Проверка ника */
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
		iMode = mDcServer->mCalls.mOnMyINFO.CallAll(dcConn, dcparser);
	#endif

	if (iMode != 1 && dcConn->mDcUser->getInUserList()) {
		if (sOldMyINFO != dcConn->mDcUser->getMyINFO()) {
			if (dcConn->mDcUser->mbHide) {
				dcConn->send(dcparser->mCommand, true); // Send to self only
			} else {
				SendMode(dcConn, dcparser->mCommand, iMode, mDcServer->mDCUserList, true); // Use cache for send to all
				//mDcServer->mDCUserList.sendToAll(dcparser->mCommand, true/*mDcServer->mDcConfig.mDelayedMyinfo*/); // Send to all
			}
		}
	} else if (!dcConn->mDcUser->getInUserList()) {
		dcConn->SetLSFlag(LOGIN_STATUS_MYINFO);
		if (!mDcServer->BeforeUserEnter(dcConn)) {
			return -1;
		}
		dcConn->ClearTimeOut(HUB_TIME_OUT_MYINFO);
	}
	return 0;
}

int DcProtocol::DC_Chat(DcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser) {
		return -2;
	}
	if (!dcConn->mDcUser->getInUserList()) {
		return -3;
	}

	/** Check chat nick */
	if ((dcparser->chunkString(CHUNK_CH_NICK) != dcConn->mDcUser->msNick) ) {
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Bad nick in chat, closing" << endl;
		}
		string sMsg = mDcServer->mDCLang.mBadChatNick;
		StringReplace(sMsg, string("nick"), sMsg, dcparser->chunkString(CHUNK_CH_NICK));
		StringReplace(sMsg, string("real_nick"), sMsg, dcConn->mDcUser->msNick);
		mDcServer->sendToUser(dcConn, sMsg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_CHAT);
		return -2;
	}
	int iMode = 0;
	#ifndef WITHOUT_PLUGINS
		iMode = mDcServer->mCalls.mOnChat.CallAll(dcConn, dcparser);
	#endif

	//Hash<unsigned long> hash;
	//unsigned long key = hash(dcparser->mCommand);
	//cout << key << endl;

	/** Sending message */
	SendMode(dcConn, dcparser->mCommand, iMode, mDcServer->mChatList, false); // Don't use cache for send to all
	return 0;
}

int DcProtocol::DC_To(DcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser) {
		return -2;
	}
	if (!dcConn->mDcUser->getInUserList()) {
		return -3;
	}
	string & sNick = dcparser->chunkString(CHUNK_PM_TO);

	/** Checking the coincidence nicks in command */
	if (dcparser->chunkString(CHUNK_PM_FROM) != dcConn->mDcUser->msNick || dcparser->chunkString(CHUNK_PM_NICK) != dcConn->mDcUser->msNick) {
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Bad nick in PM, closing" <<endl;
		}
		dcConn->closeNow(CLOSE_REASON_NICK_PM);
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnTo.CallAll(dcConn, dcparser)) {
			return 0;
		}
	#endif

	/** Search user */
	DcUser * User = (DcUser *)mDcServer->mDCUserList.GetUserBaseByNick(sNick);
	if (!User) {
		return -2;
	}

	User->send(dcparser->mCommand, true);
	return 0;
}

int DcProtocol::DC_MCTo(DcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser) {
		return -2;
	}
	if (!dcConn->mDcUser->getInUserList()) {
		return -3;
	}

	string & sNick = dcparser->chunkString(CHUNK_MC_TO);

	/** Checking the coincidence nicks in command */
	if (dcparser->chunkString(CHUNK_MC_FROM) != dcConn->mDcUser->msNick) {
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Bad nick in MCTo, closing" <<endl;
		}
		dcConn->closeNow(CLOSE_REASON_NICK_MCTO);
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnMCTo.CallAll(dcConn, dcparser)) {
			return 0;
		}
	#endif

	/** Search user */
	DcUser * User = (DcUser *)mDcServer->mDCUserList.GetUserBaseByNick(sNick);
	if (!User) {
		return -2;
	}

	string msg;
	User->send(Append_DC_Chat(msg, dcConn->mDcUser->msNick, dcparser->chunkString(CHUNK_MC_MSG)));
	if (dcConn->mDcUser->msNick != sNick) {
		dcConn->send(msg);
	}
	return 0;
}

/**
	NMDC_TYPE_SEARCH
	NMDC_TYPE_SEARCH_PAS
	NMDC_TYPE_MSEARCH
	NMDC_TYPE_MSEARCH_PAS
*/
int DcProtocol::DC_Search(DcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser || !dcConn->mDcUser->getInUserList()) {
		return -2;
	}

	// TODO: Check overloading of the system

	int iMode = 0;
	#ifndef WITHOUT_PLUGINS
		iMode = mDcServer->mCalls.mOnSearch.CallAll(dcConn, dcparser);
	#endif

	/** Sending cmd */
	string sMsg;

	switch (dcparser->miType) {

		case NMDC_TYPE_SEARCH :
			if (mDcServer->mDcConfig.mCheckSearchIp && dcConn->mIp != dcparser->chunkString(CHUNK_AS_IP)) {
				sMsg = mDcServer->mDCLang.mBadSearchIp;
				if (dcConn->Log(2)) {
					dcConn->LogStream() << "Bad ip in active search, closing" << endl;
				}
				StringReplace(sMsg, string("ip"), sMsg, dcparser->chunkString(CHUNK_AS_IP));
				StringReplace(sMsg, string("real_ip"), sMsg, dcConn->mIp);
				mDcServer->sendToUser(dcConn, sMsg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
				dcConn->closeNice(9000, CLOSE_REASON_NICK_SEARCH);
				return -1;
			}
			SendMode(dcConn, dcparser->mCommand, iMode, mDcServer->mDCUserList, true); // Use cache for send to all
			break;

		case NMDC_TYPE_SEARCH_PAS :
			dcConn->miSRCounter = 0; /** Zeroizing result counter of the passive search */
			SendMode(dcConn, dcparser->mCommand, iMode, mDcServer->mActiveList, true); // Use cache for send to all
			break;

		case NMDC_TYPE_MSEARCH :
			if (mDcServer->mDcConfig.mCheckSearchIp && (dcConn->mIp != dcparser->chunkString(CHUNK_AS_IP))) {
				sMsg = mDcServer->mDCLang.mBadSearchIp;
				if (dcConn->Log(2)) {
					dcConn->LogStream() << "Bad ip in active search, closing" << endl;
				}
				StringReplace(sMsg, string("ip"), sMsg, dcparser->chunkString(CHUNK_AS_IP));
				StringReplace(sMsg, string("real_ip"), sMsg, dcConn->mIp);
				mDcServer->sendToUser(dcConn, sMsg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
				dcConn->closeNice(9000, CLOSE_REASON_NICK_SEARCH);
				return -1;
			}
			sMsg = "$Search ";
			sMsg += dcparser->chunkString(CHUNK_AS_ADDR);
			sMsg += ' ';
			sMsg += dcparser->chunkString(CHUNK_AS_QUERY);
			SendMode(dcConn, sMsg, iMode, mDcServer->mDCUserList, true); // Use cache for send to all
			break;

		case NMDC_TYPE_MSEARCH_PAS :
			dcConn->miSRCounter = 0; /** Zeroizing result counter of the passive search */
			SendMode(dcConn, dcparser->mCommand, iMode, mDcServer->mActiveList, true); // Use cache for send to all
			break;

		default :
			return -4;

	}
	return 0;
}

int DcProtocol::DC_SR(DcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser || !dcConn->mDcUser->getInUserList()) {
		return -2;
	}

	/** Check same nick in cmd (PROTOCOL NMDC) */
	if (mDcServer->mDcConfig.mCheckSrNick && (dcConn->mDcUser->msNick != dcparser->chunkString(CHUNK_SR_FROM))) {
		string sMsg = mDcServer->mDCLang.mBadSrNick;
		if (dcConn->Log(2)) {
			dcConn->LogStream() << "Bad nick in search response, closing" << endl;
		}
		StringReplace(sMsg, "nick", sMsg, dcparser->chunkString(CHUNK_SR_FROM));
		StringReplace(sMsg, "real_nick", sMsg, dcConn->mDcUser->msNick);
		mDcServer->sendToUser(dcConn, sMsg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_SR);
		return -1;
	}

	DcUser * dcUser = NULL;
	const string & nick = dcparser->chunkString(CHUNK_SR_TO);
	if (nick != "") {
		dcUser = (DcUser *)mDcServer->mDCUserList.GetUserBaseByNick(nick);

		/** Is user exist? */
		if (!dcUser || !dcUser->mDcConn) {
			return -2;
		}
	}

	/** != 0 - error */
	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnSR.CallAll(dcConn, dcparser)) {
			return -3;
		}
	#endif

	/** Sending cmd */
	if (dcUser && (!mDcServer->mDcConfig.mMaxPassiveRes || (++dcUser->mDcConn->miSRCounter <= unsigned(mDcServer->mDcConfig.mMaxPassiveRes)))) {
		string sStr(dcparser->mCommand, 0, dcparser->mChunks[CHUNK_SR_TO].first - 1); /** Remove nick on the end of cmd */
		dcUser->mDcConn->send(sStr, true, false);
	}
	return 0;
}


int DcProtocol::DC_ConnectToMe(DcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser || !dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	if (mDcServer->mDcConfig.mCheckCtmIp && dcConn->mIp != dcparser->chunkString(CHUNK_CM_IP)) {
		string sMsg = mDcServer->mDCLang.mBadCtmIp;
		StringReplace(sMsg, string("ip"), sMsg, dcparser->chunkString(CHUNK_CM_IP));
		StringReplace(sMsg, string("real_ip"), sMsg, dcConn->mIp);
		mDcServer->sendToUser(dcConn, sMsg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_CTM);
		return -1;
	}

	DcUser * User = (DcUser *)mDcServer->mDCUserList.GetUserBaseByNick(dcparser->chunkString(CHUNK_CM_NICK));
	if (!User) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnConnectToMe.CallAll(dcConn, dcparser)) {
			return -2;
		}
	#endif

	User->send(dcparser->mCommand, true);
	return 0;
}

int DcProtocol::DC_RevConnectToMe(DcParser * dcparser, DcConn * dcConn) {

	if (!dcConn->mDcUser || !dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	/** Checking the nick */
	if (mDcServer->mDcConfig.mCheckRctmNick && (dcparser->chunkString(CHUNK_RC_NICK) != dcConn->mDcUser->msNick)) {
		string sMsg = mDcServer->mDCLang.mBadRevConNick;
		StringReplace(sMsg, string("nick"), sMsg, dcparser->chunkString(CHUNK_RC_NICK));
		StringReplace(sMsg, string("real_nick"), sMsg, dcConn->mDcUser->msNick);
		mDcServer->sendToUser(dcConn, sMsg.c_str(), mDcServer->mDcConfig.mHubBot.c_str());
		dcConn->closeNice(9000, CLOSE_REASON_NICK_RCTM);
		return -1;
	}

	/** Searching the user */
	DcUser * other = (DcUser *)mDcServer->mDCUserList.GetUserBaseByNick(dcparser->chunkString(CHUNK_RC_OTHER));
	if (!other) {
		return -2;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnRevConnectToMe.CallAll(dcConn, dcparser)) {
			return -2;
		}
	#endif

	other->send(dcparser->mCommand, true);
	return 0;
}

int DcProtocol::DC_MultiConnectToMe(DcParser *, DcConn *) {
	return 0;
}

int DcProtocol::DC_Kick(DcParser * dcparser, DcConn * dcConn) {

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnKick.CallAll(dcConn, dcparser)) {
			return -1;
		}
	#endif

	if (!dcConn->mDcUser || !dcConn->mDcUser->mbKick) {
		return -2;
	}

	DcUser * User = (DcUser *)mDcServer->mDCUserList.GetUserBaseByNick(dcparser->chunkString(CHUNK_1_PARAM));

	/** Is user exist? */
	if (!User || !User->mDcConn) {
		return -3;
	}

	User->mDcConn->closeNice(9000, CLOSE_REASON_CMD_KICK);
	return 0;
}

int DcProtocol::DC_OpForceMove(DcParser * dcparser, DcConn * dcConn) {

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnOpForceMove.CallAll(dcConn, dcparser)) {
			return -1;
		}
	#endif

	if (!dcConn->mDcUser || !dcConn->mDcUser->mbForceMove) {
		return -2;
	}

	DcUser * User = (DcUser *)mDcServer->mDCUserList.GetUserBaseByNick(dcparser->chunkString(CHUNK_FM_NICK));

	/** Is user exist? */
	if (!User || !User->mDcConn || !dcparser->chunkString(CHUNK_FM_DEST).size()) {
		return -3;
	}

	mDcServer->forceMove(User->mDcConn, dcparser->chunkString(CHUNK_FM_DEST).c_str(), dcparser->chunkString(CHUNK_FM_REASON).c_str());
	return 0;
}

int DcProtocol::DC_GetINFO(DcParser * dcparser, DcConn * dcConn) {
	if (!dcConn->mDcUser || !dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	DcUser * User = (DcUser *)mDcServer->mDCUserList.GetUserBaseByNick(dcparser->chunkString(CHUNK_GI_OTHER));
	if (!User) {
		return -2;
	}

	if (dcConn->mDcUser->mTimeEnter < User->mTimeEnter && Time() < (User->mTimeEnter + 60000)) {
		return 0;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnGetINFO.CallAll(dcConn, dcparser)) {
			return -2;
		}
	#endif

	//if(!(dcConn->mFeatures & SUPPORT_FEATUER_NOGETINFO)){
	if (!User->mbHide) {
		dcConn->send(string(User->getMyINFO()), true, false);
	}
	return 0;
}

int DcProtocol::DC_Ping(DcParser *, DcConn *) {
	return 0;
}

int DcProtocol::DC_Unknown(DcParser * dcparser, DcConn * dcConn) {

	#ifndef WITHOUT_PLUGINS
	if (!mDcServer->mCalls.mOnUnknown.CallAll(dcConn, dcparser)) {
		dcConn->closeNice(9000, CLOSE_REASON_CMD_UNKNOWN);
		return -2;
	}
	#endif
	return 0;
}

int DcProtocol::DC_Quit(DcParser *, DcConn * dcConn) {
	dcConn->closeNice(9000, CLOSE_REASON_CMD_QUIT);
	return 0;
}













// $Lock ...|
string & DcProtocol::Append_DC_Lock(string & str) {
	static const char * cmd = "$Lock EXTENDEDPROTOCOL_" INTERNALNAME "_by_setuper_" INTERNALVERSION " Pk=" INTERNALNAME NMDC_SEPARATOR;
	static unsigned int cmdLen = strlen(cmd);
	return str.append(cmd, cmdLen);
}

// $Hello nick|
string & DcProtocol::Append_DC_Hello(string & str, const string & nick) {
	static const char * cmd = "$Hello ";
	static unsigned int cmdLen = 7 + NMDC_SEPARATOR_LEN;
	str.reserve(str.size() + nick.size() + cmdLen);
	return str.append(cmd, 7).append(nick).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
}

// $HubIsFull|
string & DcProtocol::Append_DC_HubIsFull(string & str) {
	static const char * cmd = "$HubIsFull" NMDC_SEPARATOR;
	static unsigned int cmdLen = 10 + NMDC_SEPARATOR_LEN;
	return str.append(cmd, cmdLen);
}

// $GetPass|
string & DcProtocol::Append_DC_GetPass(string & str) {
	static const char * cmd = "$GetPass" NMDC_SEPARATOR;
	static unsigned int cmdLen = 8 + NMDC_SEPARATOR_LEN;
	return str.append(cmd, cmdLen);
}

// $ValidateDenide nick|
string & DcProtocol::Append_DC_ValidateDenide(string & str, const string & nick) {
	static const char * cmd = "$ValidateDenide ";
	static unsigned int cmdLen = 16 + NMDC_SEPARATOR_LEN;
	str.reserve(str.size() + nick.size() + cmdLen);
	return str.append(cmd, 16).append(nick).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
}

// $HubName hubName - topic|
string & DcProtocol::Append_DC_HubName(string & str, const string & hubName, const string & topic) {
	static const char * cmd = "$HubName ";
	static const char * cmd2 = " - ";
	static unsigned int cmdLen = 9 + NMDC_SEPARATOR_LEN;
	static unsigned int cmdLen2 = 12 + NMDC_SEPARATOR_LEN;
	if (topic.length()) {
		str.reserve(str.size() + hubName.size() + topic.size() + cmdLen2);
		return str.append(cmd, 9).append(hubName).append(cmd2, 3).append(topic).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
	} else {
		str.reserve(str.size() + hubName.size() + cmdLen);
		return str.append(cmd, 9).append(hubName).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
	}
}

// $HubTopic hubTopic|
string & DcProtocol::Append_DC_HubTopic(string & str, const string & hubTopic) {
	static const char * cmd = "$HubTopic ";
	static unsigned int cmdLen = 10 + NMDC_SEPARATOR_LEN;
	str.reserve(str.size() + hubTopic.size() + cmdLen);
	return str.append(cmd, 10).append(hubTopic).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
}

// <nick> msg|
string & DcProtocol::Append_DC_Chat(string & str, const string & nick, const string & msg) {
	static const char * cmd = "<";
	static const char * cmd2 = "> ";
	static unsigned int cmdLen = 3 + NMDC_SEPARATOR_LEN;
	str.reserve(str.size() + nick.size() + msg.size() + cmdLen);
	return str.append(cmd, 1).append(nick).append(cmd2, 2).append(msg).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
}

// $To: to From: from $<nick> msg|
string & DcProtocol::Append_DC_PM(string & str, const string & to, const string & from, const string & nick, const string & msg) {
	static const char * cmd = "$To: ";
	static const char * cmd2 = " From: ";
	static const char * cmd3 = " $<";
	static const char * cmd4 = "> ";
	static unsigned int cmdLen = 17 + NMDC_SEPARATOR_LEN;
	str.reserve(str.size() + to.size() + from.size() + nick.size() + msg.size() + cmdLen);
	str.append(cmd, 5).append(to).append(cmd2, 7).append(from).append(cmd3, 3).append(nick);
	return str.append(cmd4, 2).append(msg).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
}

// $To: to From: from $<nick> msg|
void DcProtocol::Append_DC_PMToAll(string & start, string & end, const string & from, const string & nick, const string & msg) {
	static const char * cmd = "$To: ";
	static const char * cmd2 = " From: ";
	static const char * cmd3 = " $<";
	static const char * cmd4 = "> ";
	static unsigned int cmdLen = 12 + NMDC_SEPARATOR_LEN;
	start.append(cmd, 5);
	end.reserve(end.size() + from.size() + nick.size() + msg.size() + cmdLen);
	end.append(cmd2, 7).append(from).append(cmd3, 3).append(nick);
	end.append(cmd4, 2).append(msg).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
}

// $Quit nick|
string & DcProtocol::Append_DC_Quit(string & str, const string & nick) {
	static const char * cmd = "$Quit ";
	static unsigned int cmdLen = 6 + NMDC_SEPARATOR_LEN;
	str.reserve(str.size() + nick.size() + cmdLen);
	return str.append(cmd, 6).append(nick).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
}

// $OpList nick$$|
string & DcProtocol::Append_DC_OpList(string & str, const string & nick) {
	static const char * cmd = "$OpList ";
	static const char * cmd2 = "$$"NMDC_SEPARATOR;
	static unsigned int cmdLen = 10 + NMDC_SEPARATOR_LEN;
	static unsigned int cmdLen2 = 2 + NMDC_SEPARATOR_LEN;
	str.reserve(str.size() + nick.size() + cmdLen);
	return str.append(cmd, 8).append(nick).append(cmd2, cmdLen2);
}

// $UserIP nick ip$$|
string & DcProtocol::Append_DC_UserIP(string & str, const string & nick, const string & ip) {
	static const char * cmd = "$UserIP ";
	static const char * cmd2 = " ";
	static const char * cmd3 = "$$"NMDC_SEPARATOR;
	static unsigned int cmdLen = 11 + NMDC_SEPARATOR_LEN;
	static unsigned int cmdLen2 = 2 + NMDC_SEPARATOR_LEN;
	if (ip.length()) {
		str.reserve(str.size() + nick.size() + ip.size() + cmdLen);
		str.append(cmd, 8).append(nick).append(cmd2, 1).append(ip).append(cmd3, cmdLen2);
	}
	return str;
}

string & DcProtocol::Append_DC_ForceMove(string & str, const string & address) {
	static const char * cmd = "$forceMove ";
	str.reserve(address.size() + 11 + NMDC_SEPARATOR_LEN);
	return str.append(cmd, 11).append(address).append(NMDC_SEPARATOR, NMDC_SEPARATOR_LEN);
}





void DcProtocol::SendMode(DcConn * dcConn, const string & str, int iMode, UserList & UL, bool bUseCache) {
	bool bAddSep = false;
	if (str.substr(str.size() - 1, 1) != NMDC_SEPARATOR) {
		bAddSep = true;
	}

	if (iMode == 0) { /** Send to all */
		UL.sendToAll(str, bUseCache, bAddSep);
	} else if (iMode == 3) { /** Send to all except current user */
		if (dcConn->mDcUser->getInUserList()) {
			dcConn->mDcUser->setInUserList(false);
			UL.sendToAll(str, bUseCache, bAddSep);
			dcConn->mDcUser->setInUserList(true);
		}
	} else if (iMode == 4) { /** Send to all except users with ip of the current user */
		DcConn * conn = NULL;
		vector<DcConn *> ul;
		for (DcIpList::iterator mit = mDcServer->mIPListConn->begin(DcConn::ip2Num(dcConn->getIp().c_str())); mit != mDcServer->mIPListConn->end(); ++mit) {
			conn = (DcConn *)(*mit);
			if(conn->mDcUser && conn->mDcUser->getInUserList()) {
				conn->mDcUser->setInUserList(false);
				ul.push_back(conn);
			}
		}
		UL.sendToAll(str, bUseCache, bAddSep);
		for (vector<DcConn *>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it) {
			(*ul_it)->mDcUser->setInUserList(true);
		}
	}
}


/** Sending the user-list and op-list */
int DcProtocol::SendNickList(DcConn * dcConn) {
	try {
		if ((dcConn->GetLSFlag(LOGIN_STATUS_LOGIN_DONE) != LOGIN_STATUS_LOGIN_DONE) && mDcServer->mDcConfig.mNicklistOnLogin) {
			dcConn->mbNickListInProgress = true;
		}

		if (dcConn->mFeatures & SUPPORT_FEATUER_NOHELLO) {
			if (dcConn->Log(3)) {
				dcConn->LogStream() << "Sending MyINFO list" << endl;
			}
			// seperator "|" was added in GetInfoList function
			dcConn->send(mDcServer->mDCUserList.GetInfoList(true), false, false);
		} else if (dcConn->mFeatures & SUPPORT_FEATUER_NOGETINFO) {
			if (dcConn->Log(3)) {
				dcConn->LogStream() << "Sending MyINFO list and Nicklist" << endl;
			}
			// seperator "|" was not added in GetNickList function, because seperator was "$$"
			dcConn->send(mDcServer->mDCUserList.GetNickList(), true, false);
			// seperator "|" was added in GetInfoList function
			dcConn->send(mDcServer->mDCUserList.GetInfoList(true), false, false);
		} else {
			if (dcConn->Log(3)) {
				dcConn->LogStream() << "Sending Nicklist" << endl;
			}
			// seperator "|" was not added in GetNickList function, because seperator was "$$"
			dcConn->send(mDcServer->mDCUserList.GetNickList(), true, false);
		}
		if (mDcServer->mOpList.Size()) {
			if (dcConn->Log(3)) {
				dcConn->LogStream() << "Sending Oplist" << endl;
			}
			// seperator "|" was not added in GetNickList function, because seperator was "$$"
			dcConn->send(mDcServer->mOpList.GetNickList(), true, false);
		}

		if (dcConn->mDcUser->getInUserList() && dcConn->mDcUser->mbInIpList) {
			if (dcConn->Log(3)) {
				dcConn->LogStream() << "Sending Iplist" << endl;
			}
			// seperator "|" was not added in GetIpList function, because seperator was "$$"
			dcConn->send(mDcServer->mDCUserList.GetIpList(), true);
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
			dcConn->LogStream() << "exception in SendNickList" << endl;
		}
		return -1;
	}
	return 0;
}

/** Get normal share size */
string DcProtocol::GetNormalShare(__int64 iVal) {
	ostringstream os;
	float s = static_cast<float>(iVal);
	int i = 0;
	for (; ((s >= 1024) && (i < 7)); ++i) {
		s /= 1024;
	}
	os << ::std::floor(s * 1000 + 0.5) / 1000 << " " << DcServer::currentDcServer->mDCLang.mUnits[i];
	return os.str();
}



int DcProtocol::checkCommand(DcParser * dcParser, DcConn * dcConn) {

	// Checking length of command
	if (dcParser->miLen > mDcServer->mDcConfig.mMaxCmdLen[dcParser->miType]) {
		if (dcConn->Log(1)) {
			dcConn->LogStream() << "Bad CMD(" << dcParser->miType << ") length: " << dcParser->miLen << endl;
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
	if (dcParser->SplitChunks()) {
		// Protection from commands, not belonging to DC protocol
		if (dcParser->miType != NMDC_TYPE_UNKNOWN || mDcServer->mDcConfig.mDisableNoDCCmd) {
			if (dcConn->Log(1)) {
				dcConn->LogStream() << "Wrong syntax in cmd: " << dcParser->miType << endl;
			}
			dcConn->closeNice(9000, CLOSE_REASON_CMD_SYNTAX);
			return -3;
		}
	}

	// Check flood
	if (antiflood(dcConn, dcParser->miType)) {
		return -4;
	}

	return 0;
}



bool DcProtocol::antiflood(DcConn * dcConn, unsigned int iType) {
	if (mDcServer->antiFlood(dcConn->mTimes1.mCount[iType], dcConn->mTimes1.mTime[iType],
		mDcServer->mDcConfig.mFloodCount[iType], mDcServer->mDcConfig.mFloodTime[iType])
	) {
		#ifndef WITHOUT_PLUGINS
		if (!mDcServer->mCalls.mOnFlood.CallAll(dcConn, FLOOD_TYPE_MYNIFO, 1))
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
		if (!mDcServer->mCalls.mOnFlood.CallAll(dcConn, FLOOD_TYPE_MYNIFO, 2))
		#endif
		{
			mDcServer->sendToUser(dcConn, (mDcServer->mDCLang.mFlood[iType]).c_str(), mDcServer->mDcConfig.mHubBot.c_str());
			dcConn->closeNice(9000, CLOSE_REASON_FLOOD);
			return true;
		}
	}
	return false;
}


}; // namespace protocol

}; // namespace dcserver
