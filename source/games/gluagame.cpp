/*

This file contains sample games 1, 2, & 3, otherwise known as 
Arena, Double Arena, and Eternal Arena.  These demonstrate some 
basics of how to create a Game type in Star Control: TimeWarp

*/


/* 
TimeWarp Sample Game 1: Arena
This demonstrates how to create a new Game type

For this game, we'll just have a bunch of ships fighting 
the human player.  
*/

//first, we #include the files we need.  library headers go first, then melee.h, then any other pieces of TW we need to refer to

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

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}


class LuaGame : public Game { //declaring our game type
	virtual void init(Log *_log) ; //happens when our game is first started
    lua_State *L;
    virtual void LuaGame::quit(const char *message);
	};

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
    
    
    char str[100] = "";
    sprintf(str, "loading file returned %i\n", lua_dofile(L, "./source/games/luatest/bridge.lua"));
    message.out(str, 3500, 15);
    
     
    //add the starscape background
	add(new Stars());

	// declare an integer we can use for whatever we might need an integer for
	int i; 
	for (i = 0; i < 3; i += 1) { //this causes the next stuff to happen 3 times
		add(new Asteroid()); //this adds an asteroid to the game
		} //so there will be 3 asteroids (asteroids automatically create new asteroids when they die...)

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
	//the 1st parameter is a 5-letter ship code.  e.g. plopl = Ploxis Plunderer, andgu = Androsynth Guardian, chmav = Chmmr Avatar, estgu = Estion Gunner, tauhu = Tau Hunter, thrto = Thraddash Torch
	//the 2nd parameter is the controller for the ship.  
	//the 3th parameter is the positition where it appears.  
	//the 4th parameter is the angle that the ship starts out facing (in radians), 0 in this case, meaning that it points to the right
	//Note that angles in radians range from 0 to 2 * PI (6.283...), unlike angles in degrees which range from 0 to 360
	//You can convert degrees to radians by multiplying by ANGLE_RATIO
	//the 5th parameter is the team it's on... this parameter is optional... if we leave it out, then it's on it's own team of 1, and everyone is its enemy

	//I should talk more about the 3rd parametere there, the position
	//The idea of the position is simple: an X coordinate and a Y coordinate
	//The X & Y coordinates indicate a point in space.  
	//The ship is placed with its center at that point.  
	//However, even though it's 2 coordinates, it's only one parameter
	//That's because create_ship takes a 2-dimensional vector (the type for which is called Vector2 in TimeWarp)
	//the vector contains the X & Y coordinates.  
	//There are many other functions in TimeWarp that take vectors. 
	//When you want to pass one of those functions a constant like x=13.5,y=7.9, you give them Vector2(13.5, 7.9)
	//When you recieve a Vector2 and want to know what the x or y component of it are , you append .x or .y to the end of it to get those components
	//In the above example I said "Vector2(size.x/2,size.y/2)"
	//but it would have been equivalent to just say "size/2"

	//this causes the ship to warp into the game.  note that we call add() on the phaser, not the ship, because the phaser will add the ship once it finishes warping in
	add(s->get_ship_phaser());
	//if we wanted the ship to just appear without warping in, then we would say "add(s);" instead

	//this causes the human ship to be a focus for the camera
	add_focus(s);

	//now, we create some enemies to fight

/*	Ship *e;
	e = create_ship(channel_none, "kzedr", "WussieBot", Vector2(size.x/4,size.y/4), random(PI2), enemies);
	//This is a different syntax for create_ship
	//It creates a ship and an AI for it at the same time.  
	//AIs created in this way are automatically destroyed when their ship dies
	//Notice how it takes the parameters normally taken by both a create_control and a create_ship
	//Anyway, this creates a Kzer-Za Dreadnought, and WussieBot AI to control it
	//This ship starts facing a random direction from 0 to 2 * PI radians, because of the "random(PI2)"
	//PI2 is shorthand for 2 * PI in timewarp.  In timewarp, random(x) will produce a random number from 0 to x
	//Notice that it's on channel channel_none.  This mean that the AI is considered part of physics and uses no bandwidth in network games
	//When AIs use channel_none they are not effected by network lag.  
	//You can also use a channel_server or channel_client, to locate the AI on the server or client computer.  
	//If you do so the AI will experience lag and use network bandwidth just like a human player
	add(e->get_ship_phaser());
	e = create_ship(channel_none, "kohma", "WussieBot", Vector2(size.x*3/4,size.y/4), random(PI2), enemies);
	//here we add a Kohr-Ah Marauder
	add(e->get_ship_phaser());
	e = create_ship(channel_none, "druma", "WussieBot", Vector2(size.x*3/4,size.y*3/4), random(PI2), enemies);
	//here we add a Druuge Mauler
	add(e->get_ship_phaser());
	e = create_ship(channel_none, "yehte", "WussieBot", Vector2(size.x/4,size.y*3/4), random(PI2), enemies);
	//here we add a Yehat Terminator
	add(e->get_ship_phaser());

*/
	//BTW, this is a vicious combination of enemies.  
	//Yehat for direct assault, Druuge for long range support, 
	//and the Urquans to make life miserable

	//message.out("Welcome to Sample Game 1: Arena!", 3500, 15);
	//display a welcoming message to the player
	//the message is displayed for 3500 milliseconds (3.5 seconds)
	//in color 15 of the standard TW palette, which is white

	}

    void LuaGame::quit(const char *message) {
        Game::quit(message);
        lua_close(L);
    }


REGISTER_GAME(LuaGame, "LuaTest") //registering our game type, so that it will appear in the menus

