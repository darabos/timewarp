#ifndef __GAMEX_PLANET_SCAN__
#define __GAMEX_PLANET_SCAN__


#include "gameproject.h"
#include "gamedata.h"
#include "gamegeneral.h"




class GamePlanetscan : public GameBare
{

	WindowInfo wininfo;

	class ThePlaya : public LocalPlayerInfo
	{
	public:
		ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf);
	};

	ThePlaya *player;

	virtual void init();
	virtual void quit();
	//virtual bool handle_key(int k);

	virtual void calculate();
	virtual void animate(Frame *frame);

	BITMAP *map_bmp;
	SpaceSprite *playerspr, *planetspr;
};


#endif

