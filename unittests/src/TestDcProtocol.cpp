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

#if (defined _DEBUG) && (!defined UNITTESTS_OFF)

#include "UnitTest++.h"
#include "TigerHash.h"


using namespace dcserver;

SUITE(TestDcProtocol) {

// TODO: problem with calculation code coverage

TEST(TestNmdcUser) {

	// mapping user properties NMDC <-> ADC:
	/*
		uid - ID (cid)

		sNick - NI (string)

		sDesc - DE (string)
		sEmail - EM (string)
		sClientName - AP (string)
		sClientVersion - VE (string)
		iUsHubs - HN (int)
		iRegHubs - HR (int)
		iOpHubs - HO (int)
		iSlots - SL (int)
		iLimit - US (int)
		iOpen - FS (int)
		iBandwidth - AS (int)
		iDownload - DS (int)
		iByte - mapping on AW (int)
		iShare - SS (int64)

		sTag - only NMDC (string) (calc field)
		sMode - only NMDC (string) (calc field)
		sFraction - only NMDC (string) (calc field)
		sConn - only NMDC (string)
		sSupports - only NMDC (string)
		sVersion - only NMDC (string)


		properties without inf flag:

		sIP (string)
		sIPConn (string)
		sMacAddress (string)
		iProfile (int)
		iPort (int)
		iPortConn (int)
		iEnterTime (int)
		bKick (bool)
		bRedirect (bool)
		bHide (bool)
		bInUserList (bool)
		bInIpList (bool)
		bInOpList (bool)

	*/

	CHECK_EQUAL("1", "1");
}

}; // SUITE(TestDcProtocol)

#endif // _DEBUG

/**
 * $Id$
 * $HeadURL$
 */
