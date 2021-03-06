/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
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

#ifndef NMDC_PROTOCOL_H
#define NMDC_PROTOCOL_H

#include "DcProtocol.h"
#include "NmdcParser.h"

#include <string>
#include <cmath> // for ::std::floor

using namespace ::server;
using namespace ::std;

namespace dcserver {

class DcServer;
class DcConn;
class DcUser;
class UserList;
class UserBase;


/// Main Protocol namespace
namespace protocol {


/// NMDC protocol
class NmdcProtocol : public DcProtocol {

public:

	NmdcProtocol();
	virtual ~NmdcProtocol();
	
	/// Protocol type
	virtual int getType() const;

	virtual const char * getSeparator() const;
	virtual size_t getSeparatorLen() const;
	virtual unsigned int getMaxCommandLength() const;

	/// Creating protocol parser
	virtual Parser * createParser() {
		return new NmdcParser;
	}
	
	/// Removing protocol parser
	virtual void deleteParser(Parser * parser) {
		if (parser != NULL) {
			delete parser;
		}
	}

	virtual int doCommand(Parser *, Conn *); ///< Do command
	virtual Conn * getConnForUdpData(Conn *, Parser *);

	virtual int onNewConn(Conn *);
	virtual void onFlush(Conn *);

	static void dcnEscape(const char * buf, size_t len, string & dest);
	static void dcnUnescape(const string & src, char * dest, size_t & len);
	static void lock2key(const string & lock, string & key);

	static string & appendLock(string & str);
	static string & appendHello(string & str, const string & nick);
	static string & appendHubIsFull(string & str);
	static string & appendGetPass(string & str);
	static string & appendValidateDenied(string & str, const string & nick);
	static string & appendHubName(string & str, const string & hubName, const string & topic);
	static string & appendHubTopic(string & str, const string & hubTopic);
	static string & appendQuit(string & str, const string & nick);
	static string & appendOpList(string & str, const string & nick);
	static string & appendUserIp(string & str, const string & nick, const string & ip);
	static string & appendForceMove(string & str, const string & address);

	static string & appendChat(string & str, const string & msg);
	static string & appendChat(string & str, const string & msg, const string & nick);
	static void appendPm(string & start, string & end, const string & msg, const string & nick, const string & from);

	/// Chat Direct
	virtual void sendToChat(DcConn *, const string & data, const string & nick, bool flush = true);

	/// Chat Broadcast
	virtual void sendToChatAll(DcConn *, const string & data, const string & nick, bool flush = true);

	/// Private Message
	virtual void sendToPm(DcConn *, const string & data, const string & nick, const string & from, bool flush = true);

	/// Error Message
	virtual void sendError(DcConn *, const string & errorText, int errorCode = 0);

	/// Action after add in user list
	virtual void onAddInUserList(DcUser *);

	virtual void forceMove(DcConn *, const char * address, const char * reason = NULL);
	virtual int sendNickList(DcConn *); ///< Sending user-list and op-list

	static void nickList(string & list, UserBase * userBase);
	static void myInfoList(string & list, UserBase * userBase);
	static void ipList(string & list, UserBase * userBase);

	virtual void addToOps(DcUser *);
	virtual void delFromOps(DcUser *);
	virtual void addToIpList(DcUser *);
	virtual void delFromIpList(DcUser *);
	virtual void addToHide(DcUser *);
	virtual void delFromHide(DcUser *);

protected:

	typedef int (NmdcProtocol::*Event) (NmdcParser *, DcConn *);
	Event events[NMDC_TYPE_UNKNOWN + 1];

private:

	int eventSearch(NmdcParser *, DcConn *); ///< Search request
	int eventSr(NmdcParser *, DcConn *); ///< Answer to search request
	int eventMyInfo(NmdcParser *, DcConn *); ///< MyINFO event
	int eventSupports(NmdcParser *, DcConn *); ///< Support of the additional expansions
	int eventKey(NmdcParser *, DcConn *); ///< Checking the key
	int eventValidateNick(NmdcParser *, DcConn *); ///< Checking and reg nick
	int eventVersion(NmdcParser *, DcConn *); ///< Checking a version
	int eventGetNickList(NmdcParser *, DcConn *); ///< Sending user-list
	int eventChat(NmdcParser *, DcConn *); ///< Chat message
	int eventTo(NmdcParser *, DcConn *); ///< Private message
	int eventMyPass(NmdcParser *, DcConn *); ///< Checking password
	int eventConnectToMe(NmdcParser *, DcConn *); ///< Active connection
	int eventRevConnectToMe(NmdcParser *, DcConn *); ///< Passive connection
	int eventMultiConnectToMe(NmdcParser *, DcConn *); ///< Multi connection (for linked hubs)
	int eventKick(NmdcParser *, DcConn *); ///< Kick
	int eventOpForceMove(NmdcParser *, DcConn *); ///< Force move
	int eventGetInfo(NmdcParser *, DcConn *); ///< Get user's MyINFO
	int eventMcTo(NmdcParser *, DcConn *); ///< Private message in chat
	int eventUserIp(NmdcParser *, DcConn *); ///< UserIP cmd
	int eventPing(NmdcParser *, DcConn *); ///< Ping cmd
	int eventUnknown(NmdcParser *, DcConn *); ///< Unknown cmd
	int eventQuit(NmdcParser *, DcConn *); ///< Quit cmd

	int checkCommand(NmdcParser *, DcConn *);
	bool antiflood(DcConn *, int type);
	void sendMode(DcConn *, const string & str, int mode, UserList &, bool flush = true);

	/// Check validate nick (user)
	bool validateUser(DcConn *, const string & nick);
	bool checkNickLength(DcConn *, size_t len);

}; // class NmdcProtocol

} // namespace protocol

} // namespace dcserver

#endif // NMDC_PROTOCOL_H

/**
 * $Id$
 * $HeadURL$
 */
