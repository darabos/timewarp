#ifndef __GQUEST_H__
#define __GQUEST_H__

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

class GobPlayer;
class GobGame;

class Quest
{
  bool bExist;
  lua_State * L;
  GobPlayer * gob_player;
  GobGame * gob_game;
 
 public:
  Quest( const char * szLuaFile, GobPlayer * player, GobGame * game );
  virtual ~Quest();
    /*! \brief Summon default quest handler
      This function summon in every player think()
    */
    void Process();
    bool exist();
    private:
  // This is callback functions from Lua quest script
  /*! \brief On complite quest */
  static int l_Dialog(lua_State* ls);
  /*! \brief Add quest object ( ship, starbase) to the game */
  static int l_AddObject(lua_State* ls);
  /*! \brief Remove quest object ( ship, starbase ... ) */
  static int l_RemoveObject(lua_State*ls);
};


#endif // __GQUEST_H__
