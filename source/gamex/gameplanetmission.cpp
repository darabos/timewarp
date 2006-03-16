/* $Id$ */ 
#ifdef NOTDEFINED

#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "melee.h"
REGISTER_FILE

#include "melee/mlog.h"
#include "melee/mcontrol.h"
#include "melee/mframe.h"
#include "melee/mview.h"
#include "melee/mitems.h"

#include "scp.h"
#include "util/history.h"


#include "subgame.h"




SubGame::SubGame()
:
Game()
{
	prev = 0;
	next = 0;
}

SubGame::~SubGame()
{
	// uhm, this should be such, that it doesn't call the destruction of the melee stuff
	// that is in the startgame ...
	// so, set the pointers to null first ...
	if (this != gamestart)
	{
		asteroidExplosionSprite = NULL;	// maybe it won't clean it up this time ?? yay ;) this works...
		asteroidSprite = NULL;
	}


}


void SubGame::play_iteration(unsigned int time)
{
	// re-initialize screens and game pointers ... very important ...
	prepare();

	if (is_paused()) unpause();
	
//	if(!sound.is_music_playing())
//		play_music();
	
	if ((next_tic_time <= time) && (next_render_time > game_time) && game_ready()) {
		calculate();
		if (auto_unload) unload_unused_ship_data();//expiremental
		log->flush();
		log->listen();
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
	else idle();
	while (keypressed())
		handle_key(readkey());
}



// some stuff I copied from play_game

void GameStart::addsubgame(SubGame *asubgame)
{
	asubgame->gamestart = this;

	//videosystem.window.lock();
	//clear_to_color(videosystem.window.surface, palette_color[8]);
	//videosystem.window.unlock();
	asubgame->preinit();
	asubgame->window = new VideoWindow;
	asubgame->window->preinit();
	asubgame->window->init(&videosystem.window);
	asubgame->window->locate(0,0,0,0,0,1,0,1);
	asubgame->init(log);							// or, should you have your own log ?

	subgame[Nsubgames] = asubgame;
	++Nsubgames;

}

void GameStart::removesubgame(int k)
{
	subgame[k]->log->deinit();

	game = NULL;
	delete subgame[k];

	int i;
	for ( i = k; i < Nsubgames-1; ++i )
		subgame[i] = subgame[i+1];

	--Nsubgames;
}


void GameStart::play()
{
	set_resolution(window->w, window->h);

	prepare();
	if (is_paused()) unpause();

	Nsubgames = 1;
	subgame[0] = this;

	addsubgame(new GamePlanetView());

	while(!subgame[0]->game_done)
	{
		unsigned int time = get_time();
		poll_input();

		videosystem.poll_redraw();

		// iterate once for "this" game ... just in case it's needed ?
		//play_iteration(time);

		// and now, game-specific stuff ??

		int i;
		for ( i = Nsubgames-1; i < Nsubgames; ++i )
		{
			subgame[i]->play_iteration(time);
		}

		if (subgame[Nsubgames-1]->game_done && Nsubgames > 1)		// this occurs after a call to quit()
		{
			// remove the game from the list.
			removesubgame( Nsubgames-1 );
			//--Nsubgames;
		}
	}
	

	/*
	// the old way ;)
	// (this works btw.)
	while(!game_done)
	{
		unsigned int time = get_time();
		poll_input();
		videosystem.poll_redraw();

		play_iteration(time);
	}
	*/
}



// nothing to calculate or animate, it's just a basic framework
// (a bit of a waste of the screen, but well...)

void GameStart::calculate()
{
	Game::calculate();
}

void GameStart::animate(Frame *frame)
{
	Game::animate(frame);
}

void GameStart::animate()
{
	Game::animate();
}















void GamePlanetView::init(Log *_log)
{
	int i;

	game_done = false;
	log = _log;
	if (!log) {
		log = new Log();
		log->init();
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


/* CONTENTS OF CHANNEL channel_init :

offset	size	format		data
0		4		int			log type number
4		4		int			size of game type name
8		?		char[]		game type name
?		4		int			lag frames

*/

/* this does not make mucho sense ... I think ...
	int tmp = log->type;
	log_int(channel_init, tmp);				// ?? huh ??
	if (log->playback) log->type = tmp;

	char buffy[128];
	i = strlen(type->name);					// ?? crash ?? well, a game-name is useless here
	memcpy(buffy, type->name, i);
	if (i > 127) tw_error("long gamename1");
	log_int (channel_init, i);
	if (i > 127) tw_error("long gamename2");
	log_data(channel_init, buffy, i);
	buffy[i] = 0;
	if (strcmp(buffy, type->name)) tw_error("wrong game type");
*/

	// randomization is done "globally", so a subgame doesn't need to repeat that ?
	/*
	i = rand();
//	i = 9223;
	log_int(channel_server, i);
	random_seed[0] = i;
	tw_random_seed(i);
	i = rand();
//	i = 7386;
	log_int(channel_server, i);
	random_seed[1] = i;
	tw_random_seed_more(i);
	*/

	if (!is_paused()) pause();

	text_mode(-1);

	set_config_file("client.ini");
	msecs_per_fps = get_config_int("View", "FPS_Time", 200);
	msecs_per_render = int( get_config_float("View", "MinimumFrameRate", 10) + 0.5 );
	msecs_per_render_max = 100;
	prediction = get_config_int("Network", "Prediction", 50);
	if ((prediction < 0) || (prediction > 100)) {tw_error ("Prediction out of bounds (0 < %d < 100)", prediction);}

	log_file("server.ini");
	camera_hides_cloakers = get_config_int("View", "CameraHidesCloakers", 1);
	time_ratio = (int)(1000. / get_config_float ("Game", "SC2FrameRate", 20));
	distance_ratio = (3840. / get_config_float ("Game", "SC2TotalDistance", 8000));
	frame_time = (int)(1000. / get_config_float ("Game", "TicRate", 40) + 0.5);
	normal_turbo = get_config_float("Game", "Turbo", 1.0);
	f4_turbo = get_config_float("Game", "F4Turbo", 10.0);	
	turbo = normal_turbo;
	friendly_fire = get_config_int("Game", "FriendlyFire", 0) == 0 ? 0 : 1; //MSVC sucks also
	shot_relativity = get_config_float("Game", "ShotRelativity", 0);
	size = Vector2 (
		get_config_float("Game", "MapWidth", 0),
		get_config_float("Game", "MapHeight", 0)
	);

//	init_lag();
	log_int(channel_server, lag_frames);
	log_int(channel_init, lag_frames);

	tic_history = new Histograph(128);
	render_history = new Histograph(128);

	prepare();


	// add some asteroids ??

	
	// copy enough sprite and sound information to allow an asteroid to be constructed...
	asteroidSprite = gamestart->asteroidSprite;
	asteroidExplosionSprite = gamestart->asteroidExplosionSprite;
	
	for ( i = 0; i < 5; ++i )
	{
		// I guess this is ok?
		add(new Asteroid());
	}

	return;
}


void GamePlanetView::calculate()
{
	Game::calculate();
}


void GamePlanetView::animate(Frame *frame)
{
	Game::animate(frame);
}

void GamePlanetView::animate()
{
	Game::animate();
}



void GamePlanetView::useplanet(solarsystem_str::planet_str *planet)
{
	/*
	// add this planet, and the moons, to the map:
	int i;
	planet->o->attributes &= ~ATTRIB_INGAME;
	add(planet->o);
	for ( i = 0; i < planet->Nmoons; ++i )
	{
		add(planet->moon[i].o);
	}
	*/

	// safer test: add some asteroids

	int i;
	for ( i = 0; i < 5; ++i )
	{
		add(new Asteroid());
	}
}













SpaceObject_ext::SpaceObject_ext(SpaceLocation *ocreator, Vector2 opos, double oangle, SpaceSprite *osprite)
:
SpaceObject(ocreator, opos, oangle, osprite)
{
	obj_collision = 0;

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;

	mass = 1.0;		// you _need_ mass for collisions !!

	layer = LAYER_CBODIES;
	set_depth(DEPTH_ASTEROIDS);
	//id    |= ID_ASTEROID;

	attributes |= ATTRIB_STANDARD_INDEX;
}


void SpaceObject_ext::inflict_damage(SpaceObject *other)
{
	obj_collision = other;
	SpaceObject::inflict_damage(other);
}




// ----------------- AND NOW THE IMPLEMENTATION PART -------------------
// (where you use it: a Button with a specific function)



// ------------- the Game where I test this button -----------



/*
From the Allegro manual: how to use the scancodes :
int readkey();
Returns the next character from the keyboard buffer, in ASCII format. 
If the buffer is empty, it waits until a key is pressed. The low byte of 
the return value contains the ASCII code of the key, and the high byte 
the scancode. The scancode remains the same whatever the state of the 
shift, ctrl and alt keys, while the ASCII code is affected by shift and 
ctrl in the normal way (shift changes case, ctrl+letter gives the 
position of that letter in the alphabet, eg. ctrl+A = 1, ctrl+B = 2, 
etc). Pressing alt+key returns only the scancode, with a zero ASCII 
code in the low byte. For example: 


      if ((readkey() & 0xff) == 'd')         // by ASCII code
         printf("You pressed 'd'\n");

      if ((readkey() >> 8) == KEY_SPACE)     // by scancode
         printf("You pressed Space\n");

      if ((readkey() & 0xff) == 3)           // ctrl+letter
         printf("You pressed Control+C\n");

      if (readkey() == (KEY_X << 8))         // alt+letter
         printf("You pressed Alt+X\n");


*/



// ---- maybe it's best to make this into a spaceobject ?? -----

solarsystem_str solarsystem[1];


SpaceObject *addsolarobject(Vector2 opos, DATAFILE *dat, char *basename, int num)
{
	char tempstr[128];
	SpaceSprite *sprite;
	SpaceObject *o;

	strcpy(tempstr, basename);
	sprintf(&tempstr[strlen(tempstr)], "%02i", num+1);
	strcat(tempstr, "_BMP");

	DATAFILE *tmpdat = find_datafile_object(dat, tempstr);
	if (!tmpdat){tw_error("no data file object moon");}

	sprite = new SpaceSprite(tmpdat, 1, SpaceSprite::MASKED | SpaceSprite::MIPMAPED, 1);
	
	o = new SpaceObject(NULL, opos, 0.0, sprite);

	o->collide_flag_anyone = ALL_LAYERS;
	o->collide_flag_sameteam = ALL_LAYERS;
	o->collide_flag_sameship = ALL_LAYERS;
	o->mass = 999.0;

	return o;
}


SpaceObject_ext *addsolarship(Vector2 opos, DATAFILE *dat, char *basename)
{
	char tempstr[128];
	SpaceSprite *sprite;
	SpaceObject_ext *o;

	strcpy(tempstr, "SHIP_");
	strcat(tempstr, basename);
	strcat(tempstr, "_BMP");

	DATAFILE *tmpdat = find_datafile_object(dat, tempstr);
	if (!tmpdat)
	{
		tw_error("no data file object solar -- ship");
	}

	sprite = new SpaceSprite(tmpdat, 1, SpaceSprite::MASKED | SpaceSprite::MIPMAPED, 64);
	
	o = new SpaceObject_ext(NULL, opos, 0.0, sprite);

	return o;
}


void TWgui::init_solarsystems()
{
	// initialize sprite data, and also system information... all from a dat file ?!
	SpaceObject *o;

	DATAFILE *dat;

	dat = load_datafile("twgui_solar.dat");
	if (!dat) {tw_error("no data file");}

	// the solar system


	
	
	// this solar system has suns
	solarsystem[0].Nsuns = 1;
	solarsystem[0].sun = new solarsystem_str::sun_str [solarsystem[0].Nsuns];

	int k;
	for ( k = 0; k < solarsystem[0].Nsuns; ++k )
	{
		solarsystem_str::sun_str *s = &solarsystem[0].sun[k];

		// this sun has stats
		strcpy(s->name, "a sun");
		s->position = 0.0 * solarsystemsize * Vector2(1.0, 1.0);
		s->type = k+1;
	
		// and also data
		s->o = addsolarobject(s->position, dat, "SUN", k);
		add(s->o);
		s->o->collide_flag_anyone = 0;		// don't "hit" the sun ... (or ??)
		s->o->collide_flag_sameteam = 0;
		s->o->collide_flag_sameship = 0;

		// this sun has planets
		s->Nplanets = 3;
		s->planet = new solarsystem_str::planet_str [s->Nplanets];

		int i;
		for ( i = 0; i < s->Nplanets; ++i )
		{
			solarsystem_str::planet_str *p = &s->planet[i];

			// this planet has stats
			strcpy(p->name, "a planet");
			p->position = s->position +
							tw_random(solarsystemsize/3) * unit_vector(tw_random(2*PI));
			p->type = i+1;

			// and also data
			p->o = addsolarobject(p->position, dat, "PLANET", i);
			add(p->o);

			// this planet has moons
			p->Nmoons = 2;
			p->moon = new solarsystem_str::moon_str [p->Nmoons];
			
			int j;
			for ( j = 0; j < p->Nmoons; ++j )
			{
				solarsystem_str::moon_str *m = &p->moon[j];
				
				// this moon has stats
				strcpy(m->name, "a moon");
				m->position = p->position +
							tw_random(planetsystemsize/2) * unit_vector(tw_random(2*PI));
				m->type = j+1;

				// and also data
				m->o = addsolarobject(m->position, dat, "MOON", j);
				// but, do _not_ add the moon data to the game, just keep them in
				// memory, in case they're needed for a sub-game.
			}
			
		}

	}


	// (for later)
	// the flag"ship" ??
	flagship = addsolarship(Vector2(0.0, -solarsystemsize/4), dat, "FLAGSHIP");
	add(flagship);

	// some enemy ??

	unload_datafile(dat);



	// for calculations and drawing, things are scaled in space, and time:

//	refscaletime = 1.0;
	refpos = Vector2(0.0, 0.0);


	// put the focus somewhere ...
	// and define the zoom level ??!!
	view->camera.pos = Vector2(0.0, 0.0);
	view->camera.z = solarsystemsize;		// this shows the whole solar system (I think)

}


void TWgui::init(Log *_log)
{
	Game::init(_log);

	// size of the solar system:
	solarsystemsize = 4000.0;
	planetsystemsize = 1.0;		// yeah, very small ... can be scaled later on.
	size.x = solarsystemsize;
	size.y = solarsystemsize;

	// call after size is defined.
	prepare();

//	add(new Stars());

	// read solar system information from disk
	init_solarsystems();


}






void TWgui::calculate()
{

	Game::calculate();

	winman->menu_add_keys(frame_time);

	// can the ship move:
/*
	refscaletime = 1.0;
	if (next)		// IN CASE THAT ANOTHER GAME (spawned by this game) IS RUNNING :
	{
		// -------- hmm, there's also something like a game::play function
		// -------- what about the values it calculates ??


		next->calculate();
		// and, scale the times, if required by "child" games ... which
		// may run in a very different timescale (could even be inf. large,
		// meaning scale=0.
		refscaletime *= next->refscaletime;
	}

	if (refscaletime == 0)
		return;

	frame_time *= refscaletime;
*/
	double dt = 1E-3*frame_time;

	// ------------ CALCULATIONS INSIDE THE SOLAR SYSTEM ------------------



	if (key[KEY_UP])
		flagship->accelerate(NULL, flagship->angle, 0.2*dt, 1.0);
	if (key[KEY_RIGHT])
		flagship->angle += PI*dt;
	if (key[KEY_LEFT])
		flagship->angle -= PI*dt;
	


	/*
	
	// check if the flagship is close to any planet ... in that case, spawn
	// a new game ?!?!

	if (flagship->obj_collision)
	{
		// check which planet it collided with :
		
		solarsystem_str::sun_str *s;
		solarsystem_str::planet_str *p;

		s = &solarsystem[0].sun[0];

		// check ??

		// spawn a new game around that planet, and disable interaction of the flagship
		// in the original game:

		flagship->collide_flag_anyone = 0;
		flagship->collide_flag_sameteam = 0;

		next = new GamePlanetView();
		next->prev = this;
		
		next->preinit();
		next->init(log);
		((GamePlanetView*)next)->useplanet(&s[0].planet[0]);

		// reset this...
		flagship->obj_collision = 0;
	}

  */
  
	/*
	solarsystem_str::sun_str *s = &solarsystem[0].sun[0];
	int i;
	for (i = 0; i < s->Nplanets; ++i )
	{
		if ( flagship->distance(s->planet[i].o) < 100.0 && flagship->collide_flag_anyone )
		{
			flagship->collide_flag_anyone = 0;	// make it untouchable
//			next = new PlanetView();
//			((PlanetView*)next)->useplanet(s->planet);
		}
	}
	*/
	


	// ------------ INTERPRET RESULTS FROM THE MELEE WINDOW ---------------


	FULL_REDRAW = true;
	// only a full_redraw draws the whole frame-surface onto the physical screen.
	// (otherwise only the small sprites are drawn there ... I think)
	// I guess this can be a bit slow, but, I don't know why it's slow... perhaps simply
	// because it's a big screen, or is it because of conversion between color depths?

	winman->calculate();

	if (zoomin->flag.left_mouse_hold)
	{
		// copied this from view_hero :
		//if (key_pressed(key_zoom_in))  n.z /= 1 + 0.002 * frame_time;
		//if (key_pressed(key_zoom_out)) n.z *= 1 + 0.002 * frame_time;
		view->camera.z /= 1 + 0.002 * frame_time;
	}
	if (zoomout->flag.left_mouse_hold)
	{
		view->camera.z *= 1 + 0.002 * frame_time;
	}

	if (abort->flag.left_mouse_press)
	{
		
//		pause();
//		// replace this with some tw_alert_menu, with buttons (and some text perhaps).
//		if (tw_alert("Game is paused", "&Abort game", "&Resume playing") == 1) {
//			game->quit("quit - Game aborted from keyboard");
//		}
//		unpause();

		meleemenu->focus2other(alertmenu);
//		meleemenu->disable();			// this menu won't generate signals now, but it'll be drawn still
//		alertmenu->show();
		alert_text->set_text("Quit game?", makecol(50,50,50));
	}


	// ------------ INTERPRET RESULTS FROM THE ALERT WINDOW ---------------
	// (if the alert window is active, that is).


	// can an overall menu return a value ??
	if (alert_yes->flag.left_mouse_press)
		game->quit("quit - Game aborted from keyboard");

	if (alert_no->flag.left_mouse_press)
	{
		alertmenu->back2other();
//		alertmenu->hide();		// this menu is disabled, and won't be drawn
//		meleemenu->enable();	// switch control back to the meleemenu
		FULL_REDRAW = true;		// redraw the whole scene next iteration.
	}



	// Some more calculations needed.

	// this makes sure that you don't zoom out more than the size of the solar system.
	if (view->camera.z > solarsystemsize)
		view->camera.z = solarsystemsize;

	// also, no need to zoom in too far I think.
	if (view->camera.z < 0.25 * solarsystemsize)
		view->camera.z = 0.25 * solarsystemsize;

}





REGISTER_GAME ( GameStart, "gamestart" );


#endif
