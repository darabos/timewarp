
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
//#include "../../melee/mview.h"
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
	int bpp = bitmap_color_depth(screenreserve);
	//bpp = 32;
	BITMAP *bmp = find_datafile_bmp(datafile, bmpname);	// this adds _BMP
	return clone_bitmap(bpp, bmp, scale);
}

// make a backup of the old screen
// note, it's scaled to maximum W,H=1.0, 1.0 being the screen width
// default_W is the screen width for which all buttons in the menu are developed, and
// this is used to scale them to other screen resolutions.
AreaReserve::AreaReserve(char *identbase, int dx, int dy, char *datafilename, BITMAP *outputscreen)
{
	// from the scaled thing, calculate the actual size on the current screen

	screenreserve = outputscreen;
	if (!screenreserve)
	{
		tw_error("Something wrong with the screen!!");
	}

	strcpy(ident, identbase);
	strcat(ident, "_DAT");

	// from this, derive certain file names which you (may) need:
	char backgrname[128], mousename[128], infoname[128];
	strcpy(backgrname, ident);
	strcat(backgrname, "/BACKGR");
	strcpy(mousename, ident);
	strcat(mousename, "/MOUSE");
	strcpy(infoname, ident);
	strcat(infoname, "/INFO_TXT");
	

	datafile = load_datafile(datafilename);
	if (!datafile)
	{
		tw_error("AreaReserve: Could not load datafile");
	}

	// read the default resolution from the info.txt file :
	DATAFILE *infodat = find_datafile_object(datafile, infoname);
	set_config_data( (char*) infodat->dat, infodat->size);

	default_W = get_config_int(NULL, "res", 800);
	//default_W = def_W;
	scale = double(screen->w) / double(default_W);
	

	disabled = false;
	hidden = false;

	// this is the background, in scaled form
	backgr = bmp(backgrname);

	// for "automatic placement" purposes, which can be used if the "default buttons"
	// are an exact copy of the "background" image (you can design a menu and the buttons
	// by drawing (or copy/paste) them on a background. In the game, the separate buttons
	// will have more or less (?) identical size, and overwrite the background image.

	backgr_forsearch = 0;
	if (backgr)
	{
		BITMAP *b;
		char check_char[128];
		strcpy(check_char, backgrname);
		strcat(check_char, "_BMP");

		b = (BITMAP*) find_datafile_object(datafile, check_char)->dat;
		backgr_forsearch = (BITMAP*) create_bitmap_ex(bitmap_color_depth(b), b->w, b->h);
		blit(b, backgr_forsearch, 0, 0, 0, 0, b->w, b->h);
	}

	if (backgr)
	{
		x = iround(dx * scale);
		y = iround(dy * scale);
		W = backgr->w;	// background is already scaled !!
		H = backgr->h;
	} else {
		tw_error("AreaReserve: No background defined !!");
	}


	// back up ... well, in case of a changing background, this is hardly useful ...
	//originalscreen = create_bitmap_ex(bitmap_color_depth(screenreserve), W, H);
	//blit(screenreserve, originalscreen, x, y, 0, 0, W, H);

	// used for drawing
	drawarea = create_bitmap_ex(bitmap_color_depth(screenreserve), W, H);
	
	Nareas = 0;


	// also, initialize default mouse ptr for use inside the menu area, and for
	// all the items:
	mouse.bmp.init(bmp(mousename));

	clear_keys();

	menu_starttime = get_time();
	menu_time = menu_starttime;
}
// ok, this provides a working space


AreaReserve::~AreaReserve()
{
	//if (originalscreen)	destroy_bitmap(originalscreen);
	if (drawarea)		destroy_bitmap(drawarea);
	if (backgr)			destroy_bitmap(backgr);

	// of course ;) but it's worth checking again, since it's not required to call this,
	// it's optional
	doneinit();

	if (backgr_forsearch)	destroy_bitmap(backgr_forsearch);
	//if (datafile)		unload_datafile(datafile);

	// delete associated areas (deletions feed back through the remove call), so
	while (Nareas > 0)
	{
		delete area[0];
	}
}

void AreaReserve::doneinit()
{
	if (backgr_forsearch)	destroy_bitmap(backgr_forsearch);
	if (datafile)			unload_datafile(datafile);
}

void AreaReserve::setscreen(BITMAP *scr)
{
	screenreserve = scr;
}



void AreaReserve::center(int xcenter, int ycenter)
{
	// put back the background
	//blit(originalscreen, screenreserve, 0, 0, x, y, W, H);

	// move
	x = iround(xcenter*scale) - W / 2;
	y = iround(ycenter*scale) - H / 2;

	// read the new background
	//blit(screenreserve, originalscreen, x, y, 0, 0, W, H);
}


// changes the "disabled" flag, and possibly does other stuff as well?
void AreaReserve::enable()
{
	disabled = false;
	mouse.reset();		// reset the mouse to the newest values.
}

void AreaReserve::disable()
{
	if (!disabled)				// to prevent possible recursive call (disable can be called within focus loss in some cases...)
	{
		disabled = true;
		handle_focus_loss();	// disables other flags, also of sub-menus
	}

}


void AreaReserve::show()
{
	hidden = false;
	enable();
}

void AreaReserve::hide()
{
	hidden = true;
	disable();
	//blit(originalscreen, screenreserve, 0, 0, 0, 0, W, H);
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


void AreaReserve::remove(AreaGeneral *remarea)
{
	int i;
	for ( i = 0; i < Nareas; ++i )
	{
		if (area[i] == remarea)
			break;
	}

	if (i < Nareas)
	{
		int k;
		for ( k = i; k < Nareas-1; ++k )
			area[k] = area[k+1];

		--Nareas;

	} else {
		tw_error("AreaReserve::remove Could not remove an area !!");
	}

}


bool AreaReserve::hasmouse()
{
	int xrel, yrel;

	// this test is performed in relative coordinates (with respect to the (0,0) corner
	// of the reserved area).
	xrel = mouse.xpos();
	yrel = mouse.ypos();

	if (  xrel >= 0 && xrel < W && xrel >= 0 && yrel < H
		&& getpixel(backgr, xrel, yrel) != makecol(255,0,255) )
		return true;
	else
		return false;

}



void AreaReserve::calculate()
{

	if (disabled)
		return;

	mouse.bmp.restore();	// resets the mouse pointer.
	mouse.update();			// stores current mouse settings (x,y,wheel,buttons)

	add_keys();
	update_time();

	// if it has no mouse, and no keys, then :
	// (note, that this test is in absolute coordinates)
	if ( (mouse.xpos() < x || mouse.xpos() > x+W || mouse.ypos() < y || mouse.ypos() > y+H) &&
		 (keybuff_count == 0)
		 )
		 
	{
		if (hasfocus)	// if the mouse leaves the area, update all menu items to reflect this
		{
			hasfocus = false;
			handle_focus_loss();
		}

		return;			// check if the mouse is over this area
	}

	// otherwise, keys and/or mouse should be checked by the menu items:

	hasfocus = true;

	// make position relative in the reserve box area: 0,0 = top left of the draw_area
	// since from now on, all calculations are done there.
	mouse.move(-x, -y);

	for (int i = 0; i < Nareas; ++i )
		area[i]->calculate();

	return;
}



void AreaReserve::animate()
{

	if (hidden)
		return;

	scare_mouse();	// otherwise it'll leave artifacts when things are drawn.

	if (!disabled)
	{
		// draw the background
		// also copy transparent color!
		blit(backgr, drawarea, 0, 0, 0, 0, W, H);
	
		// draw the buttons
		for (int i = 0; i < Nareas; ++i )
			area[i]->animate();

		// update the main screen
		// ignore transparent color
		masked_blit(drawarea, screenreserve, 0, 0, x, y, W, H);
	} else
		//draw_lit_sprite(drawarea, screenreserve, x, y, makecol(100,100,100));
		masked_blit(drawarea, screenreserve, 0, 0, x, y, W, H);


	unscare_mouse();
}


void AreaReserve::handle_focus_loss()
{
	for (int i = 0; i < Nareas; ++i )
		area[i]->handle_menu_focus_loss();
}



void AreaReserve::scalepos(int *ax, int *ay)
{
	(*ax) = iround( (*ax) * scale );
	(*ay) = iround( (*ay) * scale );
}


bool AreaReserve::search_bmp_location(BITMAP *bmp_default, int *ax, int *ay)
{
	ASSERT(bmp_default);
	ASSERT(ax);
	ASSERT(ay);
	
	if (!backgr_forsearch)
		return false;

	// search by comparing pixels :
	int i, j;
	for (j = 0; j < backgr_forsearch->h; ++j)
	{
		for (i = 0; i < backgr_forsearch->w; ++i)
		{

			// check if the image block at this position, matches that of the background
			int c1 = 0, c2 = 0;
			int m = 0, n;

			ASSERT( bmp_default->h <= 0 );
			ASSERT( bmp_default->w <= 0 );

			for (n = 0; n < bmp_default->h; ++n)
			{
				for (m = 0; m < bmp_default->w; ++m)
				{
					c1 = getpixel(bmp_default, m, n);
					if (getr(c1) == 255 && getg(c1) == 0 && getb(c1) == 255)	// only compare the non-masked part
					{
						c2 = c1;
						continue;
					}

					c2 = getpixel(backgr_forsearch, i+m, j+n);
					if (c2 != c1)
						break;
				}
				if (c2 != c1)
					break;
			}

			if (m == bmp_default->w && n == bmp_default->h)
			{
				(*ax) = i;
				(*ay) = j;
				return true;
			}

		}
	}

	return false;
}





void AreaReserve::focus2other(AreaReserve *other)
{
	disable();			// this menu won't generate signals now, but it'll be drawn still
	other->show();
	other->prev = this;
}

void AreaReserve::back2other()
{
	prev->enable();			// switch control back to the meleemenu
	hide();					// this menu is disabled, and won't be drawn
	prev = 0;
}


void AreaReserve::update_time()
{
	menu_time = get_time() - menu_starttime;
}

void AreaReserve::clear_keys()
{
	keybuff_count = 0;
}

void AreaReserve::add_key(int akey)
{
	if (disabled)
		return;

	if (keybuff_count < 128)
	{
		keybuff[keybuff_count] = akey;
		++keybuff_count;
	}

}

void AreaReserve::add_keys()
{
	if (keyboard_needs_poll())
		poll_keyboard();

	clear_keys();

	// check all the "keys"
	int i;		// i = the scancode ;)
	for ( i = 0; i < KEY_MAX; ++i )
	{
		if (key[i])
		{
			// control overrules alt and shft
			if (key[KEY_LCONTROL] || key[KEY_RCONTROL])
				add_key( mapkey(i, KEY_LCONTROL) );

			// alt overrules shft
			else if (key[KEY_ALT] || key[KEY_ALTGR])
				add_key( mapkey(i, KEY_ALT) );
				
			// shift overrules a normal press
			else if (key[KEY_LSHIFT] || key[KEY_RSHIFT])
				add_key( mapkey(i, KEY_LSHIFT) );

			else
				add_key( mapkey(i, 0) );
			
		}
	}
}











bool AreaGeneral::hasmouse()
{
	// this depends on the particular way an area is defined.
	return false;
}



bool AreaGeneral::haskey()
{
	return flag.haskey;
}

bool AreaGeneral::haskeypress()
{
	return flag.haskeypress;
}



// this should be checked only once per iteration; that's the assumption
// made for the detection flag
void AreaGeneral::update_key()
{
	// check if there's a key-trigger:
	int i;
	bool foundkey = false;
	for (i = 0; i < areareserve->keybuff_count; ++i)
	{
		if ( (areareserve->keybuff[i]) == trigger_key )
		{
			foundkey = true;
		}
	}

	// this is usually the case.
	flag.haskey = false;
	flag.haskeypress = false;

	if ( foundkey )
	{

		// either a key-hold, or a key-press:
		//if (trigger_keepkey || ( (!trigger_keepkey) && (!trigger_keydetection) ))
		flag.haskey = true;

		if (!trigger_keydetection)
			flag.haskeypress = true;

		trigger_keydetection = true;	// the key is being held.

	} else
		trigger_keydetection = false;	// the key isn't being touched.

}


// another one, for a ghostbutton - that's a button-style thing that has all the
// variables, but not the graphics and mouse-sensitivity.

AreaGeneral::AreaGeneral(AreaReserve *menu)
{
	menu->add(this);
	areareserve = menu;
	selected = false;
}

AreaGeneral::AreaGeneral(AreaReserve *menu, char *identbranch, int asciicode, bool akeepkey)
{

	menu->add(this);

	trigger_key = asciicode;
	//trigger_keepkey = akeepkey;
	trigger_keydetection = false;

	// let the area know to what collection it belongs
	areareserve = menu;

	strcpy(ident, areareserve->ident);		// a data block
	strcat(ident, "/");
	strcat(ident, identbranch);				// stuff within the data block

	selected = false;


	//mouse.bmp.init(areareserve->bmp(strmouse));
	// use the default menu-mouse, defined for area-reserve:
	mouse.bmp.init(areareserve->mouse.bmp.newmousebmp);
	// (you can override this with a similar call but using another bitmap later on, if required).
}


// well ... not much, just remove it from the areareserve list.
AreaGeneral::~AreaGeneral()
{
	areareserve->remove(this);
}


void AreaGeneral::update_mouse()
{
	mouse.copyinfo(&areareserve->mouse);
}

void AreaGeneral::check_focus()
{
	if ( hasmouse() || haskey() )
	{

		flag.lastfocus = flag.focus;
		//focus = 1;
		flag.focus = true;

		if (flag.focus != flag.lastfocus)
			handle_focus();

	} else {

		flag.lastfocus = flag.focus;
		//focus = 0;
		flag.focus = false;

		if (flag.focus != flag.lastfocus)
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
	//update_key();
	flag.reset();
}

void AreaGeneral::calculate()
{

	// copy information from the areareserve manager
	update_mouse();
	update_key();

	check_focus();

	subcalculate();		// users can put stuff in here, which is then calculated just
	// before the mouse handles, but just after the mouse/ key updates.

	// check if button 1 (left) releases -> clicks for this item

	flag.left_mouse_press = false;
	flag.left_mouse_release = false;
	flag.right_mouse_press = false;
	flag.left_mouse_hold = false;

	if (flag.focus)
	{

		if ( mouse.left.press() || haskeypress() )
		{
			flag.left_mouse_press = true;
			handle_lpress();
		}

		if (mouse.left.hold() || haskey())
		{
			flag.left_mouse_hold = true;
			handle_lhold();
		}

		if (mouse.left.release() && flag.focus)
		{
			flag.left_mouse_release = true;
			handle_lrelease();
		}

		if (mouse.right.press())
		{
			flag.right_mouse_press = true;
			handle_rpress();
		}

		// change the mouse pointer if needed, if it has the mouse:
		mouse.bmp.set();
		// note, "restore" is done by default by the area manager; this sets
		// it back to the custom mouse pointer, if needed.

	}

}



void AreaGeneral::animate()
{
	
	// check if button 1 (left) releases -> clicks for this item
	// unconditional drawing:
	if (flag.focus)
	{
		if (flag.left_mouse_hold)
			draw_selected();
		else
			draw_focus();

	} else
		draw_default();
	
}


void AreaGeneral::draw_default()
{
}

void AreaGeneral::draw_focus()
{
}

void AreaGeneral::draw_selected()
{
}



AreaGeneral::flag_struct::flag_struct()
{
	// these flags are updated by the area's calculate function.
//	focus = 0;
//	left_mouse_press = left_mouse_release = left_mouse_hold = 0;
//	right_mouse_press = right_mouse_release = right_mouse_hold = 0;
	reset();
}


//AreaGeneral::flag_struct::reset() 
void AreaGeneral::flag_struct::reset() //added void 7/1/2003 Culture20
{
	// these flags are updated by the area's calculate function.
	focus = 0;
	left_mouse_press = left_mouse_release = left_mouse_hold = 0;
	right_mouse_press = right_mouse_release = right_mouse_hold = 0;
	haskey = haskeypress = 0;
}





// that was the general part ; now a real rectangular area.
// (similarly, you could define triangular and other types
// of areas).


AreaBox::AreaBox(AreaReserve *menu, char *identbranch, int ax, int ay, int asciicode, bool akeepkey)
:
AreaGeneral(menu, identbranch, asciicode, akeepkey)
{
//	AreaGeneral::init(menu, identbranch);

	x = ax;
	y = ay;
	areareserve->scalepos(&x, &y);
	//x = ax * areareserve->scale;
	//y = ay * areareserve->scale;
}

AreaBox::~AreaBox()
{
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





void AreaBox::draw_rect()
{
	BITMAP *b = areareserve->drawarea;
	int x1, y1, x2, y2;
	
	x1 = x;
	y1 = y;
	x2 = x + W - 1;
	y2 = y + H - 1;

	// draw something simple:

	int D = 4;	// width of the rectangle
	D *= iround(areareserve->scale - 1);
	if (D < 0)
		D = 0;

	while (D >= 0)
	{
		if (x2-x1 > 2*D && y2-y1 > 2*D)
			rect(b, x1+D, y1+D, x2-D, y2-D, makecol(200,200,200));
		--D;
	}
}


int rect_fancy_getcolor(double phase, double L, double Ltot)
{
	unsigned char r, g, b;
	
	r = (unsigned char)(128 + 127 * sin(phase + PI2 * L/Ltot));
	g = (unsigned char)(128 + 127 * sin(phase + 0.3*PI + PI2 * L/Ltot));
	b = (unsigned char)(128 + 127 * sin(phase + 0.7*PI + PI2 * L/Ltot));

	return makecol(r, g, b);
}


void AreaBox::draw_rect_fancy()
{
	BITMAP *b = areareserve->drawarea;
	int x1, y1, x2, y2;
	
	x1 = x;
	y1 = y;
	x2 = x + W - 1;
	y2 = y + H - 1;


	int D = 2;	// width of the rectangle
	D = iround(D * areareserve->scale) - 1;		// cause 0 also counts as 1...
	if (D < 0)
		D = 0;

	int ix, iy;


	double phase = areareserve->menu_time * 1E-3 * PI;	//offset angle

	int L;

	while (D >= 0)
	{
		double Ltotal = (W-2*D) * (H-2*D);

		if (Ltotal <= 0)
			break;

		L = 0;

		iy = y1 + D;
		ix = x1 + D - 1;

		while(ix < x2-D )
		{
			++ix;
			putpixel(b, ix, iy, rect_fancy_getcolor(phase, L, Ltotal));
			++L;
		}

		while(iy < y2-D )
		{
			++iy;
			putpixel(b, ix, iy, rect_fancy_getcolor(phase, L, Ltotal));
			++L;
		}

		while(ix > x1+D )
		{
			--ix;
			putpixel(b, ix, iy, rect_fancy_getcolor(phase, L, Ltotal));
			++L;
		}

		while(iy > y1+D+1 )
		{
			--iy;
			putpixel(b, ix, iy, rect_fancy_getcolor(phase, L, Ltotal));
			++L;
		}


		--D;
	}

}


void AreaBox::locate_by_backgr(char *stron)
{
	BITMAP *tmp;
	bool k;	
	char check_char[128];

	strcpy(check_char, stron);
	strcat(check_char, "_BMP");

	tmp = (BITMAP*) find_datafile_object(areareserve->datafile, check_char)->dat;
	if (!tmp)
	{
		tw_error("Could not find the comparison bmp in the datafile");
	}
	
	k = areareserve->search_bmp_location(tmp, &x, &y);
	areareserve->scalepos(&x, &y);		// is normally called by the other init();

	if (!k)
	{
		tw_error("Could not find the bmp on the background image");
	}
}



// obtain a bitmap, specific to this "object" :
BITMAP *AreaBox::getbmp(char *name)
{
	char streditbox[128];
	strcpy(streditbox,  ident);
	strcat(streditbox,  name);

	// a background image is needed of course.
	return areareserve->bmp(streditbox);
}







// an additional class, which has its own background and drawing area, which
// can be used to create custom representations of information (eg., text or smaller
// bitmaps))



AreaTablet::AreaTablet(AreaReserve *menu, char *identbranch, int ax, int ay, int asciicode, bool akeepkey)
:
AreaBox(menu, identbranch, ax, ay, asciicode, akeepkey)
{

	// (auto-) initialize the background and create a drawing area

	bool autoplace = false;
	if (ax == -1 && ay == -1)
		autoplace = true;

	initbackgr(autoplace);		// this also sets W and H.
								// and x,y in case of "autoplace"

	if (W != 0)
		drawarea = create_bitmap_ex(bitmap_color_depth(areareserve->drawarea), W, H);
	else
		drawarea = 0;

}


AreaTablet::~AreaTablet()
{
	if (backgr)
		destroy_bitmap(backgr);
	if (drawarea)
		destroy_bitmap(drawarea);
}


void AreaTablet::initbackgr(bool autoplace)
{
	backgr = getbmp("_BACKGR");
	
	if (backgr)
	{
		W = backgr->w;		// note: background is already scaled when it's initialized
		H = backgr->h;
	} else {
		//tw_error("Could not initialize background of matrix");
		W = 0;
		H = 0;
	}

	if (autoplace && W != 0)
	{
		char streditbox[128];
		strcpy(streditbox,  ident);
		strcat(streditbox,  "_BACKGR");

		locate_by_backgr(streditbox);
	}

}

void AreaTablet::animate()
{
	blit(backgr, drawarea, 0, 0, 0, 0, W, H);

	subanimate();

	blit(drawarea, areareserve->drawarea, 0, 0, x, y, W, H);
}


void AreaTablet::subanimate()
{
	// nothing; you can put extra drawing commands here, stuff that's drawn onto
	// the background before being blitted onto the reserved area.
}













WindowManagerElement::WindowManagerElement(WindowManagerElement *aprev,
										   AreaReserve *ap, WindowManagerElement *anext)
{
	prev = aprev;
	p = ap;
	next = anext;

	if (prev)				// update the link
		prev->next = this;
	if (next)
		next->prev = this;
}

WindowManagerElement::~WindowManagerElement()
{
	// move links around this element
	if (prev)
		prev->next = next;
	if (next)
		next->prev = prev;
}



WindowManager::WindowManager()
{
	head = 0;
	tail = 0;
}

WindowManager::~WindowManager()
{
	while (head)
		remove(head);
}

AreaReserve *WindowManager::first()
{
	iter = head;
	if (iter)
		return iter->p;
	else
		return 0;
}

AreaReserve *WindowManager::next()
{
	iter = iter->next;
	if (iter)
		return iter->p;
	else
		return 0;
}


void WindowManager::add(AreaReserve *newreserve)
{
		
	w = new WindowManagerElement(tail, newreserve, 0);

	if (!head)
		head = w;

	// added at the back
	tail = w;
}


void WindowManager::remove(WindowManagerElement *e)
{
	if ( e == NULL)
		return;

	head = e->next;
	tail = e->prev;

	// head = 0 and tail = 0 if this is the last element.

	delete e;
		
}




void WindowManager::calculate()
{
	
	for (a = first(); a != 0; a = next())
	{
		a->calculate();
	}
}

void WindowManager::animate()
{
	
	for (a = first(); a != 0; a = next())
	{
		a->animate();
	}
}

void WindowManager::setscreen(BITMAP *scr)
{
	for (a = first(); a != 0; a = next())
	{
		a->setscreen(scr);
	}
}


