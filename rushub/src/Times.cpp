/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by dan at verliba dot cziel Muller
 * E-Mail: dan at verliba dot cz@verliba.cz
 *
 * modified: 27 Aug 2009
 * Copyright (C) 2009-2011 by Setuper
 * E-Mail: setuper at gmail dot com (setuper@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Times.h"

#include <sstream>
#include <string.h> // strlen
#include <time.h> // ctime_s


#ifdef _WIN32
	#ifndef __int64
		#define __int64 long long
	#endif
	void gettimeofday(struct timeval * tv, struct timezone *) {
		union {
			FILETIME ft;
			unsigned __int64 ns100;
		} now;
		GetSystemTimeAsFileTime(&now.ft);
		//116444736000000000 = (24 * 3600) * ((1970 - 1601) * 365 + 89) * (1000000000 / 100)
		tv->tv_sec = (long) ((now.ns100 - 116444736000000000LL) / 10000000LL);
		tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL);
	}
#endif // _WIN32

namespace utils {


Time::Time() : mPrintType(0) {
	tv_sec = tv_usec = 0l;
}



Time::Time(bool now) : mPrintType(0) {
	if (now) {
		get();
	} else {
		tv_sec = tv_usec = 0l;
	}
}



Time::Time(double sec) : mPrintType(0) {
	tv_sec = (long)sec;
	tv_usec = long((sec - tv_sec) * 1000000);
}



Time::Time(long sec, long usec) : mPrintType(0) {
	tv_sec = sec;
	tv_usec = usec;
}



Time::Time(const Time & t) : mPrintType(0) {
	tv_sec = t.tv_sec;
	tv_usec = t.tv_usec;
}



Time::~Time() {
}



int Time::operator > (const Time & t) const {
	return tv_sec > t.tv_sec ? 1 : (tv_sec < t.tv_sec ? 0 : tv_usec > t.tv_usec);
}



int Time::operator >= (const Time & t) const {
	return tv_sec > t.tv_sec ? 1 : (tv_sec < t.tv_sec ? 0 : tv_usec >= t.tv_usec);
}



int Time::operator < (const Time & t) const {
	return tv_sec < t.tv_sec ? 1 : (tv_sec > t.tv_sec ? 0 : tv_usec < t.tv_usec);
}



int Time::operator <= (const Time & t) const {
	return tv_sec < t.tv_sec ? 1 : (tv_sec > t.tv_sec ? 0 : tv_usec <= t.tv_usec);
}



int Time::operator == (const Time & t) const {
	return ((tv_usec == t.tv_usec) && (tv_sec == t.tv_sec));
}



Time & Time::operator = (const Time & t) {
	tv_usec = t.tv_usec;
	tv_sec = t.tv_sec;
	return *this;
}



Time Time::operator + (const Time & t) const {
	return Time(long(tv_sec + t.tv_sec), long(tv_usec + t.tv_usec)).normalize();
}



Time Time::operator - (const Time & t) const {
	return Time(long(tv_sec - t.tv_sec), long(tv_usec - t.tv_usec)).normalize();
}

Time Time::operator + (int msec) const {
	return Time(tv_sec, long(tv_usec + msec * 1000)).normalize();
}



Time Time::operator - (int sec) const {
	return Time(long(tv_sec - sec), tv_usec).normalize();
}



Time & Time::operator += (const Time & t) {
	tv_sec += t.tv_sec;
	tv_usec += t.tv_usec;
	normalize();
	return *this;
}



Time & Time::operator -= (const Time & t) {
	tv_sec -= t.tv_sec;
	tv_usec -= t.tv_usec;
	normalize();
	return *this;
}



Time & Time::operator -= (int sec) {
	tv_sec -= sec;
	normalize();
	return *this;
}



Time & Time::operator += (int msec) {
	tv_usec += 1000 * msec;
	normalize();
	return *this;
}



Time & Time::operator += (long usec) {
	tv_usec += usec;
	normalize();
	return *this;
}



Time & Time::operator /= (int i) {
	tv_usec += 1000000 * (tv_sec % i);
	tv_usec /= i;
	tv_sec = long(tv_sec / i);
	normalize();
	return *this;
}



Time & Time::operator *= (int i) {
	tv_sec *= i;
	tv_usec *= i;
	normalize();
	return *this;
}



Time Time::operator / (int i) const {
	return Time(long(tv_sec / i), (tv_usec + 1000000 * (tv_sec % i)) / i).normalize();
}



Time Time::operator * (int i) const {
	return Time(long(tv_sec * i), long(tv_usec * i)).normalize();
}



int Time::operator ! () const {
	return !tv_sec && !tv_usec;
}



Time::operator bool() const {
	return !(!tv_sec && !tv_usec);
}



Time::operator double() const {
	return double(tv_sec) + double(tv_usec) / 1000000.;
}



Time::operator __int64() const {
	if (tv_sec > 0) {
		if (tv_usec > 0) {
			return (__int64)(tv_sec) * 1000 + (__int64)(tv_usec) / 1000;
		} else {
			return (__int64)(tv_sec) * 1000 + (__int64)(-tv_usec) / 1000;
		}
	} else {
		if (tv_usec > 0) {
			return (__int64)(-tv_sec) * 1000 + (__int64)(tv_usec) / 1000;
		} else {
			return (__int64)(-tv_sec) * 1000 + (__int64)(-tv_usec) / 1000;
		}
	}
}



Time & Time::get() {
	gettimeofday(this, NULL);
	return *this;
}



void Time::asTimeVals(int & w, int & d, int & h, int & m) const {
	long rest = tv_sec;
	w = rest / (24 * 3600 * 7);
	rest %= (24 * 3600 * 7);
	d = rest / (24 * 3600);
	rest %= (24 * 3600);
	h = rest / 3600;
	rest %= 3600;
	m = rest / 60;
}



std::ostream & operator << (std::ostream & os, const Time & t) {
	#if defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER >= 1400)
		char buf[26] = { '\0' };
	#elif defined(_WIN32)
		char * buf;
	#else
		char * buf;
		char buff[26] = { '\0' };
	#endif
	long n, rest;

	switch (t.mPrintType) {

		case 4 : // asDateMsec
		case 1 : // asDate
			#ifdef _WIN32
				time_t ta;
				ta = (time_t)t.tv_sec;
				#if defined(_MSC_VER) && (_MSC_VER >= 1400)
					ctime_s(buf, 26, &ta);
				#else
					buf = ctime(&ta);
				#endif
			#else
				const time_t * ta;
				ta = (time_t*)&t.tv_sec;
				buf = ctime_r(ta, buff);
			#endif
			buf[strlen(buf) - 1] = 0;
			os << buf;
			if (t.mPrintType == 4) {
				os << "|" << t.tv_usec / 1000;
			}
			break;

		case 2 : // asPeriod
			rest = t.tv_sec;
			os << rest << " sec ";
			os << t.tv_usec / 1000 << " ms ";
			os << t.tv_usec % 1000 << " µs ";
			break;

		case 3 : // asFullPeriod
			rest = t.tv_sec;

			n = rest / (24 * 3600 * 7);
			rest %= (24 * 3600 * 7);
			if (n) {
				os << n << " weeks ";
			}

			n = rest / (24 * 3600);
			rest %= (24 * 3600);
			if (n) {
				os << n << " days ";
			}

			n = rest / 3600;
			rest %= 3600;
			if (n) {
				os << n << " hours ";
			}

			n = rest / 60;
			if (n) {
				os << n << " min ";
			}
			rest %= 60;
			os << rest << " sec";
			break;

		default : // asString
			os << t.tv_sec << " s " << t.tv_usec << " µs";
			break;

	}
	return os;
}


const Time & Time::asDate() const {
	mPrintType = 1;
	return *this;
}



const Time & Time::asPeriod() const {
	mPrintType = 2;
	return *this;
}



const Time & Time::asFullPeriod() const {
	mPrintType = 3;
	return *this;
}



const Time & Time::asDateMsec() const {
	mPrintType = 4;
	return *this;
}



Time & Time::normalize() {
	if (tv_usec >= 1000000 || tv_usec <= -1000000) {
		tv_sec += tv_usec / 1000000;
		tv_usec %= 1000000;
	}
	if (tv_sec < 0 && tv_usec > 0) {
		++tv_sec;
		tv_usec -= 1000000;
	}
	if (tv_sec > 0 && tv_usec < 0) {
		--tv_sec;
		tv_usec += 1000000;
	}
	return *this;
}


}; // namespace utils

/**
 * $Id$
 * $HeadURL$
 */
