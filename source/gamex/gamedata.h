#ifndef __GAMEX_DATA__
#define __GAMEX_DATA__

#include "../melee/mframe.h"




class RaceInfo
{
	void AI();

	int Ncolonies;
	int Nmines;

	Vector2 spherecenter;
	double spheresize;
};


class LocalPlayerInfo;

// general game data about the player
class PlayerInfo
{
	char *configfilename;
public:
	Vector2 pos;
	double angle;

	// <0 means you're in hyperspace
	int		istar;

	// <0 means you're in solarsys view
	int		iplanet;

	// <0 means you're in planetsys view
	int		iplanetcode;

	// >= 0 mean you're in explore view.
	// each planet should have a unique code.

	void init(char *filename);
	void write();
	void sync(LocalPlayerInfo *p);
};

extern PlayerInfo playerinfo;




// the stuff that's needed for individual games
class LocalPlayerInfo : public SpaceObject
{
public:
	SpaceObject *collisionwith;	// with what object it collided.
	PlayerInfo *playerinfo;
	LocalPlayerInfo(SpaceSprite *osprite, PlayerInfo *playinf);
	virtual void calculate();
	virtual void inflict_damage(SpaceObject *other);
};




// the branches
class MapSpacebody
{
public:
	//char type[32];
	int type;			// the char version should be in a table - why ... because indexed stuff is easier to work with!!
	Vector2 position;	// 0,0 = center
	char name[64];
	SpaceObject *o;

	int Nsub;
	MapSpacebody **sub;	// at most 3 moons.

	virtual void init(FILE *f, int level);
	virtual void save(FILE *f, int level);
	virtual int add(int level);
};



// the root
class MapEverything
{
public:
	int Nregions;
	MapSpacebody **region;

	void init(char *filename);
	void save(char *filename);
};

extern MapEverything mapeverything;




// the following is needed, because
// in real life, it's easy to work/recognize strings,
// while in programming, you prefer indexed stuff so that you can find something easily
// (bonus : you get an extra check for unrecognized types).


struct IndexType
{
	//int index;
	char *type_string;
};

struct IndexTypeList
{
	int N, max;
	IndexType *type;
	
	IndexTypeList(int max, char *fname);
	~IndexTypeList();

	int get_index(char *typestr);

};

extern IndexTypeList *startypelist;
extern IndexTypeList *planettypelist;
extern IndexTypeList *moontypelist;

const int maxstartypes = 128;
const int maxplanettypes = 128;
const int maxmoontypes = 128;
// also includes the moon types



#endif


