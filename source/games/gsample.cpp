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
#include "../scp.h"


class SampleGame1 : public Game { //declaring our game type
	virtual void init(Log *_log) ; //happens when our game is first started
	};

void SampleGame1::init(Log *_log) {
	//you need to call Game::init very early on, to set stuff up... rarely do you want to do anything before that
	Game::init(_log);

	//prepare needs to be called before you add items, or do physics or graphics or anything like that.  Don't ask why.  
	prepare(); 

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

	Ship *e;
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


	//BTW, this is a vicious combination of enemies.  
	//Yehat for direct assault, Druuge for long range support, 
	//and the Urquans to make life miserable

	message.out("Welcome to Sample Game 1: Arena!", 3500, 15);
	//display a welcoming message to the player
	//the message is displayed for 3500 milliseconds (3.5 seconds)
	//in color 15 of the standard TW palette, which is white

	}

REGISTER_GAME(SampleGame1, "Sample 1: Arena") //registering our game type, so that it will appear in the menus

/*
TimeWarp Sample 2: Double Arena!

The differences from sample game 1:
1.  players are allowed in network games.  
2.  Players are permitted to pick their own ships.  
3.  The ship panels for human players are displayed.  
4.  The map is slightly larger.  
5.  The player is forced to use the "Hero" view.  
*/


class SampleGame2 : public Game
{ //declaring our game type
public:
	virtual void init(Log *_log) ; //happens when our game is first started
	virtual void init_players();
	TeamCode humans, enemies;
};


void SampleGame2::init_players()
{
	// re-use the same fleet for all players.
	//this is where we deviate from SampleGame1
	Fleet fleet;//first we declare a fleet
	log_file("fleets/all.scf"); //this is a fleet file containing every ship (more or less)
	fleet.load(NULL, "Fleet");

	int p;

	// check all the (human) players, allowing one per connected computer (not checking
	// for hotseat players).
	Control *c;
	int slot[100];

	for ( p = 0; p < num_network; ++p )
	{
		player[p] = new_player();
		player[p]->channel = channel_network[p];
		
		if (p == p_local)
		{
			c = create_control(channel_network[p], "Human");
		
		//to load a fleet from disk you say fleet.load(filename, fleetname); (fleetname is usually "Fleet", because that is the standard name used in .scf files)
		//but that's not network enabled, so we manually load the file using the network-enabled log_file(), and pass NULL for the file name
		//BTW, the cost of the fleet is fleet.cost and the size is fleet.size 
		//(if fleet is a pointer, then fleet->cost, fleet->size, and fleet->load instead)
			slot[p] = c->choose_ship(window, "Hey You!\nPick a ship!", &fleet);
		//note that we said &fleet instead of fleet because choose_ship expects a pointer at a fleet instead of just a fleet
		} else {

			c = create_control(channel_network[p], "VegetableBot");
		}

		player[p]->control = c;
		player[p]->color = p + 1;

		//now i is the number of the ship that the user selected from that list
		// (or if it's a remote player, it's chosen remotely, and isn't know here yet - it
		// will be after the share_update, so specify the slot here already
		share(p, &slot[p]);//log_int(channel_server, i);
	}
	num_players = num_network;

	// send/receive all choices.
	share_update();

	for ( p = 0; p < num_network; ++p )
	{
		int i;
		i = slot[p];

		//necessary for networking... because i would otherwise be different on the 2 computers...
		//channel_server is used instead of channel_client becaues we want the servers value to override the clients
		if (i == -1) i = random(fleet.getSize());
		//handles the case where random or always random was used
		
		//now we're back to the way we were in SampleGame1 for a little bit
		Ship *s = create_ship(fleet.getShipType(i)->id, player[p]->control, Vector2(size.x/2 + 100, size.y), 0, humans);
		add(s->get_ship_phaser());
		
		add_focus(s, player[p]->channel);
		//in SampleGame1, we just said "add_focus(s);" to make it a focus for the camera
		//but here it's different, because we don't want it to be a focus for all computers, only the server computer
		
		ShipPanel *sp = new ShipPanel(s); //here we create a ship panel for our ship
		sp->window->init(window); //here we tell the ship panel to display itself on our games window
		//this next bit of code tells the ship panel where on our display it should show up
		//it's little complicated, so I'll try to explain:
		//the location is made up of 4 pairs
		//the first 2 pairs describe the X & Y coordinates of the upper left corner
		//the next 2 pairs describe the width & height
		//the first number in each pair is a number of pixels
		//the second number is a portion of the screen width or height
		sp->window->locate(
			//the first pair of numbers describes the X coordinate of the 
			//upper left hand corner of the ship panel
			//-PANEL_WIDTH, 1 means it should be -PANEL_WIDTH + 1 * SCREEN_WIDTH
			//or, in english, PANEL_WIDTH to the left of the right edge of the screen
			-PANEL_WIDTH, 1,
			//the second pair of numbers describes the Y coordinate of the 
			//upper left hand corner of the ship panel
			//0, 0 means it should be 0 + 0 * SCREEN_HEIGHT
			//or, in english, the top edge of the screen
			0, 0,
			//the third pair of numbers describes the width of the ship panel
			//PANEL_WIDTH, 0 means it should be PANEL_WIDTH + 0 * SCREEN_WIDTH
			//or, in english, the normal width for a panel
			PANEL_WIDTH, 0,
			//the fourth pair of numbers describes the height of the ship panel
			PANEL_HEIGHT, 0
			);
		
		
		add(sp); //now we make the panel appear in the game
	}
}

void SampleGame2::init(Log *_log)
{
	Game::init(_log);

	size *= 1.2;
	//size is the size of the game map
	//it is a Vector2, so you can say size.x or size.y to refer to its components
	//size should be modified before prepare() is called
	//If for some reason you modify size later, call prepare() again
	//but you probably shouldn't be modifying it later. 
	//The normal size is 3840 by 3840
	//(that's written as Vector2(3840,3840))
	prepare(); 

	add(new Stars());

	int i;
	for (i = 0; i < 7; i += 1) add(new Asteroid());
	//This time, in addition to the asteroids, we'll add a planet
	SpaceObject *planet = new Planet(Vector2(size.x/4,0), meleedata.planetSprite, random(3));
	//the "random(3)" indicates that it uses one of the 3 normal planet pictures at random
	add(planet);
	//now we add an indicator to point towards the planet
	add(new WedgeIndicator(planet, 150, 4));
	//the second paremeter is the size of the indicator, the third parameter is a color (as a palette index)
	//alternatively we could use a blinky indicator, like "addPresence(new BlinkyIndicator(planet));"

	humans = new_team();
	enemies = new_team();



	init_players();


	/*
	if ((glog->type == Log::log_net1server) || (glog->type == Log::log_net1client)) { 
		//if we're in a network game...
		//then do the same thing, only for the other player
		Control *c = create_control(channel_client, "Human");
		//we can re-use the same fleet, since we never modified it and both players use the same fleet
		i = c->choose_ship(window, "Hey You!\nPick a ship!", &fleet);
		log_int(channel_client, i);
		if (i == -1) i = random() % fleet.getSize();
		Ship *s = create_ship(fleet.getShipType(i)->id, c, size/2+Vector2(100,0), PI, humans);
		add(s->get_ship_phaser());
		add_focus(s, channel_client);
		ShipPanel *sp = new ShipPanel(s);
		sp->window->locate(
			-PANEL_WIDTH, 1,
			PANEL_HEIGHT, 0,
			PANEL_WIDTH, 0,
			PANEL_HEIGHT, 0
			);
		add(sp);
		}
		*/

	//now, we need some enemies to fight

	Ship *e;
	e = create_ship(channel_server, "kzedr", "WussieBot", size/4, random(PI2), enemies);
	add(e->get_ship_phaser());
	e = create_ship(channel_server, "kohma", "WussieBot", Vector2(size.x*3/4, size.y/4), random(PI2), enemies);
	add(e->get_ship_phaser());
	e = create_ship(channel_server, "druma", "WussieBot", size*3/4, random(PI2), enemies);
	add(e->get_ship_phaser());
	e = create_ship(channel_server, "yehte", "WussieBot", Vector2(size.x/4, size.y*3/4), random(PI2), enemies);
	add(e->get_ship_phaser());
	//note that all the enemy AIs are located on the server
	//you can also have AIs colocated
	//to do that, you pass channel_none
	//benefits: an AI that is colocated uses no network bandwidth
	//disadvantages: colocating an AI may not work with all AI types; if it doesn't work, it will cause desynchs
	//Of course, only AIs can be colocated; you can't colocate a Human control, or bad things will happen

	change_view("Hero"); 
	//sets it to Hero view mode
	view_locked = true;
	//prevents the view mode from being changed through the menu
	view->window->locate(0,0,0,0,
		-PANEL_WIDTH,1,0,1);
	//we tell the main window to occupy the screen region to the left of the ship panels

	friendly_fire = 0;
	//here we override the normal settings for friendly fire to make sure that human players can't hurt each other

	message.out("Welcome to Sample Game: Double Arena!", 3000, 15);
	}

REGISTER_GAME(SampleGame2, "Sample 2: Double Arena")


/* 

TimeWarp Sample 3: Eternal Arena!

The differences from sample game 2:
1.  5 seconds after either all AIs or all humans die, the game restarts.  
2.  The player isn't foreced to use the hero view anymore

*/




class SampleGame3 : public SampleGame2
{ //declaring our game type
	virtual void init(Log *_log) ; //happens when our game is first started
	Control *human_control[2];
	virtual void calculate() ; 
	//TeamCode human_team, enemy_team; //the two teams
	int respawn_time; //the time left until the game is restarted
	void pick_new_ships(); //restarts the game
	Fleet fleet;
};

void SampleGame3::calculate()
{
	Game::calculate();
	if (respawn_time == -1) {
	  int i;
		unsigned int humans = 0, enemies = 0;
		for (i = 0; i < gametargets.N; i += 1) {
			if (gametargets.item[i]->get_team() == humans) humans += 1;
			if (gametargets.item[i]->get_team() == enemies) enemies += 1;
			}
		//if either team has no targetable items remaining (generally ships), pick new ships
		if (!humans || !enemies) respawn_time = game_time + 5000; //5000 milliseconds is 5 seconds
		}
	else if (respawn_time <= game_time) pick_new_ships();
	return;
}

void SampleGame3::pick_new_ships() {
	int i;
	window->lock();//call before writting to window
	clear_to_color(window->surface, pallete_color[4]);
	window->unlock();//call after writting to window
	for (i = 0; i < num_items; i += 1) {
		if (item[i]->exists()) item[i]->die();
		}
	//remove all existing items

	//player 1 selects a new ship
	i = human_control[0]->choose_ship(window, "Hey You!\nPick a ship!", reference_fleet);
	log_int(i, channel_server);
	if (i == -1) i = random() % reference_fleet->getSize();
	Ship *s = create_ship(reference_fleet->getShipType(i)->id, human_control[0], size/2 + Vector2(100, 0), 0, humans);
	add(s->get_ship_phaser());
	//notice that we DON'T call add_focus on the ship here?
	//that's because there's a focus on the ships controller already
	//look in SampleGame3::init
	ShipPanel *sp = new ShipPanel(s);
	sp->window->init(window);
	sp->window->locate(
		0, 0.8,
		0, 0, 
		0, 0.2,
		0, 100.0/240
		);
	add(sp);

	if (glog->type == Log::log_net) { 
		log_file("fleets/all.scf");
		i = human_control[1]->choose_ship(window, "Hey You!\nPick a ship!", reference_fleet);
		log_int(i, channel_network[1]);
		if (i == -1) i = random() % reference_fleet->getSize();
		Ship *s = create_ship(reference_fleet->getShipType(i)->id, human_control[1], size/2 + Vector2(100, 0), 180, humans);
		add(s->get_ship_phaser());
		ShipPanel *sp = new ShipPanel(s);
		sp->window->init(window);
		sp->window->locate(
			0, 0.8,
			0, 100.0/240, 
			0, 0.2,
			0, 100.0/240
			);
		add(sp);
		}


	Ship *e;
	e = create_ship(channel_none, "kzedr", "WussieBot", size/4, random(PI2), enemies);
	add(e->get_ship_phaser());
	e = create_ship(channel_none, "kohma", "WussieBot", Vector2(size.x*3/4, size.y/4), random(PI2), enemies);
	add(e->get_ship_phaser());
	e = create_ship(channel_none, "druma", "WussieBot", size*3/4, random(PI2), enemies);
	add(e->get_ship_phaser());
	e = create_ship(channel_none, "yehte", "WussieBot", Vector2(size.x/4, size.y*3/4), random(PI2), enemies);
	add(e->get_ship_phaser());

	//We don't add more asteroids here, because the destruction of the old ones 
	//will automatically generate more asteroids
	SpaceObject *planet = new Planet(Vector2(size.x/4, 0), meleedata.planetSprite, random(3));
	add(planet);
	add(new WedgeIndicator(planet, 150, 4));

	//if (human_panel[0]) human_panel[0]->locate(view->view_x + view->view_w, 0);   //if the panel still exists, position on the right-hand side
	//if (human_panel[1]) human_panel[1]->locate(view->view_x + view->view_w, 100); //same for the other panel, if there is another panel

	redraw();
	respawn_time = -1;
	return;
	}

void SampleGame3::init(Log *_log) {
	Game::init(_log);

	size = Vector2(4200,4200);
	//size is the size of the game map
	//it should be modified before prepare() is called
	//If for some reason you modify it later, call prepare() again
	//but you probably shouldn't be modifying it later. 
	//The normal size is 3840x3840
	prepare();

	add(new Stars());

	humans = new_team();
	enemies = new_team();

	init_players();

	respawn_time = 500;

	int i;
	for (i = 0; i < 7; i += 1) add(new Asteroid());

	if (p_local == 0) fleet.load("fleets/all.scf", "Fleet");
	log_fleet(channel_server, &fleet);
	//this time, instead of transmitting the fleet file over the network and then loading 
	//it on both sides
	//we instead load it on the server, and then transmit the fleet data (rather than the 
	//raw file)

	view->window->locate(0,0,0,0,0,0.8,0,1);
	message.out("Welcome to Sample Game 3: Eternal Arena!", 3000, 15);
	}

REGISTER_GAME(SampleGame3, "Sample 3: Eternal Arena")

