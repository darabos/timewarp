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




void GamePlanetscan::init()
{

	GameBare::init();

	double H = 1000;
	size = Vector2(H, H);
	prepare();

//	mapwrap = false;
	wininfo.init( Vector2(500,500), 800.0, view->frame );
	wininfo.zoomlimit(size.x);
	wininfo.scaletowidth(size.x);	// zoom out to this width.


	// create star objects ?!
	int istar, iplanet;
	MapSpacebody *starmap, *solarmap, *planetmap;
	starmap = mapeverything.region[0];	// use the starmap of the 1st region

	istar = 0;
	solarmap = starmap->sub[istar];	// use the solarsystem belonging to that star

	iplanet = 0;
	planetmap = solarmap->sub[iplanet];	// use the planet (and moons) belonging to that planet orbit.


	SpaceObject *solarbody;
	//SpaceSprite *spr;

	// load the planet (or moon) sprite
	char *typestr;
	char txt[512];
	typestr = startypelist->type[planetmap->type].type_string;
	sprintf(txt, "gamex/planetscan/planet_%s_01.bmp", typestr);
	planetspr = create_sprite( txt, SpaceSprite::MASKED );
	solarbody = new SolarBody(0, 0.5*size, 0.0, planetspr, 0.5*size, iplanet,
						0,0,0,0);
	planetmap->o = solarbody;
	add(solarbody);
	solarbody->pos = Vector2(500, 400);

	// load the surface map of this planet (or moon)
	sprintf(txt, "gamex/planetscan/map_%s_01.bmp",typestr);
	map_bmp = load_bitmap(txt, 0);
	// (at the moment, this is resolution dependent...)
	// this should be scaled using a scaled blit...


	//solarbody = new SolarBody(0, planetmap->sub[i]->position, 0.0, spr);
	//planetmap->sub[i]->o = solarbody;
	//add(solarbody);

	// the player
	//SpaceSprite *playerspr;
	playerspr = create_sprite( "gamex/planetscan/player_01.bmp", SpaceSprite::MASKED, 64 );
	player = new ThePlaya(playerspr, &playerinfo);
	add(player);
	player->pos = Vector2(500,500);
}


void GamePlanetscan::quit()
{

	delete planetspr;
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

}

void GamePlanetscan::animate(Frame *frame)
{
	// draw the planet background.

	int ix, iy;
	ix = 100;
	iy = 300;
	blit(map_bmp, frame->surface, 0, 0, ix, iy, map_bmp->w, map_bmp->h);


	GameBare::animate(frame);
}



GamePlanetscan::ThePlaya::ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf)
:
LocalPlayerInfo(osprite, playinf)
{
	pos = 0;		// locate the player somewhere on the local map
}




