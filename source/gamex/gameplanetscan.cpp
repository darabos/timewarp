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
//#include "gameplanetview.h"
#include "gameplanetscan.h"

#include "stuff/space_body.h"




void GamePlanetscan::init_menu()
{
	// place the menu into video-memory, cause we're using this as basis for
	// drawing; the game draws onto part of the menu.
	T = new TWindow("gamex/interface/planetscan", 0, 0, game_screen, true);

	maparea = new Area(T, "map_");

	surf_area = new Area(T, "surface_");
}



void GamePlanetscan::init()
{

	GameBare::init();

	double H = 1000;
	size = Vector2(H, H*tempframe->ratio);
	prepare();

	wininfo.zoomlimit(size.x);
	wininfo.scaletowidth(size.x);	// zoom out to this width.


	// create star objects ?!
	int istar, iplanet, imoon;
	MapSpacebody *starmap, *solarmap, *planetmap, *moonmap;
	starmap = mapeverything.region[0];	// use the starmap of the 1st region

	istar = playerinfo.istar;
	iplanet = playerinfo.iplanet;
	imoon = playerinfo.imoon;

	if (istar == -1)
		istar = 0;
	if (iplanet == -1)
		iplanet = 0;

	solarmap = starmap->sub[istar];	// use the solarsystem belonging to that star
	planetmap = solarmap->sub[iplanet];	// use the planet (and moons) belonging to that planet orbit.

	if (imoon >= 0)
		moonmap = planetmap->sub[imoon];	// use the planet (and moons) belonging to that planet orbit.
	else
		moonmap = 0;
	

	//SpaceSprite *spr;

	char *typestr;
	char txt[512];

	if ( imoon == -1 )
	{
		// exploring a PLANET
		// load the planet sprite

		typestr = planettypelist->type[planetmap->type].type_string;

		// load the surface map of this planet (or moon)
		sprintf(txt, "gamex/planetscan/planet_%s_01.bmp",typestr);
		map_bmp = load_bitmap(txt, 0);
		// (at the moment, this is resolution dependent...)
		// this should be scaled using a scaled blit...

	} else {

		// exploring a MOON
		typestr = moontypelist->type[moonmap->type].type_string;
		
		// load the surface map of this planet (or moon)
		sprintf(txt, "gamex/planetscan/moon_%s_01.bmp",typestr);
		map_bmp = load_bitmap(txt, 0);
		// (at the moment, this is resolution dependent...)
		// this should be scaled using a scaled blit...

	}

	// map_bmp should have 32 bit depth, because colors are assumed to come from
	// a 32 bpp depth image. Loading from disk is sometimes (or often) 24 bit.
	BITMAP *tmp;
	tmp = create_bitmap_ex(32, map_bmp->w, map_bmp->h);
	clear_to_color(tmp, makecol32(255,0,255));
	blit(map_bmp, tmp, 0, 0, 0, 0, tmp->w, tmp->h);
	destroy_bitmap(map_bmp);
	map_bmp = tmp;


	// the player
	//SpaceSprite *playerspr;
	playerspr = create_sprite( "gamex/planetscan/player_01.bmp", SpaceSprite::MASKED, 64 );
	player = new ThePlaya(playerspr, &playerinfo);
	add(player);
	player->pos = Vector2(500,500);



	
	// add a 3d rotating planet ?!
	SpaceSprite *dummy;

	int planet_radius = 100;
	int image_size = 2 * planet_radius;

	BITMAP *image32bit = create_bitmap_ex(32, image_size, image_size);
	clear_to_color(image32bit, makecol32(255,0,255));
	circlefill(	image32bit,
		image32bit->w/2, image32bit->h/2,
		image32bit->w/2 - 2, makecol(255,255,255));
	
	dummy  = new SpaceSprite(image32bit, SpaceSprite::IRREGULAR | SpaceSprite::MASKED | SpaceSprite::NO_AA);
		
	Vector2 opos = 0.5 * map_size;
	SpaceSprite *color_map, *spec_map;

	color_map  = new SpaceSprite(map_bmp, SpaceSprite::IRREGULAR | SpaceSprite::MASKED );

	spec_map = 0;
	int aPlanetUsespec = 0;

	double tilt, spin, sun_hangle, sun_vangle;
	tilt = tw_random(-40.0, 40.0);
	spin = tw_random(-30.0, 30.0);
	sun_vangle = tw_random(-20.0, 60.0);
	sun_hangle = tw_random(-45.0, 45.0);
	double sunr = 1.0;
	double sung = 1.0;
	double sunb = 1.0;

	rotatingplanet = new Planet3D(opos, color_map, spec_map, dummy,
		planet_radius, aPlanetUsespec,
		spin, tilt, 
		sun_vangle, sun_hangle,
		sunr, sung, sunb,
		true);

	// you should delete dummy yourself ?
		

}


void GamePlanetscan::quit()
{

	delete playerspr;
	destroy_bitmap(map_bmp);

	GameBare::quit();
}




void GamePlanetscan::calculate()
{
	GameBare::calculate();


	double dt = frame_time * 1E-3;

	if (key[KEY_UP])
		player->accelerate(0, player->angle, 0.1*dt, 1.0);

	if (key[KEY_LEFT])
		player->angle -= 0.5*PI2 * dt;

	if (key[KEY_RIGHT])
		player->angle += 0.5*PI2 * dt;

	// this viewtype needs no zoom, and always centers on the center of the planet system map.
	wininfo.center(0.5*size);


	rotatingplanet->calculate();

}

void GamePlanetscan::animate(Frame *frame)
{
	// draw the planet background.

	blit(map_bmp, surf_area->backgr, 0, 0, 0, 0, map_bmp->w, map_bmp->h);

	int x, y;

	x = (maparea->backgr->w - rotatingplanet->size.x) / 2;
	y = (maparea->backgr->h - rotatingplanet->size.y) / 2;
	rotatingplanet->animate_pre();
	rotatingplanet->get_sprite()->draw(x, y, 0, maparea->backgr);

	GameBare::animate(frame);

}



GamePlanetscan::ThePlaya::ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf)
:
LocalPlayerInfo(osprite, playinf)
{
	pos = 0;		// locate the player somewhere on the local map
}




