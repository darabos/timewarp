#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE

#include "../frame.h"
#include "../melee/mview.h"


#include "gamedata.h"


IndexTypeList *startypelist;
IndexTypeList *planettypelist;
IndexTypeList *moontypelist;

PlayerInfo		playerinfo;
MapEverything	mapeverything;
RaceManager		racelist;



void PlayerInfo::init(char *filename)
{
	configfilename = filename;

	set_config_file(filename);
	pos.x = get_config_float(0, "PosX", 0.0);
	pos.y = get_config_float(0, "PosY", 0.0);
	angle = get_config_float(0, "Angle", 0.0);

	istar = get_config_int(0, "Star", -1);
	iplanet = get_config_int(0, "Planet", -1);
	imoon = get_config_int(0, "Moon", -1);	// of planets and moons
}


void PlayerInfo::write()
{
	set_config_file(configfilename);
	set_config_float(0, "PosX", pos.x);
	set_config_float(0, "PosY", pos.y);
	set_config_float(0, "Angle", angle);

	set_config_int(0, "Star", istar);
	set_config_int(0, "Planet", iplanet);
	set_config_int(0, "Moon", imoon);
	//set_config_int(0, "PlanetCode", iplanetcode);
}


void PlayerInfo::sync(LocalPlayerInfo *p)
{
	pos = p->pos;
	angle = p->angle;
}



// For each game, you'll need some physical object to represent part of
// the general player information, which is relevant for that aspect of
// the game.

LocalPlayerInfo::LocalPlayerInfo(SpaceSprite *osprite, PlayerInfo *playinf)
:
SpaceObject(0, 0, 0, osprite)
{
	playerinfo = playinf;

	pos = playinf->pos;
	angle = playinf->angle;

	layer = LAYER_SHIPS;

	attributes |= ATTRIB_STANDARD_INDEX;

	collisionwith = 0;
}


void LocalPlayerInfo::calculate()
{
	collisionwith = 0;
	SpaceObject::calculate();
	//sprite_index = get_index(angle);
}


void LocalPlayerInfo::inflict_damage(SpaceObject *other)
{
	// use this to detect whether you've collided with something!
	collisionwith = other;
	SpaceObject::inflict_damage(other);
}











void MapEverything::init(char *filename)
{
	startypelist = new IndexTypeList(maxstartypes, "startypes.txt");
	planettypelist = new IndexTypeList(maxplanettypes, "planettypes.txt");
	moontypelist = new IndexTypeList(maxplanettypes, "moontypes.txt");
	
	FILE *f = fopen(filename, "rt");
	if (!f) { tw_error("failed to initialize map info");}

	fscanf(f, "%i", &Nregions);

	region = new MapSpacebody* [Nregions];

	int i;
	for ( i = 0; i < Nregions; ++i )
	{
		region[i] = new MapSpacebody();
		region[i]->init(f, 0);
	}

	fclose(f);

}



void MapEverything::save(char *filename)
{
	
	FILE *f = fopen(filename, "wt");
	if (!f)	{tw_error("failed to save map info");}

	fprintf(f, "%i\n\n", Nregions);

	int i;
	for ( i = 0; i < Nregions; ++i )
	{
		region[i]->save(f, 0);
	}

	fclose(f);
}


const bool hascontent(char *t)
{
	int i;

	i = 0;
	while (t[i])
	{
		if (t[i] != ' ' && t[i] > 20)
			return true;

		++i;
	}

	return false;
}

void MapSpacebody::init(FILE *f, int level)
{
	if (!f)
	{
		// default settings ..
		Nsub = 0;
		sub = 0;
		strcpy(name, "noname");
		type = 0;
		position = 0;
		o = 0;

		return;
	}

	char txt[512], chartype[512];
	txt[0] = 0;
	while (!hascontent(txt))
		fgets(txt, 512, f);		// reads a line, skipping empty lines

	strncpy(name, txt, 64);
	if (name[strlen(name)-1] == '\n')
		name[strlen(name)-1] = 0;

	fscanf(f, "%s", chartype);
	// this includes a newline character, which must be skipped:
	if (chartype[strlen(chartype)-1] == '\n')
		chartype[strlen(chartype)-1] = 0;

	// it's a region, star, planet, or moon.

	if (level == 0)
		type = -1;

	if (level == 1)
		type = startypelist->get_index(chartype);

	if (level == 2)
		type = planettypelist->get_index(chartype);

	if (level == 3)
		type = moontypelist->get_index(chartype);

	fscanf(f, "%lf %lf", &position.x, &position.y);
	fscanf(f, "%i\n\n", &Nsub);

	sub = new MapSpacebody* [Nsub];

	int i;
	for ( i = 0; i < Nsub; ++i )
	{
		sub[i] = new MapSpacebody();
		sub[i]->init(f, level+1);
	}

	scalepos = 1;
}



void MapSpacebody::save(FILE *f, int level)
{
	char base[128];
	int i;

	if (!f)
		return;

	for ( i = 0; i < level; ++i )
		base[i] = '\t';
	base[i] = 0;

	fprintf(f, "%s %s\n", base, name);

	if (level == 0)
		fprintf(f, "%s empty_type\n", base);

	if (level == 1)
		fprintf(f, "%s %s\n", base, startypelist->type[this->type].type_string);

	if (level == 2)
		fprintf(f, "%s %s\n", base, planettypelist->type[this->type].type_string);

	if (level == 3)
		fprintf(f, "%s %s\n", base, moontypelist->type[this->type].type_string);


	fprintf(f, "%s %lf %lf\n", base, position.x, position.y);
	fprintf(f, "%s %i\n\n", base, Nsub);

	for ( i = 0; i < Nsub; ++i )
		sub[i]->save(f, level+1);

}


int MapSpacebody::add(int level)
{

	int i;
	MapSpacebody **oldsub;

	oldsub = sub;
	sub = new MapSpacebody* [Nsub + 1];

	for ( i = 0; i < Nsub; ++i )
		sub[i] = oldsub[i];

	delete oldsub;

	sub[Nsub] = new MapSpacebody();
	sub[Nsub]->init(0, level);	// the level does not matter.

	++Nsub;

	return Nsub-1;
}



int MapSpacebody::rem(int k)
{

	if (k < 0 || k >= Nsub || Nsub == 0)
		return 0;

	int i;
	for ( i = k; i < Nsub-1; ++i )
		sub[i] = sub[i+1];
	--Nsub;

	return Nsub-1;
}



IndexTypeList::IndexTypeList(int omax, char *fname)
{
	max = omax;
	type = new IndexType [max];

	FILE *f;
	char line[512];
	
	strcpy(line, "gamex/types/");
	strcat(line, fname);
	f = fopen(line, "rt");
	if (!f)
	{
		error("File does not exist [%s]", line);
	}
	
	N = 0;

	while (fgets(line, 512, f))
	{
		// get rid of the \n character as well...
		int L = strlen(line);
		if (L > 0 && line[L-1] == '\n')
		{
			--L;
			line[L] = 0;
		}
		
		type[N].type_string = new char [L];
		strcpy(type[N].type_string, line);

		++N;

		if (N > max){tw_error("init: too many types");}
	}

	fclose(f);
}



IndexTypeList::~IndexTypeList()
{
	int i;
	for ( i = 0; i < N; ++i )
	{
		delete type[i].type_string;
	}
	delete type;
}



int IndexTypeList::get_index(char *typestr)
{
	int i;
	for ( i = 0; i < N; ++i )
	{
		if (strcmp(type[i].type_string, typestr) == 0)
			break;
	}

	if (i == N)		// no match
		return -1;
	else
		return i;	// a match
}












Patrol::Patrol()
{
	// default to a single system-fleet (and no hyperspace patrol fleets, and no capital (special) fleet)
	numhyperfleets = 0;
	numsystemfleets = 1;
	numcapitalfleets = 0;

	range = 0;
}




RaceSettlement::RaceSettlement(RaceInfo *arace)
{
	next = 0;

	race = arace;

	istar = 0;
	iplanet = 0;
	imoon = 0;
}

RaceSettlement::~RaceSettlement()
{
}

void RaceSettlement::locate(int astar, int aplanet, int amoon)
{
	istar = astar;
	iplanet = aplanet;
	imoon = amoon;
}

void RaceSettlement::calculate()
{
}

void RaceSettlement::animate_starmap(Frame *f)
{
	if (!patrol.range)
		return;

	BITMAP *bmp = f->surface;

	// in case of the starmap, show the circles of influence

	int R;
	Vector2 P;

	P = corner(mapeverything.region[0]->sub[istar]->position * mapeverything.region[0]->scalepos);

	R = patrol.range * mapeverything.region[0]->scalepos * space_zoom;

	//void circlefill(BITMAP *bmp, int x, int y, int radius, int color);
	circlefill(bmp, P.x, P.y, R, race->color);
}






RaceColony::RaceColony(RaceInfo *arace)
:
RaceSettlement(arace)
{
	// this is a default colony of 10 K individuals (1 million).
	population = 10;

	population *= race->cinfo.start_population_multiplier;

	calculate();
}


void RaceColony::calculate()
{
	double dt = frame_time * 1E-3;

	// growth ?
	population *= exp(log(2.0) * dt / race->cinfo.doubling_period);

	patrol.range = 20 + sqrt(population / 100);

	if (patrol.range > 100)
		patrol.range = 100;

	patrol.numsystemfleets = population / 10;
	patrol.numhyperfleets = population / 1E3;
	patrol.numcapitalfleets = population / 1E6;
}









RaceInfo::RaceInfo(char *arace_id, int acolor)
{
	next = 0;

	firstcol = 0;
	lastcol = 0;

	id = new char [strlen(arace_id)+1];
	strcpy(id, arace_id);

	color = acolor;

	strcpy(cinfo.env_type, "gaia");
	cinfo.doubling_period = 10.0;	// 10 years to double the population
	cinfo.start_population_multiplier = 1.0;
}


RaceInfo::~RaceInfo()
{
	delete id;
}


void RaceInfo::calculate()
{
}



void RaceInfo::add(RaceColony *rc)
{
	if (!firstcol)
		firstcol = rc;

	if (lastcol)
		lastcol->next = rc;

	lastcol = rc;

}

void RaceInfo::init_colonies(char *ininame)
{
	set_config_file(ininame);
	
	// init colonies
	
	int i, N;
	N = get_config_int(0, "N", 0);
	
	for ( i = 0; i < N; ++i )
	{
		RaceColony *rc;
		rc = new RaceColony(this);
		add(rc);

		char colid[64];
		sprintf(colid, "colony%03i", i);

		int istar, iplan, imoon;
		istar = get_config_int(colid, "star", 0);
		iplan = get_config_int(colid, "planet", 0);
		imoon = get_config_int(colid, "moon", 0);
		rc->locate(istar, iplan, imoon);

		rc->population = get_config_float(colid, "population", 0);

		rc->calculate();
	}
}


void RaceInfo::animate_starmap(Frame *f)
{
	RaceSettlement *current;
	current = firstcol;

	while (current)
	{
		current->animate_starmap(f);
		current = current->next;
	}
}







RaceManager::RaceManager()
{
	first = 0;
	last = 0;
}

void RaceManager::add(RaceInfo *ri)
{
	if (!first)
		first = ri;
	
	if (last)
		last->next = ri;
	
	last = ri;
}



void RaceManager::readracelist()
{
	al_ffblk info;

	// find all the directory names - each directory indicates a different race,
	// and has the race name as well
	
	int err;
	err = al_findfirst("gamex/gamedata/races/*", &info, FA_DIREC );

	while (!err)
	{
		char *racename;
		racename = info.name;

		if (strcmp(racename, ".") != 0 && strcmp(racename, "..") != 0)
		{
			
			char fname[512];
			strcpy(fname, "gamex/gamedata/races/");
			strcat(fname, racename);
			strcat(fname, "/race.ini");
			set_config_file(fname);
			

			int r, g, b;
			r = get_config_int("color", "r", 254);
			g = get_config_int("color", "g", 1);
			b = get_config_int("color", "b", 254);
			
			RaceInfo *ri;
			ri = new RaceInfo(racename, makecol(r,g,b)); 
			add(ri);

			strncpy(ri->shipid, get_config_string("ship", "id", "none"), 16);
			
			ri->cinfo.doubling_period = get_config_float("colony", "doublingperiod", 10.0);
			ri->cinfo.start_population_multiplier = get_config_float("colony", "startpopmultiplier", 1.0);
			strcpy(ri->cinfo.env_type, get_config_string("colony", "envtype", "gaia"));
			if (strlen(ri->cinfo.env_type) > 64) { tw_error("invalid env type"); }

		
			strcpy(fname, "gamex/gamedata/races/");
			strcat(fname, racename);
			strcat(fname, "/colonies.ini");
			ri->init_colonies(fname);

			strcpy(fname, "gamex/gamedata/races/");
			strcat(fname, racename);
			strcat(fname, "/fleet.bmp");

			BITMAP *bmp;
			bmp = load_bitmap(fname, 0);

			SpaceSprite *fleetspr;
			fleetspr = new SpaceSprite(bmp);

			ri->fleetsprite = fleetspr;

			destroy_bitmap(bmp);
		
		}



		err = al_findnext(&info);
	}

}


void RaceManager::writeracelist()
{

	RaceInfo *current;
	current = first;

	while (current)
	{
		char fname[512];
		strcpy(fname, "gamex/gamedata/races/");
		strcat(fname, current->id);
		strcat(fname, "/race.ini");
		set_config_file(fname);
		
		int r, g, b;
		r = getr(current->color);
		g = getg(current->color);
		b = getb(current->color);

		set_config_int("color", "r", r);
		set_config_int("color", "g", g);
		set_config_int("color", "b", b);

		set_config_float("colony", "doublingperiod", current->cinfo.doubling_period);
		set_config_float("colony", "startpopmultiplier", current->cinfo.start_population_multiplier);
		set_config_string("colony", "envtype", current->cinfo.env_type);
		

		current = current->next;
	}

}



void RaceManager::animate_starmap(Frame *f)
{
	RaceInfo *current;
	current = first;

	while (current)
	{
		current->animate_starmap(f);
		current = current->next;
	}
}

