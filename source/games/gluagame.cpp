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

#include "../other/luaport.h"




class LuaGame : public Game { //declaring our game type
	virtual void init(Log *_log) ; //happens when our game is first started
    lua_State *L;
    virtual void LuaGame::quit(const char *message);
	virtual void calculate();
};


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




// just return the whole string...
// it also advances the string pointer ...

static const char *get_lua_chunk (lua_State *L, void *data, size_t *size)
{
	char **s = (char**) data;

	char *d = *s;

	*size = strlen(*s);
	*s += *size;

	if (!*size)
		return 0;
	else
		return d;
}



void lua_dochunk(lua_State *L, char **data)
{
	if (lua_load(L, get_lua_chunk, data, "loaded_chunk") != 0)
		tw_error("lua: error loading memory chunk");

	if (lua_pcall(L, 0, LUA_MULTRET, 0) != 0)
		tw_error("lua: something went wrong executing command");
}




void LuaGame::init(Log *_log)
{
	//you need to call Game::init very early on, to set stuff up... rarely do you want to do anything before that
	Game::init(_log);
	
	//prepare needs to be called before you add items, or do physics or graphics or anything like that.  Don't ask why.  
	prepare(); 
	
	

    L = lua_init();

    
    
    char str[100] = "";
    sprintf(str, "loading file returned %i\n",
		lua_dofile(L, "./source/games/luatest/bridge.lua"));

    message.out(str, 3500, 15);


	// this is used to see, if something can be initialized from a memory string - it can.
	char *com = "addPlanet(1500,1500);";

	lua_dochunk(L, &com);
	
    
	
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


void LuaGame::calculate()
{
	Game::calculate();

	// also execute the calculate() function in the lua script,
	// if such a function exists that is ...

	lua_pushstring(L, "calculate");
	lua_gettable(L, LUA_GLOBALSINDEX);
	lua_call(L, 0, 0);	// 0 arguments, 0 results

}


REGISTER_GAME(LuaGame, "LuaTest") //registering our game type, so that it will appear in the menus

