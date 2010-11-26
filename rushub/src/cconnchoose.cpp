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

#include "cconnchoose.h"

namespace nServer {

cConnChoose::iterator cConnChoose::sBegin;
cConnChoose::iterator cConnChoose::sEnd;

/** Adding in mConnBaseList */
bool cConnChoose::AddConn(cConnBase *conn) {
	if(!conn) return false;

	tSocket s = (tSocket)(*conn);

	if(mConnBaseList.Contain(s)) return false;
	if(s >= mMaxSocket) mMaxSocket = s + 1;

	bool ret = mConnBaseList.Add(s, conn);
	return ret;
}

/** Del from mConnBaseList */
bool cConnChoose::DelConn(cConnBase *conn) {
	OptOut(conn, eEF_ALL_AND_CLOSE);
	return mConnBaseList.Remove((tSocket)(*conn));
}

/** HasConn */
bool cConnChoose::HasConn(cConnBase *conn) {
	return mConnBaseList.Contain((tSocket)(*conn));
}

}; // nServer
