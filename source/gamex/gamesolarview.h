#ifndef __GAMEX_SOLAR_VIEW__
#define __GAMEX_SOLAR_VIEW__


#include "gameproject.h"
#include "gamedata.h"
#include "gamegeneral.h"

#include "../twgui/twbuttontypes.h"
#include "../twgui/twpopup.h"
#include "../twgui/twmenuexamples.h"

#include "stuff/space_body.h"



// ellipticity
const double b_default = 4.0;

// derive ellips R from position
extern void ellipsparams(Vector2 relpos, double ellb, double &R, Vector2 &Poffs, int &col);


class MapEditor2 : public MapEditor
{
public:
	int isurfacetype;	// surfacetype of the selected object

	Vector2 mapcenter;

	virtual MapObj *create_mapobj(Vector2 pos);
	virtual void move();
	virtual void add();
	virtual void newselection();
	virtual void replace();

	virtual void colorizeobj(SolarBody *s);
};


class GameSolarview : public GameBare
{
public:
	MapSpacebody *starmap, *solarmap;
	TextEditBox *starname;
	char oldstarname[128];

	//IconTV
	TVarea	*tv2;
	IconTV *Tedit;
	// contents of Tedit
	Button *bnew, *breplace;
	//int istarselect;

	MapEditor2	*mapeditor;

	PopupList *rpopup, *dpopup;
	TextButton *crace, *cdialogname;
	Button *cnew, *crem, *cupdate;
	TextEditBox *cpop;
	char *list[32];

	ValueEdit *ve;
	
	MousePtr *ptr;

	int starnum;

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

	SpaceSprite *starspr, *playerspr, **planettypespr, *planetspr;
	BITMAP **surfacebmp;

	virtual void init_menu();

	void save_surface();
	void init_surface();
	void check_radius();
};


#endif
