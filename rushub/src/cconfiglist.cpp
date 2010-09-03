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

#include "cconfiglist.h"

namespace nConfig
{

cConfigListBase::cConfigListBase() : cObj("cConfigListBase")
{
  mBase = NULL; /** NULL base pointer */
  mFactory = new cConfigFactory;
}

cConfigListBase::~cConfigListBase()
{
  tHashType Hash;
  cConfig *config; /** Pointer on object */
  for(tVIt it = mKeyList.begin(); it != mKeyList.end(); ++it)
  {
    Hash = *it; /** Get hash from vector */
    config = mList.Find(Hash); /** Get object by hash */
    mList.Remove(Hash); /** Del object from list by hash */
    this->mFactory->Delete(config); /** Del object */
  }
  if(mFactory != NULL) delete mFactory;
  mFactory = NULL;
}

/** Adding config */
cConfig * cConfigListBase::Add(const string &sKey, cConfig *cbi)
{
  tHashType Hash = mList.mHash(sKey);
  if(!mList.Add(Hash, cbi)) /** Add */
  {
    if(Log(1))
    {
      cConfig *other = mList.Find(Hash);
      LogStream() << "Don't add " << sKey << " because of " << (other ? other->msName.c_str() : "NULL") << endl;
      return NULL;
    }
  }
  mKeyList.push_back(Hash); /** Push back of vector */
  cbi->msName = sKey; /** Record name */
  return cbi;
}

/** Get config by name */
cConfig * cConfigListBase::operator[](const char * sName)
{
  return mList.Find(mList.mHash(sName));
}

/** Get config by name */
cConfig * cConfigListBase::operator[](const string &sName)
{
  return mList.Find(mList.mHash(sName));
}

/** Set new address */
void cConfigListBase::SetBaseTo(void *new_base)
{
  if(mBase)
    for(tVIt it = mKeyList.begin(); it != mKeyList.end(); ++it)
      mList.Find(*it)->mAddr = (void*)(long(mList.Find(*it)->mAddr) + (long(new_base) - long(mBase)));
  mBase = new_base;
}

/** Creating and adding configs */
#define ADDVAL(TYPE) \
cConfig * cConfigList::Add(const string &sName, TYPE &Var) \
{ \
  cConfig * cbi = this->mFactory->Add(Var); \
  return this->cConfigListBase::Add(sName, cbi); \
} \
cConfig * cConfigList::Add(const string &sName, TYPE &Var, TYPE const &Def) \
{ \
  cConfig * cbi = this->Add(sName, Var); \
  *cbi = Def; \
  return cbi; \
}
#define ADDPVAL(TYPE) \
cConfig * cConfigList::Add(const string &sName, TYPE* &Var) \
{ \
  cConfig * cbi = this->mFactory->Add(Var); \
  return this->cConfigListBase::Add(sName, cbi); \
} \
cConfig * cConfigList::Add(const string &sName, TYPE* &Var, TYPE const &Def) \
{ \
  cConfig * cbi = this->Add(sName, Var); \
  *cbi = &Def; \
  return cbi; \
}
#define ADDVALUES(TYPE) \
ADDVAL(TYPE); \
ADDPVAL(TYPE);

ADDVAL(char);
ADDVAL(char*);
ADDVALUES(bool);
ADDVALUES(int);
ADDVALUES(double);
ADDVALUES(long);
ADDVALUES(unsigned);
ADDVALUES(unsigned long);
ADDVALUES(__int64);
ADDVALUES(string);

}; // nConfig
