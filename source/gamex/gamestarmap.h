#ifndef __GAMEX_STARMAP__
#define __GAMEX_STARMAP__

//#include "../twgui/subgame.h"

#include "gameproject.h"
#include "gamedata.h"
#include "gamegeneral.h"

#include "../twgui/twbuttontypes.h"
#include "../twgui/twpopup.h"

#include "stuff/space_body.h"


double blackbodyspectrum(double f, double T);
double spec_r(double T);
double spec_g(double T);
double spec_b(double T);





class GameStarmap : public GameBare
{
public:

	IconTV *Tedit;
	// contents of Tedit
	Button *bnew, *breplace;
	//int istarselect;

	MapEditor	*mapeditor;

	double scalepos;
	//MapObj *selectionstar, *lastselectionstar;

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

	SpaceSprite **startypespr, **starspr, *playerspr;
	//bool maphaschanged;

//	void update_bplot();
	//void mapeditor_stuff();
};





#endif


