/*
 * RusHub - hub server for Direct Connect peer to peer network.
 *
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

#ifndef TASKS_LIST_H
#define TASKS_LIST_H

#include "List.h"

namespace luaplugin {

enum {
	eTB_Save           = 1 << 1,
}; /** TasksBytes */

typedef enum {
	eT_No = 0,
	eT_Save,
	eT_RestartScript,
	eT_StopScript,
	eT_MoveUp,
	eT_MoveDown,
	eT_ScriptError
} tTask; /** tTask */


struct Task {

	Task():mParam(NULL) {
	}

	Task(tTask iType) : mType(iType), mParam(NULL) {
	}

	~Task() {
	}

	tTask mType;

	void * mParam;

}; // Task



class TasksList : public List<Task *> {

public:

	TasksList() : miTackChecker(0) {
	}

	~TasksList() {
	}

	void AddTask(void * Param, tTask iType = eT_No); /** Adding tasks */

	void CheckTasks();

	virtual void onRemove(Task *);

private:

	int miTackChecker; /** Bits of the common tasks */

private:

	void CommonTasks(); /** Common tasks */

}; // class TasksList

}; // namespace luaplugin

#endif // TASKS_LIST_H

/**
 * $Id$
 * $HeadURL$
 */
