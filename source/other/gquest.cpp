#include <allegro.h>

#ifdef ALLEGRO_MSVC
	#pragma warning (disable:4786)
#endif

#include "../melee.h"
REGISTER_FILE
#include "../melee/mship.h"
#include "../games/ggob.h"
#include "gdialog.h"
#include "../melee/mview.h" // for message

static GobPlayer * g_player = NULL;
#define gobgame ((GobGame*)game)

#define NOT_IMPLEMENTED 300

Quest::Quest( const char * szLuaFile, GobPlayer * player )
{
  gob_player = player;
  strName = szLuaFile;

  bExist = true;

  L = lua_open();
  InitConversationModule( L );

  // Register C function
  lua_register(L, "Dialog",        l_Dialog);
  lua_register(L, "AddObject",     l_AddObject);
  lua_register(L, "RemoveObject",  l_RemoveObject);
  lua_register(L, "AddBuckazoids", l_AddBuckazoids);
  lua_register(L, "PrintMessage", l_PrintMessage);

  // Load Quest
  lua_dofile(L, szLuaFile);

  // Register Events
  int top = lua_gettop(L);

  lua_pushstring(L, "GAME_EVENT_SHIP_DIE");
  lua_gettable   ( L, LUA_GLOBALSINDEX );
  if ( lua_isfunction(L, -1) )
	RegisterEvent(GAME_EVENT_SHIP_DIE, gobgame);
  lua_settop(L, top);

  lua_pushstring(L, "GAME_EVENT_SHIP_GET_DAMAGE");
  lua_gettable   ( L, LUA_GLOBALSINDEX );
  if ( lua_isfunction(L, -1) )
	RegisterEvent(GAME_EVENT_SHIP_GET_DAMAGE, gobgame);
  lua_settop(L, top);

  lua_pushstring(L, "GAME_EVENT_ENTER_STATION");
  lua_gettable   ( L, LUA_GLOBALSINDEX );
  if ( lua_isfunction(L, -1) )
	RegisterEvent(GAME_EVENT_ENTER_STATION, gobgame);
  lua_settop(L, top);
}

Quest::~Quest()
{
  lua_close(L);
}

bool Quest::exist() 
{
  return bExist;
}

const char * Quest::GetName() const
{
	return strName.c_str();
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

  g_player = NULL;
  return;
}

void Quest::ProcessEvent ( IEvent* event )
{
  g_player = gob_player;
  int type = event->GetEventType();
  int top;
  const char * temp;
  switch ( type )
    {
    case GAME_EVENT_ALL:
      {tw_error ("Process Event failed");}
      break;
    case GAME_EVENT_SHIP_DIE:
      // Test Implementation
      top = lua_gettop(L);
      lua_pushstring ( L, "GAME_EVENT_SHIP_DIE" );
      lua_gettable   ( L, LUA_GLOBALSINDEX );
      
      
      if ( !lua_isfunction(L, -1) )
	{
	  tw_error("Quest script is not contain GAME_EVENT_SHIP_DIE function");
	};
      temp = ((EventShipDie*)event)->victim->get_shiptype() -> id;
      lua_pushstring( L, ((EventShipDie*)event)->victim->get_shiptype() -> id ); //reserved
      
      lua_call(L, 1, 0 );
      lua_settop(L, top);
      break;
    case GAME_EVENT_SHIP_GET_DAMAGE:
      break;
    case GAME_EVENT_ENTER_STATION:
      // Test Implementation
      top = lua_gettop(L);
      lua_pushstring ( L, "GAME_EVENT_ENTER_STATION" );
      lua_gettable   ( L, LUA_GLOBALSINDEX );
      
      if ( !lua_isfunction(L, -1) )
	{
	  tw_error("Quest script is not contain GAME_EVENT_ENTER_STATION function");
	};
      
      lua_pushnumber( L, 1 ); //need to be implemented
      
      lua_call(L, 1, 0 );
      lua_settop(L, top);
      
      break;
    default:
      break;
    }
  g_player = NULL;
}

int Quest::l_Dialog(lua_State* ls)
{
  return NOT_IMPLEMENTED;
}

int Quest::l_AddObject(lua_State* ls)
{
  gobgame->add_new_enemy();
  return NOT_IMPLEMENTED;
}

int Quest::l_RemoveObject(lua_State* ls)
{
  return NOT_IMPLEMENTED;
}
int Quest::l_AddBuckazoids(lua_State*ls)
{
	int top = lua_gettop(ls);
	if ( top != 1 )
	{
		tw_error ("Wrong argument count for AddBuckazoids");
	}
	int money = lua_tonumber(ls, -1);
	g_player->buckazoids += money;
	return 0;
}
int Quest::l_PrintMessage(lua_State*ls)
{
	int top = lua_gettop(ls);
	if ( top != 1 )
	{
		tw_error ("Wrong argument count for PrintMessage");
	}
    const char * str = lua_tostring(ls, -1);
    message.out((char *)str, 10000, makecol(0,255,0));
	return 0;
}

QuestSource::QuestSource()
{ 
  Lquest = lua_open();

  // Register C function
  //lua_register(L, "RemoveObject",  l_RemoveObject);

  RegisterEvent (GAME_EVENT_CLEANQUESTTRASH, gobgame); // can't generate from quest
  RegisterEvent (GAME_EVENT_QUESTSUCCESS, gobgame );
  RegisterEvent (GAME_EVENT_QUESTFAILED, gobgame );
  RegisterEvent (GAME_EVENT_ASKFORQUEST, gobgame );
}

QuestSource::~QuestSource()
{
  lua_close( Lquest );
}


int QuestSource::LoadQuestList ( const char* qlist )
{
  lua_dofile ( Lquest, qlist );

  int top  = lua_gettop ( Lquest );
  lua_pushstring ( Lquest, "QuestSuccess" );
  lua_gettable   ( Lquest, LUA_GLOBALSINDEX );
  
  if ( !lua_isfunction(Lquest, -1) )
    {
      tw_error("QuestSuccess failed");
    };
  lua_pushstring ( Lquest, "QuestFailed" );
  lua_gettable   ( Lquest, LUA_GLOBALSINDEX );
  
  if ( !lua_isfunction(Lquest, -1) )
    {
      tw_error("QuestSuccess failed");
    };
  lua_pushstring ( Lquest, "GetNextQuest" );
  lua_gettable   ( Lquest, LUA_GLOBALSINDEX );
  
  if ( !lua_isfunction(Lquest, -1) )
    {
      tw_error("GetNextQuest failed");
    };

  lua_pushstring ( Lquest, "GetQuest" );
  lua_gettable   ( Lquest, LUA_GLOBALSINDEX );
  
  if ( !lua_isfunction(Lquest, -1) )
    {
      tw_error("QuestSuccess failed");
    };
  return true;
}

Quest* QuestSource::GetNextQuest( GobPlayer* p)
{
  int top = lua_gettop(Lquest);
  lua_pushstring ( Lquest, "GetNextQuest" );
  lua_gettable   ( Lquest, LUA_GLOBALSINDEX );
  
  if ( !lua_isfunction(Lquest, -1) )
    {
      tw_error("GetNextQuest failed");
    };
  
  lua_call(Lquest, 0, 1 );
  
  if ( !lua_isstring(Lquest, -1) )
    {
      tw_error("Wrong argument for GetNextQuest");
    }
  
  Quest * q = NULL;
  const char * quest = lua_tostring(Lquest, -1);
  if ( !strcmp(quest, "NO_QUEST") )
    q = new Quest( quest, p );
  questList.push_back(q);

  lua_settop(Lquest, top);
		
  return q;
}

Quest* QuestSource::GetQuest ( const char * name, GobPlayer * player )
{
  ASSERT(name);
  Quest * q = new Quest( name, player );
  return q;
}

int QuestSource::QuestSuccess( Quest* q )
{
  int top = lua_gettop(Lquest);
  lua_pushstring ( Lquest, "QuestSuccess" );
  lua_gettable   ( Lquest, LUA_GLOBALSINDEX );
  
  if ( !lua_isfunction(Lquest, -1) )
    {
      tw_error("QuestSuccess failed");
    };

  lua_pushstring( Lquest, q->GetName() );
  
  lua_call(Lquest, 1, 0 );
  
  lua_settop(Lquest, top);
  
  return 1;
}

int QuestSource::QuestFailed( Quest* q )
{
int top = lua_gettop(Lquest);
  lua_pushstring ( Lquest, "QuestFailed" );
  lua_gettable   ( Lquest, LUA_GLOBALSINDEX );
  
  if ( !lua_isfunction(Lquest, -1) )
    {
      tw_error("QuestSuccess failed");
    };
  
  lua_pushstring( Lquest, q->GetName() );
  
  lua_call(Lquest, 1, 0 );
  
  lua_settop(Lquest, top);
		
  return 1;
}

void QuestSource::RemoveTrash()
{
  for (std::list<Quest*>::iterator i = questList.begin();
	 i != questList.end(); i++)
    {
      if ( !((*i)->exist()) )
	{
	  delete *i;
	  questList.erase(i);
	}
    }
}

void QuestSource::ProcessEvent ( IEvent* event )
{
  int type = event->GetEventType();
  switch ( type )
    {
    case GAME_EVENT_ALL:
      {tw_error ("Process Event failed");}
      break;

    case GAME_EVENT_CLEANQUESTTRASH:
      RemoveTrash();
      break;
    case GAME_EVENT_QUESTSUCCESS:
      QuestSuccess( ((EventQuestSuccess*)event)->quest );
      break;
    case GAME_EVENT_QUESTFAILED:
      QuestFailed( ((EventQuestFailed*)event)->quest );
      break;
    case GAME_EVENT_ASKFORQUEST:
      GetNextQuest( ((EventAskForQuest*)event)->player );
      break;
    default:
      break;
    }
}
