#ifdef NOTDEFINED

#ifndef __GAME_PLANET_VIEW__
#define __GAME_PLANET_VIEW__

//#include "twgui/subgame.h"





// an extended class of objects, with some extra usefulness.

class SpaceObject_ext : public SpaceObject
{
public:
	SpaceObject *obj_collision;	// with what object it collided.

	SpaceObject_ext(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite);
	virtual void inflict_damage(SpaceObject *other);
};


struct solarsystem_str
{

	struct moon_str
	{
		int type;
		Vector2 position;	// 0,0 = center
		char name[64];
		SpaceObject *o;
	};

	struct planet_str
	{
		int type;
		Vector2 position;	// 0,0 = center
		char name[64];
		SpaceObject *o;

		int Nmoons;
		moon_str *moon;	// at most 3 moons.
	};

	struct sun_str
	{
		int type;
		Vector2 position;	// 0,0 = center
		char name[64];
		SpaceObject *o;

		int Nplanets;
		planet_str *planet;
	};

	int Nsuns;
	sun_str *sun;


};





class GameStart;

// an extended game class, which can be embedded in a sequence
// of initiated games ...

class SubGame : public Game
{
public:

	SubGame		*prev, *next;
	GameStart	*gamestart;		// the first game (contains the melee data ?)

	double		refscaletime;		// scale factor for time in the "prev" game

	SubGame();
	~SubGame();

	virtual void play_iteration(unsigned int time);
	//virtual void calculate();
	//virtual void animate(Frame *frame);
	//virtual void animate();
};



const int MaxSubGames = 16;	// some arbitrary number, but it's only 16 pointers so it's not a big issue imo


//class GameStart : public SubGame
class GameStart : public SubGame
{
public:

	SubGame	*subgame[MaxSubGames];		// not too many..
	int Nsubgames;

	//virtual void preinit();
	//virtual void init(Log *log);

	virtual void calculate();
	virtual void animate(Frame *frame);
	virtual void animate();

	void useplanet(solarsystem_str::planet_str *planet);

	void addsubgame(SubGame *asubgame);
	void removesubgame(int k);

	virtual void play();
	//virtual void play_iteration(unsigned int time);	// same as in the subgame?
};



class GameSolarView : public SubGame
{
	SpaceObject_ext		*flagship;

public:

	//virtual void preinit();
	virtual void init(Log *log);

	virtual void calculate();
	virtual void animate(Frame *frame);

	void usesolar(solarsystem_str *solarsystem);
};


class GamePlanetView : public SubGame
{
public:

	//virtual void preinit();
	virtual void init(Log *log);

	virtual void calculate();
	virtual void animate(Frame *frame);
	virtual void animate();

	void useplanet(solarsystem_str::planet_str *planet);
};



#endif // __GAME_PLANET_VIEW__


#endif // NOTDEFINED
