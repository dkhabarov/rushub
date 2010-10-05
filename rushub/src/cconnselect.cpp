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

#include "cconnselect.h"

//#if USE_SELECT

namespace nServer
{

cConnSelect::~cConnSelect()
{
  tResList::iterator it;
  sChooseRes *ChR;
  tSocket sock;
  for(it = mResList.begin(); it != mResList.end();) {
    ChR = *it;
    ++it;
    if(ChR) {
      sock = ChR->mFd;
      delete ChR;
      mResList.Remove(sock);
    }
  }
}

bool cConnSelect::OptIn(tSocket sock, tEventFlag eMask)
{
  if(eMask & eEF_INPUT ) 
    if(!mReadFS.Set(sock)) return false;
  if(eMask & eEF_OUTPUT) 
    if(!mWriteFS.Set(sock)) return false;
  if(eMask & eEF_ERROR ) 
    if(!mExceptFS.Set(sock)) return false;
  if(eMask & eEF_CLOSE ) 
    if(!mCloseFS.Set(sock)) return false;

  sChooseRes *ChR = mResList.Find(sock);
  if(!ChR) {
    ChR = new sChooseRes;
    ChR->mFd = sock;
    ChR->mEvents = eMask;
    mResList.Add(sock, ChR);
  }
  else
    ChR->mEvents |= eMask;

  return true;
}

void cConnSelect::OptOut(tSocket sock, tEventFlag eMask)
{
  if(eMask & eEF_INPUT ) mReadFS.Clr(sock);
  if(eMask & eEF_OUTPUT) mWriteFS.Clr(sock);
  if(eMask & eEF_ERROR ) mExceptFS.Clr(sock);
  if(eMask & eEF_CLOSE ) mCloseFS.Clr(sock);

  sChooseRes *ChR = mResList.Find(sock);
  if(ChR) {
    ChR->mEvents -= (ChR->mEvents & eMask);
    if(!ChR->mEvents) {
      delete ChR;
      mResList.Remove(sock);
    }
  }
}

int cConnSelect::OptGet(tSocket sock)
{
  int eMask = 0;
  if(mReadFS.IsSet(sock)  ) eMask |= eEF_INPUT;
  if(mWriteFS.IsSet(sock) ) eMask |= eEF_OUTPUT;
  if(mExceptFS.IsSet(sock)) eMask |= eEF_ERROR;
  if(mCloseFS.IsSet(sock) ) eMask |= eEF_CLOSE;
  return eMask;
}

int cConnSelect::RevGet(tSocket sock)
{
  int eMask = 0;
  if(mResReadFS.IsSet(sock)  ) eMask |= eEF_INPUT;
  if(mResWriteFS.IsSet(sock) ) eMask |= eEF_OUTPUT;
  if(mResExceptFS.IsSet(sock)) eMask |= eEF_ERROR;
  if(mCloseFS.IsSet(sock)    ) eMask |= eEF_CLOSE;
  return eMask;
}

bool cConnSelect::RevTest(tSocket sock)
{
  if(mResWriteFS.IsSet(sock) ) return true;
  if(mResReadFS.IsSet(sock)  ) return true;
  if(mResExceptFS.IsSet(sock)) return true;
  if(mCloseFS.IsSet(sock)    ) return true;
  return false;
}

/** Do select */
int cConnSelect::Select(cTime &tmout)
{
  mResReadFS = mReadFS;
  mResWriteFS = mWriteFS;
  mResExceptFS = mExceptFS;

  /** select */
  int ret = ::select(mMaxSocket, &mResReadFS, &mResWriteFS, &mResExceptFS, (timeval *)(&tmout));
  if(ret == SOCKET_ERROR) return -1;

  ClearRevents();
  SetRevents(mResReadFS, eEF_INPUT);
  SetRevents(mResWriteFS, eEF_OUTPUT);
  SetRevents(mResExceptFS, eEF_ERROR);
  SetRevents(mCloseFS, eEF_CLOSE);
  return ret;
}

void cConnSelect::ClearRevents(void)
{
  tResList::iterator it;
  for(it = mResList.begin(); it != mResList.end(); ++it)
    if(*it) (*it)->mRevents = 0;
}


void cConnSelect::SetRevents(cSelectFD &fdset, unsigned eMask)
{
  tSocket sock;
  #ifdef _WIN32
  for(unsigned i = 0; i < fdset.fd_count; ++i) {
    sock = fdset.fd_array[i];
    sChooseRes *ChR = mResList.Find(sock);
    if(!ChR) { 
      ChR = new sChooseRes;
      ChR->mFd = sock;
      ChR->mEvents = 0; 
      ChR->mRevents = eMask; 
      mResList.Add(sock, ChR);
    }
    else
      ChR->mRevents |= eMask;
  }
  #else
  for(unsigned i = 0; i < FD_SETSIZE; ++i) {
    sock = i;
    if(FD_ISSET(sock, &fdset)) {
      sChooseRes *ChR = mResList.Find(sock);
      if(!ChR) {
        ChR = new sChooseRes;
        ChR->mFd = sock;
        ChR->mEvents = 0;
        ChR->mRevents = eMask;
        mResList.Add(sock, ChR);
      }
      else
        ChR->mRevents |= eMask;
    }
  }
  #endif
}

}; // nServer

//#endif // USE_SELECT
