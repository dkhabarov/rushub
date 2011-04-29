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

#ifndef PLUGIN_LIST_H
#define PLUGIN_LIST_H

#include "HashMap.h"
#include "CallList.h"
#include "Plugin.h"

#include "DcConn.h" // DcConn and DcConnBase conversion
#include "WebConn.h"

#include <string>
#include <iostream>


using ::std::string;
using ::std::ostream;

using namespace ::utils;
using namespace ::webserver;



namespace dcserver {

	class DcServerBase;
	class DcUser;
	class UserList;

}; // namespace dcserver



namespace plugin {

using namespace ::dcserver;

class PluginLoader;



class PluginList : public Obj, public PluginListBase {

	/** For LoadAll */
	friend class ::dcserver::DcServer;

	/** For SetCallList */
	friend class CallList;

public:

	typedef HashMap<PluginLoader *> PluginLoaderMap;

public:

	PluginList(const string & path);

	virtual ~PluginList();

	virtual PluginList & operator = (const PluginList &) {
		return *this;
	}

	void setServer(DcServerBase *);

	virtual const string & getPluginDir() const {
		return mPluginDir;
	}

	/** Load all plugins */
	bool loadAll();

	/** Load plugin by lib */
	bool loadPlugin(const string & filePath);

	/** Unload plugin by name */
	bool unloadPlugin(const string & name);

	/** Reload plugin by name */
	bool reloadPlugin(const string & name);

	/** Set call list */
	bool setCallList(string id, CallList *);


	virtual bool regCallList(const char * id, Plugin *);

	virtual bool unregCallList(const char * id, Plugin *);


	void list(ostream & os);

	void listAll(ostream & os);


	Plugin * getPlugin(const string & name);

	Plugin * getPluginByLib(const string & lib);


	/** onPluginLoad event */
	void onPluginLoad(Plugin *);

protected:

	typedef unsigned long Hash_t;
	typedef HashMap<CallList *> CallListMap;

	vector<Hash_t> mKeyList;

	/** Plugin dir */
	const string & mPluginDir;

	/** List with loaders of plugins */
	PluginLoaderMap mPluginLoaders;
	vector<Hash_t> mPluginLoaderHash;
	CallListMap mCallLists;

private:

	DcServerBase * mDcServerBase;

}; // class PluginList



class CallListBase : public CallList {

public:

	CallListBase(PluginList * pluginList, const char * id) :
		CallList(pluginList, string(id))
	{
	}

	virtual int callOne(Plugin *) = 0;

}; // class CallListBase



class CallListSimple : public CallListBase {

public:

	typedef int (Plugin::*tpFunc) ();

public:

	CallListSimple(PluginList * pluginList, const char * id, tpFunc func) :
		CallListBase(pluginList, id),
		mFunc(func)
	{
	}

	virtual ~CallListSimple() {
	}

	virtual int callOne(Plugin * plugin) {
		return (plugin->*mFunc)();
	}

protected:

	tpFunc mFunc;

}; // class CallListSimple


template <class A>
class CallListType1 : public CallListBase {

public:

	typedef int (Plugin::*tpFunc) (A);

public:

	CallListType1(PluginList * pluginList, const char * id, tpFunc func) :
		CallListBase(pluginList, id),
		mFunc(func)
	{
	}

	virtual ~CallListType1() {
	}

	virtual int callOne(Plugin * plugin) {
		return (plugin->*mFunc) (mData);
	}

	virtual int callAll(A t) {
		mData = t;
		return CallList::callAll();
	}

protected:

	tpFunc mFunc;
	A mData;

}; // class CallListType1



template <class A, class B>
class CallListType2 : public CallListBase {

public:

	typedef int (Plugin::*tpFunc) (A, B);

public:

	CallListType2(PluginList * pluginList, const char * id, tpFunc func) :
		CallListBase(pluginList, id),
		mFunc(func)
	{
	}

	virtual ~CallListType2() {
	}

	virtual int callOne(Plugin * plugin) {
		return (plugin->*mFunc) (mData1, mData2);
	}

	virtual int callAll(A a, B b) {
		mData1 = a;
		mData2 = b;
		return CallList::callAll();
	}

protected:

	tpFunc mFunc;
	A mData1;
	B mData2;

}; // class CallListType2

template <class A, class B, class C>
class CallListType3 : public CallListBase {

public:

	typedef int (Plugin::*tpFunc) (A, B, C);

public:

	CallListType3(PluginList * pluginList, const char * id, tpFunc func) :
		CallListBase(pluginList, id),
		mFunc(func)
	{
	}

	virtual ~CallListType3() {
	}

	virtual int callOne(Plugin * plugin) { return (plugin->*mFunc) (mData1, mData2, mData3); }

	virtual int callAll(A a, B b, C c) {
		mData1 = a;
		mData2 = b;
		mData3 = c;
		return CallList::callAll();
	}

protected:

	tpFunc mFunc;
	A mData1;
	B mData2;
	C mData3;

}; // class CallListType3

class CallListConnection : public CallListBase {

public:

	typedef int (Plugin::*tpFuncConn) (DcConnBase *);

public:

	CallListConnection(PluginList * pluginList, const char * id, tpFuncConn func) :
		CallListBase(pluginList, id),
		mFunc(func)
	{
		mDcConn = NULL;
	}

	virtual ~CallListConnection() {
	}

	virtual int callOne(Plugin * plugin) {
		return (plugin->*mFunc) (mDcConn);
	}

	virtual int callAll(DcConnBase * dcConnBase) {
		mDcConn = static_cast<DcConn *> (dcConnBase);
		if (mDcConn != NULL) {
			return this->CallList::callAll();
		} else {
			return 1;
		}
	}

protected:

	tpFuncConn mFunc;
	DcConn * mDcConn;

}; // class CallListConnection

typedef CallListType1<DcConnBase *> CallListConn;
typedef CallListType2<DcConnBase *, WebParserBase *> CallListConnWebParser;
typedef CallListType2<DcConnBase *, int> CallListConnInt;
typedef CallListType3<DcConnBase *, int, int> CallListConnIntInt;

}; // namespace plugin

#endif // PLUGIN_LIST_H

/**
 * $Id$
 * $HeadURL$
 */
