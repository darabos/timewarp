/* $Id$ */ 
/*
This file is part of "Star Control: TimeWarp" 
                    http://timewarp.sourceforge.net/
Copyright (C) 2001-2004  TimeWarp development team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef __GDIALOG_H__
#define __GDIALOG_H__

extern "C" {
#	include "lua.h"
#	include "lualib.h"
#	include "lauxlib.h"
}


void InitConversationModule ( lua_State* L  );

int l_DialogStart(lua_State* ls);
int l_DialogSetAlienImage(lua_State* ls);
int l_DialogSetMusic(lua_State* ls);
int l_DialogWrite(lua_State* ls);
int l_KeyPressed (lua_State* ls);
int l_DialogAnswer(lua_State* ls);
int l_DialogEnd ( lua_State* ls );

#endif // __G_DIALOG_H__
