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
 
#include "AdcProtocol.h"
#include "DcServer.h" // for mDcServer
#include "DcConn.h" // for DcConn

namespace dcserver {

namespace protocol {


AdcProtocol::AdcProtocol() :
	mDcServer(NULL),
	mSidNum(0)
{
	setClassName("AdcProtocol");

	events[ADC_TYPE_SUP] = &AdcProtocol::eventSup;
	events[ADC_TYPE_STA] = &AdcProtocol::eventSta;
	events[ADC_TYPE_INF] = &AdcProtocol::eventInf;
	events[ADC_TYPE_MSG] = &AdcProtocol::eventMsg;
	events[ADC_TYPE_SCH] = &AdcProtocol::eventSch;
	events[ADC_TYPE_RES] = &AdcProtocol::eventRes;
	events[ADC_TYPE_CTM] = &AdcProtocol::eventCtm;
	events[ADC_TYPE_RCM] = &AdcProtocol::eventRcm;
	events[ADC_TYPE_GPA] = &AdcProtocol::eventGpa;
	events[ADC_TYPE_PAS] = &AdcProtocol::eventPas;
	events[ADC_TYPE_QUI] = &AdcProtocol::eventQui;
	events[ADC_TYPE_GET] = &AdcProtocol::eventGet;
	events[ADC_TYPE_GFI] = &AdcProtocol::eventGfi;
	events[ADC_TYPE_SND] = &AdcProtocol::eventSnd;
	events[ADC_TYPE_SID] = &AdcProtocol::eventSid;
	events[ADC_TYPE_CMD] = &AdcProtocol::eventCmd;
	events[ADC_TYPE_NAT] = &AdcProtocol::eventNat;
	events[ADC_TYPE_RNT] = &AdcProtocol::eventRnt;
	events[ADC_TYPE_PSR] = &AdcProtocol::eventPsr;
	events[ADC_TYPE_PUB] = &AdcProtocol::eventPub;
	events[ADC_TYPE_UNKNOWN] = &AdcProtocol::eventUnknown;
}



AdcProtocol::~AdcProtocol() {
}



const char * AdcProtocol::getSeparator() const {
	return ADC_SEPARATOR;
}



size_t AdcProtocol::getSeparatorLen() const {
	return ADC_SEPARATOR_LEN;
}



unsigned long AdcProtocol::getMaxCommandLength() const {
	return 10240;
}



Parser * AdcProtocol::createParser() {
	return new AdcParser;
}



void AdcProtocol::deleteParser(Parser * parser) {
	if (parser != NULL) {
		delete parser;
	}
}



int AdcProtocol::doCommand(Parser * parser, Conn * conn) {

	AdcParser * adcParser = static_cast<AdcParser *> (parser);
	DcConn * dcConn = static_cast<DcConn *> (conn);

//	if (checkCommand(adcParser, dcConn) < 0) {
//		return -1;
//	}

	#ifndef WITHOUT_PLUGINS
//		if (mDcServer->mCalls.mOnAny.callAll(dcConn->mDcUser, adcParser->mType)) {
//			return 1;
//		}
	#endif

	#ifdef _DEBUG
		if (adcParser->mType == TYPE_UNPARSED) {
			throw "Unparsed command";
		}
	#endif

	if (dcConn->log(5)) {
		dcConn->logStream() << "[S]Stage " << adcParser->mType << endl;
	}

	(this->*(this->events[adcParser->mType])) (adcParser, dcConn);

	if (dcConn->log(5)) {
		dcConn->logStream() << "[E]Stage " << adcParser->mType << endl;
	}
	return 0;
}



Conn * AdcProtocol::getConnForUdpData(Conn *, Parser *) {
	return NULL;
}



int AdcProtocol::onNewConn(Conn * conn) {

	DcConn * dcConn = static_cast<DcConn *> (conn);

	const char * sid = NULL;
	UserBase * userBase = NULL;
	do {
		sid = getSid(++mSidNum);
		userBase = mDcServer->mAdcUserList.getUserBaseByUid(sid);
	} while (userBase != NULL || strcmp(sid, "AAAA") == 0); // "AAAA" is a hub

	dcConn->mDcUser->setUid(string(sid));

	string cmds("ISUP ADBASE ADTIGR");
	cmds.append(ADC_SEPARATOR);
	cmds.append("ISID ").append(sid);
	dcConn->send(cmds, true);

	return 0;
}





int AdcProtocol::eventSup(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventSta(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventInf(AdcParser * adcParser, DcConn * dcConn) {

	string inf = adcParser->mCommand;
	string oldInf = dcConn->mDcUser->getInf();

	size_t pos = inf.find(" PD");
	if (pos != inf.npos) {
		size_t pos_s = inf.find(" ", pos + 3);
		if (pos_s != inf.npos) {
			inf.erase(pos, pos_s - pos);
		}
	}

	dcConn->mDcUser->setInf(inf);

	if (/*mode != 1 &&*/ dcConn->mDcUser->getInUserList()) {
		if (oldInf != dcConn->mDcUser->getInf()) {
			if (dcConn->mDcUser->getHide()) {
				dcConn->send(dcConn->mDcUser->getInf(), true); // Send to self only
			} else {
				//sendMode(dcConn, dcConn->mDcUser->getInf(), mode, mDcServer->mDcUserList, true); // Use cache for send to all
				mDcServer->mAdcUserList.sendToAllAdc(dcConn->mDcUser->getInf(), true);
			}
		}
	} else if (!dcConn->mDcUser->getInUserList()) {

		string hubInf("IINF CT32 VE" INTERNALVERSION " NIADC" INTERNALNAME);
		hubInf.append(" DE").append(mDcServer->mDcConfig.mTopic);
		dcConn->send(hubInf, true);
		
		dcConn->mSendNickList = true;
		mDcServer->beforeUserEnter(dcConn);
	}

	return 0;
}



int AdcProtocol::eventMsg(AdcParser * adcParser, DcConn * dcConn) {

	if (!dcConn->mDcUser->getInUserList()) {
		return -1;
	}

	if (adcParser->mHeader == HEADER_ECHO) {
		size_t pos = adcParser->mCommand.find(" PM");
		if (pos != adcParser->mCommand.npos) {
			size_t pos1 = adcParser->mCommand.find(" ", 5);
			if (pos1 != adcParser->mCommand.npos) {
				size_t pos2 = adcParser->mCommand.find(" ", pos1 + 1);
				if (pos2 != adcParser->mCommand.npos) {
					string sid;
					sid.assign(adcParser->mCommand, pos1 + 1, pos2 - pos1 - 1);

					// Search user
					DcUser * dcUser = static_cast<DcUser *> (mDcServer->mAdcUserList.getUserBaseByUid(sid));
					if (!dcUser) {
						return -2;
					}
					dcUser->send(adcParser->mCommand, true);
					dcConn->mDcUser->send(adcParser->mCommand, true);
				}
			}
		}
		return 0;
	}

	/*
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
	*/

	/** Sending message */
	if (adcParser->mHeader == HEADER_BROADCAST) {
		mDcServer->mChatList.sendToAllAdc(adcParser->mCommand, false);
	}
	//sendMode(dcConn, adcparser->mCommand, mode, mDcServer->mChatList, false); // Don't use cache for send to all
	return 0;
}



int AdcProtocol::eventSch(AdcParser * adcParser, DcConn *) {

	//todo HEADER_FEATURE

	if (adcParser->mHeader == HEADER_BROADCAST) {
		mDcServer->mAdcUserList.sendToAllAdc(adcParser->mCommand, true);
	}
	return 0;
}



int AdcProtocol::eventRes(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventCtm(AdcParser * adcParser, DcConn *) {

	if (adcParser->mHeader == HEADER_DIRECT) {
		size_t pos1 = adcParser->mCommand.find(" ", 5);
		if (pos1 != adcParser->mCommand.npos) {
			size_t pos2 = adcParser->mCommand.find(" ", pos1 + 1);
			if (pos2 != adcParser->mCommand.npos) {
				string sid;
				sid.assign(adcParser->mCommand, pos1 + 1, pos2 - pos1 - 1);

				// Search user
				DcUser * dcUser = static_cast<DcUser *> (mDcServer->mAdcUserList.getUserBaseByUid(sid));
				if (!dcUser) {
					return -2;
				}
				dcUser->send(adcParser->mCommand, true);
			}
		}
	}

	return 0;
}



int AdcProtocol::eventRcm(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventGpa(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventPas(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventQui(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventGet(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventGfi(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventSnd(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventSid(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventCmd(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventNat(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventRnt(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventPsr(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventPub(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventUnknown(AdcParser *, DcConn *) {
	return 0;
}



void AdcProtocol::infList(string & list, UserBase * userBase) {
	// INF ...\nINF ...\n
	if (!userBase->hide()) {
		list.append(userBase->getInf());
		list.append(ADC_SEPARATOR);
	}
}



const char * AdcProtocol::getSid(unsigned int num) {
	static const char* base32 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
	static char ret[5];
	ret[0] = base32[num >> 0xF & 0x1F];
	ret[1] = base32[num >> 0xA & 0x1F];
	ret[2] = base32[num >> 0x5 & 0x1F];
	ret[3] = base32[num & 0x1F];
	ret[4] = 0;
	return ret;
}



/// Sending the user-list
int AdcProtocol::sendNickList(DcConn * dcConn) {
	try {

		if (mDcServer->mEnterList.find(dcConn->mDcUser->getUidHash())) {
			dcConn->mNickListInProgress = true;
		}
/*
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
*/

		dcConn->send(mDcServer->mAdcUserList.getList(), true);
	} catch(...) {
		if (dcConn->errLog(0)) {
			dcConn->logStream() << "exception in sendNickList" << endl;
		}
		return -1;
	}
	return 0;
}



void AdcProtocol::onFlush(Conn * conn) {
	DcConn * dcConn = static_cast<DcConn *> (conn);
	if (dcConn->mNickListInProgress) {
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


}; // namespace protocol

}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
