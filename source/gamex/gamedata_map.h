#ifndef __GAMEX_DATA_MAP__
#define __GAMEX_DATA_MAP__

#include "../melee/mframe.h"

#include <stdio.h>

#include "gamedata.h"





class MapSpacebody;



// the branches
class MapSpacebody
{
public:
	//SpacebodyInfo *info;

	//char type[32];
	int type;			// the char version should be in a table - why ... because indexed stuff is easier to work with!!
	IndexTypeList *typelist;

	Vector2 position;	// 0,0 = center
	char name[64];
	SpaceObject *o;
	double scalepos;

	int id;
	int Nsub;
	MapSpacebody **sub;	// at most 3 moons.

	virtual void init(FILE *f);
	virtual void save(FILE *f);
	virtual int add();
	virtual int rem(int k);

	bool check_id(int id2);

	virtual void newsub(FILE *f) {};
	virtual void init_type(char *s) {};

	int level;
};


class MapPlanet : public MapSpacebody
{
public:
	virtual void newsub(FILE *f);
	virtual void init_type(char *s);
};


class MapStar : public MapSpacebody
{
public:
	virtual void newsub(FILE *f);
	virtual void init_type(char *s);
};


// the root
class MapEverything : public MapSpacebody
{
public:
	//int Nregions;
	//MapSpacebody **region;

	void init(char *filename);
	void save(char *filename);
	void discard();
//	virtual void init(FILE *f, int level);
//	virtual void save(FILE *f, int level);

	int gen_id();

	virtual void newsub(FILE *f);
	virtual void init_type(char *s);
};

extern MapEverything mapeverything;



#endif


