
#ifndef __GAMEX_MELEE__
#define __GAMEX_MELEE__

#include "gameproject.h"
#include "gamedata.h"
#include "gamegeneral.h"

#include "melee/mgame.h"


class GameMelee : public GameBare
{
public:

	Ship	*player;
	XFleet	*enemyfleet;

	TeamCode	team_player, team_enemy;

	virtual void init();
	virtual void quit();
	//virtual bool handle_key(int k);

	virtual void init_menu();

	virtual void calculate();
	virtual void animate(Frame *frame);

	void set_xfleet(XFleet *f);

	GameMelee();
	virtual ~GameMelee();

	virtual void handle_edge(SpaceLocation *s);


	virtual Ship *create_ship(const char *id, bool human, Vector2 pos, double angle, int team);

	virtual void ship_died(Ship *who, SpaceLocation *source);
};



#endif
