#ifndef __GQUEST_H__
#define __GQUEST_H__

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "gevent.h"

class GobPlayer;


class Quest: public EventListner
{
  bool bExist;
  lua_State * L;
  GobPlayer * gob_player;
 
 public:
  Quest( const char * szLuaFile, GobPlayer * player );
  virtual ~Quest();
  /*! \brief Summon default quest handler
    This function summon in every player think()
  */
  void Process();
  bool exist();
  
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
};


class QuestSource 
{
 protected:
  virtual int LoadQuestList( const char* qlist );
 public:
  virtual int GetNextQuest(Quest* q, GobPlayer* p) = 0;
  virtual int QuestSuccess(Quest* q, GobPlayer* p) = 0;
  virtual int WhenMeet(GobPlayer* p) = 0;
};

class StarBaseQuestSource: public QuestSource
{
 public:
  StarBaseQuestSource(const char* qlist);
  virtual ~StarBaseQuestSource();
  virtual int GetNextQuest(Quest* q, GobPlayer* p);
  virtual int QuestSuccess(Quest* q, GobPlayer* p);
  virtual int QuestFailed(Quest* q, GobPlayer* p);
  virtual int WhenMeet(GobPlayer* p);

};

#endif // __GQUEST_H__
