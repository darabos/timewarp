#ifndef __GPLEXPLR_H__
#define __GPLEXPLR_H__



#include "../melee/mgame.h"    //Game stuff
#include "../melee/mview.h"    //Views & text messages

#include <stdio.h>

SpaceObject *find_closest(SpaceLocation *s, int test_attr, double R);


class Plsurface;

// adapt draw if the object is at a frozen position ...
// for one, it doesn't need AA, second, the round-off errors in the approx. zoomed
// draw coordinates make the objects move a little wrt. the background, which I'd like
// to avoid.
class SpaceObjectFrozen : public SpaceObject
{
public:
	SpaceObjectFrozen(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite, char *txt, bool otangible, int ohealth);
	
	virtual void animate(Frame *frame);
	virtual int handle_damage (SpaceLocation *source, double normal, double direct = 0);

	void drawpos(Frame *frame, int index, BITMAP **bmp, int *x, int *y);
	char name[128];
	bool tangible;
	double	starthealth, health;
};

// a static thingy.
class Mountain : public SpaceObjectFrozen
{
public:
	Plsurface *plsurface;
	Mountain(Plsurface *surf, Vector2 opos, double oangle, SpaceSprite *osprite, char *txt, bool otangible, int ohealth);
	virtual void calculate();
};


// this one can move around.
class Creature : public SpaceObjectFrozen
{
public:
	Plsurface *plsurface;
	int		dindex;
	double	damage, dist, v, turntime, time, habitat;
	bool	toggledir;

	Creature(Plsurface *surf, Vector2 opos, double oangle, SpaceSprite *osprite, char *txt, bool otangible, int ohealth,
				int ohabitat, int ovel, int odamage);
	virtual void calculate();
	virtual void inflict_damage(SpaceObject *other);
	virtual int handle_damage (SpaceLocation *source, double normal, double direct = 0);
};


struct MeasurePeriod
{
	MeasurePeriod();

	void setperiod(double operiod);
	bool ready();
	void reset();

	double time, period;
};

class GunBody : public Mountain
{
public:
	SpaceSprite	*weaponsprite;
	double	weaponvel, weapondamage, turnvel;

	GunBody(Plsurface *surf, Vector2 opos, double oangle, SpaceSprite *obodysprite,
				 SpaceSprite *oweaponsprite, char *txt, bool otangible, int ohealth,
				   int oturnvel, int oweaponvel, int oweapondamage);
	virtual void calculate();
	virtual int handle_damage (SpaceLocation *source, double normal, double direct = 0);

	MeasurePeriod weaponperiod, turnperiod;
	double	weaponrange, viewrange;
};



// ------------------ NEW VIEW CLASS -----------------------
// this one is the most basic one possible
// it always has space_zoom = 1.


class View_Frozen : public View
{
	double f;
	double max, min;
	public:
	virtual void calculate(Game *game);
	virtual void init(View *old);
	virtual void prepare ( Frame *frame, int time );
	//virtual void set_window (BITMAP *dest, int x, int y, int w, int h);
	};



class gplexplr : public Game
{ //declaring our game type
	virtual void init(Log *_log) ; //happens when our game is first started
	virtual void calculate();
	bool handle_key(int k);
	
	public:
		double ZoomDefault;	// is camera.z, see struct CameraPosition {

	Ship *player;
	Plsurface	*plsurface;
};


// this manages the drawing of the (planet) background ; it also initializes the map ?
class Plsurface : public Presence
{
public:
	struct UnitProperty
	{
		SpaceObjectFrozen*	s;
	};

	typedef char	ObjNameType[64];
	struct ObjectProperty
	{
		SpaceSprite *spr;
		ObjNameType name;
		int habitat, vel, damage, health;
		bool tangible;
	};


	BITMAP	**terrain_tile;
	//SpaceSprite **object_spr;
	//ObjNameType	*object_name;
	ObjectProperty	*object_type;		// the available types of objects
	SpaceObjectFrozen	**obj;
	UnitProperty	*unit;
	int		*terrain_index, Nx, Ny, Ntiles, Nobjects, Nobjectsmax, Nobjecttypes,
			Nunits, Nunitsmax;

	int BitmapSize;
	Vector2 size();

	Plsurface(char *datafilename, char *subdatafilename);
	~Plsurface();
	void calculate();
	void animate(Frame *frame);
	int surf_color(int x, int y);

	int edittype;
	void edit_add();
	bool edit_test(int i, char *txt);
	void additem(Vector2 P, int i);
	void additem(Vector2 P, char *descr);
	int finditem(char *txt);

	void addobj(SpaceObjectFrozen *s);
	void addunit(SpaceObjectFrozen *u);

	void edit_save(char *fname);
	void edit_read(char *fname);
	char *editfilename;
	bool editmode;

	void delobj(int i);
	void delunit(int i);

	int countobjects(char *name);
	SpaceObject *findobject(char *name, int nskip);
};


#endif // __GPLEXPLR_H__


