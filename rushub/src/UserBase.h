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

#ifndef USER_BASE_H
#define USER_BASE_H

#include <string>

using namespace ::std;

namespace dcserver {

/** Base user class for quick list */
class UserBase {

public:

	virtual const string & getUid() const = 0;
	virtual const string & getInfo() = 0;
	virtual const string & getIp() const = 0;
	virtual int getProfile() const = 0;
	virtual bool isHide() const = 0;
	virtual bool isCanSend() const = 0;
	virtual void send(const string & msg, bool sep = false, bool flush = true) = 0;
	virtual bool hasFeature(int feature) const = 0;

}; // UserBase

}; // namespace dcserver

#endif // USER_BASE_H

/**
 * $Id$
 * $HeadURL$
 */
