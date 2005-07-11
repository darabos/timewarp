/* $Id$ */ 
/*
Twgui: GPL license - Rob Devilee, 2004.
*/

#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../scp.h"


#include "twwindow.h"
#include "twbuttontypes.h"

#include "utils.h"

// to implement a Button, you add bitmaps-feedback to the box-area control
// name#default.bmp
// name#focus.bmp
// name#selected.bmp


Button::Button(TWindow *menu, char *identbranch, int asciicode, bool akeepkey)
:
GraphicButton(menu, identbranch, asciicode, akeepkey)
{
	init_pos_size(&bmp_default, "default");

	bmp_focus = getrle("focus");
	bmp_selected = getrle("selected");
}


Button::~Button()
{
	destroy_rle(&bmp_default);
	destroy_rle(&bmp_focus);
	destroy_rle(&bmp_selected);
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
		//draw_boundaries(bmp_default);
		draw_rect_fancy();
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

	markfordeletion = true;

	masked = false;

	W = size.x;
	H = size.y;

	/*
	buffered = false;		// TESTING ONLY FOR NOW !!
	if (!buffered)
	{
		destroy_bmp(&backgr);
		check_unbuffered_bmp();
	}
	*/

	// also, adjust the target backgr bitmap : erase (make transparent so that it's not drawn) this
	// part of the bitmap !! Cause, this part is redrawn entirely using this SubArea !!
	//rectfill(mainwindow->backgr, pos.x, pos.y, pos.x+W-1, pos.y+H-1, 0);
	// no: this is not compatible with rle sprites.
}


Area::~Area()
{
	if (markfordeletion)
		destroy_rle(&backgr);
}

void Area::changebackgr(char *fname)
{
	RLE_SPRITE *newb;
	//newb = getbmp(fname);
	newb = getrle_nobutton(fname);

	if (newb)
	{
		if (markfordeletion)
			destroy_rle(&backgr);

		backgr = newb;
		markfordeletion = true;	// locally initialized, hence locally destroyed...
	}
}


void Area::changebackgr(RLE_SPRITE *newb)
{
	if (newb)
	{
		if (markfordeletion)
			destroy_rle(&backgr);		// hmm, well, don't do this, leave that to the program that created it !!
		
		backgr = newb;
		markfordeletion = false;	// not locally initialized, hence not locally destroyed...
	}
}

/*
void Area::overwritebackgr(BITMAP *newb, double scale, int col)
{
	if (newb && backgr)
	{
		clear_to_color(backgr, col);
		stretch_blit(newb, backgr, 0, 0, newb->w, newb->h,
			0, 0, round(newb->w * scale), round(newb->h * scale));
	}
}
*/


void Area::animate()
{
//	if (buffered)
//	{
		draw(backgr);
//	} else {
//		// well, otherwise there's no drawing operation - you just have to take care
//		// that the bitmap positions are correct (for external drawing operations!!)
//		check_unbuffered_bmp();
//	}
}


void Area::check_unbuffered_bmp()
{
	/*
	if (backgr)
	{
		if (!buffered)
			destroy_bmp(&backgr);
	}

	if (!backgr)
	{
		backgr = create_sub_bitmap(mainwindow->drawarea, round(pos.x), round(pos.y),
			W, H);
	}
	*/
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











AreaDraw::AreaDraw(TWindow *menu, char *identbranch, int asciicode, bool akeepkey)
:
Area(menu, identbranch, asciicode, akeepkey)
{
	Area::backgr = 0;	// that it another backgr.
	backgrdraw = 0;

	init_pos_size(&backgrdraw, "backgr");

	markfordeletion = true;

	masked = false;

	W = size.x;
	H = size.y;

	/*
	buffered = false;		// TESTING ONLY FOR NOW !!
	if (!buffered)
	{
		destroy_bmp(&backgr);
		check_unbuffered_bmp();
	}
	*/

	// also, adjust the target backgr bitmap : erase (make transparent so that it's not drawn) this
	// part of the bitmap !! Cause, this part is redrawn entirely using this SubArea !!
	//rectfill(mainwindow->backgr, pos.x, pos.y, pos.x+W-1, pos.y+H-1, 0);
	// no: this is not compatible with rle sprites.
}


AreaDraw::~AreaDraw()
{
	if (markfordeletion)
		destroy_bmp(&backgrdraw);
}


void AreaDraw::animate()
{
	draw(backgrdraw);
}


bool AreaDraw::hasmouse()
{
	// the first rough check whether it's in the boxed bitmap area
	return GraphicButton::hasmouse(backgrdraw);
}


bool AreaDraw::isvalid()
{
	return backgrdraw != 0;
};













SubArea::SubArea(TWindow *menu, char *identbranch, int asciicode, bool akeepkey)
:
Area(menu, identbranch, asciicode, akeepkey)
{
	//init_pos_size(&backgr, "backgr");
	//destroy_bmp(&backgr);

	init_pos_size("backgr");
	backgr = 0;

	W = size.x;
	H = size.y;

	reset_backgr();

	Area::backgr = 0;	// this is a different backgr !! So set it to 0.

	// also, adjust the target backgr bitmap : erase (make transparent so that it's not drawn) this
	// part of the bitmap !! Cause, this part is redrawn entirely using this SubArea !!
//	rectfill(mainwindow->backgr, pos.x, pos.y, pos.x+W, pos.y+H, MASK_COLOR_32);
}


void SubArea::reset_backgr()
{
	olddrawarea = mainwindow->backgr;
	if (mainwindow->backgr)
	{
		// draw directly on top of the drawing area ...
		destroy_bmp(&backgr);
		backgr = create_sub_bitmap(mainwindow->backgr, pos.x, pos.y, W, H);
	}
}

void SubArea::calculate()
{
	Area::calculate();

	if (!backgr || olddrawarea != mainwindow->backgr)
	{
		reset_backgr();
	}

}

void SubArea::animate()
{
	if (backgr)
	{
		//Area::animate();	no, cause that uses another backgr !!
		draw(backgr);
	}
}


bool SubArea::hasmouse()
{
	// the first rough check whether it's in the boxed bitmap area
	return GraphicButton::hasmouse(backgr);
}



SubArea::~SubArea()
{
	destroy_bmp(&backgr);
}



// an additional class, which has its own background and drawing area, which
// can be used to create custom representations of information (eg., text or smaller
// bitmaps))
// name#backgr.bmp



AreaTablet::AreaTablet(TWindow *menu, char *identbranch, int asciicode, bool akeepkey)
:
Area(menu, identbranch, asciicode, akeepkey)
{

	//init_pos_size(&backgr, "backgr");
	if (!mainwindow->twscreen)
	{
		drawarea = 0;
		return;
	}

	if (size.x != 0)
		drawarea = create_bitmap_ex(bitmap_color_depth(mainwindow->twscreen), round(size.x), round(size.y));
	else
		drawarea = 0;

}


AreaTablet::~AreaTablet()
{
	//if (backgr)
	//	destroy_bitmap(backgr);
	destroy_bmp(&drawarea);
}


void AreaTablet::animate()
{
	if (backgr && drawarea)
		draw_rle_sprite(drawarea, backgr, 0, 0);

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





/** A button which can be in 2 states (on/off) name#off.bmp name#on.bmp ; it is
initialized using the "off" state, so the off-state bmp should be shown on the background. */


SwitchButton::SwitchButton(TWindow *menu, char *identbranch, int asciicode, bool initialstate)
:
GraphicButton(menu, identbranch, asciicode)
{
	init_pos_size(&bmp_off, "off");
	bmp_on = getrle("on");

	state = initialstate;
}

SwitchButton::~SwitchButton()
{
	destroy_rle(&bmp_on);
	destroy_rle(&bmp_off);
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

	button = getrle("button");

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
		pmax = round(size.y - bhhalf);
	} else {
		pmin = bwhalf;
		pmax = round(size.x - bwhalf);
	}

	pbutton = pmin;
}


ScrollBar::~ScrollBar()
{
	destroy_rle(&button);
}


void ScrollBar::handle_lhold()
{
	if (direction == ver)
		pbutton = round(mainwindow->mpos.y - pos.y);		// mouse pos relative in the little bar area
	else
		pbutton = round(mainwindow->mpos.x - pos.x);
	
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
		//masked_blit(button, drawarea, 0, 0, round(size.x/2 - bwhalf), pbutton-bhhalf, button->w, button->h);
		draw_rle_sprite(drawarea, button, round(size.x/2 - bwhalf), pbutton-bhhalf);
	else
		//masked_blit(button, drawarea, 0, 0, pbutton-bwhalf, round(size.y/2 - bhhalf), button->w, button->h);
		draw_rle_sprite(drawarea, button, pbutton-bwhalf, round(size.y/2 - bhhalf));
}


void ScrollBar::setrelpos(double arelpos)
{
	if (relpos == arelpos)
		return;

	relpos = arelpos;

	// also update the button position to reflect this change
	pbutton = pmin + round(relpos * (pmax - pmin));
}


void ScrollBar::calculate()
{
	AreaTablet::calculate();
}












