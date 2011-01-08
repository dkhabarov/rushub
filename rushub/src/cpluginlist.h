/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

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

#ifndef CPLUGINLIST_H
#define CPLUGINLIST_H

#include "tchashmap.h"
#include "ccalllist.h"
#include "cplugin.h"

#include "cdcconn.h" /** cDCConn and cDCConnBase conversion */
#include "cwebconn.h"

#include <string>
#include <iostream>

using ::std::string;
using ::std::ostream;

using namespace ::nUtils;
using namespace nWebServer;

namespace nDCServer {
	class cDCServerBase;
	class cDCUser;
	class cUserList;
	class cDCTag;
	class cDCParserBase;
};

namespace nPlugin {

using namespace ::nDCServer;

class cPluginLoader;

class cPluginList : public cObj, public cPluginListBase {
	friend class nDCServer::cDCServer; /** For LoadAll */
	friend class cCallList; /** For SetCallList */

public:
	typedef tcHashMap<cPluginLoader*> tPluginList;

public:
	cPluginList(const string & sPath);
	virtual ~cPluginList() {}
	virtual cPluginList & operator=(const cPluginList &) { return *this; }
	void SetServer(cDCServerBase *);

	virtual const string & GetPluginDir() const { return msPluginDir; }

	bool LoadAll(); /** Load all plugins */
	bool LoadPlugin(const string &sPathFile); /** Load plugin by lib */
	bool UnloadPlugin(const string &sName); /** Unload plugin by name */
	bool ReloadPlugin(const string &sName); /** Reload plugin by name */
	bool SetCallList(string sId, cCallList*); /** Set call list */

	virtual bool RegCallList(const char * sId, cPlugin*);
	virtual bool UnregCallList(const char * sId, cPlugin*);

	void List(ostream &os);
	void ListAll(ostream &os);

	cPlugin * GetPlugin(const string &sName);
	cPlugin * GetPluginByLib(const string &sLib);

	void OnPluginLoad(cPlugin*); /** OnPluginLoad event */

private:
	cDCServerBase * mDCServer;

protected:
	typedef tcHashMap<cCallList*> tAllCallLists;

	const string & msPluginDir; /** Plugin dir */

	tPluginList mPluginList; /** List with loaders of plugins */
	tAllCallLists mAllCallLists;

}; // cPluginList

class cCallListBase : public cCallList {

public:
	cCallListBase(cPluginList *PluginList, const char * sId) :
		cCallList(PluginList, string(sId))
	{}
	virtual int CallOne(cPlugin * Plugin) = 0;
}; // cCallListBase

class cCL_Simple : public cCallListBase {

public:
	typedef int (cPlugin::*tpFunc) ();

protected:
	tpFunc mFunc;

public:
	cCL_Simple(cPluginList *PluginList, const char *sId, tpFunc pFunc) :
		cCallListBase(PluginList, sId), mFunc(pFunc)
	{}
	virtual ~cCL_Simple() {}
	virtual int CallOne(cPlugin * Plugin) { return (Plugin->*mFunc)(); }
}; // cCL_Simple


template <class Type1> class tcCallListType1 : public cCallListBase {

public:
	typedef int (cPlugin::*tpFunc1) (Type1*);

protected:
	tpFunc1 mFunc1;
	Type1 *mData1;

public:
	tcCallListType1(cPluginList * PluginList, const char* sId, tpFunc1 pFunc) :
		cCallListBase(PluginList, sId), mFunc1(pFunc)
	{ mData1 = NULL; }
	virtual ~tcCallListType1() {}

	virtual int CallOne(cPlugin *Plugin) { return (Plugin->*mFunc1)(mData1); }
	virtual int CallAll(Type1 *param1) {
		mData1 = param1;
		if((mData1 != NULL)) return this->cCallList::CallAll();
		else return 1;
	}
}; // tcCallListType1

template <class Type1, class Type2> class tcCallListType2 : public cCallListBase {

public:
	typedef int (cPlugin::*tpFunc2) (Type1*, Type2*);

protected:
	tpFunc2 mFunc2;
	Type1 *mData1;
	Type2 *mData2;

public:
	tcCallListType2(cPluginList *PluginList, const char* sId, tpFunc2 pFunc) :
		cCallListBase(PluginList, sId), mFunc2(pFunc)
	{ mData1 = NULL; mData2 = NULL; }
	virtual ~tcCallListType2() {}

	virtual int CallOne(cPlugin *Plugin) { return (Plugin->*mFunc2)(mData1, mData2); }
	virtual int CallAll(Type1 *param1, Type2 *param2) {
		mData1 = param1;
		mData2 = param2;
		if((mData1 != NULL) && (mData2 !=NULL)) return this->cCallList::CallAll();
		else return 1;
	}
}; // tcCallListType2

template <class Type1, class Type2, class Type3> class tcCallListType3 : public cCallListBase {

public:
	typedef int (cPlugin::*tpFunc3) (Type1 , Type2 , Type3);

protected:
	tpFunc3 mFunc3;
	Type1 mData1;
	Type2 mData2;
	Type3 mData3;

public:
	tcCallListType3(cPluginList *PluginList, const char* sId, tpFunc3 pFunc) :
		cCallListBase(PluginList, sId), mFunc3(pFunc)
	{}
	virtual ~tcCallListType3() {}
	virtual int CallOne(cPlugin *Plugin) { return (Plugin->*mFunc3)(mData1, mData2, mData3); }
	virtual int CallAll(Type1 param1, Type2 param2, Type3 param3) {
		mData1 = param1;
		mData2 = param2;
		mData3 = param3;
		return this->cCallList::CallAll();
	}
}; // tcCallListType3

class cCL_Connection : public cCallListBase {

public:
	typedef int (cPlugin::*tpFuncConn) (cDCConnBase *);

protected:
	tpFuncConn mFunc;
	cDCConn *mDCConn;

public:
	cCL_Connection(cPluginList *PluginList, const char* sId, tpFuncConn pFunc) :
		cCallListBase(PluginList, sId), mFunc(pFunc)
	{ mDCConn = NULL; }
	virtual ~cCL_Connection() {}

	virtual int CallOne(cPlugin *Plugin) { return (Plugin->*mFunc)(mDCConn); }
	virtual int CallAll(cDCConnBase *dcconn) {
		mDCConn = (cDCConn*)dcconn;
		if(mDCConn != NULL) return this->cCallList::CallAll();
		else return 1;
	}
}; // cCL_Connection

typedef tcCallListType1<cDCUserBase> cCL_User;
typedef tcCallListType1<string> cCL_String;
typedef tcCallListType2<cDCConnBase, string> cCL_ConnString;
typedef tcCallListType2<cDCUserBase, cDCUserBase> cCL_UserUser;
typedef tcCallListType2<cDCConnBase, cDCParserBase> cCL_ConnParser;
typedef tcCallListType2<cDCConnBase, cWebParserBase> cCL_ConnWebParser;
typedef tcCallListType3<cDCUserBase*, cDCUserBase*, string*> cCL_UserUserString;
typedef tcCallListType3<cDCConnBase*, int, int> cCL_ConnIntInt;

}; // nPlugin

#endif // CPLUGINLIST_H
