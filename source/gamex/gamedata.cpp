#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE


#include "gamedata.h"


IndexTypeList *startypelist;
IndexTypeList *planettypelist;
IndexTypeList *moontypelist;

PlayerInfo		playerinfo;
MapEverything	mapeverything;


void PlayerInfo::init(char *filename)
{
	configfilename = filename;

	set_config_file(filename);
	pos.x = get_config_float(0, "PosX", 0.0);
	pos.y = get_config_float(0, "PosY", 0.0);
	angle = get_config_float(0, "Angle", 0.0);

	istar = get_config_int(0, "Star", -1);
	iplanet = get_config_int(0, "Planet", -1);
	iplanetcode = get_config_int(0, "PlanetCode", -1);	// of planets and moons
}


void PlayerInfo::write()
{
	set_config_file(configfilename);
	set_config_float(0, "PosX", pos.x);
	set_config_float(0, "PosY", pos.y);
	set_config_float(0, "Angle", angle);

	set_config_int(0, "Star", istar);
	set_config_int(0, "Planet", iplanet);
	set_config_int(0, "PlanetCode", iplanetcode);
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
