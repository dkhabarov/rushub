/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
 *
 * modified: 27 Aug 2009
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

#ifndef CALL_LIST_H
#define CALL_LIST_H

#include "Obj.h"
#include "Plugin.h"

#include <string>
#include <iostream>
#include <list>
#include <algorithm>
#include <functional>

using namespace ::std;
using namespace ::utils;



namespace plugin {

class Plugin;
class PluginList;
struct ufCallOne;

/// Subclass for call list
class CallList : public Obj {

	friend struct ufCallOne; // for mPluginList
	friend class PluginList; // for mName

public:

	/** Unary function of plugin list for current call list */
	struct ufCallOne : public unary_function<void, list<Plugin *>::iterator> {

	public:

		/** Returning value from plugin */
		int mCallReturn;

		ufCallOne();

		void setCallList(CallList * callList);

		void operator() (Plugin *);

	private:

		/** Pointer on call list */
		CallList * mCallList;

	};

public:

	CallList(PluginList *, const string & id);

	virtual ~CallList();

	/** Get name of the call list */
	virtual const string & getName() const;

	/** Registers plugin in call list */
	bool reg(Plugin *);

	/** Remove registration from call list */
	bool unreg(Plugin *);

	/** Call all plugins */
	int callAll();

	/** Call one plugin */
	virtual int callOne(Plugin *) = 0;

private:

	typedef list<Plugin *> tPlugins;

	/// Plugin list of call list
	tPlugins mPlugins;

	/// Pointer on common plugin list
	PluginList * mPluginList;

	/// Unary function
	ufCallOne mCallOne;

	/// Name of the call list
	string mName;

	typedef vector<Plugin *> RemovedPlugins_t;

	/// Removed plugins
	RemovedPlugins_t removedPlugins;

	CallList(const CallList &);
	CallList & operator = (const CallList &);

}; // class CallList



/// Base class for call list
class CallListBase : public CallList {

public:

	CallListBase(PluginList * pluginList, const string & id) :
		CallList(pluginList, id)
	{
	}

	virtual int callOne(Plugin *) = 0;

}; // class CallListBase



/// Simple call list
class CallListSimple : public CallListBase {

public:

	typedef int (Plugin::*tpFunc) ();

public:

	CallListSimple(PluginList * pluginList, const string & id, tpFunc func) :
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



/// Call list with one argument
template <class A>
class CallListType1 : public CallListBase {

public:

	typedef int (Plugin::*tpFunc) (A);

public:

	CallListType1(PluginList * pluginList, const string & id, tpFunc func) :
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

private:

	CallListType1(const CallListType1<A> &);
	CallListType1<A>	& operator = (const CallListType1<A> &);

}; // class CallListType1



/// Call list with two arguments
template <class A, class B>
class CallListType2 : public CallListBase {

public:

	typedef int (Plugin::*tpFunc) (A, B);

public:

	CallListType2(PluginList * pluginList, const string & id, tpFunc func) :
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

private:

	CallListType2(const CallListType2<A, B> &);
	CallListType2<A, B>	& operator = (const CallListType2<A, B> &);

}; // class CallListType2



/// Call list with three arguments
template <class A, class B, class C>
class CallListType3 : public CallListBase {

public:

	typedef int (Plugin::*tpFunc) (A, B, C);

public:

	CallListType3(PluginList * pluginList, const string & id, tpFunc func) :
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

private:

	CallListType3(const CallListType3<A, B, C> &);
	CallListType3<A, B, C>	& operator = (const CallListType3<A, B, C> &);

}; // class CallListType3


typedef CallListType1<DcUserBase *> CallListUser;
typedef CallListType1<WebUserBase *> CallListWebUser;
typedef CallListType2<DcUserBase *, int> CallListUserInt;
typedef CallListType3<DcUserBase *, int, int> CallListUserIntInt;


} // namespace plugin

#endif // CALL_LIST_H

/**
 * $Id$
 * $HeadURL$
 */
