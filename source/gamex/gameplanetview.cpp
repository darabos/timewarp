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


#include "gamesolarview.h"
#include "gameplanetview.h"

// the 2 possible subgames :
#include "gameplanetscan.h"
#include "gamemelee.h"

#include "stuff/space_body.h"
#include "stuff/backgr_stars.h"

#include "../twgui/twgui.h"



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







void GamePlanetview::init()
{

	GameBare::init();

	view->window->locate(
		0, 0,
		0, 0,
		0.0, 1.0,
		0.0, 1.0
		);

	// first, allocate the (old) screen from memory ... this should match
	// exactly the "global" screen ...
	newscreen = create_video_bitmap(screen->w, screen->h);
	if (!newscreen)
	{
		tw_error("Oh my !");
	}
	show_video_bitmap(newscreen);
	clear_to_color(newscreen, 0);

	// ok ... so we've safely allocated the drawable area I guess ...
	// this is needed, cause
	// this global one gets deleted for some reason, perhaps a view callback?
	// and any "new" video_bitmap is *first* allocated from this global area !! So you
	// can't use this global area normally while you're using other video-bitmaps.

	// place the menu into video-memory, cause we're using this as basis for
	// drawing; the game draws onto part of the menu.
	T = new TWindow("gamex/interface/planetview", 0, 0, newscreen, true);

	AreaTablet *maparea;
	maparea = new AreaTablet(T, "map_");

	FONT *usefont;
	usefont = videosystem.get_font(3);



	tmpbmp = maparea->backgr;//create_sub_bitmap(T->backgr, 30, 30, 700, 400);

	// use this for game-drawing to part of the menu
	tempframe = new Frame2(1024);
	tempframe->setsurface(tmpbmp);
	// needed to fool the frame routine about the mother window (otherwise it'll create
	// new bitmaps with the mother window size??!!)
	tempframe->window->w = tempframe->surface->w;
	tempframe->window->h = tempframe->surface->h;
	tempframe->window->x = 0;
	tempframe->window->y = 0;
//	tempframe->window->surface = tempframe->surface;


	double ratio = tmpbmp->h / double(tmpbmp->w);
	double H = 2000;
	size = Vector2(H, H*ratio);
	prepare();

//	mapwrap = false;
	wininfo.init( Vector2(200,200), 800.0, tempframe);//view->frame );
	wininfo.zoomlimit(size.x);
	wininfo.scaletowidth(size.x);	// zoom out to this width.


	// create star objects ?!
	int istar, iplanet;
	MapSpacebody *starmap, *solarmap, *planetmap;
	starmap = mapeverything.region[0];	// use the starmap of the 1st region


	istar = playerinfo.istar;
	iplanet = playerinfo.iplanet;

	if (istar == -1)
		istar = 0;
	if (iplanet == -1)
		iplanet = 0;

	solarmap = starmap->sub[istar];	// use the solarsystem belonging to that star
	planetmap = solarmap->sub[iplanet];	// use the planet (and moons) belonging to that planet orbit.


	// Button that displays the name of the planet.
	TextEditBox *starname;
	starname = new TextEditBox(T, "starname_", usefont, planetmap->name, 128);
	starname->set_textcolor(makecol(255,255,0));
	// (on exit, you should copy the (edited) name to the star/planet structure.


	// from this, you know the planet position, relative to the sun - we use it
	// the other way around this time: the sun position, relative to the planet !!
	Vector2 sunpos;
	double R, angle, b, b_sqrt;
	
	R = planetmap->position.y * 0.5*size.x;
	angle = (PI/180) * planetmap->position.x;
	
	b = 1.0;
	b_sqrt = sqrt(b);
	
	double c;
	c = 10;
	R *= c;

	// there is no extra offset; that's already in the planet pos ... you only need the
	// distance to the center, not the center+offset, of the ellips.

	Vector2 P;
	P = R*Vector2(cos(angle), (1/b_sqrt)*sin(angle));

	sunpos = -P;		// increase the distance .. as if you've zoomed in 10 times.
	// ok, this is the sun's position relative to the planet.
	
	

	Vector2 centerpos;

	centerpos = 0.5 * size;

	//Vector2 offs;
	//offs = Vector2(3E3, 2E3);
	//sunpos = centerpos + offs;	// temp value.

	SpaceObject *solarbody;

	// load the planet data

	char txt[512];
	sprintf(txt, "gamex/planetview/planet_%s_01.bmp",
					//startypelist->type[planetmap->type].type_string);
					planettypelist->type[planetmap->type].type_string);
	planetspr = create_sprite( txt, SpaceSprite::MASKED );
	//double b = 1.0;
	//double R = (sunpos - centerpos).length();
	solarbody = new SolarBody(0, centerpos, 0.0, planetspr, centerpos+sunpos, iplanet,
								centerpos+sunpos, R, b, makecol(115,0,0));
	planetmap->o = solarbody;
	add(solarbody);

	// load moon sprites
	int i;
	for ( i = 0; i < moontypelist->N; ++i )
	{
		char tmp[512];
		sprintf(tmp, "gamex/planetview/moon_%s_01.bmp", moontypelist->type[i].type_string);
		moonspr[i] = create_sprite( tmp, SpaceSprite::MASKED );
	}

	// load the star data
	for ( i = 0; i < planetmap->Nsub; ++i )
	{
//		char txt[512];
//		sprintf(txt, "gamex/planetview/moon_%s_01.bmp", planetmap->sub[i]->type);
//		spr = create_sprite( txt, SpaceSprite::MASKED );


		// ellips info ... similar to solar_view.cpp
		double R, angle, b, b_sqrt;

		R = planetmap->sub[i]->position.y * 0.5*size.x;
		angle = (PI/180) * planetmap->sub[i]->position.x;

		b = 2.0;
		b_sqrt = sqrt(b);

		Vector2 Poffs;
		Poffs = Vector2(0, 0.25 * R/b_sqrt);

		Vector2 P;
		P = centerpos + Poffs + R*Vector2(cos(angle), (1/b_sqrt)*sin(angle));


		int col;
		col = makecol(115,0,0);

		int k;
		k = planetmap->sub[i]->type;
		solarbody = new SolarBody(0, P, 0.0, moonspr[k], sunpos, i,
								centerpos+Poffs, R, b, col);

		planetmap->sub[i]->o = solarbody;
		add(solarbody);
	}


	team_player = new_team();
	team_aliens = new_team();

	// the player
	//SpaceSprite *playerspr;
	playerspr = create_sprite( "gamex/planetview/player_01.bmp", SpaceSprite::MASKED, 64 );
	player = new ThePlaya(playerspr, &playerinfo);
	add(player);

//	switch_team(player->ally_flag, team_player);

	locate_onedge_aligned_to_center(player, 0.5*size, 0.45*size);


	// test
	player->pos = Vector2(0,0);
	player->angle = 0.25 * PI;


	// an enemy fleet
	fleetspr = create_sprite( "gamex/planetview/fleet_01.bmp", SpaceSprite::MASKED, 64 );
	fleeticon = new FleetIcon(Vector2(400,300), fleetspr, "orzne");
	add(fleeticon);
//	switch_team(fleeticon->ally_flag, team_aliens);


	StarBackgr *sb;
	sb = new StarBackgr();
	sb->init(100, tempframe);
	add(sb);

	T->tree_doneinit();


	// performance check
	tic_history = new Histograph(128);
	render_history = new Histograph(128);
}


void GamePlanetview::quit()
{
	delete planetspr;
	delete playerspr;
	delete fleetspr;

	int i;
	for ( i = 0; i < moontypelist->N; ++i )
		delete moonspr[i];

	if (!hardexit)	// game is not quitted completely
	{
		playerinfo.iplanet = -1;	// cause you've left the planet orbit
		playerinfo.angle = player->angle;
	}
	else
		playerinfo.sync(player);

	if (tmpbmp)
		destroy_bitmap(tmpbmp);

	if (newscreen)
		delete (newscreen);

	GameBare::quit();
}



void GamePlanetview::refocus()
{
	playerinfo.sync(player);
}




void GamePlanetview::calculate()
{
	if (next)
		return;

	double t = get_time2();

	T->tree_calculate();

	GameBare::calculate();

	if (!(player && player->exists()))
	{
		quit();
		return;
	}


	double dt = frame_time * 1E-3;

	if (key[KEY_UP])
		player->accelerate(0, player->angle, 0.1*dt, 1.0);

	if (key[KEY_LEFT])
		player->angle -= 0.5*PI2 * dt;

	if (key[KEY_RIGHT])
		player->angle += 0.5*PI2 * dt;

	// this viewtype needs no zoom, and always centers on the center of the planet system map.
	wininfo.center(0.5*size);


	/*
	// check if the playa hits something or someone
	if (player->collisionwith)
	{
		// spawn a melee game ?!
		if (player->collisionwith->id != ID_FLEETICON)
			gamerequest = 0;//new GamePlanetscan();
//		else
//			gamerequest = new GameMelee();
	}
	*/

	t = get_time2() - t;// - paused_time;
	tic_history->add_element(pow(t, 4.0));
}





void GamePlanetview::checknewgame()
{
	//return;

	if (player->collisionwith)
	{

		// only activate planet view mode, if you've hit a planet.
		if (player->collisionwith->id == ID_FLEETICON)
		{

			// which planet did you collide with ?!
//			playerinfo.iplanet = ((SolarBody*) player->collisionwith)->solar_body_num;
	
			player->vel = 0;

			// what's the race ?
			XFleet *f;
			f = &((FleetIcon*) player->collisionwith)->fleet;

			// spawn a subgame
			if (!gamerequest && !next)
			{
				GameMelee *g;
				g = new GameMelee();
				g->set_xfleet(f);

				gamerequest = g;
			}

		}
	}

}


void GamePlanetview::animate()
{
	double t = get_time2();

	//idle(20);
	
	::space_zoom = wininfo.zoomlevel;
	::space_center = wininfo.mapcenter;
	::space_view_size = wininfo.framesize;

	// override the use of the "default" screen, instead re-route everything to the
	// menu bitmaps.

	tempframe->full_redraw = true;
	FULL_REDRAW = true;
	tempframe->erase();
	tempframe->prepare();


//	message.print(1500, 14, "%p %p %p", T->drawarea, T->backgr, ::screen);

	animate(tempframe);
	


	// the menu draws to the game frame
	//T->tree_setscreen(frame->surface);

	T->tree_setscreen(newscreen);
	T->tree_animate();


	t = get_time2() - t;// - paused_time;
	render_history->add_element(pow(t, 4.0));
}


void GamePlanetview::animate(Frame *frame)
{
	if (next)
		return;

	// this draws to the menu (frame2)
	GameBare::animate(frame);
	
	// (and the game frame then draws to video memory?)
	show_ticinfo(frame, tic_history, render_history, 4.0);
}



GamePlanetview::ThePlaya::ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf)
:
LocalPlayerInfo(osprite, playinf)
{
	pos = 0;		// locate the player somewhere on the local map
	collide_flag_sameship = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_anyone = ALL_LAYERS;
	mass = 1;
}




FleetIcon::FleetIcon(Vector2 opos, SpaceSprite *osprite, char *oidname)
:
SpaceObject(0, opos, 0.0, osprite)
{
	id = ID_FLEETICON;
	collide_flag_sameship = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_anyone = ALL_LAYERS;
	layer = LAYER_SHIPS;
	mass = 1;

	// construct a small fleet of 2 Orz ...
	fleet.add("orzne", 2);
}






GamePlanetview::~GamePlanetview()
{
}

