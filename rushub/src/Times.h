/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
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

#ifndef TIMES_H
#define TIMES_H

#include <ostream>

#ifdef _WIN32
	#include <winsock2.h> // for class timeval
	void gettimeofday(struct timeval *, struct timezone *);
#else
	#ifndef __int64
		#define __int64 long long
	#endif
	#include <sys/time.h> // for gettimeofday
#endif

namespace utils {

/** Class of time with microsecond decision and arithmetical operation */
class Time : public timeval {

public:

	Time();
	Time(bool now);
	Time(double sec);
	Time(long sec, long usec = 0);
	Time(const Time &);
	~Time();
	
	int operator > (const Time &) const;
	int operator >= (const Time &) const;
	int operator < (const Time &) const;
	int operator <= (const Time &) const;
	int operator == (const Time &) const;
	Time & operator = (const Time &);
	Time operator + (const Time &) const;
	Time operator - (const Time &) const;
	Time operator + (int msec) const;
	Time operator - (int sec) const;
	Time & operator += (const Time &);
	Time & operator -= (const Time &);
	Time & operator -= (int sec);
	Time & operator += (int msec);
	Time & operator += (long usec);

	int operator ! () const;
	operator bool() const;
	operator double() const;
	operator __int64() const;

	/// Get seconds
	inline long sec() const {
		return tv_sec;
	}

	/// Get milisec
	inline __int64 msec() const {
		return __int64(*this);
	}

	Time & get();
	void asTimeVals(int & w, int & d, int & h, int & m) const;
	friend std::ostream & operator << (std::ostream & os, const Time & t);

	const Time & asDate() const;
	const Time & asPeriod() const;
	const Time & asFullPeriod() const;
	const Time & asDateMsec() const;

private:

	/** print-type of the time */
	mutable int mPrintType;

private:

	Time & normalize();

}; // Time

}; // namespace utils

#endif // TIMES_H

/**
 * $Id$
 * $HeadURL$
 */
