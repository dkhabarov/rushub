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

#ifndef ADC_PROTOCOL_H
#define ADC_PROTOCOL_H

#include "DcProtocol.h"
#include "AdcParser.h"

#include <string>

using namespace ::server;
using namespace ::std;

namespace dcserver {

class DcConn;
class DcServer;
class UserBase;

namespace protocol {


/// ADC protocol
class AdcProtocol : public DcProtocol {

public:

	AdcProtocol();
	virtual ~AdcProtocol();
	
	virtual const char * getSeparator() const;
	virtual size_t getSeparatorLen() const;
	virtual unsigned long getMaxCommandLength() const;

	virtual Parser * createParser();
	virtual void deleteParser(Parser *);

	virtual int doCommand(Parser *, Conn *);
	virtual Conn * getConnForUdpData(Conn *, Parser *);

	virtual int onNewConn(Conn *);
	virtual void onFlush(Conn *);

	void setServer(DcServer * dcServer) {
		mDcServer = dcServer;
	}

	static void infList(string & list, UserBase *);
	static const char * getSid(unsigned int num);

	virtual string & appendChat(string & str, const string & uid, const string & msg);
	virtual string & appendPm(string & str, const string & to, const string & from, const string & uid, const string & msg);

	virtual void forceMove(DcConn *, const char * address, const char * reason = NULL);
	virtual int sendNickList(DcConn *);

protected:

	DcServer * mDcServer;

	typedef int (AdcProtocol::*Event) (AdcParser *, DcConn *);
	Event events[ADC_TYPE_INVALID + 1];

private:

	unsigned int mSidNum;

private:

	int eventSup(AdcParser *, DcConn *); ///< SUP
	int eventSta(AdcParser *, DcConn *); ///< STA
	int eventInf(AdcParser *, DcConn *); ///< INF
	int eventMsg(AdcParser *, DcConn *); ///< MSG
	int eventSch(AdcParser *, DcConn *); ///< SCH
	int eventRes(AdcParser *, DcConn *); ///< RES
	int eventCtm(AdcParser *, DcConn *); ///< CTM
	int eventRcm(AdcParser *, DcConn *); ///< RCM
	int eventGpa(AdcParser *, DcConn *); ///< GPA
	int eventPas(AdcParser *, DcConn *); ///< PAS
	int eventQui(AdcParser *, DcConn *); ///< QUI
	int eventGet(AdcParser *, DcConn *); ///< GET
	int eventGfi(AdcParser *, DcConn *); ///< GFI
	int eventSnd(AdcParser *, DcConn *); ///< SND
	int eventSid(AdcParser *, DcConn *); ///< SID
	int eventCmd(AdcParser *, DcConn *); ///< CMD
	int eventNat(AdcParser *, DcConn *); ///< NAT
	int eventRnt(AdcParser *, DcConn *); ///< RNT
	int eventPsr(AdcParser *, DcConn *); ///< PSR
	int eventPub(AdcParser *, DcConn *); ///< PUB
	int eventVoid(AdcParser *, DcConn *); ///< Void cmd (ping)
	int eventUnknown(AdcParser *, DcConn *); ///< Unknown cmd

	int checkCommand(AdcParser *, DcConn *);

}; // AdcProtocol

}; // namespace protocol

}; // namespace dcserver

#endif // ADC_PROTOCOL_H

/**
 * $Id$
 * $HeadURL$
 */
