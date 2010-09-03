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

#ifndef CMEANFREQUENCY_H
#define CMEANFREQUENCY_H

#include "ctime.h" /** cTime */
#include <iostream>

using namespace std;

namespace nUtils {
/**
  Template of the determination of the average frequency
*/

template<class T, int max_size = 20> class cMeanFrequency
{
  public:
  cTime mOverPeriod; /** Value of the period, in current which is considered frequency */
  cTime mPeriodPart; /** Value of each part of period */
  cTime mStart, mEnd; /** Initial and final time of the period, in current which is considered frequency */
  cTime mPart; /** Begin (End) of the current part of period */
  int mResolution; /** Number of the parts of the period */
  T mCounts[max_size]; /** Array with amount of the hits in that or other part of the period */
  int mStartIdx; /** Current index to positions */
  int mNumFill; /** Amount filled period */

  void Dump(void) {
    cout << "mOverPeriod: " << mOverPeriod.AsPeriod() << endl
      << " mStart, mEnd: " << mStart.AsDate() << ", " << mEnd.AsDate() << endl
      << " mPart, mPeriodPart: " << mPart.AsDate() << ", " << mPeriodPart.AsPeriod() << endl
      << " mResolution:" << mResolution << endl
      << " mCounts[" ;
    for(int i = 0; i < max_size; ++i) cout << mCounts[i] << ", ";
    cout << "] " << endl << "mStartIdx:" << mStartIdx << ", mNumFill:" << mNumFill << endl << endl;
  }

  cMeanFrequency() {
    cTime now;
    mResolution = max_size;
    mOverPeriod = cTime(0.);
    mPeriodPart = 0;
    Reset(now);
  }

  cMeanFrequency(const cTime &now) {
    mResolution = max_size;
    SetPeriod(1.);
    Reset(now);
  };

  cMeanFrequency(const cTime &now, double per, int res):
    mOverPeriod(per),
    mPeriodPart(per/res),
    mResolution(res)
  {
    Reset(now);
  };

  void Insert(const cTime &now, T data = 1) {
    Adjust(now);
    mCounts[(mStartIdx + mNumFill) % mResolution] += data;
  };

  double GetMean(const cTime &now) {
    double Sum = CountAll(now);
    if(!mNumFill) return 0.;
    Sum *= mResolution / mNumFill;
    Sum /= double(mOverPeriod);
    return Sum;
  }

  T CountAll(const cTime &now) {
    Adjust(now);
    T sum = 0;
    int i, j = mStartIdx;
    for(i = 0; i < mNumFill; ++i)
    {
      sum += mCounts[j++];
      if(j >= mResolution) j = 0;
    }
    return sum;
  };

  void Adjust(const cTime &now) {
    if(mEnd < now) {
      cTime t(mEnd);
      t += mOverPeriod;
      if(t < now){ Reset(now); return; }
      while(mEnd < now) Shift();
    } else if(mNumFill < mResolution) {
      while((mPart < now) && (mNumFill < mResolution)) {
        mPart += mPeriodPart;
        ++mNumFill;
      }
    }
  };

  void Shift() {
    mEnd   += mPeriodPart;
    mStart += mPeriodPart;
    mCounts[mStartIdx] = 0;
    if(mNumFill > 0) --mNumFill;
    ++mStartIdx;
    if(mStartIdx >= mResolution) mStartIdx -= mResolution;
  };

  void Reset(const cTime &now) {
    memset(&mCounts, 0, sizeof(mCounts));
    mStart = now;

    mEnd   = mStart;
    mEnd  += mOverPeriod;

    mPart  = mStart;
    mPart += mPeriodPart;

    mNumFill  = 0;
    mStartIdx = 0;
  }

  void SetPeriod(double per) {
    mOverPeriod = cTime(per);
    mPeriodPart = mOverPeriod / mResolution;
  };

}; // cMeanFrequency

}; // nUtils

#endif // CMEANFREQUENCY_H
