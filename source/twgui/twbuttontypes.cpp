
#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
#include "../melee/mview.h"
REGISTER_FILE

#include "twwindow.h"
#include "twbuttontypes.h"


// to implement a Button, you add bitmaps-feedback to the box-area control
// name#default.bmp
// name#focus.bmp
// name#selected.bmp


Button::Button(TWindow *menu, char *identbranch, int asciicode, bool akeepkey)
:
GraphicButton(menu, identbranch, asciicode, akeepkey)
{
	init_pos_size(&bmp_default, "default");

	bmp_focus = getbmp("focus");
	bmp_selected = getbmp("selected");
}


Button::~Button()
{
	if (bmp_default)
		destroy_bitmap(bmp_default);
	if (bmp_focus)
		destroy_bitmap(bmp_focus);
	if (bmp_selected)
		destroy_bitmap(bmp_selected);
}


// this is the default drawing (at rest):
void Button::draw_default()
{
	draw(bmp_default);
}

void Button::draw_focus()
{
	if (!draw(bmp_focus))
	{
		draw_default();
		draw_boundaries(bmp_default);
	}
}

void Button::draw_selected()
{
	if (!draw(bmp_selected))
		draw_default();
}


bool Button::hasmouse()
{
	return GraphicButton::hasmouse(bmp_default);
}


bool Button::isvalid()
{
	return bmp_default != 0;
};




Area::Area(TWindow *menu, char *identbranch, int asciicode, bool akeepkey)
:
GraphicButton(menu, identbranch, asciicode, akeepkey)
{
	init_pos_size(&backgr, "backgr");
}


Area::~Area()
{
	if (backgr)
		destroy_bitmap(backgr);
}

void Area::changebackgr(char *fname)
{
	BITMAP *newb;
	newb = getbmp(fname);

	if (newb)
	{
		destroy_bitmap(backgr);
		backgr = newb;
	}
}


void Area::animate()
{
	draw(backgr);
}


bool Area::hasmouse()
{
	// the first rough check whether it's in the boxed bitmap area
	return GraphicButton::hasmouse(backgr);
}


bool Area::isvalid()
{
	return backgr != 0;
};



// an additional class, which has its own background and drawing area, which
// can be used to create custom representations of information (eg., text or smaller
// bitmaps))
// name#backgr.bmp



AreaTablet::AreaTablet(TWindow *menu, char *identbranch, int asciicode, bool akeepkey)
:
Area(menu, identbranch, asciicode, akeepkey)
{

	//init_pos_size(&backgr, "backgr");

	if (size.x != 0)
		drawarea = create_bitmap_ex(bitmap_color_depth(mainwindow->drawarea), size.x, size.y);
	else
		drawarea = 0;

}


AreaTablet::~AreaTablet()
{
	//if (backgr)
	//	destroy_bitmap(backgr);
	if (drawarea)
		destroy_bitmap(drawarea);
}


void AreaTablet::animate()
{
	blit(backgr, drawarea, 0, 0, 0, 0, size.x, size.y);

	subanimate();

	draw(drawarea);
//	draw(backgr);
}


void AreaTablet::subanimate()
{
	// nothing; you can put extra drawing commands here, stuff that's drawn onto
	// the background before being blitted onto the reserved area.
}


bool AreaTablet::hasmouse()
{
	// the first rough check whether it's in the boxed bitmap area
	return GraphicButton::hasmouse(backgr);
}


bool AreaTablet::isvalid()
{
	return backgr != 0;
};





// A button which can be in 2 states (on/off)
// name#on.bmp
// name#off.bmp


SwitchButton::SwitchButton(TWindow *menu, char *identbranch, int asciicode, bool initialstate)
:
GraphicButton(menu, identbranch, asciicode)
{
	init_pos_size(&bmp_on, "on");
	bmp_off = getbmp("off");

	state = initialstate;
}

SwitchButton::~SwitchButton()
{
	if (bmp_on)
		destroy_bitmap(bmp_on);
	if (bmp_off)
		destroy_bitmap(bmp_off);
}

void SwitchButton::draw_default()
{
	if (state)
		draw(bmp_on);
	else
		draw(bmp_off);
}

void SwitchButton::draw_focus()
{
	draw_default();
	draw_rect_fancy();
}

// is the same as focus, cause a switch cannot be selected all the time !!
void SwitchButton::draw_selected()
{
	draw_focus();
}


void SwitchButton::calculate()
{
	GraphicButton::calculate();

	// determine if the state of the button is being changed by (some) interaction:
	if (flag.left_mouse_press)
		state = !state;	// switch state.
}


bool SwitchButton::hasmouse()
{
	// the first rough check whether it's in the boxed bitmap area
	return GraphicButton::hasmouse(bmp_on);
}


bool SwitchButton::isvalid()
{
	return bmp_on != 0;
};






GhostButton::GhostButton(TWindow *menu)
:
EmptyButton(menu)
{
	passive = true;
}
GhostButton::~GhostButton()
{
}





ScrollBar::ScrollBar(TWindow *menu, char *identbranch)
:
AreaTablet(menu, identbranch, 255)
{
	relpos = 0.0;	// between 0 and 1

	button = getbmp("button");

	if (button)
	{
		bwhalf = button->w/2; 
		bhhalf = button->h/2;
	} else {
		bwhalf = 0;
		bhhalf = 0;
	}

	if (size.y >= size.x)
		direction = ver;	// vertically oriented
	else
		direction = hor;	// horizontally oriented

	if (direction == ver)
	{
		pmin = bhhalf;
		pmax = size.y - bhhalf;
	} else {
		pmin = bwhalf;
		pmax = size.x - bwhalf;
	}

	pbutton = pmin;
}


ScrollBar::~ScrollBar()
{
	if (button)
		destroy_bitmap(button);
}


void ScrollBar::handle_lhold()
{
	if (direction == ver)
		pbutton = mainwindow->mpos.y - pos.y;		// mouse pos relative in the little bar area
	else
		pbutton = mainwindow->mpos.x - pos.x;
	
	if (pbutton < pmin)
		pbutton = pmin;
	
	if (pbutton > pmax)
		pbutton = pmax;
	
	relpos = double(pbutton - pmin) / double(pmax - pmin);
}


void ScrollBar::subanimate()
{
	AreaTablet::subanimate();

	if (direction == ver)
		masked_blit(button, drawarea, 0, 0, size.x/2 - bwhalf, pbutton-bhhalf, button->w, button->h);
	else
		masked_blit(button, drawarea, 0, 0, pbutton-bwhalf, size.y/2 - bhhalf, button->w, button->h);
}


void ScrollBar::setrelpos(double arelpos)
{
	if (relpos == arelpos)
		return;

	relpos = arelpos;

	// also update the button position to reflect this change
	pbutton = pmin + iround(relpos * (pmax - pmin));
}


void ScrollBar::calculate()
{
	AreaTablet::calculate();
}




