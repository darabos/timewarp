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

#include "../melee/mshot.h"


static GamePlanetscan::ThePlaya *localplayer;




static bool useframe1 = true;//false;		// the big area with the planet
static bool useframe2 = true;		// the smaller area: the scan surface
BITMAP *bmpframe2 = 0;;

double scalesurface = 8.0;



const int ID_MINERAL = 0x0f5b3a7ec;

class MineralType
{
public:
	MineralType();
	char id[64];	// common, exotic, etc.
	double value;	// RU per weight
	int ilist;

	SpaceSprite *sprite;
	// should have 3 frames, each matching a weight range
	// 1: weight < 5
	// 2: weight < 10
	// 3: weight > 15
	// or something ...

	void init(char *aid, int k);
};

MineralType::MineralType()
{
	sprite = 0;
	value = 1.0;
}

void MineralType::init(char *aid, int k)
{
	strcpy(id, aid);
	char *tmp = strrchr(id, '.');
	if (tmp)
		tmp[0] = 0;	// discard all following the '.'

	ilist = k;

	char fname[512];
	strcpy(fname, "gamex/gamedata/mineraltypes/");
	strcat(fname, id);
	strcat(fname, ".ini");

	set_config_file(fname);

	value = get_config_float(0, "value", 1.0);

	strcpy(fname, "gamex/gamedata/mineraltypes/");
	strcat(fname, id);
	strcat(fname, "_01.bmp");
	sprite = create_sprite( fname, SpaceSprite::MASKED, 3 );

}


const int ID_LIFEFORM = 0x0a9e0b3fe;


class LifeformType
{
public:
	LifeformType();
	char id[64];	// common, exotic, etc.
	double value;	// RU per weight

	double	health, damage;
	double	v;
	int		agressive;

	SpaceSprite *sprite;

	void init(char *aid);
};

LifeformType::LifeformType()
{
	sprite = 0;
	value = 1.0;
}

void LifeformType::init(char *aid)
{
	strcpy(id, aid);
	char *tmp = strrchr(id, '.');
	if (tmp)
		tmp[0] = 0;	// discard all following the '.'

	char fname[512];
	strcpy(fname, "gamex/gamedata/lifeformtypes/");
	strcat(fname, id);
	strcat(fname, ".ini");

	set_config_file(fname);

	value = get_config_float(0, "value", 1.0);

	health = get_config_float(0, "health", 1.0);
	damage = get_config_float(0, "damage", 1.0);
	v = get_config_float(0, "vel", 0.0);

	agressive = get_config_int(0, "agressive", 0);;

	strcpy(fname, "gamex/gamedata/lifeformtypes/");
	strcat(fname, id);
	strcat(fname, "_01.bmp");
	sprite = create_sprite( fname, SpaceSprite::MASKED, 3 );

}




MineralType *mineraltype[16];

char **mineralnamelist;
int Nmineraltypes;

void init_mineraltypes()
{
	createfilelist(&mineralnamelist, &Nmineraltypes,
									"gamex/gamedata/mineraltypes/*.ini", 0);
	
	int i;
	for ( i = 0; i < Nmineraltypes; ++i )
	{
		char *id;

		id = mineralnamelist[i];


		MineralType *mt;
		mt = new MineralType();
		mt->init(id, i);

		mineraltype[i] = mt;
	}
}

MineralType *get_mineraltype(const char *id)
{
	int i = 0;

	for ( i = 0; i < Nmineraltypes; ++i )
	{
		if (strcmp(id, mineraltype[i]->id) == 0)
			return mineraltype[i];
	}
	return 0;
}



LifeformType *lifeformtype[16];
char **lifeformnamelist;
int Nlifeformtypes;


void init_lifeformtypes()
{
	createfilelist(&lifeformnamelist, &Nlifeformtypes,
									"gamex/gamedata/lifeformtypes/*.ini", 0);
	
	int i;
	for ( i = 0; i < Nmineraltypes; ++i )
	{
		char *id;

		id = lifeformnamelist[i];


		LifeformType *lt;
		lt = new LifeformType();
		lt->init(id);

		lifeformtype[i] = lt;
	}
}


LifeformType *get_lifeformtype(const char *id)
{
	int i = 0;

	for ( i = 0; i < Nlifeformtypes; ++i )
	{
		if (strcmp(id, lifeformtype[i]->id) == 0)
			return lifeformtype[i];
	}
	return 0;
}




class Mineral : public MapObj
{
public:
	double	weight;
	MineralType	*type;

	Mineral(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite);
	virtual void calculate();
	virtual void inflict_damage(SpaceObject *other);

	void init(char *section, MineralType *t);

	virtual void animate(Frame *f);

	void mined();
};

Mineral::Mineral(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite)
:
MapObj(creator, opos, oangle, osprite)
{
	id = ID_MINERAL;

	layer = LAYER_SHIPS;
	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;
}

void Mineral::init(char *section, MineralType *t)
{
//	const char *s;
//	s = get_config_string(section, "type", "common");
//	type = get_mineraltype(s);
//	sprite = type->sprite;

	type = t;

	weight = get_config_int(section, "weight", 1);


	sprite_index = weight/5;
	if (sprite_index > 2)
		sprite_index = 2;
}

void Mineral::calculate()
{
}


void Mineral::inflict_damage(SpaceObject *other)
{
	if (other == localplayer)
	{
		damage_factor = weight;			// cargo increase ...
		MapObj::inflict_damage(other);	// it's left to "other" to see if it's a mineral through the ID

		localplayer->handle_mineral(this);
		//state = 0;	// this is done by the localplayer.
	}
}

void Mineral::mined()
{
	// when this thing is mined, its state = 0 ...
	state = 0;
}


void Mineral::animate(Frame *f)
{
	if (useframe1)
		MapObj::animate(f);

	// show the bitmap on the scan-map.
	if (useframe2)
	{
		//MapObj::animate(frame2);
		double s = bmpframe2->w / 400.0;
		int x = pos.x * s / scalesurface - 0.5*sprite->get_bitmap(0)->w;
		int y = pos.y * s / scalesurface - 0.5*sprite->get_bitmap(0)->h;
		sprite->draw(x, y, sprite_index, bmpframe2);
	}
}


Mineral *init_mineral(char *section, Vector2 pos)
{
	const char *s;
	s = get_config_string(section, "type", "common");
	MineralType	*type;
	type = get_mineraltype(s);

	Mineral *m;
	m = new Mineral(0, pos, 0, type->sprite);
	
	m->init(section, type);	// handles the rest of the ini file ...

	return m;
}




class Lifeform : public MapObj
{
public:
	double	health;
	double	v;
	int		agressive;
	double	animframelen;
	LifeformType	*type;

	Lifeform(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite);
	virtual ~Lifeform();
	virtual void calculate();

	virtual void inflict_damage(SpaceObject *other);
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);

	void init(char *section, LifeformType *t);

	virtual void animate(Frame *f);

	Periodics *per;

	void mined();
};

Lifeform::Lifeform(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite)
:
MapObj(creator, opos, oangle, osprite)
{
	id = ID_LIFEFORM;

	layer = LAYER_SHIPS;
	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;

	animframelen = 0.25;	// in seconds
	per = new Periodics(animframelen);

	sprite_index = 0;

	health = 1;
	damage_factor = 0;
	v = 0;
	agressive = 0;
}


Lifeform::~Lifeform()
{
	delete per;
}


void Lifeform::init(char *section, LifeformType *t)
{
	type = t;

	health = type->health;
	damage_factor = type->damage;
	v = type->v;
	agressive = type->agressive;

}

void Lifeform::calculate()
{
	if (!(target && target->exists()))
	{
		target = 0;
		//return;
	}

	// animation
	if (per->update())
	{
		++sprite_index;
		if (sprite_index >= sprite->frames())
			sprite_index = 0;
	}

	if (agressive && v && target)
	{
		// move towards the target, if it's close ..
	}
}


void Lifeform::inflict_damage(SpaceObject *other)
{
	MapObj::inflict_damage(other);	// it's left to "other" to see if it's a mineral through the ID
}


int Lifeform::handle_damage(SpaceLocation *source, double normal, double direct)
{
	if (source == localplayer)
	{
		health -= normal + direct;
		if (health <= 0)
			localplayer->handle_life(this);
	}
	return 0;
}



void Lifeform::animate(Frame *f)
{
	if (useframe1)
		MapObj::animate(f);

	// show the bitmap on the scan-map.
	if (useframe2)
	{
		//MapObj::animate(frame2);
		double s = bmpframe2->w / 400.0;
		int x = pos.x * s / scalesurface - 0.5*sprite->get_bitmap(0)->w;
		int y = pos.y * s / scalesurface - 0.5*sprite->get_bitmap(0)->h;
		sprite->draw(x, y, sprite_index, bmpframe2);
	}
}


void Lifeform::mined()
{
	state = 0;
}



Lifeform *init_lifeform(char *section, Vector2 pos)
{
	const char *s;
	s = get_config_string(section, "type", "plantA");
	LifeformType	*type;
	type = get_lifeformtype(s);

	Lifeform *l;
	l = new Lifeform(0, pos, 0, type->sprite);
	
	l->init(section, type);	// handles the rest of the ini file ...

	return l;
}




/*
class Structure : public MapObj
{
public:
	double	damage;
	char	*type;

	Structure(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite);
	virtual void calculate();
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

*/

/*
class SurfaceInfo : public SpacebodyInfo
{
public:
	//int	idcode;

	SurfaceInfo();

//	Mineral		**minerals;
//	Lifeform	**lifeforms;
//	Structure	**structures;

	int	Nlife, Nenergy, Nstructure;

	// environment stats
	double orbit, atmo, temp, mass, radius, gravity, day, tilt, albedo1, albedo2;
	int weatherclass, tectonicsclass;

	virtual void init(MapSpacebody *planet);
	virtual void write(MapSpacebody *planet);
};


class StarInfo : public SpacebodyInfo
{
public:

	double star_temp;	// in degr. Kelvin (K)
	double systemsize;	// in astr. units (ae)
	double sunradius;	// in ae (1 ae = earth orbit)

	StarInfo();

	// environment stats

	virtual void init(MapSpacebody *planet);
	virtual void write(MapSpacebody *planet);
};


SurfaceInfo::SurfaceInfo()
{
}

void SurfaceInfo::write(MapSpacebody *planet)
{
	 CHANGED: this is edited/written only in solarview/planetview mode !!
	char tmp[16];
	sprintf(tmp, "%X", planet->id);

	char fname[512];
	strcpy(fname, "gamex/gamedata/surface/");
	strcat(fname, tmp);
	strcat(fname, ".ini");

	set_config_file(fname);



	set_config_float("stats", "albedo1", albedo1);
	set_config_float("stats", "albedo2", albedo2);

	set_config_float("stats", "atmo", atmo);
	set_config_float("stats", "temp", temp);
	set_config_float("stats", "mass", mass);
	set_config_float("stats", "radius", radius);
	set_config_float("stats", "gravity", gravity);
	set_config_float("stats", "day", day);
	set_config_float("stats", "tilt", tilt);

	set_config_int("stats", "weather", weatherclass);
	set_config_int("stats", "tectonics", tectonicsclass);
	
}


void SurfaceInfo::init(MapSpacebody *planet)
{
	char tmp[16];
	sprintf(tmp, "%X", planet->id);

	char fname[512];
	strcpy(fname, "gamex/gamedata/surface/");
	strcat(fname, tmp);
	strcat(fname, ".ini");

	set_config_file(fname);


	// the plotted inner part of the solar system is zoomed-in this way...
	double systemsize = 100.0;	// in astr. units (ae)
	orbit = exp(planet->position.length() * systemsize / 1000.0);

	double star_temp = 6000.0;	// in degr. Kelvin (K)
	double sunradius = 0.001;	// ?? in ae ...
	double tempref;				// if there is no atmosphere ...
	tempref = star_temp * sqrt(0.5 * sunradius / orbit);
	// then this is the temp, for airless moons and such, with 0 albedo,
	// on average over the whole surface (incl. night-side, yeah otherwise it wouldn't
	// be an average right;). This is of course completely meaningless 
	// cause it can vary from 300K on the equator facing the sun, to 0K on the night
	// side and the polar caps. But ... well, vehicles can perhaps operate in
	// low temps relatively well cause they generate their own heat, but in
	// high temps very poorly cause they can't cool themselves. So, it makes "some"
	// sense ... that would make a moving "hotspot" for a rotating planet important
	// perhaps ? Actual temperatures varying on the surface facing the sun would be:
	// tempref = star_temp * sqrt(sunradius / orbit) * cos(lat) * cos(lon);
	// where lat=latitude, lon=olngitude (not taking albedo or something into account).
	// and 0 temperature in the night-side.

	// albedo is the fraction of energy that is radiated back into space ...
	// and since energy ~ temp-to-the-power-4, albedo is root/root
	// high value means, lots of light (=energy) is reflected. It can be radiated
	// back because of the surface, or atmosphere (clouds). The reflected energy
	// never reaches the planet and doesn't contribute to temperature increase.
	albedo1 = 0.4;	// typical for earth
	albedo1 = get_config_float("stats", "albedo1", 0.0);
	tempref *= sqrt(sqrt(1 - albedo1));

	// green-house effect in case of atmosphere; radiated energy has different
	// frequency (cause T is lower), and therefore a different overall albedo
	// for the outgoing energy can be used, than what's used for incoming energy.
	albedo2 = 0;	// nothing is reflected back to the planet, all is passed to space.
	albedo2 = get_config_float("stats", "albedo2", 0.0);
	tempref /= sqrt(sqrt(1 - albedo2));

	atmo = get_config_float("stats", "atmo", 1.0);
	temp = get_config_float("stats", "temp", 1.0);
	mass = get_config_float("stats", "mass", 1.0);
	radius = get_config_float("stats", "radius", 1.0);
	gravity = get_config_float("stats", "gravity", 1.0);
	day = get_config_float("stats", "day", 1.0);
	tilt = get_config_float("stats", "tilt", 1.0);
	weatherclass = get_config_int("stats", "weather", 1);
	tectonicsclass = get_config_int("stats", "tectonics", 1);
}




StarInfo::StarInfo()
{
	star_temp = 6000.0;	// in degr. Kelvin (K)
	systemsize = 100.0;	// in astr. units (ae)
	sunradius = 0.001;	// ?? in ae ...
}

void StarInfo::write(MapSpacebody *star)
{
	char tmp[16];
	sprintf(tmp, "%X", star->id);

	char fname[512];
	strcpy(fname, "gamex/gamedata/surface/");
	strcat(fname, tmp);
	strcat(fname, ".ini");

	set_config_file(fname);

	set_config_float("stats", "temp", star_temp);
	set_config_float("stats", "systemsize", systemsize);
	set_config_float("stats", "radius", sunradius);
}

void StarInfo::init(MapSpacebody *star)
{
	char tmp[16];
	sprintf(tmp, "%X", star->id);

	char fname[512];
	strcpy(fname, "gamex/gamedata/surface/");
	strcat(fname, tmp);
	strcat(fname, ".ini");

	set_config_file(fname);

	star_temp = get_config_float("stats", "temp", 6000);
	systemsize = get_config_float("stats", "systemsize", 100.0);
	sunradius = get_config_float("stats", "radius", 0.001);
}

*/






void GamePlanetscan::init_menu()
{
	// place the menu into video-memory, cause we're using this as basis for
	// drawing; the game draws onto part of the menu.
	T = new TWindow("gamex/interface/planetscan", 0, 0, game_screen, true);

	maparea = new Area(T, "map_");

	surf_area = new Area(T, "surface_");
	bmpframe2 = surf_area->backgr;
}



void GamePlanetscan::init()
{

	GameBare::init();

	
	//size = Vector2(H, H*tempframe->ratio);
	size = surf_area->size;	// the planet surface has that many pixels.
	size *= scalesurface;		// the planet surface is "extra big" for melee-type purpose
	prepare();

	wininfo.zoomlimit(1.0, 10.0);
	wininfo.set_zoom(1.0);


	// create star objects ?!
	int istar, iplanet, imoon;
	MapSpacebody *starmap, *solarmap, *planetmap, *moonmap;
	starmap = mapeverything.sub[0];	// use the starmap of the 1st region

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
		typestr = planettypelist->type[moonmap->type].type_string;
		
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
	player->pos = 0.5 * size;//Vector2(500,500);

	player->collide_flag_anyone = ALL_LAYERS;
	player->collide_flag_sameteam = ALL_LAYERS;
	player->collide_flag_sameship = ALL_LAYERS;

	player->weaponsprite = create_sprite( "gamex/planetscan/shot_01.bmp", SpaceSprite::MASKED, 64 );

	localplayer = player;




	
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



	// test:
	// place a number of mineral sprites on the surface, of any type.

	init_mineraltypes();
	init_lifeformtypes();

	int N = 5;
	int i;
	for ( i = 0; i < N; ++i )
	{
		int k;

		k = random(Nmineraltypes);
		SpaceSprite *spr;
		spr = mineraltype[k]->sprite;

		Mineral *mine;
		mine = new Mineral(0, random(Vector2(400,200)) * scalesurface, 0, spr);
		mine->type = mineraltype[k];
		mine->weight = 1;
		//mine->init(mineralnamelist[k]);

		add(mine);

		
		k = random(Nlifeformtypes);
		spr = lifeformtype[k]->sprite;

		Lifeform *life;
		life = new Lifeform(0, random(Vector2(400,200)) * scalesurface, 0, spr);
		//mine->init(mineralnamelist[k]);

		add(life);
	}

}


void GamePlanetscan::quit()
{
	delete playerspr;
	destroy_bitmap(map_bmp);

	// transfer the mined minerals to the mothership:
	int i;
	for ( i = 0; i < 16; ++i )
		playerinfo.mineral_weight[i] += localplayer->mineral_weight[i];
	playerinfo.bio_weight += localplayer->bio_weight;

	GameBare::quit();
}




void GamePlanetscan::calculate()
{
	// this viewtype needs no zoom, and always centers on the center of the planet system map.
	double d, dmin;
	d = player->pos.y;
	dmin = 0.5 * space_view_size.y * space_zoom;
	if (d < dmin)
		d = dmin;
	else if (d > map_size.y - dmin)
		d = map_size.y - dmin;
	else
		d = player->pos.y;
	wininfo.center(Vector2(player->pos.x, d));


	GameBare::calculate();


	double dt = frame_time * 1E-3;

	if (key[KEY_UP])
		player->accelerate(0, player->angle, 0.1*dt, 0.5);

	if (key[KEY_LEFT])
		player->angle -= 0.5*PI2 * dt;

	if (key[KEY_RIGHT])
		player->angle += 0.5*PI2 * dt;

	if (keyhandler.keyhit[KEY_ENTER])
		player->activate_weapon();


	//rotatingplanet->calculate();

}

void GamePlanetscan::animate(Frame *frame)
{
	// redraw the planet background.
	blit(map_bmp, surf_area->backgr, 0, 0, 0, 0, map_bmp->w, map_bmp->h);

	if (!useframe1)
	{
		// draw the rotating planet, but only if you're not actually
		// exploring it...
		int x, y;
		x = (maparea->backgr->w - rotatingplanet->size.x) / 2;
		y = (maparea->backgr->h - rotatingplanet->size.y) / 2;
		rotatingplanet->animate_pre();
		rotatingplanet->get_sprite()->draw(x, y, 0, maparea->backgr);

	} else {
		// otherwise, draw an enlarged version of the background on there.

		
		int x, y, w, h;

		w = (map_bmp->w * scalesurface) * space_zoom;
		h = (map_bmp->h * scalesurface) * space_zoom;

		x = -space_center.x * space_zoom + 0.5 * space_view_size.x;
		y = -space_center.y * space_zoom + 0.5 * space_view_size.y;

		stretch_blit(map_bmp, maparea->backgr,
			0, 0, map_bmp->w, map_bmp->h,
			x, y, w, h);

		stretch_blit(map_bmp, maparea->backgr,
			0, 0, map_bmp->w, map_bmp->h,
			x+w, y, w, h);

		stretch_blit(map_bmp, maparea->backgr,
			0, 0, map_bmp->w, map_bmp->h,
			x-w, y, w, h);

		// also, show the player position, but on the lower map ... duh ...

		Vector2 P;
		P = player->pos;
		P /= scalesurface;
		int d = 5;
		int c = makecol(250,50,50);
		hline(bmpframe2, P.x-d, P.y, P.x+d, c);
		vline(bmpframe2, P.x, P.y-d, P.y+d, c);

	}

	ti = true;

	GameBare::animate(frame);

}



GamePlanetscan::ThePlaya::ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf)
:
LocalPlayerInfo(osprite, playinf)
{
	pos = 0;		// locate the player somewhere on the local map

	int i;
	for ( i = 0; i < 16; ++i )
		mineral_weight[i] = 0;
	bio_weight = 0;
	tot_min_weight = 0;

	max_weight = 30;
}


int GamePlanetscan::ThePlaya::activate_weapon()
{
	double weaponVelocity = 0.1;
	double weaponDamage = 1.0;
	double weaponRange = 100.0;	// in pixels
	double weaponArmour = 0;

	physics->add(new Missile(this, Vector2(0, (size.y*.25)), angle,
		weaponVelocity, weaponDamage,
		weaponRange, weaponArmour, this, weaponsprite));

	return 0;
}





void GamePlanetscan::ThePlaya::handle_mineral(Mineral *m)
{
	if (tot_min_weight + m->weight < max_weight)
	{
		tot_min_weight += m->weight;
		mineral_weight[m->type->ilist] += m->weight;

		m->mined();
	}
}



void GamePlanetscan::ThePlaya::handle_life(Lifeform *l)
{
	bio_weight += l->type->value;
	l->mined();
}



void GamePlanetscan::handle_edge(SpaceLocation *s)
{
	while (s->pos.x < 0)			s->pos.x += map_size.x;
	while (s->pos.x > map_size.x)	s->pos.x -= map_size.x;

	if (s->pos.y < 0)
	{
		s->pos.y = 0;
		s->vel.y = 0;
	}

	if (s->pos.y > map_size.y-1E-6)
	{
		s->pos.y = map_size.y-1E-6;
		s->vel.y = 0;
	}
}




