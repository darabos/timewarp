#ifndef __GAMEX_DATA__
#define __GAMEX_DATA__

#include "melee/mframe.h"

#include <stdio.h>

//#include "gamedata_map.h"


const bool hascontent(char *t);

class RaceInfo;

// race-specific info
class ColonyRaceInfo
{
public:
	char	env_type[64];
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
	virtual void animate_map(Frame *f, int imap) {};
};


class RaceSettlement : public FGPresence
{
protected:

public:
	RaceInfo	*race;
	char		id[80];

	char		dialogname[128];	// name of the file; should be located in the races' subdirectory!!

	bool modified;

	RaceSettlement *next;
	int istar, iplanet, imoon;
	
	Patrol patrol;

	RaceSettlement(RaceInfo *arace);
	virtual ~RaceSettlement();

	void locate(int astar, int aplanet, int amoon);

	virtual void calculate();
	virtual void animate_map(Frame *f, int imap);

};


class RaceColony : public RaceSettlement
{
protected:

public:
	int		hidden;		// if true, the colony has complete-radiosilence and
	// won't show up; should be possible to activate it (using it's id?).

	RaceColony(RaceInfo *arace);
	double	population, initpop;

	virtual void calculate();

	virtual void set_info(double pop, char *adialogname);

	void changeowner(RaceInfo *newrace);
};



class RaceInfo : public Presence
{
public:
	bool modified;

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
	virtual void animate_map(Frame *f, int imap);

	ColonyRaceInfo cinfo;

	RaceColony *firstcol, *lastcol;

	void init_colonies(char *ininame);
	void write_colonies(char *ininame);

	void add(RaceColony *rc);
	void remlist(RaceColony *rc);
	void rem(RaceColony *rc);

	RaceColony *find_colony(int istar, int iplan, int imoon);
	char shipid[16];
};





class RaceManager
{
public:
	RaceInfo	*first, *last;

	RaceManager();

	void add(RaceInfo *ri);

	void readracelist();
	void writeracelist();

	virtual void animate_map(Frame *f, int imap);

	RaceInfo *get(char *useid);

	virtual RaceColony *findcolony(int istar, int iplan, int imoon);
};

extern RaceManager racelist;


class LocalPlayerInfo;

// general game data about the player
class PlayerInfo
{
	char *configfilename;
public:
	Vector2 pos, vel;
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
	void sync2(LocalPlayerInfo *p);

	double RU;	// your pile of money ...

	// cargo bay
	double mineral_weight[16];
	double bio_weight;
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
	char basename[512];

	int N, max;
	IndexType *type;
	
	IndexTypeList(char *fname);
	~IndexTypeList();

	int get_index(char *typestr, int defaultval = -1);

};

extern IndexTypeList *startypelist;
extern IndexTypeList *planettypelist;
//extern IndexTypeList *moontypelist;	// moons or small planets are not distinguishable
extern IndexTypeList *surfacetypelist;
extern IndexTypeList *starsurfacetypelist;
extern IndexTypeList *mineraltypelist;
extern IndexTypeList *structuretypelist;

#endif // __GAMEX_DATA__


