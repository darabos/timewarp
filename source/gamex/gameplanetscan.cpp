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

#include "../melee/manim.h"

#include "../scp.h"
#include "../util/history.h"


#include "gamesolarview.h"
//#include "gameplanetview.h"
#include "gameplanetscan.h"

#include "gamedialogue.h"

#include "stuff/space_body.h"

#include "../melee/mshot.h"

#include "../twgui/gamebuttonevent.h"


static GamePlanetscan::ThePlaya *localplayer;



static bool useframe1 = true;//false;		// the big area with the planet
static bool useframe2 = true;		// the smaller area: the scan surface
BITMAP *bmpframe2 = 0;;

double scalesurface = 8.0;


class AnimatedObject : public Animation
{
public:

	AnimatedObject(SpaceLocation *creator, Vector2 opos,
					SpaceSprite *osprite, double frame_length);

	virtual void animate(Frame *f);
	virtual void inflict_damage(SpaceObject *other);
};

// spawns some object for a short time, can also "walk" a little.
class Spawner : public SpaceLocation
{
public:
	SpaceSprite *spawnsprite;
	double framelen;
	Periodics *per_life, *per_spawn;

	Spawner(SpaceLocation *creator, Vector2 lpos, double langle, SpaceSprite *aspawnsprite,
					double frame_length, double alifetime, double aspawninterval);
	void spawnit();
	virtual void calculate();
	virtual void animate(Frame *f);
};




AnimatedObject::AnimatedObject(SpaceLocation *creator, Vector2 opos,
					SpaceSprite *osprite, double frame_length)
:
Animation(creator, opos, 
	osprite, 0, osprite->frames(), frame_length, 
	DEPTH_SHIPS, 1)
{

	attributes &= ~(ATTRIB_UNDETECTABLE);

	damage_factor = 0.1;

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;

	layer = LAYER_SHIPS;
	set_depth(DEPTH_SHIPS);
}


void AnimatedObject::animate(Frame *f)
{
	BITMAP *bmp;
	bmp = sprite->get_bitmap(sprite_index);
	Vector2 P;
	P = corner(pos, size);
	masked_blit(bmp, f->surface,
		0, 0,
		//pos.x - 0.5*bmp->w - space_center.x, pos.y - 0.5*bmp->h - space_center.y,
		P.x, P.y,
		bmp->w, bmp->h);
}

void AnimatedObject::inflict_damage(SpaceObject *other)
{
	Animation::inflict_damage(other);
	state = 0;
}



Spawner::Spawner(SpaceLocation *creator, Vector2 lpos, double langle, SpaceSprite *aspawnsprite,
				 double frame_length, double alifetime, double aspawninterval)
:
SpaceLocation(creator, lpos, langle)
{
	framelen = frame_length;
	spawnsprite = aspawnsprite;
	per_life = new Periodics(alifetime);
	per_spawn = new Periodics(aspawninterval);
}


void Spawner::spawnit()
{
	physics->add(new AnimatedObject(this, pos, spawnsprite, framelen));
}

void Spawner::calculate()
{
	if (per_life->update())
	{
		state = 0;
		return;
	}

	if (per_spawn->update())
		spawnit();
}

void Spawner::animate(Frame *f)
{
	// nothing
}




class FireStorm : public Spawner
{
public:

	double v, Rran, Aran;

	FireStorm(SpaceLocation *creator, Vector2 lpos, double langle, SpaceSprite *aspawn,
				 double frame_length, double alifetime, double aspawninterval,
				 double av, double aRran, double aAran);

	virtual void calculate();
};

FireStorm::FireStorm(SpaceLocation *creator, Vector2 lpos, double langle,
					SpaceSprite *aspawn,
					double frame_length, double alifetime, double aspawninterval,
					double av, double aRran, double aAran)
:
Spawner(creator, lpos, langle, aspawn, frame_length, alifetime, aspawninterval)
{
	v = av;
	Rran = aRran;	// scattered positioning
	Aran = aAran;	// change in angle
}

void FireStorm::calculate()
{
	Spawner::calculate();

	pos += v * frame_time * unit_vector(angle) + Vector2(random(-Rran,Rran), random(-Rran,Rran));
	angle += random(-Aran, Aran);
}




/* Structure type:
Each structure is unique. It is decribed by these files:
id.ini
id.bmp
id.dialogue
*/

const int ID_STRUCTURE = 0x06b03d8a1;

class StructureType
{
public:
	StructureType();
	
	char id[64];

	SpaceSprite *sprite;

	void init(char *aid);
};


StructureType::StructureType()
{
	sprite = 0;
	id[0] = 0;
}

void StructureType::init(char *aid)
{
	strcpy(id, aid);

	char fname[512];

	sprintf(fname, "gamex/gamedata/structuretypes/%s_01.bmp", id);
	sprite = create_sprite( fname, SpaceSprite::MASKED );
}


StructureType *structuretype[128];

//char **mineralnamelist;
//int Nmineraltypes;

void init_structuretypes()
{
	
	int i;
	for ( i = 0; i < structuretypelist->N; ++i )
	{
		char *id;

		id = structuretypelist->type[i].type_string;


		StructureType *st;
		st = new StructureType();
		st->init(id);

		structuretype[i] = st;
	}
}


StructureType *get_structuretype(char *id)
{
	int i;
	i = structuretypelist->get_index(id, 0);

	return structuretype[i];
}





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
	sprite = create_sprite( fname, SpaceSprite::MASKED );

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
	sprite = create_sprite( fname, SpaceSprite::MASKED );

}




MineralType *mineraltype[16];

//char **mineralnamelist;
//int Nmineraltypes;

void init_mineraltypes()
{
//	createfilelist(&mineralnamelist, &Nmineraltypes,
//									"gamex/gamedata/mineraltypes/*.ini", 0);
	
	int i;
	for ( i = 0; i < mineraltypelist->N; ++i )
	{
		char *id;

		id = mineraltypelist->type[i].type_string;


		MineralType *mt;
		mt = new MineralType();
		mt->init(id, i);

		mineraltype[i] = mt;
	}
}

MineralType *get_mineraltype(char *id)
{
	int i;
	i = mineraltypelist->get_index(id, 0);

	return mineraltype[i];
}



LifeformType *lifeformtype[16];
char **lifeformnamelist;
int Nlifeformtypes;


void init_lifeformtypes()
{
	createfilelist(&lifeformnamelist, &Nlifeformtypes,
									"gamex/gamedata/lifeformtypes/*.ini", 0);
	
	int i;
	for ( i = 0; i < Nlifeformtypes; ++i )
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


	sprite_index = iround(weight/5);
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
		int x = iround(pos.x * s / scalesurface - 0.5*sprite->get_bitmap(0)->w);
		int y = iround(pos.y * s / scalesurface - 0.5*sprite->get_bitmap(0)->h);
		sprite->draw(x, y, sprite_index, bmpframe2);
	}
}


/*
Mineral *init_mineral(char *section, Vector2 pos)
{
	char s[512];
	strcpy(s, get_config_string(section, "type", "common"));
	MineralType	*type;
	type = get_mineraltype(s);

	Mineral *m;
	m = new Mineral(0, pos, 0, type->sprite);
	
	m->init(section, type);	// handles the rest of the ini file ...

	return m;
}
*/




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
		int x = iround(pos.x * s / scalesurface - 0.5*sprite->get_bitmap(0)->w);
		int y = iround(pos.y * s / scalesurface - 0.5*sprite->get_bitmap(0)->h);
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






class Structure : public MapObj
{
public:
	StructureType	*type;
	GamePlanetscan *g;

	Structure(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite);
	virtual void calculate();
	virtual void inflict_damage(SpaceObject *other);

	void init(char *section, StructureType *t, GamePlanetscan *ag);

	virtual void animate(Frame *f);

};

Structure::Structure(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite)
:
MapObj(creator, opos, oangle, osprite)
{
	id = ID_STRUCTURE;

	layer = LAYER_SHIPS;
	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;

	g = 0;
}

void Structure::init(char *section, StructureType *t, GamePlanetscan *ag)
{
	g = ag;
	type = t;
	sprite_index = 0;
}

void Structure::calculate()
{
}


void Structure::inflict_damage(SpaceObject *other)
{
	if (other == localplayer)
	{
		//damage_factor = 0;
		//MapObj::inflict_damage(other);	// it's left to "other" to see if it's a structure through the ID

		//localplayer->handle_mineral(this);
		//state = 0;	// this is done by the localplayer.

		char txt[512];
		sprintf(txt, "gamex/gamedata/structuretypes/%s.dialog", type->id);

		GameAliendialog *ad;
		ad = new GameAliendialog();
		ad->set_dialog(txt);
		g->gamerequest = ad;

		collide_flag_anyone = 0;
		collide_flag_sameteam = 0;
		collide_flag_sameship = 0;
	}
}


void Structure::animate(Frame *f)
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

/*
Structure *init_structure(char *section, Vector2 pos, GamePlanetscan *ag)
{
	char s[512];
	strcpy(s, get_config_string(section, "type", "common"));
	StructureType	*type;
	type = get_structuretype(s);

	Structure *m;
	m = new Structure(0, pos, 0, type->sprite);
	
	m->init(section, type, ag);	// handles the rest of the ini file ...

	return m;
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
	size.x = surf_area->size.x;	// the planet surface has that many pixels.
	size.y = surf_area->size.y;
	size *= scalesurface;		// the planet surface is "extra big" for melee-type purpose
	prepare();

	wininfo.zoomlimit(1.0, 10.0);
	wininfo.set_zoom(1.0);


	// create star objects ?!
	int istar, iplanet, imoon;
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
	

	init_mineraltypes();
	init_lifeformtypes();
	init_structuretypes();

	//SpaceSprite *spr;

	char txt[512];

	if ( !moonmap )
		body = planetmap;
	else
		body = moonmap;

	sprintf(txt, "gamex/gamedata/surface/%08X.ini", body->id);
	set_config_file(txt);
	const char *st;
	st = get_config_string(0, "surface", "default");
	sprintf(txt, "gamex/planetscan/surface_%s_01.bmp", st);
	map_bmp = load_bitmap(txt, 0);


	// init stats ...
	sprintf(txt, "gamex/planetscan/surface_%s.ini", st);
	set_config_file(txt);

	nmin = get_config_int(0, "NumMin", 0);
	nmax = get_config_int(0, "NumMax", 10);
	avsize = get_config_int(0, "AvSize", 10);

	int i;
	for ( i = 0; i < mineraltypelist->N; ++i )
		frac[i] = get_config_float("fraction", mineraltypelist->type[i].type_string, 0.0);

	int totfrac = 0;
	for ( i = 0; i < mineraltypelist->N; ++i )
		totfrac += iround(frac[i]);

	for ( i = 0; i < mineraltypelist->N; ++i )
		frac[i] /= totfrac;
	// so that the sum of all fractions == 1

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


	team_player = new_team();
	team_enemy  = new_team();
	player->set_team(team_player);


	
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


	// read the mineral loactions/ types from disk :
	
	sprintf(txt, "gamex/gamedata/surface/%08X.ini", body->id);
	set_config_file(txt);

	int N;
	N = get_config_int("minerals", "N", 0);

	for ( i = 0; i < N; ++i )
	{
		char id[512];
		char val[512];

		sprintf(id, "mineral%03i", i);
		strcpy(val, get_config_string("minerals", id, ""));

		double x, y;
		sscanf(val, "%lf %lf %s", &x, &y, id);

		int k;
		k = mineraltypelist->get_index(id, 0);
	
		SpaceSprite *spr;
		spr = mineraltype[k]->sprite;

		mine[i] = new Mineral(0, Vector2(x,y) * scalesurface, 0, spr);
		mine[i]->type = mineraltype[k];
		mine[i]->weight = 1;
		//mine->init(mineralnamelist[k]);

		add(mine[i]);
	}


	// read the structures
	N = get_config_int("structures", "N", 0);

	for ( i = 0; i < N; ++i )
	{
		char id[512];
		char val[512];

		sprintf(id, "structure%03i", i);
		strcpy(val, get_config_string("structures", id, ""));

		double x, y;
		sscanf(val, "%lf %lf %s", &x, &y, id);

		int k;
		k = structuretypelist->get_index(id, 0);
	
		SpaceSprite *spr;
		spr = structuretype[k]->sprite;

		Structure *st;
		st = new Structure(0, Vector2(x,y) * scalesurface, 0, spr);
		st->type = structuretype[k];
		st->g = this;

		add(st);
	}




	// an extra window, with "random" buttons so that you can re-randomize the minerals
	// and lifeform map (given the constraints in the surface.ini file)

	Pran = new Popup("gamex/interface/planetscan/edit", 0, 0, T->screen);
	Pran->exclusive = false;

	branmin = new Button(Pran, "ranm_");
	branlif = new Button(Pran, "ranl_");


	Pran->layer = 1;
	T->layer = 2;

	T->add(Pran);
	T->tree_doneinit();

	Pran->show();
	Pran->focus();

	branmin->bind(new BEvent<GamePlanetscan>(this, &GamePlanetscan::handle_ranmin, 0));


	// hazards ...
	spr[0] = create_sprite("gamex/planetscan/hazards/fireball_01.bmp", SpaceSprite::MASKED);
	spr[1] = create_sprite("gamex/planetscan/hazards/firewall_01.bmp", SpaceSprite::MASKED);
	spr[2] = create_sprite("gamex/planetscan/hazards/lightning_01.bmp", SpaceSprite::MASKED);
	spr[3] = create_sprite("gamex/planetscan/hazards/quake_01.bmp", SpaceSprite::MASKED);
	spr[4] = create_sprite("gamex/planetscan/hazards/whirl_01.bmp", SpaceSprite::MASKED);
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
	if (next)
		return;

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
	if (next)
		return;

	// redraw the planet background.
	blit(map_bmp, surf_area->backgr, 0, 0, 0, 0, map_bmp->w, map_bmp->h);

	if (!useframe1)
	{
		// draw the rotating planet, but only if you're not actually
		// exploring it...
		int x, y;
		x = iround((maparea->backgr->w - rotatingplanet->size.x) / 2);
		y = iround((maparea->backgr->h - rotatingplanet->size.y) / 2);
		rotatingplanet->animate_pre();
		rotatingplanet->get_sprite()->draw(x, y, 0, maparea->backgr);

	} else {
		// otherwise, draw an enlarged version of the background on there.

		
		int x, y, w, h;

		w = iround((map_bmp->w * scalesurface) * space_zoom);
		h = iround((map_bmp->h * scalesurface) * space_zoom);

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
		hline(bmpframe2, iround(P.x-d), iround(P.y), iround(P.x+d), c);
		vline(bmpframe2, iround(P.x), iround(P.y-d), iround(P.y+d), c);

		handle_env();
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



void GamePlanetscan::handle_ranmin()
{
	int i;

	// delete existing mineral types from the game:
	for ( i = 0; i < num_items; ++i )
	{
		if (item[i]->id == ID_MINERAL)
			item[i]->state = 0;
	}

	// place a number of mineral sprites on the surface, of any type.

	int N;
	char txt[512];

	N = random(iround(nmin), iround(nmax));

	for ( i = 0; i < N; ++i )
	{
		// instead of :k = random(mineraltypelist->N);
		// choose from a probability (-density-) distribution of the available types:
		double r;
		int k;

		r = random(1.0);
		for ( k = 0; k < mineraltypelist->N; ++k )
		{
			r -= frac[k];
			if (r <= 0)
				break;
		}
		if (k == mineraltypelist->N)	// invalid value (should not occur)
			k = mineraltypelist->N - 1;


		SpaceSprite *spr;
		spr = mineraltype[k]->sprite;

		mine[i] = new Mineral(0, random(Vector2(400,200)) * scalesurface, 0, spr);
		mine[i]->type = mineraltype[k];
		mine[i]->weight = 1;
		//mine->init(mineralnamelist[k]);

		add(mine[i]);

	
		/*
		k = random(Nlifeformtypes);
		spr = lifeformtype[k]->sprite;

		Lifeform *life;
		life = new Lifeform(0, random(Vector2(400,200)) * scalesurface, 0, spr);
		//mine->init(mineralnamelist[k]);

		add(life);
		*/
	}


	// write the mineral loactions/ types to disk :
	
	sprintf(txt, "gamex/gamedata/surface/%08X.ini", body->id);
	set_config_file(txt);

	for ( i = 0; i < N; ++i )
	{
		char id[512];
		char val[512];

		set_config_int("minerals", "N", N);

		sprintf(id, "mineral%03i", i);
		sprintf(val, "%f %f %s", mine[i]->pos.x/scalesurface, mine[i]->pos.y/scalesurface,
		mine[i]->type->id);
		set_config_string("minerals", id, val);
	}

	flush_config_file();


}




//	Spawner(SpaceLocation *creator, Vector2 lpos, double langle, SpaceSprite *aspawnsprite,
//					double frame_length, double alifetime, double aspawninterval);
void GamePlanetscan::handle_env()
{
	FireStorm *fs = 0;

	// meteors
	if (random(10) == 0)
	{
		
		double a = random(0.1 * PI2, 0.2 * PI2);
		double da = 1.0;
		double ra = 1000.0;
		double v = 0.2;


		fs = new FireStorm(0, player->pos+Vector2(random(-400,400),random(-200,200)), 
			a, spr[0], 75, 1.5, 0.05,
			v, 1.0, 0.0);

	}

	// firewall
	if (random(20) == 0)
	{
		
		double a = random(PI2);
		double da = 1.0;
		double ra = 1000.0;
		double v = 0.1;


		fs = new FireStorm(player, player->pos+Vector2(random(-400,400),random(-200,200)), 
			a, spr[1], 50, 3.0, 0.05,
			v, 2.0, 0.1*PI);

	}

	// lightning
	if (random(20) == 0)
	{
		
		double a = random(PI2);
		double da = 1.0;
		double ra = 1000.0;
		double v = 0;


		fs = new FireStorm(player, player->pos+Vector2(random(-400,400),random(-200,200)), 
			a, spr[2], 50, 0.5, 0.05,
			v, 16.0, 0.0);
	}

	// quake
	if (random(20) == 0)
	{
		
		double a = random(PI2);
		double da = 1.0;
		double ra = 1000.0;
		double v = 0.2;


		fs = new FireStorm(player, player->pos+Vector2(random(-400,400),random(-200,200)), 
			a, spr[3], 100, 2.0, 0.05,
			v, 2.0, 0.1*PI);
	}

	// whirl
	if (random(20) == 0)
	{
		
		double a = random(PI2);
		double da = 1.0;
		double ra = 1000.0;
		double v = 0.0;


		fs = new FireStorm(player, player->pos+Vector2(random(-400,400),random(-200,200)), 
			a, spr[4], 100, 0.55, 0.05,
			v, 4.0, 0);

	}

	if (fs)
	{
		add(fs);
		fs->set_team(team_enemy);
	}
}


