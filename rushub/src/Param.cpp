/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
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

#include "Param.h"
#include "DcUser.h"

namespace dcserver {


Param::Param(DcUser * dcUser, const char * name) : 
	mName(name),
	mType(TYPE_NONE),
	mMode(MODE_NONE),
	mDcUser(dcUser),
	mAction(NULL)
{
}



Param::~Param() {
}



const string & Param::getName() const {
	return mName;
}



int Param::getType() const {
	return mType;
}



int Param::getMode() const {
	return mMode;
}



const string & Param::getString() const {
	return mValue;
}



int Param::setString(const string & value) {
	return set(value, TYPE_STRING);
}



int Param::setString(string * value) {
	return set(value, TYPE_STRING);
}



const int & Param::getInt() const {
	return mValue;
}



int Param::setInt(int value) {
	return set(value, TYPE_INT);
}



int Param::setInt(int * value) {
	return set(value, TYPE_INT);
}



const bool & Param::getBool() const {
	return mValue;
}



int Param::setBool(bool value) {
	return set(value, TYPE_BOOL);
}



int Param::setBool(bool * value) {
	return set(value, TYPE_BOOL);
}



const double & Param::getDouble() const {
	return mValue;
}



int Param::setDouble(double value) {
	return set(value, TYPE_DOUBLE);
}



int Param::setDouble(double * value) {
	return set(value, TYPE_DOUBLE);
}



const long & Param::getLong() const {
	return mValue;
}



int Param::setLong(long value) {
	return set(value, TYPE_LONG);
}



int Param::setLong(long * value) {
	return set(value, TYPE_LONG);
}



const __int64 & Param::getInt64() const {
	return mValue;
}



int Param::setInt64(__int64 value) {
	return set(value, TYPE_INT64);
}



int Param::setInt64(__int64 * value) {
	return set(value, TYPE_INT64);
}



const string & Param::toString() {
	return utils::toString(mValue, mBuf);
}


}; // namespace dcserver

/**
 * $Id$
 * $HeadURL$
 */
