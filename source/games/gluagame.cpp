/**
  gametype to test out how Lua and C++ and Timewarp get along.
  Very experimental.
*/



#include <allegro.h> //allegro library header

#include "melee.h"          //used by all TW source files.  well, mostly.  
REGISTER_FILE                  //done immediately after #including melee.h, just because I said so
#include "melee/mgame.h"    //Game stuff
#include "melee/mcbodies.h" //asteroids & planets
#include "melee/mship.h"    //ships
#include "melee/mlog.h"     //networking / demo recording / demo playback
#include "melee/mcontrol.h" //controllers & AIs
#include "melee/mview.h"    //Views & text messages
#include "melee/mshppan.h"  //ship panels...
#include "melee/mitems.h"   //indicators...
#include "melee/mfleet.h"   //fleets...

#include "melee/manim.h"

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}


#define lua_register2(L,n,f) \
               (lua_pushstring(L, n), \
                lua_pushcfunction(L, f), \
                lua_settable(L, LUA_GLOBALSINDEX))
     /* lua_State *L;    */
     /* const char *n;   */
     /* lua_CFunction f; */

class LuaGame : public Game { //declaring our game type
	virtual void init(Log *_log) ; //happens when our game is first started
    lua_State *L;
    virtual void LuaGame::quit(const char *message);
};

LuaGame * globalLuaGame = NULL;

class SpecialAsteroid : public Asteroid {
    char name[80];

   public:
    SpecialAsteroid(const char * Name) :
    Asteroid()
    {
        strncpy(name, Name, 80);
    }

    void death() {
        //GOOD OO
        //Asteroid::death();
        
        //BAD OO
        Animation *a = new Animation(this, pos,
            explosion, 0, explosion->frames(), time_ratio, get_depth());
        a->match_velocity(this);
        game->add(a);
        message.print(1000,15,"You blew up a special asteroid named: \"%s\" which won't respawn", name);
    }
};


int addAsteroid(lua_State * L) {
    int args = lua_gettop(L);    // number of arguments 

    if (args == 1) {
        globalLuaGame->add(new SpecialAsteroid(lua_tostring(L,-1)));
        lua_pop(L,1);
    }
    else
    {
        globalLuaGame->add(new Asteroid());
    }
    return 0;
}

int addPlanet(lua_State * L) {

    int args = lua_gettop(L);    // number of arguments 
    if (args != 2)
        return 0; // TODO prevent stack overflow

    if ( ! lua_isnumber(L, -1) ||
         ! lua_isnumber(L, -2) )
         return 0; // TODO prevent stack overflow

    double x = lua_tonumber(L, -1);
    lua_pop(L, 1);
    double y = lua_tonumber(L, -1);
    lua_pop(L, 1);

    Vector2 v(x, y);
    Planet * planet = create_planet(v);
    //globalLuaGame->add( planet );

    return 0;
}

/** Adds an asteroid to the game environment.  As a Lua call, the prototype is:

    NOT TRUE YET:
      //addAsteroid(x,y)
    // @param x the x coordinate of the asteroid
    // @param y the y coordinate of the asteroid
    /NOT TRUE YET:
*/
//int LuaGame::addAsteroid(lua_State * state) {
//    globalLuaGame.add(new Asteroid());
//   return 0;
//}


void LuaGame::init(Log *_log) {
	//you need to call Game::init very early on, to set stuff up... rarely do you want to do anything before that
	Game::init(_log);

	//prepare needs to be called before you add items, or do physics or graphics or anything like that.  Don't ask why.  
	prepare(); 

	
    L = lua_open();
    
    luaopen_base(L);//TODO minimize the libraries used here
    luaopen_table(L);
    luaopen_io(L);
    luaopen_string(L);
    luaopen_math(L);
    luaopen_debug(L);

    //terrible hack, only for experimental use!!!
    //TODO remove this
    globalLuaGame = this;

    lua_register(L, "addAsteroid", addAsteroid);
    lua_register(L, "addPlanet", addPlanet);
    
    
    char str[100] = "";
    sprintf(str, "loading file returned %i\n", lua_dofile(L, "./source/games/luatest/bridge.lua"));
    message.out(str, 3500, 15);

    
     
    //add the starscape background
	add(new Stars());

	//first, we create the teams.  This battle has just 2 sides; the human player, and the AI enemies
	TeamCode human, enemies;
	human = new_team();
	enemies = new_team();

	//this creates a keyboard controller, using configuration 0 by default, since we don't specify another configuration
	Control *c = create_control(channel_server, "Human");
	//the first parameter is channel_server, which means that this player is on the server computer
	//the second parameter is "Human", the name of a controller type

	//this creates a ship
	Ship *s = create_ship("plopl", c, Vector2(size.x/2,size.y/2), 0, human);

	//this causes the ship to warp into the game.  note that we call add() on the phaser, not the ship, because the phaser will add the ship once it finishes warping in
	add(s->get_ship_phaser());
    //add(s);
	add_focus(s);
	}

    void LuaGame::quit(const char *message) {
        Game::quit(message);
        lua_close(L);
    }


REGISTER_GAME(LuaGame, "LuaTest") //registering our game type, so that it will appear in the menus

