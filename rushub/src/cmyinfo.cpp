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

#include "cmyinfo.h"

namespace nDCServer {



MyInfo::MyInfo() : magicByte(0), share(0) {
}

MyInfo::~MyInfo() {
}

MyInfo & MyInfo::operator = (const MyInfo &) {
	return *this;
}



const string & MyInfo::getMyInfo() const {
	return myInfo;
}

void MyInfo::setMyInfo (const string & myInfo, cDCParserBase * parser, __int64 & totalHubShare) {

	if (this->myInfo != myInfo) {
		this->myInfo = myInfo;

		totalHubShare -= share;
		share = StringToInt64(parser->ChunkString(eCH_MI_SIZE));
		totalHubShare += share;

		email = parser->ChunkString(eCH_MI_MAIL);
		connection = parser->ChunkString(eCH_MI_SPEED);

		int connSize = connection.size();
		if (connSize != 0) {
			magicByte = int(connection[--connSize]);
			connection.assign(connection, 0, connSize);
		}

		description = parser->ChunkString(eCH_MI_DESC);
		dcTag.parse(description);
	}
}



const string & MyInfo::getDescription() const {
	return description;
}

void MyInfo::setDescription (const string & description) {
	this->description = description;
	construct();
}



const string & MyInfo::getEmail() const {
	return email;
}

void MyInfo::setEmail (const string & email) {
	this->email = email;
	construct();
}



const string & MyInfo::getConnection() const {
	return connection;
}

void MyInfo::setConnection (const string & connection) {
	this->connection = connection;
	construct();
}



unsigned MyInfo::getMagicByte() const {
	return magicByte;
}

void MyInfo::setMagicByte (unsigned magicByte) {
	this->magicByte = magicByte;
	construct();
}



__int64 MyInfo::getShare() const {
	return share;
}

void MyInfo::setShare (__int64 share) {
	this->share = share;
	construct();
}



void MyInfo::construct() {

}


}; // nDCServer
