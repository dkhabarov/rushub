/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
 * begin: Wed Jun 11 2003
 * Copyright (C) 2003 by Daniel Muller
 * E-Mail: dan at verliba dot cz
 *
 * modified: 27 Aug 2009
 * Copyright (C) 2009-2012 by Setuper
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

#ifndef MEAN_FREQUENCY_H
#define MEAN_FREQUENCY_H

#include "Times.h" // Time

#include <iostream>

using namespace ::std;

namespace utils {

/**
	Template of the determination of the average frequency
*/
template<class T, int max_size = 20>
class MeanFrequency {

public:

	Time mOverPeriod; ///< Value of the period, in current which is considered frequency
	Time mPeriodPart; ///< Value of each part of period
	Time mStart, mEnd; ///< Initial and final time of the period, in current which is considered frequency
	Time mPart; ///< Begin (End) of the current part of period
	int mResolution; ///< Number of the parts of the period
	T mCounts[max_size]; ///< Array with amount of the hits in that or other part of the period
	int mStartIdx; ///< Current index to positions
	int mNumFill; ///< Amount filled period

	MeanFrequency() {
		Time now(true);
		mResolution = max_size;
		mOverPeriod = Time(0.);
		mPeriodPart = 0;
		reset(now);
	}

	MeanFrequency(const Time & now) : mOverPeriod(true), mPeriodPart(true) {
		mResolution = max_size;
		setPeriod(1.);
		reset(now);
	};

	MeanFrequency(const Time & now, double per, int res):
		mOverPeriod(per),
		mPeriodPart(per/res),
		mResolution(res)
	{
		reset(now);
	};

	void insert(const Time & now, T data = 1) {
		adjust(now);
		mCounts[(mStartIdx + mNumFill) % mResolution] += data;
	};

	double getMean(const Time & now) {
		double Sum = countAll(now);
		if (!mNumFill) {
			return 0.;
		}
		Sum *= mResolution / mNumFill;
		Sum /= double(mOverPeriod);
		return Sum;
	}

	T countAll(const Time & now) {
		adjust(now);
		T sum = 0;
		int i, j = mStartIdx;
		for (i = 0; i < mNumFill; ++i) {
			sum += mCounts[j++];
			if (j >= mResolution) {
				j = 0;
			}
		}
		return sum;
	};

	void adjust(const Time & now) {
		if (mEnd < now) {
			Time t(mEnd);
			t += mOverPeriod;
			if (t < now) {
				reset(now);
				return;
			}
			while (mEnd < now) {
				shift();
			}
		} else if (mNumFill < mResolution) {
			while ((mPart < now) && (mNumFill < mResolution)) {
				mPart += mPeriodPart;
				++mNumFill;
			}
		}
	};

	void shift() {
		mEnd += mPeriodPart;
		mStart += mPeriodPart;
		mCounts[mStartIdx] = 0;
		if (mNumFill > 0) {
			--mNumFill;
		}
		++mStartIdx;
		if (mStartIdx >= mResolution) {
			mStartIdx -= mResolution;
		}
	};

	void reset(const Time &now) {
		memset(&mCounts, 0, sizeof(mCounts));
		mStart = now;

		mEnd = mStart;
		mEnd += mOverPeriod;

		mPart = mStart;
		mPart += mPeriodPart;

		mNumFill = 0;
		mStartIdx = 0;
	}

	void setPeriod(double per) {
		mOverPeriod = Time(per);
		mPeriodPart = mOverPeriod / mResolution;
	};

}; // class MeanFrequency

} // namespace utils

#endif // MEAN_FREQUENCY_H

/**
 * $Id$
 * $HeadURL$
 */
