#ifndef __GAMEX_STARMAP__
#define __GAMEX_STARMAP__

//#include "../twgui/subgame.h"

#include "gameproject.h"
#include "gamedata.h"
#include "gamegeneral.h"

#include "gamesolarview.h"

#include "../twgui/twbuttontypes.h"
#include "../twgui/twpopup.h"

#include "stuff/space_body.h"


double blackbodyspectrum(double f, double T);
double spec_r(double T);
double spec_g(double T);
double spec_b(double T);


class MapEditor1 : public MapEditor2
{
public:
	virtual MapObj *create_mapobj(Vector2 pos);
	virtual void move();

	virtual void colorizeobj(SolarBody *s);

	virtual void define_stats();

	virtual void set_config();
	virtual void get_config();
};





/** \brief Loads and handles the starmap, which gives a overview of
the game-world (stars, territories).

*/
class GameStarmap : public GameBare
{
public:

//	IconTV *Tedit;
	// contents of Tedit
//	Button *bnew, *breplace;
	//int istarselect;

	MapEditor1	*mapeditor;

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

	SpaceSprite **startypespr, *starspr, *playerspr;
	BITMAP **surfacebmp;


	void load_startypes(SpaceSprite ***planettypespr);
	void load_surfacetypes(BITMAP ***surfacebmp);
};





#endif // __GAMEX_STARMAP__


