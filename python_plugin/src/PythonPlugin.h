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

#ifndef PYTHON_PLUGIN_H
#define PYTHON_PLUGIN_H

#include "Plugin.h" // ::dcserver
#include "PythonInterpreter.h"
#include "apiFuncs.h"

#include <vector>
#include <list>
#include <string>

// Visual Leak Detector
#ifdef VLD
	#include <vld.h>
#endif

#define PLUGIN_NAME "PythonPlugin"
#define PLUGIN_VERSION "1.0"


#ifndef STR_LEN
# define STR_LEN(S) S , sizeof(S) / sizeof(S[0]) - 1
#endif

namespace dcserver {
	class DcServerBase;
}; // namespace dcserver


using namespace ::std;
using namespace ::plugin;
using namespace ::dcserver;
using namespace ::pythonplugin;


class PythonPlugin : public Plugin {

public:

	PythonPlugin();
	virtual ~PythonPlugin();

	virtual void onLoad(DcServerBase *); ///< Actions when loading plugin
	bool regAll(DcServerBase *); ///< Registration all events for this plugin

	// events
	virtual int onUserConnected(DcUserBase *);
	virtual int onUserDisconnected(DcUserBase *);
	virtual int onUserEnter(DcUserBase *);
	virtual int onUserExit(DcUserBase *);
	virtual int onSupports(DcUserBase *);
	virtual int onKey(DcUserBase *);
	virtual int onValidateNick(DcUserBase *);
	virtual int onMyPass(DcUserBase *);
	virtual int onVersion(DcUserBase *);
	virtual int onGetNickList(DcUserBase *);
	virtual int onMyINFO(DcUserBase *);
	virtual int onChat(DcUserBase *);
	virtual int onTo(DcUserBase *);
	virtual int onConnectToMe(DcUserBase *);
	virtual int onRevConnectToMe(DcUserBase *);
	virtual int onSearch(DcUserBase *);
	virtual int onSR(DcUserBase *);
	virtual int onKick(DcUserBase *);
	virtual int onOpForceMove(DcUserBase *);
	virtual int onGetINFO(DcUserBase *);
	virtual int onMCTo(DcUserBase *);
	virtual int onTimer();
	virtual int onAny(DcUserBase *, int type);
	virtual int onUnknown(DcUserBase *);
	virtual int onFlood(DcUserBase *, int type1, int type2);
	virtual int onWebData(WebUserBase *);

}; // class PythonPlugin

#endif // PYTHON_PLUGIN_H

/**
 * $Id$
 * $HeadURL$
 */
