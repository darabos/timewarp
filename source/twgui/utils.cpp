
#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"

#include "utils.h"
#include "area.h"
#include "twgui.h"

// ----------------- SOME GENERALLY USEFUL STRUCTURES / FUNCTIONS ------------------


// map a key to some ascii code, and map unmappable stuff to 128+scan code:
// this is useful for the menus
// note, separated are the "key" and the extra "control" key (ctrl, alt)
int mapkey(int scancode_key, int scancode_ctrl)
{
	int k;

	k = scancode_key;

	k |= scancode_ctrl << 8;
	
	// thus we have 2 scan codes, no more needed.
	return k;
}


BITMAP *find_datafile_bmp(DATAFILE *datafile, char *identif)
{
	char objname[128];

	if (strlen(identif) > 120)
	{
		tw_error("string exceeds max length");
	}

	strcpy(objname, identif);
	strcat(objname, "_BMP");	// default extension for .bmp files.

	DATAFILE *dat = find_datafile_object(datafile, objname);

	if (!dat)
	{
		//char txt[512];
		//sprintf(txt, "Could not find %s", objname);
		//tw_error(txt);
		return 0;
	}

	if (dat->type == DAT_BITMAP)
		return (BITMAP*) dat->dat;
	else
		return 0;
}


BITMAP *clone_bitmap(int bpp, BITMAP *src, double scale)
{
	BITMAP *dest, *convert;

	if ( !src )
		return 0;

	int W, H;

	W = iround( src->w * scale );
	H = iround( src->h * scale );

	dest = create_bitmap_ex(bpp, W, H);

	convert = create_bitmap_ex(bpp, src->w, src->h);
	blit(src, convert, 0, 0, 0, 0, src->w, src->h);		// use this to convert color depth

	if (W != src->w || H != src->h )
		stretch_blit(convert, dest, 0, 0, convert->w, convert->h, 0, 0, dest->w, dest->h);
	else
		blit(convert, dest, 0, 0, 0, 0, W, H);

	destroy_bitmap(convert);

	return dest;
}











normalmouse::normalmouse()
{
	bmp.original_mouse_sprite = mouse_sprite;
	bmp.newmousebmp = 0;

	reset();
}


void normalmouse::reset()
{
	update();		// initialize with current mouse values.
	pos2 = pos;
	oldpos = pos;
}


void normalmouse::copyinfo(normalmouse *othermouse)
{
	pos    = othermouse->pos;
	oldpos = othermouse->oldpos;
	pos2 = othermouse->pos2;
	left   = othermouse->left;
	mid    = othermouse->mid;
	right  = othermouse->right;
	bmp.newmousebmp = othermouse->bmp.newmousebmp;
	bmp.original_mouse_sprite = othermouse->bmp.original_mouse_sprite;
}


// NOTE:
// this means that pos and oldpos must always be re-set to the default
// (unmoved) position every iteration.
void normalmouse::move(int dx, int dy)
{
	pos.x += dx;
	pos.y += dy;

	oldpos.x += dx;
	oldpos.y += dy;
}

int normalmouse::vx()
{
	return pos.x - oldpos.x;
}

int normalmouse::vy()
{
	return pos.y - oldpos.y;
}


void normalmouse::update()
{
	if (mouse_needs_poll())
		poll_mouse();
	
	oldpos = pos2;	// this is exactly the "old" position, unmoved
	pos.set(mouse_x, mouse_y, mouse_z);
	pos2 = pos;

	int b = mouse_b;

	left.update(bool(b & 1));		// make distinction here; individual buttons shouldn't know of each other
	mid.update(bool(b & 4));
	right.update(bool(b & 2));
}


void normalmouse::mousebutton::update(bool newstatus)
{
	// we are mainly interested in changes in the button status.
	if (newstatus != button.status)
	{
		oldbutton = button;
	
		button.status = newstatus;	// on or of ;)

		if (button.status != oldbutton.status)
			button.time = get_time2() * 1E-3;				// detect timing of a change in button status

		button.change = true;

	} else
		button.change = false;
}



bool normalmouse::mousebutton::press()
{
	if (!button.change)
		return false;

	if (button.status && (!oldbutton.status))
		return true;
	else 
		return false;

}

bool normalmouse::mousebutton::release()
{
	if (!button.change)
		return false;

	if ((!button.status) && oldbutton.status)
		return true;
	else 
		return false;
}

bool normalmouse::mousebutton::dclick()
{
	if (!button.change)		// check if the mouse is clicked
		return false;

	if (!button.status)		// check if the mouse button is pressed
		return false;

	if (button.time - oldbutton.time < 0.2)	// check time between release and press
		return true;
	

	return false;
}

bool normalmouse::mousebutton::hold()
{
	return button.status;
}


void normalmouse::bmpstr::init(BITMAP *newbmp)
{
	newmousebmp = newbmp;
}

void normalmouse::bmpstr::set()
{
	if (newmousebmp)
	{
		set_mouse_sprite(newmousebmp);
	}
}

void normalmouse::bmpstr::restore()
{
	set_mouse_sprite(original_mouse_sprite);
}

void normalmouse::posstr::set(int xnew, int ynew, int znew)
{
	x = xnew;
	y = ynew;
	wheel = znew;
}

void normalmouse::posstr::move(int dx, int dy)
{
	x += dx;
	y += dy;
}





scrollpos_str::scrollpos_str()
{
	x = 0;
	y = 0;

	left = 0;
	right = 0;
	up = 0;
	down = 0;
	scrollhor = 0;
	scrollvert = 0;
}


void scrollpos_str::set(int xscroll, int yscroll, int Nxscroll, int Nyscroll,
						int Nx_show, int Ny_show)
{
	x = xscroll;	// this is top-left corner item of the visible screen
	y = yscroll;

	xselect = x;	// this has some extra movement capacility within the visible screen
	yselect = y;

	Nxshow = Nx_show;	// the size (in items) of the visible screen
	Nyshow = Ny_show;

	Nx = Nxscroll - Nxshow + 1;	// the scrollable freedom
	Ny = Nyscroll - Nyshow + 1;

	// note: Nx = 1 indicates the "base" position; values >1 indicate out-of-sight positions.
	// or: Nx-Nxshow indicates the out-of-sight positions, +1 indicates the base value position.
	
	// check ranges.
	if (Nx < 1)
		Nx = 1;
	if (Ny < 1)
		Ny = 1;

	check_pos();
}


void scrollpos_str::check_pos()
{
	// check the scroll position
	if (x > Nx-1)
		x = Nx-1;

	if (y > Ny-1)
		y = Ny-1;

	if (x < 0)
		x = 0;

	if (y < 0)
		y = 0;

	// check the select position
	// note, the selected item _must_ always lie within the visible area:

	if (xselect < x)
		xselect = x;

	if (xselect > x + Nxshow - 1)
		xselect = x + Nxshow - 1;

	if (yselect < y)
		yselect = y;

	if (yselect > y + Nyshow - 1)
		yselect = y + Nyshow - 1;
}


void scrollpos_str::add(int dx, int dy)
{
	x += dx;
	y += dy;

	check_pos();
}



void scrollpos_str::check_sel()
{
	if (xselect < x)
		x = xselect;

	if (xselect > x + Nxshow - 1)
		x = xselect-Nxshow+1;

	if (yselect < y)
		y = yselect;

	if (yselect > y + Nyshow - 1)
		y = yselect-Nyshow+1;
}



void scrollpos_str::set_sel(int xsel, int ysel)
{
	xselect = xsel;
	yselect = ysel;

	check_sel();
	check_pos();
}



// alpha is a value between 0 and 1
void scrollpos_str::set_percent_pos_x(double alpha)
{
	x = iround( (Nx-1) * alpha );
	check_pos();
}

void scrollpos_str::set_percent_pos_y(double alpha)
{
	y = iround( (Ny-1) * alpha );
	check_pos();
}

void scrollpos_str::set_pos(int xnew, int ynew)
{
	if ( xnew >= 0 && xnew < Nx && ynew >= 0 && ynew < Ny )
	{
		x = xnew;
		y = ynew;

		if (scrollhor)
			scrollhor->setrelpos(double(x)/Nx);
		if (scrollvert)
			scrollvert->setrelpos(double(y)/Ny);
	}

	check_pos();
}



void scrollpos_str::bind(AreaGeneral *aleft, AreaGeneral *aright,
								AreaGeneral *aup, AreaGeneral *adown,
								ScrollBar *ascrollhor, ScrollBar *ascrollvert)
{
	left = aleft;
	right = aright;
	up = aup;
	down = adown;
	scrollvert = ascrollvert;
	scrollhor = ascrollhor;
}


void scrollpos_str::calculate()
{
	int xold, yold;

	xold = x;
	yold = y;


	if (left && left->flag.left_mouse_press)
	{
		--xselect;
		if (xselect < x)
			x = xselect;

	}

	if (right && right->flag.left_mouse_press)
	{
		++xselect;
		if (xselect > x + Nxshow - 1)
			x = xselect-Nxshow+1;
	}

	if (up && up->flag.left_mouse_press)
	{
		--yselect;
		if (yselect < y)
			y = yselect;

	}

	if (down && down->flag.left_mouse_press)
	{
		++yselect;
		if (yselect > y + Nyshow - 1)
			y = yselect-Nyshow+1;
	}


	check_pos();

	if (scrollhor && xold != x)
		scrollhor->setrelpos(double(x)/Nx);

	if (scrollvert && yold != y)
		scrollvert->setrelpos(double(y)/Ny);
	

	if (scrollhor && scrollhor->flag.left_mouse_hold)
		set_percent_pos_x(scrollhor->relpos);

	if (scrollvert && scrollvert->flag.left_mouse_hold)
		set_percent_pos_y(scrollvert->relpos);

}

