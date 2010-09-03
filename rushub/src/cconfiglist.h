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

#ifndef CCONFIGLIST_H
#define CCONFIGLIST_H

#include "cobj.h"
#include "tchashmap.h"
#include "cconfig.h"
#include <vector>

using std::vector;
using namespace nUtils;

namespace nConfig
{

#define NEWITEM(TYPE, SUFFIX) \
virtual cConfig##SUFFIX  *Add(TYPE  &v){return new cConfig##SUFFIX (v);} \
virtual cConfigP##SUFFIX *Add(TYPE* &v){return new cConfigP##SUFFIX(v);}

/** The Class of the making the object for any type. 
Contains the functions of the creation of all available types. */
class cConfigFactory
{
public:
  cConfigFactory(){}
  NEWITEM(bool, Bool);            /** virtual cConfigBool    * Add(bool &var){return new cConfigBool(var)} */
  NEWITEM(double, Double);        /** virtual cConfigDouble  * Add(double &var){return new cConfigDouble(var)} */
  NEWITEM(int, Int);              /** virtual cConfigInt     * Add(int &var){return new cConfigInt(var)} */
  NEWITEM(long, Long);            /** virtual cConfigLong    * Add(long &var){return new cConfigLong(var)} */
  NEWITEM(unsigned int, UInt);    /** virtual cConfigUInt    * Add(unsigned int &var){return new cConfigUInt(var)} */
  NEWITEM(unsigned long, ULong);  /** virtual cConfigULong   * Add(unsigned long &var){return new cConfigULong(var)} */
  NEWITEM(__int64, Int64);        /** virtual cConfigInt64   * Add(__int64 &var){return new cConfigInt64(var)} */
  NEWITEM(char, Char);            /** virtual cConfigChar    * Add(char &var){return new cConfigChar(var)} */
  NEWITEM(string, String);        /** virtual cConfigString  * Add(string &var){return new cConfigString(var)} */
  virtual void Delete(cConfig *Item){ delete Item; } /** Удаление объекта настройки */
}; // cConfigFactory

/** Base config class (Config Base Base) */
class cConfigListBase : public cObj
{
public:

  typedef unsigned tHashType;
  typedef vector<unsigned> tVector; /** Vector */
  typedef tVector::iterator tVIt; /** Iterator of vector */
  typedef tcHashMap<cConfig*, unsigned> tHashMap;
  typedef tHashMap::iterator tHLMIt; /** Iterator of list */

  tHashMap mList; /** Main list of data */
  tVector mKeyList; /** Main vector of all hash values */

  cConfigFactory *mFactory; /** Config factory */
  void *mBase; /** Main pointer */

  tcHash<tHashType> mHash; /** Hash function */

  /** Iterator for config class */
  struct iterator
  {
    cConfigListBase *mConfigListBase;
    tVIt mIterator;

    iterator(){}
    iterator(cConfigListBase *ConfigList, const tVIt &it) : mConfigListBase(ConfigList), mIterator(it){}
    cConfig *operator *(){return mConfigListBase->mList.Find(*mIterator);}
    iterator &operator ++(){++mIterator; return *this;}
    bool operator != (iterator &it){return mIterator != it.mIterator;}
    iterator(iterator &it){operator = (it);}
    iterator &Set(cConfigListBase *C, const tVIt &it){mConfigListBase = C; mIterator = it; return *this;}
    iterator &operator = (iterator &it){mIterator = it.mIterator; mConfigListBase = it.mConfigListBase; return *this;}
  };
  iterator mBegin; /** Begin iterator */
  iterator mEnd; /** End iterator */

public:
  cConfigListBase();
  virtual ~cConfigListBase();

  cConfig *operator[](const char *); /** Get config by name */
  cConfig *operator[](const string &); /** Get config by name */
  cConfig *Add(const string &, cConfig *); /** Add config object in list */
  void SetBaseTo(void *new_base); /** Set base pointer in address */

  virtual bool Load() = 0; /** Loading from store */
  virtual bool Save() = 0; /** Saving to store */

  iterator &begin(){ return mBegin.Set(this, mKeyList.begin()); } /** Set begin iterator */
  iterator &end()  { return mEnd.Set(this, mKeyList.end()); } /** Set end iterator */

}; // cConfigListBase

/** Adding */
#define ADDMTDS(TYPE) \
cConfig *Add(const string &sName, TYPE &Var); \
cConfig *Add(const string &sName, TYPE &Var, TYPE const &Def);

#define ADDPMTDS(TYPE) \
cConfig *Add(const string &sName, TYPE* &Var); \
cConfig *Add(const string &sName, TYPE* &Var, TYPE const &Def);

#define ADDMETHODS(TYPE) \
ADDMTDS(TYPE); \
ADDPMTDS(TYPE);


/**
  This class is a main class for each config with like structure.
  You may link real given with their names. You may get these data or change them.
  Convert from/in string of the type std::string, read/write from/in stream. Very useful class.
*/
class cConfigList : public cConfigListBase
{
public:
  cConfigList(){}
  virtual ~cConfigList(){}

  ADDMTDS(char);
  ADDMTDS(char*);
  ADDMETHODS(bool);
  ADDMETHODS(int);
  ADDMETHODS(long);
  ADDMETHODS(double);
  ADDMETHODS(unsigned int);
  ADDMETHODS(unsigned long);
  ADDMETHODS(__int64);
  ADDMETHODS(string);

}; // cConfigList

}; // nConfig

#endif // CCONFIGLIST_H
