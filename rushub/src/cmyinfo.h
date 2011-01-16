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

#ifndef CMYINFO_H
#define CMYINFO_H

#include <string>

#include "cdctag.h"

using namespace std;
using namespace nDCServer::nProtoEnums;

#ifndef _WIN32
	#ifndef __int64
		#define __int64 long long
	#endif
#endif

namespace nDCServer {

class MyInfo {

public:

	/// DC Tag
	DcTag dcTag;

public:

	MyInfo();
	~MyInfo();
	MyInfo & operator = (const MyInfo &);

	const string & getMyInfo() const;
	void setMyInfo (const string & myInfo, cDCParserBase * parser, __int64 & totalHubShare);

	const string & getDescription() const;
	void setDescription (const string & description);

	const string & getEmail() const;
	void setEmail (const string & email);

	const string & getConnection() const;
	void setConnection (const string & connection);

	unsigned getMagicByte() const;
	void setMagicByte (unsigned magicByte);

	__int64 getShare() const;
	void setShare (__int64 share);

private:

	string myInfo;

	/// User's description
	string description;

	/// User's e-mail
	string email;

	/// User's connection
	string connection;

	/// User's magic byte
	unsigned magicByte;

	/// Share size
	__int64 share;

private:

	void construct();

}; // MyInfo

}; // nDCServer

#endif // CMYINFO_H
