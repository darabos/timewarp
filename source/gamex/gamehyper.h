#ifndef __GAMEX_HYPER__
#define __GAMEX_HYPER__

#include "gameproject.h"
#include "gamedata.h"
#include "gamegeneral.h"



class HyperspaceExplosions
{
	int		Nlevels;
	SpaceSprite	*spr[12];	// max 12 levels?
	double		level[12];	// position scaling (closer to the eye)...
	double		W[12];
	double		period[12], wait[12], density[12];

	WindowInfo	*wininfo;
	int		winNx[12], winNy[12], mapNx[12], mapNy[12];
	int		ixpos[12], iypos[12];

public:
	HyperspaceExplosions(WindowInfo *owininfo);
	void addlevel(SpaceSprite *ospr, double olevel,
							double period, double wait, double density);
	void init();
	void init_cell(int ix, int iy, int ilevel);
	void expand();
};




class GameHyperspace : public GameBare
{
public:

	AreaTablet	*bradar;

	MousePtr *ptr;
//	MapEverything *map;
	MapSpacebody *starmap;
	
	int Nlayers;
	SpaceSprite *spr[16];	//*sprA, *sprB, *sprC;
	HyperspaceExplosions	*hyperexpl;

	class ThePlaya : public LocalPlayerInfo
	{
	public:
		ThePlaya(SpaceSprite *osprite, PlayerInfo *playinf);
	};

	ThePlaya *player;

	virtual void init();
	virtual void quit();
	virtual void refocus();

	virtual bool handle_key(int k);

	virtual void calculate();
	virtual void animate(Frame *frame);

	double	escapetime;
	int		enterstar(SpaceObject *p);

	virtual void checknewgame();


	// performance check
	Histograph *tic_history;
	Histograph *render_history;


	double scalepos;


	SpaceSprite *star_hyperspr[32], *star_radarspr[32],
				*radarplayerspr;

	//BITMAP *submap;
	void plot_submap(BITMAP *submap);

	virtual void calc_kinetic(SpaceLocation *s);
	virtual void handle_edge(SpaceLocation *s);

	virtual void init_menu();
};




#endif


