/* $Id$ */ 

/* melee with fixed fleets ... */

#include <allegro.h>
#include <stdio.h>
#include <string.h>
#include "../melee.h"
#include "../other/radar.h"
#include "../melee/mshppan.h"

REGISTER_FILE
#include "../melee/mframe.h"
#include "../melee/mgame.h"
#include "../melee/mmain.h"
#include "../melee/mcbodies.h"
#include "../melee/mview.h"
#include "../other/orbit.h"
#include "../melee/mitems.h"
#include "../util/aastr.h"
#include "../melee/mcontrol.h"
//#define STATION_LOG


#include "../melee/mlog.h"

#include "twgui/twgui.h"

#include "../melee/mfleet.h"

#include "../other/ttf.h"

#define Num_Planet_Pics 7

static const int max_players = 128;
static const int max_fleet_ships = 32;

class SC1Arena : public Game
{
public:
	int healthtoggle;

	//Fleet fleet[max_players];
	Ship *playership[max_players];
	bool ready2spawn[max_players];
	int count_spawn[max_players];

	int player_shipchoice;
	int lastkeys;

	virtual void init(Log *_log);

	bool GetSprites(SpaceSprite *Pics[], char *fileName, char *cmdStr, int numSprites, int attribs);
	SpaceSprite *GetSprite(char *fileName, char *spriteName, int attribs);

	virtual void calculate();
	virtual void animate( Frame* frame );
	virtual void animate_predict(Frame *frame, int time);

	virtual void quit(const char *message);

	void start_menu(int *select, char *title);

	virtual bool handle_key(int k);

	// event:
	int spawn_ship_playernum;
	int spawn_ship_fleetindex;
	void spawn_a_ship(int iplayer);

	void spawn_a_ship2(int iplayer, int iship);

	virtual void register_events();
};



void SC1Arena::init(Log *_log)
{
	STACKTRACE;

	// this also initializes the players
	Game::init(_log);

	//prepare needs to be called before you add items, or do physics or graphics or anything like that.  Don't ask why.  
	prepare(); 



	// set the anti-alias to 0:
	int antia_on =  get_config_int(NULL, "Anti-AliasingOn",0);;
	set_tw_aa_mode(antia_on);


	add(new Stars());
//	char starName[100];

	// add some environment
	
	Planet *planet = create_planet();
	
	add(new WedgeIndicator(planet, 75, 4));

	int num_asteroids = 8;
	int i;
	for (i = 0; i < num_asteroids; i += 1)
		add(new Asteroid());



	FULL_REDRAW = false;

	log_resetmode();

	int v1 = 99;
	int v2 = 88;
	share(0, &v1);
	share(1, &v2);
	share_update();

	set_config_file("gflmelee.ini");

	int choice_fleet[max_players];
	char txt[64];
	sprintf(txt, "player(human) %i allegiance", p_local);
	start_menu(&choice_fleet[p_local], txt);

	// send (or receive) ... channel_server is locally either the server, or the client.
	// fleet numbers are fixed: fleet 0 vs fleet 1


	for ( i = 0; i < num_network; ++i )
	{

		//channel_current = channel_network[i];
		share(i, &choice_fleet[i]);
		//log_int(choice_fleet[i]); do not use log-int here, since you've to actually send data
	}

	// and the bots ?
	// only the server can set bots, so choose the channel server.
	for ( i = num_network ; i <  num_players; ++i )
	{
		if (!player[i])
			continue;

		if (hostcomputer())
		{
			char txt[64];
			sprintf(txt, "player(bot) allegiance %i", i);
			// these are chosen by the server...
			start_menu(&choice_fleet[i], txt);
		}
		share(0, &choice_fleet[i]);
	}
	share_update();

	


	// initialize the fleets from the ini file ...

	int ifleet;

	set_config_file("gflmelee.ini");

	for ( ifleet = 0; ifleet < num_players; ++ifleet )
	{
		if (!player[ifleet])
			continue;

		char ident[512];
		sprintf(ident, "Alliance%02i", choice_fleet[ifleet]+1);

		// scan this section for the ships that are available ...
		int Nships;
		Nships = get_config_int(ident, "Nships", 0);

		if (Nships > max_fleet_ships)
			tw_error("Too many ships in this fleet");

		int iship;
		for ( iship = 0; iship < Nships; ++iship )
		{
			char identshp[512] = "";
			sprintf(identshp, "ship%02i", iship + 1);

			// note, we've to store shipname in memory, cause
			// the ini operations in the ship-init overwrite value in st.
			char shipname[512];
			const char *st;
			st = get_config_string(ident, identshp, 0);
			strcpy(shipname, st);

			player[ifleet]->fleet->addShipType(shiptype(shipname));

		}
	}

	// start with nothing for each player...

	for ( i = 0; i < num_players; ++i )
	{
		if (!player[i])
			continue;

		// the player starts with no ship
		playership[i] = 0;

		// initially, you're allowed to choose a ship, cause you got no ship yet!
		ready2spawn[i] = true;

		count_spawn[i] = 0;


		// is already done in Game::init_players
		// and also, define a team for the player
		//player[i]->team = new_team();

		// and, a color (for indicators)
		//player[i]->color = palette_color[player[i]->team];
	}


	player_shipchoice = 0;

	lastkeys = 0;

	healthtoggle = 0x0FFFFFFFF;

	spawn_ship_playernum = -1;
	spawn_ship_fleetindex = -2;


}



void SC1Arena::quit(const char *message)
{
	Game::quit(message);
}



void SC1Arena::animate_predict(Frame *frame, int time)
{
	STACKTRACE;

	Game::animate_predict(frame, time);

}


void SC1Arena::animate( Frame* frame )
{
	STACKTRACE;

	
	Game::animate( frame );

	FULL_REDRAW = false;

	// if a local player has to choose a new ship...
	int p;
	for ( p = 0; p < num_network; ++p )
	{
		if (!player[p])
			continue;

		if (player[p]->islocal() && !playership[p])
		{
			FULL_REDRAW = true;
			
			int xpos = 10;
			int ypos = screen->h / 2;
			// show all ships that are still available to you
			int i;
			for ( i = 0; i < player[p]->fleet->getSize(); ++i )
			{
				ShipType *t;
				t = player[p]->fleet->getShipType(i);
				
				if (!t->data->islocked())
					t->data->load();
				
				BITMAP *bmp = t->data->spriteShip->get_bitmap(0);
				masked_blit(bmp, frame->surface, 0, 0, xpos, ypos, bmp->w, bmp->h);
				
				if (i == player_shipchoice)
				{
					rect(frame->surface, xpos, ypos, xpos+bmp->w-1, ypos+bmp->h-1, makecol(200,200,200));
				}
				
				xpos += bmp->w + 5;
				
			}
		}

	}
}





void SC1Arena::start_menu(int *select, char *titletext)
{
	STACKTRACE;;
///*

//	*select = 0;
//	return;

	unscare_mouse();
	show_mouse(window->surface);

	int i;



	FONT *usefont;// = videosystem.get_font(i);

	int points = 24 * (screen->w/800.0);
	usefont = load_ttf_font("fonts/jobbernole.ttf", points, 0, 1.5);
	reset_color_props();

//	view->frame->prepare();



//	AreaReserve *A;
	TWindow *A;

	// this is needed for a non-flickering image
	BITMAP *temp_screen = 0;
	temp_screen = create_video_bitmap(screen->w, screen->h);
	if (!temp_screen)
		temp_screen = create_bitmap(screen->w, screen->h);

	// this uses a log_file as well ... to determine the ref screen size.
	A = new TWindow("interfaces/sc1arena", 0, 0, temp_screen);

	// other stuff resets the log file, so make sure you got the correct one.
	set_config_file("gflmelee.ini");
	//log_file("gSC1Arena.ini");
	// don't use log_file, cause that's in memory, while we'd like to save settings on disk.

	int Nfleets;
	Nfleets = get_config_int("FleetInit", "Nalliances", 0);

	if ( Nfleets == 0 )
	{
		tw_error("No fleets defined");
	}

	// check all the fleet names

	// at most 99 fleets ... seems safe enough
	char flname[99][64];
	for ( i = 0; i < Nfleets; ++i )
	{
		char allyid[64];
		sprintf(allyid, "Alliance%02i", i+1);
		strncpy(flname[i], get_config_string(allyid, "Name", "<undefined>"), 60);
	}


	int col;
	Button *prev, *next, *ok;
	TextButton *title;
	TextButton *fl;

	col = makecol(0,0,0);
//	col[1] = makecol(200,200,200);

	prev = new Button(A, "prev_");
	next = new Button(A, "next_");

//	up[1] = new Button(A, "up2_");
//	down[1] = new Button(A, "down2_");

	ok = new Button(A, "ok_");

	fl = new TextButton(A, "text_", usefont);		
	fl->set_text("", col);

	title = new TextButton(A, "title_", usefont);		
	title->set_text(titletext, col);

	A->tree_doneinit();

//	fl[1] = new TextButton(A, "text2_", usefont);		
//	fl[1]->set_text("", col[1]);

	// the window manager.

//	WindowManager *winman;
//	winman = new WindowManager;
//	winman->add(A);

	//int select[2];
	*select = get_config_int("FleetInit", "PlayerAlliance", 0);
//	select[1] = get_config_int("FleetInit", "EnemyAlliance", 0);

	if (*select < 0)
		*select = 0;
	if (*select > Nfleets-1)
		*select = Nfleets-1;

	A->tree_doneinit();
	//A->tree_setscreen(temp_screen);

	while ( !ok->flag.left_mouse_press )
	{
		idle(5);
		A->tree_calculate();
		A->tree_animate();
		blit(temp_screen, screen, 0, 0,  0, 0, temp_screen->w, temp_screen->h);

		if (next->flag.left_mouse_press)
		{
			++*select;
			if ( *select > Nfleets-1 )
				*select = 0;
		}

		if (prev->flag.left_mouse_press)
		{
			--*select;
			if ( *select < 0 )
				*select = Nfleets-1;
		}
		
		fl->set_text(flname[*select], col);


	}

	set_config_int("FleetInit", "PlayerAlliance", *select);
//	set_config_int("FleetInit", "EnemyAlliance", select[1]);

	show_mouse(0);
	destroy_bitmap(temp_screen);
	show_mouse(screen);

}



void SC1Arena::spawn_a_ship2(int iplayer, int iship)
{
	count_spawn[iplayer] += 1;

	int N = player[iplayer]->fleet->getSize();
	if ( iship < 0 || iship >= N )
	{
		tw_error("Error in fleet choice");
	}

	ShipType *t = player[iplayer]->fleet->getShipType(iship);
	if (!t)
	{
		tw_error("Mistake in transmitting fleet number");
	}
	Ship *s;
	s = create_ship(t->id, player[iplayer]->control,
		tw_random(map_size), tw_random(PI2),
		player[iplayer]->team);
	playership[iplayer] = s;
	
	player[iplayer]->fleet->clear_slot(iship);
	
	add ( new WedgeIndicator ( s, 30, player[iplayer]->team ) );	// this uses a palette index...
	
	add ( new HealthBar(s, &healthtoggle));
	
	add( s->get_ship_phaser() );
}

void SC1Arena::spawn_a_ship(int iplayer)
{
	// always use channel_current

	if (!log_synched)
	{

		// this is for write+read -mode
		// unfortunately, in read-mode, this gloabal var will be overwritten, thus upsetting the next write mode call... duh...
		log_int(spawn_ship_playernum);
		log_int(spawn_ship_fleetindex);

		if (spawn_ship_playernum != iplayer)
			tw_error("Player index mismatch in spawn_a_ship (write)");
		
		// disable spawning the ship again.
		ready2spawn[spawn_ship_playernum] = false;


	}
	else
	{
		int i, k;
		log_int(i);
		log_int(k);

		if (i != iplayer)
			tw_error("Player index mismatch in spawn_a_ship (read)");

		// this happens in read-mode only
		spawn_a_ship2(i, k);

		// you're allowed to spawn a new ship (as soon as this ship is dead)
		ready2spawn[i] = true;
	}
}


//static bool has_registered = false;
void SC1Arena::register_events()
{
	Game::register_events();

	EVENT(SC1Arena, spawn_a_ship);
}



bool SC1Arena::handle_key(int k)
{
	Game::handle_key(k);

	switch (k >> 8)
	{
	case KEY_H:
		healthtoggle = ~healthtoggle;
		break;
	}

	return true;
}


void SC1Arena::calculate()
{
	STACKTRACE;

	Game::calculate();


	// and then what ... check if the fleets/ player ships are alive ?

	//message.print(1500, 14, "players[%i] network[%i]", num_players, num_network);

	int i;
	for ( i = 0; i < num_players; ++i )
	{
		if (!player[i])
			continue;

		if (playership[i] && !playership[i]->exists())
			playership[i] = 0;

		// bots: these should choose a new ship randomly
		if (!playership[i] && i >= num_network)
		{
			if (player[i]->fleet->getSize() > 0)
			{
				int k;
				k = tw_random( player[i]->fleet->getSize() );
				
				// issue an event to create the ship (not really needed here cause it's a bot but well...):
				spawn_ship_playernum = i;
				spawn_ship_fleetindex = k;
				if ( i < 0 || i > num_players)
				{
					tw_error("Shouldn't happen");
				}
				if ( k < 0 || k >= player[i]->fleet->getSize() )
				{
					tw_error("Error in fleet choice");
				}

				// do NOT use a CALL for this ... cause it's local, really ...
				spawn_a_ship2(i, k);
			}

		}
	}


	// the local player should choose a new ship manually ... note that in single-player mode,
	// there can be >1 local players, so:
	int p;
	for ( p = 0; p < num_network; ++p )
	{
		if (!player[p])
			continue;

		if (player[p]->islocal() && !playership[p])
		{
			int keys = player[p]->control->keys;
			
			if (player[p]->fleet->getSize() > 0)
			{
				
				if ((keys & keyflag::right)  &&  !(lastkeys & keyflag::right))
					++player_shipchoice;
				
				if ((keys & keyflag::left)  &&  !(lastkeys & keyflag::left))
					--player_shipchoice;
				
				if (player_shipchoice < 0)
					player_shipchoice = player[p]->fleet->getSize()-1;
				
				if (player_shipchoice > player[p]->fleet->getSize()-1)
					player_shipchoice = 0;
				
				
				if ((keys & keyflag::fire)  &&  !(lastkeys & keyflag::fire) && ready2spawn[p])
				{
					// issue an event to create the ship:
					spawn_ship_playernum = p;
					spawn_ship_fleetindex = player_shipchoice;
					
					if ( p < 0 || p >= num_network)
					{
						tw_error("Shouldn't happen");
					}
					CALL(spawn_a_ship, p);
					
				}
				
			} else {
				// you should be able to focus on another ship ...
				if ((keys & keyflag::right)  &&  !(lastkeys & keyflag::right))
				{
					// try to focus on the next available ship (enemy or not, it does not matter)
					// (not supported yet...)
				}
			}
			lastkeys = keys;
		}
	}
}


REGISTER_GAME (SC1Arena, "SC1 Arena");


