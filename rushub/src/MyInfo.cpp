/*
 * RusHub - hub server for Direct Connect peer to peer network.

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

#include "MyInfo.h"

namespace dcserver {



MyInfo::MyInfo() : magicByte(0), share(0) {
}

MyInfo::~MyInfo() {
}

MyInfo & MyInfo::operator = (const MyInfo &) {
	return *this;
}



void MyInfo::setMyInfo(const string & myInfo, DcParser * parser, __int64 & totalHubShare) {

	if (this->myInfo != myInfo) {
		this->myInfo = myInfo;

		totalHubShare -= share;
		share = StringToInt64(parser->chunkString(CHUNK_MI_SIZE));
		totalHubShare += share;

		email = parser->chunkString(CHUNK_MI_MAIL);
		connection = parser->chunkString(CHUNK_MI_SPEED);

		int connSize = connection.size();
		if (connSize != 0) {
			magicByte = int(connection[--connSize]);
			connection.assign(connection, 0, connSize);
		}

		description = parser->chunkString(CHUNK_MI_DESC);
		dcTag.parse(description);
	}
}



void MyInfo::setDescription(const string & description) {
	this->description = description;
	construct();
}


void MyInfo::setEmail(const string & email) {
	this->email = email;
	construct();
}


void MyInfo::setConnection(const string & connection) {
	this->connection = connection;
	construct();
}


void MyInfo::setMagicByte(unsigned magicByte) {
	this->magicByte = magicByte;
	construct();
}


void MyInfo::setShare(__int64 share) {
	this->share = share;
	construct();
}



void MyInfo::construct() {

}


}; // namespace dcserver
