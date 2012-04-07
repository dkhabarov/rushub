/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2012 by Setuper
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

#include "DcProtocol.h"
#include "DcServer.h" // for mDcServer
#include "DcConn.h" // for DcConn

namespace dcserver {

namespace protocol {


DcProtocol::DcProtocol() :
	mDcServer(NULL)
{
	setClassName("DcProtocol");
}



DcProtocol::~DcProtocol() {
}



// return true if use cache
const string & DcProtocol::getFirstMsg(bool & useCache) {

	static int64_t shareVal = -1;
	static int usersVal = -1;
	static long timeVal = -1;
	static string timeCache, shareCache, cache;
	Time Uptime(mDcServer->mTime);
	Uptime -= mDcServer->mStartTime;
	long min = Uptime.sec() / 60;
	if (timeVal != min) {
		timeVal = min;
		useCache = false;
		stringstream oss;
		int w, d, h, m;
		Uptime.asTimeVals(w, d, h, m);
		if (w) {
			oss << w << " " << mDcServer->mDcLang.mTimes[0] << " ";
		}
		if (d) {
			oss << d << " " << mDcServer->mDcLang.mTimes[1] << " ";
		}
		if (h) {
			oss << h << " " << mDcServer->mDcLang.mTimes[2] << " ";
		}
		oss << m << " " << mDcServer->mDcLang.mTimes[3];
		timeCache = oss.str();
	}
	if (shareVal != mDcServer->miTotalShare) {
		shareVal = mDcServer->miTotalShare;
		useCache = false;
		DcServer::getNormalShare(shareVal, shareCache);
	}
	if (usersVal != mDcServer->getUsersCount()) {
		usersVal = mDcServer->getUsersCount();
		useCache = false;
	}	

	if (!useCache) {
		stringReplace(mDcServer->mDcLang.mFirstMsg, string(STR_LEN("HUB")), cache, string(STR_LEN(INTERNALNAME " " INTERNALVERSION)));
		stringReplace(cache, string(STR_LEN("uptime")), cache, timeCache);
		stringReplace(cache, string(STR_LEN("users")), cache, usersVal);
		stringReplace(cache, string(STR_LEN("share")), cache, shareCache);
	}

	return cache;
}


bool DcProtocol::checkState(DcConn * dcConn, const char * cmd, unsigned int state) {
	if (dcConn->isState(state)) {
		if (dcConn->log(LEVEL_DEBUG)) {
			dcConn->logStream() << "Bad state in " << cmd << endl;
		}
		dcConn->closeNow(CLOSE_REASON_CMD_STATE);
		return false;
	}
	return true;
}


}; // namespace protocol

}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
