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
#include "../twgui/twpopup.h"




void GamePlanetview::init_menu()
{
	// place the menu into video-memory, cause we're using this as basis for
	// drawing; the game draws onto part of the menu.
	T = new TWindow("gamex/interface/planetview", 0, 0, game_screen, true);

	maparea = new Area(T, "map_");
}



void GamePlanetview::init()
{

	GameBare::init();




	double H = 2000;
	size = Vector2(H, H*tempframe->ratio);
	prepare();


	//wininfo.init( Vector2(200,200), 800.0, tempframe);//view->frame );
	wininfo.zoomlimit(size.x);
	wininfo.scaletowidth(size.x);	// zoom out to this width.


	FONT *usefont;
	usefont = videosystem.get_font(3);

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



	// position of the planet relative to the sun
	Vector2 relplanetpos;
	relplanetpos = planetmap->position;
	

	Vector2 centerpos;

	centerpos = Vector2(0.5, 0.25) * size;


	SpaceObject *solarbody;

	// load the planet data

	char txt[512];
	sprintf(txt, "gamex/planetview/planet_%s_01.bmp",
					planettypelist->type[planetmap->type].type_string);
	planetspr = create_sprite( txt, SpaceSprite::MASKED );

	Vector2 Poffs;
	int col;
	double R;
	
	double b = b_default;
	ellipsparams(relplanetpos, b, R, Poffs, col);

	Vector2 sunpos;
	sunpos = centerpos - relplanetpos;	// "offset center" of the ellips.
	solarbody = new SolarBody(0, centerpos, 0.0, planetspr, sunpos, iplanet,
								sunpos+Poffs, R, b, makecol(115,0,0));
	
	planetmap->o = solarbody;
	add(solarbody);
	solarbody->id = 0;	// so that it's not editable by the mapeditor

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
		Vector2 Poffs;
		int col;
		double R;
		
		double b = b_default;
		Vector2 P;
		P = planetmap->sub[i]->position;
		ellipsparams(P - centerpos, b,
			R, Poffs, col);

		int k;
		k = planetmap->sub[i]->type;
		solarbody = new SolarBody(0, P, 0.0, moonspr[k], centerpos+relplanetpos, i,
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


	// stuff for the map editor

	SpaceSprite *mousespr;
	mousespr = create_sprite( "gamex/mouse/pointer_starmap.bmp", SpaceSprite::MASKED );
	ptr = new MousePtr (mousespr);
	add(ptr);

	Tedit = new IconTV("gamex/interface/starmap/edit", 400, 200, game_screen);
	Tedit->exclusive = false;
	bnew = new Button(Tedit, "new_");
	breplace = new Button(Tedit, "replace_");
	Tedit->setsprites(moonspr, moontypelist->N);

	T->add(Tedit);
	T->tree_doneinit();

	Tedit->show();
	Tedit->focus();
	Tedit->layer = 1;	// shown first
	T->layer = 2;		// always shown later

	mapeditor = new MapEditor2();
	mapeditor->set_game(this, ptr);
	mapeditor->set_interface( Tedit, breplace, bnew );
	mapeditor->set_mapinfo( planetmap, 3, 1.0);

	mapeditor->mapcenter = centerpos;
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

	if (mapeditor->maphaschanged)
	{
		// write the map to disk
		mapeverything.save("gamex/mapinfo.txt");
	}

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


	// editor stuff

	ptr->newpos(mouse_x - maparea->pos.x, mouse_y - maparea->pos.y);
	mapeditor->calculate();

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
	GameBare::animate();
}


void GamePlanetview::animate(Frame *frame)
{
	if (next)
		return;

	// this draws to the menu (frame2)
	GameBare::animate(frame);
	
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

