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

#include "cdcserver.h"
#include "cwebconn.h"

using namespace nWebServer;

namespace nDCServer {

cDCListIterator::cDCListIterator(cDCServer * server) : 
  mIt(server->mConnList.begin()),
  mEnd(server->mConnList.end())
{
}

cDCServer * cDCServer::sCurrentServer = NULL;

cDCServer::cDCServer(const string & sConfPath, const string & sExPath):
  cServer(DC_SEPARATOR),
  mMainPath(sConfPath, sExPath),
  mSysLoading(eSL_OK),
  mDCUserList("UserList", true, true, true),
  mDCBotList("BotList", true),
  mOpList("OpList", true),
  mIpList("IpList"),
  mActiveList("ActiveList"),
  mHelloList("HelloList"),
  mEnterList("EnterList"),
  mChatList("ChatList"),
  miTotalUserCount(0),
  miTotalShare(0),
  mConfigLoader(),
  mPluginList(mMainPath.msConfPath + "plugins/"),
  msHubName(INTERNALNAME" "INTERNALVERSION" "__DATE__" "__TIME__),
  mListenFactory(NULL),
  mWebListenFactory(NULL),
  mIPEnterFlood(mDCConfig.miFloodCountReconnIp, mDCConfig.miFloodTimeReconnIp),
  mCalls(&mPluginList)
{
  SetClassName("cDCServer");

  sCurrentServer = this; /** Current server */

  mConnFactory = new cDCConnFactory(&mDCProtocol, this); /** cConnFactory */
  mIPListConn = new cDCIPList(); /** cDCIPList */

  mDCProtocol.SetServer(this);
  mDCConfig.SetServer(this);
  mDCLang.SetServer(this);
  mPluginList.SetServer(this);


  if(!mDCConfig.Load()) mDCConfig.Save();
  if(!mDCLang.Load()) mDCLang.Save();

  mPluginList.LoadAll(); /** Load plugins */

  /** Set params of lists */
  string nmdc;
  nmdc = "$NickList ";
  mDCUserList.SetNickListStart(nmdc);
  nmdc = "$OpList ";
  mOpList.SetNickListStart(nmdc);
  nmdc = "$$";
  mDCUserList.SetNickListSeparator(nmdc);
  mOpList.SetNickListSeparator(nmdc);

  nmdc = "$ActiveList ";
  mActiveList.SetNickListStart(nmdc);

  if(mDCConfig.mbRegMainBot) /** ����������� ��������� ���� */
    if(RegBot(mDCConfig.msHubBot, mDCConfig.msMainBotMyINFO, 
    mDCConfig.msMainBotIP, mDCConfig.mbMainBotKey) == -2)
      RegBot(mDCConfig.msHubBot, string("$ $$$0$"), 
      mDCConfig.msMainBotIP, mDCConfig.mbMainBotKey);

  if(!GetSysVersion()) msSysVersion = "unknown";
}

cDCServer::~cDCServer() {
  if(Log(1)) LogStream() << "Destruct cDCServer" << endl;

  cUserList::iterator it, it_e = mDCUserList.end();
  cDCUser *Us;
  for(it = mDCUserList.begin(); it != it_e;) {
    Us = (cDCUser *)*it;
    ++it;
    if(Us->mDCConn) DelConnection(Us->mDCConn);
    else if(Us->mbInUserList) this->RemoveFromDCUserList(Us);
  }
  // del User
  Close();
  if(mIPListConn) delete mIPListConn; mIPListConn = NULL;
  if(mListenFactory) delete mListenFactory; mListenFactory = NULL;
  if(mWebListenFactory) delete mWebListenFactory; mWebListenFactory = NULL;
  if(mConnFactory) delete mConnFactory; mConnFactory = NULL;
  
  //if(cObj::mOfs.is_open()) cObj::mOfs.close();
  //if(cObj::mErrOfs.is_open()) cObj::mErrOfs.close();
}

/** Close server */
void cDCServer::Close()
{
  mbRun = false;
  for(tCLIt it = mConnList.begin(); it != mConnList.end(); ++it) {
    DelConn(*it);
  }
  for(tLLIt it = mListenList.begin(); it != mListenList.end(); ++it) {
    DelConn(*it);
  }
}

void cDCServer::DelConn(cConn * conn) {
  if(conn) {
    mConnChooser.DelConn(conn);
    if(conn->mConnFactory) {
      cConnFactory * Factory = conn->mConnFactory;
      Factory->DelConn(conn);
    } else delete conn;
    conn = NULL;
  }
}

/** Listening ports */
int cDCServer::Listening(int iPort) {
  if(!mListenFactory) mListenFactory = new cListenFactory(this);
  int iRes = cServer::Listening(mListenFactory, msIp, (iPort != 0) ? iPort : miPort);
  cout << "Server " INTERNALNAME " " INTERNALVERSION " is running on " << msIp << ":" << miPort << " TCP" << endl;

  if(mDCConfig.mbWebServer) {
    if(!mWebListenFactory) mWebListenFactory = new cWebListenFactory(this);
    cServer::Listening(mWebListenFactory, mDCConfig.msWebServerIP, mDCConfig.miWebServerPort);
    cout << "Web-Server is running on " << mDCConfig.msWebServerIP << ":" << mDCConfig.miWebServerPort << " TCP" << endl;
  }

  /** Listening additional ports */
  istringstream is(mDCConfig.msSubPorts);
  int i = 1;
  while(i) {
    i = 0;
    is >> i;
    if(i) {
      cConn * conn = cServer::Listen(msIp, i);
      if(conn) conn->mListenFactory = mListenFactory;
    }
  }
  return iRes;
}

int cDCServer::OnTimer(cTime &now) {

  mHelloList.FlushCache();
  mDCUserList.FlushCache();
  mDCBotList.FlushCache();
  mEnterList.FlushCache();
  mOpList.FlushCache();
  mIpList.FlushCache();
  mChatList.FlushCache();
  mActiveList.FlushCache();

  /** Execute each second */
  if(abs(int(now - mChecker)) >= 1000) {
    mChecker = now;

    if(0 < mMeanFrequency.mNumFill) {
      double iFrequency = mMeanFrequency.GetMean(mTime);
      if     (iFrequency < 0.4 * mDCConfig.miSysLoading) mSysLoading = eSL_SYSTEM_DOWN; // 0.4 (>2000 ms)
      else if(iFrequency < 0.9 * mDCConfig.miSysLoading) mSysLoading = eSL_CRITICAL;    // 0.9 (>1000 ms)
      else if(iFrequency < 3.6 * mDCConfig.miSysLoading) mSysLoading = eSL_MIDDLE;      // 3.6 (>250  ms)
      else if(iFrequency < 18  * mDCConfig.miSysLoading) mSysLoading = eSL_LOWER;       // 18  (>50   ms)
      else mSysLoading = eSL_OK;
    } else mSysLoading = eSL_OK;

    mIPEnterFlood.Del(now); // Removing ip addresses, which already long ago entered

    mDCUserList.AutoResize();
    mDCBotList.AutoResize();
    mHelloList.AutoResize();
    mEnterList.AutoResize();
    mActiveList.AutoResize();
    mChatList.AutoResize();
    mOpList.AutoResize();
    mIpList.AutoResize();
  }

  #ifndef WITHOUT_PLUGINS
    if(mCalls.mOnTimer.CallAll()) return 1;
  #endif

  return 0;
}


/** Function action after joining the client */
int cDCServer::OnNewConn(cConn *conn) {
  if(!conn) {
    if(ErrLog(0)) LogStream() << "Fatal error: OnNewConn null pointer" << endl;
    throw "Fatal error: OnNewConn null pointer";
  }
  cDCConn * dcconn = (cDCConn *) conn;

  /** Checking flood-entry (by ip) */
  if(mIPEnterFlood.Check(dcconn->GetNetIp())) {
    conn->CloseNow();
    return 1;
  }

  if(conn->Log(5)) conn->LogStream() << "[S]Stage OnNewConn" << endl;
  string sMsg;
  cDCProtocol::Append_DC_Lock(sMsg);
  dcconn->Send(cDCProtocol::Append_DC_HubName(sMsg, mDCConfig.msHubName, mDCConfig.msTopic), false, false);

  #ifndef WITHOUT_PLUGINS
    if(mCalls.mOnUserConnected.CallAll(dcconn))
      dcconn->Flush();
    else
  #endif
  {
    sMsg = mDCLang.msFirstMsg;
    StringReplace(sMsg, string("HUB"), sMsg, string(INTERNALNAME " " INTERNALVERSION));
    StringReplace(sMsg, string("total_users"), sMsg, miTotalUserCount);

    cTime Uptime;
    Uptime -= mStartTime;
    stringstream oss;
    oss << Uptime.AsFullPeriod();
    string sUptime(oss.str());
    StringReplace(sUptime, string("weeks"), sUptime, mDCLang.msTimes[0], true);
    StringReplace(sUptime, string("days"),  sUptime, mDCLang.msTimes[1], true);
    StringReplace(sUptime, string("hours"), sUptime, mDCLang.msTimes[2], true);
    StringReplace(sUptime, string("min"),   sUptime, mDCLang.msTimes[3], true);
    StringReplace(sUptime, string("sec"),   sUptime, mDCLang.msTimes[4], true);

    StringReplace(sMsg, string("uptime"), sMsg, sUptime);

    SendToUser(dcconn, sMsg.c_str(), (char*)mDCConfig.msHubBot.c_str());
  }
  dcconn->SetTimeOut(eTO_LOGIN, mDCConfig.miTimeout[eTO_LOGIN], mTime); /** ����-��� �� ���� */
  dcconn->SetTimeOut(eTO_KEY, mDCConfig.miTimeout[eTO_KEY], mTime);
  if(conn->Log(5)) conn->LogStream() << "[E]Stage OnNewConn" << endl;
  conn->Flush();
  return 0;
}

/** Returns pointer to line of the connection, in which will be recorded got data */
string * cDCServer::GetPtrForStr(cConn *conn) {
  return conn->GetPtrForStr();
}

/** Function of the processing enterring data */
void cDCServer::OnNewData(cConn *conn, string *str) {
  string & sStr = *str;
  static string sProtoChar = "$";

  if(conn->Log(4)) conn->LogStream() << "IN: " << sStr << DC_SEPARATOR << endl;
  int iType = conn->mParser->Parse();

  /* Protection from commands, not belonging to DC protocol */
  if(iType == eDC_UNKNOWN && 
    conn->mParser->miLen && 
    sStr.compare(0, 1, sProtoChar) && 
    mDCConfig.mbDisableNoDCCmd) {
    if(conn->Log(1)) conn->LogStream() << "Bad DC cmd: " << sStr.substr(0, 10) << " ..., close" << endl;
    conn->CloseNow();
    return;
  }

  /* Ping from client side */
  if(!conn->mParser->miLen && conn->RecvBufIsEmpty()) { /** Separate ping and void cmd */
    cTime now;
    if(double(now - conn->mTimePing) < mDCConfig.miMinClientPingInt) {
      if(conn->Log(1)) conn->LogStream() << "Ping flood, close" << endl;
      SendToUser((cDCConn *)conn, mDCLang.msFreqClientPing.c_str(), (char*)mDCConfig.msHubBot.c_str());
      conn->CloseNice(9000);
      return;
    }
    conn->mTimePing = now;
    return;
  }

  if(conn->mParser->miLen > mDCConfig.mMaxCmdLen[iType]) { // Checking the length of the command
    if(conn->Log(1)) conn->LogStream() << "Bad CMD(" << iType << ") length: " << conn->mParser->miLen << endl;
    conn->CloseNow();
    return;
  }
  conn->mProtocol->DoCmd(conn->mParser, conn);
}

/** Function checks min interval */
bool cDCServer::MinDelay(cTime &time, double sec) {
	cTime now;
  if(::fabs(double(now - time)) >= sec) {
		time = now;
		return true;
	}
	return false;
}

/** Antiflood function */
bool cDCServer::AntiFlood(unsigned &iCount, cTime &Time, const unsigned &iCountLimit, const double &iTimeLimit) {
  bool bRet = false;
  cTime now;
  if(::fabs(double(now - Time)) < iTimeLimit) {
    if(iCountLimit < ++iCount) bRet = true;
    else return false;
  }
  Time = now;
  iCount = 0;
  return bRet;
}

void cDCServer::AddToOps(cDCUser * User) {
  if(!User->mbInOpList) {
    User->mbInOpList = true;
    if(User->mbInUserList) {
      string sMsg;
      mOpList.AddWithNick(User->msNick, User);
      if(User->mbHide) {
        User->Send(cDCProtocol::Append_DC_OpList(sMsg, User->msNick), false); // newPolitic
      } else {
        mDCUserList.SendToAll(cDCProtocol::Append_DC_OpList(sMsg, User->msNick), true/*mDCConfig.mbDelayedMyINFO*/, false);
        mEnterList.SendToAll(sMsg, true/*mDCConfig.mbDelayedMyINFO*/, false);
      }
    }
  }
}

void cDCServer::DelFromOps(cDCUser * User) {
  if(User->mbInOpList) {
    User->mbInOpList = false;
    if(User->mbInUserList) {
      string sMsg1, sMsg2, sMsg3;
      mOpList.RemoveByNick(User->msNick);
      if(User->mbHide) {
        if(!User->mDCConn) return;
        string s = User->GetMyINFO();
        User->Send(cDCProtocol::Append_DC_Quit(sMsg1, User->msNick), false, false);
        if(User->mDCConn->mFeatures & eSF_NOHELLO)
          User->Send(s, true, false);
        else if(User->mDCConn->mFeatures & eSF_NOGETINFO) {
          User->Send(cDCProtocol::Append_DC_Hello(sMsg2, User->msNick), false, false);
          User->Send(s, true, false);
        } else
          User->Send(cDCProtocol::Append_DC_Hello(sMsg2, User->msNick), false, false);

        if((User->mDCConn->mFeatures & eSF_USERIP2) || User->mbInIpList)
          User->Send(cDCProtocol::Append_DC_UserIP(sMsg3, User->msNick, User->GetIp()), false); // newPolitic
        else {
          s.clear();
          User->Send(s); // newPolitic
        }
      } else {
        mDCUserList.SendToAll(cDCProtocol::Append_DC_Quit(sMsg1, User->msNick), true/*mDCConfig.mbDelayedMyINFO*/, false);
        mEnterList.SendToAll(sMsg1, true/*mDCConfig.mbDelayedMyINFO*/, false);
        mHelloList.SendToAll(cDCProtocol::Append_DC_Hello(sMsg2, User->msNick), true/*mDCConfig.mbDelayedMyINFO*/, false);
        mDCUserList.SendToAll(User->GetMyINFO(), true/*mDCConfig.mbDelayedMyINFO*/);
        mEnterList.SendToAll(User->GetMyINFO(), true/*mDCConfig.mbDelayedMyINFO*/);
        mIpList.SendToAll(cDCProtocol::Append_DC_UserIP(sMsg3, User->msNick, User->GetIp()), true, false);
      }
    }
  }
}

void cDCServer::AddToIpList(cDCUser * User) {
  if(!User->mbInIpList) {
    User->mbInIpList = true;
    if(User->mbInUserList) {
      mIpList.AddWithNick(User->msNick, User);
      User->Send(mDCUserList.GetIpList(), true); // newPolitic
    }
  }
}

void cDCServer::DelFromIpList(cDCUser * User) {
  if(User->mbInIpList) {
    User->mbInIpList = false;
    if(User->mbInUserList) {
      mIpList.RemoveByNick(User->msNick);
    }
  }
}

void cDCServer::AddToHide(cDCUser * User) {
  if(!User->mbHide) {
    User->mbHide = true;
    if(User->mbInUserList) {
      string sMsg;
      User->mbInUserList = false;
      mDCUserList.SendToAll(cDCProtocol::Append_DC_Quit(sMsg, User->msNick), true/*mDCConfig.mbDelayedMyINFO*/, false);
      mEnterList.SendToAll(sMsg, true/*mDCConfig.mbDelayedMyINFO*/, false);
      User->mbInUserList = true;
      mOpList.Remake();
      mDCUserList.Remake();
    }
  }
}

void cDCServer::DelFromHide(cDCUser * User) {
  if(User->mbHide) {
    User->mbHide = false;
    if(User->mbInUserList) {
      string sMsg1, sMsg2, sMsg3;
      if(User->mbInOpList) {
        User->mbInUserList = false;
        mHelloList.SendToAll(cDCProtocol::Append_DC_Hello(sMsg1, User->msNick), true/*mDCConfig.mbDelayedMyINFO*/, false);
        sMsg2 = string(User->GetMyINFO()).append(DC_SEPARATOR);
        mDCUserList.SendToAll(cDCProtocol::Append_DC_OpList(sMsg2, User->msNick), true/*mDCConfig.mbDelayedMyINFO*/, false);
        mEnterList.SendToAll(sMsg2, true/*mDCConfig.mbDelayedMyINFO*/, false);
        mIpList.SendToAll(cDCProtocol::Append_DC_UserIP(sMsg3, User->msNick, User->GetIp()), true, false);
        User->mbInUserList = true;
      } else {
        User->mbInUserList = false;
        mHelloList.SendToAll(cDCProtocol::Append_DC_Hello(sMsg1, User->msNick), true/*mDCConfig.mbDelayedMyINFO*/, false);
        mDCUserList.SendToAll(User->GetMyINFO(), true/*mDCConfig.mbDelayedMyINFO*/);
        mEnterList.SendToAll(User->GetMyINFO(), true/*mDCConfig.mbDelayedMyINFO*/);
        mIpList.SendToAll(cDCProtocol::Append_DC_UserIP(sMsg3, User->msNick, User->GetIp()), true, false);
        User->mbInUserList = true;
      }
      mOpList.Remake();
      mDCUserList.Remake();
    }
  }
}

bool cDCServer::ValidateUser(cDCConn *dcconn, const string &sNick) {
  /** Checking for bad symbols in nick */
  static string ForbidedChars("$| ");
  if(sNick.npos != sNick.find_first_of(ForbidedChars)) {
    if(dcconn->Log(2)) dcconn->LogStream() << "Bad nick chars: '" << sNick << "'" << endl;
    SendToUser(dcconn, mDCLang.msBadChars.c_str(), (char*)mDCConfig.msHubBot.c_str());
    return false;
  }

  /** Checking for this nick used */
  tUserKey Key = mDCUserList.Nick2Key(sNick);
  if(mDCUserList.ContainsKey(Key)) {
    string sMsg;
    if(dcconn->Log(2)) dcconn->LogStream() << "Bad nick (used): '" << sNick << "'" << endl;
    SendToUser(dcconn, StringReplace(mDCLang.msUsedNick, string("nick"), sMsg, sNick).c_str(), (char*)mDCConfig.msHubBot.c_str());
    dcconn->Send(cDCProtocol::Append_DC_ValidateDenide(sMsg.erase(), sNick));
    return false;
  }
  return true;
}

bool cDCServer::CheckNickLength(cDCConn *dcconn, const unsigned iLen) {
  if(dcconn->miProfile == -1 && (iLen > mDCConfig.miMaxNickLen || iLen < mDCConfig.miMinNickLen)) {
    string sMsg;
    if(dcconn->Log(2)) dcconn->LogStream() << "Bad nick len: " << iLen << " [" << mDCConfig.miMinNickLen << ", " << mDCConfig.miMaxNickLen << "]" << endl;
    StringReplace(mDCLang.msBadNickLen, string("min"), sMsg, (int)mDCConfig.miMinNickLen);
    SendToUser(dcconn, StringReplace(sMsg, string("max"), sMsg, (int)mDCConfig.miMaxNickLen).c_str(), (char*)mDCConfig.msHubBot.c_str());
    return false;
  }
  return true;
}


bool cDCServer::BeforeUserEnter(cDCConn *dcconn) {
  unsigned iWantedMask;
  if(mDCConfig.mbDelayedLogin) iWantedMask = eLS_LOGIN_DONE - eLS_NICKLST;
  else iWantedMask = eLS_LOGIN_DONE;
  
  if(iWantedMask == dcconn->GetLSFlag(iWantedMask)) {
    if(dcconn->Log(3)) dcconn->LogStream() << "Begin login" << endl;
    if(dcconn->mbSendNickList) {
      if(!mDCConfig.mbDelayedLogin) DoUserEnter(dcconn);
      else mEnterList.Add(dcconn->mDCUser);

      /** Can happen so that list not to send at a time */
      mDCProtocol.SendNickList(dcconn);

      dcconn->mbSendNickList = false;
      return true;
    }
    if(!dcconn->mDCUser->mbInUserList) DoUserEnter(dcconn);
    return true;
  } else { /** Invalid sequence of the sent commands */
    dcconn->CloseNow();
    return false;
  }
}


/** User entry */
void cDCServer::DoUserEnter(cDCConn *dcconn) {
  /** Check entry stages */
  if(eLS_LOGIN_DONE != dcconn->GetLSFlag(eLS_LOGIN_DONE)) {
    if(dcconn->Log(2)) dcconn->LogStream() << "User Login when not all done"<<endl;
    dcconn->CloseNow();
    return;
  }

  tUserKey Key = mDCUserList.Nick2Key(dcconn->mDCUser->msNick);

  /** User is already considered came */
  if(mEnterList.ContainsKey(Key)) {
    /** We send user contents of cache without clear this cache */
    mEnterList.FlushForUser(dcconn->mDCUser);
    mEnterList.RemoveByKey(Key);
  }

  /** Adding user to the user list */
  if(!AddToUserList((cDCUser *)dcconn->mDCUser)) {
    dcconn->CloseNow();
    return;
  }

  /** Show to all */
  ShowUserToAll(dcconn->mDCUser);
  
  AfterUserEnter(dcconn);
  
  dcconn->ClearTimeOut(eTO_LOGIN);
  ((cDCUser *)dcconn->mDCUser)->mTimeEnter.Get();
}

/** Adding user in the user list */
bool cDCServer::AddToUserList(cDCUser * User) {
  if(!User) {
    if(ErrLog(1)) LogStream() << "Adding a NULL user to userlist" << endl;
    return false;
  }
  if(User->mbInUserList) {
    if(ErrLog(2)) LogStream() << "User is already in the user list" << endl;
    return false;
  }

  tUserKey Key = mDCUserList.Nick2Key(User->msNick);

  if(mDCUserList.Log(4)) mDCUserList.LogStream() << "Before add: " << User->msNick << " Size: " << mDCUserList.Size() << endl;

  if(!mDCUserList.AddWithKey(Key, User)) {
    if(Log(1)) LogStream() << "Adding twice user with same nick " << User->msNick << " (" << mDCUserList.Find(Key)->msNick << ")" << endl;
    User->mbInUserList = false;
    return false;
  }

  if(mDCUserList.Log(4)) mDCUserList.LogStream() << "After add: " << User->msNick << " Size: " << mDCUserList.Size() << endl;
  
  User->mbInUserList = true;
  if(!User->IsPassive()) mActiveList.AddWithKey(Key, User);
  if(User->mbInOpList) mOpList.AddWithKey(Key, User);
  if(User->mbInIpList) mIpList.AddWithKey(Key, User);

  if(User->mDCConn) {
    User->mDCConn->mbIpRecv = true; /** Installing the permit on reception of the messages on ip */
    mChatList.AddWithKey(Key, User);

    if(!(User->mDCConn->mFeatures & eSF_NOHELLO)) mHelloList.AddWithKey(Key, User);
    if(User->mDCConn->Log(3)) User->mDCConn->LogStream() << "Adding at the end of Nicklist" << endl;
    if(User->mDCConn->Log(3)) User->mDCConn->LogStream() << "Becomes in list" << endl;
  }
  return true;
}

/** Removing user from the user list */
bool cDCServer::RemoveFromDCUserList(cDCUser *User) {
  tUserKey Key = mDCUserList.Nick2Key(User->msNick);
  if(mDCUserList.Log(4)) mDCUserList.LogStream() << "Before leave: " << User->msNick << " Size: " << mDCUserList.Size() << endl;
  if(mDCUserList.ContainsKey(Key)) {
    #ifndef WITHOUT_PLUGINS
      if(User->mDCConn) mCalls.mOnUserExit.CallAll(User->mDCConn);
    #endif

    /** We make sure that user with such nick one! */
    cDCUser *other = (cDCUser *)mDCUserList.GetUserBaseByNick(User->msNick);
    if(!User->mDCConn) {/** Removing the bot */
      mDCUserList.RemoveByKey(Key);
    }
    else if(other && other->mDCConn && User->mDCConn && other->mDCConn == User->mDCConn) {
      mDCUserList.RemoveByKey(Key);
      if(mDCUserList.Log(4)) mDCUserList.LogStream() << "After leave: " << User->msNick << " Size: " << mDCUserList.Size() << endl;
    } else {
      /** Such can happen only for users without connection or with different connection */
      if(User->ErrLog(1)) User->LogStream() << "Not found the correct user for nick: " << User->msNick << endl;
      return false;
    }
  }

  /** Removing from lists */
  mOpList.RemoveByKey(Key);
  mIpList.RemoveByKey(Key);
  mHelloList.RemoveByKey(Key);
  mEnterList.RemoveByKey(Key);
  mChatList.RemoveByKey(Key);
  mActiveList.RemoveByKey(Key);
  mDCBotList.RemoveByKey(Key);

  if(User->mbInUserList) {
    User->mbInUserList = false;

    string sMsg;
    cDCProtocol::Append_DC_Quit(sMsg, User->msNick);

    /** Delay in sending MyINFO (and Quit) */
    mDCUserList.SendToAll(sMsg, true/*mDCConfig.mbDelayedMyINFO*/, false);
    mbQuitAction = true;
  }
  return true;
}

/** Show user to all */
bool cDCServer::ShowUserToAll(cDCUser *User) {
  string sMsg1, sMsg2;
  if(User->mbHide && User->mDCConn) {
    if(User->mDCConn->mFeatures & eSF_NOHELLO)
      User->mDCConn->Send(string(User->GetMyINFO()), true, false);
    else if(User->mDCConn->mFeatures & eSF_NOGETINFO) {
      User->mDCConn->Send(cDCProtocol::Append_DC_Hello(sMsg1, User->msNick), false, false);
      User->mDCConn->Send(string(User->GetMyINFO()), true, false);
    } else
      User->mDCConn->Send(cDCProtocol::Append_DC_Hello(sMsg1, User->msNick), false, false);

    if(User->mbInOpList)
      User->mDCConn->Send(cDCProtocol::Append_DC_OpList(sMsg2, User->msNick), false, false);
  } else {

    /** Sending the greeting for all users, not supporting feature NoHello (except enterring users) */
    mHelloList.SendToAll(cDCProtocol::Append_DC_Hello(sMsg1, User->msNick), true/*mDCConfig.mbDelayedMyINFO*/, false);

    /** Show MyINFO string to all users */
    mDCUserList.SendToAll(User->GetMyINFO(), true/*mDCConfig.mbDelayedMyINFO*/); // use cache -> so this can be after user is added

    /** Show MyINFO string of the current user to all enterring users */
    mEnterList.SendToAll(User->GetMyINFO(), true/*mDCConfig.mbDelayedMyINFO*/);

    /** Op entry */
    if(User->mbInOpList) {
      mDCUserList.SendToAll(cDCProtocol::Append_DC_OpList(sMsg2, User->msNick), true/*mDCConfig.mbDelayedMyINFO*/, false);
      mEnterList.SendToAll(sMsg2, true/*mDCConfig.mbDelayedMyINFO*/, false);
    }
  }

  /** Prevention of the double sending MyINFO string */
  if(!mDCConfig.mbDelayedLogin) {
    User->mbInUserList = false;
    mDCUserList.FlushCache();
    mEnterList.FlushCache();
    User->mbInUserList = true;
  }

  if(mDCConfig.mbSendUserIp)
  {
    string sStr;
    User->mbInUserList = false;
    cDCProtocol::Append_DC_UserIP(sStr, User->msNick, User->GetIp());
    if(sStr.length()) mIpList.SendToAll(sStr, true);
    User->mbInUserList = true;

    if(User->mbInIpList)
      User->Send(mDCUserList.GetIpList(), true, false);
    else if(User->mDCConn && (User->mDCConn->mFeatures & eSF_USERIP2)) // UserIP2
      User->Send(sStr, false, false);
  }
  static string s;
  User->Send(s);
  return true;
}

void cDCServer::AfterUserEnter(cDCConn *dcconn) {
  if(dcconn->Log(3)) dcconn->LogStream() << "Entered the hub." << endl;
  #ifndef WITHOUT_PLUGINS
	  mCalls.mOnUserEnter.CallAll(dcconn);
	#endif
}

/** Get user by nick (or NULL)
*/
cDCUser * cDCServer::GetDCUser(const char *sNick) {
  string sN(sNick);
  if(sN.size()) {
    cUserBase * User = mDCUserList.GetUserBaseByNick(sN);
    if(User) return (cDCUser *)User;
    cDCConn * dcconn;
    for(tCLIt it = mConnList.begin(); it != mConnList.end(); ++it) {
      dcconn = (cDCConn *)(*it);
      if(dcconn && dcconn->_miConnType == 1 && dcconn->mDCUser && dcconn->mDCUser->msNick == sN)
        return (cDCUser *)dcconn->mDCUser;
    }
  }
  return NULL;
}

cDCUserBase * cDCServer::GetDCUserBase(const char *sNick) {
  cDCUser * User = GetDCUser(sNick);
  if(User) return (cDCUserBase *)User;
  return NULL;
}

void cDCServer::GetDCConnBase(const char * sIP, vector<cDCConnBase *> & vconn) {
  cDCIPList::iterator it;
  for(it = mIPListConn->begin(cDCConn::Ip2Num(sIP)); it != mIPListConn->end(); ++it) {
    cDCConn * dcconn = (cDCConn *)(*it);
    if(dcconn->_miConnType == 1)
      vconn.push_back(dcconn);
  }
}

/** Send data to user */
bool cDCServer::SendToUser(cDCConnBase *DCConn, const char *sData, char *sNick, char *sFrom) {
  if(!DCConn || !sData) return false;

  // PM
  if(sFrom && sNick) {
    string sTo("<unknown>"), sStr;
    if(DCConn->mDCUserBase)
      sTo = DCConn->mDCUserBase->GetNick();
    DCConn->Send(cDCProtocol::Append_DC_PM(sStr, sTo, sFrom, sNick, sData), false); // newPolitic
    return true;
  }

  // Chat
  if(sNick) {
    string sStr;
    DCConn->Send(cDCProtocol::Append_DC_Chat(sStr, sNick, sData), false); // newPolitic
    return true;
  }

  // Simple Msg
  string sMsg(sData);
  if(DCConn->_miConnType == 1 && sMsg.substr(sMsg.size() - 1, 1) != DC_SEPARATOR) sMsg.append(DC_SEPARATOR);
  DCConn->Send(sMsg, false); // newPolitic
	return true;
}

/** Send data to nick */
bool cDCServer::SendToNick(const char *sTo, const char *sData, char *sNick, char *sFrom) {
  if(!sTo || !sData) return false;
	cDCUser *User = GetDCUser(sTo);
	if(!User || !User->mDCConn) return false;

  // PM
  if(sFrom && sNick) {
    string sStr;
    User->mDCConn->Send(cDCProtocol::Append_DC_PM(sStr, sTo, sFrom, sNick, sData), false); // newPolitic
    return true;
  }

  // Chat
  if(sNick) {
    string sStr;
    User->mDCConn->Send(cDCProtocol::Append_DC_Chat(sStr, sNick, sData), false); // newPolitic
    return true;
  }

  // Simple Msg
  string sMsg(sData);
  if(sMsg.substr(sMsg.size() - 1, 1) != DC_SEPARATOR) sMsg.append(DC_SEPARATOR);
  User->mDCConn->Send(sMsg, false); // newPolitic
	return true;
}

/** Send data to all */
bool cDCServer::SendToAll(const char *sData, char *sNick, char *sFrom) {
  if(!sData) return false;

  // PM
  if(sFrom && sNick) {
    string sStart, sEnd;
    cDCProtocol::Append_DC_PMToAll(sStart, sEnd, sFrom, sNick, sData);
    mDCUserList.SendToWithNick(sStart, sEnd);
    return true;
  }

  // Chat
  if(sNick) {
    string sStr;
    mDCUserList.SendToAll(cDCProtocol::Append_DC_Chat(sStr, sNick, sData), true, false);
    return true;
  }
  
  // Simple Msg
  string sMsg(sData);
  if(sMsg.substr(sMsg.size() - 1, 1) != DC_SEPARATOR) sMsg.append(DC_SEPARATOR);
  mDCUserList.SendToAll(sMsg, true, false);
  return true;
}

/** Send data to profiles */
bool cDCServer::SendToProfiles(unsigned long iProfile, const char *sData, char *sNick, char *sFrom) {
  if(!sData) return false;

  // PM
  if(sFrom && sNick) {
    string sStart, sEnd;
    cDCProtocol::Append_DC_PMToAll(sStart, sEnd, sFrom, sNick, sData);
    mDCUserList.SendToWithNick(sStart, sEnd, iProfile);
    return true;
  }
  
  // Chat
  if(sNick) {
    string sStr;
    mDCUserList.SendToProfiles(iProfile, cDCProtocol::Append_DC_Chat(sStr, sNick, sData), false);
    return true;
  }

  // Simple Msg
  string sMsg(sData);
  if(sMsg.substr(sMsg.size() - 1, 1) != DC_SEPARATOR) sMsg.append(DC_SEPARATOR);
  mDCUserList.SendToProfiles(iProfile, sMsg, false);
  return true;
}

bool cDCServer::SendToIP(const char *sIP, const char *sData, unsigned long iProfile, char *sNick, char *sFrom) {
  if(!sIP || !sData || !cConn::CheckIp(sIP)) return false;

  // PM
  if(sFrom && sNick) {
    string sStart, sEnd;
    cDCProtocol::Append_DC_PMToAll(sStart, sEnd, sFrom, sNick, sData);
    mIPListConn->SendToIPWithNick(sIP, sStart, sEnd, iProfile);
    return true;
  }
  
  // Chat
  if(sNick) {
    string sStr;
    mIPListConn->SendToIP(sIP, cDCProtocol::Append_DC_Chat(sStr, sNick, sData), iProfile); // newPolitic
    return true;
  }

  // Simple Msg
  string sMsg(sData);
  if(sMsg.substr(sMsg.size() - 1, 1) != DC_SEPARATOR) sMsg.append(DC_SEPARATOR);
  mIPListConn->SendToIP(sIP, sMsg, iProfile); // newPolitic
	return true;
}

/** Send data to all except nick list */
bool cDCServer::SendToAllExceptNicks(cDCServer::List_t &NickList, const char *sData, char *sNick, char *sFrom) {
  if(!sData) return false;

  cUserBase * User;
  vector<cUserBase *> ul;
  for(List_t::iterator it = NickList.begin(); it != NickList.end(); ++it) {
    User = mDCUserList.GetUserBaseByNick(*it);
    if(User && User->mbInUserList) {
      User->mbInUserList = false;
      ul.push_back(User);
    }
  }

  if(sFrom && sNick) { // PM
    string sStart, sEnd;
    cDCProtocol::Append_DC_PMToAll(sStart, sEnd, sFrom, sNick, sData);
    mDCUserList.SendToWithNick(sStart, sEnd);
  }else if(sNick){ // Chat
    string sStr;
    mDCUserList.SendToAll(cDCProtocol::Append_DC_Chat(sStr, sNick, sData), false, false);
  }else{ // Simple Msg
    string sMsg(sData);
    if(sMsg.substr(sMsg.size() - 1, 1) != DC_SEPARATOR) sMsg.append(DC_SEPARATOR);
    mDCUserList.SendToAll(sMsg, false, false);
  }

  for(vector<cUserBase *>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it)
    (*ul_it)->mbInUserList = true;

  return true;
}

bool cDCServer::SendToAllExceptIps(cDCServer::List_t & IPList, const char *sData, char *sNick, char *sFrom) {
  if(!sData) return false;

  cDCConn * dcconn;
  vector<cDCConn*> ul;
  bool bBadIP = false;
  for(List_t::iterator it = IPList.begin(); it != IPList.end(); ++it) {
    if(!cDCConn::CheckIp(*it)) bBadIP = true;
    for(cDCIPList::iterator mit = mIPListConn->begin(cDCConn::Ip2Num((*it).c_str())); mit != mIPListConn->end(); ++mit) {
      dcconn = (cDCConn*)(*mit);
      if(dcconn->mDCUser && dcconn->mDCUser->mbInUserList) {
        dcconn->mDCUser->mbInUserList = false;
        ul.push_back(dcconn);
      }
    }
  }

  if(!bBadIP) {
    if(sFrom && sNick) { // PM
      string sStart, sEnd;
      cDCProtocol::Append_DC_PMToAll(sStart, sEnd, sFrom, sNick, sData);
      mDCUserList.SendToWithNick(sStart, sEnd);
    }else if(sNick){ // Chat
      string sStr;
      mDCUserList.SendToAll(cDCProtocol::Append_DC_Chat(sStr, sNick, sData), false, false);
    }else{ // Simple Msg
      string sMsg(sData);
      if(sMsg.substr(sMsg.size() - 1, 1) != DC_SEPARATOR) sMsg.append(DC_SEPARATOR);
      mDCUserList.SendToAll(sMsg, false, false);
    }
  }
  
  for(vector<cDCConn*>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it)
    (*ul_it)->mDCUser->mbInUserList = true;

  return (bBadIP == true) ? false : true;
}

int cDCServer::CheckCmd(const string & sData) {
  mDCParser.ReInit();
  mDCParser.mStr = sData;
  mDCParser.Parse();
  if(mDCParser.SplitChunks()) return -1;
  if(mDCParser.miType > 0 && mDCParser.miType < 3) return 3;
  return mDCParser.miType;
}

const char * cDCServer::GetConfig(const string & sName) {
  cConfig * config = mDCConfig[sName];
  if(!config) return NULL;
  config->ConvertTo(sBuf);
  return sBuf.c_str();
}

const char * cDCServer::GetLang(const string & sName) {
  cConfig * config = mDCLang[sName];
  if(!config) return NULL;
  config->ConvertTo(sBuf);
  return sBuf.c_str();
}

bool cDCServer::SetConfig(const string & sName, const string & sValue) {
  if(sName == "sHubIP" || 
     sName == "iMainPort" || 
     sName == "sSubPorts"
  ) return false;

  if(sName == "sLocale" && 
    !setlocale(LC_ALL, sValue.c_str())
  ) return false;

  cConfig * config = mDCConfig[sName];
  if(!config) return false;

  if(sName == "sHubBot") {
    UnregBot(mDCConfig.msHubBot);
  } else if(sName == "bRegMainBot") {
    if(sValue == "true" || 0 != atoi(sValue.c_str()) ) {
      if(RegBot(mDCConfig.msHubBot, mDCConfig.msMainBotMyINFO, 
        mDCConfig.msMainBotIP, mDCConfig.mbMainBotKey) == -2)
          RegBot(mDCConfig.msHubBot, string("$ $$$0$"), 
            mDCConfig.msMainBotIP, mDCConfig.mbMainBotKey);
    } else
      UnregBot(mDCConfig.msHubBot);
  }

  config->ConvertFrom(sValue);

  if(sName == "sHubBot") {
    if(mDCConfig.mbRegMainBot) /** ����������� ��������� ���� */
      if(RegBot(mDCConfig.msHubBot, mDCConfig.msMainBotMyINFO, 
        mDCConfig.msMainBotIP, mDCConfig.mbMainBotKey) == -2)
          RegBot(mDCConfig.msHubBot, string("$ $$$0$"), 
            mDCConfig.msMainBotIP, mDCConfig.mbMainBotKey);
  } else if(sName == "sHubName" || sName == "sTopic") {
    string sMsg;
    SendToAll(cDCProtocol::Append_DC_HubName(sMsg, mDCConfig.msHubName, mDCConfig.msTopic).c_str());
  }

  mDCConfig.Save();
  return true;
}

bool cDCServer::SetLang(const string & sName, const string & sValue) {
  cConfig * config = mDCLang[sName];
  if(!config) return false;
  config->ConvertFrom(sValue);
  mDCLang.Save();
  return true;
}

int cDCServer::RegBot(const string & sNick, const string & sMyINFO, const string & sIP, bool bKey) {
  if(!sNick.length() || sNick.length() > 64 || sNick.find_first_of(" |$") != sNick.npos) return -1;

  string sINFO(sMyINFO);
  cDCUser *User = new cDCUser(sNick);
  User->mDCServer = this;
  User->mbInOpList = bKey;
  User->SetIp(sIP);
  if(!sINFO.length()) sINFO = "$ $$$0$";
  if(!User->SetMyINFO(string("$MyINFO $ALL ") + sNick + " " + sINFO, sNick)) return -2;

  if(!AddToUserList(User)) {
    delete User;
    return -3;
  }
  mDCBotList.AddWithNick(sNick, User);
  ShowUserToAll(User);
  return 0;
}

int cDCServer::UnregBot(const string & sNick) {
  cDCUser * User = (cDCUser*)mDCUserList.GetUserBaseByNick(sNick);
  if(!User || User->mDCConn) return -1;
  RemoveFromDCUserList(User);
  delete User;
  return 0;
}

#ifdef _WIN32
bool cDCServer::GetSysVersion() {
  OSVERSIONINFOEX osvi;
  bool bOsVersionInfoEx;
  msSysVersion = "";

  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

  if(!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *) &osvi))) {
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if(!GetVersionEx((OSVERSIONINFO *) &osvi))
      return false;
  }

  switch(osvi.dwPlatformId) {

    case VER_PLATFORM_WIN32_NT: // Windows NT

      if(osvi.dwMajorVersion <= 4)
        msSysVersion.append("Microsoft Windows NT ");
      if(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
        msSysVersion.append("Microsoft Windows 2000 ");

      if(bOsVersionInfoEx) {

        // Check workstation type
        if(osvi.wProductType == VER_NT_WORKSTATION) {

          if(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
            msSysVersion.append("Microsoft Windows XP ");
          else if(osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
            msSysVersion.append("Microsoft Windows Vista ");
          else if(osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
            msSysVersion.append("Microsoft Windows 7 ");
          else
            msSysVersion.append("Microsoft Windows (unknown version) ");


          if(osvi.wSuiteMask & VER_SUITE_PERSONAL)
            msSysVersion.append("Home Edition ");
          else
            msSysVersion.append("Professional ");

        } else if(osvi.wProductType == VER_NT_SERVER) { // Check server type

          if(osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
            msSysVersion.append("Microsoft Windows 2003 ");
          else if(osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
            msSysVersion.append("Microsoft Windows Server 2008 ");
          else if(osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
            msSysVersion.append("Microsoft Windows Server 2008 R2 ");
          else
            msSysVersion.append("Microsoft Windows (unknown version) ");

          if(osvi.wSuiteMask & VER_SUITE_DATACENTER)
            msSysVersion.append("DataCenter Server ");
          else if(osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
            if(osvi.dwMajorVersion == 4)
              msSysVersion.append("Advanced Server ");
            else
              msSysVersion.append("Enterprise Server ");
          else if(osvi.wSuiteMask == VER_SUITE_BLADE)
            msSysVersion.append("Web Server ");
          else
            msSysVersion.append("Server ");

        }

      } else {
        HKEY hKey;
        char szProductType[80];
        DWORD dwBufLen = 80;
        LONG lRet = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\ProductOptions", 0, KEY_QUERY_VALUE, &hKey);

        if(lRet != ERROR_SUCCESS)
          return false;

        lRet = RegQueryValueExA( hKey, "ProductType", NULL, NULL, (LPBYTE) szProductType, &dwBufLen);

        if((lRet != ERROR_SUCCESS) || (dwBufLen > 80))
          return false;

        RegCloseKey(hKey);

        if(lstrcmpiA("WINNT", szProductType) == 0)
          msSysVersion.append("Professional ");
        if(lstrcmpiA("LANMANNT", szProductType) == 0)
          msSysVersion.append("Server ");
        if(lstrcmpiA( "SERVERNT", szProductType) == 0)
          msSysVersion.append("Advanced Server ");
      }

      // Version, service pack, number of the build
      if(osvi.dwMajorVersion <= 4) {
        char buf[256];
        sprintf(buf, "version %d.%d %s (Build %d)", 
          osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
        msSysVersion.append(buf);
      } else {
        char buf[256];
        sprintf(buf, "%s (Build %d)", osvi.szCSDVersion, osvi.dwBuildNumber & 0xFFFF);
        msSysVersion.append(buf);
      }

      break;
    
    case VER_PLATFORM_WIN32_WINDOWS: // Windows 95

      if(osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0) {
        msSysVersion.append("Microsoft Windows 95 ");
        if(osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B')
          msSysVersion.append("OSR2 ");
      } 

      if(osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
        msSysVersion.append("Microsoft Windows 98 ");
        if(osvi.szCSDVersion[1] == 'A')
          msSysVersion.append("SE ");
      } 

      if(osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90) {
        msSysVersion.append("Microsoft Windows Millennium Edition ");
      } 
      break;

    case VER_PLATFORM_WIN32s: // Windows
      msSysVersion.append("Microsoft Win32s ");
      break;

    default:
      break;
  }
  return true; 
}
#else
bool cDCServer::GetSysVersion() {
  msSysVersion = "UNIX System";
  return true;
}
#endif

}; // nDCServer
