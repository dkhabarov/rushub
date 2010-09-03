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

#include "cdcprotocol.h"
#include "cdcserver.h" /** for mDCServer */
#include "cdcconn.h" /** for cDCConn */

namespace nDCServer {

namespace nProtocol {

#define BADCMDPARAM(CMD) \
if(dcparser->SplitChunks()) { \
  if(dcconn->Log(1)) dcconn->LogStream() << "Attempt to attack in " CMD " (bad cmd param)" << endl; \
  dcconn->CloseNow(); \
  return -1; \
}

#define BADFLAG(CMD, FLAG) \
if(dcconn->GetLSFlag(FLAG)) { \
  if(dcconn->Log(1)) dcconn->LogStream() << "Attempt to attack in " CMD " (repeated sending)" << endl; \
  dcconn->CloseNow(); \
  return -1; \
}

/* antiflood */
#ifndef WITHOUT_PLUGINS
#define ANTIFLOOD(TYPE, FT) \
  if(mDCServer->AntiFlood(dcconn->mTimes1.mi##TYPE, dcconn->mTimes1.m##TYPE, \
  mDCServer->mDCConfig.miFloodCount##TYPE, mDCServer->mDCConfig.miFloodTime##TYPE) \
) { \
  if(!mDCServer->mCalls.mOnFlood.CallAll(dcconn, FT, 1)) { \
    mDCServer->SendToUser(dcconn, (mDCServer->mDCLang.msFlood##TYPE).c_str(), (char*)mDCServer->mDCConfig.msHubBot.c_str()); \
    dcconn->CloseNice(9000, eCR_FLOOD); return -1; \
  } \
} \
if(mDCServer->AntiFlood(dcconn->mTimes2.mi##TYPE, dcconn->mTimes2.m##TYPE, \
  mDCServer->mDCConfig.miFloodCount##TYPE##2, mDCServer->mDCConfig.miFloodTime##TYPE##2) \
) { \
  if(!mDCServer->mCalls.mOnFlood.CallAll(dcconn, FT, 2)) { \
    mDCServer->SendToUser(dcconn, (mDCServer->mDCLang.msFlood##TYPE).c_str(), (char*)mDCServer->mDCConfig.msHubBot.c_str()); \
    dcconn->CloseNice(9000, eCR_FLOOD); return -1; \
  } \
}
#else
#define ANTIFLOOD(TYPE, FT) \
  if(mDCServer->AntiFlood(dcconn->mTimes1.mi##TYPE, dcconn->mTimes1.m##TYPE, \
  mDCServer->mDCConfig.miFloodCount##TYPE, mDCServer->mDCConfig.miFloodTime##TYPE) \
) { \
  mDCServer->SendToUser(dcconn, (mDCServer->mDCLang.msFlood##TYPE).c_str(), (char*)mDCServer->mDCConfig.msHubBot.c_str()); \
  dcconn->CloseNice(9000, eCR_FLOOD); return -1; \
} \
if(mDCServer->AntiFlood(dcconn->mTimes2.mi##TYPE, dcconn->mTimes2.m##TYPE, \
  mDCServer->mDCConfig.miFloodCount##TYPE##2, mDCServer->mDCConfig.miFloodTime##TYPE##2) \
) { \
  mDCServer->SendToUser(dcconn, (mDCServer->mDCLang.msFlood##TYPE).c_str(), (char*)mDCServer->mDCConfig.msHubBot.c_str()); \
  dcconn->CloseNice(9000, eCR_FLOOD); return -1; \
}
#endif

cDCProtocol::cDCProtocol() {
  SetClassName("cDCProtocol");
}

void cDCProtocol::SetServer(cDCServer * server) {
  mDCServer = server;
}

int cDCProtocol::DoCmd(cParser *parser, cConn *conn) {
  cDCConn *dcconn = (cDCConn *)conn;
  cDCParser *dcparser = (cDCParser *)parser;
  
  if(strlen(parser->mStr.data()) < parser->mStr.size()) {
    // Sending null chars, probably attempt of an attack.
    conn->CloseNow();
    return -1;
  }
  
  #ifndef WITHOUT_PLUGINS
    if(parser->miType != eUNPARSED) {
      if(mDCServer->mCalls.mOnAny.CallAll(dcconn, dcparser)) return 1;
    }
  #endif
  
  if(conn->Log(5)) conn->LogStream() << "[S]Stage " << parser->miType << endl;
  switch(parser->miType) {
    case eDC_MSEARCH:
    case eDC_MSEARCH_PAS:
    case eDC_SEARCH_PAS:
    case eDC_SEARCH:        DC_Search(dcparser, dcconn); break;
    case eDC_SR:            DC_SR(dcparser, dcconn); break;
    case eDC_MYNIFO:        DC_MyINFO(dcparser, dcconn); break;
    case eDC_SUPPORTS:      DC_Supports(dcparser, dcconn); break;
    case eDC_KEY:           DC_Key(dcparser, dcconn); break;
    case eDC_VALIDATENICK:  DC_ValidateNick(dcparser, dcconn); break;
    case eDC_VERSION:       DC_Version(dcparser, dcconn); break;
    case eDC_GETNICKLIST:   DC_GetNickList(dcparser, dcconn); break;
    case eDC_CHAT:          DC_Chat(dcparser, dcconn); break;
    case eDC_TO:            DC_To(dcparser, dcconn); break;
    case eDC_MYPASS:        DC_MyPass(dcparser, dcconn); break;
    case eDC_CONNECTTOME:   DC_ConnectToMe(dcparser, dcconn); break;
    case eDC_RCONNECTTOME:  DC_RevConnectToMe(dcparser, dcconn); break;
    case eDC_MCONNECTTOME:  DC_MultiConnectToMe(dcparser, dcconn); break;
    case eDC_KICK:          DC_Kick(dcparser, dcconn); break;
    case eDC_OPFORCEMOVE:   DC_OpForceMove(dcparser, dcconn); break;
    case eDC_GETINFO:       DC_GetINFO(dcparser, dcconn); break;
    case eDC_UNKNOWN:
      #ifndef WITHOUT_PLUGINS
      if(!mDCServer->mCalls.mOnUnknown.CallAll(dcconn, dcparser)) {
        dcconn->CloseNice(9000, eCR_UNKNOWN_CMD);
      }
      #endif
      ANTIFLOOD(Unknown, eFT_UNKNOWN);
      break;
    case eDC_QUIT:          dcconn->CloseNice(9000, eCR_QUIT); break;
    case eUNPARSED:         parser->Parse(); return DoCmd(parser, dcconn);
    default: if(Log(2)) LogStream() << "Incoming untreated event" << endl; break;
  }
  if(conn->Log(5)) conn->LogStream() << "[E]Stage " << parser->miType << endl;
  return 0;
}

int cDCProtocol::DC_Supports(cDCParser *dcparser, cDCConn *dcconn) {
  ANTIFLOOD(NickList, eFT_NICKLIST);

  static string sFeature;
  istringstream is(dcparser->mStr);
  is >> sFeature;
  dcconn->mFeatures = 0;
  while(1)
  {
    sFeature = this->mDCServer->msEmpty;
    is >> sFeature;
    if(!sFeature.size()) break;
    if(sFeature      == "UserCommand") dcconn->mFeatures |= eSF_USERCOMMAND;
    else if(sFeature == "NoGetINFO")   dcconn->mFeatures |= eSF_NOGETINFO;
    else if(sFeature == "NoHello")     dcconn->mFeatures |= eSF_NOHELLO;
    else if(sFeature == "UserIP2")     dcconn->mFeatures |= eSF_USERIP2;
    else if(sFeature == "TTHSearch")   dcconn->mFeatures |= eSF_TTHSEARCH;
    else if(sFeature == "QuickList")   dcconn->mFeatures |= eSF_QUICKLIST;
  }
  dcconn->msSupports.assign(dcparser->mStr, 10, dcparser->mStr.size() - 10);

  #ifndef WITHOUT_PLUGINS
    if(mDCServer->mCalls.mOnSupports.CallAll(dcconn, dcparser))
      return -3;
  #endif

  static string sMsg("$Supports UserCommand NoGetINFO NoHello UserIP2"DC_SEPARATOR);
  dcconn->Send(sMsg, false, false); // newPolitic & not

  return 0;
}

int cDCProtocol::DC_Key(cDCParser *dcparser, cDCConn *dcconn) {
  BADFLAG("Key", eLS_KEY);
  BADCMDPARAM("Key");

  #ifndef WITHOUT_PLUGINS
    if(mDCServer->mCalls.mOnKey.CallAll(dcconn, dcparser)) {
      //string sLock, sKey;
      //sLock = Append_DC_Lock(sLock).substr(1, sLock.size() - 1);
      //Lock2Key(sLock, sKey);
    }
  #endif

  dcconn->SetLSFlag(eLS_KEY); /** User has sent key */
  dcconn->ClearTimeOut(eTO_KEY);
  dcconn->SetTimeOut(eTO_VALNICK, mDCServer->mDCConfig.miTimeout[eTO_VALNICK], mDCServer->mTime);
  dcconn->mTimes.mKey.Get();
  return 0;
}

int cDCProtocol::DC_ValidateNick(cDCParser *dcparser, cDCConn *dcconn) {
  BADFLAG("ValidateNick", eLS_VALNICK);
  BADCMDPARAM("ValidateNick");

  string &sNick = dcparser->ChunkString(eCH_1_PARAM);
  unsigned iNickLen = sNick.length();

  /** Additional checking the nick length */
  if(iNickLen > 0xFF) {
    if(dcconn->Log(1)) dcconn->LogStream() << "Attempt to attack by long nick" << endl;
    dcconn->CloseNow();
    return -1;
  }

  if(dcconn->Log(3)) dcconn->LogStream() << "User " << sNick << " to validate nick" << endl;

  try {
    cDCUser *NewUser = new cDCUser(sNick);
    if(!dcconn->SetUser(NewUser)) {
      dcconn->CloseNow();
      return -2;
    }
  } catch(...) {
    if(mDCServer->ErrLog(1)) mDCServer->LogStream() << "Unhandled exception in cDCProtocol::DC_ValidateNick" << endl;
    if(dcconn->ErrLog(1)) dcconn->LogStream() << "Error in SetUser closing" << endl;
    dcconn->CloseNice(9000);
    return -2;
  }

  /** Checking validate user */
  if(!mDCServer->ValidateUser(dcconn, sNick)) {
    dcconn->CloseNice(9000, eCR_INVALID_USER);
    return -2;
  }

  dcconn->SetLSFlag(eLS_ALOWED);
  ++mDCServer->miTotalUserCount;

  dcconn->SetLSFlag(eLS_VALNICK | eLS_NICKLST); /** We Install NICKLST because user can not call user list */
  dcconn->ClearTimeOut(eTO_VALNICK);
  
  string sMsg;

  
  #ifndef WITHOUT_PLUGINS
    if(mDCServer->mCalls.mOnValidateNick.CallAll(dcconn, dcparser)) {
      dcconn->Send(Append_DC_GetPass(sMsg), false); // newPolitic /** We are sending the query for reception of the password */
      dcconn->SetTimeOut(eTO_PASS, mDCServer->mDCConfig.miTimeout[eTO_PASS], mDCServer->mTime);
      return -3;
    }
  #endif

  if(!mDCServer->CheckNickLength(dcconn, iNickLen))
    dcconn->CloseNice(9000, eCR_INVALID_USER);
  dcconn->SetTimeOut(eTO_MYINFO, mDCServer->mDCConfig.miTimeout[eTO_MYINFO], mDCServer->mTime);
  dcconn->SetLSFlag(eLS_PASSWD); /** Does not need password */
  
  dcconn->Send(Append_DC_Hello(sMsg, dcconn->mDCUser->msNick), false); // newPolitic /** Protection from change the command */
  return 0;
}

int cDCProtocol::DC_MyPass(cDCParser *dcparser, cDCConn *dcconn) {
  if(!dcconn->mDCUser) { /* Check of existence of the user for current connection */
    if(dcconn->Log(2)) dcconn->LogStream() << "Mypass before validatenick" << endl;
    mDCServer->SendToUser(dcconn, mDCServer->mDCLang.msBadLoginSequence.c_str(), (char*)mDCServer->mDCConfig.msHubBot.c_str());
    dcconn->CloseNice(9000, eCR_LOGIN_ERR);
    return -1;
  }
  BADFLAG("MyPass", eLS_PASSWD);
  BADCMDPARAM("MyPass");

  // Checking the accepted password, otherwise send $BadPass|
  // or $Hello Nick|$LogedIn Nick|
  bool bOp = false;
  #ifndef WITHOUT_PLUGINS
    bOp = (mDCServer->mCalls.mOnMyPass.CallAll(dcconn, dcparser));
  #endif
  
  if(!mDCServer->CheckNickLength(dcconn, dcconn->mDCUser->msNick.length()))
    dcconn->CloseNice(9000, eCR_INVALID_USER);
  string sMsg;
  dcconn->SetLSFlag(eLS_PASSWD); /** Password is accepted */
  Append_DC_Hello(sMsg, dcconn->mDCUser->msNick);
  if(bOp) { /** If entered operator, that sends command LoggedIn ($LogedIn !) */
    sMsg.append("$LogedIn ");
    sMsg.append(dcconn->mDCUser->msNick);
    sMsg.append(DC_SEPARATOR);
  }
  dcconn->Send(sMsg, false); // newPolitic
  dcconn->ClearTimeOut(eTO_PASS);
  dcconn->SetTimeOut(eTO_MYINFO, mDCServer->mDCConfig.miTimeout[eTO_MYINFO], mDCServer->mTime);
  return 0;
}

int cDCProtocol::DC_Version(cDCParser *dcparser, cDCConn *dcconn) {
  BADFLAG("Version", eLS_VERSION);
  BADCMDPARAM("Version");

  string & sVersion = dcparser->ChunkString(eCH_1_PARAM);
  if(dcconn->Log(3)) dcconn->LogStream() << "Version:" << sVersion << endl;

  #ifndef WITHOUT_PLUGINS
    if(mDCServer->mCalls.mOnVersion.CallAll(dcconn, dcparser) && sVersion != "1,0091")
      dcconn->CloseNice(9000, eCR_SYNTAX); /** Checking of the version */
  #endif

  dcconn->SetLSFlag(eLS_VERSION); /** Version was checked */
  dcconn->msVersion = sVersion;
  return 0;
}

int cDCProtocol::DC_GetNickList(cDCParser *dcparser, cDCConn *dcconn) {
  ANTIFLOOD(NickList, eFT_NICKLIST);

  #ifndef WITHOUT_PLUGINS
    if(mDCServer->mCalls.mOnGetNickList.CallAll(dcconn, dcparser))
      return 3;
  #endif

  if(!dcconn->GetLSFlag(eLS_MYINFO) && mDCServer->mDCConfig.mbNicklistOnLogin) {
    if(mDCServer->mDCConfig.mbDelayedLogin) {
      int LSFlag = dcconn->GetLSFlag(eLS_LOGIN_DONE);
      if(LSFlag & eLS_NICKLST) LSFlag -= eLS_NICKLST;
      dcconn->ReSetLSFlag(LSFlag);
    }
    dcconn->mbSendNickList = true;
    return 0;
  }
  return SendNickList(dcconn);
}

int cDCProtocol::DC_MyINFO(cDCParser *dcparser, cDCConn *dcconn) {
  /** Check syntax of the command */
  if(dcparser->SplitChunks()) {
    if(dcconn->Log(2)) dcconn->LogStream() << "MyINFO syntax error, closing" << endl;
    mDCServer->SendToUser(dcconn, mDCServer->mDCLang.msMyinfoError.c_str(), (char*)mDCServer->mDCConfig.msHubBot.c_str());
    dcconn->CloseNice(9000, eCR_SYNTAX);
    return -1;
  }

  ANTIFLOOD(MyINFO, eFT_MYNIFO);

  const string &sNick = dcparser->ChunkString(eCH_MI_NICK);

  /** Check existence user, otherwise check support QuickList */
  if(!dcconn->mDCUser) {
    if(0) { // if(QuickList)
      dcconn->mDCUser->msNick = sNick;
    } else {
      if(dcconn->Log(2)) dcconn->LogStream() << "Myinfo without user: " << dcparser->mStr << endl;
      mDCServer->SendToUser(dcconn, mDCServer->mDCLang.msBadLoginSequence.c_str(), (char*)mDCServer->mDCConfig.msHubBot.c_str());
      dcconn->CloseNice(9000, eCR_LOGIN_ERR);
      return -2;
    }
  } else if(sNick != dcconn->mDCUser->msNick) { /** �������� ���� */
    if(dcconn->Log(2)) dcconn->LogStream() << "Bad nick in MyINFO, closing" << endl;
    mDCServer->SendToUser(dcconn, mDCServer->mDCLang.msBadMyinfoNick.c_str(), (char*)mDCServer->mDCConfig.msHubBot.c_str());
    dcconn->CloseNice(9000, eCR_SYNTAX);
    return -1;
  }

  string sOldMyINFO = dcconn->mDCUser->GetMyINFO();
  dcconn->mDCUser->SetMyINFO(dcparser);
  
  int iMode = 0;
  #ifndef WITHOUT_PLUGINS
    iMode = mDCServer->mCalls.mOnMyINFO.CallAll(dcconn, dcparser);
  #endif

  if(iMode != 1 && dcconn->mDCUser->mbInUserList) {
    if(sOldMyINFO != dcconn->mDCUser->GetMyINFO()) {
      if(dcconn->mDCUser->mbHide)
        dcconn->Send(dcparser->mStr, true); // newPolitic // send to self only
      else
        SendMode(dcconn, dcparser->mStr, iMode, mDCServer->mDCUserList);
        //mDCServer->mDCUserList.SendToAll(dcparser->mStr, true/*mDCServer->mDCConfig.mbDelayedMyINFO*/); // send to all
    }
  } else if(!dcconn->mDCUser->mbInUserList) {
    dcconn->SetLSFlag(eLS_MYINFO);
    if(!mDCServer->BeforeUserEnter(dcconn)) return -1;
    dcconn->ClearTimeOut(eTO_MYINFO);
  }
  return 0;
}

int cDCProtocol::DC_Chat(cDCParser *dcparser, cDCConn *dcconn) {
  if(dcparser->SplitChunks()) {
    if(dcconn->Log(2)) dcconn->LogStream() << "Chat syntax error, closing" << endl;
    dcconn->CloseNow(eCR_SYNTAX);
    return -1;
  }

  ANTIFLOOD(Chat, eFT_CHAT);

  if(!dcconn->mDCUser) return -2;
  if(!dcconn->mDCUser->mbInUserList) return -3;

  static string sMsg;

  /** Check chat nick */
  if((dcparser->ChunkString(eCH_CH_NICK) != dcconn->mDCUser->msNick) ) {
    if(dcconn->Log(2)) dcconn->LogStream() << "Bad nick in chat, closing" << endl;
    sMsg = mDCServer->mDCLang.msBadChatNick;
    StringReplace(sMsg, string("nick"), sMsg, dcparser->ChunkString(eCH_CH_NICK));
    StringReplace(sMsg, string("real_nick"), sMsg, dcconn->mDCUser->msNick);
    mDCServer->SendToUser(dcconn, sMsg.c_str(), (char*)mDCServer->mDCConfig.msHubBot.c_str());
    dcconn->CloseNice(9000, eCR_CHAT_NICK);
    return -2;
  }
  int iMode = 0;
  #ifndef WITHOUT_PLUGINS
    iMode = mDCServer->mCalls.mOnChat.CallAll(dcconn, dcparser);
  #endif

  //tcHash<unsigned long> Hash;
  //unsigned long Key = Hash(dcparser->mStr);
  //cout << Key << endl;

  /** Sending message */
  SendMode(dcconn, dcparser->mStr, iMode, mDCServer->mChatList);
  return 0;
}

int cDCProtocol::DC_To(cDCParser *dcparser, cDCConn *dcconn) {
  if(dcparser->SplitChunks()) {
    if(dcconn->Log(2)) dcconn->LogStream() << "PM syntax error, closing" << endl;
    dcconn->CloseNow(eCR_SYNTAX);
    return -1;
  }

  ANTIFLOOD(To, eFT_TO);
  
  if(!dcconn->mDCUser) return -2;
  if(!dcconn->mDCUser->mbInUserList) return -3;
  string &sNick = dcparser->ChunkString(eCH_PM_TO);

  /** Checking the coincidence nicks in command */
  if(dcparser->ChunkString(eCH_PM_FROM) != dcconn->mDCUser->msNick || dcparser->ChunkString(eCH_PM_NICK) != dcconn->mDCUser->msNick) {
    if(dcconn->Log(2)) dcconn->LogStream() << "Bad nick in PM, closing" <<endl;
    dcconn->CloseNow();
    return -1;
  }

  #ifndef WITHOUT_PLUGINS
    if(mDCServer->mCalls.mOnTo.CallAll(dcconn, dcparser)) return 0;
  #endif

  /** Search user */
  cDCUser *User = (cDCUser *)mDCServer->mDCUserList.GetUserBaseByNick(sNick);
  if(!User) return -2;

  User->Send(dcparser->mStr, true); // newPolitic
  return 0;
}

/**
  eDC_SEARCH
  eDC_SEARCH_PAS
  eDC_MSEARCH
  eDC_MSEARCH_PAS
*/
int cDCProtocol::DC_Search(cDCParser *dcparser, cDCConn *dcconn) {
  if(dcparser->SplitChunks()) {
    if(dcconn->Log(2)) dcconn->LogStream() << "Search syntax error, closing" << endl;
    dcconn->CloseNow(eCR_SYNTAX);
    return -1;
  }

  ANTIFLOOD(Search, eFT_SEARCH);

  if(!dcconn->mDCUser || !dcconn->mDCUser->mbInUserList) return -2;

  // TODO: Check overloading of the system

  int iMode = 0;
  #ifndef WITHOUT_PLUGINS
    iMode = mDCServer->mCalls.mOnSearch.CallAll(dcconn, dcparser);
  #endif

  /** Sending cmd */
  string sMsg;
  switch(dcparser->miType) {
    case eDC_SEARCH:
      if(mDCServer->mDCConfig.mbCheckSearchIp && dcconn->msIp != dcparser->ChunkString(eCH_AS_IP)) {
        sMsg = mDCServer->mDCLang.msBadSearchIp;
        if(dcconn->Log(2)) dcconn->LogStream() << "Bad ip in active search, closing" << endl;
        StringReplace(sMsg, string("ip"), sMsg, dcparser->ChunkString(eCH_AS_IP));
        StringReplace(sMsg, string("real_ip"), sMsg, dcconn->msIp);
        mDCServer->SendToUser(dcconn, sMsg.c_str(), (char*)mDCServer->mDCConfig.msHubBot.c_str());
        dcconn->CloseNice(9000, eCR_SYNTAX);
        return -1;
      }
      SendMode(dcconn, dcparser->mStr, iMode, mDCServer->mDCUserList);
      break;
    case eDC_SEARCH_PAS:
      dcconn->miSRCounter = 0; /** Zeroizing result counter of the passive search */
      SendMode(dcconn, dcparser->mStr, iMode, mDCServer->mActiveList);
      break;
    case eDC_MSEARCH:
      if(mDCServer->mDCConfig.mbCheckSearchIp && (dcconn->msIp != dcparser->ChunkString(eCH_AS_IP))) {
        sMsg = mDCServer->mDCLang.msBadSearchIp;
        if(dcconn->Log(2)) dcconn->LogStream() << "Bad ip in active search, closing" << endl;
        StringReplace(sMsg, string("ip"), sMsg, dcparser->ChunkString(eCH_AS_IP));
        StringReplace(sMsg, string("real_ip"), sMsg, dcconn->msIp);
        mDCServer->SendToUser(dcconn, sMsg.c_str(), (char*)mDCServer->mDCConfig.msHubBot.c_str());
        dcconn->CloseNice(9000, eCR_SYNTAX);
        return -1;
      }
      sMsg = "$Search ";
      sMsg += dcparser->ChunkString(eCH_AS_ADDR);
      sMsg += ' ';
      sMsg += dcparser->ChunkString(eCH_AS_QUERY);
      SendMode(dcconn, sMsg, iMode, mDCServer->mDCUserList);
      break;
    case eDC_MSEARCH_PAS:
      dcconn->miSRCounter = 0; /** Zeroizing result counter of the passive search */
      SendMode(dcconn, dcparser->mStr, iMode, mDCServer->mActiveList);
      break;
    default: return -4;
  }
  return 0;
}

int cDCProtocol::DC_SR(cDCParser *dcparser, cDCConn *dcconn) {
  if(dcparser->SplitChunks()) {
    if(dcconn->Log(2)) dcconn->LogStream() << "SR syntax error, closing" << endl;
    dcconn->CloseNow(eCR_SYNTAX);
    return -1;
  }

  ANTIFLOOD(SR, eFT_SR);

  if(!dcconn->mDCUser || !dcconn->mDCUser->mbInUserList) return -2;

  /** Check same nick in cmd (PROTOCOL NMDC) */
  if(mDCServer->mDCConfig.mbCheckSRNick && (dcconn->mDCUser->msNick != dcparser->ChunkString(eCH_SR_FROM))) {
    string sMsg = mDCServer->mDCLang.msBadSRNick;
    if(dcconn->Log(2)) dcconn->LogStream() << "Bad nick in search response, closing" << endl;
    StringReplace(sMsg, "nick", sMsg, dcparser->ChunkString(eCH_SR_FROM));
    StringReplace(sMsg, "real_nick", sMsg, dcconn->mDCUser->msNick);
    mDCServer->SendToUser(dcconn, sMsg.c_str(), (char*)mDCServer->mDCConfig.msHubBot.c_str());
    dcconn->CloseNice(9000, eCR_SYNTAX);
    return -1;
  }

  cDCUser *User = (cDCUser *)mDCServer->mDCUserList.GetUserBaseByNick(dcparser->ChunkString(eCH_SR_TO));

  /** Is user exist? */
  if(!User || !User->mDCConn) return -2;

  /** != 0 - error */
  #ifndef WITHOUT_PLUGINS
    if(mDCServer->mCalls.mOnSR.CallAll(dcconn, dcparser))
      return -3;
  #endif

  /** Sending cmd */
  if(!mDCServer->mDCConfig.miMaxPassiveRes || (User->mDCConn->miSRCounter++ < unsigned(mDCServer->mDCConfig.miMaxPassiveRes))) {
    string sStr(dcparser->mStr, 0, dcparser->mChunks[eCH_SR_TO].first - 1); /** Remove nick on the end of cmd */
    User->mDCConn->Send(sStr, true, false); // newPolitic & not
  }
  return 0;
}


int cDCProtocol::DC_ConnectToMe(cDCParser *dcparser, cDCConn *dcconn) {
  if(dcparser->SplitChunks()) {
    if(dcconn->Log(2)) dcconn->LogStream() << "CTM syntax error, closing" << endl;
    dcconn->CloseNow(eCR_SYNTAX);
    return -1;
  }

  ANTIFLOOD(CTM, eFT_CTM);

  if(!dcconn->mDCUser || !dcconn->mDCUser->mbInUserList) return -1;
  
  if(mDCServer->mDCConfig.mbCheckCTMIp && dcconn->msIp != dcparser->ChunkString(eCH_CM_IP)) {
    string sMsg = mDCServer->mDCLang.msBadCTMIp;
    StringReplace(sMsg, string("ip"), sMsg, dcparser->ChunkString(eCH_CM_IP));
    StringReplace(sMsg, string("real_ip"), sMsg, dcconn->msIp);
    mDCServer->SendToUser(dcconn, sMsg.c_str(), (char*)mDCServer->mDCConfig.msHubBot.c_str());
    dcconn->CloseNice(9000, eCR_SYNTAX);
    return -1;
  }

  cDCUser *User = (cDCUser *)mDCServer->mDCUserList.GetUserBaseByNick(dcparser->ChunkString(eCH_CM_NICK));
  if(!User) return -1;

  #ifndef WITHOUT_PLUGINS
    if(mDCServer->mCalls.mOnConnectToMe.CallAll(dcconn, dcparser)) return -2;
  #endif

  User->Send(dcparser->mStr, true); // newPolitic
  return 0;
}

int cDCProtocol::DC_RevConnectToMe(cDCParser *dcparser, cDCConn *dcconn) {
  if(dcparser->SplitChunks()) {
    if(dcconn->Log(2)) dcconn->LogStream() << "RCTM syntax error, closing" << endl;
    dcconn->CloseNow(eCR_SYNTAX);
    return -1;
  }

  ANTIFLOOD(RCTM, eFT_RCTM);

  if(!dcconn->mDCUser || !dcconn->mDCUser->mbInUserList) return -1;

  /** Checking the nick */
  if(mDCServer->mDCConfig.mbCheckRctmNick && (dcparser->ChunkString(eCH_RC_NICK) != dcconn->mDCUser->msNick)) {
    static string sMsg;
    sMsg = mDCServer->mDCLang.msBadRevConNick;
    StringReplace(sMsg, string("nick"), sMsg, dcparser->ChunkString(eCH_RC_NICK));
    StringReplace(sMsg, string("real_nick"), sMsg, dcconn->mDCUser->msNick);
    mDCServer->SendToUser(dcconn, sMsg.c_str(), (char*)mDCServer->mDCConfig.msHubBot.c_str());
    dcconn->CloseNice(9000, eCR_SYNTAX);
    return -1;
  }

  /** Searching the user */
  cDCUser *other = (cDCUser *)mDCServer->mDCUserList.GetUserBaseByNick(dcparser->ChunkString(eCH_RC_OTHER));
  if(!other) return -2;

  #ifndef WITHOUT_PLUGINS
    if(mDCServer->mCalls.mOnRevConnectToMe.CallAll(dcconn, dcparser)) return -2;
  #endif

  other->Send(dcparser->mStr, true); // newPolitic
  return 0;
}

int cDCProtocol::DC_MultiConnectToMe(cDCParser *, cDCConn *dcconn) {
  ANTIFLOOD(CTM, eFT_CTM);
  return 0;
}

int cDCProtocol::DC_Kick(cDCParser *dcparser, cDCConn *dcconn) {
  if(dcparser->SplitChunks()) {
    if(dcconn->Log(2)) dcconn->LogStream() << "Kick syntax error, closing" << endl;
    dcconn->CloseNow(eCR_SYNTAX);
    return -1;
  }

  #ifndef WITHOUT_PLUGINS
    if(mDCServer->mCalls.mOnKick.CallAll(dcconn, dcparser)) return -1;
  #endif
  return 0;
}

int cDCProtocol::DC_OpForceMove(cDCParser *dcparser, cDCConn *dcconn) {
  if(dcparser->SplitChunks()) {
    if(dcconn->Log(2)) dcconn->LogStream() << "OpForceMove syntax error, closing" << endl;
    dcconn->CloseNow(eCR_SYNTAX);
    return -1;
  }

  #ifndef WITHOUT_PLUGINS
    if(mDCServer->mCalls.mOnOpForceMove.CallAll(dcconn, dcparser)) return -1;
  #endif
  return 0;
}

int cDCProtocol::DC_GetINFO(cDCParser *dcparser, cDCConn *dcconn) {
  if(dcparser->SplitChunks()) {
    if(dcconn->Log(2)) dcconn->LogStream() << "GetINFO syntax error, closing" << endl;
    dcconn->CloseNow(eCR_SYNTAX);
    return -1;
  }
  if(!dcconn->mDCUser || !dcconn->mDCUser->mbInUserList) return -1;

  cDCUser *User = (cDCUser *)mDCServer->mDCUserList.GetUserBaseByNick(dcparser->ChunkString(eCH_GI_OTHER));
  if(!User) return -2;

  if(dcconn->mDCUser->mTimeEnter < User->mTimeEnter && cTime() < (User->mTimeEnter + 60000))
    return 0;

  #ifndef WITHOUT_PLUGINS
    if(mDCServer->mCalls.mOnGetINFO.CallAll(dcconn, dcparser)) return -2;
  #endif

  //if(!(dcconn->mFeatures & eSF_NOGETINFO)){
  if(!User->mbHide) {
    dcconn->Send(string(User->GetMyINFO()), true, false); // newPolitic & not
  }
  return 0;
}















string & cDCProtocol::Append_DC_Lock(string &sStr) {
  static string s("$Lock EXTENDEDPROTOCOL_" INTERNALNAME "_by_setuper_" INTERNALVERSION " Pk=" INTERNALNAME);
  sStr.append(s);
  sStr.append(DC_SEPARATOR);
  return sStr;
}

string & cDCProtocol::Append_DC_Hello(string &sStr, const string &sNick) {
  static string s("$Hello ");
  sStr.append(s);
  sStr.append(sNick);
  sStr.append(DC_SEPARATOR);
  return sStr;
}

string & cDCProtocol::Append_DC_HubIsFull(string &sStr) {
  static string s("$HubIsFull"DC_SEPARATOR);
  sStr.append(s);
  return sStr;
}

string & cDCProtocol::Append_DC_GetPass(string &sStr) {
  static string s("$GetPass"DC_SEPARATOR);
  sStr.append(s);
  return sStr;
}

string & cDCProtocol::Append_DC_ValidateDenide(string &sStr, const string &sNick) {
  static string s("$ValidateDenide ");
  sStr.append(s);
  sStr.append(sNick);
  sStr.append(DC_SEPARATOR);
  return sStr;
}

string & cDCProtocol::Append_DC_HubName(string &sStr, const string &sHubName, const string &sTopic) {
  static string s1("$HubName ");
  static string s2(" - ");
  sStr.append(s1);
  sStr.append(sHubName);
  if(sTopic.length()) {
    sStr.append(s2);
    sStr.append(sTopic);
  }
  sStr.append(DC_SEPARATOR);
  return sStr;
}

string & cDCProtocol::Append_DC_HubTopic(string &sStr, const string &sHubTopic) {
  static string s("$HubTopic ");
  sStr.append(s);
  sStr.append(sHubTopic);
  sStr.append(DC_SEPARATOR);
  return sStr;
}

string & cDCProtocol::Append_DC_Chat(string &sStr, const string &sNick, const string &sMsg) {
  sStr.reserve(sStr.size() + sNick.size() + sMsg.size() + 4);
  sStr.append("<");
  sStr.append(sNick);
  sStr.append("> ");
  sStr.append(sMsg);
  sStr.append(DC_SEPARATOR);
  return sStr;
}

string & cDCProtocol::Append_DC_PM(string &sStr, const string &sTo, const string &sFrom, const string &sNick, const string &sMsg) {
  //$To: sTo From: sFrom $<sNick> sMsg|
  sStr.reserve(sStr.size() + sTo.size() + sFrom.size() + sNick.size() + sMsg.size() + 18);
  sStr.append("$To: ");
  sStr.append(sTo);
  sStr.append(" From: ");
  sStr.append(sFrom);
  sStr.append(" $<");
  sStr.append(sNick);
  sStr.append("> ");
  sStr.append(sMsg);
  sStr.append(DC_SEPARATOR);
  return sStr;
}

void cDCProtocol::Append_DC_PMToAll(string &sStart, string &sEnd, const string &sFrom, const string &sNick, const string &sMsg) {
  //$To: sTo From: sFrom $<sNick> sMsg|
  sStart.append("$To: ");
  sEnd.reserve(sFrom.size() + sNick.size() + sMsg.size() + 13);
  sEnd.append(" From: ");
  sEnd.append(sFrom);
  sEnd.append(" $<");
  sEnd.append(sNick);
  sEnd.append("> ");
  sEnd.append(sMsg);
  sEnd.append(DC_SEPARATOR);
}

string & cDCProtocol::Append_DC_Quit(string &sStr, const string &sNick) {
  sStr.append("$Quit ");
  sStr.append(sNick);
  sStr.append(DC_SEPARATOR);
  return sStr;
}

string & cDCProtocol::Append_DC_OpList(string &sStr, const string &sNick) {
  sStr.append("$OpList ");
  sStr.append(sNick);
  sStr.append("$$"DC_SEPARATOR);
  return sStr;
}

string & cDCProtocol::Append_DC_UserIP(string &sStr, const string &sNick, const string &sIP) {
  if(sIP.length()) {
    sStr.append("$UserIP ");
    sStr.append(sNick);
    sStr.append(" ");
    sStr.append(sIP);
    sStr.append("$$"DC_SEPARATOR);
  }
  return sStr;
}


void cDCProtocol::SendMode(cDCConn *dcconn, const string & sStr, int iMode, cUserList & UL) {
  bool bAddSep = false;
  if(sStr.substr(sStr.size() - 1, 1) != DC_SEPARATOR) bAddSep = true;

  if(iMode == 0) /** Send to all */
    UL.SendToAll(sStr, true, bAddSep);
  else if(iMode == 3) { /** Send to all except current user */
    if(dcconn->mDCUser->mbInUserList) {
      dcconn->mDCUser->mbInUserList = false;
      UL.SendToAll(sStr, false, bAddSep);
      dcconn->mDCUser->mbInUserList = true;
    }
  } else if(iMode == 4) { /** Send to all except users with ip of the current user */
    cDCConn * conn;
    vector<cDCConn*> ul;
    for(cDCIPList::iterator mit = mDCServer->mIPListConn->begin(cDCConn::Ip2Num(dcconn->GetIp().c_str())); mit != mDCServer->mIPListConn->end(); ++mit) {
      conn = (cDCConn*)(*mit);
      if(conn->mDCUser && conn->mDCUser->mbInUserList) {
        conn->mDCUser->mbInUserList = false;
        ul.push_back(conn);
      }
    }
    UL.SendToAll(sStr, false, bAddSep);
    for(vector<cDCConn*>::iterator ul_it = ul.begin(); ul_it != ul.end(); ++ul_it)
      (*ul_it)->mDCUser->mbInUserList = true;
  }
}


/** Sending the user-list and op-list */
int cDCProtocol::SendNickList(cDCConn *dcconn) {
  try {
    if((dcconn->GetLSFlag(eLS_LOGIN_DONE) != eLS_LOGIN_DONE) && mDCServer->mDCConfig.mbNicklistOnLogin)
      dcconn->mbNickListInProgress = true;

    if(dcconn->mFeatures & eSF_NOHELLO) {
      if(dcconn->Log(3)) dcconn->LogStream() << "Sending MyINFO list" << endl;
      // seperator "|" was added in GetInfoList function
      dcconn->Send(mDCServer->mDCUserList.GetInfoList(true), false, false);
    } else if(dcconn->mFeatures & eSF_NOGETINFO) {
      if(dcconn->Log(3)) dcconn->LogStream() << "Sending MyINFO list and Nicklist" << endl;
      // seperator "|" was not added in GetNickList function, because seperator was "$$"
      dcconn->Send(mDCServer->mDCUserList.GetNickList(), true, false);
      // seperator "|" was added in GetInfoList function
      dcconn->Send(mDCServer->mDCUserList.GetInfoList(true), false, false);
    } else {
      if(dcconn->Log(3)) dcconn->LogStream() << "Sending Nicklist" << endl;
      // seperator "|" was not added in GetNickList function, because seperator was "$$"
      dcconn->Send(mDCServer->mDCUserList.GetNickList(), true, false);
    }
    if(mDCServer->mOpList.Size()) {
      if(dcconn->Log(3)) dcconn->LogStream() << "Sending Oplist" << endl;
      // seperator "|" was not added in GetNickList function, because seperator was "$$"
      dcconn->Send(mDCServer->mOpList.GetNickList(), true, false);
    }

    if(dcconn->mDCUser->mbInUserList && dcconn->mDCUser->mbInIpList) {
      if(dcconn->Log(3)) dcconn->LogStream() << "Sending Iplist" << endl;
      // seperator "|" was not added in GetIpList function, because seperator was "$$"
      dcconn->Send(mDCServer->mDCUserList.GetIpList(), true); // newPolitic
    } else {
      if(!dcconn->SendBufIsEmpty()) // buf would not flush, if it was empty
        dcconn->Flush(); // newPolitic
      else {
        static string s(DC_SEPARATOR);
        dcconn->Send(s);
      }
    }
  } catch(...) {
    if(dcconn->ErrLog(2)) dcconn->LogStream() << "exception in SendNickList" << endl;
    dcconn->CloseNow();
    return -1;
  }
  return 0;
}

/** Get normal share size */
string cDCProtocol::GetNormalShare(__int64 iVal) {
  ostringstream os;
  float s = static_cast<float>(iVal);
  int i = 0;
  for(; ((s >= 1024) && (i < 7)); ++i) s /= 1024;
  os << ::std::floor(s * 1000 + 0.5) / 1000 << " " << mDCServer->mDCLang.msUnits[i];
  return os.str();
}


}; // nProtocol

}; // nDCServer
