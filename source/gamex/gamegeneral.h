#ifndef __GAMEX_GENERAL__
#define __GAMEX_GENERAL__

#include "../melee.h"

Vector2 uncorner ( Vector2 pos );
void replace_color(BITMAP *bmp, int col1, int col2);


class MousePtr : public SpaceObject
{
public:
	SpaceObject		*selection;		// this means, you can only select stuff which has a physical presence (sprite)
	double			mouserange;

	MousePtr(SpaceSprite *osprite);
	virtual void animate(Frame *f);
	virtual void calculate();
};


class WindowInfo
{
public:
	Vector2	maparea,	// the part of the map that's mapped onto the visual screen; can be used to "zoom" if necessary
			mapcenter;
	Vector2 framesize;
	double	refscreenw;	// extra zoom factor to correct for different resolutions (?)
	double	zoomlevel;	// zoom level required by the game.
	double	minzoom, maxzoom;	// zoom limits.

	void init(Vector2 maparea, double refscreenw, Frame *frame);
	void center(Vector2 pos);
	void centermove(double dx, double dy);

	void zoom(double z);
	void set_zoom(double z);
	void zoomlimit(double W);
	void scaletowidth(double W);	// zoom out to this width.
	void testzoom();
	void edgecorrect();
};

/*
// this one has a different animate subroutine.
// this is better suited for fixed-zoom modes, I think.
class SpaceSpriteWin : public SpaceSprite
{
	WindowInfo *wininfo;
public:
	SpaceSpriteWin(BITMAP **bmplist, int sprite_count, int rotations, int _attributes, WindowInfo *owininfo);
	virtual void animate(Vector2 pos, int index, Frame *frame, double scale);
};
*/


//SpaceSprite *create_sprite(char *bmpfilename, int _attributes);
SpaceSprite *create_sprite(char *bmpfilename, int _attributes, int rotations = 1, int bpp=32, double scale = 1);





struct XFleetShipinfo
{
	// then name of the race
	char name[32];

	void setname(char *c);
};

// at most 32 ships in a fleet ...
const int maxfleetsize = 32;

class XFleet
{
public:
	int Nships;
	XFleetShipinfo info[maxfleetsize];
	XFleet();
	void add(char *race, int N=1);
};




// other useful routines ...

Vector2  bitmap_size(BITMAP *b);
Vector2i bitmap_sizei(BITMAP *b);




class Periodics
{
	double t;
public:
	Periodics(double aperiod);
	double period;
	bool update();
};


// truncate the value of x, so that it does not exceed xmin or xmax
void staywithin(double xmin, double *x, double xmax);

void staywithin(int xmin, int *x, int xmax);

void staywithin(Vector2 pmin, Vector2 *p, Vector2 pmax);


// locatee the ship on the edge of a rectangle, based on the orientation
// towards the center of the map
void locate_onedge_aligned_to_center(SpaceLocation *s, Vector2 center, Vector2 wh);


#endif


