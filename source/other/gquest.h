/* $Id$ */ 
#ifndef __GQUEST_H__
#define __GQUEST_H__

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <string>
#include <list>

#include "gevent.h"


class GobPlayer;


class Quest: public EventListner
{
	bool bExist;
	lua_State * L;
	GobPlayer * gob_player;
	std::string strName;

public:
	Quest( const char * szLuaFile, GobPlayer * player );
	virtual ~Quest();
	/*! \brief Summon default quest handler
	This function summon in every player think()
	*/
	void Process();
	bool exist();
	const char * GetName() const;
	GobPlayer * GetPlayer();

	/*! \brief Process Event */
	virtual void  ProcessEvent( IEvent* event);

private:
	// This is callback functions from Lua quest script
	/*! \brief On complite quest */
	static int l_Dialog(lua_State* ls);
	/*! \brief Add quest object ( ship, starbase) to the game */
	static int l_AddObject(lua_State* ls);
	/*! \brief Remove quest object ( ship, starbase ... ) */
	static int l_RemoveObject(lua_State*ls);  
	/*! \brief Add buckazoids */
	static int l_AddBuckazoids(lua_State*ls);
	/*! \brief Print message */
	static int l_PrintMessage(lua_State*ls);
	/*! \brief Add Enemy ship */
	static int l_AddEnemyShip(lua_State* ls);
	/*! \brief Change object team to enemy (x, y - specify object position)
		\param 1 - x 
		\param 2 - y
	*/
	static int l_MakeEnemy(lua_State* ls);
	/*! \brief Change object team to player team (x, y - specify object position)
		\param 1 - x
		\paran 2 - y
	*/
	static int l_MakeAlly(lua_State* ls);
};


/*! \brief Intend to manage quests */
class QuestSource : public EventListner
{
protected:
	lua_State * Lquest;
	std::list<Quest*> questList;
public:
	QuestSource();
	virtual ~QuestSource();

	virtual int LoadQuestList( const char* qlist );
	virtual Quest* GetNextQuest( GobPlayer* p );
	virtual Quest* GetQuest ( const char * name, GobPlayer* p );
	virtual int QuestSuccess( Quest* q );
	virtual int QuestFailed( Quest* q );

	virtual void RemoveTrash(GobPlayer* pl = NULL);
	virtual void ProcessEvent(IEvent* event);

	virtual void ProcessQuests();
};


#endif // __GQUEST_H__
