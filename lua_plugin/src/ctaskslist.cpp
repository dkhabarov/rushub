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

#include "ctaskslist.h"
#include "clua.h"

namespace nLua {

/** Do tasks */
void Tasks(void * val) {
  cTasksList::cTask * task = (cTasksList::cTask *) val;
  cLuaInterpreter * Param = (cLuaInterpreter *)(task->mParam);
  switch(task->miType) {
    case eT_RestartScript:
      cLua::mCurLua->RestartScript(Param); break;
    case eT_StopScript:
      cLua::mCurLua->StopScript(Param); break;
    case eT_MoveUp:
      cLua::mCurLua->MoveUp(Param); break;
    case eT_MoveDown:
      cLua::mCurLua->MoveDown(Param); break;
    case eT_ScriptError:
    default: break;
  }
}

void cTasksList::CommonTasks() {
  if(miTackChecker & eTB_Save) cLua::mCurLua->Save();
  miTackChecker = 0;
}

/** Add task */
void cTasksList::AddTask(void * Param, tTask iType) {
  if(!Param) miTackChecker = miTackChecker | (1 << iType); /** Common task */
  else {
    cTask * Task = new cTask(iType);
    Task->mParam = Param;
    mList.Add(Task);
  }
}

void cTasksList::CheckTasks() {
  mList.Loop(Tasks);
  mList.Clear();
  if(miTackChecker) CommonTasks();
}

}; // nLua
