/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
 *
 * modified: 27 Aug 2009
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

#ifndef NMDC_PARSER_H
#define NMDC_PARSER_H

#include "Protocol.h"
#include "Plugin.h"

using namespace ::server;

namespace dcserver {


/// Protocol enums namespace
namespace protoenums {

typedef enum { /** Types of the commands */
	NMDC_TYPE_NO = -1,
	NMDC_TYPE_MSEARCH,        ///< 0  = $MultiSearch
	NMDC_TYPE_MSEARCH_PAS,    ///< 1  = $MultiSearch Hub:
	NMDC_TYPE_SEARCH_PAS,     ///< 2  = $Search Hub:
	NMDC_TYPE_SEARCH,         ///< 3  = $Search
	NMDC_TYPE_SR,             ///< 4  = $SR
	NMDC_TYPE_SR_UDP,         ///< 5  = $SR UDP
	NMDC_TYPE_MYNIFO,         ///< 6  = $MyNIFO
	NMDC_TYPE_SUPPORTS,       ///< 7  = $Support
	NMDC_TYPE_KEY,            ///< 8  = $Key
	NMDC_TYPE_VALIDATENICK,   ///< 9  = $ValidateNick
	NMDC_TYPE_VERSION,        ///< 10 = $Version
	NMDC_TYPE_GETNICKLIST,    ///< 11 = $GetNickList
	NMDC_TYPE_CHAT,           ///< 12 = Chat
	NMDC_TYPE_TO,             ///< 13 = $To
	NMDC_TYPE_QUIT,           ///< 14 = $Quit
	NMDC_TYPE_MYPASS,         ///< 15 = $MyPass
	NMDC_TYPE_CONNECTTOME,    ///< 16 = $ConnecToMe
	NMDC_TYPE_RCONNECTTOME,   ///< 17 = $RevConnectToMe
	NMDC_TYPE_MCONNECTTOME,   ///< 18 = $MultiConnectToMe
	NMDC_TYPE_KICK,           ///< 19 = $Kick
	NMDC_TYPE_OPFORCEMOVE,    ///< 20 = $OpForceMove
	NMDC_TYPE_GETINFO,        ///< 21 = $GetINFO
	NMDC_TYPE_MCTO,           ///< 22 = $MCTo
	NMDC_TYPE_USERIP,         ///< 23 = $UserIP
	NMDC_TYPE_PING,           ///< 24 = |
	NMDC_TYPE_UNKNOWN,        ///< 25 = $Unknown
} NmdcType;


/** The Following constants were developed for accomodation 
	corresponding to parameter for each enterring commands of 
	the protocol DC in variable mChunks, are used in function 
	DcServer::DC_* as well as in NmdcParser::splitChunks... 
	they must correspond
	Now max number of chunks = 9 !
*/

/** A number of the chunks for simple commands (without parameter) */
enum {
	CHUNK_0_ALL,
};

/** A number of the chunks for commands with one parameter.
		$Key [key],
		$ValidateNick [nick],
		$Version [1,0091],
		$Quit [nick],
		$MyPass [pass],
		$Kick [nick]
*/
enum {
	CHUNK_1_ALL,
	CHUNK_1_PARAM,
};

/** A number of the chunks for partition chat message
		<[nick]> [msg]
*/
enum {
	CHUNK_CH_ALL,
	CHUNK_CH_NICK,
	CHUNK_CH_MSG,
};

/** A number of the chunks for the $GetINFO command
		$GetINFO [remote_nick] [nick]
*/
enum {
	CHUNK_GI_ALL,
	CHUNK_GI_OTHER,
	CHUNK_GI_NICK,
};

/** A number of the chunks for the $RevConnectToMe command
		$RevConnectToMe [nick] [remote_nick]
*/
enum {
	CHUNK_RC_ALL,
	CHUNK_RC_NICK,
	CHUNK_RC_OTHER,
};

/** A number of the chunks for the private message
		$To: [remote_nick] From: [nick] $<[[nick]> [msg]]
*/
enum {
	CHUNK_PM_ALL,
	CHUNK_PM_TO,
	CHUNK_PM_FROM,
	CHUNK_PM_CHMSG,
	CHUNK_PM_NICK,
	CHUNK_PM_MSG,
};

/** A number of the chunks for the $MyINFO command
		$MyINFO $ALL [nick] [[desc]$ $[speed]$[email]$[share]$]
*/
enum {
	CHUNK_MI_ALL,
	CHUNK_MI_DEST,
	CHUNK_MI_NICK,
	CHUNK_MI_INFO,
	CHUNK_MI_DESC,
	CHUNK_MI_SPEED,
	CHUNK_MI_MAIL,
	CHUNK_MI_SIZE,
};

/** A number of the chunks for the $ConnectToMe command
		$ConnectToMe [remote_nick] [ip]:[port]
*/
enum {
	CHUNK_CM_ALL,
	CHUNK_CM_NICK,
	CHUNK_CM_ACTIVE,
	CHUNK_CM_IP,
	CHUNK_CM_PORT,
};

/** A number of the chunks for the $OpForceMove command
		$OpForceMove $Who:[remote_nick]$Where:[address]$Msg:[reason]
*/
enum {
	CHUNK_FM_ALL,
	CHUNK_FM_NICK,
	CHUNK_FM_DEST,
	CHUNK_FM_REASON,
};

/** A number of the chunks for the active search command
		$Search [[ip]:[port]] [[sizerestricted?isminimumsize?size?datatype]?[searchpattern]]
*/
enum {
	CHUNK_AS_ALL,
	CHUNK_AS_ADDR,
	CHUNK_AS_IP,
	CHUNK_AS_PORT,
	CHUNK_AS_QUERY,
	CHUNK_AS_SEARCHLIMITS,
	CHUNK_AS_SEARCHPATTERN,
};

/** A number of the chunks for the passive search command
		$Search Hub:[nick] [[sizerestricted?isminimumsize?size?datatype]?[searchpattern]]
*/
enum {
	CHUNK_PS_ALL,
	CHUNK_PS_NICK,
	CHUNK_PS_QUERY,
	CHUNK_PS_SEARCHLIMITS,
	CHUNK_PS_SEARCHPATTERN,
};

/** A number of the chunks for the search results command
		$SR [nick] [file/path][0x05][filesize] [freeslots]/[totalslots][0x05][hubname] ([hubhost][:[hubport]])[0x05][searching_nick]
*/
enum {
	CHUNK_SR_ALL,
	CHUNK_SR_FROM,
	CHUNK_SR_PATH,
	CHUNK_SR_SIZE,
	CHUNK_SR_SLOTS,
	CHUNK_SR_SL_FR,
	CHUNK_SR_SL_TO,
	CHUNK_SR_HUBINFO,
	CHUNK_SR_TO,
};

/** A number of the chunks for the private message
		$MCTo: [remote_nick] $[nick] [msg]
*/
enum {
	CHUNK_MC_ALL,
	CHUNK_MC_TO,
	CHUNK_MC_FROM,
	CHUNK_MC_MSG,
};

}; // namespace protoenums

class DcUser;

namespace protocol {

using namespace ::dcserver::protoenums;



/// NMDC protocol parser
class NmdcParser : public Parser {

public:

	NmdcParser();
	virtual ~NmdcParser();

	int getCommandType() const {
		return mType;
	}
	
	/** Do parse for command and return type of this command */
	virtual int parse();
	
	virtual void reInit();
	
	bool splitChunks();
	static bool isPassive(const string & description);
	static int checkCmd(NmdcParser & dcParser, const string & data, DcUser * dcUser = NULL);


	static void parseDesc(DcUser *, const string & desc);
	static void parseTag(DcUser *, const string & tag);
	static void findParam(DcUser *, const string & tag, const char * find, const char * key, bool toInt = true);
	static void setParam(DcUser *, const char * name, const string & value, bool remove = false);
	static void setParam(DcUser *, const char * name, int value, bool remove = false);
	static void getTag(DcUser *, string & tag);
	static void parseInfo(DcUser *, NmdcParser * parser);
	static void formingInfo(DcUser *, string & info);

private:

	bool mError;
	size_t mKeyLength; ///< Key-word len

}; // class NmdcParser

}; // namespace protocol

}; // namespace dcserver

#endif // NMDC_PARSER_H

/**
 * $Id$
 * $HeadURL$
 */
