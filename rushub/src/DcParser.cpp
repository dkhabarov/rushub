/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz

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

#include "DcParser.h"

namespace dcserver {

namespace protocol {

using namespace ::dcserver::protoenums;

/** �������� �������� ����� ������ */
ProtocolCommand aDC_Commands[] = {
	ProtocolCommand("$MultiSearch "),      // check: ip, delay
	ProtocolCommand("$MultiSearch Hub:"),  // check: nick, delay
	ProtocolCommand("$Search Hub:"),       // check: nick, delay //this must be first!! before the next one
	ProtocolCommand("$Search "),           // check: ip, delay
	ProtocolCommand("$SR "),               // check: nick
	ProtocolCommand("$MyINFO "),           // check: after_nick, nick, share_min_max
	ProtocolCommand("$Supports "),
	ProtocolCommand("$Key "),
	ProtocolCommand("$ValidateNick "),
	ProtocolCommand("$Version "),
	ProtocolCommand("$GetNickList"),
	ProtocolCommand("<"),                  // check: nick, delay, size, line_count
	ProtocolCommand("$To: "),              // check: nick, other_nick
	ProtocolCommand("$Quit "),             // no chech necessary
	ProtocolCommand("$MyPass "),
	ProtocolCommand("$ConnectToMe "),      // check: ip, nick
	ProtocolCommand("$RevConnectToMe "),   // check: nick, other_nick
	ProtocolCommand("$MultiConnectToMe "), // not implemented
	ProtocolCommand("$Kick "),             // check: op, nick, conn
	ProtocolCommand("$OpForceMove $Who:"), // check: op, nick
	ProtocolCommand("$GetINFO "),          // check: logged_in(FI), nick
	ProtocolCommand("$MCTo: "),            // check: nick, other_nick
	ProtocolCommand("|")                   // ping cmd (cap)
};


DcParser::DcParser() : Parser(10), DcParserBase(mCommand) { /** Max number of chunks - 10 */
	SetClassName("DcParser");
}

DcParser::~DcParser() {
}

/** Do parse for command and return type of this command */
int DcParser::Parse() {
	miLen = mCommand.size(); /** Set cmd len */
	if(miLen) for(int i = 0; i < NMDC_TYPE_UNKNOWN; ++i) {
		if(aDC_Commands[i].Check(mCommand)) { /** Check cmd from mCommand */
			miType = NmdcType(i); /** Set cmd type */
			miKWSize = aDC_Commands[i].mLength; /** Set length of key word for command */
			return miType;
		}
	}
	if(!miLen) miType = NMDC_TYPE_PING;
	else if(miType == NMDC_TYPE_UNPARSED) miType = NMDC_TYPE_UNKNOWN; /** Unknown cmd */
	return miType;
}

/** Get string address for the chunk of command */
string & DcParser::chunkString(unsigned int n) {
	if(!n) return mCommand; /** Empty line always full, and this pointer for empty line */
	if(n > mChunks.size()) /** This must not never happen, but if this happens, we are prepared */
		return mStrings[0];

	unsigned long flag = 1 << n;
	if(!(mStrMap & flag)) {
		mStrMap |= flag;
		try {
			tChunk &c = mChunks[n];
			if(c.first >= 0 && c.second >= 0 && (unsigned)c.first < mCommand.length() && (unsigned)c.second < mCommand.length()) 
				mStrings[n].assign(mCommand, c.first, c.second); /** Record n part in n element of the array of the lines */
			else if(ErrLog(1)) LogStream() << "Badly parsed message : " << mCommand << endl;
		} catch(...) {
			if(ErrLog(1)) LogStream() << "Ecxeption in chunk string" << endl;
		}
	}
	return mStrings[n];
}

bool DcParser::IsPassive(const string & sDesc) {
	if (!sDesc.size()) return false;
	size_t l = sDesc.size() - 1;
	if(sDesc[l] != '>') return false;
	size_t i = sDesc.find_last_of('<', l);
	if(i == sDesc.npos) return false;
	string sChunk;
	sChunk.assign(sDesc, int(i), int(l - i));
	if(
		(sChunk.find("M:P") == sDesc.npos) || 
		(sChunk.find("M:5") == sDesc.npos) || 
		(sChunk.find("M:S") == sDesc.npos)
	) return false;
	return true;
}

/** Split command to chunks */
bool DcParser::SplitChunks() {
	SetChunk(0, 0, mCommand.length()); /** Zero part - always whole command */
	switch(miType) {
		case NMDC_TYPE_MSEARCH:
		case NMDC_TYPE_MSEARCH_PAS:
		case NMDC_TYPE_SEARCH: /** $Search [[ip]:[port]] [[F?T?0?9]?[searchpattern]] */
			if(!SplitOnTwo(miKWSize, ' ', CHUNK_AS_ADDR, CHUNK_AS_QUERY)) mbError = 1;
			if(!SplitOnTwo(':', CHUNK_AS_ADDR, CHUNK_AS_IP, CHUNK_AS_PORT)) mbError = 1;
			if(!SplitOnTwo('?', CHUNK_AS_QUERY, CHUNK_AS_SEARCHLIMITS, CHUNK_AS_SEARCHPATTERN, 0)) mbError = 1;
			break;
		case NMDC_TYPE_SEARCH_PAS: /** $Search Hub:[nick] [[F?T?0?9]?[searchpattern]] */
			if(!SplitOnTwo(miKWSize, ' ', CHUNK_PS_NICK, CHUNK_PS_QUERY)) mbError =1;
			if(!SplitOnTwo('?', CHUNK_PS_QUERY, CHUNK_PS_SEARCHLIMITS, CHUNK_PS_SEARCHPATTERN, 0)) mbError = 1; /** Searching for on the right */
			break;
		case NMDC_TYPE_SR: /** Return search results
			$SR [result_nick] [file_path]^E[file_size] [free_slots]/[total_slots]^E[hub__name] ([hub_host][:[hub_port]])^E[searching_nick]
			$SR [result_nick] [file_path]^E[file_size] [free_slots]/[total_slots]^ETTH:[TTHROOT] ([hub_ip][:[hub_port]])^E[searching_nick]
			$SR [result_nick] [file_path]              [free_slots]/[total_slots]^ETTH:[TTHROOT] ([hub_ip][:[hub_port]])^E[searching_nick]
			1)  ----FROM-----|------------------------------------------------(PATH)------------------------------------------------------
			2)               |----PATH---|------------------------------------(SIZE)------------------------------------------------------
			3)                           |-----------------------------------(HUBINFO)----------------------------------|----TO-----------
			4)                           |----------------(SIZE)-----------------|---------------HUBINFO----------------|
			5)                           |----SIZE----|---------(SLOTS)----------|
			6)                                            |----SL_FR---/----SL_TO----|

			1)  ----FROM-----|-------------------------------------(PATH)-----------------------------------------------------------------
			2)               |------------------PATH-----------------------------|-----------------------------------(SIZE)---------------
			3)                                                                   |-------------------HUBINFO------------|----TO-----------

		//1)  ----FROM-----|-------------------------------------(PATH)-----------------------------------------------------------------
		//2)               |--------PATH------------|-----------------------(SLOTS)-----------------------------------------------------
		//3)                                        |-----------SLOTS----------|----------------------------------(HUBINFO)-------------
		//4)                                                                   |--------------HUBINFO-----------------|----TO-----------
		*/

			if(!SplitOnTwo(miKWSize, ' ', CHUNK_SR_FROM, CHUNK_SR_PATH)) mbError = 1;
			if(!SplitOnTwo(0x05, CHUNK_SR_PATH,    CHUNK_SR_PATH,    CHUNK_SR_SIZE)) mbError = 1;
			if(!SplitOnTwo(0x05, CHUNK_SR_SIZE,    CHUNK_SR_HUBINFO, CHUNK_SR_TO, false)) mbError = 1;
			if(SplitOnTwo(0x05,  CHUNK_SR_HUBINFO, CHUNK_SR_SIZE,    CHUNK_SR_HUBINFO)) {
				if(!SplitOnTwo(' ', CHUNK_SR_SIZE,  CHUNK_SR_SIZE,  CHUNK_SR_SLOTS)) mbError = 1;
				if(!SplitOnTwo('/', CHUNK_SR_SLOTS, CHUNK_SR_SL_FR, CHUNK_SR_SL_TO)) mbError = 1;
			} else SetChunk(CHUNK_SR_SIZE, 0, 0);
			break;
		case NMDC_TYPE_MYNIFO: /** $MyINFO $ALL [nick] [[desc]$ $[speed]$[email]$[share]$] */
			if(!SplitOnTwo(miKWSize, ' ', CHUNK_MI_DEST, CHUNK_MI_NICK)) mbError = 1;
			if(!SplitOnTwo(' ', CHUNK_MI_NICK, CHUNK_MI_NICK, CHUNK_MI_INFO)) mbError = 1;
			if(!SplitOnTwo("$ $", CHUNK_MI_INFO, CHUNK_MI_DESC, CHUNK_MI_SPEED)) mbError = 1;
			if(!SplitOnTwo('$', CHUNK_MI_SPEED, CHUNK_MI_SPEED, CHUNK_MI_MAIL)) mbError = 1;
			if(!SplitOnTwo('$', CHUNK_MI_MAIL, CHUNK_MI_MAIL, CHUNK_MI_SIZE)) mbError = 1;
			if(!ChunkRedRight(CHUNK_MI_SIZE, 1)) mbError = 1; /** Removing the last char $ */
			break;
		case NMDC_TYPE_SUPPORTS: break; /** This command has a different number parameters */
		/** Commands with one parameter */
		case NMDC_TYPE_KEY: /** $Key [key] */
		case NMDC_TYPE_VALIDATENICK: /** $ValidateNick [nick] */
		case NMDC_TYPE_VERSION: /** $Version [1,0091] */
		case NMDC_TYPE_QUIT: /** $Quit [nick] */
		case NMDC_TYPE_MYPASS: /** $MyPass [pass] */
		case NMDC_TYPE_KICK: /** $Kick [nick] */
			/* can be an empty line? */
			if(miLen == miKWSize) mbError = 1;
			else SetChunk(CHUNK_1_PARAM, miKWSize, miLen - miKWSize);
			break;
		case NMDC_TYPE_CHAT: /** <[nick]> [msg] */
			if(!SplitOnTwo(miKWSize, "> ", CHUNK_CH_NICK, CHUNK_CH_MSG)) mbError = 1;
			break;
		case NMDC_TYPE_TO: /** $To: [remote_nick] From: [nick] $<[[nick]> [msg]] */
			if(!SplitOnTwo(miKWSize," From: ", CHUNK_PM_TO, CHUNK_PM_FROM)) mbError = 1;
			if(!SplitOnTwo(" $<", CHUNK_PM_FROM, CHUNK_PM_FROM, CHUNK_PM_CHMSG)) mbError = 1;
			if(!SplitOnTwo("> ", CHUNK_PM_CHMSG, CHUNK_PM_NICK, CHUNK_PM_MSG)) mbError = 1;
			break;
		case NMDC_TYPE_CONNECTTOME: /** $ConnectToMe [nick] [[ip]:[port]] */
			if(!SplitOnTwo(miKWSize, ' ', CHUNK_CM_NICK, CHUNK_CM_ACTIVE)) mbError = 1;
			if(!SplitOnTwo(':', CHUNK_CM_ACTIVE, CHUNK_CM_IP, CHUNK_CM_PORT)) mbError = 1;
			break;
		case NMDC_TYPE_RCONNECTTOME: /** $RevConnectToMe [nick] [remote_nick] */
			if(!SplitOnTwo(miKWSize, ' ', CHUNK_RC_NICK, CHUNK_RC_OTHER)) mbError = 1;
			break;
		case NMDC_TYPE_MCONNECTTOME:
			break;
		case NMDC_TYPE_OPFORCEMOVE: /** $OpForceMove $Who:[remote_nick]$Where:[address]$Msg:[reason] */
			if(!SplitOnTwo(miKWSize, "$Where:", CHUNK_FM_NICK, CHUNK_FM_DEST)) mbError = 1;
			if(!SplitOnTwo("$Msg:", CHUNK_FM_DEST, CHUNK_FM_DEST, CHUNK_FM_REASON)) mbError = 1;
			break;
		case NMDC_TYPE_GETINFO: /** $GetINFO [remote_nick] [nick] */
			if(!SplitOnTwo(miKWSize, ' ', CHUNK_GI_OTHER, CHUNK_GI_NICK)) mbError = 1;
			break;
		case NMDC_TYPE_MCTO: /** $MCTo: [remote_nick] $[nick] [msg] */
			if(!SplitOnTwo(miKWSize," $", CHUNK_MC_TO, CHUNK_MC_FROM)) mbError = 1;
			if(!SplitOnTwo(' ', CHUNK_MC_FROM, CHUNK_MC_FROM, CHUNK_MC_MSG)) mbError = 1;
			break;
		default:
			break;
	}
	return mbError;
}

}; // namespace protocol

}; // namespace dcserver