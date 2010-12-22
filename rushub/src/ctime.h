/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

 * modified: 10 Dec 2009
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

#ifndef CTIME_H
#define CTIME_H

#if defined _WIN32 || HAVE_OSTREAM
	#include <ostream>
#else
	//namespace std{
		#include <ostream>
	//};
#endif

#include <time.h>
#include <string>
using std::string;

#ifdef _WIN32
	#include "conndef.h" // for class timeval
	void gettimeofday(struct timeval*, struct timezone*);
#else
	#include <sys/time.h> // for gettimeofday
#endif

namespace nUtils {

/** Class of time with microsecond decision and arithmetical operation */
class cTime: public timeval {
public:
	~cTime(){}
	cTime(): mPrintType(0){Get();}
	cTime(double sec): mPrintType(0){tv_sec = (long)sec; tv_usec = long((sec - tv_sec) * 1000000);};
	cTime(long sec, long usec = 0): mPrintType(0){tv_sec = sec; tv_usec = usec;};
	cTime(const cTime &t) : mPrintType(0){tv_sec = t.tv_sec; tv_usec = t.tv_usec;};

	int operator>  (const cTime &t) const{if(tv_sec > t.tv_sec) return 1; if(tv_sec < t.tv_sec) return 0; return (tv_usec > t.tv_usec);}
	int operator>= (const cTime &t) const{if(tv_sec > t.tv_sec) return 1; if(tv_sec < t.tv_sec) return 0; return (tv_usec >= t.tv_usec);}
	int operator<  (const cTime &t) const{if(tv_sec < t.tv_sec) return 1; if(tv_sec > t.tv_sec) return 0; return (tv_usec < t.tv_usec);}
	int operator<= (const cTime &t) const{if(tv_sec < t.tv_sec) return 1; if(tv_sec > t.tv_sec) return 0; return (tv_usec <= t.tv_usec);}
	int operator== (const cTime &t) const{return ((tv_usec == t.tv_usec) && (tv_sec == t.tv_sec));}
	//int & operator/ (const cTime &t){long sec = tv_sec / i; long usec = tv_usec + 1000000 * (tv_sec % i); usec /= i; return cTime(sec, usec).Normalize();}
	cTime & operator= (const cTime &t){tv_usec = t.tv_usec; tv_sec = t.tv_sec; return *this;}
	cTime   operator+ (const cTime &t) const {long sec = tv_sec + t.tv_sec; long usec = tv_usec + t.tv_usec; return cTime(sec, usec).Normalize();}
	cTime   operator- (const cTime &t) const {long sec = tv_sec - t.tv_sec; long usec = tv_usec - t.tv_usec; return cTime(sec, usec).Normalize();}
	cTime   operator+ (int msec) const {long _usec = tv_usec + msec * 1000; return cTime(tv_sec, _usec).Normalize();}
	cTime   operator- (int sec) const {long _sec = tv_sec - sec; return cTime(_sec, tv_usec).Normalize();}
	cTime & operator+= (const cTime &t){tv_sec += t.tv_sec; tv_usec += t.tv_usec; Normalize(); return *this;}
	cTime & operator-= (const cTime &t){tv_sec -= t.tv_sec; tv_usec -= t.tv_usec; Normalize(); return *this;}
	cTime & operator-= (int sec){tv_sec -= sec; Normalize(); return *this;}
	cTime & operator+= (int msec){tv_usec += 1000 * msec; Normalize(); return *this;}
	cTime & operator+= (long usec){tv_usec += usec; Normalize(); return *this;}
	cTime & operator/= (int i){long sec = tv_sec / i; tv_usec += 1000000 * (tv_sec % i); tv_usec /= i; tv_sec = sec; Normalize(); return *this;}
	cTime & operator*= (int i){tv_sec *= i; tv_usec *= i; Normalize(); return *this;}
	cTime   operator/  (int i) const {long sec = tv_sec / i; long usec = tv_usec + 1000000 * (tv_sec % i); usec /= i; return cTime(sec, usec).Normalize();}
	cTime   operator*  (int i) const {long sec = tv_sec * i; long usec = tv_usec * i; return cTime(sec, usec).Normalize();}
	operator double(){return double(tv_sec) + double(tv_usec) / 1000000.;}
	operator long()  {return long(tv_sec) * 1000000 + long(tv_usec);}
	operator int()   {return int(tv_sec * 1000 + double(tv_usec) / 1000.);}
	operator bool()  {return !(!tv_sec && !tv_usec);}
	int operator! () {return !tv_sec && !tv_usec;}

	/** Get seconds */
	long Sec() const {return tv_sec;}

	/** Get milisec */
	unsigned long MiliSec() const { return (unsigned long)(tv_sec) * 1000 + (unsigned long)(tv_usec) / 1000; }

	/*bool LocalTime(struct tm &result){ return localtime_r(this, &result) == &result;}*/
	cTime &Get();
	cTime &Normalize();
	void Null() { tv_sec = tv_usec = 0; }
	string AsString() const;
	void AsTimeVals(int &w, int &d, int &h, int &m) const;
	friend std::ostream &operator << (std::ostream &os, const cTime &t);

private:
	/** print-type of the time */
	mutable int mPrintType;

public:
	const cTime &AsDate() const { mPrintType = 1; return *this; }
	const cTime &AsPeriod() const { mPrintType = 2; return *this; }
	const cTime &AsFullPeriod() const { mPrintType = 3; return *this; }
	const cTime &AsDateMS() const { mPrintType = 4; return *this; }

}; // cTime

}; // nUtils

#endif // CTIME_H
