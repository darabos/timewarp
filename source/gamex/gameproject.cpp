#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE


#include "gameproject.h"

#include "gamestarmap.h"
#include "edit/edit_dialogue.h"
#include "gamedialogue.h"


BITMAP *game_screen, *game_screen1, *game_screen2;


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



void GameBare::init_menu()
{
	T = 0;
	maparea = 0;
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


	tempframe = new Frame2(1024);

	init_menu();

	if (!T)
	{
		tw_error("Game menu is not defined !!");
	}

	// use this for game-drawing to part of the menu
	if (maparea)
	{
		tempframe->setsurface(maparea->backgr);

		wininfo.init( Vector2(800,800), 800.0, tempframe );
	}

	// performance check
	tic_history = new Histograph(128);
	render_history = new Histograph(128);
	ti = true;

	// enable mouse drawing using the allegro pointer.
	hideallegromouse = false;
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

	// (and the game frame then draws to video memory?)
	if (ti)
		show_ticinfo(frame, tic_history, render_history, 4.0);

}


// copied (and simplified - no prediction) from Game class
void GameBare::animate()
{
	if (next)
		return;

	if (poll_scroll())		//in case you still have to wait for a page flip...
		return;

	bool usepageflip = false;

	// switch drawing screen
	if (usepageflip)
	{
		if (game_screen == game_screen1)
			game_screen = game_screen2;
		else
			game_screen = game_screen1;
	} else
		game_screen = game_screen1;
	//clear_to_color(game_screen, 0);



	double t = get_time2();

	tempframe->full_redraw = true;
	FULL_REDRAW = true;
	tempframe->erase();
	tempframe->prepare();

	acquire_bitmap(tempframe->surface);
	animate(tempframe);
	release_bitmap(tempframe->surface);

	acquire_bitmap(game_screen);
	acquire_bitmap(game_screen2);
	if (T && !next)
	{
		if (usepageflip)
			T->tree_setscreen(game_screen);		// game_screen can change value...
		else
			T->tree_setscreen(game_screen2);	// use a temp video area

		T->tree_animate();
	}

	// extra action: draw the temp bitmap into the screen area
	if (!usepageflip)
		blit(game_screen2, game_screen, 0, 0, 0, 0, game_screen->w, game_screen->h);

	release_bitmap(game_screen2);
	release_bitmap(game_screen);

	if (!hideallegromouse)
		show_mouse(game_screen);
	else
		show_mouse(0);

	// when drawing is finished, you could do a page flip to the new screen.
	// but this can take as much as 8 ms, which is a long time ... cause it has to
	// wait for a vertical retrace sync of the monitor. So ... since the picture varies only
	// slightly in-between updates, and updating the video memory goes very fast (<2 ms),
	// page-flip isn't really needed...
	if (usepageflip)
	{
		show_video_bitmap(game_screen);	// if you've to wait explicitly for a retrace ...
	}

	t = get_time2() - t;// - paused_time;


	render_history->add_element(pow(t, 4.0));
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
	::targets = &gametargets;
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

	// first, allocate the (old) screen from memory ... this should match
	// exactly the "global" screen ...
	game_screen1 = create_video_bitmap(screen->w, screen->h);
	game_screen2 = create_video_bitmap(screen->w, screen->h);
	if (!(game_screen1 && game_screen2))
	{
		tw_error("Failed to initialize game_screen!");
	}
	game_screen = game_screen1;
	show_video_bitmap(game_screen);
	clear_to_color(game_screen, 0);
	// ok ... so we've safely allocated the drawable area; this is needed, cause
	// any "new" video_bitmap is *first* allocated from the global screen area
	// cause the global screen area isn't allocated as a bitmap yet!! So you
	// can't use this global area normally while you're using other video-bitmaps.
}


void GameProject::quit()
{
	if (game_screen1)
		show_video_bitmap(game_screen1);

	if (game_screen1)
		destroy_bitmap(game_screen1);

	if (game_screen2)
		destroy_bitmap(game_screen2);

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
	::space_zoom = wininfo.zoomlevel;
	::space_center = wininfo.mapcenter;
	::space_center_nowrap += min_delta(wininfo.mapcenter, space_center_nowrap, map_size);
	::space_view_size = wininfo.framesize;

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

	// maintain/update target list (remove dead objects)
	gametargets.calculate();

	if (T && !next)
		T->tree_calculate();


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




void GameBare::show_ticinfo(Frame *f, Histograph *tic_history, Histograph *render_history, double hist_power)
{
	
	// show the render time ...
	int dur = 1;

	double tt = pow(tic_history->get_average(0, 1000/frame_time), 1/hist_power);
	double rt = pow(render_history->get_average(0, 1000/frame_time), 1/hist_power);
	char *tmp;
	
	if (tt*8 < frame_time)
		tmp = "good";
	else if (tt*2 < frame_time)
		tmp = "ok";
	else if (tt < frame_time)
		tmp = "bad";
	else
		tmp = "BAD!";
	message.print(dur, 12, "tic time: %.3fms (that's %s)", tt, tmp);
	
	if (rt < 2)
		tmp = "good";
	else if (rt < 20)
		tmp = "ok";
	else if (rt < 50)
		tmp = "bad";
	else
		tmp = "BAD!";
	message.print(dur, 12, "render time: %.3fms (that's %s)", rt, tmp);
	message.print(dur, 12, "debug: %d", debug_value);
	message.print(dur, 12, "shipdatas loaded: %d", shipdatas_loaded);

	message.animate(f);
	message.flush();
}










Frame2::Frame2(int max_items)
:
Frame(max_items)
{
}

Frame2::~Frame2()
{
}


void Frame2::setsurface(Surface *newsurface)
{
	surface = newsurface;

	// needed to fool the frame routine about the mother window (otherwise it'll create
	// new bitmaps with the mother window size??!!)
	window->w = surface->w;
	window->h = surface->h;
	window->x = 0;
	window->y = 0;

	ratio = double(surface->h) / double(surface->w);
}

void Frame2::erase()
{
	Frame::erase();
}

void Frame2::draw()
{
	Frame::draw();
}

void Frame2::prepare()
{
	return;	// do nothing, the real "prepare" comes from the setsurface.
}



