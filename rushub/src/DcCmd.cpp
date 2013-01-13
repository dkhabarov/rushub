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


#include "DcCmd.h"

namespace dcserver {

namespace protocol {


DcCmd::DcCmd(int protocolType /*= DC_PROTOCOL_TYPE_ALL*/) :
	mProtocolType(protocolType)
{
	ASSERT(protocolType >= -1 && protocolType < DC_PROTOCOL_TYPE_SIZE);
}



DcCmd::~DcCmd() {
}



void DcCmd::parse(const string & cmd) {
	bool typeAll = false;
	switch (mProtocolType) {
		case DC_PROTOCOL_TYPE_ALL:
			typeAll = true;
		case DC_PROTOCOL_TYPE_ADC:
			// TODO: parse
			mChunk1[DC_PROTOCOL_TYPE_ADC] = cmd;
			if (!typeAll) {
				break;
			}
		case DC_PROTOCOL_TYPE_NMDC:
			// TODO: parse
			mChunk1[DC_PROTOCOL_TYPE_NMDC] = cmd;
			if (!typeAll) {
				break;
			}
	}
}



/// If nick is empty, then build simple char msg (without nick), else with this nick
void DcCmd::buildChat(const string & data, const string & nick, bool toAll) {
	bool typeAll = false;
	switch (mProtocolType) {
		case DC_PROTOCOL_TYPE_ALL:
			typeAll = true;
		case DC_PROTOCOL_TYPE_ADC:
			if (nick.empty()) {
				if (toAll) {
					AdcProtocol::appendChatAll(mChunk1[DC_PROTOCOL_TYPE_ADC], data);
				} else {
					AdcProtocol::appendChat(mChunk1[DC_PROTOCOL_TYPE_ADC], data);
				}
			} else {
				if (toAll) {
					AdcProtocol::appendChatAll(mChunk1[DC_PROTOCOL_TYPE_ADC], data, nick);
				} else {
					AdcProtocol::appendChat(mChunk1[DC_PROTOCOL_TYPE_ADC], data, nick);
				}
			}
			if (!typeAll) {
				break;
			}
		case DC_PROTOCOL_TYPE_NMDC:
			if (nick.empty()) {
				NmdcProtocol::appendChat(mChunk1[DC_PROTOCOL_TYPE_NMDC], data);
			} else {
				NmdcProtocol::appendChat(mChunk1[DC_PROTOCOL_TYPE_NMDC], data, nick);
			}
			if (!typeAll) {
				break;
			}
	}
}



void DcCmd::buildPm(const string & data, const string & nick, const string & from) {
	bool typeAll = false;
	switch (mProtocolType) {
		case DC_PROTOCOL_TYPE_ALL:
			typeAll = true;
		case DC_PROTOCOL_TYPE_ADC:
			AdcProtocol::appendPm(mChunk1[DC_PROTOCOL_TYPE_ADC], mChunk2[DC_PROTOCOL_TYPE_ADC], data, nick, from);
			if (!typeAll) {
				break;
			}
		case DC_PROTOCOL_TYPE_NMDC:
			NmdcProtocol::appendPm(mChunk1[DC_PROTOCOL_TYPE_NMDC], mChunk2[DC_PROTOCOL_TYPE_NMDC], data, nick, from);
			if (!typeAll) {
				break;
			}
	}
}



const string & DcCmd::getChunk1(int protocolType) const {
	ASSERT(protocolType >= 0 && protocolType < DC_PROTOCOL_TYPE_SIZE);
	return mChunk1[protocolType];
}



const string & DcCmd::getChunk2(int protocolType) const {
	ASSERT(protocolType >= 0 && protocolType < DC_PROTOCOL_TYPE_SIZE);
	return mChunk2[protocolType];
}


} // namespace protocol

} // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
