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

#ifndef CCONNSELECT_H
#define CCONNSELECT_H

#include "cconnchoose.h"

//#if USE_SELECT

#ifndef _WIN32
  #include <sys/select.h>
  #include <memory.h>
#endif

#include "tchashtable.h"

namespace nServer {


/** cConnSelect */
class cConnSelect : public cConnChoose
{

public:

  typedef tcHashTable<sChooseRes *> tResList;
  struct iterator;

protected:

  tResList mResList; /** Res list */

public:
  cConnSelect(){};
  virtual ~cConnSelect();

  unsigned Size() { return mResList.Size(); }

  virtual bool OptIn(tSocket, tEventFlag);
  virtual void OptOut(tSocket, tEventFlag);
  virtual int OptGet(tSocket);
  virtual int RevGet(tSocket);
  virtual bool RevTest(tSocket);
  virtual inline int Choose(cTime &tmout){ return this->Select(tmout); };


  struct cSelectFD : public fd_set
  {
    cSelectFD()
    {
      FD_ZERO(this);
    }
    cSelectFD & operator = (const cSelectFD &set)
    {
      #ifdef _WIN32
        fd_count = set.fd_count;
        static size_t fd_array_size = sizeof(fd_array);
        ::memcpy(&fd_array, &(set.fd_array), fd_array_size);
      #else
        static size_t fds_bits_size = sizeof(fds_bits);
        memcpy(&fds_bits, &(set.fds_bits), fds_bits_size);
      #endif
      return *this;
    }
    bool IsSet(tSocket sock){ return FD_ISSET(sock, this) != 0; }

    void Clr(tSocket sock){ FD_CLR(sock, this); }

    bool Set(tSocket sock)
    {
      #ifdef _WIN32
        if(fd_count >= FD_SETSIZE) return false;
      #endif
      FD_SET(sock, this);
      return true;
    }
  };

  void ClearRevents();
  void SetRevents(cSelectFD &fdset, unsigned eMask);

  struct iterator
  {
    tResList::iterator mIt; /** iterator for list */
    cConnSelect *mSel; /** for operator [] */
    iterator(){}
    iterator(cConnSelect *sel, tResList::iterator it) : mIt(it), mSel(sel)
    {}

    iterator & operator = (const iterator &it)
    {
      mSel= it.mSel;
      mIt = it.mIt;
      return *this;
    }
    bool operator != (const iterator &it){ return mIt != it.mIt; }
    sChooseRes & operator *()
    {
      //__try {
        if((*mIt)->mConnBase == NULL)
          (*mIt)->mConnBase = (*mSel)[(*mIt)->mFd];
      /*} __except(1) {
        if(mSel->mResList.ErrLog(0)) mSel->mResList.LogStream() << "Fatal error: " << endl
          << "error in operator *()" << endl
          << "Item = " << mIt.mItem << endl
          << "Hash = " << mIt.i.i << endl
          << "End = " << mIt.i.end << endl;
      }*/
      return *(*mIt);
    }

    iterator & operator ++()
    {
      //__try {
      while(!(++mIt).IsEnd() && !(*mIt)->mRevents){}
      /*} __except(1) {
        if(mSel->mResList.ErrLog(0)) mSel->mResList.LogStream() << "Fatal error: " << endl
          << "error in operator ++()" << endl
          << "Item = " << mIt.mItem << endl
          << "Hash = " << mIt.i.i << endl
          << "End = " << mIt.i.end << endl;
      }*/
      return *this;
    }
  }; // iterator

  iterator begin(){ return iterator(this, mResList.begin()); }
  iterator end(){ return iterator(this, mResList.end()); }

protected:

  cSelectFD mReadFS; /** For read */
  cSelectFD mWriteFS; /** For write */
  cSelectFD mExceptFS; /** Errors */
  cSelectFD mCloseFS; /** Closed */

  // select results
  cSelectFD mResReadFS;
  cSelectFD mResWriteFS;
  cSelectFD mResExceptFS;

protected:

  /** Do select */
  int Select(cTime &tmout);

}; // cConnSelect

}; // nServer

//#endif // USE_SELECT

#endif // CCONNSELECT_H
