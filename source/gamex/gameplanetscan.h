#ifndef __GAMEX_PLANET_SCAN__
#define __GAMEX_PLANET_SCAN__


#include "gameproject.h"
#include "gamedata.h"
#include "gamegeneral.h"


#include "../other/planet3d.h"


class Mineral;
class Lifeform;

/** \brief Loads and handles the planetscan subgametype; this happens when you
orbit a planet. There are 2 modes, (1) the actual scan, where you get an overview
of the planet surface, and (2) the part where you control a planet lander to
explore the surface.

*/
class GamePlanetscan : public GameBare
{
public:
	Area *surf_area;
//	SpaceObject *solarbody;

	Popup *Pran;
	Button *branmin, *branlif;

	MapSpacebody *starmap, *solarmap, *planetmap, *moonmap, *body;

	Planet3D *rotatingplanet;


	class ThePlaya : public LocalPlayerInfo
	{
	public:
		double mineral_weight[16], bio_weight, tot_min_weight;
		double max_weight;

		ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf);
		void handle_mineral(Mineral *m);
		void handle_life(Lifeform *l);

		int activate_weapon();

		SpaceSprite *weaponsprite;
	};

	ThePlaya *player;

	virtual void init();
	virtual void init_menu();
	virtual void quit();
	//virtual bool handle_key(int k);

	virtual void calculate();
	virtual void animate(Frame *frame);

	virtual void handle_edge(SpaceLocation *s);

	BITMAP *map_bmp;
	SpaceSprite *playerspr;		//*planetspr;
	//double scalesurface;

	int		nmin, nmax;
	double	avsize, frac[32];
	Mineral *mine[64];

	void handle_ranmin();

	SpaceSprite *spr[10];

	void handle_env();	// firestorm, quake, lightning

	TeamCode team_player, team_enemy;
};


#endif // __GAMEX_PLANET_SCAN__

