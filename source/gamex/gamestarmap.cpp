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

#include "gamestarmap.h"

#include "general/sprites.h"

//#include "gamedata.h"

/*

  colors (in 10E12 Hz)
  red :		622 - 780
  green:	492 - 577
  blue:		455 - 492

  T: in Celcius
  f: in 10E12 Hz
*/

double blackbodyspectrum(double L, double T)
{
	T += 273.15;	// now in Kelvin
	L *= 1.0E-9 * 1E+2;	// cause it's in cm

	double h = 6.62606876E-27;		// erg s
	double c = 2.9979245800E+10;	// cm/s
	// wave number
	double k = 1.3806503E-16;		// erg/K

	double a = (2 * h * c*c) / (L*L*L*L*L);
	double b = exp((h * c) / (L * k * T));

	return a / (b - 1);
}

static double sqr(double x)
{
	return x*x;
}

double spec_int(double Lmin, double Lmax, double T)
{
	int N = 50;
	int i;

	double I;
	I = 0;

	for ( i = 0; i < N; ++i )
		I += blackbodyspectrum(Lmin + i*(Lmax-Lmin)/(N-1), T) *
				exp( -1 * sqr((i-0.5*N) / (0.5*N)) );

	return I / N;
}


double spec_r(double T)
{
	return spec_int(600, 700, T);
}

double spec_g(double T)
{
	return spec_int(520, 580, T);
}

double spec_b(double T)
{
	return spec_int(420, 470, T);
}




void GameStarmap::init_menu()
{
	// place the menu into video-memory, cause we're using this as basis for
	// drawing; the game draws onto part of the menu.
	T = new TWindow("gamex/interface/starmap", 0, 0, game_screen, true);

	maparea = new Area(T, "map_");
}


void GameStarmap::init()
{

	GameBare::init();
	//::physics = this;	// is already in prepare

	double H = 1000;
	scalepos = 100;
	H *= scalepos;

	size = Vector2(H, H);

	prepare();

//	mapwrap = false;
	//wininfo.init( Vector2(400,400), 800.0, tempframe );
	wininfo.zoomlimit(size.x);
	wininfo.center(Vector2(0,0));
	wininfo.scaletowidth(0.5 * size.x);	// zoom out to this width.


	// load star sprites
	startypespr = new SpaceSprite* [startypelist->N];
	int i;
	for ( i = 0; i < startypelist->N; ++i )
	{
		char tmp[512];
		sprintf(tmp, "gamex/stars/star_%s_01.bmp", startypelist->type[i].type_string);
		startypespr[i] = create_sprite( tmp, SpaceSprite::MASKED );
	}

	// create star objects ?!
	starmap = mapeverything.region[0];	// use the starmap of the 1st region
	starmap->scalepos = scalepos;

	starspr = new SpaceSprite* [starmap->Nsub];

	for ( i = 0; i < starmap->Nsub; ++i )
	{
		MapObj *star;
		int k;

		k = starmap->sub[i]->type;
		starspr[i] = new SpaceSprite(startypespr[k]->get_bitmap(0));
		double T = 6000.0;
		colorize(starspr[i], spec_r(T), spec_g(T), spec_b(T));
		brighten(starspr[i]);

		star = new MapObj(0, starmap->sub[i]->position * scalepos, 0.0, starspr[i]);
		star->starnum = i;

		add(star);
	}

	SpaceSprite *mousespr;
	mousespr = create_sprite( "gamex/mouse/pointer_starmap.bmp", SpaceSprite::MASKED );
	ptr = new MousePtr (mousespr);
	add(ptr);


	// the player
	playerspr = create_sprite( "gamex/stars/player_01.bmp", SpaceSprite::MASKED );
	player = new ThePlaya(playerspr, &playerinfo);
	add(player);


	mouseper = new Periodics(0.1);
	keyper = new Periodics(0.1);

//	selectionstar = 0;
//	lastselectionstar = 0;
//	maphaschanged = false;	// check if the map changed; if so, it's to be written back to disk


	// define another (sub)menu

	Tedit = new IconTV("gamex/interface/starmap/edit", 400, 200, game_screen);
	Tedit->exclusive = false;
	bnew = new Button(Tedit, "new_");
	breplace = new Button(Tedit, "replace_");
	Tedit->tv->set(starspr, startypelist->N);



	T->add(Tedit);
	T->tree_doneinit();

	Tedit->show();
	Tedit->focus();
	Tedit->layer = 1;	// shown first
	T->layer = 2;		// always shown later

	mapeditor = new MapEditor();
	mapeditor->set_game(this, ptr);
	mapeditor->set_interface( Tedit, breplace, bnew );
	mapeditor->set_mapinfo( starmap, 1, scalepos);




	/*
	// create a colony somewhere ?? For testing purpose only ...
	RaceInfo *race;
	RaceColony *colony;

	race = new RaceInfo("testrace", makecol(100,20,20));
	add(race);

	colony = new RaceColony(race);
	colony->locate(0, 0, -1);
	colony->patrol.range = 1000;
	add(colony);
	*/
}



void GameStarmap::calculate()
{
	if (next)
		return;

	//::space_view_size = wininfo.framesize;
	//::space_zoom = wininfo.zoomlevel;
	//::space_center = wininfo.mapcenter;


	GameBare::calculate();

	ptr->newpos(mouse_x - maparea->pos.x, mouse_y - maparea->pos.y);

	if ( mouseper->update() && (mouse_b & 2) )
	{
		if (view && view->frame && view->frame->window && view->frame->window->surface)
			wininfo.centermove((mouse_x - maparea->pos.x - 0.5*view->frame->window->surface->w) / space_zoom,
								(mouse_y - maparea->pos.y - 0.5*view->frame->window->surface->h) / space_zoom);
		//wininfo.center(Vector2(0,0));
	}


	double dt = frame_time * 1E-3;

	if (key[KEY_EQUALS])
		wininfo.zoom(1 + 1*dt);

	if (key[KEY_MINUS])
		wininfo.zoom(1 / (1 + 1*dt));


	mapeditor->calculate();

}

Vector2 corner2 ( Vector2 pos, Vector2 size ) {STACKTRACE
	pos -= space_center;
	pos -= size / 2;
	return pos * space_zoom + space_view_size / 2;
}


void GameStarmap::animate(Frame *frame)
{
	if (next)	// shouldn't happen for this game type ?!
		return;


	if (ptr->pos.x == 0 || ptr->pos.y == 0 || !maparea->hasmouse())
		hideallegromouse = false;
	else if (Tedit->grabbedmouse)
		hideallegromouse = false;
	else
		hideallegromouse = true;


	// draw race territories
	racelist.animate_map(frame, 1);

	// draw a grid ...
	int ix, iy;
	Vector2 P1, P2;

	int c;
	c = makecol(128, 64, 0);

	for ( iy = 0; iy < 11; ++iy )
	{
		P1 = corner2( Vector2(     0, iy*10000), 0);
		P2 = corner2( Vector2(100000, iy*10000), 0);

		hline(frame->surface, P1.x, P1.y, P2.x, c);
	}

	for ( ix = 0; ix < 11; ++ix )
	{
		P1 = corner2( Vector2(ix*10000,      0), 0);
		P2 = corner2( Vector2(ix*10000, 100000), 0);

		vline(frame->surface, P1.x, P1.y, P2.y, c);
	}


	// draw the stars

	GameBare::animate(frame);


}



GameStarmap::ThePlaya::ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf)
:
LocalPlayerInfo(osprite, playinf)
{
	pos = 0;		// locate the player somewhere on the local map
}




void GameStarmap::quit()
{
	delete playerspr;

	int i;
	for ( i = 0; i < startypelist->N; ++i )
		delete startypespr[i];
	delete startypespr;

	for ( i = 0; i < starmap->Nsub; ++i )
		delete starspr[i];
	delete starspr;

	if (mapeditor->maphaschanged)
	{
		// write the map to disk
		mapeverything.save("gamex/mapinfo.txt");
	}

	GameBare::quit();
}









