
// NOTE: the high-level routines and structures are on the bottom of this file.
// the lowest level functions are on the top.

// Three distinctions: 1. area detection and definition, 2. bitmap feedback, 3. a small
// class for each button you need to perform a specific task based on a particular mouse
// operation.

// This makes (imo) best use of classes and their
// overloading (specialization) abilities. I think that's the most general way and can support
// a wide variety of implementations.

#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE

#include "area.h"





// --------------------- DEFINING AREAS -------------------
// areas can be places where special actions can take place (graphically)
// but first, these places need to be defined, and their (functional) link
// to those possible actions needs to be shown.



// divide the screen in different areas, each of which has the basic properties
// of focus, defocus, and mouse status.

// this checks whether a area (usually a box) has focus or not depending on I/O (mouse, keyboard)


// a menu superstructure ... to manage a bunch of related areas ... in a reserved area/ region


BITMAP* AreaReserve::bmp(char *bmpname)
{
	return clone_bitmap(bitmap_color_depth(drawarea),
						find_datafile_bmp(datafile, bmpname),
						scale);
}

// make a backup of the old screen
// note, it's scaled to maximum W,H=1.0, 1.0 being the screen width
// default_W is the screen width for which all buttons in the menu are developed, and
// this is used to scale them to other screen resolutions.
AreaReserve::AreaReserve(char *identbase, int dx, int dy, int dW, int dH, int def_W, char *datafilename, BITMAP *outputscreen, char *backgrname)
{
	// from the scaled thing, calculate the actual size on the current screen

	screenreserve = outputscreen;

	strcpy(ident, identbase);

	datafile = load_datafile(datafilename);
	if (!datafile)
	{
		tw_error("AreaReserve: Could not load datafile");
	}

	default_W = def_W;
	scale = double(screen->w) / double(default_W);
	

	x = dx * scale;
	y = dy * scale;
	W = dW * scale;
	H = dH * scale;

	disabled = 0;

	// back up
	originalscreen = create_bitmap_ex(bitmap_color_depth(screenreserve), W, H);
	blit(screenreserve, originalscreen, x, y, 0, 0, W, H);

	// used for drawing
	drawarea = create_bitmap_ex(bitmap_color_depth(screenreserve), W, H);
	
	backgr = 0;
	if (backgrname)
	{
		backgr = bmp(backgrname);//find_datafile_bmp(datafile, backgrname);
		if (!backgr)
		{
			tw_error("AreaReserve: No background defined !!");
		}
		else
		{
			stretch_blit(backgr, drawarea, 0, 0, backgr->w, backgr->h, 0, 0, W, H);
		}
	}

	if (!backgr)
	{
		backgr = create_bitmap_ex(bitmap_color_depth(screenreserve), W, H);
		clear_to_color(backgr, makecol(110,220,0));
	}

	
	
	blit(backgr, drawarea, x, y, 0, 0, W, H);

	Nareas = 0;


}
// ok, this provides a working space



AreaReserve::~AreaReserve()
{
	if (originalscreen)	destroy_bitmap(originalscreen);
	if (drawarea)		destroy_bitmap(drawarea);
	if (backgr)			destroy_bitmap(backgr);
	unload_datafile(datafile);
}


void AreaReserve::add(AreaGeneral *newarea)
{
	area[Nareas] = newarea;
	++Nareas;
	if (Nareas >= max_area)
	{
		tw_error("Error: too many sub-area requests");
	}
}




void AreaReserve::calculate()
{

	if (disabled)
		return;

	mouse.bmp.restore();	// resets the mouse pointer.
	mouse.update();			// stores current mouse settings (x,y,x,buttons)

	if (mouse.xpos() < x || mouse.xpos() > x+W || mouse.ypos() < y || mouse.ypos() > y+H)
	{
		if (hasfocus)
		{
			for (int i = 0; i < Nareas; ++i )
				area[i]->handle_menu_focus_loss();
		}

		hasfocus = false;
		return;			// check if the mouse is over this area
	}

	hasfocus = true;

	// make position relative in the reserve box area: 0,0 = top left of the draw_area
	// since from now on, all calculations are done there.
	mouse.pos.move(-x, -y);

	for (int i = 0; i < Nareas; ++i )
		area[i]->calculate();


	return;
}


void AreaReserve::animate()
{

	scare_mouse();	// otherwise it'll leave artifacts when things are drawn.

	// draw the background
	blit(backgr, drawarea, 0, 0, 0, 0, W, H);

	// draw the buttons
	for (int i = 0; i < Nareas; ++i )
		area[i]->animate();

	// update the main screen
	blit(drawarea, screenreserve, 0, 0, x, y, W, H);

	unscare_mouse();
}





bool AreaGeneral::hasmouse()
{
	// this depends on the particular way an area is defined.
	return false;
}



void AreaGeneral::init(AreaReserve *menu, char *identbranch)
{

	menu->add(this);

	// let the area know to what collection it belongs
	areareserve = menu;

	strcpy(ident, areareserve->ident);
	strcat(ident, identbranch);

	selected = false;
}


void AreaGeneral::update_mouse()
{
	mouse.copyinfo(&areareserve->mouse);
}

void AreaGeneral::check_focus()
{
	if (hasmouse())
	{

		lastfocus = focus;
		focus = 1;

		if (focus != lastfocus)
			handle_focus();
	}
}

void AreaGeneral::check_defocus()
{
	if (!hasmouse())
	{

		lastfocus = focus;
		focus = 0;

		if (focus != lastfocus)
			handle_defocus();
	}
}




// This routine is very important, since it can deliver an instant up-date
// when the menu as a whole has suddenly lost focus (mouse can move away in
// an undetected period of time, eg. an independent game loop), and the usual
// calls to calculate aren't made...
void AreaGeneral::handle_menu_focus_loss()
{
	selected = 0;
}

void AreaGeneral::calculate()
{

	// copy information from the areareserve manager
	update_mouse();

	check_focus();
	check_defocus();

	// check if button 1 (left) releases -> clicks for this item
	if (focus)
	{

		if (mouse.left.press())
		{
			handle_lpress();
		}

		if (mouse.left.release() && focus)
		{
			handle_lrelease();
		}

		if (mouse.right.press())
		{
			handle_rpress();
		}

		// change the mouse pointer if needed, if it has the mouse:
		if (lastfocus == 0)
			mouse.bmp.set();
		// note, "restore" is done by default by the area manager; this sets
		// it back to the custom mouse pointer, if needed.

	}


	if (focus && mouse.left.hold())
		selected = true;
	else
		selected = false;
	

}



void AreaGeneral::animate()
{
	
	// check if button 1 (left) releases -> clicks for this item
	// unconditional drawing:
	if (focus)
	{
		if (mouse.left.hold())
			draw_selected();
		else
			draw_focus();

	} else
		draw_defocus();
	
}


void AreaGeneral::draw_defocus()
{
}

void AreaGeneral::draw_focus()
{
}

void AreaGeneral::draw_selected()
{
}









// that was the general part ; now a real rectangular area.
// (similarly, you could define triangular and other types
// of areas).


void AreaBox::init(AreaReserve *menu, char *identbranch, int ax, int ay, int aW, int aH)
{
	AreaGeneral::init(menu, identbranch);

	x = ax * areareserve->scale;
	y = ay * areareserve->scale;
	W = aW * areareserve->scale;
	H = aH * areareserve->scale;

}




// check a square area to see if it has the mouse on it.

bool AreaBox::hasmouse()
{
	if (mouse.xpos() >= x && mouse.xpos() < x+W &&
		mouse.ypos() >= y && mouse.ypos() < y+H )
		return true;
	else
		return false;
}





