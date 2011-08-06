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
#include "DcConn.h" // for DcConn

namespace dcserver {

namespace protocol {


AdcProtocol::AdcProtocol() {
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



int AdcProtocol::onNewConn(Conn *) {
	return 0;
}





int AdcProtocol::eventSup(AdcParser *, DcConn * dcConn) {
	string s("ISUP ADBAS0 ADBASE ADTIGR\nISID AAAI");
	dcConn->send(s, true);
	return 0;
}



int AdcProtocol::eventSta(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventInf(AdcParser * adcParser, DcConn * dcConn) {
	dcConn->send(adcParser->mCommand, true);
	return 0;
}



int AdcProtocol::eventMsg(AdcParser * adcParser, DcConn * dcConn) {
	dcConn->send(adcParser->mCommand, true);
	return 0;
}



int AdcProtocol::eventSch(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventRes(AdcParser *, DcConn *) {
	return 0;
}



int AdcProtocol::eventCtm(AdcParser *, DcConn *) {
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


}; // namespace protocol

}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
