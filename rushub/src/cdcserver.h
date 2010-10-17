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

#ifndef CDCSERVER_H
#define CDCSERVER_H

#include "cserver.h"
#include "cdcserverbase.h"
#include "cdcconn.h"
#include "cdcprotocol.h"

#include "cdcconfig.h"
#include "cuserlist.h"
#include "cpluginlist.h"

#include "stringutils.h" /** for StringReplace */

#include <istream> /** for istringstream */

//#define WITHOUT_PLUGINS 1

#include "cconfigloader.h"
#include "cantiflood.h"
#include "cdciplist.h"
#include "cmainpath.h"

namespace nConfig{class cConfigLoader;};
using namespace nConfig;

namespace nDCServer {

using namespace nServer; /** for cServer */
using namespace nProtocol;
using namespace nUtils; /** for StringReplace */
using namespace nPlugin;


/** Индикаторы загруженности хаба */
typedef enum
{
  eSL_OK,         //< OK
  eSL_LOWER,      //< LOWER
  eSL_MIDDLE,     //< MIDDLE
  eSL_CRITICAL,   //< CRITICAL
  eSL_SYSTEM_DOWN //< SYSTEM_DOWN
} tSysLoading;

typedef enum /** Flood types */
{
  eFT_NO = -1,
  eFT_CHAT,
  eFT_TO,
  eFT_MYNIFO,
  eFT_NICKLIST,
  eFT_SEARCH,
  eFT_SR,
  eFT_CTM,
  eFT_RCTM,
  eFT_UNKNOWN
} tFloodType;

class cDCServer;

class cDCListIterator : public cDCConnListIterator
{
private:
  list<cConn *>::iterator mIt, mEnd;
public:
  cDCListIterator(cDCServer * server);
  ~cDCListIterator(){}
  virtual cDCConnBase * operator() (void) {
    if(mIt == mEnd) return NULL;
    cDCConn* Conn = (cDCConn*)(*mIt);
    if(!Conn) {++mIt; return this->operator()();}
    ++mIt;
    return Conn;
  }
}; // cDCListIterator

class cDCServer : public cServer, public cDCServerBase
{

  friend class ::nDCServer::cDCListIterator; /** for mConnList */
  friend class ::nDCServer::nProtocol::cDCProtocol; /** for BeforeUserEnter in cDCProtocol::DC_MyINFO */
  friend class ::nDCServer::cDCConn; /** for DoUserEnter in cDCConn::OnFlush and MinDelay in cDCConn::OnTimer */
  friend class ::nDCServer::cDCConnFactory; /** for RemoveFromDCUserList in cDCConnFactory::DelConn */
  friend class ::nWebServer::cWebConnFactory; /** for call plugins */

public:
  typedef cUserList::tKeyType tUserKey;

  static cDCServer * sCurrentServer; /** Current server */

  cMainPath mMainPath;
  tSysLoading mSysLoading; /** Indicator of the system overloading */
  string msSysVersion; /** Verion of OS System */
  
  cTime mStartTime; /** Start time of the hub */
  cDCProtocol mDCProtocol; /** DC Protocol */
  cDCParser mDCParser; /** Parser for checking syntax of commands */
  cDCConfig mDCConfig; /** Config settings of the server */
  cDCLang mDCLang; /** Settings of language */

  cFullUserList mDCUserList; /** User list */
  cUserList mDCBotList; /** Bot list */
  cUserList mOpList; /** Op list */
  cUserList mIpList; /** Ip list */
  cUserList mActiveList; /** Active user list */
  cUserList mHelloList; /** Hello user list */
  cUserList mEnterList;
  cUserList mChatList;
  
  int miTotalUserCount; /** Total number of the users */
  __int64 miTotalShare; /** Total hub share size */

  cConfigLoader mConfigLoader; /** Config loader */
  cPluginList mPluginList;
  
  string mTimeBuf; /** Time buffer for plugins */

  cDCIPList * mIPListConn; /** IP list of connections */

public:
  cDCServer(const string & sConfPath, const string &sExPath);
  virtual ~cDCServer();

  const string & GetMainDir() const { return mMainPath.msConfPath; }
  const string & GetTime(){ stringstream oss; oss << mTime.AsDate(); mTimeBuf = oss.str(); return mTimeBuf;}
  const string & GetHubInfo() const { return msHubName; }
  const string & GetLocale() const { return mDCConfig.msLocale; };
  const string & GetSystemVersion() const { return msSysVersion; }
  int GetMSec() const { cTime tm; return tm;}
  int GetUpTime() const { cTime tm; tm -= mStartTime; return tm.Sec(); } /** Work time (sec) */
  int GetUsersCount() const { return miTotalUserCount; }
  __int64 GetTotalShare() const { return miTotalShare; }
  
  void AddToOps(cDCUser * User);
  void DelFromOps(cDCUser * User);
  void AddToIpList(cDCUser * User);
  void DelFromIpList(cDCUser * User);
  void AddToHide(cDCUser * User);
  void DelFromHide(cDCUser * User);

  int CheckCmd(const string &);

  /** Listebing of ports */
  virtual int Listening(int iPort = 0);

  /** Main timer */
  virtual int OnTimer(cTime &now);

  /** Pointer on the user (or NULL) */
  cDCUser * GetDCUser(const char *sNick);
  void GetDCConnBase(const char * sIP,  vector<cDCConnBase *> & vconn);
  cDCUserBase * GetDCUserBase(const char *sNick);
  cDCConnListIterator * GetDCConnListIterator(){ return new cDCListIterator(this); }

  bool SendToUser(cDCConnBase *DCConn, const char *sData, char *sNick = NULL, char *sFrom = NULL);
  bool SendToNick(const char *sTo, const char *sData, char *sNick = NULL, char *sFrom = NULL);
  bool SendToAll(const char *sData, char *sNick = NULL, char *sFrom = NULL);
  bool SendToProfiles(unsigned long iProfile, const char *sData, char *sNick = NULL, char *sFrom = NULL);
  bool SendToIP(const char *sIP, const char *sData, unsigned long iProfile = 0, char *sNick = NULL, char *sFrom = NULL);
  bool SendToAllExceptNicks(List_t & NickList, const char *sData, char *sNick = NULL, char *sFrom = NULL);
  bool SendToAllExceptIps(List_t & IPList, const char *sData, char *sNick = NULL, char *sFrom = NULL);

  const char * GetConfig(const string & sName);
  const char * GetLang(const string & sName);
  bool SetConfig(const string & sName, const string & sValue);
  bool SetLang(const string & sName, const string & sValue);

  int RegBot(const string & sNick, const string & sMyINFO, const string & sIP, bool bKey = true);
  int UnregBot(const string & sNick);

protected:

  /** Function action when joining the client */
  int OnNewConn(cConn *);

  /** Returns pointer to line of the connection, in which will be recorded got data */
  string * GetPtrForStr(cConn *);

  /** Function of the processing enterring data */
  void OnNewData(cConn *, string *);

  /** Function checks min interval */
  bool MinDelay(cTime &then, double sec);

  /** Antiflood function */
  bool AntiFlood(unsigned &iCount, cTime &Time, const unsigned &iCountLimit, const double &iTimeLimit);
  
  /** Check validate user */
  bool ValidateUser(cDCConn *, const string &sNick);

  /** Check nick len */
  bool CheckNickLength(cDCConn *dcconn, const unsigned iLen);

  /** Actions before user entry */
  bool BeforeUserEnter(cDCConn *);

  /** User entry */
  void DoUserEnter(cDCConn *);

  /** Adding user in the user list */
  bool AddToUserList(cDCUser *);

  /** Removing user from the user list */
  bool RemoveFromDCUserList(cDCUser *);

  /** Show user to all */
  bool ShowUserToAll(cDCUser *);

  /** Actions after user entry */
  void AfterUserEnter(cDCConn *);

  /** Close server */
  void Close();

private:

  cTime mChecker; /** Checking time */
  tCLIt conn_it; /** Iterator for optimum */
  string msHubName; /** Hub name for plugins */
  string sBuf; /** Temp buffer */

  cListenFactory * mListenFactory;
  cWebListenFactory * mWebListenFactory;

  cAntiFlood mIPEnterFlood;
  struct cIPEnter {
    cTime mTime;
    unsigned miCount;
    cIPEnter() : miCount(0) {}
  };
  /** List recently came ip addresses */
  typedef tcList<unsigned long, cIPEnter*> tIPEnterList;
  tIPEnterList * mIPEnterList;

  void DelConn(cConn * conn);
  bool GetSysVersion();


  struct sCalls
  {
    sCalls(cPluginList * List) :
      mOnUserConnected(List, "Conn", &cPlugin::OnUserConnected),
      mOnUserDisconnected(List, "Disconn", &cPlugin::OnUserDisconnected),
      mOnUserEnter(List, "Enter", &cPlugin::OnUserEnter),
      mOnUserExit(List, "Exit", &cPlugin::OnUserExit),
      mOnSupports(List, "Supports", &cPlugin::OnSupports),
      mOnKey(List, "Key", &cPlugin::OnKey),
      mOnValidateNick(List, "Validate", &cPlugin::OnValidateNick),
      mOnMyPass(List, "MyPass", &cPlugin::OnMyPass),
      mOnVersion(List, "Version", &cPlugin::OnVersion),
      mOnGetNickList(List, "GetNickList", &cPlugin::OnGetNickList),
      mOnMyINFO(List, "MyINFO", &cPlugin::OnMyINFO),
      mOnChat(List, "Chat", &cPlugin::OnChat),
      mOnTo(List, "To", &cPlugin::OnTo),
      mOnConnectToMe(List, "CTM", &cPlugin::OnConnectToMe),
      mOnRevConnectToMe(List, "RCTM", &cPlugin::OnRevConnectToMe),
      mOnSearch(List, "Search", &cPlugin::OnSearch),
      mOnSR(List, "SR", &cPlugin::OnSR),
      mOnKick(List, "Kick", &cPlugin::OnKick),
      mOnOpForceMove(List, "OpForce", &cPlugin::OnOpForceMove),
      mOnGetINFO(List, "GetINFO", &cPlugin::OnGetINFO),
      mOnTimer(List, "Timer", &cPlugin::OnTimer),
      mOnAny(List, "Any", &cPlugin::OnAny),
      mOnUnknown(List, "Unknown", &cPlugin::OnUnknown),
      mOnFlood(List, "Flood", &cPlugin::OnFlood),
      mOnWebData(List, "WebData", &cPlugin::OnWebData)
    {};
    cCL_Connection     mOnUserConnected;
    cCL_Connection     mOnUserDisconnected;
    cCL_Connection     mOnUserEnter;
    cCL_Connection     mOnUserExit;
    cCL_ConnParser     mOnSupports;
    cCL_ConnParser     mOnKey;
    cCL_ConnParser     mOnValidateNick;
    cCL_ConnParser     mOnMyPass;
    cCL_ConnParser     mOnVersion;
    cCL_ConnParser     mOnGetNickList;
    cCL_ConnParser     mOnMyINFO;
    cCL_ConnParser     mOnChat;
    cCL_ConnParser     mOnTo;
    cCL_ConnParser     mOnConnectToMe;
    cCL_ConnParser     mOnRevConnectToMe;
    cCL_ConnParser     mOnSearch;
    cCL_ConnParser     mOnSR;
    cCL_ConnParser     mOnKick;
    cCL_ConnParser     mOnOpForceMove;
    cCL_ConnParser     mOnGetINFO;
    cCL_Simple         mOnTimer;
    cCL_ConnParser     mOnAny;
    cCL_ConnParser     mOnUnknown;
    cCL_ConnIntInt     mOnFlood;
    cCL_ConnWebParser  mOnWebData;
  } mCalls;

}; // cServer

}; // nDCServer

#endif // CDCSERVER_H
