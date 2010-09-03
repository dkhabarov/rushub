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

#include "cdcuser.h"
#include "cdcconn.h"
#include "cdcserver.h"

namespace nDCServer
{

cDCUser::cDCUser() :
  cObj("cDCUser"),
  cUserBase(),
  cDCUserBase(),
  mDCServer(NULL),
  mTagSep(',')
{
  mDCConn = NULL;
  mDCConnBase = NULL;
}

cDCUser::cDCUser(const string & sNick) :
  cObj("cDCUser"),
  cUserBase(),
  cDCUserBase(),
  mDCServer(NULL),
  mTagSep(',')
{
  mDCConn = NULL;
  mDCConnBase = NULL;
  msNick = sNick;
  mNil = 0;
}

cDCUser::~cDCUser()
{
  mDCConn = NULL;
  mDCConnBase = NULL;
  mDCServer = NULL;
}

bool cDCUser::CanSend()
{
  return mbInUserList && mDCConn && mDCConn->mbOk;
}

void cDCUser::Send(string & sData, bool bAddSep, bool bFlush)
{
  if(mDCConn) mDCConn->Send(sData, bAddSep, bFlush);
}

int cDCUser::GetProfile() const {
  if(mDCConn) return mDCConn->miProfile;
  else return cUserBase::GetProfile();
}

void cDCUser::SetIp(const string & sIP)
{
  if(sIP.size() && cDCConn::CheckIp(sIP))
    msIp = sIP;
}

/** Set MyINFO string (for plugins). With cmd & nick check */
bool cDCUser::SetMyINFO(const string & sMyINFO, const string & sNick)
{
  if(mDCServer->CheckCmd(sMyINFO) != eDC_MYNIFO ||
  mDCServer->mDCParser.ChunkString(eCH_MI_NICK) != sNick)
    return false;
  if(msMyINFO != sMyINFO) {
    msMyINFO = sMyINFO;
    ParseMyINFO(&mDCServer->mDCParser);
  }
  return true;
}

bool cDCUser::SetMyINFO(cDCParserBase * Parser)
{
  if(msMyINFO != Parser->msStr) {
    msMyINFO = Parser->msStr;
    ParseMyINFO(Parser);
  }
  return true;
}

void cDCUser::ParseMyINFO(cDCParserBase * Parser) {

  unsigned int OldNil = mNil;
  mNil = 0; // Set null value for all params

  msDesc = Parser->ChunkString(eCH_MI_DESC);
  size_t l = msDesc.size();
  if (l) {
    unsigned int i = msDesc.find_last_of('<');
    if (msDesc[--l] == '>' && i != msDesc.npos) {
      mNil |= eMYINFO_TAG;
      string sOldTag = msTag; ++i;
      msTag.assign(msDesc, i, l - i);
      msDesc.assign(msDesc, 0, --i);
      if (msTag.compare(sOldTag) != 0) // optimization
        ParseTag();
      else
        mNil |= OldNil;
    }
  }

  msConnection = Parser->ChunkString(eCH_MI_SPEED);
  int connSize = msConnection.size();
  if(connSize) {
    miByte = int(msConnection[--connSize]);
    msConnection.assign(msConnection, 0, connSize);
  }

  msEmail = Parser->ChunkString(eCH_MI_MAIL);

  __int64 iShare = StringToInt64(Parser->ChunkString(eCH_MI_SIZE));
  mDCServer->miTotalShare -= miShare;
  mDCServer->miTotalShare += iShare;
  miShare = iShare;

}

void cDCUser::ParseTag() {

  /* client and version */
  size_t iTagSize = msTag.size();
  size_t iClient = msTag.find(mTagSep);
  size_t iP = msTag.find("V:");

  mNil |= eMYINFO_CLIENT;
  if(iClient == msTag.npos) iClient = iTagSize;
  if(iP != msTag.npos) {
    mNil |= eMYINFO_VERSION;
    msVersion.assign(msTag, iP + 2, iClient - iP - 2);
    msClient.assign(msTag, 0, iP);
  } else {
    iP = msTag.find(' ');
    if(iP != msTag.npos && iP < iClient) { ++iP;
      if(atof(msTag.substr(iP, iClient - iP).c_str())) {
        mNil |= eMYINFO_VERSION;
        msVersion.assign(msTag, iP, iClient - iP);
        msClient.assign(msTag, 0, --iP);
      } else msClient.assign(msTag, 0, iClient);
    } else msClient.assign(msTag, 0, iClient);
  }
  trim(msClient);
  trim(msVersion);

  /* mode */
  iP = msTag.find("M:");
  if(iP != msTag.npos) {
    mNil |= eMYINFO_MODE;
    iP += 2;
    size_t iMode = msTag.find(mTagSep, iP);
    if(iMode == msTag.npos) iMode = iTagSize;
    msMode.assign(msTag, iP, iMode - iP);
    if(iMode > iP) {
      unsigned p = msMode[0];
      if(p == 80 || p == 53 || p == 83) mbPassive = true;
    }
  }
  string tmp;

  /* hubs */
  iP = msTag.find("H:");
  if(iP != msTag.npos) {
    iP += 2;
    size_t iUnReg = msTag.find('/', iP);
    if(iUnReg == msTag.npos) {
      iUnReg = msTag.find(mTagSep, iP);
      if(iUnReg == msTag.npos) iUnReg = iTagSize;
    } else {
      size_t iReg = msTag.find('/', ++iUnReg);
      if(iReg == msTag.npos) {
        iReg = msTag.find(mTagSep, iUnReg);
        if(iReg == msTag.npos) iReg = iTagSize;
      } else {
        size_t iOp = msTag.find('/', ++iReg);
        if(iOp == msTag.npos) {
          iOp = msTag.find(mTagSep, iReg);
          if(iOp == msTag.npos) iOp = iTagSize;
        }
        miOpHubs = atoi(tmp.assign(msTag, iReg, iOp - iReg).c_str());
        if(tmp.size()) mNil |= eMYINFO_UNREG;
      }
      miRegHubs = atoi(tmp.assign(msTag, iUnReg, iReg - iUnReg - 1).c_str());
      if(tmp.size()) mNil |= eMYINFO_REG;
    }
    miUnRegHubs = atoi(tmp.assign(msTag, iP, iUnReg - iP - 1).c_str());
    if(tmp.size()) mNil |= eMYINFO_OP;
  }

  /* slots and limits */
  FindIntParam("S:", miSlots, eMYINFO_SLOT);
  FindIntParam("L:", miLimit, eMYINFO_LIMIT);
  FindIntParam("O:", miOpen, eMYINFO_OPEN);
  FindIntParam("B:", miBandwidth, eMYINFO_BANDWIDTH);
  FindIntParam("D:", miDownload, eMYINFO_DOWNLOAD);

  iP = msTag.find("F:");
  if(iP != msTag.npos) {
    mNil |= eMYINFO_FRACTION;
    iP += 2;
    size_t iFraction = msTag.find(mTagSep, iP);
    if(iFraction == msTag.npos) iFraction = iTagSize;
    msFraction.assign(msTag, iP, iFraction - iP);
  }
}

void cDCUser::FindIntParam(const string find, int & param, unsigned f) {
  size_t iP = msTag.find(find);
  if(iP != msTag.npos) {
    string tmp;
    mNil |= f;
    iP += 2;
    size_t iParam = msTag.find(mTagSep, iP);
    if(iParam == msTag.npos) iParam = msTag.size();
    param = atoi(tmp.assign(msTag, iP, iParam - iP).c_str());
  }
}

void cDCUser::SetOpList(bool bInOpList) /** Set/unset user in OpList (for plugins) */
{
  if(bInOpList) mDCServer->AddToOps(this);
  else mDCServer->DelFromOps(this);
}

void cDCUser::SetIpList(bool bInIpList) /** Set/unset user in IpList (for plugins) */
{
  if(bInIpList) mDCServer->AddToIpList(this);
  else mDCServer->DelFromIpList(this);
}

void cDCUser::SetHide(bool bHide) /** Set/unset user in HideList (for plugins) */
{
  if(bHide) mDCServer->AddToHide(this);
  else mDCServer->DelFromHide(this);
}

}; // nDCServer
