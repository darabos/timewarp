/* 

The Second Set Of Sample Games For STAR CONTROL: TIMEWARP

INCOMPLETE ; not done, not commented

Ideas:

Sample 4: defend a starbase

?Sample 5: play billiards with some planets

?Sample 6: triathlon race : kill, fly, gravwhip, ?, against the clock

Current status:

Sample 4 seems to be working, but needs more work
perhaps I should add a high-score table?

Samples 5 and 6 have not yet been started

*/


#include <stdio.h>   //standard io libary (needed for sprintf)
#include <allegro.h> //allegro library header

#include "melee.h"          //used by all TW source files.  well, mostly.  
REGISTER_FILE                  //done immediately after #including melee.h, just because I said so
#include "melee/mgame.h"    //Game stuff
#include "melee/mcbodies.h" //asteroids & planets
#include "melee/mship.h"    //ships
#include "melee/mshot.h"   //shots, missiles, lasers
#include "melee/mlog.h"     //networking / demo recording / demo playback
#include "melee/mcontrol.h" //controllers & AIs
#include "melee/mview.h"    //Views & messages
#include "melee/mshppan.h"  //ship panels...

#include "melee/mitems.h"  //ship panels...
#include "melee/manim.h"  //ship panels...


// Sample game 4: Defender of the Starbase

//first we define the starbase
class DefenderStation : public Orbiter {
	public:
	double health, maxhealth; //current and maximum starbase health
	int healtime, nexthealtime;   //rate & phase of ship healing
	int regentime, nextregentime; //rate & phase of self healing
	virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);
	//handle_damage lets the starbase be hurt & die
	virtual void calculate ();
	//calculate does the continuous things like regeneration
	DefenderStation ( SpaceSprite *sprite, SpaceLocation *orbit );
	//the constructor.  all game items must have constructors
	//this one just sets up values for health, etc.
};

//now we declare the game type
//we declared the station type first, since the game type has a pointer at it
class DefenderGame : public Game {
	public:
	TeamCode player_team, enemy_team;
	SpaceSprite *stationsprite;
	//this holds the graphics for the space station
	//because it's not a ship, you have to manage its graphics manually
	virtual void preinit ();
	//initialize all pointers to NULL, so that the destructor won't crash if it's called early for some reason
	virtual ~DefenderGame();
	//the destructor.  it needs to deallocate any special resources that are handled manually (stationsprite, for this game)
	virtual void init ( Log *_log ) ;

	void restart();

	//begin the game
	virtual void calculate ( ) ;
	//here we check to see if the player lost the game
	//and we add new enemies if it's time for that
	virtual void fps();
	//this is used to display the starbases current health
	Control *player;
	//a pointer at the player
	DefenderStation *starbase;
	//a pointer at the starbase
	int time_for_next_attack;
	int time_between_attacks;
	//rate & phase of enemy ships spawning
};

#define defendergame ((DefenderGame*)game)

int DefenderStation::handle_damage ( SpaceLocation *source, double normal, double direct) {
	double old = health;
	health -= normal;
	health -= direct;
	if (health <= 0) {
		add ( new Animation ( this, pos, meleedata.kaboomSprite, 0, meleedata.kaboomSprite->frames(), 50, DEPTH_EXPLOSIONS) );
		state = 0;
	}
	return (int)(old-health);
}

DefenderStation::DefenderStation( SpaceSprite *sprite, SpaceLocation *orbit)
: Orbiter ( sprite, orbit, 600 ) 
{
	health = maxhealth = 30;
	regentime = 6000;
	nextregentime = 0;
	healtime = 2000;
	nexthealtime = 0;
}

void DefenderStation::calculate ( ) {
	Orbiter::calculate();
	if ( (health < maxhealth) && (nextregentime <= game->game_time) ) {
		health += 1;
		nextregentime = game->game_time + regentime;
	}
	Ship *ship = defendergame->player->ship;
	if ( ship && (distance(ship) < 300) && (nexthealtime <= game->game_time) && (ship->crew < ship->crew_max) && (magnitude_sqr(ship->get_vel() - vel) < 0.01) ) {
		ship->handle_damage(this, -1);
		nexthealtime = game->game_time + healtime;
		SpaceLocation *l = new PointLaser ( this, palette_color[10], 0, 150, this, ship, 0);
		game->add ( l );
		l->set_depth(DEPTH_SHIPS + 0.2);
	}
	accelerate(this, trajectory_angle(center), 0.00005, MAX_SPEED);
	return;
}

static int num_ships = 4;
static const char *someships[] = { "thrto", "supbl", "syrpe", "tautu", NULL };

void DefenderGame::calculate ( ) {
	Game::calculate();
	if (starbase && !starbase->exists()) {
		starbase = NULL;
		char buffy[1024];
		sprintf(buffy, "You lost after %d seconds", game_time/1000);
		message.print(9999000, 15, buffy);
		switch (tw_alert(buffy, "Exit", "Continue", "Try Again")) {
		case 1: error ( "quit" );
		case 2: break;
		case 3: restart();
		}
	}
	else if (game_time >= time_for_next_attack) {
		time_for_next_attack += time_between_attacks + (random(10000)) - 4000;
		time_between_attacks -= 10;
		time_between_attacks = iround(time_between_attacks * 0.975);
		time_between_attacks += 10;
		SpaceObject *whatever = create_ship ( channel_none, someships[random(num_ships)], "WussieBot", Vector2(0, 2000), 0, enemy_team);
		add (whatever);
	}
	return;
}
void DefenderGame::preinit() {
	Game::preinit();
	stationsprite = NULL;
//because the desctructor deals with stationsprite, we have to initialize it here
//just in case the normal init() function doesn't get called
}
DefenderGame::~DefenderGame() {
//we do not call the parent destructor (Game::~Game) because C++ calls it for us
	if (stationsprite) delete stationsprite;
//we have to manually unload space station graphics since they aren't part of a ship
}

void DefenderGame::fps() {
	int s = 0;
	if (starbase) s = (int)starbase->health;
	message.print((int)msecs_per_fps, 15, "Current Time: %d", game->game_time / 1000);
	message.print((int)msecs_per_fps, 12, "Starbase Health: %d", s);
	int p = 0;
	if (player->ship) p = (int)player->ship->getCrew();
	message.print((int)msecs_per_fps, 12, "Your Health: %d", p);
}

void DefenderGame::restart() {
	int i;
	for (i = 0; i < num_items; i += 1) {
		if (item[i]->exists()) item[i]->die();
	}
	game_time = 0;

	Ship *ship = create_ship("supbl", player, Vector2(500, 200), 270);
	add(ship);

	Planet *planet = new Planet ( 0, meleedata.planetSprite, random(3) );
	add ( planet );

	starbase =  new DefenderStation ( stationsprite, planet);
	add ( starbase );
	starbase->change_owner ( ship );
	gametargets.add (starbase);

	time_for_next_attack = game_time + 3 * 1000;//first attack in 3 seconds from now
	time_between_attacks = 24 * 1000;//24 seconds between attacks


	message.out("Defend the starbase!", 30000);

}

void DefenderGame::init( Log * _log) {
	Game::init(_log);

	prepare();
	add ( new Stars() );

	player = create_control ( channel_server, "Human" );
	add_focus(player);

	DATAFILE *tmpdata; 
	tmpdata = load_datafile_object("gob.dat", "station0sprite");
	if (!tmpdata) error( "couldn't find gob.dat#station0sprite");
	stationsprite = new SpaceSprite(tmpdata, 1, SpaceSprite::MASKED, 64);
	stationsprite->permanent_phase_shift(8);
	unload_datafile_object(tmpdata);

	restart();
	
	message.out("An enemy ship will attack it every 20-30 seconds", 20000, 7);
	message.out("The starbase will heal you if you fly close to", 20000, 7);
	message.out("  it and match its velocity", 20000, 7);
	message.out("It can also heal itself, but slowly", 20000, 7);
	return;
}

REGISTER_GAME ( DefenderGame, "Sample 4: Defender" )

