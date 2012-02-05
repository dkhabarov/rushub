/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
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

#include "NmdcParser.h"
#include "DcUser.h"

#include <stdlib.h> // atoi unix

namespace dcserver {

namespace protocol {

using namespace ::dcserver::protoenums;


/// Protocol command
class ProtocolCommand {

public:

	/// Key-word of cmd
	string mKey;

	/// Cmd len
	size_t mLength;

public:

	ProtocolCommand() {
	}

	ProtocolCommand(const char * key, size_t len) : mKey(key, len) {
		mLength = len;
	}

	virtual ~ProtocolCommand() {
	}

	/// Checking that string contains command
	bool check(const string & str) {
		return 0 == str.compare(0, mLength, mKey);
	}

}; // ProtocolCommand



/// Main NMDC commands keywords
ProtocolCommand aDC_Commands[] = {
	ProtocolCommand("$MultiSearch ", 13),      // check: ip, delay
	ProtocolCommand("$MultiSearch Hub:", 17),  // check: nick, delay
	ProtocolCommand("$Search Hub:", 12),       // check: nick, delay //this must be first!! before the next one
	ProtocolCommand("$Search ", 8),            // check: ip, delay
	ProtocolCommand("$SR ", 4),                // check: nick
	ProtocolCommand("$SR ", 4),                // check: nick (UDP SR)
	ProtocolCommand("$MyINFO ", 8),            // check: after_nick, nick, share_min_max
	ProtocolCommand("$Supports ", 10),
	ProtocolCommand("$Key ", 5),
	ProtocolCommand("$ValidateNick ", 14),
	ProtocolCommand("$Version ", 9),
	ProtocolCommand("$GetNickList", 12),
	ProtocolCommand("<", 1),                   // check: nick, delay, size, line_count
	ProtocolCommand("$To: ", 5),               // check: nick, other_nick
	ProtocolCommand("$Quit ", 6),              // no check necessary
	ProtocolCommand("$MyPass ", 8),
	ProtocolCommand("$ConnectToMe ", 13),      // check: ip, nick
	ProtocolCommand("$RevConnectToMe ", 16),   // check: nick, other_nick
	ProtocolCommand("$MultiConnectToMe ", 18), // not implemented
	ProtocolCommand("$Kick ", 6),              // check: op, nick, conn
	ProtocolCommand("$OpForceMove $Who:", 18), // check: op, nick
	ProtocolCommand("$GetINFO ", 9),           // check: logged_in(FI), nick
	ProtocolCommand("$MCTo: ", 7),             // check: nick, other_nick
	ProtocolCommand("$UserIP ", 8),
	ProtocolCommand("|", 1)                    // ping cmd (cap)
};



NmdcParser::NmdcParser() :
	Parser(9), // Max number of chunks - 9 !!!
	mError(false),
	mKeyLength(0)
{ 
	setClassName("NmdcParser");
}



NmdcParser::~NmdcParser() {
}



/// Do parse for command and return type of this command
int NmdcParser::parse() {
	mLength = mCommand.size(); // Set cmd len
	if (mLength) {
		for (int i = 0; i < NMDC_TYPE_UNKNOWN; ++i) {
			if (aDC_Commands[i].check(mCommand)) { // Check cmd from mCommand
				mType = NmdcType(i); // Set cmd type
				mKeyLength = aDC_Commands[i].mLength; // Set length of key word for command
				return mType;
			}
		}
		mType = NMDC_TYPE_UNKNOWN; // Unknown cmd
	} else {
		mType = NMDC_TYPE_PING;
	}
	return mType;
}



void NmdcParser::reInit() {
	Parser::reInit();

	mError = false;
	mKeyLength = 0;
}



bool NmdcParser::isPassive(const string & description) {
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

/// Split command to chunks
bool NmdcParser::splitChunks() {

	if (mIsParsed) {
		return mError;
	}
	mIsParsed = true;

	setChunk(0, 0, mCommand.size()); // Zero part - always whole command

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
				mError = true; // Searching for on the right
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

		case NMDC_TYPE_SUPPORTS : // This command has a different number parameters
			break;

		// Commands with one parameter
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
			// Can be an empty line?
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

		case NMDC_TYPE_UNKNOWN : // Cmd without $ in begining position
			if (mCommand.compare(0, 1, "$")) {
				mError = true;
			}
			break;

		default :
			break;

	}
	return mError;
}



void NmdcParser::parseInfo(DcUser * dcUser, const string & info) {

	if (dcUser->getInfo() == info) {
		return;
	}

	NmdcParser nmdcParser;
	nmdcParser.mCommand = info;
	nmdcParser.parse();

	if (!nmdcParser.splitChunks() && nmdcParser.mType == NMDC_TYPE_MYNIFO) {

		// Share
		dcUser->getParamForce(USER_PARAM_SHARE)->setInt64(stringToInt64(nmdcParser.chunkString(CHUNK_MI_SIZE)));

		// Email
		dcUser->getParamForce(USER_PARAM_EMAIL)->setString(nmdcParser.chunkString(CHUNK_MI_MAIL));

		string speed = nmdcParser.chunkString(CHUNK_MI_SPEED);
		string magicByte;
		size_t size = speed.size();
		if (size != 0) {
			magicByte = speed[--size];
			speed.assign(speed, 0, size);
		}

		dcUser->getParamForce(USER_PARAM_BYTE)->setString(magicByte);
		dcUser->getParamForce(USER_PARAM_CONNECTION)->setString(speed);

		parseDesc(dcUser, nmdcParser.chunkString(CHUNK_MI_DESC));
	}
}



void NmdcParser::parseDesc(DcUser * dcUser, const string & description) {
	string tag, desc = description;
	size_t size = desc.size();
	if (size) {
		size_t i = desc.find_last_of('<');
		if (i != desc.npos && desc[--size] == '>') {
			++i;
			tag.assign(desc, i, size - i);
			desc.assign(desc, 0, --i);
		}
	}

	dcUser->getParamForce(USER_PARAM_DESC)->setString(desc);

	// TODO: optimization check old tag
	parseTag(dcUser, tag);
}



void NmdcParser::parseTag(DcUser * dcUser, const string & tag) {

	size_t tagSize = tag.size();

	string clientName, clientVersion;
	string unregHubs, regHubs, opHubs;

	if (tagSize) {
		// client name and version
		size_t clientPos = tag.find(',');
		if (clientPos == tag.npos) {
			clientPos = tagSize;
		}

		size_t v = tag.find("V:");
		if (v != tag.npos) {
			clientVersion.assign(tag, v + 2, clientPos - v - 2);
			clientName.assign(tag, 0, v);
		} else {
			size_t cn_e_pos = clientPos;
			size_t s = tag.find(' ');
			if (s != tag.npos && s < clientPos) {
				size_t b = s + 1;
				if (atof(tag.substr(b, clientPos - b).c_str())) {
					clientVersion.assign(tag, b, clientPos - b);
					cn_e_pos = s;
				}
			}
			clientName.assign(tag, 0, cn_e_pos);
		}

		// hubs
		size_t h = tag.find("H:");
		if (h != tag.npos) {
			h += 2;
			size_t unregPos = tag.find('/', h);
			if (unregPos == tag.npos) {
				unregPos = tag.find(',', h);
				if (unregPos == tag.npos) {
					unregPos = tagSize;
				}
			} else {
				size_t regPos = tag.find('/', ++unregPos);
				if (regPos == tag.npos) {
					regPos = tag.find(',', unregPos);
					if (regPos == tag.npos) {
						regPos = tagSize;
					}
				} else {
					size_t opPos = tag.find('/', ++regPos);
					if (opPos == tag.npos) {
						opPos = tag.find(',', regPos);
						if (opPos == tag.npos) {
							opPos = tagSize;
						}
					}
					opHubs.assign(tag, regPos, opPos - regPos);
				}
				regHubs.assign(tag, unregPos, regPos - unregPos - 1);
			}
			unregHubs.assign(tag, h, unregPos - h - 1);
		}
	}

	setParam(dcUser, USER_PARAM_CLIENT_NAME, clientName, !clientName.size());
	setParam(dcUser, USER_PARAM_CLIENT_VERSION, clientVersion, !clientVersion.size());
	setParam(dcUser, USER_PARAM_UNREG_HUBS, atoi(unregHubs.c_str()), !unregHubs.size());
	setParam(dcUser, USER_PARAM_REG_HUBS, atoi(regHubs.c_str()), !regHubs.size());
	setParam(dcUser, USER_PARAM_OP_HUBS, atoi(opHubs.c_str()), !opHubs.size());

	// slots and limits
	findParam(dcUser, tag, "M:", USER_PARAM_MODE, false);
	findParam(dcUser, tag, "S:", USER_PARAM_SLOTS);
	findParam(dcUser, tag, "L:", USER_PARAM_LIMIT);
	findParam(dcUser, tag, "O:", USER_PARAM_OPEN);
	findParam(dcUser, tag, "B:", USER_PARAM_BANDWIDTH);
	findParam(dcUser, tag, "D:", USER_PARAM_DOWNLOAD);
	findParam(dcUser, tag, "F:", USER_PARAM_FRACTION, false);
}



void NmdcParser::findParam(DcUser * dcUser, const string & tag, const char * find, const char * key, bool toInt /*= true*/) {
	string param;
	size_t pos = tag.find(find);
	if (pos != tag.npos) {
		pos += 2;
		size_t sepPos = tag.find(',', pos);
		if (sepPos == tag.npos) {
			sepPos = tag.size();
		}
		param.assign(tag, pos, sepPos - pos);
	}
	if (toInt) {
		setParam(dcUser, key, atoi(param.c_str()), !param.size());
	} else {
		setParam(dcUser, key, param, !param.size());
	}
}



void NmdcParser::setParam(DcUser * dcUser, const char * name, const string & value, bool remove /*= false*/) {
	if (remove) {
		dcUser->removeParam(name);
	} else {
		dcUser->getParamForce(name)->setString(value);
	}
}



void NmdcParser::setParam(DcUser * dcUser, const char * name, int value, bool remove /*= false*/) {
	if (remove) {
		dcUser->removeParam(name);
	} else {
		dcUser->getParamForce(name)->setInt(value);
	}
}



void NmdcParser::getTag(DcUser * dcUser, string & tag) {
	tag.reserve(0x2F); // usual length of tag
	tag.erase();
	ParamBase * param = dcUser->getParam(USER_PARAM_CLIENT_NAME);
	if (param) {
		tag.append(param->toString());
	}
	param = dcUser->getParam(USER_PARAM_CLIENT_VERSION);
	if (param) {
		if (tag.size()) {
			tag += ' ';
		}
		tag.append("V:", 2).append(param->toString());
	}
	param = dcUser->getParam(USER_PARAM_MODE);
	if (param) {
		if (tag.size()) {
			tag += ',';
		}
		tag.append("M:", 2).append(param->toString());
	}
	param = dcUser->getParam(USER_PARAM_UNREG_HUBS);
	if (param) {
		if (tag.size()) {
			tag += ',';
		}
		tag.append("H:", 2).append(param->toString());

		param = dcUser->getParam(USER_PARAM_REG_HUBS);
		if (param) {
			tag += '/';
			tag.append(param->toString());
		}

		param = dcUser->getParam(USER_PARAM_OP_HUBS);
		if (param) {
			tag += '/';
			tag.append(param->toString());
		}
	}
	param = dcUser->getParam(USER_PARAM_SLOTS);
	if (param) {
		if (tag.size()) {
			tag += ',';
		}
		tag.append("S:", 2).append(param->toString());
	}
	param = dcUser->getParam(USER_PARAM_OPEN);
	if (param) {
		if (tag.size()) {
			tag += ',';
		}
		tag.append("O:", 2).append(param->toString());
	}
	param = dcUser->getParam(USER_PARAM_LIMIT);
	if (param) {
		if (tag.size()) {
			tag += ',';
		}
		tag.append("L:", 2).append(param->toString());
	}
	param = dcUser->getParam(USER_PARAM_BANDWIDTH);
	if (param) {
		if (tag.size()) {
			tag += ',';
		}
		tag.append("B:", 2).append(param->toString());
	}
	param = dcUser->getParam(USER_PARAM_DOWNLOAD);
	if (param) {
		if (tag.size()) {
			tag += ',';
		}
		tag.append("D:", 2).append(param->toString());
	}
	param = dcUser->getParam(USER_PARAM_FRACTION);
	if (param) {
		if (tag.size()) {
			tag += ',';
		}
		tag.append("F:", 2).append(param->toString());
	}
	if (tag.size()) {
		tag = "<" + tag;
		tag += '>';
	}
}



void NmdcParser::formingInfo(DcUser * dcUser, string & info) {
	ParamBase * param = NULL;
	info.reserve(0x7F); // usual length of command
	info.assign("$MyINFO $ALL ", 13);
	info.append(dcUser->getUid());
	info += ' ';
	param = dcUser->getParam(USER_PARAM_DESC);
	if (param != NULL) {
		info.append(param->toString());
	}
	info.append(dcUser->getNmdcTag());
	info.append("$ $", 3);
	param = dcUser->getParam(USER_PARAM_CONNECTION);
	if (param != NULL) {
		info.append(param->toString());
	}
	param = dcUser->getParam(USER_PARAM_BYTE);
	if (param != NULL) {
		info.append(param->toString());
	}
	info += '$';
	param = dcUser->getParam(USER_PARAM_EMAIL);
	if (param != NULL) {
		info.append(param->toString());
	}
	info += '$';
	param = dcUser->getParam(USER_PARAM_SHARE);
	if (param != NULL) {
		info.append(param->toString());
	}
	info += '$';
}


}; // namespace protocol

}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
