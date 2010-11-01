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

#ifndef CDCCONN_H
#define CDCCONN_H

#include "cdcconnbase.h"
#include "cconn.h"
#include "cdcuser.h"
#include "ctimeout.h"

using namespace nServer;
using namespace nUtils;

namespace nDCServer {

namespace nEnums {

/** Login steps (Login Status) */
typedef enum {
  eLS_KEY        = 1 << 0, //< Key was checked (once)
  eLS_ALOWED     = 1 << 1, //< It allow for entry
  eLS_VALNICK    = 1 << 2, //< Nick was checked (once)
  eLS_PASSWD     = 1 << 3, //< Password was right or password was not need
  eLS_VERSION    = 1 << 4, //< Version was checked
  eLS_MYINFO     = 1 << 5, //< MyINFO string was received
  eLS_NICKLST    = 1 << 6, //< GetNickList flag
  eLS_LOGIN_DONE = eLS_KEY|eLS_ALOWED|eLS_VALNICK|eLS_PASSWD|eLS_VERSION|eLS_MYINFO|eLS_NICKLST
} tLoginStatus;

/** TimeOuts */
typedef enum {
  eTO_KEY = 0, //< Waiting $Key after $Lock
  eTO_VALNICK, //< Waiting $ValidateNick after $Lock
  eTO_LOGIN,   //< Life time of the connection object before full entry (DoUserEnter)
  eTO_MYINFO,  //< After $ValidateNick and before $MyINFO timeout
  eTO_PASS,    //< Waiting pass
  eTO_MAX      //< Max timeout type
} tTimeOut;

/** Supports features (SupportFeatures) */
typedef enum {
  eSF_USERCOMMAND = 1     , //< UserCommand feature
  eSF_NOGETINFO   = 1 << 1, //< NoGetINFO feature
  eSF_NOHELLO     = 1 << 2, //< NoHello feature
  eSF_USERIP2     = 1 << 3, //< UserIP2 feature
  eSF_TTHSEARCH   = 1 << 4, //< TTHSearch feature
  eSF_QUICKLIST   = 1 << 5, //< Quicklist feature
  eSF_PASSIVE     = 1 << 6, //< Passive mode feature
} tSupportFeature;

/** Enumeration of reasons to closing connection (Close Reason) */
enum {
  eCR_DEFAULT = 0,  //< Default
  eCR_SYNTAX,       //< Symtax error in NMDC command
  eCR_HUB_LOAD,     //< Critical loading on the hub, do not take new users
  eCR_TIMEOUT,      //< Timeout to receive of the command
  eCR_TO_ANYACTION, //< Any action timeout
  eCR_INVALID_USER, //< Invalid user (bad nick or IP)
  eCR_CHAT_NICK,    //< Using bad nick in the chat
  eCR_QUIT,         //< Using $Quit command
  eCR_LOGIN_ERR,    //< Bad login sequence
  eCR_KICKED,       //< User was kicked
  eCR_FORCEMOVE,    //< User was force moved on other hub
  eCR_PLUGIN,
  eCR_UNKNOWN_CMD,  //< Unknown cmd
  eCR_FLOOD
};

}; // nEnums

using namespace ::nDCServer::nEnums;

class cDCServer; /** Server() */

namespace nProtocol {class cDCProtocol;}
using nProtocol::cDCProtocol;

class cDCConnFactory : public cConnFactory
{

public:
  cDCConnFactory(cProtocol *protocol, cServer *s);
  virtual ~cDCConnFactory();
  virtual cConn * CreateConn(tSocket sock = 0);
  virtual void DelConn(cConn * &);
};

class cDCConn : public cConn, public cDCConnBase
{
  friend class nProtocol::cDCProtocol; /** for miSRCounter from cDCProtocol::DC_SR */

public:
  unsigned mFeatures;         //< Features
  string msSupports;          //< Support cmd param
  string msVersion;           //< DC version
  string msData;              //< Some user's data
  int miProfile;              //< Profile
  int miCloseReason;          //< Reason of close connection
  bool mbSendNickList;        //< Sending user list when login
  bool mbIpRecv;              //< Permit on reception of the messages, sending on my ip
  bool mbNickListInProgress;  //< True while sending first nicklist
  cDCUser * mDCUser;          //< User object

  struct sTm /** Timers */
  {
    cTime mSearch;
    unsigned miSearch;
    cTime mSR;
    unsigned miSR;
    cTime mMyINFO;
    unsigned miMyINFO;
    cTime mChat;
    unsigned miChat;
    cTime mNickList;
    unsigned miNickList;
    cTime mTo;
    unsigned miTo;
    cTime mCTM;
    unsigned miCTM;
    cTime mRCTM;
    unsigned miRCTM;
    cTime mUnknown;
    unsigned miUnknown;
    sTm() : 
      mSearch(0l), miSearch(0), 
      mSR(0l), miSR(0), 
      mMyINFO(0l), miMyINFO(0), 
      mChat(0l), miChat(0), 
      mNickList(0l), miNickList(0), 
      mTo(0l), miTo(0), 
      mCTM(0l), miCTM(0), 
      mRCTM(0l), miRCTM(0),
      mUnknown(0l), miUnknown(0)
    {}
  } mTimes1, mTimes2;

public:

  cDCConn(tSocket sock = 0, cServer *s = NULL);
  virtual ~cDCConn();
  virtual const string & GetVersion() const { return msVersion; }  //< Client's protocol version
  virtual const string & GetIp() const { return msIp; }            //< Get string of IP
  virtual const string & GetData() const { return msData; }        //< Get some user data
  virtual const string & GetMacAddr() const { return msMAC; }      //< Get mac address
  virtual const string & GetSupports() const { return msSupports; }
  virtual int GetPort() const { return miPort; }             //< Get real port
  virtual int GetPortConn() const { return miPortConn; }     //< Get connection port
  virtual int GetProfile() const { return miProfile; }       //< Get profile
  virtual unsigned long GetNetIp() const { return miNetIp; }       //< Get numeric IP
  virtual void SetProfile(int iProfile) { miProfile = iProfile; }
  virtual void SetData(const string & sData){ msData = sData; }
  virtual long GetEnterTime() const { return mTimes.mKey.Sec(); }

  /** Sending RAW command to the client */
  virtual int Send(const string & sData, bool bAddSep = false, bool bFlush = true);
  /** Flush sending buffer */
  virtual void OnFlush();

  inline void SetLSFlag(unsigned int s){mLoginStatus |= s;}               //< Setting entry status flag
  inline void ReSetLSFlag(unsigned int s){mLoginStatus = s;}              //< Reset flag
  inline unsigned int GetLSFlag(unsigned int s){return mLoginStatus & s;} //< Get flag

  int SetTimeOut(tTimeOut, double Sec, cTime &now); //< Set timeout
  int ClearTimeOut(tTimeOut); //< Clear timeout
  int CheckTimeOut(tTimeOut t, cTime &now); // Check timeout

  virtual int OnTimer(cTime &now); //< Timer for current connection

  virtual void CloseNow(int iReason = 0);
  virtual void CloseNice(int msec, int iReason = 0);
  virtual void Disconnect(){ CloseNice(9000, eCR_PLUGIN); } // for plugins

  /** Pointer to the server */
  inline cDCServer * Server(){ return (cDCServer*) mServer; }
  bool SetUser(cDCUser * User); /** Set user object for current connection */

private:
  cTime mLastSend;
  unsigned int mLoginStatus; //< Login status
  
protected:

  cTimeOut mTimeOut[eTO_MAX];
  unsigned miSRCounter; //< Counter search results

  struct sTimes { /** Timers */
    cTime mKey;   //< Time sending cmd $Key to the server
    cTime mPing;  //< Time last ping from the client
  } mTimes;

protected:
  virtual int OnCloseNice(); /** Event of nice close connection */

}; // cDCConn

}; // nDCServer

#endif // CDCCONN_H
