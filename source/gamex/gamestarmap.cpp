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

#include "gamedata.h"
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



void GameStarmap::load_startypes(SpaceSprite ***planettypespr)
{
	// load star sprites
	startypespr = new SpaceSprite* [startypelist->N];
	int i;
	for ( i = 0; i < startypelist->N; ++i )
	{
		char tmp[512];
		sprintf(tmp, "%s%s_01.bmp",
			startypelist->basename,
			startypelist->type[i].type_string);
		startypespr[i] = create_sprite( tmp, SpaceSprite::MASKED );
	}
}


void GameStarmap::load_surfacetypes(BITMAP ***surfacebmp)
{
	// load surface bitmaps
	(*surfacebmp) = new BITMAP* [starsurfacetypelist->N];
	int i;
	for ( i = 0; i < starsurfacetypelist->N; ++i )
	{
		char tmp[512];
		sprintf(tmp, "%s%s_01.bmp",
			starsurfacetypelist->basename,
			starsurfacetypelist->type[i].type_string);
		load_bitmap32(&(*surfacebmp)[i], tmp);
		scale_bitmap32(&(*surfacebmp)[i], 0.2);
	}
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

	wininfo.zoomlimit(size.x);
	wininfo.center(Vector2(0,0));
	wininfo.scaletowidth(0.5 * size.x);	// zoom out to this width.


	/*
	// load star sprites
	startypespr = new SpaceSprite* [startypelist->N];
	int i;
	for ( i = 0; i < startypelist->N; ++i )
	{
		char tmp[512];
		sprintf(tmp, "gamex/stars/star_%s_01.bmp", startypelist->type[i].type_string);
		startypespr[i] = create_sprite( tmp, SpaceSprite::MASKED );
	}
	*/

	// load planet sprites
	load_startypes(&startypespr);
	load_surfacetypes(&surfacebmp);

	// create star objects
	starmap = mapeverything.sub[0];	// use the starmap of the 1st region
	starmap->scalepos = scalepos;

	int i;
	char tmp[512];

	for ( i = 0; i < starmap->Nsub; ++i )
	{
		MapObj *star;
		int k;

		k = starmap->sub[i]->type;
		starspr = new SpaceSprite(startypespr[k]->get_bitmap(0));

		double T;

		sprintf(tmp, "gamex/gamedata/surface/%08X.ini", starmap->sub[i]->id);
		set_config_file(tmp);
		T = get_config_float(0, "temperature", 5000.0);

		colorize(starspr, spec_r(T), spec_g(T), spec_b(T));
		brighten(starspr);

		star = new MapObj(0, starmap->sub[i]->position * scalepos, 0.0, starspr);
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


	// define another (sub)menu

//	Tedit = new IconTV("gamex/interface/starmap/edit", 400, 200, game_screen);
//	Tedit->exclusive = false;
//	bnew = new Button(Tedit, "new_");
//	breplace = new Button(Tedit, "replace_");
//	Tedit->tv->set(startypespr, startypelist->N);



	FONT *usefont = videosystem.get_font(3);

	mapeditor = new MapEditor1();
	mapeditor->set_game(this, ptr);
//	mapeditor->init_interface(T, usefont, startypespr, surfacebmp);
	mapeditor->init_interface(T, usefont, startypespr, startypelist->N,
		surfacebmp, starsurfacetypelist->N);
	mapeditor->set_mapinfo( starmap, scalepos);


	T->tree_doneinit();

	mapeditor->Tedit->show();
	mapeditor->Tedit->focus();
	T->layer = 2;		// always shown later

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
	else if (mapeditor->Tedit->grabbedmouse)
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

	for ( i = 0; i < starsurfacetypelist->N; ++i )
		del_bitmap(&surfacebmp[i]);
	delete surfacebmp;

	if (mapeditor->maphaschanged)
	{
		// write the map to disk
		mapeverything.save("gamex/mapinfo.txt");
	}

	GameBare::quit();
}









void MapEditor1::define_stats()
{
	ved->values[0]->set(vtype_float, "temperature", 2000.0, 100000.0);
	ved->values[1]->set(vtype_float, "radius (s)", 0.01, 1000.0);
}



void MapEditor1::set_config()
{
	set_config_float(0, "temperature",    ved->values[0]->value);
	set_config_float(0, "radius",  ved->values[1]->value);

	// and the surface type string ?
	char *t;
	t = starsurfacetypelist->type[tvsurf->isel].type_string;
	set_config_string(0, "surface", t);
}


void MapEditor1::get_config()
{

	ved->values[0]->value = get_config_float(0, "temperature", 0);
	ved->values[1]->value = get_config_float(0, "radius", 0);

	ved->edit_update();

	// and the surface type ?
	char tmp[512];
	strcpy(tmp, get_config_string(0, "surface", "default"));
	tvsurf->set_sel ( surfacetypelist->get_index(tmp, 0) );

}




MapObj *MapEditor1::create_mapobj(Vector2 pos)
{

	MapObj *s;

	s = new MapObj(0, pos, 0.0, Tedit->tv->makespr());
	s->starnum = Tedit->tv->isel;

	return s;
}



void MapEditor1::move()
{
	MapEditor::move();

	SolarBody	*s;
	s = (SolarBody*) selection;
	
	s->stayhere = s->pos;	// in this case, movement is allowed ...
	
}




void MapEditor1::colorizeobj(SolarBody *s)
{
	SpaceSprite *spr = s->get_sprite();

	double temperature = ved->values[0]->value;
	
	double rat, gat, bat;	// extra atmospheric weight.
	rat = 0.7;	// you don't have that on a starship,
	gat = 0.6;	// but well, this makes it more understandable
	bat = 0.4;	// for us earthdwellers ... otherwise the sun would look slightly blueish ...
	// the blue reduction is somewhat exaggerated
	
	double rc, gc, bc;
	rc = spec_r(temperature) * rat;
	gc = spec_g(temperature) * gat;
	bc = spec_b(temperature) * bat;
	
	balance(&rc, &gc, &bc);
	colorize(spr, rc, gc, bc);
	brighten(spr);
}






