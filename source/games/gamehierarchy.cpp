/* $Id$ */ 
#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE

#include "../melee/mlog.h"
#include "../melee/mcontrol.h"
#include "../melee/mframe.h"
#include "../melee/mview.h"
#include "../melee/mitems.h"

#include "../scp.h"
#include "../util/history.h"


#include "gamehierarchy.h"



void IterGame::play_iteration(unsigned int time)
{
	STACKTRACE

	// re-initialize screens and game pointers ... very important ...
	prepare();

	if (is_paused()) unpause();
	
//	if(!sound.is_music_playing())
//		play_music();
	
	if ((next_tic_time <= time) && (next_render_time > game_time) &&
		(game_ready() || game_time == 0))	// note that game_time==0 is also needed, cause otherwise it'll wait for data, while no data've been generated yet.
	{		
		_STACKTRACE("Game::play - Game physics");
		
//		gen_buffered_data();
		glog->flush_noblock();
		glog->listen();
		
		calculate();
		
		if (auto_unload) unload_unused_ship_data();//expiremental

		if (key[KEY_F4])
			turbo = f4_turbo;
		else
			turbo = normal_turbo;
		next_tic_time += (frame_time / turbo);
		if ((hiccup_margin >= 0) && (next_tic_time + hiccup_margin < get_time()))
			next_tic_time = get_time();
		if (next_fps_time <= game_time) {
			next_fps_time += msecs_per_fps;
			fps();
		}
	}
	else if (interpolate_frames || (game_time > next_render_time - msecs_per_render)) {
		animate();
		next_render_time = game_time + msecs_per_render;
	}
	else idle(1);
	while (keypressed())
		handle_key(readkey());
}








// some stuff I copied from play_game

void MainGame::addsubgame(SubGame *asubgame)
{
	STACKTRACE

	asubgame->maingame = this;

	// clear the log ...
	glog->reset();

	// clear the events .. and reread this for this at least ?
	events.clear();	// hmmm...
	//register_events();

	//videosystem.window.lock();
	//clear_to_color(videosystem.window.surface, palette_color[8]);
	//videosystem.window.unlock();
	asubgame->preinit();
	asubgame->window = new VideoWindow;
	asubgame->window->preinit();
	asubgame->window->init(&videosystem.window);
	asubgame->window->locate(0,0,0,0,0,1,0,1);

	asubgame->init(glog);	// all subgames share the same log (is this ok?)
	// this init should reference the already-initialized melee-data of the maingame



	subgame[Nsubgames] = asubgame;
	++Nsubgames;
}


void MainGame::removesubgame(int k)
{
	STACKTRACE

	glog->deinit();	// this is empty, dunno why

//	DATAFILE *d = melee;	// remember this important pointer.

	game = NULL;

	delete subgame[k];	// this _must_ be deleted completely, otherwise the gamedata aren't removed

//	melee = d;	// restore this pointer !!


	int i;
	for ( i = k; i < Nsubgames-1; ++i )
		subgame[i] = subgame[i+1];

	--Nsubgames;

}


void MainGame::play()
{
	STACKTRACE

	set_resolution(window->w, window->h);

	prepare();
	if (is_paused()) unpause();

	Nsubgames = 1;
	subgame[0] = this;

	while(!subgame[0]->game_done)
	{
		unsigned int time = get_time();
		poll_input();

		videosystem.poll_redraw();

		// and now, game-specific stuff ??

		int i;
		for ( i = Nsubgames-1; i < Nsubgames; ++i )
		{
			subgame[i]->play_iteration(time);
		}

		if (subgame[Nsubgames-1]->game_done && Nsubgames > 1)		// this occurs after a call to quit()
		{
			// remove the game from the list.
			removesubgame( Nsubgames-1 );		// by default, remove every trace from this game type
			//--Nsubgames;
		}
	}
	
}

















SubGame::SubGame()
:
IterGame()
{
//	prev = 0;
//	next = 0;
}

SubGame::~SubGame()
{
	// uhm, this should be such, that it doesn't call the destruction of the melee stuff
	// that is in the startgame ...
	// so, set the pointers to null first ...
//	if (this != maingame)	// comparison isn't needed, since it's always this type of game.
//	{

	/*
		// the purpose of setting those things to NULL is to prevent them
		// from being deleted.

		meleedata.panelSprite             = NULL;
		meleedata.kaboomSprite            = NULL;
		meleedata.hotspotSprite           = NULL;
		meleedata.sparkSprite             = NULL;
		meleedata.asteroidExplosionSprite = NULL;
		meleedata.asteroidSprite          = NULL;
		meleedata.planetSprite            = NULL;
		meleedata.xpl1Sprite              = NULL;
		
		planet_victory			= NULL;
		
		music = NULL;


		melee = NULL;			// the all-purpose melee data block!!
		// hmm, if I set this to 0, it's not recognized anymore ...
		// therefore, this pointer restored after the deletion... by the main game
		// for now, the pointer=0, otherwise the data are deleted physically
//	}
		*/

}



void SubGame::init(Log *_log)
{
	STACKTRACE


	game_done = false;
	//log = _log;
	if (!glog) {
		glog = new Log();
		glog->init();
	}

	lag_frames = 0;
	show_fps = 0;
	game_time = 0;
	frame_time = 1;
	frame_number = 0;
	hiccup_margin = 100;
	next_tic_time = get_time();
	next_render_time = game_time;
	next_fps_time = game_time;
	view_locked = false;
	local_checksum = client_checksum = server_checksum = 0;

	Physics::init();
	prepare();

	if (!window) {
		window = new VideoWindow();
		window->preinit();
	}

	set_config_file("client.ini");
	change_view(get_config_string("View", "View", "Hero")); 

	window->add_callback(this);				// so .. a window can handle >2 games ??


	if (!is_paused()) pause();

	text_mode(-1);
	

	// start copying stuff ...


	set_config_file("client.ini");
	msecs_per_fps		= maingame->msecs_per_fps;
	msecs_per_render	= maingame->msecs_per_render;
	prediction			= maingame->prediction;
	if ((prediction < 0) || (prediction > 100)) tw_error ("Prediction out of bounds (0 < %d < 100)", prediction);

//	camera_hides_cloakers	= maingame->camera_hides_cloakers;	// this is a global variable
//	time_ratio				= maingame->time_ratio;			// this is a global variable
//	distance_ratio			= maingame->distance_ratio;		// this is a global variable
	frame_time				= maingame->frame_time;
	normal_turbo			= maingame->normal_turbo;
	f4_turbo				= maingame->f4_turbo;
	turbo = normal_turbo;
	friendly_fire			= maingame->friendly_fire;
	shot_relativity			= maingame->shot_relativity;

//	size					= maingame->size;
	// now, this is a parameter that each one of you should define yourself.

	//init_lag();								// this initializes lag_frames, but that's already done in maingame
	//log_int(channel_server, lag_frames);		// no need to exchange this info again
	//log_int(channel_init, lag_frames);
	lag_frames = maingame->lag_frames;

	tic_history = new Histograph(128);
	render_history = new Histograph(128);

	prepare();

	return;
}

