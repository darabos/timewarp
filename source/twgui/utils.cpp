
#include <allegro.h>
#include <string.h>

#include "../melee.h"

#include "utils.h"

// ----------------- SOME GENERALLY USEFUL STRUCTURES ------------------


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
		return 0;

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

	W = src->w * scale;
	H = src->h * scale;

	convert = create_bitmap_ex(bpp, src->w, src->h);
	blit(src, convert, 0, 0, 0, 0, src->w, src->h);		// use this to convert color depth

	dest = create_bitmap_ex(bpp, W, H);
	stretch_blit(convert, dest, 0, 0, convert->w, convert->h, 0, 0, dest->w, dest->h);

	destroy_bitmap(convert);

	return dest;
}





button_bitmaps::~button_bitmaps()
{
	if (bmp_defocus)
		destroy_bitmap(bmp_defocus);
	if (bmp_focus)
		destroy_bitmap(bmp_focus);
	if (bmp_selected)
		destroy_bitmap(bmp_selected);
}



void button_bitmaps::init(BITMAP *bdefocus, BITMAP *bfocus, BITMAP*bselected)
{
	bmp_defocus  = bdefocus;  //clone_bitmap(bpp, bdefocus,  scale);
	bmp_focus    = bfocus;    //clone_bitmap(bpp, bfocus,    scale);
	bmp_selected = bselected; //clone_bitmap(bpp, bselected, scale);
}







normalmouse::normalmouse()
{
	bmp.original_mouse_sprite = mouse_sprite;
	bmp.newmousebmp = 0;
}


void normalmouse::copyinfo(normalmouse *othermouse)
{
	pos    = othermouse->pos;
	oldpos = othermouse->oldpos;
	left   = othermouse->left;
	mid    = othermouse->mid;
	right  = othermouse->right;
}



void normalmouse::update()
{
	
	oldpos = pos;
	pos.set(mouse_x, mouse_y, mouse_z);

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





void scrollpos_str::set(int xscroll, int yscroll, int Nxscroll, int Nyscroll)
{
	x = xscroll;
	y = yscroll;
	Nx = Nxscroll;
	Ny = Nyscroll;
}


void scrollpos_str::add(int dx, int dy)
{
	x += dx;
	y += dy;

	if (x > Nx-1)
		x = Nx-1;

	if (y > Ny-1)
		y = Ny-1;

	if (x < 0)
		x = 0;

	if (y < 0)
		y = 0;
}

