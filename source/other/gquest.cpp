
#include "games/ggob.h"
#include "gdialog.h"
#include "melee.h"
REGISTER_FILE
#include "melee/mship.h"

static GobPlayer * g_player = NULL;
static GobGame * g_game = NULL;

Quest::Quest( const char * szLuaFile, GobPlayer * player, GobGame * game )
{
  gob_player = player;
  gob_game = game;
  
  bExist = true;

  L = lua_open();
  InitConversationModule( L, game );

  // Register C function
  lua_register(L, "Dialog", l_Dialog);
  lua_register(L, "AddObject", l_AddObject);
  lua_register(L, "RemoveObject", l_RemoveObject);
  // Load Quest
  lua_dofile(L, szLuaFile);
}

Quest::~Quest()
{
  lua_close(L);
}

bool Quest::exist() 
{
  return bExist;
}

void Quest::Process()
{
  g_player = gob_player;
  g_game = gob_game;

  int top = lua_gettop(L);
  lua_pushstring(L, "Process");
  lua_gettable(L, LUA_GLOBALSINDEX);

  if ( !lua_isfunction(L, -1) )
    {
      tw_error("Quest script is not contain Process function");
    };

  // gather information
  int time = gob_game->game_time;
  int enemy = gob_game->gobenemies;
  int kills = gob_player->kills;
  int x = iround(gob_player->ship->normal_pos().x);
  int y = iround(gob_player->ship->normal_pos().y);

  // Pass addition information to lua script 
  lua_pushnumber(L, time);
  lua_pushnumber(L, enemy);
  lua_pushnumber(L, kills);
  lua_pushnumber(L, x);
  lua_pushnumber(L, y);
  lua_pushnumber(L, 0); //reserved
  lua_pushnumber(L, 0); //reserved
  lua_pushstring(L, "test"); //reserved

  lua_call(L, 8, 0);
  
  lua_pushstring(L, "Complited");
  lua_gettable(L, LUA_GLOBALSINDEX);
  if ( lua_isnumber(L, -1) )
    {
      bExist = lua_tonumber(L, -1);
    }
  else
    {
      tw_error("Unable to read Complited variable from lua script");
    }
  lua_settop(L, top);
  return;
}

int Quest::l_Dialog(lua_State* ls)
{
  return 0;
}

int Quest::l_AddObject(lua_State* ls)
{
  g_game->add_new_enemy();
  return 0;
}

int Quest::l_RemoveObject(lua_State* ls)
{
  return 0;
}
