
#include <allegro.h>
#include <stdio.h>
//#include <string.h>

#include "../melee.h"
REGISTER_FILE


//#include "../melee/mframe.h"
//#include "../melee/mview.h"
//#include "../melee/mitems.h"

#include "gamesolarview.h"
#include "gameplanetview.h"

#include "stuff/space_body.h"
#include "stuff/backgr_stars.h"




void GameSolarview::init_menu()
{
	// place the menu into video-memory, cause we're using this as basis for
	// drawing; the game draws onto part of the menu.
	T = new TWindow("gamex/interface/planetview", 0, 0, game_screen, true);

	maparea = new Area(T, "map_");
	makevideobmp(maparea->backgr);	// for faster drawing (I think ...)
}




void GameSolarview::init()
{

	GameBare::init();

	double H = 4000;
	size = Vector2(H, H*tempframe->ratio);

	prepare();

//	mapwrap = false;
	//wininfo.init( Vector2(200,200), 1024.0, tempframe );
	wininfo.zoomlimit(size.x);
	wininfo.scaletowidth(size.x);	// zoom out to this width.


	// create star objects ?!
	int istar;
	MapSpacebody *starmap, *solarmap;
	starmap = mapeverything.region[0];	// use the starmap of the 1st region

//	playerinfo.istar = 0;
	istar = playerinfo.istar;
	if (istar < 0)
		istar = 0;

	solarmap = starmap->sub[istar];	// use the solarsystem belonging to that star


	SpaceObject *solarbody;
	//SpaceSprite *spr;

	Vector2 sunpos;
	sunpos = 0.5 * size;

	// load the star data
	char txt[512];
	sprintf(txt, "gamex/solarview/star_%s_01.bmp",
						startypelist->type[solarmap->type].type_string);
	starspr = create_sprite( txt, SpaceSprite::MASKED );
	solarbody = new SolarBody(0, sunpos, 0.0, starspr, sunpos, istar,
									0, 0, 0, 0);
	// solarmap->o = solarbody;	don't do this, or the hyperspace map may go crazy ...
	// since the stars are already referenced (and edited?) therein...
	add(solarbody);

	// the star is untouchable.
	solarbody->collide_flag_anyone = 0;
	solarbody->collide_flag_sameteam = 0;
	solarbody->collide_flag_sameship = 0;

	// load planet sprites
	int i;
	for ( i = 0; i < planettypelist->N; ++i )
	{
		char tmp[512];
		sprintf(tmp, "gamex/solarview/planet_%s_01.bmp", planettypelist->type[i].type_string);
		planetspr[i] = create_sprite( tmp, SpaceSprite::MASKED );
	}

	// load the planet data
	for ( i = 0; i < solarmap->Nsub; ++i )
	{
		// "position" is in this case, the angular position, x=angle, y=r (in percentage of map size).

		double R, angle, b, b_sqrt;

		R = solarmap->sub[i]->position.y * 0.5*size.x;
		angle = (PI/180) * solarmap->sub[i]->position.x;

		b = 2.0;
		b_sqrt = sqrt(b);

		Vector2 Poffs;
		Poffs = Vector2(0, 0.25 * R/b_sqrt);

		Vector2 P;
		P = sunpos + Poffs + R*Vector2(cos(angle), (1/b_sqrt)*sin(angle));

		int col;
		if (R < 500.0)
			col = makecol(115,0,0);
		else if (R < 1000.0)
			col = makecol(0,85,0);
		else if (R < 1500.0)
			col = makecol(0,44,66);
		else
			col = makecol(0,0,115);

		int k;
		k = solarmap->sub[i]->type;
		solarbody = new SolarBody(0, P, 0.0, planetspr[k], sunpos, i,
									sunpos+Poffs, R, b, col	// ellips information
									);
		solarmap->sub[i]->o = solarbody;
		add(solarbody);

	}

	// the player
	//SpaceSprite *playerspr;
	playerspr = create_sprite( "gamex/solarview/player_01.bmp", SpaceSprite::MASKED, 64 );
	player = new ThePlaya(playerspr, &playerinfo);
	add(player);

	locate_onedge_aligned_to_center(player, 0.5*size, 0.45*size);


	StarBackgr *b;
	b = new StarBackgr();
	b->init(100, view->frame);
	add(b);
}


void GameSolarview::quit()
{
	delete starspr;
	delete playerspr;

	int i;
	for ( i = 0; i < planettypelist->N; ++i )
		delete planetspr[i];

	if (!hardexit)	// game is not quitted completely
	{
		playerinfo.istar = -1;	// cause you've left the solar system
		playerinfo.angle = player->angle;
	}
	else
		playerinfo.sync(player);

	GameBare::quit();
}


void GameSolarview::refocus()
{
	if (!hardexit)
	{
		player->angle = playerinfo.angle;	// first, update the angle ...
		playerinfo.sync(player);
	}
}


void GameSolarview::calculate()
{
	if (next)
		return;

	GameBare::calculate();

	if (!(player && player->exists()))
	{
		state = 0;
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

	if (key[KEY_EQUALS])
		wininfo.zoom(1 + 1*dt);

	if (key[KEY_MINUS])
		wininfo.zoom(1 / (1 + 1*dt));

	wininfo.center(player->pos);
	wininfo.edgecorrect();

}



void GameSolarview::checknewgame()
{
	//return;

	if (player->collisionwith)
	{

		// only activate planet view mode, if you've hit a planet.
		if (player->collisionwith->id == ID_SOLAR_BODY)
		{

			// playerinfo:
			// istar and iplanet should be known for this subgame.

			// which planet did you collide with ?!
			playerinfo.iplanet = ((SolarBody*) player->collisionwith)->solar_body_num;
			playerinfo.angle = player->angle;
	
			player->vel = 0;

			// spawn a subgame
			if (!gamerequest && !next)
				gamerequest = new GamePlanetview();
		}
	}

}




void GameSolarview::animate(Frame *frame)
{
	if (next)
		return;

//	FULL_REDRAW = 1;

//	::space_zoom = wininfo.zoomlevel;
//	::space_center = wininfo.mapcenter;


	GameBare::animate(frame);

}



GameSolarview::ThePlaya::ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf)
:
LocalPlayerInfo(osprite, playinf)
{
	pos = 0;		// locate the player somewhere on the local map
	collide_flag_sameship = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_anyone = ALL_LAYERS;
	mass = 1;
}












