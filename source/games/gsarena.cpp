/* $Id$ */ 
#include <allegro.h> //allegro library header
#include <stdio.h>

#include "../melee.h"          //used by all TW source files.  well, mostly.
REGISTER_FILE                  //done immediately after #including melee.h, just because I said so
#include "../melee/mgame.h"    //Game stuff
#include "../melee/mmain.h"    //Game stuff
#include "../melee/mcbodies.h" //asteroids & planets
#include "../melee/mship.h"    //ships
#include "../melee/mlog.h"     //networking / demo recording / demo playback
#include "../melee/mcontrol.h" //controllers & AIs
#include "../melee/mview.h"    //Views & messages
#include "../melee/mshppan.h"  //ship panels...
#include "../melee/mitems.h"  //ship panels...
#include "../melee/mfleet.h"
#include "../other/orbit.h"
#include "../scp.h"

class SuperArena : public NormalGame { //declaring our game type
	virtual void init(Log *_log) ; //happens when our game is first started
//	ShipPanel *human_panel[2];  //we need to keep track of the ship panels, in case we have to move them around because the player changes screen resolutions
//	Control *human_control[2];

	virtual void calculate() ;
	//to do the screen layouts, we need to know if those panels still exist, so we need to check every frame for them going away
	//calculate is good for things that need to be done every frame
//	TeamCode team[9]; //the teams (array)
	TeamCode station_team;
	int respawn_time; //the time left until the game is restarted
	void pick_new_ships(); //restarts the game

//	void new_ship(int team, int human);


	Ship *s[9]; //the starbases
	int started,KTS;
	int died[9];
	int num_teams; //added by orz, attempted bugfix
	};

void SuperArena::calculate() {
	STACKTRACE;

	Game::calculate();
//	if (human_panel[0] && !human_panel[0]->exists()) human_panel[0] = NULL;
//	if (human_panel[1] && !human_panel[1]->exists()) human_panel[1] = NULL;

	if (respawn_time == -1)
	{
	  //	int i, humans = 0, enemies = 0;
		int i, t[100];
		int n;
		for ( n = 0; n < num_players; ++n )
			t[n] = 0;

		for (i = 0; i < gametargets.N; i += 1)
		{

			SpaceObject *o = gametargets.item[i];

			for ( n = 0; n < num_players; ++n )
			{
				// don't count your starbase...
				if (o != s[n] && o->get_team() == player[n]->team)
					t[n] += 1;
			}
		}

		//if either team has no targetable items remaining (generally ships), pick new ships
	  //		if (humans = 0) respawn_time = game_time + 5000; //5000 milliseconds is 10 seconds
	  //	if (!humans || !enemies) respawn_time = game_time + 5000; //5000 milliseconds is 10 seconds
 //		iMessage("Team1 have %d ships",t1);
	//	iMessage("Team2 have %d ships",t2);
		int nalive = 0;
		for ( n = 0; n < num_players; ++n )
		{
			if (t[n] > 0)
				++nalive;
		}

		if (nalive <= 1)
			respawn_time = game_time + 5000;

		if ((started == 1) && (KTS == 1)) {
			for (int num=1; num < num_players; num+=1) {

				// CRASH s is invalid
				if ( (died[num]!=1) && (!s[num]->exists()) )
				{

					message.print( 4000, 4000, "Team %d starbase was destroyed", num+1 ); //("Player's 1 Starbase was destroyed", 3000);
					for (i = 0; i < gametargets.N; i += 1)
					{
						if (gametargets.item[i]->get_team() == player[num]->team) {
							gametargets.item[i]->handle_damage(NULL, 0, 999);
							//target[i]->handle_damage(target[i]);
						}
					}
					died[num]=1;
				}
			}
		}
   }
	else if (respawn_time <= game_time) pick_new_ships();

	return;
	}

void SuperArena::pick_new_ships()
{
	STACKTRACE;

	/*
	int i,t,p[9],j;
	num_teams = t = get_config_int(NULL, "Teams", 2);
	int n;
	for ( n = 0; n < num_teams; ++n )
	{
		char name[512];
		sprintf(name, "Team%i", n);
		p[n] = get_config_int(NULL, name, 2);
	}
	*/

	// is KTS on?
	log_file("sarena.ini");
	KTS = get_config_int(NULL, "KTS", 0);

	acquire_screen();
	clear_to_color(screen, pallete_color[4]);
	release_screen();
	int i;
	for (i = 0; i < num_items; i += 1)
	{
		if (item[i]->exists()) item[i]->die();
	}

//	if (human_panel[0]) human_panel[0]->window->hide();
//	if (human_panel[1]) human_panel[1]->window->hide();

	NormalGame::choose_new_ships();


//	Ship *e;
	Planet *Pl;
	OrbitHandler *handler;
//	char buf[256];

	if (KTS==1)
	{
		int i;
		for ( i = 0; i < num_players; ++i )
		{
			Pl = new Planet(random(size), meleedata.planetSprite, random()%meleedata.planetSprite->frames());
			add(Pl);
			s[i] = create_ship(channel_none, "staba", "WussieBot", random(size), 1, player[i]->team);
			add ( new BlinkyIndicator(s[i], player[i]->color) );
			handler = new OrbitHandler(Pl,Vector2(size.x/2,size.y/2),random(PI2), (SpaceLocation *)Pl,
				(SpaceLocation *)s[i], 400, 10.0*ANGLE_RATIO,0);
			add(s[i]);
			game->add(handler);
		}

		died[i] = 0;
	}



	// planets & stations code
	int g,m,s;
	log_file("sarena.ini");
	g = get_config_int(NULL, "Planet", 0);
	if (g > 0) {
		Pl = new Planet(Vector2(size.x/2, size.y/2), meleedata.planetSprite, random()%meleedata.planetSprite->frames());
		add(Pl);
		s = get_config_int(NULL, "Station", 0);
		if (s > 0) {
			Ship *Station1 = create_ship(channel_none, "staba", "WussieBot", Vector2(size.x/2, size.y/2), 1, station_team);
			handler = new OrbitHandler(Pl,Vector2(size.x/2,size.y/2),random(PI2), (SpaceLocation *)Pl,
			(SpaceLocation *)Station1, 400, 10.0*ANGLE_RATIO,0);
			add(Station1);
			game->add(handler);
		}
	}

	log_file("sarena.ini");
	g = get_config_int(NULL, "Oplanets", 0);
	s = get_config_int(NULL, "Ostations", 0);
	for (m = 0; m < g; m += 1) {
		Pl = new Planet(Vector2(random() % (int)size.x, random() % (int)size.y), meleedata.planetSprite, random()%meleedata.planetSprite->frames());
		add(Pl);
		if (s>0) {
			Ship *Station1 = create_ship(channel_none, "staba", "WussieBot", Vector2(size.x/2, size.y/2), 1, station_team);
			handler = new OrbitHandler(Pl,Vector2(size.x/2,size.y/2),random(PI2), (SpaceLocation *)Pl,
			(SpaceLocation *)Station1, 400, 10.0*ANGLE_RATIO,0);
			add(Station1);
			game->add(handler);
		}
		s = s-1;
	}

	Planet *c;
//	load_datafile("melee.dat");
	log_file("sarena.ini");
	m = get_config_int(NULL, "Comets", 2);
	for (g = 1; g < m; g += 1) {
		c = new Planet (Vector2(random()%int(size.x), random()%int(size.y)), meleedata.kaboomSprite, 1);
		c->mass = get_config_int(NULL, "Comet_mass", 2);
		c->gravity_force *= 0;
		c->gravity_whip = 0;
		c->accelerate(NULL, random(PI2), get_config_int(NULL, "Comet_acc", 2), get_config_int(NULL, "Comet_max", 2));
		add (c);
		}

	//if (human_panel[0]) human_panel[0]->locate(view->view_x + view->view_w, 0);   //if the panel still exists, position on the right-hand side
	//if (human_panel[1]) human_panel[1]->locate(view->view_x + view->view_w, 100); //same for the other panel, if there is another panel

	redraw();
	respawn_time = -1;
	started = 1;
   return;
	}

//void SuperArena::new_ship(int team, int human) {



void SuperArena::init(Log *_log)
{
	STACKTRACE;

	// there should be exactly <num_teams> players/bots
	log_file("sarena.ini");
	num_teams = get_config_int(NULL, "Teams", 2);

	if (num_players < num_teams)
	{
		num_bots += (num_teams - num_players);
	}
	
	if (num_players > num_teams)
		num_bots = 0;

	num_players = num_teams;	// this may mean, people can be ignored this way??


	
	NormalGame::init(_log);
	
	// override normal player settings...
	int i, j;
	int n;
	for ( n = 0; n < num_players; ++n )
	{
		char name[512];
		sprintf(name, "Team%i", n);
		player[n]->team = new_team();// get_config_int(NULL, name, 2);
	}
	station_team = new_team();

//	int j,i;
	log_file("sarena.ini");
	j = get_config_int(NULL, "Size", 3840);
	view->window->locate(0,0,0,0,0,0.9,0,1);
	size.x = j;
	size.y = j;
	//width & height are the dimensions of the game map
	//they should be modified before prepare() is called
	//If for some reason you modify them later, call prepare() again
	//but you probably shouldn't be modifying them later.
	//The normal size is 3840x3840
	prepare();
	
	//log_file("sarena.ini");
	add(new Stars());
	
	/*
	human_control[0] = create_control(channel_server, "Human");
	
	if ((glog->type == Log::log_net1server) || (glog->type == Log::log_net1client)) {
		human_control[1] = create_control(channel_network[1], "Human");
	}
	else human_control[1] = NULL;
	*/
	
	// asteroids code
	log_file("sarena.ini");
	j = get_config_int(NULL, "Asteroids", 2);
	if (j > 0) for (i = 1; i < j; i += 1) add(new Asteroid());
	else {
		j = get_config_int(NULL, "Size", 2);
		for (i = 0; i < j/900; i += 1) add(new Asteroid());
	}
//	human_panel[0] = NULL;
//	human_panel[1] = NULL;
	
	change_view("Hero");
	//sets it to Hero view mode
	view_locked = false;
	//prevents the view mode from being changed through the menu
	
	respawn_time = 500;
	
	message.out("Welcome to Teams Arena!", 3000, 15);
}

REGISTER_GAME(SuperArena, "Super Arena (.INI)")
