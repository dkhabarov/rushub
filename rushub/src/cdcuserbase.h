/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * Copyright (C) 2009-2010 by Setuper
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

#ifndef CDCUSERBASE_H
#define CDCUSERBASE_H

#include <string>
#ifndef _WIN32
	#define __int64 long long
#endif

using namespace std;

namespace nDCServer {

typedef enum { /** Params with null values flags */
	eMYINFO_TAG       = 1 << 0,  //< Tag
	eMYINFO_CLIENT    = 1 << 1,  //< Client name
	eMYINFO_VERSION   = 1 << 2,  //< Client version
	eMYINFO_MODE      = 1 << 3,  //< Mode
	eMYINFO_UNREG     = 1 << 4,  //< Usual hubs
	eMYINFO_REG       = 1 << 5,  //< Reg hubs
	eMYINFO_OP        = 1 << 6,  //< Op hubs
	eMYINFO_SLOT      = 1 << 7,  //< Slots
	eMYINFO_LIMIT     = 1 << 8,  //< Limit
	eMYINFO_OPEN      = 1 << 9, //< Open
	eMYINFO_BANDWIDTH = 1 << 10, //< Bandwidth
	eMYINFO_DOWNLOAD  = 1 << 11, //< Download
	eMYINFO_FRACTION  = 1 << 12, //< Fraction
} tMYINFONilType;

class cDCConnBase;

/** Base user class */
class cDCUserBase {

public:

	cDCConnBase * mDCConnBase;
	unsigned int mNil;

public:

	cDCUserBase(){}
	virtual ~cDCUserBase(){}

	virtual const string & GetNick() const = 0;
	virtual const string & GetMyINFO() const = 0;
	virtual bool IsInUserList() const = 0;
	virtual bool IsInOpList() const = 0;
	virtual bool IsInIpList() const = 0;
	virtual bool IsHide() const = 0;

	virtual bool SetMyINFO(const string &sMyINFO, const string & sNick) = 0;
	virtual void SetOpList(bool) = 0;
	virtual void SetIpList(bool) = 0;
	virtual void SetHide(bool) = 0;

	virtual const string & GetDesc() const = 0;
	virtual const string & GetEmail() const = 0;
	virtual const string & GetConnection() const = 0;
	virtual unsigned GetByte() const = 0;
	virtual __int64 GetShare() const = 0;

	virtual const string & GetTag() const = 0;
	virtual const string & GetClient() const = 0;
	virtual const string & GetVersion() const = 0;
	virtual unsigned GetUnRegHubs() const = 0;
	virtual unsigned GetRegHubs() const = 0;
	virtual unsigned GetOpHubs() const = 0;
	virtual unsigned GetSlots() const = 0;
	virtual unsigned GetLimit() const = 0;
	virtual unsigned GetOpen() const = 0;
	virtual unsigned GetBandwidth() const = 0;
	virtual unsigned GetDownload() const = 0;
	virtual const string & GetFraction() const = 0;
	virtual const string & GetMode() const = 0;

}; // cDCUserBase

}; // nDCServer

#endif // CDCUSERBASE_H
