
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


static const int max_enemies = 1024;
static const int max_timers = 32;

const int maxgroup = 16;

struct SideScroll_GroupInfo;



/** information about individual ship(s) in a group */
struct SideScroll_ShipInfo
{
	SideScroll_GroupInfo *group;

	Ship *ship;

	int current_timer;

	void calculate(int timeframe, Game *game, Vector2 screen_pos, int team);

	// the local timer: starts at 0
	int localtime;

	SideScroll_ShipInfo(SideScroll_GroupInfo *agroup);
	void create_ship(Game *game, Vector2 screen_pos, int team);
};




/** a group of ships */
struct SideScroll_GroupInfo
{
	bool status;

	char shipid[64];
	double spawnpos;
	Vector2 relpos;
	double angle;

	int Ntimers;
	int timer[max_timers];
	int key[max_timers];

	// how many ships are spawned in this group. They move in a line.
	int groupnum;
	// how many of the group are spawned
	int numspawned;
	// spacing (in time) between ships in the group.
	int groupinterval;

	// the local timer: starts at 0
	int localtime;

	// time at which the next ship will be spawned. Starts at 0.
	int next_time;

	SideScroll_GroupInfo();
	virtual ~SideScroll_GroupInfo();

	SideScroll_ShipInfo *shipinfo[maxgroup];

	void calculate(int timeframe, Game *game, Vector2 screen_pos, int team);

	// if true, then this group has been given the start-signal.
	bool startnow;
	void initiate();
};


SideScroll_ShipInfo::SideScroll_ShipInfo(SideScroll_GroupInfo *agroup)
{
	group = agroup;

	current_timer = 0;
	localtime = 0;

	ship = 0;
}

void SideScroll_ShipInfo::create_ship(Game *game, Vector2 screen_pos, int team)
{
	Ship *s;
	s = game->create_ship(channel_none,
		group->shipid, "WussieBot",
		screen_pos + group->relpos,
		group->angle, team);

	s->control->auto_update = false;
	s->vel = s->speed_max * unit_vector(s->angle);
	game->add(s);

	// scale the crew:
	s->crew = 1 + int(s->crew/20.0);

	s->collide_flag_sameteam = 0;

	ship = s;
}



void SideScroll_ShipInfo::calculate(int timeframe, Game *game, Vector2 screen_pos, int team)
{
	if (!ship)
		return;

	// what if the ship is killed...
	if (!ship->exists())
	{
		ship = 0;
		return;
	}

	localtime += timeframe;

	if (localtime > group->timer[current_timer])
	{
		// jump to the next key-command
		++current_timer;
	}

	if (current_timer >= group->Ntimers)
	{
		// clean up the ship
		ship->state = 0;//die();
		ship = 0;
	} else {
		// apply keys to the ship control
		ship->control->keys = group->key[current_timer];
	}
}



SideScroll_GroupInfo::SideScroll_GroupInfo()
{
	status = true;
	shipid[0] = 0;
	Ntimers = 0;
	
	localtime = 0;

	next_time = 0;

	numspawned = 0;
	startnow = false;

	int i;
	for ( i = 0; i < maxgroup; ++i )
	{
		shipinfo[i] = 0;
	}
}

SideScroll_GroupInfo::~SideScroll_GroupInfo()
{
	int i;
	for ( i = 0; i < maxgroup; ++i )
	{
		if (shipinfo[i])
			delete shipinfo[i];
	}
}

void SideScroll_GroupInfo::initiate()
{
	startnow = true;
}


void SideScroll_GroupInfo::calculate(int timeframe, Game *game, Vector2 screen_pos, int team)
{
	if (!startnow)
		return;

	if (!status)
		return;


	// check if there's at least one live ship left...
	status = false;

	if (numspawned < groupnum)
		status = true;	// there will be more spawned.

	if (numspawned < groupnum && localtime >= next_time)
	{
		// spawn the next ship in the group.
		shipinfo[numspawned] = new SideScroll_ShipInfo(this);
		shipinfo[numspawned]->create_ship(game, screen_pos, team);

		next_time += groupinterval;
		++numspawned;
	}

	localtime += timeframe;


	// check all the ships ...
	// and update their stats.
	int i;
	for ( i = 0; i < numspawned; ++i )
	{
		if (shipinfo[i]->ship)
		{
			shipinfo[i]->calculate(timeframe, game, screen_pos, team);
			status = true;
		}
	}

}








class GSideScroll : public Game
{ //declaring our game type
public:
	virtual void init(Log *_log) ; //happens when our game is first started
	virtual void init_players();
	virtual void calculate();
	virtual void init_objects();
	virtual void animate(Frame *f);

	TeamCode team_human, team_enemy;

	// keeping track of level:
	void init_next_level();
	int level_count;

	// info about this level:
	SideScroll_GroupInfo enemy_info[max_enemies];
	int Nenemies;
	int current_enemy;

	Vector2 screen_pos;
	double screen_vel;

	Vector2 battlearea;
	double startpos;
};


void GSideScroll::init_objects()
{
	// do nothing ?
}


void GSideScroll::init_next_level()
{
	++ level_count;

	set_config_file("gsidescroll.ini");

	char tmp[512];
	sprintf(tmp, "level%i", level_count+1);

	Nenemies = get_config_int(tmp, "N", 0);
	if (Nenemies > max_enemies)
		tw_error("Trying to initialize too many enemies");

	int i;
	for ( i = 0; i < Nenemies; ++i )
	{
		char id[512];
		sprintf(id, "enemy%i", i+1);
	
		char **args;
		int Nargs;
		args = get_config_argv(tmp, id, &Nargs);

		if (Nargs < 5)
		{
			tw_error("Not enough data for the enemy ship");
		}

		int k = 0;

		// at which position is the ship spawned (start at 0 and count upwards)
		enemy_info[i].spawnpos = atof(args[k]);
		++k;

		// how many ships, and at which time interval?
		enemy_info[i].groupnum = atoi(args[k]);
		++k;
		if (enemy_info[i].groupnum > maxgroup)
			enemy_info[i].groupnum = maxgroup;

		enemy_info[i].groupinterval = atoi(args[k]);
		++k;
		
		// read the ship name
		strncpy(enemy_info[i].shipid, args[k], sizeof(enemy_info[k].shipid)-1);
		++k;
		// read ship position and angle:
		enemy_info[i].relpos.x = atof(args[k]);	// where on the screen does it appear
		++k;
		enemy_info[i].relpos.y = atof(args[k]);
		++k;
		enemy_info[i].angle = atof(args[k])*PI/180.0;	// initial orientation of the ship (in degrees)
		++k;
		// other ship parameters:
		// velocity is at max.
		// key = 0

		// read the timers and keys for this ship:
		int count = 0;
		while ( k+1 < Nargs )
		{

			// first, read the keys.
			// this key-combo is applied until time exceeds the timer that follows.

			if (strlen(args[k]) < 5)
				tw_error("Expected key info, in the form of 11111 (left/right/thrust/fire/special) ");

			enemy_info[i].key[count] = 0;
			if (args[k][0] == '1')	enemy_info[i].key[count] |= keyflag::left;
			if (args[k][1] == '1')	enemy_info[i].key[count] |= keyflag::right;
			if (args[k][2] == '1')	enemy_info[i].key[count] |= keyflag::thrust;
			if (args[k][3] == '1')	enemy_info[i].key[count] |= keyflag::fire;
			if (args[k][4] == '1')	enemy_info[i].key[count] |= keyflag::special;
			++k;

			// read the timer for the next key. (in milliseconds)
			enemy_info[i].timer[count] = atoi(args[k]);
			++k;

			++count;
		}

		enemy_info[i].Ntimers = count;

	}

	current_enemy = 0;


	screen_pos.y = size.y - startpos;		// start at bottom (and move up)
	screen_pos.x = size.x/2;	// stay focused on the middle.
}


void GSideScroll::init_players()
{
	team_human = new_team();
	team_enemy = new_team();

	Game::init_players();

	// give a ship to each player

	int p;
	for ( p = 0; p < num_network; ++p )
	{
		//now we're back to the way we were in SampleGame1 for a little bit
		Ship *s = create_ship("orzne", player[p]->control, Vector2(size.x/2, size.y-100.0), -0.5*PI, team_human);
		add(s);
		add_focus(s, player[p]->channel);
	}


}

void GSideScroll::init(Log *_log)
{
	Game::init(_log);

	set_config_file("gsidescroll.ini");

	size.x = get_config_int("init", "mapw", 2000);
	size.y = get_config_int("init", "maph", 10000);
	screen_vel = get_config_int("init", "velocity", 100) * 1E-3;	// in file: in pixels/sec, in game: in pixels/millisec
	battlearea.x = get_config_int("init", "areaw", 800);
	battlearea.y = get_config_int("init", "areah", 600);
	startpos = get_config_int("init", "startpos", 600);
	
	// copy to global values.
	prepare(); 


	add(new Stars());


	change_view("Hero"); 
	//sets it to Hero view mode
	view_locked = true;
	//prevents the view mode from being changed through the menu
	view->window->locate(0,0,0,0,
		0,1,0,1);
	//we tell the main window to occupy the screen region to the left of the ship panels

	friendly_fire = 0;
	//here we override the normal settings for friendly fire to make sure that human players can't hurt each other

	level_count = -1;
	init_next_level();
}



void GSideScroll::calculate()
{
	screen_pos.y -= screen_vel * frame_time;

	// fix player position(s):
	double xmin = screen_pos.x - 0.5*battlearea.x  + 20.0;
	double xmax = screen_pos.x + 0.5*battlearea.x  - 20.0;
	double ymin = screen_pos.y - 0.5*battlearea.y  + 20.0;
	double ymax = screen_pos.y + 0.5*battlearea.y  - 20.0;

	int p;
	for ( p = 0; p < num_network; ++p )
	{
		Ship *s = player[p]->control->ship;

		double x, y;
		x = s->pos.x;
		y = s->pos.y;
		
		// test boundaries
		if (x < xmin)	s->pos.x = xmin;
		if (x > xmax)	s->pos.x = xmax;
		if (y < ymin)	s->pos.y = ymin;
		if (y > ymax)	s->pos.y = ymax;
		s->pos = s->normal_pos();
	}

	// initialize enemies:
	double y = size.y - screen_pos.y;
	while (current_enemy < Nenemies && enemy_info[current_enemy].spawnpos <= y)
	{
		// if the player passes a spawnpoint, an enemy is added.
		enemy_info[current_enemy].initiate();


		++current_enemy;
	}

	// check the enemy array.
	int i;
	bool all_gone = true;
	for ( i = 0; i < Nenemies; ++i )
	{
		if (enemy_info[i].status)	// check if there are still active enemies somewhere.
			all_gone = false;

		enemy_info[i].calculate(frame_time, this, screen_pos, team_enemy);

	}

	// if all the ships of the current level are treated
	if (all_gone)
	{
		init_next_level();	// initialize next level

		if (Nenemies == 0)	// if the level is empty, then exit.
			quit("bye");
	}

	// if the player is dead
	if (!player[0]->control->ship)
	{
		quit("bye");
	}


	// normal game calculations (ship position updates etc)
	Game::calculate();

}

void GSideScroll::animate(Frame *f)
{
	// override zoom settings:
	::space_center = screen_pos;
	::space_center_nowrap = screen_pos;
	::space_zoom = (screen->w / battlearea.x);

	message.print(1, 7, "crew: %2i  batt %2i  pos = %4i", int(player[0]->control->ship->crew),
		int(player[0]->control->ship->batt), int(size.y-screen_pos.y));

	Game::animate(f);
}


REGISTER_GAME(GSideScroll, "Sidescroll")

