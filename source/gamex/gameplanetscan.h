#ifndef __GAMEX_PLANET_SCAN__
#define __GAMEX_PLANET_SCAN__


#include "gameproject.h"
#include "gamedata.h"
#include "gamegeneral.h"


#include "../other/planet3d.h"


class GamePlanetscan : public GameBare
{
	Area *surf_area;
//	SpaceObject *solarbody;

	Planet3D *rotatingplanet;


	class ThePlaya : public LocalPlayerInfo
	{
	public:
		ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf);
	};

	ThePlaya *player;

	virtual void init();
	virtual void init_menu();
	virtual void quit();
	//virtual bool handle_key(int k);

	virtual void calculate();
	virtual void animate(Frame *frame);

	BITMAP *map_bmp;
	SpaceSprite *playerspr;		//*planetspr;

};


#endif

