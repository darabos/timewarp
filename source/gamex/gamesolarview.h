#ifndef __GAMEX_SOLAR_VIEW__
#define __GAMEX_SOLAR_VIEW__


#include "gameproject.h"
#include "gamedata.h"
#include "gamegeneral.h"

#include "../twgui/twbuttontypes.h"
#include "../twgui/twpopup.h"

#include "stuff/space_body.h"

#include "gamesolarview.h"


// ellipticity
const double b_default = 4.0;

// derive ellips R from position
extern void ellipsparams(Vector2 relpos, double ellb, double &R, Vector2 &Poffs, int &col);


class MapEditor2 : public MapEditor
{
public:
	Vector2 mapcenter;

	virtual MapObj *create_mapobj(Vector2 pos);
	virtual void calculate();
};


class GameSolarview : public GameBare
{
	IconTV *Tedit;
	// contents of Tedit
	Button *bnew, *breplace;
	//int istarselect;

	MapEditor2	*mapeditor;

	MousePtr *ptr;


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
