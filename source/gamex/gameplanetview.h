#ifndef __GAMEX_PLANET_VIEW__
#define __GAMEX_PLANET_VIEW__


#include "gameproject.h"
#include "gamedata.h"
#include "gamegeneral.h"

#include "../twgui/twwindow.h"
#include "../twgui/twbuttontypes.h"


const int ID_FLEETICON = 0x08235497;

class FleetIcon : public SpaceObject
{
public:
	XFleet	fleet;
	FleetIcon(Vector2 opos, SpaceSprite *osprite, char *oidname);
};



class GamePlanetview : public GameBare
{
	IconTV *Tedit;
	// contents of Tedit
	Button *bnew, *breplace;
	TextEditBox *starname;
	char oldstarname[128];
	//int istarselect;
	MapSpacebody *starmap, *solarmap, *planetmap;

	MapEditor2	*mapeditor;

	MousePtr *ptr;


	//WindowInfo wininfo;

	virtual ~GamePlanetview();

	class ThePlaya : public LocalPlayerInfo
	{
	public:
		ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf);
	};

	ThePlaya *player;

	FleetIcon *fleeticon;

	TeamCode	team_player, team_aliens;

	virtual void init();
	virtual void quit();
	virtual void refocus();
	//virtual bool handle_key(int k);

	virtual void calculate();
	virtual void animate();
	virtual void animate(Frame *frame);

	virtual void checknewgame();

	SpaceSprite **planetspr, *playerspr, *fleetspr;

	virtual void init_menu();

};





#endif

