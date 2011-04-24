/*
 * RusHub - hub server for Direct Connect peer to peer network.

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

#include "TaskList.h"
#include "LuaPlugin.h"

namespace luaplugin {

/** Do tasks */
void Tasks(void * val) {
	TasksList::cTask * task = (TasksList::cTask *) val;
	LuaInterpreter * Param = (LuaInterpreter *) (task->mParam);
	switch (task->mType) {
		case eT_RestartScript :
			LuaPlugin::mCurLua->RestartScript(Param);
			break;

		case eT_StopScript :
			LuaPlugin::mCurLua->StopScript(Param);
			break;

		case eT_MoveUp :
			LuaPlugin::mCurLua->MoveUp(Param);
			break;

		case eT_MoveDown :
			LuaPlugin::mCurLua->MoveDown(Param);
			break;

		case eT_ScriptError :

		default :
			break;

	}
}

void TasksList::CommonTasks() {
	if (miTackChecker & eTB_Save) {
		LuaPlugin::mCurLua->save();
	}
	miTackChecker = 0;
}

/** Add task */
void TasksList::AddTask(void * Param, tTask iType) {
	if (!Param) {
		miTackChecker = miTackChecker | (1 << iType); /** Common task */
	} else {
		cTask * Task = new cTask(iType);
		Task->mParam = Param;
		mList.add(Task);
	}
}

void TasksList::CheckTasks() {
	mList.Loop(Tasks);
	mList.clear();
	if (miTackChecker) {
		CommonTasks();
	}
}

}; // namespace luaplugin
