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

#ifndef TASKS_LIST_H
#define TASKS_LIST_H

#include "List.h"

namespace luaplugin {

enum {
	COMMONTASK_SAVE           = 1 << 1,
}; /** CommonTask */

typedef enum {
	TASKTYPE_No = 0,
	TASKTYPE_SAVE,
	TASKTYPE_RESTARTSCRIPT,
	TASKTYPE_STOPSCRIPT,
	TASKTYPE_MOVEUP,
	TASKTYPE_MOVEDOWN,
	TASKTYPE_SCRIPTERROR
} TaskType; /** TaskType */


struct Task {

	Task() : mParam(NULL) {
	}

	Task(TaskType type) : mType(type), mParam(NULL) {
	}

	~Task() {
	}

	TaskType mType;

	void * mParam;

}; // Task



class TasksList : public List<Task *> {

public:

	TasksList() : mTackChecker(0) {
	}

	virtual ~TasksList() {
		clear(); // clear before destruct
	}

	void addTask(void * param, TaskType type = TASKTYPE_No); /** Adding tasks */

	void checkTasks();

	virtual void onRemove(Task *);

private:

	int mTackChecker; /** Bits of the common tasks */

private:

	void commonTasks(); /** Common tasks */

}; // class TasksList

}; // namespace luaplugin

#endif // TASKS_LIST_H

/**
 * $Id$
 * $HeadURL$
 */
