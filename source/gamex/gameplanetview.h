#ifndef __GAMEX_PLANET_VIEW__
#define __GAMEX_PLANET_VIEW__


#include "gameproject.h"
#include "gamedata.h"
#include "gamegeneral.h"

#include "../twgui/twwindow.h"
#include "../twgui/twbuttontypes.h"


const int ID_FLEETICON = 0x08235497;



class Surface3D
{
public:
	BITMAP *image32bit, *dummy;
	int mapW, mapH;

	int image_size, visual_size;
	int PlanetUsespec;
	double theta, fi, rad;

	struct base_map_type {double lat, lon, diff, spec;} *base_map;
	
	unsigned int	*base_map_linear;		// mapping of coordinates
	unsigned int	*base_shade_linear, *base_spec_linear;		// shades ?
	unsigned char	*color_map_linear, *spec_map_linear;
	
	unsigned int	*base_map_and_shade_resorted;
	// linear means in this case, a linear array

	int jmin[1000], jmax[1000];


	Surface3D();
	~Surface3D();

	void reset(int planet_diameter, BITMAP *color_map,
					  BITMAP *spec_map, bool invcolor,
					  double sun_r, double sun_g, double sun_b);

	void plot();
	void plot(BITMAP *trg);
	void clear();
};


class MapEditor3 : public MapEditor2
{
public:
	virtual void colorizeobj(SolarBody *s);
};


class GamePlanetview : public GameBare
{
public:
//	IconTV *Tedit;
	// contents of Tedit
//	Button *bnew, *breplace;
	TextEditBox *starname;
	char oldstarname[128];
	//int istarselect;
	MapSpacebody *starmap, *solarmap, *planetmap;

	MapEditor3	*mapeditor;

	MousePtr *ptr;


	SpaceSprite **planettypespr;
	BITMAP **surfacebmp;

	virtual ~GamePlanetview();

	class ThePlaya : public LocalPlayerInfo
	{
	public:
		ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf);
	};

	ThePlaya *player;

//	FleetIcon *fleeticon;

	TeamCode	team_player, team_aliens;

	virtual void init();
	virtual void quit();
	virtual void refocus();
	//virtual bool handle_key(int k);

	virtual void calculate();
	virtual void animate();
	virtual void animate(Frame *frame);

	virtual void checknewgame();

	SpaceSprite *planetspr, *playerspr;

	virtual void init_menu();

};





#endif // __GAMEX_PLANET_VIEW__

