/* $Id$ */ 
/*
Twgui: GPL license - Rob Devilee, 2004.
*/


#include <allegro.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


#include "twbutton.h"
#include "twpopup.h"

#include "utils.h"




// here, the background is initialized (by mainwindow)
Popup::Popup(char *ident, int xcenter, int ycenter, BITMAP *outputscreen)
:
TWindow(ident, xcenter, ycenter, outputscreen)
{
	movingthereserve = false;

	if (!(xcenter == -1 && ycenter == -1))
		center(xcenter, ycenter);		// center around this position, in relative coordinates

	returnvalueready = false;
}

Popup::~Popup()
{
}


void Popup::doneinit()
{
	TWindow::doneinit();

	hide();
}


// this calls close with return value
// (the general close is ok, it closes and hides the menu).
void Popup::check_end()
{
	// nothing
}


void Popup::calculate()
{
	// must be called before the "return", otherwise focus-loss isn't called
	TWindow::calculate();

	if (disabled)
		return;

	// also, check if the mouse is clicked, and dragged - in that case, move the menu:
	// but, only if it's clicked in a part that is not a button ;)
	
	if ( Tmouse.left.press() && grabbedmouse )
	{
		EmptyButton *current;
		current = button_first;
		while (current)
		{
			if (current->hasmouse())//flag.focus)
				break;

			current = current->next;
		}

		// if there are no buttons that have the focus of the mouse, then movement is allowed
		if ( !current )
			movingthereserve = true;
	}

	if (!(Tmouse.left.hold() || Tmouse.left.press()) )
		movingthereserve = false;


	if ( movingthereserve )
	{
		int dx, dy;
		dx = Tmouse.vx();
		dy = Tmouse.vy();
		x += dx;
		y += dy;

		// update the relative mouse position also
		mpos.x = Tmouse.pos.x - x;
		mpos.y = Tmouse.pos.y - y;

	}
	
	check_end();
}




void Popup::close(int areturnstatus)
{
	hide();

	returnstatus = areturnstatus;
	returnvalueready = true;
	grabbedmouse = false;
}


// call this once (true) to probe if the popup has completed its function; it
// resets its "ready" status as well.
bool Popup::ready()
{
	if (returnvalueready)
	{
		returnvalueready = false;
		return true;
	} else
		return false;
}

// call this to get a int-value from some selection mechanism (eg a list)
int Popup::getvalue()
{
//	if (!returnvalueready)
//		return -1;				// otherwise it's undefined

	ready();
	return returnstatus;
}


// restore the ability to do calculations and actions
void Popup::enable()
{
	TWindow::enable();
	returnvalueready = false;
	grabbedmouse = false;
}



void Popup::newscan()
{
	show();	// (more general than enable)
	ready();
	grabbedmouse = false;
}





















// This popup is invoked by a push-button somewhere. I'm not going to create some
// special push button type for that, I can just as well let this (new) class
// access the state of that general button type.

PopupT::PopupT(char *identbranch, int axshift, int ayshift, BITMAP *outputscreen)
:
Popup(identbranch, axshift, ayshift, outputscreen)
{
	trigger = 0;

	init_components(identbranch);

	xshift = round(axshift * scale);
	yshift = round(ayshift * scale);
}

PopupT::PopupT(EmptyButton *atrigger, char *identbranch, int axshift, int ayshift)
:
Popup(identbranch,
			axshift, ayshift,
			atrigger->mainwindow->twscreen)
{
	trigger = atrigger;

	init_components(identbranch);

	xshift = round(axshift * scale);
	yshift = round(ayshift * scale);
}



void PopupT::init_components(char *id)
{
	returnvalueready = false;

	// and default option settings:
	option.disable_othermenu = true;
	option.place_relative2mouse = true;
	option.place_relative2window = false;

	close_on_defocus = true;
}


PopupT::~PopupT()
{
	/*
	if (left)
		delete left;
	if (right)
		delete right;
	if (up)
		delete up;
	if (down)
		delete down;

	if (scrollhor)
		delete scrollhor;
	if (scrollvert)
		delete scrollvert;
	if (scroll_control)
		delete scroll_control;
		*/

}


void PopupT::calculate()
{
	// must be called before the "return", otherwise focus-loss isn't called
	Popup::calculate();

	// first, do some tests for activation, that should always be done.
	// if you press the trigger, this menu is activated
	// (if a trigger isn't needed, but some more forcefull external control,
	// trigger==0)
	if ( trigger && disabled && trigger->flag.left_mouse_press )
	{
		if (option.disable_othermenu)
		{
			trigger->mainwindow->disable();
			trigger->mainwindow->handle_focus_loss();
		}
		this->show();		// this also resets the mouse position and old position, so that it has
							// the most recent value (since it hasn't been updated by the
							// calculate funtion for a while).
		//this->focus();

		// but where exactly : near the mouse !!
		if (option.place_relative2mouse)
		{
			x = round(trigger->mainwindow->x + trigger->mainwindow->mpos.x + xshift);
			y = round(trigger->mainwindow->y + trigger->mainwindow->mpos.y + yshift);
		} else if (option.place_relative2window) {
			x = trigger->mainwindow->x + xshift;
			y = trigger->mainwindow->y + yshift;
		}
		// otherwise don't do anything, just leave the window there.

		// don't let the menu go off-screen:
		
		if ( x+W > twscreen->w )
			x = twscreen->w - W;
		if ( x < 0 )
			x = 0;
		
		if ( y+H > twscreen->h )
			y = twscreen->h - H;
		if ( y < 0 )
			y = 0;
		
		
	}

	if (disabled)
		return;



	// must be called before the "return", otherwise focus-loss isn't called
//	mainwindow::calculate();



	check_end();
}


// if you leave the area, it's auto-hiding -- but only if you're not keeping the
// left-key pressed, which means you're moving the thing - when you move *fast*, the
// mouse pointer can exit the area before the area follows the mouse ...

void PopupT::handle_focus_loss()
{
	/*
	if (!close_on_defocus)
		return;
	
	if (Tmouse.left.hold())
	{
		hasfocus = true;
		return;
	}
	*/

//	check_end();
}

void PopupT::close(int areturnstatus)
{
	if (trigger)
	{
		// give back focus/control to the window that called this window.
		trigger->mainwindow->show();
		//trigger->mainwindow->focus();
	}

	Popup::handle_focus_loss();
	this->hide();

	returnstatus = areturnstatus;
	returnvalueready = true;
}


// you can (and should) replace this routine with one suited for your own purposes
void PopupT::check_end()
{
//	if (!hidden && !hasfocus && close_on_defocus)	// to prevent this from being called twice, due to recursive call to handle_focus() in hide()
//	{
		//throw("PopupList : Losing focus !!");

//		close(-1);		// nothing ;)
//	}
}










