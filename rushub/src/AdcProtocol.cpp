/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
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
 
#include "AdcProtocol.h"
#include "DcServer.h" // for mDcServer
#include "DcConn.h" // for DcConn
#include "TigerHash.h"
#include "Encoder.h"

namespace dcserver {

namespace protocol {


AdcProtocol::AdcProtocol() :
	DcProtocol(),
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
	events[ADC_TYPE_VOID] = &AdcProtocol::eventVoid;
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



unsigned int AdcProtocol::getMaxCommandLength() const {
	return 10240u;
}



Parser * AdcProtocol::createParser() {
	return new AdcParser;
}



void AdcProtocol::deleteParser(Parser * parser) {
	if (parser != NULL) {
		delete parser;
	}
}



static void escaper(char c, string & dst) {

	switch (c) {

		case ' ':
			dst += '\\';
			dst += 's';
			break;

		case '\n':
			dst += '\\';
			dst += 'n';
			break;

		case '\\': // Fallthrough
			dst += '\\';

		default:
			dst += c;
			break;

	}
}



int AdcProtocol::doCommand(Parser * parser, Conn * conn) {

	AdcParser * adcParser = static_cast<AdcParser *> (parser);
	DcConn * dcConn = static_cast<DcConn *> (conn);

	LOG(LEVEL_TRACE, "doCommand");

	if (checkCommand(adcParser, dcConn) < 0) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnAny.callAll(dcConn->mDcUser, adcParser->mType)) {
			return 1;
		}
	#endif

	#ifdef _DEBUG
		if (adcParser->mType == TYPE_UNPARSED) {
			throw "Unparsed command";
		}
	#endif

	LOG_CLASS(dcConn, LEVEL_TRACE, "[S]Stage " << adcParser->mType);

	if (0 == (this->*(this->events[adcParser->mType])) (adcParser, dcConn)) {
		DcUser * dcUser = NULL;
		switch(adcParser->getHeader()) {

			case HEADER_BROADCAST:
				mDcServer->mDcUserList.sendToAllAdc(adcParser->mCommand, true, false);
				break;

			case HEADER_DIRECT:
				// find target user
				dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(adcParser->getSidTarget()));
				if (dcUser) { // User was found
					dcUser->send(adcParser->mCommand, true);
				}
				break;

			case HEADER_ECHO:
				// find target user
				dcUser = static_cast<DcUser *> (mDcServer->mDcUserList.getUserBaseByNick(adcParser->getSidTarget()));
				if (dcUser) { // User was found
					dcUser->send(adcParser->mCommand, true);
					dcConn->mDcUser->send(adcParser->mCommand, true);
				}
				break;

			case HEADER_FEATURE:
				// send to users
				mDcServer->mDcUserList.sendToFeature(adcParser->mCommand, 
					adcParser->getPositiveFeatures(), 
					adcParser->getNegativeFeatures(),
					true);
				break;

			default:
				break;
		}
	}

	LOG_CLASS(dcConn, LEVEL_TRACE, "[E]Stage " << adcParser->mType);
	return 0;
}



Conn * AdcProtocol::getConnForUdpData(Conn *, Parser *) {
	return NULL;
}



int AdcProtocol::onNewConn(Conn * conn) {

	DcConn * dcConn = static_cast<DcConn *> (conn);
	dcConn->mDcUser->setUid(string(genNewSid()));

	#ifndef WITHOUT_PLUGINS
		mDcServer->mCalls.mOnUserConnected.callAll(dcConn->mDcUser);
	#endif

	dcConn->setLoginTimeOut(mDcServer->mDcConfig.mTimeoutLogon, mDcServer->mTime); // Timeout for enter
	return 0;
}





int AdcProtocol::eventSup(AdcParser *, DcConn * dcConn) {

	if (dcConn->isState(STATE_NORMAL)) {

		// TODO save features
		return 0;

	} else if (!checkState(dcConn, "SUP", STATE_PROTOCOL)) {
		return -1;
	}

	// TODO check existing BASE feature!
	// "Failed to negotiate base protocol"

	dcConn->reserve(29); // 18 + 1 + 5 + 4 + 1
	dcConn->send(STR_LEN("ISUP ADBASE ADTIGR"), true, false);
	dcConn->send(STR_LEN("ISID "), false, false);
	dcConn->send(dcConn->mDcUser->getUid(), true, false);

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnSupports.callAll(dcConn->mDcUser)) {
			// Don't send first msg and hubinfo
			dcConn->flush();
			return -1;
		}
	#endif

	// Get First Message
	static string cache;
	bool flush = false;
	const string & buff = getFirstMsg(flush);
	if (flush) {
		cp1251ToUtf8(buff, cache, escaper);
	}

	// TODO optimization of transformation
	string hubName, topic;
	cp1251ToUtf8(mDcServer->mDcConfig.mHubName, hubName, escaper);
	cp1251ToUtf8(mDcServer->mDcConfig.mTopic, topic, escaper);

	// Send Hub Info
	static const string hubInf(STR_LEN("IINF CT32 VE" INTERNALVERSION " NI"));
	dcConn->reserve(9 + hubInf.size() + hubName.size() + topic.size() + cache.size()); // hubInf.size() + hubName.size() + 3 + topic.size() + 1 + 4 + cache.size() + 1
	dcConn->send(hubInf, false, false);
	dcConn->send(hubName, false, false);
	dcConn->send(STR_LEN(" DE"), false, false);
	dcConn->send(topic, true, false);

	// Send First Message
	dcConn->send(STR_LEN("IMSG "), false, false);
	dcConn->send(cache, true, true);


	dcConn->setState(STATE_PROTOCOL); // set protocol state
	return 0;
}



int AdcProtocol::eventSta(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventInf(AdcParser * adcParser, DcConn * dcConn) {

	if (!dcConn->isState(STATE_NORMAL)) {
		if (!checkState(dcConn, "INF", STATE_IDENTIFY)) {
			return -1;
		}
	}

	dcConn->mDcUser->setInfo(adcParser->mCommand);

	if (!dcConn->isState(STATE_NORMAL) && !verifyCid(dcConn->mDcUser)) {
		dcConn->closeNow(CLOSE_REASON_USER_INVALID); // TODO reason
		return -2;
	}

	if (dcConn->isState(STATE_NORMAL)) {
		// TODO: check CID and NI
		// Not allow change PID, CID and nick !!!
	}


	#ifndef WITHOUT_PLUGINS
		// TODO PD param in scripts!
		mDcServer->mCalls.mOnMyINFO.callAll(dcConn->mDcUser);
	#endif

	// TODO change to normal state
	if (dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		if (dcConn->mDcUser->isTrueBoolParam(USER_PARAM_CAN_HIDE)) {
			dcConn->send(dcConn->mDcUser->getInfo(), true); // Send to self only
		} else {
			// TODO sendMode

			// How send changed params?

			//mDcServer->mDcUserList.sendToAllAdc(dcConn->mDcUser->getInfo(), true, false);
			return 0; // Don't use getInfo in normal state!
		}
	} else { // !dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)

		dcConn->setState(STATE_IDENTIFY);

		// Validation (for request pass)
		#ifndef WITHOUT_PLUGINS
			if (mDcServer->mCalls.mOnValidateNick.callAll(dcConn->mDcUser)) {

				// TODO GPA random CID
				dcConn->send("IGPA QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ", 44, true, true); // We are sending the query for reception of the password
				return -4;
			}
		#endif

		dcConn->setState(STATE_VERIFY);
		dcConn->mSendNickList = true;
		mDcServer->beforeUserEnter(dcConn);
	}
	return 1;
}



int AdcProtocol::eventMsg(AdcParser * adcParser, DcConn * dcConn) {

	// TODO check SID

	if (!dcConn->mDcUser->isTrueBoolParam(USER_PARAM_IN_USER_LIST)) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		if (adcParser->mCommand.find(" PM") != adcParser->mCommand.npos) { // TODO refactoring
			if (mDcServer->mCalls.mOnTo.callAll(dcConn->mDcUser)) {
				return -2;
			}
		} else{
			if (mDcServer->mCalls.mOnChat.callAll(dcConn->mDcUser)) {
				return -2;
			}
		}
	#endif

	if (adcParser->getHeader() == HEADER_ECHO) {
		return 0;
	}

	// Sending message
	if (adcParser->getHeader() == HEADER_BROADCAST) {
		// TODO sendMode
		mDcServer->mChatList.sendToAllAdc(adcParser->mCommand, true, true);
		return 1;
	}
	return 0;
}



int AdcProtocol::eventSch(AdcParser *, DcConn * dcConn) {

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnSearch.callAll(dcConn->mDcUser)) {
			return -1;
		}
	#endif

	return 0;
}



int AdcProtocol::eventRes(AdcParser *, DcConn * dcConn) {

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnSR.callAll(dcConn->mDcUser)) {
			return -1;
		}
	#endif

	return 0;
}



int AdcProtocol::eventCtm(AdcParser *, DcConn * dcConn) {

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnConnectToMe.callAll(dcConn->mDcUser)) {
			return -1;
		}
	#endif

	return 0;
}



int AdcProtocol::eventRcm(AdcParser *, DcConn * dcConn) {

	#ifndef WITHOUT_PLUGINS
		if (mDcServer->mCalls.mOnRevConnectToMe.callAll(dcConn->mDcUser)) {
			return -1;
		}
	#endif

	return 0;
}



int AdcProtocol::eventGpa(AdcParser *, DcConn *) {

	#ifndef WITHOUT_PLUGINS
		// TODO call plugins for ADC event GPA
	#endif

	return 0;
}



int AdcProtocol::eventPas(AdcParser *, DcConn * dcConn) {

	if (!checkState(dcConn, "PAS", STATE_VERIFY)) {
		return -1;
	}

	#ifndef WITHOUT_PLUGINS
		mDcServer->mCalls.mOnMyPass.callAll(dcConn->mDcUser);
	#endif

	dcConn->setState(STATE_VERIFY);
	dcConn->mSendNickList = true;
	mDcServer->beforeUserEnter(dcConn);
	return 1;
}



int AdcProtocol::eventQui(AdcParser *, DcConn *) {

	#ifndef WITHOUT_PLUGINS
		// TODO call plugins for ADC event QUI
	#endif

	return 0;
}



int AdcProtocol::eventGet(AdcParser *, DcConn *) {

	#ifndef WITHOUT_PLUGINS
		// TODO call plugins for ADC event GET
	#endif

	return 0;
}



int AdcProtocol::eventGfi(AdcParser *, DcConn *) {

	#ifndef WITHOUT_PLUGINS
		// TODO call plugins for ADC event GFI
	#endif

	return 0;
}



int AdcProtocol::eventSnd(AdcParser *, DcConn *) {

	#ifndef WITHOUT_PLUGINS
		// TODO call plugins for ADC event SND
	#endif

	return 0;
}



int AdcProtocol::eventSid(AdcParser *, DcConn *) {

	#ifndef WITHOUT_PLUGINS
		// TODO call plugins for ADC event SID
	#endif

	return 0;
}



int AdcProtocol::eventCmd(AdcParser *, DcConn *) {

	#ifndef WITHOUT_PLUGINS
		// TODO call plugins for ADC event CMD
	#endif

	return 0;
}



int AdcProtocol::eventNat(AdcParser *, DcConn *) {

	#ifndef WITHOUT_PLUGINS
		// TODO call plugins for ADC event NAT
	#endif

	return 0;
}



int AdcProtocol::eventRnt(AdcParser *, DcConn *) {

	#ifndef WITHOUT_PLUGINS
		// TODO call plugins for ADC event RNT
	#endif

	return 0;
}



int AdcProtocol::eventPsr(AdcParser *, DcConn *) {

	#ifndef WITHOUT_PLUGINS
		// TODO call plugins for ADC event PSR
	#endif

	return 0;
}



int AdcProtocol::eventPub(AdcParser *, DcConn *) {

	#ifndef WITHOUT_PLUGINS
		// TODO call plugins for ADC event PUB
	#endif

	return 0;
}



int AdcProtocol::eventVoid(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventUnknown(AdcParser *, DcConn * dcConn) {

	#ifndef WITHOUT_PLUGINS
		if (!mDcServer->mCalls.mOnUnknown.callAll(dcConn->mDcUser)) {
			dcConn->closeNice(9000, CLOSE_REASON_CMD_UNKNOWN);
			return -1;
		}
	#endif

	return 0;
}



void AdcProtocol::infList(string & list, UserBase * userBase) {
	// INF ...\nINF ...\n
	if (!userBase->isHide()) {
		list.reserve(userBase->getInfo().size() + ADC_SEPARATOR_LEN);
		list.append(userBase->getInfo());
		list.append(STR_LEN(ADC_SEPARATOR));
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



const char * AdcProtocol::genNewSid() {
	const char * sid = NULL;
	UserBase * userBase = NULL;
	do {
		sid = getSid(++mSidNum);
		userBase = mDcServer->mDcUserList.getUserBaseByNick(sid);
	} while (userBase != NULL || strcmp(sid, "AAAA") == 0); // "AAAA" is a hub
	return sid;
}



// IMSG <msg>
void AdcProtocol::sendToChat(DcConn * dcConn, const string & data, bool flush /*= true*/) {
	string tmp;
	const string & out = toUtf8(data, tmp);
	dcConn->reserve(6 + out.size()); // 5 + out.size() + 1
	dcConn->send(STR_LEN("IMSG "), false, false);
	dcConn->send(out, true, flush);
}





// DMSG <to_sid> <msg>
void AdcProtocol::sendToChat(DcConn * dcConn, const string & data, const string & nick, bool flush /*= true*/) {
	// TODO: nick->uid
	const string & uid = nick;

	string tmp;
	const string & out = toUtf8(data, tmp);
	dcConn->reserve(11 + out.size()); // 5 + 4 + 1 + out.size() + 1
	dcConn->send(STR_LEN("DMSG "), false, false);
	dcConn->send(uid, false, false);
	dcConn->send(STR_LEN(" "), false, false);
	dcConn->send(out, true, flush);
}



// IMSG <msg>
void AdcProtocol::sendToChatAll(DcConn * dcConn, const string & data, bool flush /*= true*/) {
	sendToChat(dcConn, data, flush);
}



// BMSG <to_sid> <msg>
void AdcProtocol::sendToChatAll(DcConn * dcConn, const string & data, const string & nick, bool flush /*= true*/) {
	// TODO: nick->uid
	const string & uid = nick;

	string tmp;
	const string & out = toUtf8(data, tmp);
	dcConn->reserve(11 + out.size()); // 5 + 4 + 1 + out.size() + 1
	dcConn->send(STR_LEN("BMSG "), false, false);
	dcConn->send(uid, false, false);
	dcConn->send(STR_LEN(" "), false, false);
	dcConn->send(out, true, flush);
}



// EMSG <to_sid> <target_sid> <msg> PM<group_sid>
void AdcProtocol::sendToPm(DcConn * dcConn, const string & data, const string & nick, const string & from, bool flush /*= true*/) {
	// TODO: nick->uid
	const string & uid = nick;

	string tmp;
	const string & out = toUtf8(data, tmp);
	dcConn->reserve(23 + out.size()); // 5 + 4 + 1 + 4 + 1 + out.size() + 3 + 4 + 1
	dcConn->send(STR_LEN("EMSG "), false, false);
	dcConn->send(uid, false, false);
	dcConn->send(STR_LEN(" "), false, false);
	dcConn->send(dcConn->mDcUser->getUid(), false, false);
	dcConn->send(STR_LEN(" "), false, false);
	dcConn->send(out, false, false);
	dcConn->send(STR_LEN(" PM"), false, false);
	dcConn->send(from, true, flush);
}



void AdcProtocol::sendError(DcConn * dcConn, const string & errorText, int errorCode /*= 0*/) {
	string cmd(STR_LEN("ISTA ")), tmp;
	cmd.append(toString(SEVERITY_LEVEL_FATAL, tmp)); // Disconnect
	if (errorCode < 10) {
		cmd.append("0");
	}
	cmd.append(toString(errorCode, tmp)); // Error code
	cmd.append(STR_LEN(" "));

	const string & out = toUtf8(errorText, tmp);
	cmd.reserve(10 + out.size());
	cmd.append(out); // Error text
	dcConn->send(cmd, true);
}



void AdcProtocol::forceMove(DcConn * /*dcConn*/, const char * /*address*/, const char * /*reason*/ /*= NULL*/) {

	// TODO impl

}



/// Sending the user-list
int AdcProtocol::sendNickList(DcConn * dcConn) {
	if (mDcServer->mEnterList.find(dcConn->mDcUser->getNickHash())) {
		dcConn->mNickListInProgress = true;
	}
	dcConn->send(mDcServer->mDcUserList.getList(USER_LIST_ADC_INFO), true, true);
	return 0;
}



void AdcProtocol::onFlush(Conn * conn) {
	DcConn * dcConn = static_cast<DcConn *> (conn);
	if (dcConn->mNickListInProgress) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Enter after nicklist");
		dcConn->mNickListInProgress = false;
		mDcServer->doUserEnter(dcConn);
	}
}



int AdcProtocol::checkCommand(AdcParser * adcParser, DcConn * dcConn) {

	// TODO Checking length of command

	if (adcParser->mType == ADC_TYPE_INVALID) {
		LOG_CLASS(dcConn, LEVEL_DEBUG, "Wrong syntax cmd");
		sendError(dcConn, adcParser->getErrorText(), adcParser->getErrorCode());
		dcConn->closeNice(9000, CLOSE_REASON_CMD_SYNTAX);
		return -1;
	}

	// Checking null chars
	if (adcParser->mCommand.find('\0') != adcParser->mCommand.npos) {
		LOG_CLASS(dcConn, LEVEL_WARN, "Sending null chars, probably attempt an attack");
		dcConn->closeNow(CLOSE_REASON_CMD_NULL);
		return -2;
	}

	// Split
	adcParser->splitChunks();

	// TODO Check flood
	return 0;
}

bool AdcProtocol::verifyCid(DcUser * dcUser) {
	ParamBase * paramPd = dcUser->getParam("PD");
	if (paramPd == NULL) {
		return false;
	}

	ParamBase * paramId = dcUser->getParam("ID");
	if (paramId == NULL) {
		return false;
	}

	uint8_t pid[TigerHash::BYTES];
	Encoder::fromBase32(paramPd->getString().c_str(), pid, sizeof(pid));

	TigerHash th;
	th.update(static_cast<uint8_t *> (pid), sizeof(pid));

	string cid;
	Encoder::toBase32(th.finalize(), TigerHash::BYTES, cid);

	// Removing PID
	dcUser->removeParam("PD");

	return paramId->getString() == cid;
}



const string & AdcProtocol::toUtf8(const string & data, string & msg) {
	if (isUtf8(data.c_str(), data.size())) {
		msg.clear();
		msg.reserve(data.size());
		const char * ch = data.c_str();
		while (*ch) {
			escaper(*ch++, msg);
		}
		return data;
	}
	cp1251ToUtf8(data, msg, escaper);
	return msg;
}


} // namespace protocol

} // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
