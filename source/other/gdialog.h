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
int l_DialogWrite(lua_State* ls);
int l_DialogAnswer(lua_State* ls);
int l_DialogEnd ( lua_State* ls );

#endif // __G_DIALOG_H__
