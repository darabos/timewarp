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

// the possible subgames :
#include "gameplanetscan.h"
#include "gamemelee.h"
#include "gamedialogue.h"

#include "stuff/space_body.h"
#include "stuff/backgr_stars.h"

#include "../twgui/twgui.h"
#include "../twgui/twpopup.h"

#include "general/sprites.h"


static const int ID_MAP_PLANET = 0x0b8f934ae;

GamePlanetview *gplanet;



void GamePlanetview::init_menu()
{
	// place the menu into video-memory, cause we're using this as basis for
	// drawing; the game draws onto part of the menu.
	T = new TWindow("gamex/interface/planetview", 0, 0, game_screen, true);

	maparea = new Area(T, "map_");
}



void GamePlanetview::init()
{
	gplanet = this;

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
	starmap = mapeverything.sub[0];	// use the starmap of the 1st region


	istar = playerinfo.istar;
	iplanet = playerinfo.iplanet;

	if (istar == -1)
		istar = 0;
	if (iplanet == -1)
		iplanet = 0;

	// for testing:
	playerinfo.istar = istar;
	playerinfo.iplanet = iplanet;

	solarmap = starmap->sub[istar];	// use the solarsystem belonging to that star
	planetmap = solarmap->sub[iplanet];	// use the planet (and moons) belonging to that planet orbit.


	// Button that displays the name of the planet.
	starname = new TextEditBox(T, "starname_", usefont, planetmap->name, sizeof(planetmap->name));
	starname->set_textcolor(makecol(255,255,0));
	strcpy(oldstarname, planetmap->name);
	// (on exit, you should copy the (edited) name to the star/planet structure.



	// position of the planet relative to the sun
	Vector2 relplanetpos;
	relplanetpos = planetmap->position;
	

	Vector2 centerpos;

	centerpos = Vector2(0.5, 0.25) * size;


	SpaceObject *solarbody;


	load_planettypes(&planettypespr);
	load_surfacetypes(&surfacebmp);


	Vector2 Poffs;
	int col;
	double R;
	
	double b = b_default;
	ellipsparams(relplanetpos, b, R, Poffs, col);

	Vector2 sunpos;
	sunpos = centerpos - relplanetpos;	// "offset center" of the ellips.
	int k;
	k = planetmap->type;
	planetspr = new SpaceSprite(planettypespr[k]->get_bitmap(0));
	solarbody = new SolarBody(0, centerpos, 0.0, planetspr, sunpos, iplanet,
								sunpos+Poffs, R, b, makecol(115,0,0));
	
	Surface3D *s3d;
	s3d = new Surface3D();

	char tmp[512];
	sprintf(tmp, "gamex/gamedata/surface/%08X.ini", planetmap->id);
	set_config_file(tmp);
	strcpy(tmp, get_config_string(0, "surface", "default"));
	k = surfacetypelist->get_index(tmp, 0);
	
	int d;
	//r = planettypespr[k]->get_bitmap(0)->w;
	d = solarbody->get_sprite()->get_bitmap(0)->w;
	
	s3d->reset(d, gplanet->surfacebmp[k], 0, true, 1.0, 1.0, 1.0);	
	s3d->plot(solarbody->get_sprite()->get_bitmap(0));

	planetmap->o = solarbody;
	add(solarbody);
	
	solarbody->id = ID_MAP_PLANET;	// so that it's not editable by the mapeditor

	// load the star data
	int i;
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

		int r;
		r = planettypespr[k]->get_bitmap(0)->w;

		// what is the planet surface ...
		sprintf(tmp, "gamex/gamedata/surface/%08X.ini", planetmap->sub[i]->id);
		set_config_file(tmp);

		strcpy(tmp, get_config_string(0, "surface", "default"));
		k = surfacetypelist->get_index(tmp, 0);


		s3d->reset(r, surfacebmp[k], 0, true, 1.0, 1.0, 1.0);

		s3d->plot();
		planetspr = new SpaceSprite(s3d->dummy);

		// apply the planet-surface to this planet/moon
		// but, how ??
		
		solarbody = new SolarBody(0, P, 0.0, planetspr, centerpos+relplanetpos, i,
								centerpos+Poffs, R, b, col);

		planetmap->sub[i]->o = solarbody;
		add(solarbody);
	}


	team_player = new_team();
	team_aliens = new_team();

	// the player
	//SpaceSprite *playerspr;
	playerspr = create_sprite( "gamex/solarview/player_01.bmp", SpaceSprite::MASKED, 64 );
	player = new ThePlaya(playerspr, &playerinfo);
	add(player);


	locate_onedge_aligned_to_center(player, 0.5*size, 0.45*size);


	StarBackgr *sb;
	sb = new StarBackgr();
	sb->init(100, tempframe);
	add(sb);


	// stuff for the map editor

	SpaceSprite *mousespr;
	mousespr = create_sprite( "gamex/mouse/pointer_starmap.bmp", SpaceSprite::MASKED );
	ptr = new MousePtr (mousespr);
	add(ptr);

	T->layer = 2;		// always shown later

	mapeditor = new MapEditor3();
	mapeditor->set_game(this, ptr);

	//mapeditor->init_interface(T, usefont, planettypespr, surfacebmp);
	mapeditor->init_interface(T, usefont, planettypespr, planettypelist->N,
		surfacebmp, surfacetypelist->N);


	mapeditor->set_mapinfo( planetmap, 1.0);

	mapeditor->mapcenter = centerpos;

	T->tree_doneinit();

	mapeditor->Tedit->show();
	mapeditor->Tedit->focus();

	delete s3d;
}


void GamePlanetview::quit()
{
	delete playerspr;

	int i;
	for ( i = 0; i < planettypelist->N; ++i )
		delete planettypespr[i];

	for ( i = 0; i < surfacetypelist->N; ++i )
		del_bitmap(&surfacebmp[i]);
	delete surfacebmp;

	if (!hardexit)	// game is not quitted completely
	{
		// set the player position exactly equal to the planet for appearing in solar orbit
		MapSpacebody *solarmap;
		solarmap = mapeverything.sub[0]->sub[playerinfo.istar];
		playerinfo.pos = solarmap->sub[playerinfo.iplanet]->position * solarmap->scalepos;
		playerinfo.vel = 0;

		playerinfo.iplanet = -1;	// cause you've left the planet orbit
		playerinfo.angle = player->angle;
	}
	else
		playerinfo.sync2(player);

	if (strcmp(planetmap->name, oldstarname))
		mapeditor->maphaschanged = true;

	if (mapeditor->maphaschanged)
	{
		// write the map to disk
		mapeverything.save("gamex/mapinfo.txt");
	}

	GameBare::quit();
}



void GamePlanetview::refocus()
{
	// not needed here ...
	//playerinfo.sync(player);
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

		/*
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

		} else
		*/
		if (player->collisionwith->id == MAPOBJ_ID ||
					player->collisionwith->id == ID_MAP_PLANET)
		{
			player->vel = 0;
			
			if (player->collisionwith->id == MAPOBJ_ID)
				playerinfo.imoon = ((SolarBody*) player->collisionwith)->starnum;
			else
				playerinfo.imoon = -1;	// it's not a moon, but the planet then.

			// Now, check if the planet is used by a race (a colony), if so,
			// we should show a dialog screen; otherwise, we can suffice with
			// showing the planet surface which you can then explore...

			if (!gamerequest && !next)
			{
				RaceColony *rc;
				rc = racelist.findcolony(playerinfo.istar, playerinfo.iplanet, playerinfo.imoon);

				GameBare *g;

				if (rc != 0)
				{
					GameAliendialog *ad;
					ad = new GameAliendialog();
					ad->set_colony(rc);
					g = ad;
				} else {
					
					//GamePlanetscan *g;
					g = new GamePlanetscan();
				}
				
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



/*
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
*/



GamePlanetview::~GamePlanetview()
{
}



// copied and adapted from 3d planet.

Surface3D::Surface3D()
{
	image32bit = 0;
	dummy = 0;

	base_map_linear = 0;
	base_shade_linear = 0;
	base_spec_linear = 0;
	color_map_linear = 0;
	spec_map_linear = 0;
	base_map_and_shade_resorted = 0;
}


void Surface3D::clear()
{
	if (base_map_linear)
		base_map_linear = 0;
	if (base_shade_linear)
		base_shade_linear = 0;
	if (base_spec_linear)
		base_spec_linear = 0;
	if (color_map_linear)
		color_map_linear = 0;
	if (spec_map_linear)
		spec_map_linear = 0;
	if (base_map_and_shade_resorted)
		base_map_and_shade_resorted = 0;

	del_bitmap(&dummy);
	del_bitmap(&image32bit);
}

Surface3D::~Surface3D()
{
	clear();
}


void Surface3D::reset(int planet_diameter, BITMAP *color_map,
					  BITMAP *spec_map, bool invcolor,
					  double sun_r, double sun_g, double sun_b)
{
	STACKTRACE;

	clear();

	if (spec_map)
		PlanetUsespec = 1;
	else
		PlanetUsespec = 0;

	double spin, tilt, sun_horizangle, sun_vertangle;
	spin = 0;
	tilt = 0;
	sun_horizangle = 0;
	sun_vertangle = 0;

	double min_light = 0.5;
	double max_light = 1.1;

	

	mapW = color_map->w;
	mapH = color_map->h;


	image_size = planet_diameter;
	visual_size = image_size / 2 - 1;

	dummy = create_bitmap_ex(32, image_size, image_size);
	clear_to_color(dummy, makecol32(255,0,255));

	image32bit = create_bitmap_ex(32, image_size, image_size);
	clear_to_color(image32bit, makecol32(255,0,255));

	// there are 4 chars containing the colors
	// for a map of size W, H
	// and I keep an extra copy of the map (for the rotation extrapolates on there).
	color_map_linear = new unsigned char [4*mapW*mapH * 2];
	spec_map_linear = new unsigned char [4*mapW*mapH * 2];


	int ccc;
	int i,j;

	for (i = 0; i < mapW; i++)
	{
		for (j = 0; j < mapH; j++)
		{
			ccc=getpixel(color_map, i, j);

			int spec;
			if (spec_map)
				spec = getpixel(spec_map, i, j);
			else
				spec = makecol(200,200,200);

			unsigned char r, g, b;
			unsigned char sr, sg, sb;

			// AT THIS MOMENT, it's a good moment to find out in which way the
			// videocard interprets the colors ... as rgb, or as bgr ??!!

			// map coordinate
			int k = (2*mapW*mapH) - (j+1)*(2*mapW) + i;
			if (!invcolor)
			{
				r = getr32(ccc);
				g = getg32(ccc);
				b = getb32(ccc);
			} else {
				b = getr32(ccc);
				g = getg32(ccc);
				r = getb32(ccc);
			}

			// filter the colors by the reference sun
			r = (unsigned char)(sun_r * r);
			g = (unsigned char)(sun_g * g);
			b = (unsigned char)(sun_b * b);

			*((int*) &color_map_linear[4*k]) = makecol32(r,g,b);

			// repeat this for the spec map

			sr = (unsigned char)(sun_r * getr32(spec));
			sg = (unsigned char)(sun_g * getg32(spec));
			sb = (unsigned char)(sun_b * getb32(spec));

			*((int*) &spec_map_linear[4*k]) = makecol32(sr,sg,sb);


			// keep an extra copy in memory.
			k += mapW;
			*((int*) &color_map_linear[4*k]) = makecol32(r,g,b);

			*((int*) &spec_map_linear[4*k]) = makecol32(sr,sg,sb);

		}
	}

	theta = tilt * ANGLE_RATIO;
	fi =  0;// should always be 0 !!//125* ANGLE_RATIO;
	rad = 0;

	double L, D, d;
	double focus;
	
	double th=theta;

	base_map = (base_map_type*)malloc(sizeof(base_map_type)*image_size*image_size);

	base_map_linear = new unsigned int [image_size*image_size];
	base_shade_linear = new unsigned int [image_size*image_size];
	base_spec_linear = new unsigned int [image_size*image_size];

	base_map_and_shade_resorted = new unsigned int [2*image_size*image_size];

	double fi_s = sun_horizangle*ANGLE_RATIO;
	double th_s = sun_vertangle * ANGLE_RATIO; //sun position

	
	double ts[3][3];
	double tm[3][3];


	tm[0][0] = cos(th)*cos(fi);
	tm[0][1] = sin(th);
	tm[0][2] = -cos(th)*sin(fi);
	tm[1][0] = -sin(th)*cos(fi);
	tm[1][1] = cos(th);
	tm[1][2] = sin(th)*sin(fi);
	tm[2][0] = sin(fi);
	tm[2][1] = 0;
	tm[2][2] = cos(fi);

	ts[0][0] = cos(fi_s)*cos(th_s);
	ts[0][1] = -sin(fi_s);
	ts[0][2] = cos(fi_s)*sin(th_s);
	ts[1][0] = sin(fi_s)*cos(th_s);
	ts[1][1] = cos(fi_s);
	ts[1][2] = sin(fi_s)*sin(th_s);
	ts[2][0] = -sin(th_s);
	ts[2][1] = 0;
	ts[2][2] = cos(th_s);


	// x, y is in the drawing area;
	// z points outside the screen.

	Vector3D observer(0,0,1);
	Vector3D sunshine(cos(fi_s)*cos(th_s), sin(fi_s)*cos(th_s), -sin(th_s));
	sunshine.normalize();

	L = 10;
	D = L*L - 1;
	d = sqrt(D);

	double xx,yy,zz, rr, ff, tg2;
//	double xf, yf, zf;//final;
	double xs,ys,zs;//sun
	double lon, lat;

	focus = (visual_size) * d;//tan(view_angle/2);


	for (i=-image_size/2; i<image_size/2; i++)
	{

		int k = i + image_size/2;
		if (k >= image_size)	continue;
		jmin[k] = 100000;
		jmax[k] = 0;

		for (j=-image_size/2; j<image_size/2; j++)
		{
			if ((i*i+j*j) < visual_size*visual_size)
			{
				//-int k = i + image_size/2;
				int m = j + image_size/2;
				if (m >= image_size)	continue;

				if (m < jmin[k])
					jmin[k] = m;

				if (m > jmax[k])
					jmax[k] = m;


				
				ff = (i*i+j*j);
				tg2 = ff / (focus*focus);

				zz = sqrt(L*L - D*(1+tg2)) / (1 + tg2);
				rr = sqrt(1 - zz*zz);

				if (ff > 0) {
					xx = rr * i / sqrt(ff);
					yy = rr * j / sqrt(ff); }
				else {
					xx = 0; yy = 0; }
					
//				xx = i/double(visual_size);
//				yy = j/double(visual_size);
//				zz = sqrt(1 - xx*xx - yy*yy);

				xs = xx; ys = yy; zs = zz;
				// = position of the visible sphere point in 3D space ?

				Vector3D surfacenormal(xs, ys, zs);
				surfacenormal.normalize();
				// the point that is facing you ;)

				// now, calculate the light intensity.
				// two parts: how much light does a part of the earth receive,
				// and how much does it radiate out:

				double I, J;
				I = -1 * sunshine.dot(surfacenormal);	// incoming sunshine is in opposite direction from outgoing surface normal
//				I *= surfacenormal.dot(observer);		// this is directional shading... shouldn't be here.
				if (I < 0)
					I = 0;
				I = min_light + I*(max_light-min_light);


				// and, what about specular, i.e., reflections of light towards
				// the observer?
				Vector3D reflection(0,0,0);	// the reflected sun-light direction
				reflection = -1*(sunshine.cross(surfacenormal)).cross(surfacenormal) - sunshine.dot(surfacenormal) * surfacenormal;
				J = reflection.dot(observer);
				if (J < 0)
					J = 0;
				J *= J;	// to make it more pronounced :)
				J *= J;
				J *= J;
				J *= J;
				J *= 0.8;

				
				xx = tm[0][0] * xs + tm[0][1] * ys + tm[0][2] * zs;
				yy = tm[1][0] * xs + tm[1][1] * ys + tm[1][2] * zs;
				zz = tm[2][0] * xs + tm[2][2] * zs;

				// sun is to the right ?
				// observer is perp. to the screen ?


				if (fabsl(zz) > 1e-10) {
					if (zz > 0) lon = atan(xx/zz);
					else lon = PI+atan(xx/zz); }
				else
					if (xx > 0) lon = PI/2;
					else lon = -PI/2;

				if (yy*yy < 1-1e-10)
					lat = atan(yy/sqrt(1-yy*yy));
				else {
					if (yy > 0) lat = PI/2;
					else lat = -PI/2; }

	

				double mx = mapW * lon / (2*PI);
				while (mx >= mapW) mx -= mapW;
				while (mx < 0) mx += mapW;
				double my = mapH * (1 - lat/(PI/2)) / 2.0;
				while (my >= mapH) my -= mapH;
				while (my < 0) my += mapH;

				base_map[(i+image_size/2)*image_size+j+image_size/2].lat = my;
				base_map[(i+image_size/2)*image_size+j+image_size/2].lon = mx;



				base_map[(i+image_size/2)*image_size+j+image_size/2].diff = I;//l1;
				base_map[(i+image_size/2)*image_size+j+image_size/2].spec = J;//ll;
			}
			else {
				base_map[(i+image_size/2)*image_size+j+image_size/2].lat = 0;
				base_map[(i+image_size/2)*image_size+j+image_size/2].lon = 0;
				base_map[(i+image_size/2)*image_size+j+image_size/2].diff = 0;
				base_map[(i+image_size/2)*image_size+j+image_size/2].spec = 0;
			}

			int k = (i+image_size/2) * image_size + j + image_size/2;
			if (k >= image_size*image_size)
				k = image_size*image_size - 1;

			int mx2, my2;
			mx2 = (int)base_map[k].lon;
			if (mx2 >= mapW) mx2 -= mapW;
			my2 = (int)base_map[k].lat;
			if (my2 >= mapH) my2 -= mapH;

			base_map_linear[k] = mx2 + my2 * 2*mapW;	// twice cause there's an extra copy
														// of the map colors


			// so that the shading doesn't become excessive.

			unsigned char shading;
			int shading_toomuch;
			shading_toomuch = int(base_map[k].diff * 255);
			if (shading_toomuch > 255)
				shading = 255;
			else
				shading = shading_toomuch;
			// 255 means, brightness is almost unaffected.

			base_shade_linear[k] = shading;


			base_spec_linear[k] = int(base_map[k].spec * 255);

		}
	}


	// re-sort some arrays:
	unsigned int *resort = base_map_and_shade_resorted;
	for ( j = 0; j < image_size; ++j )
	{
		for ( i = 0; i < image_size; ++i )
		{
			int k = i*image_size + j;
			*resort = base_map_linear[k];
			++resort;

			unsigned char s1 = base_shade_linear[k];
			unsigned char s2 = base_spec_linear[k];
			*resort = s1 | s2 << 8;
			++resort;

		}
	}

	delete base_map;
	base_map = 0;

}

void Surface3D::plot()
{
	plot(dummy);
}

void Surface3D::plot(BITMAP *trg)
{
	STACKTRACE;


	int i,j;

	double dl = mapW*rad/360.0;
	while (dl >= mapW) dl -= mapW;

	unsigned int* imageptr = (unsigned int*) image32bit->dat;

	unsigned int *base_sorted = base_map_and_shade_resorted;

	//unsigned int d_shift = unsigned int(dl);
	unsigned int d_shift = (unsigned int)dl; //changed 7/1/2003 Culture20

	unsigned char *color_map_linear2 = &color_map_linear[d_shift << 2];	// a shift by dl int's
	unsigned char *spec_map_linear2 = &spec_map_linear[d_shift << 2];	// a shift by dl int's

	for (i=0; i<image_size; i++)
	{
		// NOTE: this loop can be altered such that it won't iterate over "empty" space
		// by adding boundaries jmin[i] and jmax[i]
		//for (j=0; j<image_size; j++)

		if (jmin[i] >= jmax[i])
		{
			imageptr += image_size;
			base_sorted += 2 * image_size;
			continue;
		}
		

		imageptr += jmin[i];	// cause it must start at position jmin

		//int m = i + jmin[i]*image_size;
		base_sorted += 2*jmin[i];


		for (j = jmin[i]; j <= jmax[i]; j++)
		{


			unsigned char *col;
			unsigned char *speccol;
			unsigned short int specshade;

			unsigned short int shade;
			unsigned int result, e;

			if (!PlanetUsespec)
			{
				col = &color_map_linear2[(*base_sorted) << 2];
				
				++base_sorted;
				
				shade = *((unsigned char*) base_sorted);
				
				result = 0;
				
				// red
				e = (*col) * shade;
				e &= 0x0FF00;
				result |= e;
				result <<= 8;
				
				// green
				++col;
				e = (*col) * shade;
				e &= 0x0FF00;
				result |= e;
				
				
				// blue
				++col;
				e = (*col) * shade;
				e >>= 8;
				result |= e;
				
				*imageptr = result;
				
				++imageptr;
				
				
				// Note that, since shade<256 and color<256, the result of the multiplication
				// will always fit within a short-int
				// And, shifting that resulting value to the right by one byte, automatically
				// generates a value <256.
				
				++base_sorted;

			} else {

				col = &color_map_linear2[(*base_sorted) << 2];
				speccol = &spec_map_linear2[(*base_sorted) << 2];
				
				++base_sorted;

				shade = *((unsigned char*) base_sorted);
				specshade = *((unsigned char*) base_sorted + 1);
				
				result = 0;
				
				// red
				e = (*col) * shade;
				e += (*speccol) * specshade;
				if (e & 0x010000)
					e = 0x0FF00;			// truncation.
				e &= 0x0FF00;
				result |= e;
				result <<= 8;
				
				// green
				++col;
				++speccol;
				e = (*col) * shade;
				e += (*speccol) * specshade;
				if (e & 0x010000)
					e = 0x0FF00;			// truncation.
				e &= 0x0FF00;
				result |= e;
				
				// blue
				++col;
				++speccol;
				e = (*col) * shade;
				e += (*speccol) * specshade;
				if (e & 0x010000)
					e = 0x0FF00;			// truncation.
				e &= 0x0FF00;
				e >>= 8;
				result |= e;

				*imageptr = result;

				++imageptr;
				++base_sorted;
			}
			

		}

		int memjump = image_size - jmax[i] - 1;
		imageptr += memjump;

		base_sorted += 2*memjump;
	}

	// blit can convert color depths (I think?)
	blit(image32bit, trg, 0, 0, 0, 0, image_size, image_size);

}




void MapEditor3::colorizeobj(SolarBody *s)
{
	//?
	int k;
	k = objmap->sub[s->starnum]->type;
	
	BITMAP *bmp;
	bmp = s->get_sprite()->get_bitmap(0);

	int d;
	//r = planettypespr[k]->get_bitmap(0)->w;
	d = bmp->w;
	
	Surface3D s3d;

	s3d.reset(d, gplanet->surfacebmp[isurfacetype], 0, true, 1.0, 1.0, 1.0);
	
	s3d.plot(bmp);
	//planetspr = new SpaceSprite(s3d->dummy);

	brighten(bmp);
}


