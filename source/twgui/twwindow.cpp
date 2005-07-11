/* $Id$ */ 
/*
Twgui: GPL license - Rob Devilee, 2004.
*/

#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../scp.h"

#include "utils.h"

#include "twwindow.h"

#include "utils.h"

/*
// this destroys the bitmap (if it exists i.e. isn't set to 0),
// and resets the pointer to 0
void destroy_bmp(BITMAP **bmp)
{
	if (*bmp)
		destroy_bmp(*bmp);
	*bmp = 0;
}
*/




// make a backup of the old screen
// note, it's scaled to maximum W,H=1.0, 1.0 being the screen width
// default_W is the screen width for which all buttons in the menu are developed, and
// this is used to scale them to other screen resolutions.
TWindow::TWindow(char *identbase, int dx, int dy, BITMAP *outputscreen, bool vidwin)
{
	bufferedmode = false;

	prev = 0;
	next = 0;

	button_first = 0;
	button_last = 0;
	button_focus = 0;

	grabbedmouse = false;
	mpos = 0;

	// from the scaled thing, calculate the actual size on the current screen

	twscreen = outputscreen;
	if (!twscreen)
	{
		twgui_error("Something is wrong with the screen!!");
	}

	strcpy(ident, identbase);
//	strcat(ident, "_DAT");

	// from this, derive certain file names which you (may) need:
	char backgrname[128], mousename[128], infoname[128];
	strcpy(backgrname, ident);

	strcat(backgrname, "/backgr");
	strcpy(mousename, ident);
	strcat(mousename, "/mouse");
	strcpy(infoname, ident);
	strcat(infoname, "/info.txt");
	

//	datafile = load_datafile(datafilename);
//	if (!datafile)
//	{
//		twgui_error("TWindow: Could not load datafile");
//	}

	// read the default resolution from the info.txt file :
//	DATAFILE *infodat = find_datafile_object(datafile, infoname);
//	set_config_data( (char*) infodat->dat, infodat->size);
	strncpy(configfilename, infoname, sizeof(configfilename));
	set_config_file( configfilename );

	default_W = get_config_int(0, "res", 800);
	autoplace = get_config_int(0, "autoplace", 1);
	backgr_masked = get_config_int(0, "masked", 0);
	use_rle = get_config_int(0, "userle", 1);
	//default_W = def_W;
	scale = double(twscreen->w) / double(default_W);

	int locx, locy;
	locx = get_config_int(0, "locx", 0);
	locy = get_config_int(0, "locy", 0);
	

	disabled = false;
	hidden = false;


//	backgr_masked = check_mask(backgr); it's better to have this as a menu option.

	backgr = 0;
	backgr_rle = 0;

	if (use_rle)
	{
		backgr_rle = load_rle_data(backgrname);

		// check if this cached rle data has the same screen size of not (the screen size could be changed by the user).
		if ( (backgr_rle) && (backgr_rle->w != twscreen->w) )
		{
			destroy_rle(&backgr_rle);
		}

		if (!backgr_rle)
		{
			// also, convert bitmaps to more efficient rle sprites, and store it on disk
			backgr = bmp(backgrname);

			backgr_rle = get_rle_sprite(backgr);
			save_rle_data(backgr_rle, backgrname);
			destroy_bmp(&backgr);
		}
		// and what, if the rle sprite has wrong color depth: then it must be updated, too.
		if (backgr_rle->color_depth != bitmap_color_depth(outputscreen))
		{
			backgr_rle = get_rle_sprite(backgr);
			save_rle_data(backgr_rle, backgrname);
		}

	}

	if (!backgr_rle)
	{
		// this is the background, in scaled form
		backgr = bmp(backgrname, vidwin);
	}

	// for "automatic placement" purposes, which can be used if the "default buttons"
	// are an exact copy of the "background" image (you can design a menu and the buttons
	// by drawing (or copy/paste) them on a background. In the game, the separate buttons
	// will have more or less (?) identical size, and overwrite the background image.

	char check_char[128];
	strcpy(check_char, "none");
	backgr_forsearch = 0;
	if (autoplace != 0)
	{
		//BITMAP *b;
		strcpy(check_char, backgrname);
		strcat(check_char, ".bmp");

		//b = (BITMAP*) find_datafile_object(datafile, check_char)->dat;
		backgr_forsearch = load_bitmap(check_char, 0);
		//backgr_forsearch = (BITMAP*) create_bitmap_ex(bitmap_color_depth(b), b->w, b->h);
		//blit(b, backgr_forsearch, 0, 0, 0, 0, b->w, b->h);
	}


	if (dx == -1 && dy == -1)
	{
		x = locx;
		y = locy;
	} else {
		x = round(dx * scale);
		y = round(dy * scale);
	}

	if (backgr)
	{
		W = backgr->w;	// background is already scaled !!
		H = backgr->h;
	} else if (backgr_rle) {

		W = backgr_rle->w;	// background is already scaled !!
		H = backgr_rle->h;
	} else {
		twgui_error("TWindow: No background defined !!");
	}


	// back up ... well, in case of a changing background, this is hardly useful ...
	//originalscreen = create_bitmap_ex(bitmap_color_depth(twscreen), W, H);
	//blit(screen, originalscreen, x, y, 0, 0, W, H);

	// used for drawing

	drawarea = 0;

	if (bufferedmode)
	{
		if (vidwin)
			drawarea = create_video_bmp(W, H);
		
		if (!drawarea)
			drawarea = create_bitmap_ex(bitmap_color_depth(twscreen), W, H);
	} else {
		handle_unbufferedarea();
	}


	if (is_same_bitmap(backgr, drawarea) || !(backgr || backgr_rle) || (!drawarea) || !twscreen)
	{
		twgui_error("oh my!");
	}

	//Nareas = 0;


	// also, initialize default mouse ptr for use inside the menu area, and for
	// all the items:
//	Tmouse.bmp.init(bmp(mousename));

//	clear_keys();

	menu_starttime = twgui_time();
	menu_time = menu_starttime;

	exclusive = false;
	layer = 0;

	
}
// ok, this provides a working space


TWindow::~TWindow()
{
	// remove this window item from the list of windows
	// also, delete other windows that are in the same list.
	if (prev)
	{
		prev->next = 0;	// so that "prev" doesn't know "this" exists
		delete prev;
	}
	if (next)
	{
		next->prev = 0;	// so that "next" doesn't know "this" exists
		delete next;
	}

	//if (originalscreen)	destroy_bmp(originalscreen);
	destroy_bmp(&drawarea);
	destroy_bmp(&backgr);

	if (backgr_rle)
	{
		destroy_rle_sprite(backgr_rle);
		backgr_rle = 0;
	}

	// of course ;) but it's worth checking again, since it's not required to call this,
	// it's optional
	doneinit();	// (NOT tree_doneinit, since you only want to get done with this window)

	//destroy_bmp(backgr_forsearch);
	//if (datafile)		unload_datafile(datafile);

	// delete associated buttons
	EmptyButton *button;
	button = button_first;
	while (button)
	{
		EmptyButton *b;
		b = button;
		button = button->next;
		if (button)
			button->prev = 0;	// to avoid disaster as the next button is deleted...

		delete b;
	}

	set_config_file( configfilename );
	set_config_int(0, "locx", x);
	set_config_int(0, "locy", y);
}






// list functions, to handle a set of related windows. You just have to call one
// of them, and all of them will be handled.

// it's not really a tree, but a list .. oh well.

TWindow *TWindow::tree_root()
{
	TWindow *current;
	current = this;
	while (current->prev)
		current = current->prev;

	return current;
}


TWindow *TWindow::tree_last()
{
	TWindow *current;
	current = this;
	while (current->next)
		current = current->next;

	return current;
}



void TWindow::tree_calculate()
{
	TWindow *current;
	current = tree_root();

	// but first, update the keys and the mouse

	poll_keyboard();
	keyhandler.update();

	poll_mouse();
	Tmouse.update();

	// if this window is "exclusive", other windows cannot be accessed while this one has focus
	if (current->exclusive && !current->disabled)
	{
		current->calculate();
		return;
	}

	while (current)
	{
		current->calculate();
		current = current->next;
	}


}

void TWindow::tree_animate()
{
	TWindow *current;
	current = tree_last();

	// the first one should plot on top, and therefore, should be plotted last ...

	acquire_bitmap(twscreen);
	
	while (current)
	{
		current->animate();
		current = current->prev;
	}

	show_mouse(twscreen);

	release_bitmap(twscreen);
}


void TWindow::focus()
{
	// no need if this already is the root ... cause then it already has the focus.
	if (!prev)
		return;

	TWindow *root;
	root = tree_root();

	// check layers, you're not allowed to focus in a "lower" layer
	while (root && root->layer < layer)
		root = root->next;

	if (!root || root == this)		// apparently you're alone/ already focus in your own layer ...
		return;

	// remove "this" from the list
	if (prev)
		prev->next = next;
	if (next)
		next->prev = prev;
	

	// insert "this" at the "front"
	prev = root->prev;
	next = root;

	if (prev)
		prev->next = this;
	if (next)
		next->prev = this;

	// defocus the thing that just lost focus...
	if (next)
		next->handle_focus_loss();


	/*
	// show this (for debugging) in a file
	FILE *f = fopen("log.txt", "a+");
	if (!f)
		return;

	root = tree_root();

	fprintf(f, "=========\n");
	while (root)
	{
		fprintf(f, "%40s layer[%i] disabled[%i] grab[%i]\n",
			root->ident, root->layer, root->disabled, root->grabbedmouse);
		root = root->next;
	}

	fclose(f);
	//*/

}


void TWindow::tree_setscreen(BITMAP *scr)
{
	TWindow *current;
	current = tree_root();

	while (current)
	{
		current->setscreen(scr);
		current = current->next;
	}
}



// add a window at the end of the window list
void TWindow::add(TWindow *newwindow)
{
	TWindow *current;
	current = this;
	while (current->next)
		current = current->next;

	current->next = newwindow;
	newwindow->prev = current;
}


// call all the clean up routines for all the menus
void TWindow::tree_doneinit()
{
	TWindow *current;
	current = tree_root();

	while (current)
	{
		current->doneinit();
		current = current->next;
	}

}



bool TWindow::hasfocus()
{
	// only the root has primary focus...
	return (prev != 0);
}


// --------------------- DEFINING AREAS -------------------
// areas can be places where special actions can take place (graphically)
// but first, these places need to be defined, and their (functional) link
// to those possible actions needs to be shown.


// divide the screen in different areas, each of which has the basic properties
// of focus, defocus, and mouse status.

// this checks whether a area (usually a box) has focus or not depending on I/O (mouse, keyboard)


// a menu superstructure ... to manage a bunch of related areas ... in a reserved area/ region


BITMAP* TWindow::bmp(char *bmpname, bool vidmem)
{
	BITMAP *bmp, *tmpbmp;
	int bpp;

	bpp = bitmap_color_depth(twscreen);
	//bpp = 32;
	//bmp = find_datafile_bmp(datafile, bmpname);	// this adds _BMP

	char objname[128];

	if (strlen(ident) > 120)
	{
		twgui_error("string exceeds max length");
	}

	strcpy(objname, bmpname);

	char *tmp;
	tmp = &objname[strlen(objname)-4];
	if (strcmp(tmp, ".bmp"))	// if the extension isn't already .bmp
		strcat(objname, ".bmp");	// default extension for .bmp files.

	tmpbmp = load_bitmap(objname, 0);
	bmp = clone_bitmap(bpp, tmpbmp, scale, vidmem);
	destroy_bmp(&tmpbmp);

	return bmp;
}


RLE_SPRITE* TWindow::load_rle_data(char *filename)
{
	RLE_SPRITE *data;

	char objname[128];
	strcpy(objname, filename);
	strcat(objname, ".rledat");

	FILE *f;
	f = fopen(objname, "rb");
	if (!f)
		return 0;


	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	rewind(f);

	data = (RLE_SPRITE*) malloc(size);

	fread(data, 1, size, f);				// the header info (size)

	if (size != sizeof(RLE_SPRITE) + data->size)
		tw_error("Error restoring in load_rle_data: data corrupt.");

	fclose(f);

	return data;
}


void TWindow::save_rle_data(RLE_SPRITE *data, char *filename)
{
	char objname[128];
	strcpy(objname, filename);
	strcat(objname, ".rledat");

	FILE *f;
	f = fopen(objname, "wb");
	if (!f)
		return;

	fwrite(data, 1, sizeof(RLE_SPRITE) + data->size, f);
	fclose(f);
}


void TWindow::doneinit()
{
	//destroy_bmp(&backgr);	// no, keep the backgr around for other types of checks (mouse)

	destroy_bmp(&backgr_forsearch);
	//if (datafile)			unload_datafile(datafile);
}


void TWindow::setscreen(BITMAP *scr)
{
	twscreen = scr;
}



void TWindow::center(int xcenter, int ycenter)
{
	// move
	x = round(xcenter*scale) - W / 2;
	y = round(ycenter*scale) - H / 2;

}




void TWindow::center_abs(int xcenter, int ycenter)
{
	// move
	x = xcenter - W / 2;
	y = ycenter - H / 2;
}

void TWindow::center()
{
	ASSERT(twscreen)
	center(twscreen->w/2, twscreen->h/2);
}


// changes the "disabled" flag, and possibly does other stuff as well?
void TWindow::enable()
{
	disabled = false;
	grabbedmouse = false;
//	mouse.reset();		// reset the mouse to the newest values.
}

void TWindow::disable()
{
	if (!disabled)				// to prevent possible recursive call (disable can be called within focus loss in some cases...)
	{
		disabled = true;
		handle_focus_loss();	// disables other flags, also of sub-menus
		//grabbedmouse = false;
	}

}


void TWindow::show()
{
	hidden = false;	// allow animation
	enable();		// allow calculation
	focus();		// bring this window to the front of the list, so that it draws on top of the rest
}

void TWindow::hide()
{
	hidden = true;
	disable();
}



void TWindow::add(EmptyButton *newbutton)
{
	if (!button_first)
		button_first = newbutton;
		
	if (button_last)
		button_last->next = newbutton;
	

	newbutton->prev = button_last;
	button_last = newbutton;

	if (button_last->next != 0)
		tw_error("unknow button list error (already added?)");
}


void TWindow::rem(EmptyButton *newbutton)
{
	if (! newbutton)
		return;

	EmptyButton *nb;
	nb = newbutton;

	if (button_first == nb)
		button_first = nb->next;

	if (button_last == nb)
		button_last = nb->prev;

	if (nb->prev)
		nb->prev->next = nb->next;

	if (nb->next)
		nb->next->prev = nb->prev;

	//delete *newbutton;
}





bool TWindow::checkmouse()
{


	// If the mouse hasn't been "grabbed" by a window above this window
	
	int xrel, yrel;
	
	// this test is performed in relative coordinates (with respect to the (0,0) corner
	// of the reserved area).
	xrel = round(mpos.x);
	yrel = round(mpos.y);
	
	
	// if the mouse is off-window, but the button remains pressed, the mouse
	// still belongs to the window that it was clicked on... this has to be
	// checked first. You've to take into account layering, because of which you can't
	// assume that the first window in the row always is the one that's last been active.
	if (Tmouse.left.hold())
	{
		if (grabbedmouse)
			return true;
		else
			return false;

		// next, check if the mouse still points on the area of this window
	} else if (	xrel >= 0 && xrel < W &&
				yrel >= 0 && yrel < H )
		//xxx && getpixel(backgr, xrel, yrel) != makecol(255,0,255) ) {   should I restore this??
	{

		return true;
		// previous tests failed, so this window has lost the mouse
	} else {
		return false;
	}
		

}



void TWindow::setfocus(EmptyButton *newbutton)
{
	if (button_focus)
		button_focus->handle_defocus();

	button_focus = newbutton;
	if (button_focus)
		button_focus->handle_focus();
	// this is (or can be) called from within a button::handle_focus() subroutine,
	// therefore don't call that subroutine here.
}


void TWindow::calculate()
{
	// mousealreadygrabbed occurs, when any window on top has grabbed the mouse - this
	// means, that the current window can't grab it anymore. This extra parameter is
	// needed for recursive disabling of mouses
	if (prev && (prev->mousealreadygrabbed || prev->grabbedmouse))
	{
		mousealreadygrabbed = true;
	} else
		mousealreadygrabbed = false;
	// you've to check that all the time, cause it's inherited along the tree.
	

	if (disabled)
	{
		grabbedmouse = false;	// just to be sure, that if a window is "closed", its value is reset

		/* already done.
		// inherit some important information (which is needed in comparisons)
		if (prev && (prev->mousealreadygrabbed || prev->grabbedmouse))
			mousealreadygrabbed = true;
		else
			mousealreadygrabbed = false;
			*/

		return;
	}

//	mouse.bmp.restore();	// resets the mouse pointer.
//	mouse.update();			// stores current mouse settings (x,y,wheel,buttons)

//	add_keys();

	update_time();


	// position relative in the reserve box area: 0,0 = top left of the draw_area

	mpos.x = Tmouse.pos.x - x;
	mpos.y = Tmouse.pos.y - y;


	bool oldgrab = grabbedmouse;

	if (mousealreadygrabbed)
		grabbedmouse = false;
	else
		grabbedmouse = checkmouse();

	if (grabbedmouse && !oldgrab)
		handle_focus();

	else if (!grabbedmouse && oldgrab)
		handle_focus_loss();


	// if there's no contact with the mouse, then you don't have to calculate anything
	// either, I suppose ... also, the keys should only apply to the focused window,
	// that would be logical or not ?
	// Except the shortcut key for a particular button perhaps ... hmm, difficult.

	if (prev && !grabbedmouse)	// neither focused nor a mouse on top of it.
		return;


	// check the focused button
	if (keyhandler.keyhit[KEY_TAB])
	{
		EmptyButton *lastfocus;
		lastfocus = button_focus;

		if (!keyhandler.keynew[KEY_LSHIFT])
		{
			if (!lastfocus)
				lastfocus = button_last;

			if (!button_focus)
				button_focus = button_first;

			while (button_focus)
			{
				button_focus = button_focus->next;
				if (!button_focus)
					button_focus = button_first;

				if (button_focus == lastfocus || !button_focus->passive)
					break;
			}


		} else {

			if (!lastfocus)
				lastfocus = button_first;

			if (!button_focus)
				button_focus = button_last;

			while (button_focus)
			{
				button_focus = button_focus->prev;
				if (!button_focus)
					button_focus = button_last;

				if (button_focus == lastfocus || !button_focus->passive)
					break;
			}

		}

		// if you've changed buttons by tabbing, then let the old button
		// lose its focus, and let the new button gain focus.
		if (lastfocus != button_focus)
		{
			if (lastfocus)
				lastfocus->handle_defocus();

			if (button_focus)
				button_focus->handle_focus();
		}


		// you can also focus to another window, if you press control
		if (keyhandler.keynew[KEY_LCONTROL])
		{
			if (next)
			{
				next->focus();
				keyhandler.update();
				Tmouse.update();
				// otherwise the next in line of calculations brings the following one to
				// focus as well ... so better to refreshen all keys and the mouse, and
				// start completely fresh for the new window.
			}
		}


	}

	// calculate all the buttons
	EmptyButton *button;
	button = button_first;
	while (button)
	{
		button->calculate();
		button = button->next;
	}

	return;
}



void TWindow::handle_unbufferedarea()
{
	if (bufferedmode)
		return;

	if (prevx != x || prevy != y || !drawarea)
	{
		prevx = x;
		prevy = y;

		// allegro note: "the origin point must lie within the parent region."
		// this poses a problem if x<0 or y<0 or so !!
		int ix, iy, iw, ih;
		ix = x;
		iy = y;
		iw = W;
		ih = H;

		// handle negative (off-screen) coordinates: this means, less of the screen bitmap
		// needs to be used.
		if (ix < 0)
		{
			iw += ix;
			ixoffs = ix;	// remember the part that is off-bitmap
			ix = 0;
		}

		if (iy < 0)
		{
			ih += iy;
			iyoffs = iy;	// remember the part that is off-bitmap
			iy = 0;
		}

		// off-screen
		if (iw <= 0 || ih <= 0)
			return;

		if (drawarea)
			destroy_bmp(&drawarea);

		drawarea = create_sub_bitmap(twscreen, ix, iy, iw, ih);

		if (!drawarea)
		{
			tw_error("Error in reassigning menu subscreen");
		}
	}
}


void TWindow::animate()
{

	if (hidden)
		return;

	scare_mouse();	// otherwise it'll leave artifacts when things are drawn.


	// important, extra offset needed for drawing in unbuffered mode.
	// reason: stuff is drawn relative to the bitmap origin (locally) and if the
	// bitmap is only know partially, you still need to know the true origin...
	ixoffs = 0;
	iyoffs = 0;

	// move the drawarea along with the main screen, if the main screen moves!
	handle_unbufferedarea();

	if (!drawarea)
	{
		tw_error("No menu drawing area");
	}


	// release for in-game drawing
	acquire_bitmap(drawarea);
	
//	if (!disabled)
//	{
		
		// draw the background
		// also copy transparent color!

		// this takes about 1 ms on geo's comp if it's in video mem, but about 10 ms
		// if the bitmaps in normal memory ... which is a lot !! It takes ... ms for
		// a rle sprite.
		bool redraw_backgr = true;
		if (redraw_backgr)		// in principle only needed, if items on top move around.
		{
			if (backgr_rle)
				draw_rle_sprite(drawarea, backgr_rle, ixoffs, iyoffs);

			else
			{
				acquire_bitmap(backgr);
				blit(backgr, drawarea, 0,0, ixoffs, iyoffs, backgr->w, backgr->h);
				release_bitmap(backgr);
			}
		}

		// draw the buttons
		EmptyButton *button;
		button = button_first;
		int error_count = 0;
		while (button)
		{
			button->animate();
			button = button->next;

			++error_count;
			if (error_count > 50)
				tw_error("Infinite loop?");
		}


		// temporary ..
		//show_mouse(drawarea);
		
		// update the main screen
		// ignore transparent color
		if (bufferedmode && twscreen)
		{
			// this takes about 2 ms on geo's comp if it's in video mem
			masked_blit(drawarea, twscreen, 0, 0, x, y, W, H);
		}

//	} else {
//		if (bufferedmode)
//			masked_blit(drawarea, twscreen, 0, 0, x, y, W, H);
//		// otherwise, you just might get artefacts ... oh well.
//	}

	release_bitmap(drawarea);

	if (!bufferedmode)
		destroy_bmp(&drawarea);

	unscare_mouse();


}


void TWindow::handle_focus()
{
	// bring it first in the list ...
	focus();
}


void TWindow::handle_focus_loss()
{
	//for (int i = 0; i < Nareas; ++i )
	//	area[i]->handle_menu_focus_loss();
	EmptyButton *button;
	button = button_first;
	while (button)
	{
		button->handle_menu_focus_loss();
		button = button->next;
	}

	grabbedmouse = false;
}



void TWindow::scalepos(int *ax, int *ay)
{
	(*ax) = round( (*ax) * scale );
	(*ay) = round( (*ay) * scale );
}

void TWindow::scalepos(twguiVector *apos)
{
	(apos->x) = round( (apos->x) * scale );
	(apos->y) = round( (apos->y) * scale );
}



bool matchimage(BITMAP *backgr, BITMAP *foregr, int i, int j)
{
	if (!(backgr && foregr))
		return false;

	if (!(backgr->w && foregr->w))
		return false;

	if (!(backgr->h && foregr->h))
		return false;
	
	// check if the image block at this position, matches that of the background
	int c1 = 0, c2 = 0;
	int m = 0, n;
	
	for (n = 0; n < foregr->h; ++n)
	{
		for (m = 0; m < foregr->w; ++m)
		{
			c1 = getpixel(foregr, m, n);
			if (getr(c1) == 255 && getg(c1) == 0 && getb(c1) == 255)	// only compare the non-masked part
			{
				c2 = c1;
				continue;
			}
			
			c2 = getpixel(backgr, i+m, j+n);
			if (c2 != c1)
				break;
		}
		if (c2 != c1)
			break;
	}
	
	if (m == foregr->w && n == foregr->h)
		return true;
	else
		return false;
}


bool TWindow::search_bmp_location(BITMAP *bmp_default, twguiVector *apos)
{
	ASSERT(bmp_default);
	ASSERT(apos);
	
	if (!backgr_forsearch)
		return false;

	// first, test the "apos" position for a match.
	if (matchimage(backgr_forsearch, bmp_default, round(apos->x), round(apos->y)))
		return true;	// return without changing the position...

	// search by comparing pixels :
	int i, j;
	for (j = 0; j < backgr_forsearch->h; ++j)
	{
		for (i = 0; i < backgr_forsearch->w; ++i)
		{
			if (matchimage(backgr_forsearch, bmp_default, i, j))
			{
				(apos->x) = i;
				(apos->y) = j;
				return true;
			}

			/*
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
				(apos->x) = i;
				(apos->y) = j;
				return true;
			}
			*/

		}
	}

	return false;
}




/*
void TWindow::focus2other(TWindow *other)
{
	disable();			// this menu won't generate signals now, but it'll be drawn still
	other->show();
	other->prev = this;
}

void TWindow::back2other()
{
	prev->enable();			// switch control back to the meleemenu
	hide();					// this menu is disabled, and won't be drawn
	prev = 0;
}
*/


void TWindow::update_time()
{
	menu_time = twgui_time() - menu_starttime;
}

/*
void TWindow::clear_keys()
{
	keybuff_count = 0;
}

void TWindow::add_key(int akey)
{
	if (disabled)
		return;

	if (keybuff_count < 128)
	{
		keybuff[keybuff_count] = akey;
		++keybuff_count;
	}

}

void TWindow::add_keys()
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

*/



