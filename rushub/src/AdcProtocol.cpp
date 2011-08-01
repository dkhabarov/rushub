/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
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
 
#include "AdcProtocol.h"

namespace dcserver {

namespace protocol {


AdcProtocol::AdcProtocol() {
}



AdcProtocol::~AdcProtocol() {
}



const char * AdcProtocol::getSeparator() {
	return ADC_SEPARATOR;
}



size_t AdcProtocol::getSeparatorLen() {
	return ADC_SEPARATOR_LEN;
}



unsigned long AdcProtocol::getMaxCommandLength() {
	return 0;
}



int AdcProtocol::doCommand(Parser *, Conn *) {
	return 0;
}



Conn * AdcProtocol::getConnForUdpData(Conn *, Parser *) {
	return NULL;
}



int AdcProtocol::onNewConn(Conn *) {
	return 0;
}


}; // namespace protocol

}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
