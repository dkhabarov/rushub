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

#include "TaskList.h"
#include "LuaPlugin.h"

namespace luaplugin {



/** Do tasks */
void Tasks(void * value) {
	Task * task = static_cast<Task *> (value);
	LuaInterpreter * param = static_cast<LuaInterpreter *> (task->mParam);
	switch (task->mType) {
		case TASKTYPE_RESTARTSCRIPT :
			LuaPlugin::mCurLua->restartScript(param);
			break;

		case TASKTYPE_STOPSCRIPT :
			LuaPlugin::mCurLua->stopScript(param);
			break;

		case TASKTYPE_MOVEUP :
			LuaPlugin::mCurLua->moveUp(param);
			break;

		case TASKTYPE_MOVEDOWN :
			LuaPlugin::mCurLua->moveDown(param);
			break;

		case TASKTYPE_SCRIPTERROR :

		default :
			break;

	}
}



void TasksList::commonTasks() {
	if (mTackChecker & COMMONTASK_SAVE) {
		LuaPlugin::mCurLua->save();
	}
	mTackChecker = 0;
}



/** Add task */
void TasksList::addTask(void * param, TaskType type) {
	if (!param) {
		mTackChecker = mTackChecker | (1 << type); /** Common task */
	} else {
		Task * task = new Task(type);
		task->mParam = param;
		add(task);
	}
}



void TasksList::checkTasks() {
	loop(Tasks);
	clear();
	if (mTackChecker) {
		commonTasks();
	}
}



void TasksList::onRemove(Task * task) {
	delete task;
}


} // namespace luaplugin

/**
 * $Id$
 * $HeadURL$
 */
