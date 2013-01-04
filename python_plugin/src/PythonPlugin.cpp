/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * Copyright (C) 2009-2013 by Setuper
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


#include "PythonPlugin.h"
#include "Dir.h"

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif // HAVE_CONFIG_H

#if HAVE_TINYXML_H
  #include <tinyxml.h>
#else
  #include "tinyxml/tinyxml.h"
#endif // HAVE_TINYXML_H

#ifdef _WIN32
	#pragma comment(lib, "tinyxml.lib")
	#pragma comment(lib, "python3.lib")
#endif // _WIN32

using namespace ::utils;


PythonPlugin::PythonPlugin() {
	mName = PLUGIN_NAME;
	mVersion = PLUGIN_VERSION;
}



PythonPlugin::~PythonPlugin() {
}



void PythonPlugin::onLoad(DcServerBase * dcServerBase) {
	regAll(dcServerBase);
}



/// Registration all events
bool PythonPlugin::regAll(DcServerBase * dcServerBase) {
	dcServerBase->regCallList("Timer",       this);
	dcServerBase->regCallList("Conn",        this);
	dcServerBase->regCallList("Disconn",     this);
	dcServerBase->regCallList("Enter",       this);
	dcServerBase->regCallList("Exit",        this);
	dcServerBase->regCallList("Supports",    this);
	dcServerBase->regCallList("Key",         this);
	dcServerBase->regCallList("Validate",    this);
	dcServerBase->regCallList("MyPass",      this);
	dcServerBase->regCallList("Version",     this);
	dcServerBase->regCallList("GetNickList", this);
	dcServerBase->regCallList("MyINFO",      this);
	dcServerBase->regCallList("Chat",        this);
	dcServerBase->regCallList("To",          this);
	dcServerBase->regCallList("CTM",         this);
	dcServerBase->regCallList("RCTM",        this);
	dcServerBase->regCallList("Search",      this);
	dcServerBase->regCallList("SR",          this);
	dcServerBase->regCallList("Kick",        this);
	dcServerBase->regCallList("OpForce",     this);
	dcServerBase->regCallList("GetINFO",     this);
	dcServerBase->regCallList("MCTo",        this);
	dcServerBase->regCallList("Any",         this);
	dcServerBase->regCallList("Unknown",     this);
	dcServerBase->regCallList("Flood",       this);
	dcServerBase->regCallList("WebData",     this);
	return true;
}


// OnTimer
int PythonPlugin::onTimer() {
	return 0;
}

// OnFlood
int PythonPlugin::onFlood(DcUserBase *, int, int) {
	return 0;
}

// onWebData(WebID, sData)
int PythonPlugin::onWebData(WebUserBase *) {
	return 0;
}

// OnAny
int PythonPlugin::onAny(DcUserBase *, int) {
	return 0;
}

// OnUserConnected(tUser)
int PythonPlugin::onUserConnected(DcUserBase * dcUserBase) {
	return 0;
}

// OnUserDisconnected(tUser)
int PythonPlugin::onUserDisconnected(DcUserBase * dcUserBase) {
	return 0;
}

// OnUserEnter(tUser)
int PythonPlugin::onUserEnter(DcUserBase * dcUserBase) {
	return 0;
}

// OnUserExit(tUser)
int PythonPlugin::onUserExit(DcUserBase * dcUserBase) {
	return 0;
}

// OnSupports(tUser, sData)
int PythonPlugin::onSupports(DcUserBase * dcUserBase) {
	return 0;
}

// OnKey(tUser, sData)
int PythonPlugin::onKey(DcUserBase * dcUserBase) {
	return 0;
}

// OnUnknown(tUser, sData)
int PythonPlugin::onUnknown(DcUserBase * dcUserBase) {
	return 0;
}

// OnValidateNick(tUser, sData)
int PythonPlugin::onValidateNick(DcUserBase * dcUserBase) {
	return 0;
}

// OnMyPass(tUser, sData)
int PythonPlugin::onMyPass(DcUserBase * dcUserBase) {
	return 0;
}

// OnVersion(tUser, sData)
int PythonPlugin::onVersion(DcUserBase * dcUserBase) {
	return 0;
}

// OnGetNickList(tUser, sData)
int PythonPlugin::onGetNickList(DcUserBase * dcUserBase) {
	return 0;
}

// OnMyINFO(tUser, sData)
int PythonPlugin::onMyINFO(DcUserBase * dcUserBase) {
	return 0;
}

// OnChat(tUser, sData)
int PythonPlugin::onChat(DcUserBase * dcUserBase) {
	return 0;
}

// OnTo(tUser, sData)
int PythonPlugin::onTo(DcUserBase * dcUserBase) {
	return 0;
}

// OnConnectToMe(tUser, sData)
int PythonPlugin::onConnectToMe(DcUserBase * dcUserBase) {
	return 0;
}

// OnRevConnectToMe(tUser, sData)
int PythonPlugin::onRevConnectToMe(DcUserBase * dcUserBase) {
	return 0;
}

// OnSearch(tUser, sData)
int PythonPlugin::onSearch(DcUserBase * dcUserBase) {
	return 0;
}

// OnSR(tUser, sData)
int PythonPlugin::onSR(DcUserBase * dcUserBase) {
	return 0;
}

// OnKick(tUser, sData)
int PythonPlugin::onKick(DcUserBase * dcUserBase) {
	return 0;
}

// OnOpForceMove(tUser, sData)
int PythonPlugin::onOpForceMove(DcUserBase * dcUserBase) {
	return 0;
}

// OnGetINFO(tUser, sData)
int PythonPlugin::onGetINFO(DcUserBase * dcUserBase) {
	return 0;
}

// OnMCTo(tUser, sData)
int PythonPlugin::onMCTo(DcUserBase * dcUserBase) {
	return 0;
}


REG_PLUGIN(PythonPlugin);


/**
 * $Id$
 * $HeadURL$
 */
