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

#include "ConnChoose.h"

namespace server {

/** Adding in mConnBaseList */
bool ConnChoose::addConn(ConnBase * connBase) {
	if (!connBase) {
		return false;
	}

	tSocket sock = (tSocket) (*connBase);
	if (mConnBaseList.contain(sock)) {
		return false;
	}

	if (sock >= mMaxSocket) {
		mMaxSocket = sock + 1;
	}

	return mConnBaseList.add(sock, connBase);
}

/** Del from mConnBaseList */
bool ConnChoose::deleteConn(ConnBase * connBase) {
	optOut(connBase, eEF_ALL_AND_CLOSE);
	return mConnBaseList.remove((tSocket) (*connBase));
}

/** Has conn */
bool ConnChoose::hasConn(ConnBase * connBase) {
	return mConnBaseList.contain((tSocket) (*connBase));
}

}; // server
