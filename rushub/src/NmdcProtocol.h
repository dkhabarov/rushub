/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
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

#ifndef NMDC_PROTOCOL_H
#define NMDC_PROTOCOL_H

#include "Protocol.h"
#include "DcParser.h"

#include <string>
#include <cmath> // for ::std::floor

using namespace ::server;
using namespace ::std;

namespace dcserver {

class DcServer;
class DcConn;
class DcUser;
class UserList;

namespace protocol {


class NmdcProtocol : public Protocol {

public:

	NmdcProtocol();
	virtual ~NmdcProtocol();
	
	inline void setServer(DcServer * dcServer) {
		mDcServer = dcServer;
	}
	virtual int doCommand(Parser *, Conn *); /** Do command */
	
	virtual Conn * getConnForUdpData(Conn *, Parser *);

	/** Creating protocol parser */
	virtual Parser * createParser() {
		return new DcParser;
	}

	virtual string getSeparator();
	virtual unsigned long getMaxCommandLength();
	
	/** Removing protocol parser */
	virtual void deleteParser(Parser * parser) {
		if (parser != NULL) {
			delete parser;
		}
	}

	int onNewDcConn(DcConn *);

	static string & appendLock(string & str);
	static string & appendHello(string & str, const string & nick);
	static string & appendHubIsFull(string & str);
	static string & appendGetPass(string & str);
	static string & appendValidateDenide(string & str, const string & nick);
	static string & appendHubName(string & str, const string & hubName, const string & topic);
	static string & appendHubTopic(string & str, const string & hubTopic);
	static string & appendChat(string & str, const string & nick, const string & msg);
	static string & appendPm(string & str, const string & to, const string & from, const string & nick, const string & msg);
	static string & appendQuit(string & str, const string & nick);
	static string & appendOpList(string & str, const string & nick);
	static string & appendUserIp(string & str, const string & nick, const string & ip);
	static string & appendForceMove(string & str, const string & address);
	static void appendPmToAll(string & start, string & end, const string & from, const string & nick, const string & msg);

	void sendMode(DcConn *, const string & str, int mode, UserList &, bool useCache = false);
	int sendNickList(DcConn *); /** Sending user-list and op-list */
	static string getNormalShare(__int64); /** Get normal share size */

	void addToOps(DcUser *);
	void delFromOps(DcUser *);
	void addToIpList(DcUser *);
	void delFromIpList(DcUser *);
	void addToHide(DcUser *);
	void delFromHide(DcUser *);

protected:

	DcServer * mDcServer;

	typedef int (NmdcProtocol::*Event) (DcParser *, DcConn *);
	Event events[NMDC_TYPE_UNKNOWN + 1];


private:

	int eventSearch(DcParser *, DcConn *); /** Search request */
	int eventSr(DcParser *, DcConn *); /** Answer to search request */
	int eventMyInfo(DcParser *, DcConn *); /** MyINFO event */
	int eventSupports(DcParser *, DcConn *); /** Support of the additional expansions */
	int eventKey(DcParser *, DcConn *); /** Checking the key */
	int eventValidateNick(DcParser *, DcConn *); /** Checking and reg nick */
	int eventVersion(DcParser *, DcConn *); /** Checking a version */
	int eventGetNickList(DcParser *, DcConn *); /** Sending user-list */
	int eventChat(DcParser *, DcConn *); /** Chat message */
	int eventTo(DcParser *, DcConn *); /** Private message */
	int eventMyPass(DcParser *, DcConn *); /** Checking password */
	int eventConnectToMe(DcParser *, DcConn *); /** Active connection */
	int eventRevConnectToMe(DcParser *, DcConn *); /** Passive connection */
	int eventMultiConnectToMe(DcParser *, DcConn *); /** Multi connection (for linked hubs) */
	int eventKick(DcParser *, DcConn *); /** Kick */
	int eventOpForceMove(DcParser *, DcConn *); /** Force move */
	int eventGetInfo(DcParser *, DcConn *); /** Get user's MyINFO */
	int eventMcTo(DcParser *, DcConn *); /** Private message in chat */
	int eventUserIp(DcParser *, DcConn *); /** UserIP cmd */
	int eventPing(DcParser *, DcConn *); /** Ping cmd */
	int eventUnknown(DcParser *, DcConn *); /** Unknown cmd */
	int eventQuit(DcParser *, DcConn *); /** Quit cmd */

	int checkCommand(DcParser *, DcConn *);
	bool antiflood(DcConn *, unsigned int type);

	// Check validate nick (user)
	bool validateUser(DcConn *, const string & nick);
	bool checkNickLength(DcConn *, size_t len);
	bool badFlag(DcConn *, const char * cmd, unsigned int flag);

}; // NmdcProtocol

}; // namespace protocol

}; // namespace dcserver

#endif // NMDC_PROTOCOL_H

/**
 * $Id$
 * $HeadURL$
 */
