#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE

#include "../frame.h"
#include "../melee/mview.h"


#include "gamedata.h"
#include "gamedata_map.h"
#include "gamegeneral.h"


IndexTypeList *startypelist;
IndexTypeList *planettypelist;
IndexTypeList *surfacetypelist;
IndexTypeList *starsurfacetypelist;
IndexTypeList *mineraltypelist;
IndexTypeList *structuretypelist;

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

	RU = get_config_float(0, "RU", 0);
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

	set_config_float(0, "RU", RU);
}


void PlayerInfo::sync(LocalPlayerInfo *p)
{
//	pos = p->pos;
//	angle = p->angle;

	// this is now turned around, cause "old" settings are forgotten
	// when you re-appear "there"
	p->pos = pos;
	p->angle = angle;
	p->vel = vel;
}

void PlayerInfo::sync2(LocalPlayerInfo *p)
{
	pos = p->pos;
	angle = p->angle;
	vel = p->vel;
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











IndexTypeList::IndexTypeList(char *scanname)
{
	char **templist;

	strcpy(basename, scanname);
	char *tmp = strrchr(basename, '*');	// remove the *.ini from the scanname.
	if (tmp)
		tmp[0] = 0;
	else
	{ tw_error("invalid scanname"); }

	createfilelist(&templist, &N, scanname, 1);

	type = new IndexType [N];

	int i;
	for ( i = 0; i < N; ++i )
	{
		type[i].type_string = templist[i];
	}

	delete templist;

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



int IndexTypeList::get_index(char *typestr, int defaultval)
{
	int i;
	for ( i = 0; i < N; ++i )
	{
		if (strcmp(type[i].type_string, typestr) == 0)
			break;
	}

	if (i == N)		// no match
		return defaultval;
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

	modified = false;

	dialogname[0] = 0;

	strcpy(id, "none");
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

void RaceSettlement::animate_map(Frame *f, int imap)
{
	BITMAP *bmp = f->surface;

	MapSpacebody *starmap = 0, *solarmap = 0, *planetmap = 0;

	starmap = mapeverything.sub[0];

	if (istar >= 0)
		solarmap = starmap->sub[istar];

	if (iplanet >= 0)
		planetmap = solarmap->sub[iplanet];

	switch (imap)
	{
	case 1:
		{
			// a star map
			if (!patrol.range || !starmap)
				return;
			
			// in case of the starmap, show the circles of influence
			
			int R;
			Vector2 P;
			
			P = corner(starmap->sub[istar]->position * starmap->scalepos);
			
			R = iround(patrol.range * starmap->scalepos * space_zoom);
			
			//void circlefill(BITMAP *bmp, int x, int y, int radius, int color);
			circlefill(bmp, iround(P.x), iround(P.y), R, race->color);
			break;
		}

	case 2:
		{
			// a solar map
			if (!solarmap || (iplanet >= solarmap->Nsub) || (istar != playerinfo.istar) )
				return;

			int R;
			Vector2 P;
			
			P = corner(solarmap->sub[iplanet]->position * solarmap->scalepos);
			
			R = iround(80 * space_zoom);
			
			//void circlefill(BITMAP *bmp, int x, int y, int radius, int color);
			circle(bmp, iround(P.x), iround(P.y), R, race->color);

			break;
		}

		// I suppose we don't need to bother about the planet-view map: that's a bit
		// useless since the location is pretty obvious or not ?

	}
}






RaceColony::RaceColony(RaceInfo *arace)
:
RaceSettlement(arace)
{
	// this is a default colony of 10 K individuals.
	initpop = 10;

	population = initpop * race->cinfo.start_population_multiplier;

	hidden = false;

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

	patrol.numsystemfleets  = iround(population / 10);
	patrol.numhyperfleets   = iround(population / 1E3);
	patrol.numcapitalfleets = iround(population / 1E6);
}


// note that "pop" is the "default population", which is then adjusted ...
void RaceColony::set_info(double pop, char *adialogname)
{
	modified = true;

	initpop = pop;
	population = initpop * race->cinfo.start_population_multiplier;

	strcpy(dialogname, adialogname);
}


void RaceColony::changeowner(RaceInfo *newrace)
{
	// if it's not a new race
	if (newrace == race)
		return;

	// it's a new race: the old race loses it in the list, the new one retrieves it
	race->remlist(this);
	newrace->add(this);

	race = newrace;

	modified = true;
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
	cinfo.doubling_period = 25.0;	// 25 years to double the population
	cinfo.start_population_multiplier = 1.0;

	modified = false;
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

	rc->modified = true;
}

void RaceInfo::remlist(RaceColony *rc)
{
	RaceSettlement *current;

	if (firstcol == rc)
		firstcol = (RaceColony*) rc->next;
	else
	{

		current = firstcol;
		while (current && current->next != rc)
			current = current->next;
		
		if (!current)
			return;

		current->next = rc->next;
	}

	current = firstcol;

	while (current && current->next)
		current = current->next;

	lastcol = (RaceColony*)current;
}

void RaceInfo::rem(RaceColony *rc)
{
	remlist(rc);

	delete rc;
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

		rc->initpop = get_config_float(colid, "initpop", 0);
		rc->population =  get_config_float(colid, "population", 0);

		strcpy(rc->dialogname, get_config_string(colid, "dialog", ""));

		rc->hidden = get_config_int(colid, "hidden", 0);
		strcpy(rc->id, get_config_string(colid, "id", ""));

		rc->calculate();
	}
}


// saves to disk, but only if needed ?

void RaceInfo::write_colonies(char *ininame)
{
	set_config_file(ininame);

	bool mod = false;
	int N = 0;

	RaceColony *rc;
	rc = firstcol;

	while (rc)
	{
		++N;
		mod |= rc->modified;
		rc = (RaceColony*)rc->next;
	}

	if (!mod)
		return;		// no need to save if there's no change

	// save all the colonies

	set_config_int(0, "N", N);
	
	int i = 0;

	rc = firstcol;

	while (rc)
	{

		char colid[64];
		sprintf(colid, "colony%03i", i);

		set_config_int(colid, "star", rc->istar);
		set_config_int(colid, "planet", rc->iplanet);
		set_config_int(colid, "moon", rc->imoon);

		set_config_float(colid, "initpop", rc->initpop);
		set_config_float(colid, "population", rc->population);

		set_config_string(colid, "dialog", rc->dialogname);

		set_config_int(colid, "hidden", rc->hidden);
		set_config_string(colid, "id", rc->id );

		rc = (RaceColony*)rc->next;
		++i;
	}
}



void RaceInfo::animate_map(Frame *f, int imap)
{
	RaceSettlement *current;
	current = firstcol;

	while (current)
	{
		current->animate_map(f, imap);
		current = current->next;
	}
}






// returns true, if this race already has some kind of settlement at this location
RaceColony *RaceInfo::find_colony(int istar, int iplan, int imoon)
{
	RaceColony *current;
	current = firstcol;

	while (current)
	{
		if (current->istar == istar && current->iplanet == iplan && current->imoon == imoon)
			break;
		
		current = (RaceColony*)current->next;
	}

	return current;
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

	al_findclose(&info);

}


void RaceManager::writeracelist()
{

	RaceInfo *current;
	current = first;

	while (current)
	{
		char fname[512];

		// this only writes if the basic race info is changed
		if (current->modified)
		{
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
		}

		

		strcpy(fname, "gamex/gamedata/races/");
		strcat(fname, current->id);
		strcat(fname, "/colonies.ini");
		// this only writes if one of the colonies was modified.
		current->write_colonies(fname);

		current = current->next;
	}

}



void RaceManager::animate_map(Frame *f, int imap)
{
	RaceInfo *current;
	current = first;

	while (current)
	{
		current->animate_map(f, imap);
		current = current->next;
	}
}


// return the Raceinfo with the specified id ...
RaceInfo *RaceManager::get(char *useid)
{
	RaceInfo *current;
	current = first;

	while (current)
	{
		if (strcmp(current->id, useid) == 0)
			return current;
		current = current->next;
	}

	return current;
}



RaceColony *RaceManager::findcolony(int istar, int iplan, int imoon)
{
	RaceInfo *ri;
	RaceColony *rc;

	rc = 0;
	ri = first;

	while (ri)
	{
		rc = ri->find_colony(istar, iplan, imoon);
		if (rc)	break;

		ri = ri->next;
	}

	return rc;
}
