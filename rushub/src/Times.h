/*
 * RusHub - hub server for Direct Connect peer to peer network.

 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz

 * modified: 27 Aug 2009
 * Copyright (C) 2009-2011 by Setuper
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

#ifndef TIMES_H
#define TIMES_H

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
	#include "conndefine.h" // for class timeval
	void gettimeofday(struct timeval*, struct timezone*);
#else
	#include <sys/time.h> // for gettimeofday
#endif

namespace utils {

/** Class of time with microsecond decision and arithmetical operation */
class Time : public timeval {
public:
	~Time(){}
	Time(): mPrintType(0){Get();}
	Time(double sec): mPrintType(0){tv_sec = (long)sec; tv_usec = long((sec - tv_sec) * 1000000);};
	Time(long sec, long usec = 0): mPrintType(0){tv_sec = sec; tv_usec = usec;};
	Time(const Time &t) : mPrintType(0){tv_sec = t.tv_sec; tv_usec = t.tv_usec;};

	int operator>  (const Time &t) const{if(tv_sec > t.tv_sec) return 1; if(tv_sec < t.tv_sec) return 0; return (tv_usec > t.tv_usec);}
	int operator>= (const Time &t) const{if(tv_sec > t.tv_sec) return 1; if(tv_sec < t.tv_sec) return 0; return (tv_usec >= t.tv_usec);}
	int operator<  (const Time &t) const{if(tv_sec < t.tv_sec) return 1; if(tv_sec > t.tv_sec) return 0; return (tv_usec < t.tv_usec);}
	int operator<= (const Time &t) const{if(tv_sec < t.tv_sec) return 1; if(tv_sec > t.tv_sec) return 0; return (tv_usec <= t.tv_usec);}
	int operator== (const Time &t) const{return ((tv_usec == t.tv_usec) && (tv_sec == t.tv_sec));}
	//int & operator/ (const Time &t){long sec = tv_sec / i; long usec = tv_usec + 1000000 * (tv_sec % i); usec /= i; return Time(sec, usec).Normalize();}
	Time & operator= (const Time &t){tv_usec = t.tv_usec; tv_sec = t.tv_sec; return *this;}
	Time   operator+ (const Time &t) const {long sec = tv_sec + t.tv_sec; long usec = tv_usec + t.tv_usec; return Time(sec, usec).Normalize();}
	Time   operator- (const Time &t) const {long sec = tv_sec - t.tv_sec; long usec = tv_usec - t.tv_usec; return Time(sec, usec).Normalize();}
	Time   operator+ (int msec) const {long _usec = tv_usec + msec * 1000; return Time(tv_sec, _usec).Normalize();}
	Time   operator- (int sec) const {long _sec = tv_sec - sec; return Time(_sec, tv_usec).Normalize();}
	Time & operator+= (const Time &t){tv_sec += t.tv_sec; tv_usec += t.tv_usec; Normalize(); return *this;}
	Time & operator-= (const Time &t){tv_sec -= t.tv_sec; tv_usec -= t.tv_usec; Normalize(); return *this;}
	Time & operator-= (int sec){tv_sec -= sec; Normalize(); return *this;}
	Time & operator+= (int msec){tv_usec += 1000 * msec; Normalize(); return *this;}
	Time & operator+= (long usec){tv_usec += usec; Normalize(); return *this;}
	Time & operator/= (int i){long sec = tv_sec / i; tv_usec += 1000000 * (tv_sec % i); tv_usec /= i; tv_sec = sec; Normalize(); return *this;}
	Time & operator*= (int i){tv_sec *= i; tv_usec *= i; Normalize(); return *this;}
	Time   operator/  (int i) const {long sec = tv_sec / i; long usec = tv_usec + 1000000 * (tv_sec % i); usec /= i; return Time(sec, usec).Normalize();}
	Time   operator*  (int i) const {long sec = tv_sec * i; long usec = tv_usec * i; return Time(sec, usec).Normalize();}
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
	Time &Get();
	Time &Normalize();
	void Null() { tv_sec = tv_usec = 0; }
	string AsString() const;
	void AsTimeVals(int &w, int &d, int &h, int &m) const;
	friend std::ostream &operator << (std::ostream &os, const Time &t);

private:
	/** print-type of the time */
	mutable int mPrintType;

public:
	const Time &AsDate() const { mPrintType = 1; return *this; }
	const Time &AsPeriod() const { mPrintType = 2; return *this; }
	const Time &AsFullPeriod() const { mPrintType = 3; return *this; }
	const Time &AsDateMS() const { mPrintType = 4; return *this; }

}; // Time

}; // namespace utils

#endif // TIMES_H
