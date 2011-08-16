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

#include "AdcParser.h"
#include "stringutils.h"

using namespace utils;

namespace dcserver {

namespace protocol {


/// ADC command
class AdcCommand {

public:

	AdcCommand() {
	}

	AdcCommand(const char * cmd) {
		mCmd = cmd;
	}

	virtual ~AdcCommand() {
	}

	bool check(const string & cmd) const {
		return 0 == cmd.compare(1, 3, mCmd);
	}

private:

	//hash = *reinterpret_cast<const unsigned int*>(cmd);
	const char * mCmd;

}; // AdcCommand



/// Main ADC commands
AdcCommand AdcCommands[] = {
	AdcCommand("SUP"), // SUP
	AdcCommand("STA"), // STA
	AdcCommand("INF"), // INF
	AdcCommand("MSG"), // MSG
	AdcCommand("SCH"), // SCH
	AdcCommand("RES"), // RES
	AdcCommand("CTM"), // CTM
	AdcCommand("RCM"), // RCM
	AdcCommand("GPA"), // GPA
	AdcCommand("PAS"), // PAS
	AdcCommand("QUI"), // QUI
	AdcCommand("GET"), // GET
	AdcCommand("GFI"), // GFI
	AdcCommand("SND"), // SND
	AdcCommand("SID"), // SID
	AdcCommand("CMD"), // CMD
	AdcCommand("NAT"), // NAT
	AdcCommand("RNT"), // RNT
	AdcCommand("PSR"), // PSR
	AdcCommand("PUB")  // PUB
};



AdcParser::AdcParser() :
	Parser(9), // Max number of chunks - 9 !!!
	mHeader(HEADER_UNKNOWN),
	mError(false)
{ 
	setClassName("AdcParser");
}



AdcParser::~AdcParser() {
}



int AdcParser::getCommandType() const {
	return mType;
}



int AdcParser::getHeader() const {
	return mHeader;
}



const string & AdcParser::getSidSource() const {
	return mSidSource;
}



const string & AdcParser::getSidTarget() const {
	return mSidTarget;
}



const string & AdcParser::getCidSource() const {
	return mCidSource;
}



const string & AdcParser::getErrorCode() const {
	return mErrorCode;
}



const string & AdcParser::getErrorText() const {
	return mErrorText;
}



/// Do parse for command and return type of this command
int AdcParser::parse() {

	mLength = mCommand.size(); // Set cmd len

	if (mLength >= 4) { // ADC cmd key must contain 4 symbols

		if (checkHeaderSyntax()) {

			for (unsigned int i = 0; i < ADC_TYPE_INVALID; ++i) {
				if (AdcCommands[i].check(mCommand)) { // Check cmd from mCommand
					return mType = AdcType(i); // Set cmd type
				}
			}
			return mType = ADC_TYPE_UNKNOWN; // Unknown cmd
		}

	} else if (mLength == 0) {
		return mType = ADC_TYPE_VOID; // Void cmd
	}

	return mType = ADC_TYPE_INVALID; // Invalid cmd
}



bool AdcParser::checkHeaderSyntax() {

	// Check cmd name ([A-Z] [A-Z0-9] [A-Z0-9])
	if (!isUpperAlpha(mCommand[1]) || !isUpperAlphaNum(mCommand[2]) || !isUpperAlphaNum(mCommand[3])) {
		return false;
	}

	unsigned int counter = 0;

	switch (mCommand[0]) {

		case 'B': // Broadcast
			// 'B' command_name ' ' base32_character{4}
			mHeader = HEADER_BROADCAST;
			if (mLength < 9 || mCommand[4] != ' ') {
				mErrorCode = "40"; // Protocol error
				mErrorText = "Protocol syntax error";
				return false;
			} else if (!isBase32(mCommand[5]) || !isBase32(mCommand[6]) || !isBase32(mCommand[7]) || !isBase32(mCommand[8])) {
				mErrorCode = "40"; // Protocol error
				mErrorText = "Invalid source SID";
				return false;
			}
			mSidSource.assign(mCommand, 5, 4);
			break;

		case 'H': // Hub message
			// 'H' command_name
			mHeader = HEADER_HUB;
			break;

		case 'I': // Info message
			// 'I' command_name
			mHeader = HEADER_INFO;
			break;

		case 'C': // Client message
			// 'C' command_name
			mHeader =  HEADER_CLIENT;
			break;

		case 'D': // Direct message
			// 'D' command_name ' ' base32_character{4} ' ' base32_character{4}
			mHeader = HEADER_DIRECT;
			// Fallthrough

		case 'E': // Echo message
			// 'E' command_name ' ' base32_character{4} ' ' base32_character{4}
			if (mHeader == HEADER_UNKNOWN) {
				mHeader = HEADER_ECHO;
			}
			if (mLength < 14 || mCommand[4] != ' ' || mCommand[9] != ' ') {
				mErrorCode = "40"; // Protocol error
				mErrorText = "Protocol syntax error";
				return false;
			} else if (!isBase32(mCommand[5]) || !isBase32(mCommand[6]) || !isBase32(mCommand[7]) || !isBase32(mCommand[8])) {
				mErrorCode = "40"; // Protocol error
				mErrorText = "Invalid source SID";
				return false;
			} else if (!isBase32(mCommand[10]) || !isBase32(mCommand[11]) || !isBase32(mCommand[12]) || !isBase32(mCommand[13])) {
				mErrorCode = "40"; // Protocol error
				mErrorText = "Invalid target SID";
				return false;
			}
			mSidSource.assign(mCommand, 5, 4);
			mSidTarget.assign(mCommand, 10, 4);
			break;

		case 'F': // Feature broadcast
			// 'F' command_name ' ' base32_character{4} ' ' (('+'|'-') [A-Z] [A-Z0-9]{3})+
			mHeader = HEADER_FEATURE;
			if (mLength < 15 || mCommand[4] != ' ' || mCommand[9] != ' ' || mCommand[10] != '+' && mCommand[10] != '-') {
				mErrorCode = "40"; // Protocol error
				mErrorText = "Protocol syntax error";
				return false;
			} else if (!isBase32(mCommand[5]) || !isBase32(mCommand[6]) || !isBase32(mCommand[7]) || !isBase32(mCommand[8])) {
				mErrorCode = "40"; // Protocol error
				mErrorText = "Invalid source SID";
				return false;
			} else if (!isUpperAlpha(mCommand[11]) || !isUpperAlphaNum(mCommand[12]) || !isUpperAlphaNum(mCommand[13]) || !isUpperAlphaNum(mCommand[14])) {
				mErrorCode = "40"; // Protocol error
				mErrorText = "Invalid feature";
				return false;
			}
			mSidSource.assign(mCommand, 5, 4);
			break;

		case 'U': // UDP message
			// 'U' command_name ' ' base32_character+
			mHeader = HEADER_UDP;
			if (mLength < 6 || mCommand[4] != ' ' || !isBase32(mCommand[5])) {
				mErrorCode = "40"; // Protocol error
				mErrorText = "Protocol syntax error";
				return false;
			}
			counter = 6;
			while (counter < mLength && isBase32(mCommand[counter++])) {
			}
			if (counter != mLength && mCommand[counter] != ' ') {
				mErrorCode = "40"; // Protocol error
				mErrorText = "Invalid source CID";
				return false;
			}
			mCidSource.assign(mCommand, 5, counter - 5);
			break;

		default: // Unknown
			return false;

	}

	return true;
}



void AdcParser::reInit() {
	Parser::reInit();

	mHeader = HEADER_UNKNOWN;
	mSidSource.clear();
	mSidTarget.clear();
	mError = false;
}



/// Split command to chunks
bool AdcParser::splitChunks() {

	if (mIsParsed) {
		return mError;
	}
	mIsParsed = true;

	setChunk(0, 0, mCommand.size()); // Zero part - always whole command

	switch (mType) {

		case ADC_TYPE_SUP : // This command has a different number parameters
			break;

// todo chech sid length!
		case ADC_TYPE_STA : // STA code [desc]
			if (mHeader == HEADER_ECHO || mHeader == HEADER_DIRECT) {
				if (!splitOnTwo(' ', CHUNK_ADC_STA_CMD, CHUNK_ADC_STA_CMD, CHUNK_ADC_STA_TO_SID)) {
					mError = true;
				}
				if (!splitOnTwo(' ', CHUNK_ADC_STA_TO_SID, CHUNK_ADC_STA_TO_SID, CHUNK_ADC_STA_CODE)) {
					mError = true;
				}
			} else {
				if (!splitOnTwo(0, ' ', CHUNK_ADC_STA_CMD, CHUNK_ADC_STA_CODE)) {
					mError = true;
				}
			}
			if (splitOnTwo(' ', CHUNK_ADC_STA_CODE, CHUNK_ADC_STA_CODE, CHUNK_ADC_STA_DESC)) {
				splitOnTwo(' ', CHUNK_ADC_STA_DESC, CHUNK_ADC_STA_DESC, CHUNK_ADC_STA_OTHER);
			}
			break;

		case ADC_TYPE_INF : // INF sid params
			if (!splitOnTwo(0, ' ', CHUNK_ADC_INF_CMD, CHUNK_ADC_INF_SID)) {
				mError = true;
			} else if (!splitOnTwo(' ', CHUNK_ADC_INF_SID, CHUNK_ADC_INF_SID, CHUNK_ADC_INF_OTHER)) {
				mError = true;
			}
			break;
			
		case ADC_TYPE_MSG : // MSG sid [to_sid] text [me] [pm]
			if (!splitOnTwo(0, ' ', CHUNK_ADC_MSG_CMD, CHUNK_ADC_MSG_SID)) {
				mError = true;
			} else if (mHeader == HEADER_ECHO || mHeader == HEADER_DIRECT) {
				if (!splitOnTwo(' ', CHUNK_ADC_MSG_SID, CHUNK_ADC_MSG_SID, CHUNK_ADC_MSG_TO_SID)) {
					mError = true;
				} else if (!splitOnTwo(' ', CHUNK_ADC_MSG_TO_SID, CHUNK_ADC_MSG_TO_SID, CHUNK_ADC_MSG_TEXT)) {
					mError = true;
				}
			} else if (!splitOnTwo(' ', CHUNK_ADC_MSG_SID, CHUNK_ADC_MSG_SID, CHUNK_ADC_MSG_TEXT)) {
				mError = true;
			}

			if (!splitOnTwo(" ME", CHUNK_ADC_MSG_TEXT, CHUNK_ADC_MSG_TEXT, CHUNK_ADC_MSG_ME)) {
				if (!splitOnTwo(" PM", CHUNK_ADC_MSG_TEXT, CHUNK_ADC_MSG_TEXT, CHUNK_ADC_MSG_PM)) {
					splitOnTwo(' ', CHUNK_ADC_MSG_TEXT, CHUNK_ADC_MSG_TEXT, CHUNK_ADC_MSG_OTHER);
				} else {
					splitOnTwo(' ', CHUNK_ADC_MSG_PM, CHUNK_ADC_MSG_PM, CHUNK_ADC_MSG_OTHER);
				}
			} else {
				if (!splitOnTwo(" PM", CHUNK_ADC_MSG_ME, CHUNK_ADC_MSG_ME, CHUNK_ADC_MSG_PM)) {
					splitOnTwo(' ', CHUNK_ADC_MSG_ME, CHUNK_ADC_MSG_ME, CHUNK_ADC_MSG_OTHER);
				} else {
					splitOnTwo(' ', CHUNK_ADC_MSG_PM, CHUNK_ADC_MSG_PM, CHUNK_ADC_MSG_OTHER);
				}
			}
			break;

		case ADC_TYPE_SCH : // SCH sid params
			if (!splitOnTwo(0, ' ', CHUNK_ADC_PARAMS_CMD, CHUNK_ADC_PARAMS_SID)) {
				mError = true;
			} else if (!splitOnTwo(' ', CHUNK_ADC_PARAMS_SID, CHUNK_ADC_PARAMS_SID, CHUNK_ADC_PARAMS_OTHER)) {
				mError = true;
			}
			break;

		case ADC_TYPE_RES : // RES sid params
			if (!splitOnTwo(0, ' ', CHUNK_ADC_PARAMS_CMD, CHUNK_ADC_PARAMS_SID)) {
				mError = true;
			} else if (!splitOnTwo(' ', CHUNK_ADC_PARAMS_SID, CHUNK_ADC_PARAMS_SID, CHUNK_ADC_PARAMS_OTHER)) {
				mError = true;
			}
			break;

		case ADC_TYPE_CTM : // CTM sid params
			if (!splitOnTwo(0, ' ', CHUNK_ADC_PARAMS_CMD, CHUNK_ADC_PARAMS_SID)) {
				mError = true;
			} else if (!splitOnTwo(' ', CHUNK_ADC_PARAMS_SID, CHUNK_ADC_PARAMS_SID, CHUNK_ADC_PARAMS_OTHER)) {
				mError = true;
			}
			break;

		case ADC_TYPE_RCM : // RCM sid params
			if (!splitOnTwo(0, ' ', CHUNK_ADC_PARAMS_CMD, CHUNK_ADC_PARAMS_SID)) {
				mError = true;
			} else if (!splitOnTwo(' ', CHUNK_ADC_PARAMS_SID, CHUNK_ADC_PARAMS_SID, CHUNK_ADC_PARAMS_OTHER)) {
				mError = true;
			}
			break;

		case ADC_TYPE_GPA : // GPA data
			if (!splitOnTwo(0, ' ', CHUNK_ADC_SINGLE_CMD, CHUNK_ADC_SINGLE_DATA)) {
				mError = true;
			}
			break;

		case ADC_TYPE_PAS : // PAS password
			if (!splitOnTwo(0, ' ', CHUNK_ADC_SINGLE_CMD, CHUNK_ADC_SINGLE_DATA)) {
				mError = true;
			}
			break;

		case ADC_TYPE_QUI : // QUI sid
			if (!splitOnTwo(0, ' ', CHUNK_ADC_QUI_CMD, CHUNK_ADC_QUI_SID)) {
				mError = true;
			}
			break;

		case ADC_TYPE_GET : //
			break;

		case ADC_TYPE_GFI : //
			break;

		case ADC_TYPE_SND : //
			break;

		case ADC_TYPE_SID : //
			break;

		case ADC_TYPE_CMD : //
			break;

		case ADC_TYPE_NAT : //
			break;

		case ADC_TYPE_RNT : //
			break;

		case ADC_TYPE_PSR : //
			break;

		case ADC_TYPE_PUB : //
			break;

		case ADC_TYPE_UNKNOWN : //
			break;

		default :
			break;

	}
	return mError;
}

}; // namespace protocol

}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
