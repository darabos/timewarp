#include <string.h>

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
//	center = corner(center);
	// the "corner" goes wrong, if you have very large ellipses (eg of a sun far away)
	// therefore, use the unwrapped version like this:
	center = (center - space_center) * space_zoom + space_view_size/2;

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

	double min_light = 0.5;
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









/*
mapeditor:

  +/- zoom in/out on the map

  right-click = navigate on the map (becomes the new focus)

  left-click = select another star (stays fixed)
  ctrl = press, make the star follow the cursor, press again to fix it to the new position

  use the menu to replace star-type, or create a new star.

*/



MapEditor::MapEditor()
{
	maphaschanged = false;

	objmap = 0;
	selection = 0;
	//lastselection = 0;

	Tedit = 0;
	bnew = 0;
	breplace = 0;

	maplevel = 0;

	scalepos = 0;

	moveselection = false;
}

MapEditor::~MapEditor()
{
}



void MapEditor::set_game(GameBare *agame, MousePtr *aptr)
{
	g = agame;
	ptr = aptr;
}



void MapEditor::set_interface( IconTV *aTedit, Button *abreplace, Button *abnew )
{
	Tedit = aTedit;
	breplace = abreplace;
	bnew = abnew;
}

void MapEditor::set_mapinfo( MapSpacebody *aobjmap, int amaplevel, double ascalepos)
{
	objmap = aobjmap;
	maplevel = amaplevel;
	scalepos = ascalepos;
}


MapObj *MapEditor::create_mapobj(Vector2 pos)
{
	return new MapObj(0, pos, 0, Tedit->tv->makespr());
}


void MapEditor::replace()
{
	int k;
	k = selection->starnum;

	objmap->sub[k]->type = Tedit->tv->isel;
	//delete selection->
	selection->rem_sprite();
	selection->set_sprite(Tedit->tv->makespr());
}

void MapEditor::add()
{
	// just make a new selection
	selection = create_mapobj(ptr->pos);
	g->add(selection);
	
	moveselection = true;
	
	// also ... add it to the map ?? with default settings ..
	selection->starnum = objmap->add(maplevel);	// level 1 = stars
	
	int k;
	k = selection->starnum;
	objmap->sub[k]->type = Tedit->tv->isel;

	//delete selection->get_sprite();
	selection->set_sprite(Tedit->tv->makespr());
}

void MapEditor::move()
{
	selection->pos = ptr->pos;
	staywithin(0, &(selection->pos), map_size);
	
	
	int k;
	k = selection->starnum;
	objmap->sub[k]->position = selection->pos / scalepos;
	
	maphaschanged = true;
}

void MapEditor::newselection()
{
	selection = (MapObj*) ptr->selection;
	g->maparea->flag.left_mouse_press = false;
}

		
void MapEditor::calculate()
{
	// keep track of the last star that was clicked on by the mouse
//	if (ptr->selection && (ptr->selection->id == MAPOBJ_ID))
//		lastselection = (MapObj*) ptr->selection;


	// move a star (only if CTRL is pressed)
	if ( keyhandler.keyhit[KEY_LCONTROL] )
		moveselection = !moveselection;

	if (selection && moveselection)
	{
		move();
	}


	// delete a star
	if (selection && keyhandler.keyhit[KEY_DEL])
	{
		// remove from game physics
		g->remove(selection);

		// remove from the map
		objmap->rem(selection->starnum);


		// remove from memory
		delete selection;

		selection = 0;
	}

	// select a star for movement (or so ...)
	// using left-click of the mouse on the starmap area of the menu
	// but only select map-objects that you can edit...
	if ( g->maparea->flag.left_mouse_press && !moveselection &&
			ptr->selection && ptr->selection->id == MAPOBJ_ID)
	{
		newselection();
	}



	if (breplace->flag.left_mouse_press && !moveselection)
	{

		// change the picture of the selected star
		if (selection)
		{
			replace();
		}
	}

	if (bnew->flag.left_mouse_press && !moveselection)
	{
		add();
	}

}







TVarea::TVarea(TWindow *menu, char *identbranch, int asciicode, bool akeepkey)
:
Area(menu, identbranch, asciicode, akeepkey)
{
//	char tmp[512];

//	strcpy(tmp, identbranch);
//	strcat(tmp, "dec_");
//	bdec = new Button(menu, tmp);
//	strcpy(tmp, identbranch);
//	strcat(tmp, "inc_");
//	binc = new Button(menu, tmp);

	N = 0;
	isel = 0;

	blist = 0;

	f = 1.0;
}

TVarea::~TVarea()
{
	if (blist)
		delete blist;

}

void TVarea::calculate()
{
	Area::calculate();

	//if (bdec->flag.left_mouse_press || binc->flag.left_mouse_press)
	if (flag.left_mouse_press || flag.right_mouse_press)
	{
		//if (binc->flag.left_mouse_press)
		if (flag.left_mouse_press)
			++isel;

		//if (bdec->flag.left_mouse_press)
		if (flag.right_mouse_press)
			--isel;

		if (isel < 0 )
			isel = N-1;
		
		if (isel >= N)
			isel = 0;

		//changebackgr(blist[isel]);
		overwritebackgr(blist[isel], f, makecol(0,0,0));

	}
}

void TVarea::newlist(int aN)
{
	if (blist)
		delete blist;

	N = aN;
	blist = new BITMAP* [N];
}

void TVarea::initnewlist()
{
	// check the common scaling factor ... cause the biggest bitmap should
	// still fit in the window ...
	f = 1.0;
	int i;
	for ( i = 0; i < N; ++i )
	{
		double x;

		x = double(backgr->w) / double(blist[i]->w);
		if (x < f)
			f = x;

		x = double(backgr->h) / double(blist[i]->h);
		if (x < f)
			f = x;
	}

	set_sel(0);
}

void TVarea::set(BITMAP **list, int aN)
{
	newlist(aN);

	int i;
	for ( i = 0; i < N; ++i )
		blist[i] = list[i];

	initnewlist();
}

void TVarea::set(SpaceSprite **list, int aN)
{
	newlist(aN);

	int i;
	for ( i = 0; i < N; ++i )
		blist[i] = list[i]->get_bitmap(0);

	initnewlist();
}

void TVarea::set_sel(int newsel)
{
	if (!(newsel >= 0 && newsel < N))
		return;

	isel = newsel;
	overwritebackgr(blist[isel], f, makecol(0,0,0));	
}


BITMAP *TVarea::show()
{
	return blist[isel];
}


SpaceSprite *TVarea::makespr()
{
	return new SpaceSprite(blist[isel]);
}



IconTV::IconTV(char *ident, int xcenter, int ycenter, BITMAP *outputscreen)
:
Popup(ident, xcenter, ycenter, outputscreen)
{
	tv = new TVarea(this, "plot_");
}

IconTV::~IconTV()
{
}



void IconTV::calculate()
{
	if (disabled)
		return;

	Popup::calculate();

}







// should be something like "star" perhaps ... it doesn't matter much, it's just a
// sprite.

MapObj::MapObj(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite)
:
SpaceObject(creator, opos, oangle, osprite)
{
	id = MAPOBJ_ID;
	layer = LAYER_SHOTS;
	sprite_index = 0;
}

void MapObj::animate(Frame *f)
{
	//SpaceObject::animate(f);
	//sprite->animate(pos, sprite_index, f);
	Vector2 s = sprite->size(sprite_index);
	double scale = space_zoom;
	if (scale < 0.25)
		scale = 0.25;
	sprite->draw(corner(pos)-0.5*s*scale, s * scale, sprite_index, f);
}

void MapObj::calculate()
{
	SpaceObject::calculate();
}


void MapObj::rem_sprite()
{
	// removes the sprite that was assigned to it
	delete sprite;
}





SolarBody::SolarBody(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite, Vector2 osunpos,
					 int bodynum,
					 Vector2 Ec, double ER, double Eb, int Ecol)
:
MapObj(creator, opos, oangle, osprite)
{
	// copy the original sprite, cause it's going to be altered.
	sprite = 0;
	set_sprite(osprite);

	layer = LAYER_SHOTS;

	mass = 1E6;

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;
	layer = LAYER_SHIPS;

	stayhere = pos;
	sunpos = osunpos;

	//id = ID_SOLAR_BODY;
	starnum = bodynum;

	ellipscenter = Ec;
	ellipsR = ER;
	ellipsb = Eb;
	ellipscol = Ecol;
}

SolarBody::~SolarBody()
{
	if (sprite)			// delete internal copy
		delete sprite;
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

void SolarBody::set_sprite(SpaceSprite *new_sprite)
{
	if (sprite)
		delete sprite;	// delete the internal copy

	origsprite = new_sprite;
	sprite = new SpaceSprite( *new_sprite );
	size = new_sprite->size();

	drawshadow();
}


void SolarBody::rem_sprite()
{
	// removes the sprite that was assigned to it ... and
	// that's the one pointed to by origsprite
	delete origsprite;
}


void SolarBody::drawshadow()
{
	BITMAP *dest, *src;
	src = origsprite->get_bitmap(0);
	dest = sprite->get_bitmap(0);
	blit(src, dest, 0, 0, 0, 0, src->w, src->h);

	if (pos != sunpos)
	{
		sunangle = (pos - sunpos).atan();
		shadowpaintcircle(sprite, sunangle);
	}

}














