
#ifndef __GAMEX_MELEE__
#define __GAMEX_MELEE__

#include "gameproject.h"
#include "gamedata.h"
#include "gamegeneral.h"

#include "../melee/mgame.h"



struct CacheGlobals
{
	Vector2		space_center, space_view_size, space_center_nowrap;
	double		space_zoom;

	void setglobals();
	void reset();
};


class GameMelee : public GameBare
{
public:
	WindowInfo wininfo;
	CacheGlobals	cg;

	Ship	*player;
	XFleet	*enemyfleet;

	TeamCode	team_player, team_aliens;

	virtual void init();
	virtual void quit();
	//virtual bool handle_key(int k);

	virtual void calculate();
	virtual void animate(Frame *frame);

	void set_xfleet(XFleet *f);

	GameMelee();
	virtual ~GameMelee();

	virtual void handle_edge(SpaceLocation *s);
};



#endif
