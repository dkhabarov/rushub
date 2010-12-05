/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2009-2010 by Setuper
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

#ifndef CDCPROTOCOL_H
#define CDCPROTOCOL_H

#include "cprotocol.h"
#include "cdcparser.h"

#include <string>
#include <cmath> /** for ::std::floor */

using namespace nServer;
using namespace std;

namespace nDCServer {

class cDCServer;
class cDCConn;
class cUserBase;
class cUserList;

namespace nProtocol {

class cDCProtocol : public cProtocol {

public:
	cDCProtocol();
	virtual ~cDCProtocol(){};
	void SetServer(cDCServer *);
	virtual int DoCmd(cParser *, cConn *); /** Do command */
	virtual cParser * CreateParser(){return new cDCParser;} /** Creating protocol parser */
	virtual void DeleteParser(cParser *parser){if(parser != NULL) delete parser;} /** Removing protocol parser */

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
	static string & Append_DC_Kick(string &sStr, const string &sNick);
	static void Append_DC_PMToAll(string &sStart, string &sEnd, const string &sFrom, const string &sNick, const string &sMsg);

	void SendMode(cDCConn *dcconn, const string & sStr, int iMode, cUserList &, bool bUseCache = false);
	int SendNickList(cDCConn *); /** Sending user-list and op-list */
	static string GetNormalShare(__int64); /** Get normal share size */

protected:
	cDCServer * mDCServer;

	int DC_Search(cDCParser *, cDCConn *); /** Search request */
	int DC_SR(cDCParser *, cDCConn *); /** Answer to search request */
	int DC_MyINFO(cDCParser *, cDCConn *); /** MyINFO event */
	int DC_Supports(cDCParser *, cDCConn *); /** Support of the additional expansions */
	int DC_Key(cDCParser *, cDCConn *); /** Checking the key */
	int DC_ValidateNick(cDCParser *, cDCConn *); /** Checking and reg nick */
	int DC_Version(cDCParser *, cDCConn *); /** Checking a version */
	int DC_GetNickList(cDCParser *, cDCConn *); /** Sending user-list */
	int DC_Chat(cDCParser *, cDCConn *); /** Chat message */
	int DC_To(cDCParser *, cDCConn *); /** Private message */
	int DC_MyPass(cDCParser *, cDCConn *); /** Checking password */
	int DC_ConnectToMe(cDCParser *, cDCConn *); /** Active connection */
	int DC_RevConnectToMe(cDCParser *, cDCConn *); /** Passive connection */
	int DC_MultiConnectToMe(cDCParser *, cDCConn *); /** Multi connection (for linked hubs) */
	int DC_Kick(cDCParser *, cDCConn *); /** Kick */
	int DC_OpForceMove(cDCParser *, cDCConn *); /** Force move */
	int DC_GetINFO(cDCParser *, cDCConn *); /** Get user's MyINFO */
	int DC_MCTo(cDCParser *, cDCConn *); /** Private message in chat */
	int DC_Ping(cDCParser *, cDCConn *); /** Ping cmd */
	int DC_Unknown(cDCParser *, cDCConn *); /** Unknown cmd */
	int DC_Quit(cDCParser *, cDCConn *); /** Quit cmd */

}; // cDCProtocol

}; // nProtocol

}; // nDCServer

#endif // CDCPROTOCOL_H
