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

#if (defined _DEBUG) && (!defined UNITTESTS_OFF)


#include "UnitTest++.h"
#include "List.h"


using namespace luaplugin;


SUITE(TestList) {


TEST(TestList_add) {
	int a = 5;
	int b = 7;
	int c = 5;

	List<int> list;

	list.add(a);
	list.add(b);
	list.add(c);

	CHECK_EQUAL(3, list.size());
}


TEST(TestList_clear) {
	
	List<int> list;

	int a = 1;
	list.add(a);

	list.clear();

	CHECK_EQUAL(0, list.size());
}



bool func(int i) {
	return i == 1;
}

void func1(int) {
	//return false;
}

TEST(TestList_removeIf) {

	List<int> list;

	int a = 1;
	int b = 2;
	int c = 1;
	list.add(a);
	list.add(b);
	list.add(c);

	list.removeIf(func);
	list.loop(func1);

	CHECK_EQUAL(1, list.size());
}


}; // SUITE(TestList)

#endif // _DEBUG

/**
 * $Id$
 * $HeadURL$
 */
