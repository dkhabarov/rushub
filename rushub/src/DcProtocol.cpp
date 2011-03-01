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

#define BADCMDPARAM(CMD) \
if (dcparser->SplitChunks()) { \
	if (dcconn->Log(1)) { \
		dcconn->LogStream() << "Attempt to attack in " CMD " (bad cmd param)" << endl; \
	} \
	dcconn->CloseNow(CLOSE_REASON_BAD_CMD_PARAM); \
	return -1; \
}

#define BADFLAG(CMD, FLAG) \
if (dcconn->GetLSFlag(FLAG)) { \
	if (dcconn->Log(1)) { \
		dcconn->LogStream() << "Attempt to attack in " CMD " (repeated sending)" << endl; \
	} \
	dcconn->CloseNow(CLOSE_REASON_BAD_FLAG); \
	return -1; \
}

/* antiflood */
#ifndef WITHOUT_PLUGINS
#define ANTIFLOOD(TYPE, FT) \
	if (mDcServer->antiFlood(dcconn->mTimes1.mi##TYPE, dcconn->mTimes1.m##TYPE, \
	mDcServer->mDcConfig.miFloodCount##TYPE, mDcServer->mDcConfig.miFloodTime##TYPE) \
) { \
	if (!mDcServer->mCalls.mOnFlood.CallAll(dcconn, FT, 1)) { \
		mDcServer->sendToUser(dcconn, (mDcServer->mDCLang.msFlood##TYPE).c_str(), (char*)mDcServer->mDcConfig.msHubBot.c_str()); \
		dcconn->CloseNice(9000, CLOSE_REASON_FLOOD); return -1; \
	} \
} \
if (mDcServer->antiFlood(dcconn->mTimes2.mi##TYPE, dcconn->mTimes2.m##TYPE, \
	mDcServer->mDcConfig.miFloodCount##TYPE##2, mDcServer->mDcConfig.miFloodTime##TYPE##2) \
) { \
	if (!mDcServer->mCalls.mOnFlood.CallAll(dcconn, FT, 2)) { \
		mDcServer->sendToUser(dcconn, (mDcServer->mDCLang.msFlood##TYPE).c_str(), (char*)mDcServer->mDcConfig.msHubBot.c_str()); \
		dcconn->CloseNice(9000, CLOSE_REASON_FLOOD); return -1; \
	} \
}
#else
#define ANTIFLOOD(TYPE, FT) \
	if (mDcServer->antiFlood(dcconn->mTimes1.mi##TYPE, dcconn->mTimes1.m##TYPE, \
	mDcServer->mDcConfig.miFloodCount##TYPE, mDcServer->mDcConfig.miFloodTime##TYPE) \
) { \
	mDcServer->sendToUser(dcconn, (mDcServer->mDCLang.msFlood##TYPE).c_str(), (char*)mDcServer->mDcConfig.msHubBot.c_str()); \
	dcconn->CloseNice(9000, CLOSE_REASON_FLOOD); return -1; \
} \
if (mDcServer->antiFlood(dcconn->mTimes2.mi##TYPE, dcconn->mTimes2.m##TYPE, \
	mDcServer->mDcConfig.miFloodCount##TYPE##2, mDcServer->mDcConfig.miFloodTime##TYPE##2) \
) { \
	mDcServer->sendToUser(dcconn, (mDcServer->mDCLang.msFlood##TYPE).c_str(), (char*)mDcServer->mDcConfig.msHubBot.c_str()); \
	dcconn->CloseNice(9000, CLOSE_REASON_FLOOD); return -1; \
}
#endif



DcProtocol::DcProtocol() {
	SetClassName("DcProtocol");
}



DcProtocol::~DcProtocol() {
}




void DcProtocol::SetServer(DcServer * server) {
	mDcServer = server;
}



int DcProtocol::DoCmd(Parser * parser, Conn * conn) {
	DcConn * dcconn = (DcConn *)conn;
	DcParser * dcparser = (DcParser *)parser;

	// Checking length of command
	if (dcparser->miLen > mDcServer->mDcConfig.mMaxCmdLen[dcparser->miType]) {
		if (dcconn->Log(1)) {
			dcconn->LogStream() << "Bad CMD(" << dcparser->miType << ") length: " << dcparser->miLen << endl;
		}
		dcconn->CloseNow(CLOSE_REASON_BAD_CMD_LENGTH);
		return -1;
	}

	// Checking null chars
	if (strlen(dcparser->mCommand.data()) < dcparser->mCommand.size()) {
		if (dcconn->Log(1)) {
			dcconn->LogStream() << "Sending null chars, probably attempt of an attack" << endl;
		}
		dcconn->CloseNow(CLOSE_REASON_BAD_CMD_NULL);
		return -2;
	}

	#ifndef WITHOUT_PLUGINS
		if (dcparser->miType != NMDC_TYPE_UNPARSED) {
			if (mDcServer->mCalls.mOnAny.CallAll(dcconn, dcparser)) {
				return 1;
			}
		}
	#endif

	if (dcconn->Log(5)) {
		dcconn->LogStream() << "[S]Stage " << dcparser->miType << endl;
	}
	
	switch (parser->miType) {

		case NMDC_TYPE_MSEARCH :
			// Fallthrough

		case NMDC_TYPE_MSEARCH_PAS :
			// Fallthrough

		case NMDC_TYPE_SEARCH_PAS :
			// Fallthrough

		case NMDC_TYPE_SEARCH :
			DC_Search(dcparser, dcconn);
			break;

		case NMDC_TYPE_SR :
			DC_SR(dcparser, dcconn);
			break;

		case NMDC_TYPE_MYNIFO :
			DC_MyINFO(dcparser, dcconn);
			break;

		case NMDC_TYPE_SUPPORTS :
			DC_Supports(dcparser, dcconn);
			break;

		case NMDC_TYPE_KEY :
			DC_Key(dcparser, dcconn);
			break;

		case NMDC_TYPE_VALIDATENICK :
			DC_ValidateNick(dcparser, dcconn);
			break;

		case NMDC_TYPE_VERSION :
			DC_Version(dcparser, dcconn);
			break;

		case NMDC_TYPE_GETNICKLIST :
			DC_GetNickList(dcparser, dcconn);
			break;

		case NMDC_TYPE_CHAT :
			DC_Chat(dcparser, dcconn);
			break;

		case NMDC_TYPE_TO :
			DC_To(dcparser, dcconn);
			break;

		case NMDC_TYPE_MYPASS :
			DC_MyPass(dcparser, dcconn);
			break;

		case NMDC_TYPE_CONNECTTOME :
			DC_ConnectToMe(dcparser, dcconn);
			break;

		case NMDC_TYPE_RCONNECTTOME :
			DC_RevConnectToMe(dcparser, dcconn);
			break;

		case NMDC_TYPE_MCONNECTTOME :
			DC_MultiConnectToMe(dcparser, dcconn);
			break;

		case NMDC_TYPE_KICK :
			DC_Kick(dcparser, dcconn);
			break;

		case NMDC_TYPE_OPFORCEMOVE :
			DC_OpForceMove(dcparser, dcconn);
			break;

		case NMDC_TYPE_GETINFO :
			DC_GetINFO(dcparser, dcconn);
			break;

		case NMDC_TYPE_MCTO :
			DC_MCTo(dcparser, dcconn);
			break;

		case NMDC_TYPE_PING :
			DC_Ping(dcparser, dcconn);
			break;

		case NMDC_TYPE_UNKNOWN :
			DC_Unknown(dcparser, dcconn);
			break;

		case NMDC_TYPE_QUIT :
			DC_Quit(dcparser, dcconn);
			break;

		case NMDC_TYPE_UNPARSED :
			dcparser->Parse();
			return DoCmd(parser, dcconn);

		default :
			if (Log(2)) {
				LogStream() << "Incoming untreated event" << endl;
			}
			break;

	}

	if (dcconn->Log(5)) {
		dcconn->LogStream() << "[E]Stage " << dcparser->miType << endl;
	}
	return 0;
}

int DcProtocol::DC_Supports(DcParser * dcparser, DcConn * dcconn) {
	ANTIFLOOD(NickList, FLOOD_TYPE_NICKLIST);

	string feature;
	istringstream is(dcparser->mCommand);
	is >> feature;
	dcconn->mFeatures = 0;
	is >> feature;
	while (feature.size()) {
		if (feature == "UserCommand") {
			dcconn->mFeatures |= SUPPORT_FEATUER_USERCOMMAND;
		} else if (feature == "NoGetINFO") {
			dcconn->mFeatures |= SUPPORT_FEATUER_NOGETINFO;
		} else if (feature == "NoHello") {
			dcconn->mFeatures |= SUPPORT_FEATUER_NOHELLO;
		} else if (feature == "UserIP2") {
			dcconn->mFeatures |= SUPPORT_FEATUER_USERIP2;
		} else if (feature == "TTHSearch") {
			dcconn->mFeatures |= SUPPORT_FEATUER_TTHSEARCH;
		} else if (feature == "QuickList") {
			dcconn->mFeatures |= SUPPORT_FEATUER_QUICKLIST;
		}
		feature.clear();
		is >> feature;
	}
	dcconn->msSupports.assign(dcparser->mCommand, 10, dcparser->mCommand.size() - 10);

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnSupports.CallAll(dcconn, dcparser)) {
			return -3;
		}
	#endif

	static string msg("$Supports UserCommand NoGetINFO NoHello UserIP2 MCTo"NMDC_SEPARATOR);
	dcconn->send(msg, false, false);

	return 0;
}

int DcProtocol::DC_Key(DcParser * dcparser, DcConn * dcconn) {
	BADFLAG("Key", LOGIN_STATUS_KEY);
	BADCMDPARAM("Key");

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnKey.CallAll(dcconn, dcparser)) {
			//string sLock, sKey;
			//sLock = Append_DC_Lock(sLock).substr(1, sLock.size() - 1);
			//Lock2Key(sLock, sKey);
		}
	#endif

	dcconn->SetLSFlag(LOGIN_STATUS_KEY); /** User has sent key */
	dcconn->ClearTimeOut(HUB_TIME_OUT_KEY);
	dcconn->SetTimeOut(HUB_TIME_OUT_VALNICK, mDcServer->mDcConfig.miTimeout[HUB_TIME_OUT_VALNICK], mDcServer->mTime);
	dcconn->mTimes.mKey.Get();
	return 0;
}

int DcProtocol::DC_ValidateNick(DcParser * dcparser, DcConn * dcconn) {
	BADFLAG("ValidateNick", LOGIN_STATUS_VALNICK);
	BADCMDPARAM("ValidateNick");

	string &sNick = dcparser->chunkString(CHUNK_1_PARAM);
	unsigned iNickLen = sNick.length();

	/** Additional checking the nick length */
	if (iNickLen > 0xFF) {
		if (dcconn->Log(1)) {
			dcconn->LogStream() << "Attempt to attack by long nick" << endl;
		}
		dcconn->CloseNow(CLOSE_REASON_LONG_NICK);
		return -1;
	}

	if (dcconn->Log(3)) {
		dcconn->LogStream() << "User " << sNick << " to validate nick" << endl;
	}

	try {
		DcUser *NewUser = new DcUser(sNick);
		if (!dcconn->SetUser(NewUser)) {
			dcconn->CloseNow(CLOSE_REASON_SETUSER);
			return -2;
		}
	} catch(...) {
		if (mDcServer->ErrLog(0)) {
			mDcServer->LogStream() << "Unhandled exception in DcProtocol::DC_ValidateNick" << endl;
		}
		if (dcconn->ErrLog(0)) {
			dcconn->LogStream() << "Error in SetUser closing" << endl;
		}
		dcconn->CloseNice(9000, CLOSE_REASON_SETUSER);
		return -2;
	}

	/** Checking validate user */
	if (!mDcServer->ValidateUser(dcconn, sNick)) {
		dcconn->CloseNice(9000, CLOSE_REASON_INVALID_USER);
		return -2;
	}

	/** Global user's limit */
	if (mDcServer->mDcConfig.miUsersLimit >= 0 && mDcServer->miTotalUserCount >= mDcServer->mDcConfig.miUsersLimit) {
		if (dcconn->Log(3)) {
			dcconn->LogStream() << "User " << sNick << " was disconnected (user's limit: " << mDcServer->mDcConfig.miUsersLimit << ")" << endl;
		}
		mDcServer->sendToUser(dcconn, mDcServer->mDCLang.msUsersLimit.c_str(), (char*)mDcServer->mDcConfig.msHubBot.c_str());
		dcconn->CloseNice(9000, CLOSE_REASON_USERS_LIMIT);
		return -3;
	}

	dcconn->SetLSFlag(LOGIN_STATUS_ALOWED);
	++mDcServer->miTotalUserCount;

	dcconn->SetLSFlag(LOGIN_STATUS_VALNICK | LOGIN_STATUS_NICKLST); /** We Install NICKLST because user can not call user list */
	dcconn->ClearTimeOut(HUB_TIME_OUT_VALNICK);

	string sMsg;

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnValidateNick.CallAll(dcconn, dcparser)) {
			dcconn->send(Append_DC_GetPass(sMsg)); /** We are sending the query for reception of the password */
			dcconn->SetTimeOut(HUB_TIME_OUT_PASS, mDcServer->mDcConfig.miTimeout[HUB_TIME_OUT_PASS], mDcServer->mTime);
			return -4;
		}
	#endif

	if (!mDcServer->CheckNickLength(dcconn, iNickLen)) {
		dcconn->CloseNice(9000, CLOSE_REASON_NICK_LEN);
	}
	dcconn->SetTimeOut(HUB_TIME_OUT_MYINFO, mDcServer->mDcConfig.miTimeout[HUB_TIME_OUT_MYINFO], mDcServer->mTime);
	dcconn->SetLSFlag(LOGIN_STATUS_PASSWD); /** Does not need password */

	dcconn->send(Append_DC_Hello(sMsg, dcconn->mDCUser->msNick)); /** Protection from change the command */
	return 0;
}

int DcProtocol::DC_MyPass(DcParser * dcparser, DcConn * dcconn) {
	if (!dcconn->mDCUser) { /* Check of existence of the user for current connection */
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "Mypass before validatenick" << endl;
		}
		mDcServer->sendToUser(dcconn, mDcServer->mDCLang.msBadLoginSequence.c_str(), (char *) mDcServer->mDcConfig.msHubBot.c_str());
		dcconn->CloseNice(9000, CLOSE_REASON_LOGIN_ERR);
		return -1;
	}
	BADFLAG("MyPass", LOGIN_STATUS_PASSWD);
	BADCMDPARAM("MyPass");

	// Checking the accepted password, otherwise send $BadPass|
	// or $Hello Nick|$LogedIn Nick|
	int bOp = 0;
	#ifndef WITHOUT_PLUGINS
		bOp = (mDcServer->mCalls.mOnMyPass.CallAll(dcconn, dcparser));
	#endif

	if (!mDcServer->CheckNickLength(dcconn, dcconn->mDCUser->msNick.length())) {
		dcconn->CloseNice(9000, CLOSE_REASON_NICK_LEN);
	}
	string sMsg;
	dcconn->SetLSFlag(LOGIN_STATUS_PASSWD); /** Password is accepted */
	Append_DC_Hello(sMsg, dcconn->mDCUser->msNick);
	if (bOp) { /** If entered operator, that sends command LoggedIn ($LogedIn !) */
		sMsg.append("$LogedIn ");
		sMsg.append(dcconn->mDCUser->msNick);
		sMsg.append(NMDC_SEPARATOR);
	}
	dcconn->send(sMsg);
	dcconn->ClearTimeOut(HUB_TIME_OUT_PASS);
	dcconn->SetTimeOut(HUB_TIME_OUT_MYINFO, mDcServer->mDcConfig.miTimeout[HUB_TIME_OUT_MYINFO], mDcServer->mTime);
	return 0;
}

int DcProtocol::DC_Version(DcParser * dcparser, DcConn * dcconn) {
	BADFLAG("Version", LOGIN_STATUS_VERSION);
	BADCMDPARAM("Version");

	string & sVersion = dcparser->chunkString(CHUNK_1_PARAM);
	if (dcconn->Log(3)) {
		dcconn->LogStream() << "Version:" << sVersion << endl;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnVersion.CallAll(dcconn, dcparser) && sVersion != "1,0091") {
			dcconn->CloseNice(9000, CLOSE_REASON_SYNTAX_VERSION); /** Checking of the version */
		}
	#endif

	dcconn->SetLSFlag(LOGIN_STATUS_VERSION); /** Version was checked */
	dcconn->mVersion = sVersion;
	return 0;
}

int DcProtocol::DC_GetNickList(DcParser * dcparser, DcConn * dcconn) {
	ANTIFLOOD(NickList, FLOOD_TYPE_NICKLIST);

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnGetNickList.CallAll(dcconn, dcparser)) {
			return 3;
		}
	#endif

	if (!dcconn->GetLSFlag(LOGIN_STATUS_MYINFO) && mDcServer->mDcConfig.mbNicklistOnLogin) {
		if (mDcServer->mDcConfig.mbDelayedLogin) {
			int LSFlag = dcconn->GetLSFlag(LOGIN_STATUS_LOGIN_DONE);
			if (LSFlag & LOGIN_STATUS_NICKLST) {
				LSFlag -= LOGIN_STATUS_NICKLST;
			}
			dcconn->ReSetLSFlag(LSFlag);
		}
		dcconn->mbSendNickList = true;
		return 0;
	}
	return SendNickList(dcconn);
}

int DcProtocol::DC_MyINFO(DcParser * dcparser, DcConn * dcconn) {
	/** Check syntax of the command */
	if (dcparser->SplitChunks()) {
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "MyINFO syntax error, closing" << endl;
		}
		mDcServer->sendToUser(dcconn, mDcServer->mDCLang.msMyinfoError.c_str(), (char *) mDcServer->mDcConfig.msHubBot.c_str());
		dcconn->CloseNice(9000, CLOSE_REASON_SYNTAX_MYINFO);
		return -1;
	}

	ANTIFLOOD(MyINFO, FLOOD_TYPE_MYNIFO);

	const string & sNick = dcparser->chunkString(CHUNK_MI_NICK);

	/** Check existence user, otherwise check support QuickList */
	if (!dcconn->mDCUser) {
		//if (QuickList)
		//	dcconn->mDCUser->msNick = sNick;
		//} else
		{
			if (dcconn->Log(2)) {
				dcconn->LogStream() << "Myinfo without user: " << dcparser->mCommand << endl;
			}
			mDcServer->sendToUser(dcconn, mDcServer->mDCLang.msBadLoginSequence.c_str(), (char *) mDcServer->mDcConfig.msHubBot.c_str());
			dcconn->CloseNice(9000, CLOSE_REASON_MYINFO_WITHOUT_USER);
			return -2;
		}
	} else if (sNick != dcconn->mDCUser->msNick) { /** Проверка ника */
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "Bad nick in MyINFO, closing" << endl;
		}
		mDcServer->sendToUser(dcconn, mDcServer->mDCLang.msBadMyinfoNick.c_str(), (char *) mDcServer->mDcConfig.msHubBot.c_str());
		dcconn->CloseNice(9000, CLOSE_REASON_BAD_MYINFO_NICK);
		return -1;
	}

	string sOldMyINFO = dcconn->mDCUser->getMyINFO();
	dcconn->mDCUser->setMyINFO(dcparser);

	int iMode = 0;
	#ifndef WITHOUT_PLUGINS
		iMode = mDcServer->mCalls.mOnMyINFO.CallAll(dcconn, dcparser);
	#endif

	if (iMode != 1 && dcconn->mDCUser->mbInUserList) {
		if (sOldMyINFO != dcconn->mDCUser->getMyINFO()) {
			if (dcconn->mDCUser->mbHide) {
				dcconn->send(dcparser->mCommand, true); // Send to self only
			} else {
				SendMode(dcconn, dcparser->mCommand, iMode, mDcServer->mDCUserList, true); // Use cache for send to all
				//mDcServer->mDCUserList.sendToAll(dcparser->mCommand, true/*mDcServer->mDcConfig.mbDelayedMyINFO*/); // Send to all
			}
		}
	} else if (!dcconn->mDCUser->mbInUserList) {
		dcconn->SetLSFlag(LOGIN_STATUS_MYINFO);
		if (!mDcServer->BeforeUserEnter(dcconn)) {
			return -1;
		}
		dcconn->ClearTimeOut(HUB_TIME_OUT_MYINFO);
	}
	return 0;
}

int DcProtocol::DC_Chat(DcParser * dcparser, DcConn * dcconn) {
	if (dcparser->SplitChunks()) {
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "Chat syntax error, closing" << endl;
		}
		dcconn->CloseNow(CLOSE_REASON_SYNTAX_CHAT);
		return -1;
	}

	ANTIFLOOD(Chat, FLOOD_TYPE_CHAT);

	if (!dcconn->mDCUser) {
		return -2;
	}
	if (!dcconn->mDCUser->mbInUserList) {
		return -3;
	}

	/** Check chat nick */
	if ((dcparser->chunkString(CHUNK_CH_NICK) != dcconn->mDCUser->msNick) ) {
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "Bad nick in chat, closing" << endl;
		}
		string sMsg = mDcServer->mDCLang.msBadChatNick;
		StringReplace(sMsg, string("nick"), sMsg, dcparser->chunkString(CHUNK_CH_NICK));
		StringReplace(sMsg, string("real_nick"), sMsg, dcconn->mDCUser->msNick);
		mDcServer->sendToUser(dcconn, sMsg.c_str(), (char*)mDcServer->mDcConfig.msHubBot.c_str());
		dcconn->CloseNice(9000, CLOSE_REASON_CHAT_NICK);
		return -2;
	}
	int iMode = 0;
	#ifndef WITHOUT_PLUGINS
		iMode = mDcServer->mCalls.mOnChat.CallAll(dcconn, dcparser);
	#endif

	//Hash<unsigned long> hash;
	//unsigned long key = hash(dcparser->mCommand);
	//cout << key << endl;

	/** Sending message */
	SendMode(dcconn, dcparser->mCommand, iMode, mDcServer->mChatList, false); // Don't use cache for send to all
	return 0;
}

int DcProtocol::DC_To(DcParser * dcparser, DcConn * dcconn) {
	if (dcparser->SplitChunks()) {
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "PM syntax error, closing" << endl;
		}
		dcconn->CloseNow(CLOSE_REASON_SYNTAX_TO);
		return -1;
	}

	ANTIFLOOD(To, FLOOD_TYPE_TO);

	if (!dcconn->mDCUser) {
		return -2;
	}
	if (!dcconn->mDCUser->mbInUserList) {
		return -3;
	}
	string & sNick = dcparser->chunkString(CHUNK_PM_TO);

	/** Checking the coincidence nicks in command */
	if (dcparser->chunkString(CHUNK_PM_FROM) != dcconn->mDCUser->msNick || dcparser->chunkString(CHUNK_PM_NICK) != dcconn->mDCUser->msNick) {
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "Bad nick in PM, closing" <<endl;
		}
		dcconn->CloseNow(CLOSE_REASON_BAD_NICK_PM);
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnTo.CallAll(dcconn, dcparser)) {
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

int DcProtocol::DC_MCTo(DcParser * dcparser, DcConn * dcconn) {
	if (dcparser->SplitChunks()) {
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "MCTo syntax error, closing" << endl;
		}
		dcconn->CloseNow(CLOSE_REASON_SYNTAX_MCTO);
		return -1;
	}

	ANTIFLOOD(MCTo, FLOOD_TYPE_MCTO);

	if (!dcconn->mDCUser) {
		return -2;
	}
	if (!dcconn->mDCUser->mbInUserList) {
		return -3;
	}

	string & sNick = dcparser->chunkString(CHUNK_MC_TO);

	/** Checking the coincidence nicks in command */
	if (dcparser->chunkString(CHUNK_MC_FROM) != dcconn->mDCUser->msNick) {
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "Bad nick in MCTo, closing" <<endl;
		}
		dcconn->CloseNow(CLOSE_REASON_BAD_NICK_MCTO);
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnMCTo.CallAll(dcconn, dcparser)) {
			return 0;
		}
	#endif

	/** Search user */
	DcUser * User = (DcUser *)mDcServer->mDCUserList.GetUserBaseByNick(sNick);
	if (!User) {
		return -2;
	}

	string msg;
	User->send(Append_DC_Chat(msg, dcconn->mDCUser->msNick, dcparser->chunkString(CHUNK_MC_MSG)));
	if (dcconn->mDCUser->msNick != sNick) {
		dcconn->send(msg);
	}
	return 0;
}

/**
	NMDC_TYPE_SEARCH
	NMDC_TYPE_SEARCH_PAS
	NMDC_TYPE_MSEARCH
	NMDC_TYPE_MSEARCH_PAS
*/
int DcProtocol::DC_Search(DcParser * dcparser, DcConn * dcconn) {
	if (dcparser->SplitChunks()) {
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "Search syntax error, closing" << endl;
		}
		dcconn->CloseNow(CLOSE_REASON_SYNTAX_SEARCH);
		return -1;
	}

	ANTIFLOOD(Search, FLOOD_TYPE_SEARCH);

	if (!dcconn->mDCUser || !dcconn->mDCUser->mbInUserList) {
		return -2;
	}

	// TODO: Check overloading of the system

	int iMode = 0;
	#ifndef WITHOUT_PLUGINS
		iMode = mDcServer->mCalls.mOnSearch.CallAll(dcconn, dcparser);
	#endif

	/** Sending cmd */
	string sMsg;

	switch (dcparser->miType) {

		case NMDC_TYPE_SEARCH :
			if (mDcServer->mDcConfig.mbCheckSearchIp && dcconn->msIp != dcparser->chunkString(CHUNK_AS_IP)) {
				sMsg = mDcServer->mDCLang.msBadSearchIp;
				if (dcconn->Log(2)) {
					dcconn->LogStream() << "Bad ip in active search, closing" << endl;
				}
				StringReplace(sMsg, string("ip"), sMsg, dcparser->chunkString(CHUNK_AS_IP));
				StringReplace(sMsg, string("real_ip"), sMsg, dcconn->msIp);
				mDcServer->sendToUser(dcconn, sMsg.c_str(), (char *) mDcServer->mDcConfig.msHubBot.c_str());
				dcconn->CloseNice(9000, CLOSE_REASON_NICK_SEARCH);
				return -1;
			}
			SendMode(dcconn, dcparser->mCommand, iMode, mDcServer->mDCUserList, true); // Use cache for send to all
			break;

		case NMDC_TYPE_SEARCH_PAS :
			dcconn->miSRCounter = 0; /** Zeroizing result counter of the passive search */
			SendMode(dcconn, dcparser->mCommand, iMode, mDcServer->mActiveList, true); // Use cache for send to all
			break;

		case NMDC_TYPE_MSEARCH :
			if (mDcServer->mDcConfig.mbCheckSearchIp && (dcconn->msIp != dcparser->chunkString(CHUNK_AS_IP))) {
				sMsg = mDcServer->mDCLang.msBadSearchIp;
				if (dcconn->Log(2)) {
					dcconn->LogStream() << "Bad ip in active search, closing" << endl;
				}
				StringReplace(sMsg, string("ip"), sMsg, dcparser->chunkString(CHUNK_AS_IP));
				StringReplace(sMsg, string("real_ip"), sMsg, dcconn->msIp);
				mDcServer->sendToUser(dcconn, sMsg.c_str(), (char *) mDcServer->mDcConfig.msHubBot.c_str());
				dcconn->CloseNice(9000, CLOSE_REASON_NICK_SEARCH);
				return -1;
			}
			sMsg = "$Search ";
			sMsg += dcparser->chunkString(CHUNK_AS_ADDR);
			sMsg += ' ';
			sMsg += dcparser->chunkString(CHUNK_AS_QUERY);
			SendMode(dcconn, sMsg, iMode, mDcServer->mDCUserList, true); // Use cache for send to all
			break;

		case NMDC_TYPE_MSEARCH_PAS :
			dcconn->miSRCounter = 0; /** Zeroizing result counter of the passive search */
			SendMode(dcconn, dcparser->mCommand, iMode, mDcServer->mActiveList, true); // Use cache for send to all
			break;

		default :
			return -4;

	}
	return 0;
}

int DcProtocol::DC_SR(DcParser * dcparser, DcConn * dcconn) {
	if (dcparser->SplitChunks()) {
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "SR syntax error, closing" << endl;
		}
		dcconn->CloseNow(CLOSE_REASON_SYNTAX_SR);
		return -1;
	}

	ANTIFLOOD(SR, FLOOD_TYPE_SR);

	if (!dcconn->mDCUser || !dcconn->mDCUser->mbInUserList) {
		return -2;
	}

	/** Check same nick in cmd (PROTOCOL NMDC) */
	if (mDcServer->mDcConfig.mbCheckSRNick && (dcconn->mDCUser->msNick != dcparser->chunkString(CHUNK_SR_FROM))) {
		string sMsg = mDcServer->mDCLang.msBadSRNick;
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "Bad nick in search response, closing" << endl;
		}
		StringReplace(sMsg, "nick", sMsg, dcparser->chunkString(CHUNK_SR_FROM));
		StringReplace(sMsg, "real_nick", sMsg, dcconn->mDCUser->msNick);
		mDcServer->sendToUser(dcconn, sMsg.c_str(), (char*)mDcServer->mDcConfig.msHubBot.c_str());
		dcconn->CloseNice(9000, CLOSE_REASON_NICK_SR);
		return -1;
	}

	DcUser * User = (DcUser *)mDcServer->mDCUserList.GetUserBaseByNick(dcparser->chunkString(CHUNK_SR_TO));

	/** Is user exist? */
	if (!User || !User->mDCConn) {
		return -2;
	}

	/** != 0 - error */
	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnSR.CallAll(dcconn, dcparser)) {
			return -3;
		}
	#endif

	/** Sending cmd */
	if (!mDcServer->mDcConfig.miMaxPassiveRes || (User->mDCConn->miSRCounter++ < unsigned(mDcServer->mDcConfig.miMaxPassiveRes))) {
		string sStr(dcparser->mCommand, 0, dcparser->mChunks[CHUNK_SR_TO].first - 1); /** Remove nick on the end of cmd */
		User->mDCConn->send(sStr, true, false);
	}
	return 0;
}


int DcProtocol::DC_ConnectToMe(DcParser * dcparser, DcConn * dcconn) {
	if (dcparser->SplitChunks()) {
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "CTM syntax error, closing" << endl;
		}
		dcconn->CloseNow(CLOSE_REASON_SYNTAX_CTM);
		return -1;
	}

	ANTIFLOOD(CTM, FLOOD_TYPE_CTM);

	if (!dcconn->mDCUser || !dcconn->mDCUser->mbInUserList) {
		return -1;
	}

	if (mDcServer->mDcConfig.mbCheckCTMIp && dcconn->msIp != dcparser->chunkString(CHUNK_CM_IP)) {
		string sMsg = mDcServer->mDCLang.msBadCTMIp;
		StringReplace(sMsg, string("ip"), sMsg, dcparser->chunkString(CHUNK_CM_IP));
		StringReplace(sMsg, string("real_ip"), sMsg, dcconn->msIp);
		mDcServer->sendToUser(dcconn, sMsg.c_str(), (char *) mDcServer->mDcConfig.msHubBot.c_str());
		dcconn->CloseNice(9000, CLOSE_REASON_NICK_CTM);
		return -1;
	}

	DcUser * User = (DcUser *)mDcServer->mDCUserList.GetUserBaseByNick(dcparser->chunkString(CHUNK_CM_NICK));
	if (!User) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnConnectToMe.CallAll(dcconn, dcparser)) {
			return -2;
		}
	#endif

	User->send(dcparser->mCommand, true);
	return 0;
}

int DcProtocol::DC_RevConnectToMe(DcParser * dcparser, DcConn * dcconn) {
	if (dcparser->SplitChunks()) {
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "RCTM syntax error, closing" << endl;
		}
		dcconn->CloseNow(CLOSE_REASON_SYNTAX_RCTM);
		return -1;
	}

	ANTIFLOOD(RCTM, FLOOD_TYPE_RCTM);

	if (!dcconn->mDCUser || !dcconn->mDCUser->mbInUserList) {
		return -1;
	}

	/** Checking the nick */
	if (mDcServer->mDcConfig.mbCheckRctmNick && (dcparser->chunkString(CHUNK_RC_NICK) != dcconn->mDCUser->msNick)) {
		string sMsg = mDcServer->mDCLang.msBadRevConNick;
		StringReplace(sMsg, string("nick"), sMsg, dcparser->chunkString(CHUNK_RC_NICK));
		StringReplace(sMsg, string("real_nick"), sMsg, dcconn->mDCUser->msNick);
		mDcServer->sendToUser(dcconn, sMsg.c_str(), (char *) mDcServer->mDcConfig.msHubBot.c_str());
		dcconn->CloseNice(9000, CLOSE_REASON_NICK_RCTM);
		return -1;
	}

	/** Searching the user */
	DcUser * other = (DcUser *)mDcServer->mDCUserList.GetUserBaseByNick(dcparser->chunkString(CHUNK_RC_OTHER));
	if (!other) {
		return -2;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnRevConnectToMe.CallAll(dcconn, dcparser)) {
			return -2;
		}
	#endif

	other->send(dcparser->mCommand, true);
	return 0;
}

int DcProtocol::DC_MultiConnectToMe(DcParser *, DcConn * dcconn) {
	ANTIFLOOD(CTM, FLOOD_TYPE_CTM);
	return 0;
}

int DcProtocol::DC_Kick(DcParser * dcparser, DcConn * dcconn) {
	if (dcparser->SplitChunks()) {
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "Kick syntax error, closing" << endl;
		}
		dcconn->CloseNow(CLOSE_REASON_SYNTAX_KICK);
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnKick.CallAll(dcconn, dcparser)) {
			return -1;
		}
	#endif

	if (!dcconn->mDCUser || !dcconn->mDCUser->mbKick) {
		return -2;
	}

	DcUser * User = (DcUser *)mDcServer->mDCUserList.GetUserBaseByNick(dcparser->chunkString(CHUNK_1_PARAM));

	/** Is user exist? */
	if (!User || !User->mDCConn) {
		return -3;
	}

	User->mDCConn->CloseNice(9000, CLOSE_REASON_KICK);
	return 0;
}

int DcProtocol::DC_OpForceMove(DcParser * dcparser, DcConn * dcconn) {
	if (dcparser->SplitChunks()) {
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "OpForceMove syntax error, closing" << endl;
		}
		dcconn->CloseNow(CLOSE_REASON_SYNTAX_OFM);
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnOpForceMove.CallAll(dcconn, dcparser)) {
			return -1;
		}
	#endif

	if (!dcconn->mDCUser || !dcconn->mDCUser->mbForceMove) {
		return -2;
	}

	DcUser * User = (DcUser *)mDcServer->mDCUserList.GetUserBaseByNick(dcparser->chunkString(CHUNK_FM_NICK));

	/** Is user exist? */
	if (!User || !User->mDCConn || !dcparser->chunkString(CHUNK_FM_DEST).size()) {
		return -3;
	}

	mDcServer->forceMove(User->mDCConn, dcparser->chunkString(CHUNK_FM_DEST).c_str(), dcparser->chunkString(CHUNK_FM_REASON).c_str());
	return 0;
}

int DcProtocol::DC_GetINFO(DcParser * dcparser, DcConn * dcconn) {
	if (dcparser->SplitChunks()) {
		if (dcconn->Log(2)) {
			dcconn->LogStream() << "GetINFO syntax error, closing" << endl;
		}
		dcconn->CloseNow(CLOSE_REASON_SYNTAX_GETINFO);
		return -1;
	}
	if (!dcconn->mDCUser || !dcconn->mDCUser->mbInUserList) {
		return -1;
	}

	DcUser * User = (DcUser *)mDcServer->mDCUserList.GetUserBaseByNick(dcparser->chunkString(CHUNK_GI_OTHER));
	if (!User) {
		return -2;
	}

	if (dcconn->mDCUser->mTimeEnter < User->mTimeEnter && Time() < (User->mTimeEnter + 60000)) {
		return 0;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnGetINFO.CallAll(dcconn, dcparser)) {
			return -2;
		}
	#endif

	//if(!(dcconn->mFeatures & SUPPORT_FEATUER_NOGETINFO)){
	if (!User->mbHide) {
		dcconn->send(string(User->getMyINFO()), true, false);
	}
	return 0;
}

int DcProtocol::DC_Ping(DcParser *, DcConn * dcconn) {
	ANTIFLOOD(Ping, FLOOD_TYPE_PING);
	return 0;
}

int DcProtocol::DC_Unknown(DcParser * dcparser, DcConn * dcconn) {

	// Protection from commands, not belonging to DC protocol
	if (dcparser->mCommand.compare(0, 1, "$") && mDcServer->mDcConfig.mbDisableNoDCCmd) {
		if (dcconn->Log(1)) {
			dcconn->LogStream() << "Bad DC cmd: " << dcparser->mCommand.substr(0, 10) << " ..., close" << endl;
		}
		dcconn->CloseNow(CLOSE_REASON_BAD_DC_CMD);
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
	if (!mDcServer->mCalls.mOnUnknown.CallAll(dcconn, dcparser)) {
		dcconn->CloseNice(9000, CLOSE_REASON_UNKNOWN_CMD);
		return -2;
	}
	#endif

	ANTIFLOOD(Unknown, FLOOD_TYPE_UNKNOWN);
	return 0;
}

int DcProtocol::DC_Quit(DcParser *, DcConn * dcconn) {
	dcconn->CloseNice(9000, CLOSE_REASON_QUIT);
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





void DcProtocol::SendMode(DcConn * dcconn, const string & str, int iMode, UserList & UL, bool bUseCache) {
	bool bAddSep = false;
	if (str.substr(str.size() - 1, 1) != NMDC_SEPARATOR) {
		bAddSep = true;
	}

	if (iMode == 0) { /** Send to all */
		UL.sendToAll(str, bUseCache, bAddSep);
	} else if (iMode == 3) { /** Send to all except current user */
		if (dcconn->mDCUser->mbInUserList) {
			dcconn->mDCUser->mbInUserList = false;
			UL.sendToAll(str, bUseCache, bAddSep);
			dcconn->mDCUser->mbInUserList = true;
		}
	} else if (iMode == 4) { /** Send to all except users with ip of the current user */
		DcConn * conn = NULL;
		vector<DcConn *> ul;
		for (DcIpList::iterator mit = mDcServer->mIPListConn->begin(DcConn::Ip2Num(dcconn->getIp().c_str())); mit != mDcServer->mIPListConn->end(); ++mit) {
			conn = (DcConn *)(*mit);
			if(conn->mDCUser && conn->mDCUser->mbInUserList) {
				conn->mDCUser->mbInUserList = false;
				ul.push_back(conn);
			}
		}
		UL.sendToAll(str, bUseCache, bAddSep);
		for (vector<DcConn *>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it) {
			(*ul_it)->mDCUser->mbInUserList = true;
		}
	}
}


/** Sending the user-list and op-list */
int DcProtocol::SendNickList(DcConn * dcconn) {
	try {
		if ((dcconn->GetLSFlag(LOGIN_STATUS_LOGIN_DONE) != LOGIN_STATUS_LOGIN_DONE) && mDcServer->mDcConfig.mbNicklistOnLogin) {
			dcconn->mbNickListInProgress = true;
		}

		if (dcconn->mFeatures & SUPPORT_FEATUER_NOHELLO) {
			if (dcconn->Log(3)) {
				dcconn->LogStream() << "Sending MyINFO list" << endl;
			}
			// seperator "|" was added in GetInfoList function
			dcconn->send(mDcServer->mDCUserList.GetInfoList(true), false, false);
		} else if (dcconn->mFeatures & SUPPORT_FEATUER_NOGETINFO) {
			if (dcconn->Log(3)) {
				dcconn->LogStream() << "Sending MyINFO list and Nicklist" << endl;
			}
			// seperator "|" was not added in GetNickList function, because seperator was "$$"
			dcconn->send(mDcServer->mDCUserList.GetNickList(), true, false);
			// seperator "|" was added in GetInfoList function
			dcconn->send(mDcServer->mDCUserList.GetInfoList(true), false, false);
		} else {
			if (dcconn->Log(3)) {
				dcconn->LogStream() << "Sending Nicklist" << endl;
			}
			// seperator "|" was not added in GetNickList function, because seperator was "$$"
			dcconn->send(mDcServer->mDCUserList.GetNickList(), true, false);
		}
		if (mDcServer->mOpList.Size()) {
			if (dcconn->Log(3)) {
				dcconn->LogStream() << "Sending Oplist" << endl;
			}
			// seperator "|" was not added in GetNickList function, because seperator was "$$"
			dcconn->send(mDcServer->mOpList.GetNickList(), true, false);
		}

		if (dcconn->mDCUser->mbInUserList && dcconn->mDCUser->mbInIpList) {
			if (dcconn->Log(3)) {
				dcconn->LogStream() << "Sending Iplist" << endl;
			}
			// seperator "|" was not added in GetIpList function, because seperator was "$$"
			dcconn->send(mDcServer->mDCUserList.GetIpList(), true);
		} else {
			if (!dcconn->SendBufIsEmpty()) { // buf would not flush, if it was empty
				dcconn->Flush(); // newPolitic
			} else {
				static string s(NMDC_SEPARATOR);
				dcconn->send(s);
			}
		}
	} catch(...) {
		if (dcconn->ErrLog(0)) {
			dcconn->LogStream() << "exception in SendNickList" << endl;
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
	os << ::std::floor(s * 1000 + 0.5) / 1000 << " " << DcServer::currentDcServer->mDCLang.msUnits[i];
	return os.str();
}


}; // namespace protocol

}; // namespace dcserver
