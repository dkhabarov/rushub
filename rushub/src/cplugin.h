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

#ifndef CPLUGIN_H
#define CPLUGIN_H

#include "cpluginbase.h"
#include "cdcconnbase.h"
#include "cwebparserbase.h"
#include "cdcserverbase.h"
#include "cdcuserbase.h"
#include "cdcparserbase.h"

using namespace ::nPlugin;
using namespace nWebServer;

namespace nDCServer {

using namespace nProtoEnums;

namespace nPlugin {

using namespace ::nDCServer;

class cPlugin : public cPluginBase {
public:
	cDCServerBase * mDCServer;

public:
	cPlugin(){}
	virtual ~cPlugin(){}

	virtual bool RegAll(cPluginListBase*) = 0;
	virtual void OnLoad(cDCServerBase * DCServer) { mDCServer = DCServer; }
	virtual int OnUserConnected(cDCConnBase *) { return 1; }
	virtual int OnUserDisconnected(cDCConnBase *) { return 1; }
	virtual int OnUserEnter(cDCConnBase *) { return 1; }
	virtual int OnUserExit(cDCConnBase *) { return 1; }
	virtual int OnSupports(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnKey(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnValidateNick(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnMyPass(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnVersion(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnGetNickList(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnMyINFO(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnChat(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnTo(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnConnectToMe(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnRevConnectToMe(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnSearch(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnSR(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnKick(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnOpForceMove(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnGetINFO(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnMCTo(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnTimer() { return 1; }
	virtual int OnAny(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnUnknown(cDCConnBase *, cDCParserBase *) { return 1; }
	virtual int OnFlood(cDCConnBase *, int, int) { return 1; }
	virtual int OnWebData(cDCConnBase *, cWebParserBase *) { return 1; }

}; // cPlugin

}; // nPlugin

}; // nDCServer

#endif // CPLUGIN_H
