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

#include "cdciplist.h"
#include "cdcconn.h"

namespace nDCServer
{

cDCIPList::cDCIPList() : cObj("cDCIPList"), mbFlush(false), mbAddSep(false)
{
}

cDCIPList::~cDCIPList()
{
  tItem * Item;
  tIPList::iterator it;
  for(it = mIPList.begin(); it != mIPList.end(); ++it) {
    Item = (*it);
    if(Item) delete Item;
    Item = NULL;
  }
}

bool cDCIPList::Add(cDCConn* Conn)
{
  tItem * Item = mIPList.Find(Conn->GetNetIp());
  if(Item == NULL)
    mIPList.Add(Conn->GetNetIp(), new tItem(Conn->mSocket, Conn));
  else
    Item->Add(Conn->mSocket, Conn);
  return true;
}

bool cDCIPList::Remove(cDCConn* Conn)
{
  tItem *Item = NULL, *Items = mIPList.Find(Conn->GetNetIp());
  if(Items == NULL) return false;
  Item = Items;
  cConn * conn = Items->Remove(Conn->mSocket, Item);
  if(Item != Items) {
    if(Item) mIPList.Update(Conn->GetNetIp(), Item);
    else mIPList.Remove(Conn->GetNetIp()); /** Removing the list from hash-table */
    delete Items; /** removing old start element in the list */
    Items = NULL;
  }
  if(conn == NULL) return false;
  return true;
}

void cDCIPList::SendToIP(const char *sIP, string &sData, unsigned long iProfile, bool bFlush, bool bAddSep)
{ SendToIP(cConn::Ip2Num(sIP), sData, iProfile, bFlush, bAddSep); }

void cDCIPList::SendToIPWithNick(const char *sIP, string &sStart, string &sEnd, unsigned long iProfile, bool bFlush, bool bAddSep)
{ SendToIPWithNick(cConn::Ip2Num(sIP), sStart, sEnd, iProfile, bFlush, bAddSep); }

void cDCIPList::SendToIP(unsigned long iIP, string &sData, unsigned long iProfile, bool bFlush, bool bAddSep)
{
  miProfile = iProfile;
  msData1 = sData;
  mbFlush = bFlush;
  mbAddSep = bAddSep;
  tItem *Item = mIPList.Find(iIP);
  while(Item != NULL) {
    Send(Item->mData);
    Item = Item->mNext;
  }
}

void cDCIPList::SendToIPWithNick(unsigned long iIP, string &sStart, string &sEnd, unsigned long iProfile, bool bFlush, bool bAddSep)
{
  miProfile = iProfile;
  msData1 = sStart;
  msData2 = sEnd;
  mbFlush = bFlush;
  mbAddSep = bAddSep;
  tItem *Item = mIPList.Find(iIP);
  while(Item != NULL) {
    SendWithNick(Item->mData);
    Item = Item->mNext;
  }
}

int cDCIPList::Send(cDCConn * Conn)
{
  if(!Conn || !Conn->mbIpRecv) return 0;
  if(!miProfile)
    return Conn->Send(msData1, mbAddSep, mbFlush);
  else {
    static int iProfile;
    iProfile = Conn->miProfile + 1;
    if(iProfile < 0) iProfile = -iProfile;
    if(iProfile > 31) iProfile = (iProfile % 32) - 1;
    if(miProfile & (1 << iProfile))
      return Conn->Send(msData1, mbAddSep, mbFlush);
  }
  return 0;
}

int cDCIPList::SendWithNick(cDCConn * Conn)
{
  if(!Conn || !Conn->mDCUser || !Conn->mbIpRecv) return 0;
  string sStr(msData1);
  sStr.append(Conn->mDCUser->msNick);
  if(!miProfile)
    return Conn->Send(sStr.append(msData2), mbAddSep, mbFlush);
  else {
    static int iProfile;
    iProfile = Conn->miProfile + 1;
    if(iProfile < 0) iProfile = -iProfile;
    if(iProfile > 31) iProfile = (iProfile % 32) - 1;
    if(miProfile & (1 << iProfile)) {
      string sStr(msData1);
      sStr.append(Conn->mDCUser->msNick);
      return Conn->Send(sStr.append(msData2), mbAddSep, mbFlush);
    }
  }
  return 0;
}

}; // nDCServer
