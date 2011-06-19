/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
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

#include "DcParser.h"

namespace dcserver {

namespace protocol {

using namespace ::dcserver::protoenums;


/** Protocol command */
class ProtocolCommand {

public:

	/** Key-word of cmd */
	string mKey;

	/** Cmd len */
	size_t mLength;

public:

	ProtocolCommand() {
	}

	ProtocolCommand(const char * key) : mKey(key) {
		mLength = mKey.length();
	}

	virtual ~ProtocolCommand() {
	}

	/** Checking that string contains command */
	bool check(const string & str) {
		return 0 == str.compare(0, mLength, mKey);
	}

}; // ProtocolCommand



/** Main NMDC commands keywords */
ProtocolCommand aDC_Commands[] = {
	ProtocolCommand("$MultiSearch "),      // check: ip, delay
	ProtocolCommand("$MultiSearch Hub:"),  // check: nick, delay
	ProtocolCommand("$Search Hub:"),       // check: nick, delay //this must be first!! before the next one
	ProtocolCommand("$Search "),           // check: ip, delay
	ProtocolCommand("$SR "),               // check: nick
	ProtocolCommand("$SR "),               // check: nick (UDP SR)
	ProtocolCommand("$MyINFO "),           // check: after_nick, nick, share_min_max
	ProtocolCommand("$Supports "),
	ProtocolCommand("$Key "),
	ProtocolCommand("$ValidateNick "),
	ProtocolCommand("$Version "),
	ProtocolCommand("$GetNickList"),
	ProtocolCommand("<"),                  // check: nick, delay, size, line_count
	ProtocolCommand("$To: "),              // check: nick, other_nick
	ProtocolCommand("$Quit "),             // no check necessary
	ProtocolCommand("$MyPass "),
	ProtocolCommand("$ConnectToMe "),      // check: ip, nick
	ProtocolCommand("$RevConnectToMe "),   // check: nick, other_nick
	ProtocolCommand("$MultiConnectToMe "), // not implemented
	ProtocolCommand("$Kick "),             // check: op, nick, conn
	ProtocolCommand("$OpForceMove $Who:"), // check: op, nick
	ProtocolCommand("$GetINFO "),          // check: logged_in(FI), nick
	ProtocolCommand("$MCTo: "),            // check: nick, other_nick
	ProtocolCommand("$UserIP "),
	ProtocolCommand("|")                   // ping cmd (cap)
};



DcParser::DcParser() :
	Parser(10), // Max number of chunks - 10
	mError(false),
	mKeyLength(0)
{ 
	SetClassName("DcParser");
}



DcParser::~DcParser() {
}



/** Do parse for command and return type of this command */
int DcParser::parse() {
	mLength = mCommand.size(); /** Set cmd len */
	if (mLength) {
		for (int i = 0; i < NMDC_TYPE_UNKNOWN; ++i) {
			if (aDC_Commands[i].check(mCommand)) { /** Check cmd from mCommand */
				mType = NmdcType(i); /** Set cmd type */
				mKeyLength = aDC_Commands[i].mLength; /** Set length of key word for command */
				return mType;
			}
		}
	}
	if (!mLength) {
		mType = NMDC_TYPE_PING;
	} else if(mType == NMDC_TYPE_UNPARSED) {
		mType = NMDC_TYPE_UNKNOWN; /** Unknown cmd */
	}
	return mType;
}



void DcParser::reInit() {
	Parser::reInit();

	mError = false;
	mKeyLength = 0;
}



/** Get string address for the chunk of command */
string & DcParser::chunkString(unsigned int n) {
	if (!n) {
		return mCommand; /** Empty line always full, and this pointer for empty line */
	}
	if (n > mChunks.size()) { /** This must not never happen, but if this happens, we are prepared */
		return mStrings[0];
	}

	unsigned long flag = 1 << n;
	if (!(mStrMap & flag)) {
		mStrMap |= flag;
		try {
			tChunk &c = mChunks[n];
			if (c.first < mCommand.length() && c.second < mCommand.length()) {
				mStrings[n].assign(mCommand, c.first, c.second); /** Record n part in n element of the array of the lines */
			} else if (ErrLog(1)) {
				LogStream() << "Badly parsed message : " << mCommand << endl;
			}
		} catch(...) {
			if (ErrLog(1)) {
				LogStream() << "Ecxeption in chunk string" << endl;
			}
		}
	}
	return mStrings[n];
}

bool DcParser::isPassive(const string & description) {
	if (!description.size()) {
		return false;
	}
	size_t l = description.size() - 1;
	if (description[l] != '>') {
		return false;
	}
	size_t i = description.find_last_of('<', l);
	if (i == description.npos) {
		return false;
	}
	if (
		(description.find("M:P", i, l - i) == description.npos) || 
		(description.find("M:5", i, l - i) == description.npos) || 
		(description.find("M:S", i, l - i) == description.npos)
	) {
		return false;
	}
	return true;
}

/** Split command to chunks */
bool DcParser::splitChunks() {

	if (mIsParsed) {
		return mError;
	}
	mIsParsed = true;

	setChunk(0, 0, mCommand.length()); /** Zero part - always whole command */

	switch (mType) {

		case NMDC_TYPE_MSEARCH :
			// Fallthrough

		case NMDC_TYPE_MSEARCH_PAS :
			// Fallthrough
			
		case NMDC_TYPE_SEARCH : /* * $Search [[ip]:[port]] [[F?T?0?9]?[searchpattern]] */
			if (!splitOnTwo(mKeyLength, ' ', CHUNK_AS_ADDR, CHUNK_AS_QUERY)) {
				mError = true;
			} else if (!splitOnTwo(':', CHUNK_AS_ADDR, CHUNK_AS_IP, CHUNK_AS_PORT)) {
				mError = true;
			} else if (!splitOnTwo('?', CHUNK_AS_QUERY, CHUNK_AS_SEARCHLIMITS, CHUNK_AS_SEARCHPATTERN, 0)) {
				mError = true;
			}
			break;

		case NMDC_TYPE_SEARCH_PAS : /* * $Search Hub:[nick] [[F?T?0?9]?[searchpattern]] */
			if (!splitOnTwo(mKeyLength, ' ', CHUNK_PS_NICK, CHUNK_PS_QUERY)) {
				mError = true;
			} else if (!splitOnTwo('?', CHUNK_PS_QUERY, CHUNK_PS_SEARCHLIMITS, CHUNK_PS_SEARCHPATTERN, 0)) {
				mError = true; /** Searching for on the right */
			}
			break;

		case NMDC_TYPE_SR : /** Return search results
			$SR [result_nick] [file_path]^E[file_size] [free_slots]/[total_slots]^E[hub__name] ([hub_host][:[hub_port]])^E[searching_nick]
			$SR [result_nick] [file_path]^E[file_size] [free_slots]/[total_slots]^ETTH:[TTHROOT] ([hub_ip][:[hub_port]])^E[searching_nick]
			$SR [result_nick] [file_path]              [free_slots]/[total_slots]^ETTH:[TTHROOT] ([hub_ip][:[hub_port]])^E[searching_nick]

			1)  |----FROM----|------------------------------------------------(PATH)-----------------------------------------------------|
			2)               |------------------------------------------------(PATH)------------------------------------|----TO----------|
			3)               |---------------------(PATH)------------------------|---------------HUBINFO----------------|
			4)               |--------(PATH)----------|----------SLOTS-----------|
			5)                                        |----SL_FR---/----SL_TO----|
			6)               |----PATH---|---[SIZE]---|
		*/

			if (!splitOnTwo(mKeyLength, ' ', CHUNK_SR_FROM, CHUNK_SR_PATH)) {
				mError = true;
			} else if (!splitOnTwo(0x05, CHUNK_SR_PATH,  CHUNK_SR_PATH,  CHUNK_SR_TO,      false)) {
				mError = true;
			} else if (!splitOnTwo(0x05, CHUNK_SR_PATH,  CHUNK_SR_PATH,  CHUNK_SR_HUBINFO, false)) {
				mError = true;
			} else if (!splitOnTwo(' ',  CHUNK_SR_PATH,  CHUNK_SR_PATH,  CHUNK_SR_SLOTS,   false)) {
				mError = true;
			} else if (!splitOnTwo('/',  CHUNK_SR_SLOTS, CHUNK_SR_SL_FR, CHUNK_SR_SL_TO,   false)) {
				mError = true;
			}
			splitOnTwo(0x05, CHUNK_SR_PATH, CHUNK_SR_PATH, CHUNK_SR_SIZE, false);
			break;

		case NMDC_TYPE_SR_UDP : /** Return search results
			$SR [result_nick] [file_path]^E[file_size] [free_slots]/[total_slots]^E[hub__name] ([hub_host][:[hub_port]])
			$SR [result_nick] [file_path]^E[file_size] [free_slots]/[total_slots]^ETTH:[TTHROOT] ([hub_ip][:[hub_port]])
			$SR [result_nick] [file_path]              [free_slots]/[total_slots]^ETTH:[TTHROOT] ([hub_ip][:[hub_port]])

			1)  |----FROM----|------------------------------------------------(PATH)------------------------------------|
			2)               |---------------------(PATH)------------------------|---------------HUBINFO----------------|
			3)               |--------(PATH)----------|----------SLOTS-----------|
			4)                                        |----SL_FR---/----SL_TO----|
			5)               |----PATH---|---[SIZE]---|
		*/

			if (!splitOnTwo(mKeyLength, ' ', CHUNK_SR_FROM, CHUNK_SR_PATH)) {
				mError = true;
			} else if (!splitOnTwo(0x05, CHUNK_SR_PATH,  CHUNK_SR_PATH,  CHUNK_SR_HUBINFO, false)) {
				mError = true;
			} else if (!splitOnTwo(' ',  CHUNK_SR_PATH,  CHUNK_SR_PATH,  CHUNK_SR_SLOTS,   false)) {
				mError = true;
			} else if (!splitOnTwo('/',  CHUNK_SR_SLOTS, CHUNK_SR_SL_FR, CHUNK_SR_SL_TO,   false)) {
				mError = true;
			}
			splitOnTwo(0x05, CHUNK_SR_PATH, CHUNK_SR_PATH, CHUNK_SR_SIZE, false);
			break;

		case NMDC_TYPE_MYNIFO : /* * $MyINFO $ALL [nick] [[desc]$ $[speed]$[email]$[share]$] */
			if (!splitOnTwo(mKeyLength, ' ', CHUNK_MI_DEST, CHUNK_MI_NICK)) {
				mError = true;
			} else if (!splitOnTwo(' ', CHUNK_MI_NICK, CHUNK_MI_NICK, CHUNK_MI_INFO)) {
				mError = true;
			} else if (!splitOnTwo("$ $", CHUNK_MI_INFO, CHUNK_MI_DESC, CHUNK_MI_SPEED)) {
				mError = true;
			} else if (!splitOnTwo('$', CHUNK_MI_SPEED, CHUNK_MI_SPEED, CHUNK_MI_MAIL)) {
				mError = true;
			} else if (!splitOnTwo('$', CHUNK_MI_MAIL, CHUNK_MI_MAIL, CHUNK_MI_SIZE)) {
				mError = true;
			} else if (!chunkRedRight(CHUNK_MI_SIZE, 1)) { /** Removing the last char $ */
				mError = true;
			}
			break;

		case NMDC_TYPE_SUPPORTS : /** This command has a different number parameters */
			break;

		/** Commands with one parameter */
		case NMDC_TYPE_KEY : /* $Key [key] */
			// Fallthrough

		case NMDC_TYPE_VALIDATENICK : /* $ValidateNick [nick] */
			// Fallthrough

		case NMDC_TYPE_VERSION : /* $Version [1,0091] */
			// Fallthrough

		case NMDC_TYPE_QUIT : /* $Quit [nick] */
			// Fallthrough

		case NMDC_TYPE_MYPASS : /* $MyPass [pass] */
			// Fallthrough

		case NMDC_TYPE_USERIP : /* $UserIP [param] */
			// Fallthrough

		case NMDC_TYPE_KICK : /* $Kick [nick] */
			/* can be an empty line? */
			if (mLength == mKeyLength) {
				mError = true;
			} else {
				setChunk(CHUNK_1_PARAM, mKeyLength, mLength - mKeyLength);
			}
			break;

		case NMDC_TYPE_CHAT : /* <[nick]> [msg] */
			if (!splitOnTwo(mKeyLength, "> ", CHUNK_CH_NICK, CHUNK_CH_MSG)) {
				mError = true;
			}
			break;

		case NMDC_TYPE_TO : /* $To: [remote_nick] From: [nick] $<[[nick]> [msg]] */
			if (!splitOnTwo(mKeyLength," From: ", CHUNK_PM_TO, CHUNK_PM_FROM)) {
				mError = true;
			} else if (!splitOnTwo(" $<", CHUNK_PM_FROM, CHUNK_PM_FROM, CHUNK_PM_CHMSG)) {
				mError = true;
			} else if (!splitOnTwo("> ", CHUNK_PM_CHMSG, CHUNK_PM_NICK, CHUNK_PM_MSG)) {
				mError = true;
			}
			break;

		case NMDC_TYPE_CONNECTTOME : /* $ConnectToMe [nick] [[ip]:[port]] */
			if (!splitOnTwo(mKeyLength, ' ', CHUNK_CM_NICK, CHUNK_CM_ACTIVE)) {
				mError = true;
			} else if(!splitOnTwo(':', CHUNK_CM_ACTIVE, CHUNK_CM_IP, CHUNK_CM_PORT)) {
				mError = true;
			}
			break;

		case NMDC_TYPE_RCONNECTTOME : /* $RevConnectToMe [nick] [remote_nick] */
			if (!splitOnTwo(mKeyLength, ' ', CHUNK_RC_NICK, CHUNK_RC_OTHER)) {
				mError = true;
			}
			break;

		case NMDC_TYPE_MCONNECTTOME :
			break;

		case NMDC_TYPE_OPFORCEMOVE : /* $OpForceMove $Who:[remote_nick]$Where:[address]$Msg:[reason] */
			if (!splitOnTwo(mKeyLength, "$Where:", CHUNK_FM_NICK, CHUNK_FM_DEST)) {
				mError = true;
			} else if (!splitOnTwo("$Msg:", CHUNK_FM_DEST, CHUNK_FM_DEST, CHUNK_FM_REASON)) {
				mError = true;
			}
			break;

		case NMDC_TYPE_GETINFO : /* $GetINFO [remote_nick] [nick] */
			if (!splitOnTwo(mKeyLength, ' ', CHUNK_GI_OTHER, CHUNK_GI_NICK)) {
				mError = true;
			}
			break;

		case NMDC_TYPE_MCTO : /* $MCTo: [remote_nick] $[nick] [msg] */
			if (!splitOnTwo(mKeyLength," $", CHUNK_MC_TO, CHUNK_MC_FROM)) {
				mError = true;
			} else if (!splitOnTwo(' ', CHUNK_MC_FROM, CHUNK_MC_FROM, CHUNK_MC_MSG)) {
				mError = true;
			}
			break;

		case NMDC_TYPE_UNKNOWN : /* Cmd without $ in begining position */
			if (mCommand.compare(0, 1, "$")) {
				mError = true;
			}
			break;

		default :
			break;

	}
	return mError;
}

int DcParser::checkCmd(DcParser & dcParser, const string & sData, DcUserBase * dcUserBase /*= NULL*/) {
	dcParser.reInit();
	dcParser.mCommand = sData;
	dcParser.parse();
	if (dcParser.splitChunks()) {
		return -1;
	}

	if (dcParser.mType == NMDC_TYPE_MYNIFO && (dcUserBase == NULL ||
			dcUserBase->getUid().empty() ||
			dcUserBase->getUid() != dcParser.chunkString(CHUNK_MI_NICK))
	) {
		return -2;
	}

	if (dcParser.mType > 0 && dcParser.mType < 3) {
		return 3;
	}
	return dcParser.mType;
}

}; // namespace protocol

}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
