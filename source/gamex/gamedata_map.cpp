
#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "melee.h"
REGISTER_FILE

#include "frame.h"
#include "melee/mview.h"


#include "gamedata.h"
#include "gamedata_map.h"
#include "gamegeneral.h"



/*
void MapStar::init_info()
{
	info = new InfoStar();
	info->read();
}
*/


void MapPlanet::newsub(FILE *f)
{
	fscanf(f, "%i\n\n", &Nsub);

	if (Nsub < 0 || Nsub > 20) {tw_error("error: invalid system");}

	sub = (MapSpacebody**) new MapPlanet* [Nsub];

	int i;
	for ( i = 0; i < Nsub; ++i )
	{
		sub[i] = new MapPlanet();
		sub[i]->init(f);
		sub[i]->level = level + 1;
	}
}


void MapStar::newsub(FILE *f)
{
	fscanf(f, "%i\n\n", &Nsub);

	if (Nsub < 0 || Nsub > 20) {tw_error("error: invalid system");}

	sub = (MapSpacebody**) new MapPlanet* [Nsub];

	int i;
	for ( i = 0; i < Nsub; ++i )
	{
		sub[i] = new MapPlanet();
		sub[i]->init(f);
		sub[i]->level = level + 1;
	}
}


void MapEverything::newsub(FILE *f)
{
	fscanf(f, "%i\n\n", &Nsub);

	if (Nsub < 0 || Nsub > 20) {tw_error("error: invalid system");}

	sub = (MapSpacebody**) new MapStar* [Nsub];

	int i;
	for ( i = 0; i < Nsub; ++i )
	{
		sub[i] = new MapStar();
		sub[i]->init(f);
		sub[i]->level = level + 1;
	}
}




void MapPlanet::init_type(char *s)
{
	typelist = planettypelist;
	type = typelist->get_index(s);
}


void MapStar::init_type(char *s)
{
	typelist = startypelist;
	type = typelist->get_index(s);
}


void MapEverything::init_type(char *s)
{
	typelist = 0;
	type = -1;
}






// the set of every region / quadrant in existence ...
void MapEverything::init(char *filename)
{
	level = 0;

	startypelist = new IndexTypeList("gamex/stars/star_*.ini");
	planettypelist = new IndexTypeList("gamex/solarview/planet_*.ini");
	surfacetypelist = new IndexTypeList("gamex/planetscan/surface_*.ini");
	starsurfacetypelist = new IndexTypeList("gamex/stars/surface_*.ini");
	
	FILE *f = fopen(filename, "rt");
	if (!f) { tw_error("failed to initialize map info");}

	newsub(f);

	fclose(f);

}


void MapEverything::discard()
{
	delete startypelist;
	delete planettypelist;
	delete surfacetypelist;
	delete starsurfacetypelist;
}



void MapEverything::save(char *filename)
{
	FILE *f = fopen(filename, "wt");
	if (!f)	{tw_error("failed to save map info");}

	fprintf(f, "%i\n\n", Nsub);

	int i;
	for ( i = 0; i < Nsub; ++i )
	{
		sub[i]->save(f);
	}

	fclose(f);
}


// generate a unique (random) id for a map item
int MapEverything::gen_id()
{
	int id;

	for (;;)
	{
		id = random();

		int i;
		for ( i = 0; i < Nsub; ++i )
		{
			if (sub[i]->check_id(id))
				break;
		}

		if (i == Nsub)	// id not found
			break;
	}

	return id;
}






void MapSpacebody::init(FILE *f)
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
	
	init_type(chartype);

	fscanf(f, "%lf %lf %X", &position.x, &position.y, &id);

	newsub(f);

	scalepos = 1;
}



void MapSpacebody::save(FILE *f)
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
		fprintf(f, "%s %s\n", base, planettypelist->type[this->type].type_string);


	fprintf(f, "%s %lf %lf %X\n", base, position.x, position.y, id);
	fprintf(f, "%s %i\n\n", base, Nsub);

	for ( i = 0; i < Nsub; ++i )
		sub[i]->save(f);

}


int MapSpacebody::add()
{

	int i;
	MapSpacebody **oldsub;

	oldsub = sub;
	sub = new MapSpacebody* [Nsub + 1];

	for ( i = 0; i < Nsub; ++i )
		sub[i] = oldsub[i];

	delete oldsub;

	sub[Nsub] = new MapSpacebody();
	sub[Nsub]->init(0);


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


bool MapSpacebody::check_id(int id2)
{
	if (id == id2)
		return true;

	// check if one of the sub-items have identical id
	int i;
	for ( i = 0; i < Nsub; ++i )
	{
		if (sub[i]->check_id(id2))
			return true;
	}

	// neither this nor any subitem has this id.
	return false;
}






