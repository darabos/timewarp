
#include "games/ggob.h"
#include "gdialog.h"
#include "melee.h"
REGISTER_FILE
#include "melee/mship.h"

static GobPlayer * g_player = NULL;
#define gobgame ((GobGame*)game)

#define NOT_IMPLEMENTED 0

Quest::Quest( const char * szLuaFile, GobPlayer * player )
{
  gob_player = player;
  
  bExist = true;

  L = lua_open();
  InitConversationModule( L );

  // Register C function
  lua_register(L, "Dialog", l_Dialog);
  lua_register(L, "AddObject", l_AddObject);
  lua_register(L, "RemoveObject", l_RemoveObject);
  // Load Quest
  lua_dofile(L, szLuaFile);
  // Register Events

  RegisterEvent(GAME_EVENT_SHIP_DIE, gobgame);
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
	// just to speed up game
	if( random()%50 )
		return;

  // dirty hack, only for experiment
  g_player = gob_player;
  

  int top = lua_gettop(L);
  lua_pushstring(L, "Process");
  lua_gettable(L, LUA_GLOBALSINDEX);

  if ( !lua_isfunction(L, -1) )
    {
      tw_error("Quest script is not contain Process function");
    };

  // gather information
  int time = gobgame->game_time;
  int enemy = gobgame->gobenemies;
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

void Quest::ProcessEvent ( IEvent* event )
{
  int type = event->GetEventType();
  int top;
  switch ( type )
    {
    case GAME_EVENT_ALL:
      {tw_error ("Process Event failed");}
      break;
    case GAME_EVENT_SHIP_DIE:
		// Test Implementation
		top = lua_gettop(L);
		lua_pushstring(L, "GAME_EVENT_SHIP_DIE");
		lua_gettable(L, LUA_GLOBALSINDEX);


		if ( !lua_isfunction(L, -1) )
		{
			tw_error("Quest script is not contain GAME_EVENT_SHIP_DIE function");
		};
		lua_call(L, 0, 0 );
		lua_settop(L, top);
      break;
    case GAME_EVENT_SHIP_GET_DAMAGE:
      break;
    default:
      break;
    }
}

int Quest::l_Dialog(lua_State* ls)
{
  return 0;
}

int Quest::l_AddObject(lua_State* ls)
{
  gobgame->add_new_enemy();
  return 0;
}

int Quest::l_RemoveObject(lua_State* ls)
{
  return 0;
}

int QuestSource::LoadQuestList ( const char* qlist )
{
  return NOT_IMPLEMENTED;
}

StarBaseQuestSource::StarBaseQuestSource( const char* qlist )
{
  LoadQuestList( qlist );
}

StarBaseQuestSource::~StarBaseQuestSource()
{
}

int StarBaseQuestSource::GetNextQuest(Quest* q, GobPlayer* p)
{
  return NOT_IMPLEMENTED;
}

int StarBaseQuestSource::QuestSuccess(Quest* q, GobPlayer* p)
{
  return NOT_IMPLEMENTED;
}

int StarBaseQuestSource::QuestFailed(Quest* q, GobPlayer* p)
{
  return NOT_IMPLEMENTED;
}

int StarBaseQuestSource::WhenMeet(GobPlayer* p)
{
  return NOT_IMPLEMENTED;
}

