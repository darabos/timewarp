
#include "../../melee.h"

REGISTER_FILE

#include "../../frame.h"
#include "../../melee/mframe.h"
#include "../../melee/mview.h"
#include "../../melee/mitems.h"

#include "space_body.h"



// definition of the ellips:
// x*x + b*y*y = R*R.
void drawellips(Frame *f, Vector2 center, double R, double b, int col)
{

	//rectfill(g->view->frame->surface, 0, 0, 500, 500, makecol(100,20,30));

	R *= space_zoom;
	//center = (center - space_center) * space_zoom + space_view_size/2;
	center = corner(center);

	int xref, yref, jbest;
	int lastxref, lastyref, lastjbest;

	double b_sqrt = sqrt(b);

	xref = 0;
	yref = iround(-R/b_sqrt);
	jbest = 0;

	int ymin_visual, ymax_visual, ymin, ymax;

	ymin_visual = iround(-R/b_sqrt) + center.y;
	ymax_visual = iround( (R+1)/b_sqrt) + center.y;
	if (ymin_visual < 0)
		ymin_visual = 0;

	if (ymax_visual > space_view_size.y)
		ymax_visual = space_view_size.y;
	ymin = ymin_visual - center.y;
	ymax = ymax_visual - center.y;

	if (ymin > ymax)
		return;

	yref = ymin;

	// you see only the (lower) part of the ellips.
	// put xref in an appropriate starting position.
	double arg = R*R - b*ymin*ymin;
	if ( ymin_visual == 0 && arg > 0 )
	{
		xref = iround( sqrt(R*R - b*ymin*ymin) );
	}

	putpixel(f->surface,  xref + center.x, yref + center.y, col);

	// half a circle (downwards) of possible new locations to put a new pixel; radius = 2
	int dx[7] = { 2,  2,  1,  0, -1, -2, -2};
	int dy[7] = { 0,  1,  2,  2,  2,  1,  0};

	//while (yref < (R+1)/b_sqrt && xref >= 0)
	// xref>0 means, you're in the right half of the circle.
	while (yref <= ymax && xref >= 0)
	{

		lastxref = xref;
		lastyref = yref;
		lastjbest = jbest;

		int j;
		double S, Sbest;

		jbest = 0;
		Sbest = 1E9;

		for ( j = 0; j < 7; ++j )
		{
			// move in the direction of the same quadrant...
			if (abs(j - lastjbest) >= 3)
				continue;

			int ix, iy;

			ix = xref + dx[j];
			iy = yref + dy[j];

			if (ix == lastxref && iy == lastyref)
				continue;

			double x, y;
			x = ix + 0.5;
			y = iy + 0.5;

			S = fabs(x*x + b * y*y - R*R);

			if (S < Sbest)
			{
				Sbest = S;
				jbest = j;
			}
		}

		xref += dx[jbest];
		yref += dy[jbest];

		putpixel(f->surface,  xref + center.x, yref + center.y, col);
		putpixel(f->surface, -xref + center.x, yref + center.y, col);
	}

}



// This routine isn't correct;
// the observer angle doesn't "change things",
// but it doesn't matter much

void shadowpaintcircle(SpaceSprite *spr, double fi_s)
{
	BITMAP *bmp;
	bmp = spr->get_bitmap(0);

	int image_size;
	image_size = bmp->w;

	double min_light = 0.3;
	double max_light = 1.0;

	int i, j;
	
	// x, y is in the drawing area;
	// z points outside the screen.

	Vector3D observer(0, 0, 1);
	observer.normalize();

	// circle angle ... angular position of the planet in the solar plane.
	// double fi_s;
	//fi_s = -0.25*PI;
	// offset in the solar plane
	double th_s;
	th_s = 0.1 * PI;	// should be 0, but I guess a slight tilt looks nicer ?!

	Vector3D sunshine(cos(fi_s)*cos(th_s), sin(fi_s)*cos(th_s), -sin(th_s));
	sunshine.normalize();

	for (i=-image_size/2; i<image_size/2; i++)
	{
		for (j=-image_size/2; j<image_size/2; j++)
		{
			if (i*i + j*j > image_size*image_size/4)
				continue;

			double xx, yy, zz, xs, ys, zs;

			// you see planets from _very_ far away ... and they're very small,
			// so a simple equation should suffice !!
			
			xx = i/double(image_size);
			yy = j/double(image_size);
			zz = sqrt(1 - xx*xx - yy*yy);
			
			xs = xx; ys = yy; zs = zz;
			// = position of the visible sphere point in 3D space ?
			
			Vector3D surfacenormal(xs, ys, zs);
			surfacenormal.normalize();
			// the point that is facing you ;)
			
			// now, calculate the light intensity.
			// two parts: how much light does a part of the earth receive,
			// and how much does it radiate out:
			
			double I, J;
			I = -1 * sunshine.dot(surfacenormal);	// incoming sunshine is in opposite direction from outgoing surface normal
			//				I *= surfacenormal.dot(observer);		// this is directional shading... shouldn't be here.
			if (I < 0)
				I = 0;
			I = min_light + I*(max_light-min_light);
			
			
			// and, what about specular, i.e., reflections of light towards
			// the observer?
			Vector3D reflection(0,0,0);	// the reflected sun-light direction
			reflection = -1*(sunshine.cross(surfacenormal)).cross(surfacenormal) - sunshine.dot(surfacenormal) * surfacenormal;
			J = reflection.dot(observer);
			if (J < 0)
			J = 0;
			J *= J;	// to make it more pronounced :)
			J *= J;
			J *= J;
			J *= J;
			J *= 0.8;

			int col, r, g, b;

			col = getpixel(bmp, i+image_size/2, j+image_size/2);
			r = getr32(col);
			g = getg32(col);
			b = getb32(col);

			if (!(r == 255 && g == 0 && b == 255))
			{
				r *= I;
				g *= I;
				b *= I;

				// don't add too much ...
				r += J * 128;
				g += J * 128;
				b += J * 128;

				if (r > 255) r = 255;
				if (g > 255) g = 255;
				if (b > 255) b = 255;
				
				col = makecol32(r,g,b);
				putpixel(bmp, i+image_size/2, j+image_size/2, col);
			}
		}
	}

}



SolarBody::SolarBody(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite, Vector2 sunpos,
					 int bodynum,
					 Vector2 Ec, double ER, double Eb, int Ecol)
:
SpaceObject(creator, opos, oangle, osprite)
{
	layer = LAYER_SHOTS;

	mass = 1E6;

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;
	layer = LAYER_SHIPS;

	stayhere = pos;

	if (pos != sunpos)
	{
		sunangle = (pos - sunpos).atan();
		shadowpaintcircle(sprite, sunangle);
	}

	id = ID_SOLAR_BODY;
	solar_body_num = bodynum;

	ellipscenter = Ec;
	ellipsR = ER;
	ellipsb = Eb;
	ellipscol = Ecol;
}

void SolarBody::animate(Frame *f)
{
	SpaceObject::animate(f);

	if ( ellipscol != 0 )
		drawellips(f, ellipscenter, ellipsR, ellipsb, ellipscol);
}

void SolarBody::calculate()
{
	pos = stayhere;
	vel = 0;

	SpaceObject::calculate();
}















