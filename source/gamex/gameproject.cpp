#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE


#include "gameproject.h"

#include "gamestarmap.h"
#include "edit/edit_dialogue.h"
#include "gamedialogue.h"


GameBare::GameBare()
{
	prev = 0;
	next = 0;
	state = true;

	gamerequest = 0;

	view = 0;
	window = 0;

	escapetime = 0;
}

GameBare::~GameBare()
{
}


void GameBare::quit()
{
	destroy_all();

	//message.out("other shit");
	message.flush();

	delete view;
	window->remove_callback(this);
	delete window;
	
	state = 0;
}


void GameBare::refocus()
{
}


bool GameBare::handle_key(int k)
{
	switch (k >> 8)
	{
	case KEY_ESC:
		quit();
		return true;
		break;
		
	}
	
	return false;
}




void GameBare::change_view(const char * name)
{
	View *v = get_view(name, view);
	if (!v)
		tw_error("Game::change_view - invalid view name");
	if (view)
		v->replace(view);
	else 
	{
		v->window->init(window);
		v->window->locate(
			0, 0,
			0, 0,
			0, 1,
			0, 1
			);
	}
	view = v;
	return;
}


void GameBare::init()
{
	Physics::preinit();		// default stuff that should be done in the constructor.
	Physics::init();

	if (!window)	// is true, if it's called immediately after the constructor
	{
		window = new VideoWindow;
		window->preinit();
		window->init(&videosystem.window);
		window->locate(0,0,0,0,0,1,0,1);
	}

	change_view("Hero"); 

	window->add_callback(this);

	game_time = 0;
	frame_time = 1;
	frame_number = 0;
	turbo = 1.0;

	next_tic_time = get_time();
	next_render_time = game_time;

	set_config_file("client.ini");

//	msecs_per_fps = get_config_int("View", "FPS_Time", 200);
	msecs_per_render = (int)(1000. / get_config_float("View", "MinimumFrameRate", 10) + 0.5);
//	prediction = get_config_int("Network", "Prediction", 50);
//	if ((prediction < 0) || (prediction > 100)) tw_error ("Prediction out of bounds (0 < %d < 100)", prediction);

//	log_file("server.ini");
//	camera_hides_cloakers = get_config_int("View", "CameraHidesCloakers", 1);
	time_ratio = (int)(1000. / get_config_float ("Game", "SC2FrameRate", 20));
	distance_ratio = (3840. / get_config_float ("Game", "SC2TotalDistance", 8000));
	frame_time = (int)(1000. / get_config_float ("Game", "TicRate", 40) + 0.5);
//	normal_turbo = get_config_float("Game", "Turbo", 1.0);
//	f4_turbo = get_config_float("Game", "F4Turbo", 10.0);	
//	turbo = normal_turbo;
	
	size = Vector2 (
		get_config_float("Game", "MapWidth", 0),
		get_config_float("Game", "MapHeight", 0)
	);	

	friendly_fire = get_config_int("Game", "FriendlyFire", 0) == 0 ? 0 : 1; //MSVC sucks also
	shot_relativity = get_config_float("Game", "ShotRelativity", 0);

	prepare();

	hardexit = 0;	// is set to 1 if the player exits the game.
}


/*
void GameBare::checksync()
{
	// nothing
}
*/

// copied from Game class
void GameBare::set_resolution(int screen_x, int screen_y)
{
	int view_x, view_y;
	view_x = screen_x;
	view_y = screen_y;
	redraw();
	return;
}

// copied from Game class
void GameBare::redraw()
{
	if (!window->surface) return;
	scare_mouse();
	window->lock();
	rectfill(window->surface, window->x, window->y, window->x+window->w-1, window->y+window->h-1, pallete_color[8]);
	FULL_REDRAW += 1;

	view->refresh();
	//view->animate_predict(this, 0);
	animate_predict(view->frame, 0);

	FULL_REDRAW -= 1;
	window->unlock();
	unscare_mouse();
	return;
}


void GameBare::animate(Frame *frame)
{
	Physics::animate(frame);
}


// copied (and simplified - no prediction) from Game class
void GameBare::animate()
{
	int predtime = 0;

	Frame *f = view->frame;

	if (FULL_REDRAW)
		f->full_redraw = true;

	f->erase();
	view->prepare(f, predtime);

	if (f->surface)
	{
		if (f->surface)
			animate(f);

		//message.animate(frame);	// displays messages that're stored somewhere
	}

	scare_mouse();
	f->draw();
	unscare_mouse();
	return;
}


void GameBare::idle(int time)
{
	::idle(time);
	return;
}

void GameBare::play_iteration(unsigned int time)
{
	// re-initialize GLOBAL screens and game pointers to point to THIS particular game.
	prepare();

	
	
	if ((next_tic_time <= time))// && (next_render_time > game_time))
	{
		calculate();

		if (!state)	// the screen and such are removed if this is the case.
			return;

//		if (auto_unload) unload_unused_ship_data();//expiremental
//		log->flush();
//		log->listen();
//		if (key[KEY_F4])
//			turbo = f4_turbo;
//		else
//			turbo = normal_turbo;
		next_tic_time += (frame_time / turbo);
//		if ((hiccup_margin >= 0) && (next_tic_time + hiccup_margin < get_time()))
//			next_tic_time = get_time();
//		if (next_fps_time <= game_time) {
//			next_fps_time += msecs_per_fps;
//			fps();
//		}
	}

	// well ... this is done almost always ... so do it always: move it up to the
	// highest level ...
	//else if (!next)
	//	idle();
	
	//else
	if (game_time > next_render_time - msecs_per_render)
	{
		animate();
		next_render_time = game_time + msecs_per_render;
	}
	//else idle();
	

	// this should only be used by the active game ... is that so ??
	// the active game, is the last one that was added to the list ...
	if (!next)
	{
		while (keypressed())
			handle_key(readkey());
	}


}



void GameBare::prepare()
{
	Physics::prepare();
	::physics = this;			// same as the game pointer ...
	return;
}



GameProject::GameProject()
{
	first = 0;
	last = first;
}


void GameProject::init()
{
	// empty project
	// (no subgames, no data structures, just plain boring nothing)

	int i;

	i = rand();					// hmmm ....
//	tw_random_seed(i);
	rng.seed(i);

	i = rand();
//	tw_random_seed_more(i);
	rng.seed_more(i);


	text_mode(-1);
}


void GameProject::quit()
{
	// result of an empty project: nothing to do, also on exiting it
}


void GameProject::destroy(GameBare *g)
{
	if (g->state)	// do not destroy it, if the game doesn't allow so
	{
		tw_error("Warning: the game hasn't authorized its own destruction");
		return;
	}

	if (first == g)
		first = g->next;

	if (last == g)
		last = g->prev;

	// remove the game from the list
	if (g->prev)
		g->prev->next = g->next;

	if (g->next)
		g->next->prev = g->prev;

	// destroy the game
	delete g;
}

// some stuff I copied from play_game

void GameProject::add(GameBare *g)
{
	//videosystem.window.lock();
	//clear_to_color(videosystem.window.surface, palette_color[8]);
	//videosystem.window.unlock();
	g->init();
	// this init should reference the already-initialized melee-data of the maingame

	if (last)
		last->next = g;

	g->prev = last;
	g->next = 0;
	last = g;

	if (!first)
		first = g;
}


void GameProject::play()
{

	int last_hit_time = 0;

	init();

	// criterium for quitting is, there are no subgames left.
	while ( first )
	{
		unsigned int time = get_time();
		poll_input();

		videosystem.poll_redraw();

		// delete games that want to be removed
		GameBare *g, *gdel;
		g = first;
		while (g)
		{
			gdel = g;		// save copy
			g = g->next;	// the iteration

			if (!gdel->state)	// if needed, destroy the deletion candidate
			{
				if (gdel->prev)
					gdel->prev->refocus();

				destroy(gdel);
			}

		}

		// if there are remaining games, handle these ... the physics (calculate) and
		// the rendering (animate)


		// check requests for games to add:

		for (g = first; g != 0; g = g->next)
		{
			if (g->gamerequest)
			{
				add(g->gamerequest);
				g->gamerequest = 0;
			}
		}


		// and now, game-specific stuff ??

		for (g = first; g != 0; g = g->next)
		{
			//g->prepare();	// set global pointers to the current game. (is done in play_iteration).
			g->play_iteration(time);
		}

		// A short pause between calculations - so that the cpu isn't over-used.
		// Value 5 is the same as the game::idle
		idle(5);


		// add game-types, if you want to "jump ahead" ...
		if (key[KEY_LCONTROL] && time - last_hit_time > 1000)
		{
		
			if (key[KEY_1])
			{
				last_hit_time = time;
				add( new GameStarmap() );
			}

			if (key[KEY_2])
			{
				last_hit_time = time;
				add( new GameDialogue() );	// the dialog editor
			}

			if (key[KEY_3])
			{
				add( new GameAliendialog() );	// the alien interface
			}

		}
		
	}
	
	quit();
}




void GameBare::calc_kinetic(SpaceLocation *s)
{
	s->pos += s->vel * frame_time;
}

// mapwrap=true/false are 2 distinct physical environments to choose from; one has no
// boundaries (left=right), the other does have (bouncy?) boundaries.

void GameBare::handle_edge(SpaceLocation *s)
{
	Vector2 &P = s->pos;
	Vector2 &vel = s->vel;

	Vector2 Dist;
	
	Dist = vel * frame_time;

	if (P.x < 0 || P.y < 0 || P.x > map_size.x || P.y > map_size.y)
	{
		// what should you do with an object that leaves the (current) physics ?
		// it could bounce off the map edge, or it could be removed...

		s->die();

		// possible do something else ?
	}

}


void GameBare::calculate()
{
	int i;
	for (i = 0; i < num_items; i += 1) {
		if (!item[i]->exists()) continue;
		
		calc_kinetic(item[i]);
		handle_edge(item[i]);
		}
	

	// override the linear physics first
	for (i = 0; i < num_items; i += 1)
		item[i]->pos -= item[i]->vel * frame_time;

	Physics::calculate();

	// check if the player is close to a star; if so, enter the associated solar system.
	if (escapetime > 0)
		escapetime -= frame_time * 1E-3;

	if (escapetime <= 0 && !gamerequest)
	{
		checknewgame();

		if (gamerequest)
			escapetime = 5.0;
	}

}


void GameBare::checknewgame()
{
	// do nothing
}



