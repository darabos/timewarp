#ifndef __GAMEX_DATA__
#define __GAMEX_DATA__

#include "../melee/mframe.h"

#include <stdio.h>





class RaceInfo;

// race-specific info
class ColonyRaceInfo
{
public:
	char		env_type[64];
	double	doubling_period;	// double population in "years"
	double	start_population_multiplier;
};


// planet-specific info
class ColonyPlanetInfo
{
public:
	char		env_type[64];
};


// patrol stuff
class Patrol
{
public:
	Patrol();

	double	range;
	int		numhyperfleets;
	int		numsystemfleets;
	int		numcapitalfleets;
};


class FGPresence : public Presence
{
public:
	virtual void animate_starmap(Frame *f) {};
};


class RaceSettlement : public FGPresence
{
protected:
	RaceInfo	*race;

public:
	RaceSettlement *next;
	int istar, iplanet, imoon;
	
	Patrol patrol;

	RaceSettlement(RaceInfo *arace);
	virtual ~RaceSettlement();

	void locate(int astar, int aplanet, int amoon);

	virtual void calculate();
	virtual void animate_starmap(Frame *f);
};


class RaceColony : public RaceSettlement
{
protected:

public:
	RaceColony(RaceInfo *arace);
	double	population;

	virtual void calculate();
};



class RaceInfo : public Presence
{
public:
	RaceInfo	*next;

	RaceInfo(char *arace_id, int acolor);
	~RaceInfo();

//	Racemine *mines;
	//int Ncolonies;
	//int Nmines;

	//Vector2 spherecenter;
	//double spheresize;

	char	*id;
	int		color;

	SpaceSprite *fleetsprite;	// is used in hyperspace, and in solar/planet view
	// this class should clean it up...

	virtual void calculate();
	virtual void animate_starmap(Frame *f);

	ColonyRaceInfo cinfo;

	RaceColony *firstcol, *lastcol;
	void init_colonies(char *ininame);
	void add(RaceColony *rc);

	char	shipid[16];
};





class RaceManager
{
public:
	RaceInfo	*first, *last;

	RaceManager();

	void add(RaceInfo *ri);

	void readracelist();
	void writeracelist();

	virtual void animate_starmap(Frame *f);
};

extern RaceManager racelist;


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
	int		imoon;


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
	double scalepos;

	int Nsub;
	MapSpacebody **sub;	// at most 3 moons.

	virtual void init(FILE *f, int level);
	virtual void save(FILE *f, int level);
	virtual int add(int level);
	virtual int rem(int k);
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


