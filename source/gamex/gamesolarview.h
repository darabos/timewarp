#ifndef __GAMEX_SOLAR_VIEW__
#define __GAMEX_SOLAR_VIEW__


#include "gameproject.h"
#include "gamedata.h"
#include "gamegeneral.h"


class GameSolarview : public GameBare
{

	//WindowInfo wininfo;

	class ThePlaya : public LocalPlayerInfo
	{
	public:
		ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf);
	};

	ThePlaya *player;

	virtual void init();
	virtual void quit();
	virtual void refocus();
	//virtual bool handle_key(int k);

	virtual void calculate();
	virtual void animate(Frame *frame);

	virtual void checknewgame();

	SpaceSprite *starspr, *playerspr, *planetspr[32];

	virtual void init_menu();
};


#endif
