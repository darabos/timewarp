#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE

#include "../melee/mview.h"
#include "../frame.h"

#include "gamedata.h"
#include "gamegeneral.h"

#include "gameproject.h"


void makevideobmp(BITMAP *&bmp)
{
	BITMAP *newbmp;
	int w, h;

	w = bmp->w;
	h = bmp->h;

	newbmp = create_video_bitmap(w, h);

	if (newbmp)
	{
		destroy_bitmap(bmp);
		bmp = newbmp;
	}
}


/*
// to be used by the game: it's a preprocessing operation
SpaceSprite *create_sprite(char *bmpfilename, int _attributes)
{
	RGB pal[256];
	BITMAP *bmplist[1];
	bmplist[0] = load_bitmap(bmpfilename, pal);


	int sprite_count, rotations;

	sprite_count = 1;
	rotations = 1;

	SpaceSprite *s;
	s = new SpaceSprite(bmplist, sprite_count, rotations, _attributes);

	destroy_bitmap(bmplist[0]);

	return s;
}
*/




char replace01txt[512];

char *replace01(char *txt, int newnum)
{
	char newnumtxt[2];

	strcpy(replace01txt, txt);

	int n1 = newnum / 10;
	int n2 = newnum - n1 * 10;

	newnumtxt[0] = '0' + n1;
	newnumtxt[1] = '0' + n2;

	int i = 0;
	while ( replace01txt[i] != 0   && replace01txt[i+1] != 0 &&
			replace01txt[i] != '0' && replace01txt[i+1] != '1' )
		++i;

	if (replace01txt[i] != 0   && replace01txt[i+1] != 0)
	{
		replace01txt[i]   = newnumtxt[0];
		replace01txt[i+1] = newnumtxt[1];
	}

	return replace01txt;
}


// to be used by the game: it's a preprocessing operation


SpaceSprite *create_sprite_old(char *bmpfilename, int _attributes, int rotations, int bpp, double scale, bool vidmem )
{
	RGB pal[256];
	BITMAP **bmplist;
	bmplist = new BITMAP* [rotations];

	char *tmp;

	int i;
	for ( i = 0; i < rotations; ++i )
	{
		tmp = replace01(bmpfilename,i+1);

		BITMAP *tmpbmp = load_bitmap(tmp, pal);
		if (tmpbmp == 0)
			break;

		// load the bitmap data in the correct color depth (since then you
		// can use the masked_blit operation without problems).
		bmplist[i] = 0;
		if (vidmem)
			create_video_bitmap(tmpbmp->w, tmpbmp->h);
		if (!bmplist[i])
			bmplist[i] = create_bitmap_ex(bpp, tmpbmp->w, tmpbmp->h);

		blit(tmpbmp, bmplist[i], 0, 0,  0, 0,  tmpbmp->w, tmpbmp->h);
		destroy_bitmap(tmpbmp);

		if (scale != 1)
		{
			if (vidmem)
				tmpbmp = create_video_bitmap(iround(bmplist[i]->w * scale), iround(bmplist[i]->h * scale));
			else
				tmpbmp = create_bitmap_ex(bpp, iround(bmplist[i]->w * scale), iround(bmplist[i]->h * scale));

			stretch_blit(bmplist[i], tmpbmp, 0, 0, bmplist[i]->w, bmplist[i]->h,
				0, 0, tmpbmp->w, tmpbmp->h);

			destroy_bitmap(bmplist[i]);
			bmplist[i] = tmpbmp;		// point to the scaled one.
		}
	}

	int Nbmp;
	Nbmp = i;

	int sprite_count;

	if ( Nbmp == 1 )
	{
		sprite_count = 1;
	} else {
		sprite_count = rotations;
		rotations = 1;
	}


	SpaceSprite *s;
	s = new SpaceSprite(bmplist, sprite_count, rotations, _attributes);

	for ( i = 0; i < Nbmp; ++i )
		destroy_bitmap(bmplist[i]);
	delete[] bmplist;

	return s;
}




SpaceSprite *create_sprite(char *bmpfilename, int _attributes, int rotations, int bpp, double scale, bool vidmem )
{
	BITMAP **bmplist;
	int maxframes = 1024;
	bmplist = new BITMAP* [maxframes];

	char *tmp;

	int i = 0;
	for ( ; ; )
	{
		if (i >= maxframes)
		{
			tw_error("too many frames");
			break;
		}
		tmp = replace01(bmpfilename,i+1);

		// if the filename is not numbered
		if (i > 0 && strcmp(tmp, bmpfilename) == 0)
			break;

		BITMAP *tmpbmp = load_bitmap(tmp, 0);
		if (tmpbmp == 0)
			break;		// the file does not exist

		// load the bitmap data in the correct color depth (since then you
		// can use the masked_blit operation without problems).
		bmplist[i] = 0;
		if (vidmem)
			create_video_bitmap(tmpbmp->w, tmpbmp->h);
		if (!bmplist[i])
			bmplist[i] = create_bitmap_ex(bpp, tmpbmp->w, tmpbmp->h);

		blit(tmpbmp, bmplist[i], 0, 0,  0, 0,  tmpbmp->w, tmpbmp->h);
		destroy_bitmap(tmpbmp);

		if (scale != 1)
		{
			if (vidmem)
				tmpbmp = create_video_bitmap(iround(bmplist[i]->w * scale), iround(bmplist[i]->h * scale));
			else
				tmpbmp = create_bitmap_ex(bpp, iround(bmplist[i]->w * scale), iround(bmplist[i]->h * scale));

			stretch_blit(bmplist[i], tmpbmp, 0, 0, bmplist[i]->w, bmplist[i]->h,
				0, 0, tmpbmp->w, tmpbmp->h);

			destroy_bitmap(bmplist[i]);
			bmplist[i] = tmpbmp;		// point to the scaled one.
		}

		++i;
	}

	int sprite_count;
	sprite_count = i;

	SpaceSprite *s;
	s = new SpaceSprite(bmplist, sprite_count, rotations, _attributes);

	for ( i = 0; i < sprite_count; ++i )
		destroy_bitmap(bmplist[i]);
	delete[] bmplist;

	return s;
}


/*
SpaceSpriteWin::SpaceSpriteWin(BITMAP **bmplist, int sprite_count, int rotations, int _attributes, WindowInfo *owininfo)
:
SpaceSprite(bmplist, sprite_count, rotations, _attributes)
{
	wininfo = owininfo;
}


THIS ISN'T NEEDED !!
space_zoom and space_center do the job well, in whatever way those are calculated
(can be derived in the game, from something).

void SpaceSpriteWin::animate(Vector2 pos, int index, Frame *frame, double scale)
{
	Vector2 P, S;
	double sc;

	Vector2 framesize;
	framesize = Vector2(frame->surface->w, frame->surface->h);

	// fixed-scale size (consistency of plotting in differently sized frames)
	//sc = (framesize.x / wininfo->refscreenw);
	// not really needed - it's already relative to the window width!?
	sc = 1;

	sc *= wininfo->zoomlevel;

	sc *= framesize.x / wininfo->maparea.x;
	// in this way, a single maparea fits exactly into a single window (or at least
	// in the x-direction).

	S = size() * sc;

	// a single maparea fits exactly into a frame (whatever the size of the frame is)
	// the relative position on the maparea is projected into the screen.
	// if you want map-wrap (left=right), then:
	//P = framesize.x * min_delta(pos, wininfo->mapcenter, map_size) / wininfo->maparea.x;
	// otherwise, use  this:
	//P = framesize.x * (pos - wininfo->mapcenter) / wininfo->maparea.x;
	//P *= wininfo->zoomlevel;
	P = sc * (pos - wininfo->mapcenter);
	P += framesize / 2;
	P -= 0.5 * S;			// P = the top-left corner of the bmp, while the pos is the center.

	draw(P, S, index, frame);

	//sprite->animate(pos, sprite_index, space);
}
*/





// go from screen coordinates to physical world coordinates
// (this is useful for the mouse)
Vector2 uncorner ( Vector2 pos )
{
	pos -= space_view_size / 2;
	pos /= space_zoom;
	pos += space_center;

	/*
	while (pos.x < -map_size.x/2) pos.x += map_size.x;
	while (pos.x >  map_size.x/2) pos.x -= map_size.x;
	while (pos.y < -map_size.y/2) pos.y += map_size.x;
	while (pos.y >  map_size.y/2) pos.y -= map_size.x;
	*/
	normalize(pos);

	return pos;
}





MousePtr::MousePtr(SpaceSprite *osprite)
:
SpaceObject(0, 0, 0, osprite)
{
	layer = LAYER_HOTSPOTS;
	attributes |= ATTRIB_UNDETECTABLE;	// isn't used for queries now.
	vel = 0;

	layer = 0;	// it doesn't do anything.
	selection = 0;
	mouserange = 100.0;
}


void MousePtr::animate(Frame *f)
{
	int x, y;
	int w, h;

	w = sprite->width();
	h = sprite->height();

	Vector2 P;
	P = corner(pos);

	x = iround(P.x - 0.5 * w);
	y = iround(P.y - 0.5 * h);

	sprite->draw(x, y, 0, f->surface);
	f->add_box(x, y, w, h);


	if (selection)		// draw something, perhaps ??
	{
		Vector2 P;
		w = selection->get_sprite()->width();
		h = selection->get_sprite()->height();

		P = corner(selection->pos, Vector2(w,h));

		w *= iround(space_zoom);
		h *= iround(space_zoom);

		rect( f->surface, iround(P.x), iround(P.y), iround(P.x+w), iround(P.y+h), makecol(128,128,0) );
		f->add_box(iround(P.x), iround(P.y), w, h);

		/*
		P = uncorner(pos);
		w = P.x;
		h = P.y;
		char txt[512];
		sprintf(txt, "%i %i", w, h );
		message.out(txt);
		*/
	}
}

void MousePtr::newpos(int x, int y)
{
	pos.x = x;
	pos.y = y;

	pos = uncorner(pos);
	normalize(pos, map_size);

	if (pos.x < 0)
		pos.x = 0;

	if (pos.y < 0)
		pos.y = 0;

	if (pos.x > map_size.x-1)
		pos.x = map_size.x-1;

	if (pos.y > map_size.y-1)
		pos.y = map_size.y-1;

}

void MousePtr::calculate()
{
	FULL_REDRAW = 1;

	SpaceObject::calculate();

	// if the left mouse button is pressed, search for the closest object.
	// a null value means, there's no selection.
	selection = 0;
	if (mouse_b & 1)
	{
		double closestrange;
		

		double range2;
//		Vector2 pos2;
		// note, the mouse pointer is "absolute" on the screen; 
//		pos2 = pos;
//		pos = uncorner(pos);
		range2 = mouserange / space_zoom;

		closestrange = range2;

		Query a;
		for (a.begin(this, OBJECT_LAYERS, range2); a.current; a.next() )
		{
			double d;
			if (!a.current->isObject())
				continue;

			d = a.current->distance(this);

			if (d < closestrange)
			{
				closestrange = d;
				selection = (SpaceObject*) a.current;
			}

		}

//		pos = pos2;

	}
}







void WindowInfo::init(Vector2 omaparea, double orefscreenw, Frame *frame)
{
	maparea = omaparea;
	refscreenw = orefscreenw;
	mapcenter = 0;
	zoomlevel_abs = 1;
	minzoom = 0.5;

	// the drawing area (needed to calculate zoom)
	if (frame->surface)
		framesize = Vector2(frame->surface->w, frame->surface->h);
	else
		framesize = Vector2(frame->window->w, frame->window->h);

	zoomlevel_abs = framesize.x / maparea.x;
	zoomlevel = zoomlevel_abs;
	// in this way, a single maparea fits exactly into a single window (at least
	// in the x-direction).

	// this is 1:1 pixels in the reference screen.
	maxzoom = framesize.x / refscreenw;

	// correction applied to the y-size of the maparea:
	maparea.y = framesize.y / zoomlevel;

	g = 0;
}


void WindowInfo::set_game(GameBare *ag)
{
	g = ag;
}


void WindowInfo::center(Vector2 pos)
{
	mapcenter = pos;
}


void WindowInfo::centermove(double dx, double dy)
{
	mapcenter.x += dx;
	mapcenter.y += dy;
}


void WindowInfo::testzoom()
{
	if (zoomlevel < minzoom)	zoomlevel = minzoom;
	if (zoomlevel > maxzoom)	zoomlevel = maxzoom;

	// adjust the maparea to the new zoomlevel, so that the physical area matches what's seen
	// on the screen.
	maparea = framesize / zoomlevel;
}

void WindowInfo::zoom(double z)
{
	zoomlevel_abs *= z;

	if (zoomlevel_abs < maxzoom || !g)
	{
		zoomlevel = zoomlevel_abs;
		testzoom();

	} else if(g)	// zoom continues, in terms of physical inflation of space
	{
		int i;
		double scale;

		scale = z;

		for ( i = 0; i < g->num_items; ++i )
		{
			if (g->item[i]->isLocation())
			{
				g->item[i]->change_pos(scale);	// the change in position.
			}
		}

		mapcenter *= scale;

		g->size *= scale;

		g->prepare();
	}
}


void WindowInfo::set_zoom(double z)
{
	zoomlevel_abs = z;
	zoomlevel = zoomlevel_abs;
	testzoom();
}

void WindowInfo::zoomlimit(double W)
{
	minzoom = framesize.x / W;
	//maxzoom = 1.0;
	maxzoom = framesize.x / refscreenw;
}

void WindowInfo::zoomlimit(double min, double max)
{
	minzoom = min;
	//maxzoom = 1.0;
	maxzoom = max;
}

void WindowInfo::scaletowidth(double W)
{
	zoomlevel_abs = framesize.x / W;
	zoomlevel = zoomlevel_abs;
	testzoom();
}


void WindowInfo::edgecorrect()
{
	Vector2 D;

	D = mapcenter - 0.5*maparea;
	if (D.x < 0)
		mapcenter.x -= D.x;
	if (D.y < 0)
		mapcenter.y -= D.y;

	D = mapcenter + 0.5*maparea - map_size;
	if (D.x > 0)
		mapcenter.x -= D.x;
	if (D.y > 0)
		mapcenter.y -= D.y;
}





XFleet::XFleet()
{
	Nships = 0;
}


void XFleet::add(char *race, int N)
{
	while (N > 0 && Nships < maxfleetsize-1)
	{
		info[Nships].setname(race);
		++Nships;
		--N;
	}
}


void XFleetShipinfo::setname(char *c)
{
	strncpy(name, c, 30);
}





Vector2  bitmap_size(BITMAP *b)
{
	return Vector2(b->w, b->h);
}

Vector2i bitmap_sizei(BITMAP *b)
{
	return Vector2i(b->w, b->h);
}





Periodics::Periodics(double aperiod)
{
	t = 0;
	period = aperiod;
	if (physics && period < physics->frame_time * 1E-3)
	{tw_error("period is too small");}
}

bool Periodics::update()
{
	if (!physics){tw_error("Periodics doesn't work without a physics");}

	t += physics->frame_time * 1E-3;
	if (t > period)
	{
		t -= period;
		return true;
	} else
		return false;
}




void staywithin(double xmin, double *x, double xmax)
{
	if (*x < xmin)
		*x = xmin;
	if (*x > xmax)
		*x = xmax;
}


void staywithin(int xmin, int *x, int xmax)
{
	if (*x < xmin)
		*x = xmin;
	if (*x > xmax)
		*x = xmax;
}

void staywithin(Vector2 pmin, Vector2 *p, Vector2 pmax)
{
	staywithin(pmin.x, &p->x, pmax.x);
	staywithin(pmin.y, &p->y, pmax.y);
}





void locate_onedge_aligned_to_center(SpaceLocation *s, Vector2 center, Vector2 wh)
{
	Vector2 Prel;
	double dx, dy, ds, dt;

	// negative angle, because you don't want to point away from the center, but
	// towards the center.
	dx = cos(s->angle + PI);
	dy = sin(s->angle + PI);

	// treat only quadrant 1
	ds = fabs(dx);
	dt = fabs(dy);

	if (dt < ds * wh.y / wh.x)
		Prel = Vector2(wh.x, wh.x * dt / ds);
	else
		Prel = Vector2(wh.y * ds / dt, wh.y);

	// mirror back to the true quadrants
	if (dx < 0)
		Prel.x *= -1;

	if (dy < 0)
		Prel.y *= -1;

	// the position then becomes:
	s->pos = center + Prel;
}




void createfilelist(char ***List, int *N, char *scanname, int remext)
{
	int err;
	al_ffblk info;
	

	// first, a iteration 
	*N = 0;
	err = al_findfirst(scanname, &info, FA_ARCH);		
	while (!err)
	{
		if (!isevildir(info.name))
			++(*N);
		err = al_findnext(&info);
	}	
	al_findclose(&info);

	// allocate memory for the filenames
	*List = new char* [(*N)];


	// read the filenames
	int i;

	i = 0;
	err = al_findfirst(scanname, &info, FA_ARCH);	
	
	while (!err)
	{
		if (!isevildir(info.name))
		{
			if (!remext)
			{
				(*List)[i] = new char [strlen(info.name)+1];
				strcpy((*List)[i], info.name);
			} else {

				// copy the "*" wildcard part of the filename, and not the rest.
				char tmpstr[512], *tmp;
				int i1, i2;

				tmp = strrchr(scanname, '/');
				i1 = 0;
				while (tmp[i1+1] == info.name[i1] && info.name[i1] != 0)
					++i1;

				int L1, L2;
				L1 = strlen(scanname);
				L2 = strlen(info.name);

				i2 = 1;
				while (scanname[L1-i2] == info.name[L2-i2] && i2 < L1 && i2 < L2)
					++i2;

				int k;
				for ( k = i1; k <= L2-i2; ++k )
				{
					tmpstr[k-i1] = info.name[k];
				}
				tmpstr[k-i1] = 0;

				(*List)[i] = new char [strlen(tmpstr)+1];
				strcpy((*List)[i], tmpstr);
			}
			++i;
		}
		
		err = al_findnext(&info);
	}
	
	al_findclose(&info);
}

