#ifndef __GAMEX_SPACE_BODY__
#define __GAMEX_SPACE_BODY__

#include "../gamedata_map.h"

// ellips of planet orbit
void drawellips(Frame *f, Vector2 center, double R, double b, int col);

// planet shading
void shadowpaintcircle(SpaceSprite *spr, double fi_s);

#include "../../twgui/twpopup.h"
#include "../../twgui/twbuttontypes.h"
#include "../gameproject.h"
#include "../gamedata.h"


class TVarea : public Area
{
public:
	BITMAP **blist;
	int N;
	int isel;
	double f;

	Button *bdec, *binc;
	void set(BITMAP **list, int aN);
	void set(SpaceSprite **list, int aN);

	BITMAP *show();
	SpaceSprite *makespr();

	void newlist(int aN);
	void initnewlist();
	void set_sel(int newsel);

	TVarea(TWindow *menu, char *identbranch, int asciicode = 0, bool akeepkey = 0);
	virtual ~TVarea();

	virtual void calculate();
};


class IconTV : public Popup
{
public:
	TVarea *tv;

	IconTV(char *ident, int xcenter, int ycenter, BITMAP *outputscreen);
	virtual ~IconTV();

	virtual void calculate();
};




const int MAPOBJ_ID = 0x08fa51d3;

class MapObj : public SpaceObject
{
public:

	int starnum;

	MapObj(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite);

	virtual void animate(Frame *f);
	virtual void calculate();

	virtual void rem_sprite();
};




class MapEditor
{
public:

	GameBare *g;
	MousePtr *ptr;

	MapSpacebody	*objmap;

	IconTV	*Tedit;
	Button	*breplace, *bnew;

	double	scalepos;

	bool	moveselection;

	MapObj *selection;

	bool	maphaschanged;

	MapEditor();
	virtual ~MapEditor();

	virtual void calculate();
	virtual void newselection();
	virtual void replace();
	virtual void add();
	virtual void move();

	void set_game(GameBare *agame, MousePtr *aptr);
	void set_interface( IconTV *aTedit, Button *abreplace, Button *abnew );
	void set_mapinfo( MapSpacebody *aobjmap, double ascalepos);

	virtual MapObj *create_mapobj(Vector2 pos);
};





//const int ID_SOLAR_BODY = 0x098a398f5;

class SolarBody : public MapObj
{
public:
	Vector2 stayhere;
	double sunangle;
	SolarBody(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite, Vector2 sunpos,
					int bodynum,
					Vector2 Ec, double ER, double Eb, int Ecol);
	virtual ~SolarBody();
	virtual void animate(Frame *f);
	virtual void calculate();

//	int solar_body_num;

	SpaceSprite *origsprite;
	Vector2	sunpos;

	Vector2 ellipscenter;
	double ellipsR, ellipsb;
	int ellipscol;

	virtual void set_sprite ( SpaceSprite *sprite );
	virtual void rem_sprite();
	void drawshadow();
};





#endif
