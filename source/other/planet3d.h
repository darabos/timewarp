#ifndef __PLANET_3D__
#define __PLANET_3D__


#include "melee/mcbodies.h"


class Planet3D : public Planet {
	
	BITMAP *image32bit;
	SpaceSprite *map;		// color map
	SpaceSprite *dummy;		// the "target" sprite where the 2d planet image is stored
	
	int image_size, visual_size;

	int PlanetUsespec;
	
	double theta, fi, rad;
	double spin;//degrees per second
	
	int draw_reserve;
	
	//	bool AA;
	
	int mapW, mapH;
	
	//	unsigned char color_map[1000][500][4];
	//	double spec_map[1000][500];
	
	struct base_map_type {double lat, lon, diff, spec;} *base_map;
	
	unsigned int	*base_map_linear;		// mapping of coordinates
	unsigned int	*base_shade_linear, *base_spec_linear;		// shades ?
	unsigned char	*color_map_linear, *spec_map_linear;
	
	unsigned int	*base_map_and_shade_resorted;
	// linear means in this case, a linear array
	
	int jmin[1000], jmax[1000];
	
	public:
		
	Planet3D(Vector2 opos, SpaceSprite *color_map, SpaceSprite *spec_map,
			SpaceSprite *ObjectSprite,
			int planet_radius, int aPlanetUsespec,
			double turn_rate, double tilt, 
			double sun_vertangle,	// positive is pointing down
			double sun_horizangle,	// oriented along x
			double sunr, double sung, double sunb, bool invcolor = false);
		
	virtual void calculate();
	void animate_pre();		// does lots of calculations, that are needed for drawing - it puts together the picture
	virtual void animate(Frame * space);
		
	~Planet3D();
};

#endif // __PLANET_3D__
