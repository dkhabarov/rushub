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

#ifndef DC_PROTOCOL_H
#define DC_PROTOCOL_H

#include "Protocol.h"

#include <string>

using namespace ::server;
using namespace ::std;

namespace dcserver {

class DcConn;
class DcUser;
class DcServer;

namespace protocol {


/// DC Protocol states
enum {

	STATE_PROTOCOL = 1 << 0, ///< Protocol state (NMDC $Key, ADC SUP)
	STATE_VALNICK  = 1 << 1, ///< Validation Nick (NMDC)
	STATE_VERSION  = 1 << 2, ///< $Version was received (NMDC)
	STATE_INFO     = 1 << 3, ///< $MyINFO string was received (NMDC)
	STATE_NICKLST  = 1 << 4, ///< $GetNickList (NMDC)
	STATE_IDENTIFY = STATE_VALNICK | STATE_VERSION | STATE_INFO | STATE_NICKLST, ///< Identify user state
	STATE_VERIFY   = 1 << 5, ///< Password was right or password was not need (NMDC $MyPass, ADC PAS)
	STATE_NORMAL   = STATE_PROTOCOL | STATE_IDENTIFY | STATE_VERIFY, ///< Normal state - action is valid
	STATE_DATA     = 1 << 6  ///< For binary transfers (ADC)

};


/// DC protocol
class DcProtocol : public Protocol {

public:

	DcProtocol();
	virtual ~DcProtocol();

	void setServer(DcServer * dcServer) {
		mDcServer = dcServer;
	}

	/// Chat Direct
	virtual void sendToChat(DcConn *, const string & data, bool flush = true) = 0;
	virtual void sendToChat(DcConn *, const string & data, const string & uid, bool flush = true) = 0;

	/// Chat Broadcast
	virtual void sendToChatAll(DcConn *, const string & data, bool flush = true) = 0;
	virtual void sendToChatAll(DcConn *, const string & data, const string & uid, bool flush = true) = 0;

	/// Private Message
	virtual void sendToPm(DcConn *, const string & data, const string & uid, const string & from, bool flush = true) = 0;

	/// Error Message
	virtual void sendError(DcConn *, const string & errorText, int errorCode = 0) = 0;

	virtual void forceMove(DcConn * dcConn, const char * address, const char * reason = NULL) = 0;
	virtual int sendNickList(DcConn *) = 0;

	static Parser * parse(int protocolType, const string & cmd);

	static bool parseInfo(int protocolType, DcUser *, const string & info);
	static bool formingInfo(int protocolType, DcUser *, string & info);

	virtual void addToOps(DcUser *);
	virtual void delFromOps(DcUser *);
	virtual void addToIpList(DcUser *);
	virtual void delFromIpList(DcUser *);
	virtual void addToHide(DcUser *);
	virtual void delFromHide(DcUser *);

	virtual bool removeFromDcUserList(DcUser *) = 0;

protected:

	DcServer * mDcServer;

protected:

	const string & getFirstMsg(bool & flush);
	bool checkState(DcConn *, const char * cmd, unsigned int state);

private:

	DcProtocol(const DcProtocol &);
	DcProtocol & operator = (const DcProtocol &);

}; // class DcProtocol

} // namespace protocol

} // namespace dcserver

#endif // DC_PROTOCOL_H

/**
 * $Id$
 * $HeadURL$
 */
