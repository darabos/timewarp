#ifndef _MELEE_H
#include "../melee.h"
#endif
#ifndef _MFRAME_H
#include "mframe.h"
#endif

#ifndef _MCBODIES_H
#define _MCBODIES_H


class Asteroid : public SpaceObject {
	protected:
	int step;
	int speed;
	SpaceSprite *explosion;

	public:
	Asteroid();

	virtual void calculate();
	virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);
	virtual void death();
};

class Planet : public SpaceObject {
	public:
	double gravity_force;
	double gravity_mindist;
	double gravity_range;
	double gravity_power;
	double gravity_whip;
	double gravity_whip2;
	Planet(Vector2 location, SpaceSprite *sprite, int index);

	virtual void inflict_damage(SpaceObject *other);
	virtual void calculate();
};

struct StarLayer {
	float sx, sy, wx, wy;
	float minz, maxz;
	float scale;
	float twinkle_amp, twinkle_freq;
	int seed;
	short n;
	char aa_mode;
	char type;
	char first_pic, num_pics;
};

class Stars2 : public Presence {
	public:
	static SpaceSprite *starpics;
	Stars2();
	~Stars2();
	int num_layers;
	StarLayer *layer;
	void load_next ( const char *sect, const StarLayer *old );
	virtual void animate ( Frame *frame );
};

class Stars : public Presence {
	virtual void _event( Event *e);
	public:
	Stars();
	~Stars();
	SpaceSprite **pic;
	double width;
	double height;
	int num_pics;
	int num_layers;
	int num_stars;   //300?
	int aa_mode;     //0 to 5, 1, 2, or 5 recommended
	int depth;       //0 to 255

	Uint32 seed;
	void animate(Frame *space);
	void select_view( View **view);
	View **v;
};

//helpers for drawing your own starfields
void _draw_starfield_raw (
	//surface to draw starfield on
	Frame *frame,
	//star sprite
	SpaceSprite *sprite, 
	//index into star sprite
	int index, int sprites,
	//number of stars
	int n, 
	//center of screen for drawing purposes
	int cx, int cy, 
	//scrolly amount
	double x, double y, 
	//size of starfield (usually the same as wrap point, sometimes smaller)
	double w, double h, 
	//wrap point
	double mx, double my, 
	//size of stars
	double zoom,
	//anti-aliasing mode to use
	int aa_mode
);

//helpers for drawing your own starfields
void _draw_starfield_cached (
	//surface to draw starfield on
	Frame *frame,
	//star sprite
	SpaceSprite *sprite, 
	//index into star sprite
	int index,
	//number of stars
	int n, 
	//center of screen for drawing purposes
	int cx, int cy, 
	//scrolly amount
	double x, double y, 
	//size of starfield (usually the same as wrap point, sometimes smaller)
	double w, double h, 
	//wrap point
	double mx, double my, 
	//size of stars
	double zoom,
	//anti-aliasing mode to use
	int aa_mode
);

#endif