/**
  gametype to test out how Lua and C++ and Timewarp get along.
  Very experimental.
*/



#include <allegro.h> //allegro library header

#include "../melee.h"          //used by all TW source files.  well, mostly.  
REGISTER_FILE                  //done immediately after #including melee.h, just because I said so
#include "../melee/mgame.h"    //Game stuff
#include "../melee/mcbodies.h" //asteroids & planets
#include "../melee/mship.h"    //ships
#include "../melee/mlog.h"     //networking / demo recording / demo playback
#include "../melee/mcontrol.h" //controllers & AIs
#include "../melee/mview.h"    //Views & text messages
#include "../melee/mshppan.h"  //ship panels...
#include "../melee/mitems.h"   //indicators...
#include "../melee/mfleet.h"   //fleets...

#include "../melee/manim.h"

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


/** \brief A list of lua-type functions

*/

static const int max_lua_func = 1024;
static int Ndescriptors = 0;

/** \brief Class that is used to initialize a list of lua-type functions during
game initialization. This list just stores some pointers and strings, has nothing
to do with lua, rather, it is temporary storage.

*/

class lua_func_registry
{
public:
	char id[512];
	lua_CFunction f;
	lua_func_registry(char *descr, lua_CFunction luafunc);

} *descriptors[max_lua_func];

lua_func_registry::lua_func_registry(char *descr, lua_CFunction luafunc)
{
	if (Ndescriptors < max_lua_func)
	{
		// store the lua-function information, needed to initialize lua later on.
		strncpy(id, descr, sizeof(id));
		f = luafunc;

		// add this thing to a global list
		descriptors[Ndescriptors] = this;
		++Ndescriptors;
	}
}

/** \brief Accesses a list of lua-type functions and registers them with lua.
*/

static void register_functions(lua_State *L)
{
	int i;
	for ( i = 0; i < Ndescriptors; ++i )
	{
	    lua_register(L, descriptors[i]->id, descriptors[i]->f);
	}
}



// Some macros to reduce code/coding complexity :

// declares the default (needed by the arg-macros) function body
// also declares narg, the number of arguments that are to be returned
// also declares the lua-registry class, which is used to build a list of functions available to lua.
#define lua_func(func) \
extern int func(lua_State *L); \
lua_func_registry regme##func (#func, func); \
static int func(lua_State *L) \
{ \
int narg = 0;

// closing bracket, and returns the number of arguments to lua
#define lua_ret \
if (lua_gettop(L) >= 1) tw_error("lua: not all arguments were read"); \
return narg; \
}

// declares a double, and initializes with a value from the stack.
#define arg_double(x) \
double x; \
if (!lua_isnumber(L, -1)) tw_error("lua: argument is not a number"); \
x = lua_tonumber(L, -1); \
lua_pop(L, 1);


#define arg_string(x) \
const char *x; \
if (!lua_isstring(L, -1)) tw_error("lua: argument is not a string"); \
x = lua_tostring(L, -1); \
lua_pop(L, 1);






lua_func(addPlanet)	// initialize the function, and add some overhead for registering this function automatically
{
	arg_double(x);	// this gets a double from the stack
	arg_double(y);
	
	Vector2 v(x, y);
	Planet * planet = create_planet(v);
}
lua_ret	// end the function, in a way that lua likes



lua_func(addSpecialAsteroid)
{
	arg_string(s);	
    if (s)
        physics->add(new SpecialAsteroid(s));

}
lua_ret



lua_func(addAsteroid)
{
	physics->add(new Asteroid());
}
lua_ret




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


void LuaGame::init(Log *_log)
{
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

	// lua_registers functions, that are conveniently stored in a separate listing
	// till now.
	register_functions(L);
    
    
    char str[100] = "";
    sprintf(str, "loading file returned %i\n",
		lua_dofile(L, "./source/games/luatest/bridge.lua"));

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



void LuaGame::quit(const char *message)
{
	Game::quit(message);
	lua_close(L);
}


REGISTER_GAME(LuaGame, "LuaTest") //registering our game type, so that it will appear in the menus

