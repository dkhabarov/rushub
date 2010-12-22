/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

 * modified: 10 Dec 2009
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

#ifndef CUSERLIST_H
#define CUSERLIST_H

#include "cobj.h"
#include "tchashtable.h"
#include "cplugin.h" /** for DC_SEPARATOR */

#include <string>
#include <functional>
#include <algorithm> /** for ::std::transform */
#include <vector>

//#define WITHOUT_PLUGINS 1

using namespace nUtils; /** for tcHashTable */
using namespace std;

namespace nDCServer {

class cDCUser;
class cUserBase;

/** The structure, allowing add and delete the users. Quick iterations cycle for sending */
class cUserList : public cObj, public tcHashTable<cUserBase*> {

friend class cDCServer;

public:

	/** Unary function for sending data to users */
	struct ufSend : public unary_function<void, iterator> {
		string &msData; /** Data for sending */
		bool mbProfile;
		unsigned long miProfile;
		ufSend(string &sData) : msData(sData), mbProfile(false) {}
		ufSend(string &sData, unsigned long iProfile) : msData(sData), mbProfile(true), miProfile(iProfile) {}
		void operator()(cUserBase *); /** Sending operator */
	};

	/** Unary function for sending data sDataS+sNick+sDataE to each user */
	struct ufSendWithNick : public unary_function<void, iterator> {
		string &msDataStart, &msDataEnd; /** Data for sending */
		bool mbProfile;
		unsigned long miProfile;
		ufSendWithNick(string &sDataS, string &sDataE) : 
			msDataStart(sDataS),
			msDataEnd(sDataE),
			mbProfile(false)
		{}
		ufSendWithNick(string &sDataS, string &sDataE, unsigned long iProfile) : 
			msDataStart(sDataS),
			msDataEnd(sDataE),
			mbProfile(true),
			miProfile(iProfile)
		{}
		void operator()(cUserBase *); /** Sending operator */
	};

	/** Unary function for constructing nick-list */
	struct ufDoNickList : public unary_function<void, iterator> {
		string &msList; /** List */
		string msStart; /** Prefix */
		string msSep; /** Separator */

		ufDoNickList(string &sList) : msList(sList){}
		virtual ~ufDoNickList() {}
		virtual void Clear() { /** Clear var and record prefix */
			msList.erase(0, msList.size());
			msList.append(msStart.c_str(), msStart.size());
		}
		virtual void operator()(cUserBase *User);
	};

private:

	typedef tcHashTable<cUserBase*> List_t;

	string msName;
	string msCache;

	string msNickList;
	ufDoNickList mNickListMaker;

protected:

	bool mbKeepNickList;
	bool mbRemakeNextNickList;
	bool mbOptRemake; /** Flag of the absence of the change between remake */

public:

	cUserList(string sName, bool bKeepNickList = false);
	virtual ~cUserList() {}

	tKeyType Nick2Key(const string &sNick) {
		string sKey;
		sKey.assign(sNick);
		::std::transform(sKey.begin(), sKey.end(), sKey.begin(), ::tolower);
		return List_t::mHash(sKey);
	}
	cUserBase* GetUserBaseByNick(const string &sNick) { return List_t::Find(Nick2Key(sNick)); }
	cUserBase* GetUserBaseByKey(const tKeyType &Key) { return List_t::Find(Key); }
	bool ContainsNick(const string &sNick) { return List_t::Contain(Nick2Key(sNick)); }
	bool ContainsKey(const tKeyType &Key) { return List_t::Contain(Key); }
	bool AddWithNick(const string &sNick, cUserBase *User) { return List_t::Add(Nick2Key(sNick), User); }
	bool AddWithKey(const tKeyType &Key, cUserBase *User) { return List_t::Add(Key, User); }
	bool RemoveByNick(const string &Nick) { return List_t::Remove(Nick2Key(Nick)); }
	bool RemoveByKey(const tKeyType &Key) { return List_t::Remove(Key); }
	bool Add(cUserBase *User);
	bool Remove(cUserBase *User);

	virtual string &GetNickList();

	void SetNickListStart(const string &sStart) { mNickListMaker.msStart = sStart; }
	void SetNickListSeparator(const string &sSep) { mNickListMaker.msSep = sSep; }
	void Remake() { if(!mbOptRemake) mbRemakeNextNickList = true; else mbRemakeNextNickList = false; }

	/** Sending data to all from the list */
	void SendToAll(const string &sData, bool bUseCache = false, bool bAddSep = true);
	/** Sending data to profiles */
	void SendToProfiles(unsigned long iProfile, const string &sData, bool bAddSep = true);
	/** Sending data sStart+msNick+sEnd to all list */
	void SendToWithNick(string &sStart, string &sEnd);
	/** Sending data sStart+msNick+sEnd to profiles */
	void SendToWithNick(string &sStart, string &sEnd, unsigned long iProfile);
	/** Sending data from cache to user */
	void FlushForUser(cUserBase *User);
	/** Sending data from cache to all and clear cache */
	void FlushCache();

	bool AutoResize() {
		unsigned iSize, iCapacity, iNewSize;
		if(List_t::AutoResize(iSize, iCapacity, iNewSize) && Log(3)) {
			LogStream() << "Autoresizing: miSize = " << iSize << 
			", miCapacity = " << iCapacity << " -> " << iNewSize << endl;
			return true;
		}
		return false;
	}

	/** OnAdd */
	virtual void OnAdd(cUserBase *User) {
		if(!mbRemakeNextNickList && mbKeepNickList) mNickListMaker(User);
	}
	/** OnRemove */
	virtual void OnRemove(cUserBase*) {
		mbRemakeNextNickList = mbKeepNickList;
		mbOptRemake = false;
	}

	/** Redefining log level function */
	virtual int StrLog(ostream & ostr, int iLevel, int iMaxLevel, bool bIsError = false);

}; // cUserList


/** Full user list */
class cFullUserList : public cUserList {

public:

	/** Unary function for constructing MyINFO list */
	struct ufDoINFOList : public cUserList::ufDoNickList {
		string &msListComplete;
		ufDoINFOList(string &sList, string &sListComplete) :
			ufDoNickList(sList),
			msListComplete(sListComplete)
		{msSep = DC_SEPARATOR; msStart = "";}
		virtual ~ufDoINFOList(){}
		virtual void Clear(){ /** Clear var and record prefix */
			ufDoNickList::Clear();
			msListComplete.erase(0, msListComplete.size());
			msListComplete.append(msStart.data(), msStart.size());
		}
		virtual void operator()(cUserBase*);
	};

	/** Unary function for constructing ip-list */
	struct ufDoIpList : public cUserList::ufDoNickList {
		ufDoIpList(string &sList) : ufDoNickList(sList)
		{msSep = "$$"; msStart = "$UserIP ";}
		virtual ~ufDoIpList(){}
		virtual void operator()(cUserBase *User);
	};

private:

	string msINFOList;
	string msINFOListComplete;
	string msIpList;
	ufDoINFOList mINFOListMaker;
	ufDoIpList mIpListMaker;

protected:

	bool mbKeepInfoList;
	bool mbRemakeNextInfoList;

	bool mbKeepIpList;
	bool mbRemakeNextIpList;

	string msCompositeNickList;
	string msCompositeINFOList;

public:

	cFullUserList(string sName, bool bKeepNickList = false, bool bKeepInfoList = false, bool bKeepIpList = false);
	virtual ~cFullUserList() {}

	virtual string &GetNickList();
	virtual string &GetInfoList(bool bComplete = false);
	virtual string &GetIpList();

	inline void Remake() {
		if(!mbOptRemake)
			mbRemakeNextNickList = mbRemakeNextInfoList = mbRemakeNextIpList = true;
		else
			mbRemakeNextNickList = mbRemakeNextInfoList = mbRemakeNextIpList = false;
	}


	/** OnAdd */
	virtual void OnAdd(cUserBase *User) {
		cUserList::OnAdd(User);
		if(!mbRemakeNextInfoList && mbKeepInfoList) mINFOListMaker(User);
		if(!mbRemakeNextIpList && mbKeepIpList) mIpListMaker(User);
	}
	/** OnRemove */
	virtual void OnRemove(cUserBase *User) {
		cUserList::OnRemove(User);
		mbRemakeNextInfoList = mbKeepInfoList;
		mbRemakeNextIpList = mbKeepIpList;
	}
	
}; // cFullUserList

}; // nDCServer

#endif // CUSERLIST_H
