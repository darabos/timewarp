#ifndef __GAMEX_STARMAP__
#define __GAMEX_STARMAP__

//#include "../twgui/subgame.h"

#include "gameproject.h"
#include "gamedata.h"
#include "gamegeneral.h"


const int STAR_ID = 0x08fa51d3;

class Star : public SpaceObject
{
public:

	int starnum;

	Star(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite);

	virtual void animate(Frame *f);
	virtual void calculate();
};



class GameStarmap : public GameBare
{
public:

	TWindow *Tedit;
	// contents of Tedit
	Button *bdec, *binc, *bnew, *breplace, *bplot;
	int istarselect;

	double scalepos;
	Star *selectionstar, *lastselectionstar;

	Periodics *mouseper, *keyper;

	MousePtr *ptr;

	//WindowInfo wininfo;

	MapSpacebody *starmap;

	class ThePlaya : public LocalPlayerInfo
	{
	public:
		ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf);
	};

	ThePlaya *player;

	virtual void init();
	virtual void quit();
	//virtual bool handle_key(int k);
	virtual void init_menu();

	virtual void calculate();
	virtual void animate(Frame *frame);

	SpaceSprite *starspr[32], *playerspr;
	bool maphaschanged;

	void update_bplot();
	void mapeditor_stuff();
};





#endif


