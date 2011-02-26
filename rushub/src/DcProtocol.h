/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

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

#ifndef DC_PROTOCOL_H
#define DC_PROTOCOL_H

#include "Protocol.h"
#include "DcParser.h"

#include <string>
#include <cmath> // for ::std::floor

using namespace ::server;
using namespace ::std;

namespace dcserver {

class DcServer;
class DcConn;
class UserList;

namespace protocol {

class DcProtocol : public Protocol {

public:
	DcProtocol();
	virtual ~DcProtocol(){};
	void SetServer(DcServer *);
	virtual int DoCmd(Parser *, Conn *); /** Do command */
	virtual Parser * CreateParser(){return new DcParser;} /** Creating protocol parser */
	virtual void DeleteParser(Parser *parser){if(parser != NULL) delete parser;} /** Removing protocol parser */

	static string & Append_DC_Lock(string &sStr);
	static string & Append_DC_Hello(string &sStr, const string &sNick);
	static string & Append_DC_HubIsFull(string &sStr);
	static string & Append_DC_GetPass(string &sStr);
	static string & Append_DC_ValidateDenide(string &sStr, const string &sNick);
	static string & Append_DC_HubName(string &sStr, const string &sHubName, const string &sTopic);
	static string & Append_DC_HubTopic(string &sStr, const string &sHubTopic);
	static string & Append_DC_Chat(string &sStr, const string &sNick, const string &sMsg);
	static string & Append_DC_PM(string &sStr, const string &sTo, const string &sFrom, const string &sNick, const string &sMsg);
	static string & Append_DC_Quit(string &sStr, const string &sNick);
	static string & Append_DC_OpList(string &sStr, const string &sNick);
	static string & Append_DC_UserIP(string &sStr, const string &sNick, const string &sIP);
	static string & Append_DC_ForceMove(string &sStr, const string &sAddress);
	static void Append_DC_PMToAll(string &sStart, string &sEnd, const string &sFrom, const string &sNick, const string &sMsg);

	void SendMode(DcConn *dcconn, const string & sStr, int iMode, UserList &, bool bUseCache = false);
	int SendNickList(DcConn *); /** Sending user-list and op-list */
	static string GetNormalShare(__int64); /** Get normal share size */

protected:
	DcServer * mDcServer;

	int DC_Search(DcParser *, DcConn *); /** Search request */
	int DC_SR(DcParser *, DcConn *); /** Answer to search request */
	int DC_MyINFO(DcParser *, DcConn *); /** MyINFO event */
	int DC_Supports(DcParser *, DcConn *); /** Support of the additional expansions */
	int DC_Key(DcParser *, DcConn *); /** Checking the key */
	int DC_ValidateNick(DcParser *, DcConn *); /** Checking and reg nick */
	int DC_Version(DcParser *, DcConn *); /** Checking a version */
	int DC_GetNickList(DcParser *, DcConn *); /** Sending user-list */
	int DC_Chat(DcParser *, DcConn *); /** Chat message */
	int DC_To(DcParser *, DcConn *); /** Private message */
	int DC_MyPass(DcParser *, DcConn *); /** Checking password */
	int DC_ConnectToMe(DcParser *, DcConn *); /** Active connection */
	int DC_RevConnectToMe(DcParser *, DcConn *); /** Passive connection */
	int DC_MultiConnectToMe(DcParser *, DcConn *); /** Multi connection (for linked hubs) */
	int DC_Kick(DcParser *, DcConn *); /** Kick */
	int DC_OpForceMove(DcParser *, DcConn *); /** Force move */
	int DC_GetINFO(DcParser *, DcConn *); /** Get user's MyINFO */
	int DC_MCTo(DcParser *, DcConn *); /** Private message in chat */
	int DC_Ping(DcParser *, DcConn *); /** Ping cmd */
	int DC_Unknown(DcParser *, DcConn *); /** Unknown cmd */
	int DC_Quit(DcParser *, DcConn *); /** Quit cmd */

}; // DcProtocol

}; // namespace protocol

}; // namespace dcserver

#endif // DC_PROTOCOL_H
