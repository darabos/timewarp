/* $Id$ */ 
#include "../../melee.h"
REGISTER_FILE

#include "../../melee/mframe.h"
#include "../../frame.h"
#include "../../melee/mview.h"


#include "backgr_stars.h"
#include "../general/sprites.h"



StarBackgr::StarBackgr()
:
SpaceLocation(0, 0, 0)
{
	//layer = LAYER_HOTSPOTS;
	set_depth(DEPTH_STARS);
}


StarBackgr::~StarBackgr()
{
	int i;

	for ( i = 0; i < Nstars; ++i )
		delete starsprite[i];

	delete[] starsprite;
	delete[] pos;
	delete[] refdist;
}



void StarBackgr::init(int N, Frame *frame)
{

	BITMAP *bmp[3];
	int Ntypes = 3;


	load_bitmaps(bmp, "gamex/backgr", "backgr_star", Ntypes);



	Nstars = N;
	starsprite = new SpaceSprite* [Nstars];
	pos = new Vector2 [Nstars];
	refdist = new double [Nstars];


	Vector2 framesize;
	if (frame->surface)
		framesize = Vector2(frame->surface->w, frame->surface->h);
	else
		framesize = Vector2(frame->window->surface->w, frame->window->surface->h);


	// minimum of the zoom - the whole map is visible at ref map distance. Then:
	double zmin;
	zmin = framesize.x / map_size.x;
	// this corresponds to a "distance" of:
	double d;
	d = (1/zmin) - 1;

	int i;

	for ( i = 0; i < Nstars; ++i )
	{
		// create a sprite ... different choices, each with some likelihood
		int k;
		double likely = tw_random(1.0);
		if (likely < 0.8)
			k = 0;
		else if (likely < 0.95)
			k = 1;
		else
			k = 2;

		starsprite[i] = new SpaceSprite(bmp[k]);

		// teint the sprite (multiply with some color values).
		double mr, mg, mb;
		mr = tw_random(0.5, 1.0);
		mg = tw_random(0.5, 1.0);
		mb = tw_random(0.5, 1.0);
		colorize(starsprite[i], mr, mg, mb);

		//pos[i] = tw_random(Vector2(frame->window->surface->w,frame->window->surface->h)); //tw_random(map_size);
		pos[i] = -0.5*framesize + tw_random(framesize);

		// assume, the solarsystem map is at a refdist of 1.
		refdist[i] = 10.0 + tw_random(100.0);
		
		// the "visual" map at another refdist, increases linearly with distance
		// the max map size needed (depends on max distance to observer, or the min zoom level)
		pos[i] *= refdist[i] + d;

	}

	destroy_bitmaps(bmp, Ntypes);
}


void StarBackgr::calculate()
{
	// do nothing
}

void StarBackgr::animate(Frame *space)
{

	double d;
	d = (1 / space_zoom) - 1;

	int i;
	for ( i = 0; i < Nstars; ++i )
	{
		// assume, the solarsystem map is at a refdist of 1.
		// from this, calculate a "zoom" value elsewhere ...

		double z;
		z = 1 / (d + refdist[i]);

		// plotting: size should be fixed for point objects like stars (their "size" is a lens artifact).
		Vector2 s = starsprite[i]->size();
		Vector2 p;
		p = (pos[i] - space_center) * z;
		//p = pos[i]*z - space_center*space_zoom;
		p -= 0.5*s;
		p += space_view_size / 2;
		starsprite[i]->draw(p, s, 0, space);
	}
}











