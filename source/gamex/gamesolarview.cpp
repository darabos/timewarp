
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






void ellipsparams(Vector2 relpos, double ellb,
				  double &R, Vector2 &Poffs, int &col)
{
	//double R;
	double b_sqrt;
	
	
	b_sqrt = sqrt(ellb);
	
	// pure ellips that hits the point has radius R
//	Vector2 relpos;
//	relpos = pos - 0.5*map_size;
	relpos.y *= b_sqrt;

	double offs = 0.5;	// offset of the ellips
	double a;
	a = offs * offs - 1;

	double b;
	b = -2 * offs * relpos.y;

	double c;
	c = magnitude_sqr(relpos);

	// solve quadr. eq.
	double det;
	det = b*b - 4*a*c;
	if (det < 0)
	{
		tw_error("negative value for ellips root");
	}
	det = sqrt(det);

	// two solutions:
	double r1, r2;
	r1 = (-b + det) / (2*a);
	r2 = (-b - det) / (2*a);

	if (r1 > 0)
		R = r1;
	else
		R = r2;

	// shift the ellips down (so, scale y-size down, hence R is scaled linearly cause y~Rsin(a))
	Poffs = Vector2(0, offs * R / b_sqrt);
	//R *= 0.75;
	
	
	//int col;
	if (R < 500.0)
		col = makecol(115,0,0);
	else if (R < 1000.0)
		col = makecol(0,85,0);
	else if (R < 1500.0)
		col = makecol(0,44,66);
	else
		col = makecol(0,0,115);
}




MapObj *MapEditor2::create_mapobj(Vector2 pos)
{
	

	Vector2 Poffs;
	int col;
	double R;
	
	double b = b_default;
	ellipsparams(pos - mapcenter, b,
				  R, Poffs, col);

	SolarBody	*s;

	s = new SolarBody(0, pos, 0.0, Tedit->showspr(), mapcenter, Tedit->isel,
		mapcenter+Poffs, R, b, col	// ellips information
		);

	return s;
}


void MapEditor2::calculate()
{
	MapEditor::calculate();

	if (selection && moveselection)
	{


		SolarBody	*s;
		s = (SolarBody*) selection;

		s->stayhere = s->pos;	// in this case, movement is allowed ...

		Vector2 Poffs;
		int col;
		double R;

		ellipsparams(s->pos - mapcenter, s->ellipsb,
				  R, Poffs, col);

		s->ellipsR = R;
		s->ellipscol = col;
		s->ellipscenter = mapcenter + Poffs;

		s->drawshadow();
	}
}





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
	sunpos = Vector2(0.5, 0.25) * size;

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
	solarbody->id = 0;	// cause it's not a editable map object...

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
		Vector2 Poffs;
		int col;
		double R;
		
		double b = b_default;
		ellipsparams(solarmap->sub[i]->position - sunpos, b,
			R, Poffs, col);

		int k;
		k = solarmap->sub[i]->type;

		solarbody = new SolarBody(0, solarmap->sub[i]->position, 0.0, planetspr[k], sunpos, k,
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


	// stuff for the map editor

	SpaceSprite *mousespr;
	mousespr = create_sprite( "gamex/mouse/pointer_starmap.bmp", SpaceSprite::MASKED );
	ptr = new MousePtr (mousespr);
	add(ptr);

	Tedit = new IconTV("gamex/interface/starmap/edit", 400, 200, game_screen);
	Tedit->exclusive = false;
	bnew = new Button(Tedit, "new_");
	breplace = new Button(Tedit, "replace_");
	Tedit->setsprites(planetspr, planettypelist->N);

	T->add(Tedit);
	T->tree_doneinit();

	Tedit->show();
	Tedit->focus();
	Tedit->layer = 1;	// shown first
	T->layer = 2;		// always shown later

	mapeditor = new MapEditor2();
	mapeditor->set_game(this, ptr);
	mapeditor->set_interface( Tedit, breplace, bnew );
	mapeditor->set_mapinfo( solarmap, 2, 1.0);

	mapeditor->mapcenter = sunpos;
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


	if (mapeditor->maphaschanged)
	{
		// write the map to disk
		mapeverything.save("gamex/mapinfo.txt");
	}

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

	double q;
	wininfo.center(player->pos);
	wininfo.edgecorrect();

	GameBare::calculate();

	q = space_zoom;

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


	// editor stuff

	ptr->newpos(mouse_x - maparea->pos.x, mouse_y - maparea->pos.y);
	mapeditor->calculate();

}



void GameSolarview::checknewgame()
{
	//return;

	if (player->collisionwith)
	{

		// only activate planet view mode, if you've hit a planet.
		if (player->collisionwith->id == MAPOBJ_ID)
		{

			// playerinfo:
			// istar and iplanet should be known for this subgame.

			// which planet did you collide with ?!
			playerinfo.iplanet = ((SolarBody*) player->collisionwith)->starnum;
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












