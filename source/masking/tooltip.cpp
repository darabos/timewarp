////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

//#define DEBUGMODE
#include "../include/MASkinG/tooltip.h"
#include "../include/MASkinG/settings.h"
#include "../include/MASkinG/mouse.h"
#include "../include/MASkinG/dialog.h"


static int state = 0;

MAS::Tooltip::Tooltip()
	:Widget(),
	counter(0),
	hideCounter(0),
	backCol(Color(255,255,192)),
	borderCol(Color::black),
	lastMousePos(Point(-1,-1)),
	text(NULL),
	lastWidget(NULL)
{
}


void MAS::Tooltip::MsgInitSkin() {
	Widget::MsgInitSkin();
	for (int i=0; i<4; i++) {
		SetFontColor(skin->fcol[Skin::INFO_TOOLTIP][i], skin->scol[Skin::INFO_TOOLTIP][i], i);
		SetFont(skin->fnt[Skin::INFO_TOOLTIP][i], i);
	}
	SetTextMode(-1);
	SetBackColor(skin->tooltipBack);
	SetBorderColor(skin->tooltipBorder);
	SetAnimationProperties(skin->tooltipAnimationLength, skin->tooltipAnimationType);
}


bool MAS::Tooltip::MsgWantmouse() {
	Widget::MsgWantmouse();
	return false;
}


void MAS::Tooltip::Draw(Bitmap &canvas) {
	// get the Tooltip colors and font
	Color fg = GetFontColor(0);
	Color bg = GetShadowColor(0);
	Font f = GetFont(0);

	// Draw the Tooltip
	canvas.Clear(backCol);
	canvas.Rectangle(0, 0, w()-1, h()-1, borderCol);
	
	f.BoxPrint(canvas, text ? text : "wtf?", fg, bg, -1, 0, 0, w(), h(), 2, 2);
}


void MAS::Tooltip::MsgTick() {
	Widget::MsgTick();

	// states:
	//  0 - waiting for mouse to come on top of a widget that wants to display a tooltip
	//  1 - waiting for mouse to be still for a while
	//  2 - waiting for mouse to leave widgets that have tooltip text or for time to run out
	//  3 - counting down until we can hide the tooltip
	switch (state) {
		case 0: {
			if (lastWidget != GetParent()->GetMouseObject()) {
				lastWidget = GetParent()->GetMouseObject();
				if (lastWidget && lastWidget->GetTooltipText()) {
					state = 1;
					counter = 0;
					lastMousePos = GetParent()->GetMouse()->pos;
				}
			}
		}
		break;
		
		case 1: {
			Point mPos = GetParent()->GetMouse()->pos;
			lastWidget = GetParent()->GetMouseObject();
			if (mPos == lastMousePos) {
				++counter;
				// if mouse has not moved for while -> show the tooltip
				if (counter >= Settings::logicFrameRate*Settings::tooltipDelay/1000) {
					// if the current mouse widget has some tooltip text, show the tooltip
					if (lastWidget && lastWidget->GetTooltipText()) {
						// make sure to display the right text at the right coordinates
						ShowTooltip();
						
						state = 2;
						hideCounter = 0;
					}
				}
			}
			else {
				lastMousePos = mPos;
				counter = 0;
			}
		}
		break;
		
		case 2: {
			// see if the time ran out
			if (Settings::tooltipAutohideDelay > 0) {
				++hideCounter;
				if (hideCounter >= Settings::logicFrameRate*Settings::tooltipAutohideDelay/1000) {
					state = 3;
					hideCounter = 0;
				}
			}

			// see if the mouse widget changed; if it has and it wants to show a tooltip,
			// abort autohiding and show the tooltip otherwise hide it
			if (lastWidget != GetParent()->GetMouseObject()) {
				lastWidget = GetParent()->GetMouseObject();
				lastMousePos = GetParent()->GetMouse()->pos;

				if (lastWidget && lastWidget->GetTooltipText()) {
					ShowTooltip();
					hideCounter = 0;
					state = 2;
				}
				else {
					state = 3;
				}
			}
		}
		break;
		
		case 3: {
			// see if the mouse came on top of another widget that wants to display a tooltip;
			// if it has, then abort hiding and show the tooltip otherwise wait for time to run out
			Widget *mWidget = GetParent()->GetMouseObject();
			lastMousePos = GetParent()->GetMouse()->pos;
			if (mWidget && mWidget != lastWidget && mWidget->GetTooltipText()) {
				lastWidget = mWidget;
				ShowTooltip();
				state = 2;
				hideCounter = 0;
			}
			else {
				// see if time ran out
				--counter;
				if (counter <= 0) {
					HideTooltip();
					lastWidget = mWidget;
				}
			}
		}
		break;
	};
	
	// any and all mouse clicks hide the tooltip
	if (!Hidden() && GetParent()->GetMouse()->flags) {
		HideTooltip();
	}
}


void MAS::Tooltip::SetBackColor(const MAS::Color &c) {
	backCol = c;
	Redraw();
}


void MAS::Tooltip::SetBorderColor(const MAS::Color &c) {
	borderCol = c;
	Redraw();
}


void MAS::Tooltip::SetText(const char *text) {
	if (this->text) {
		delete [] this->text;
		this->text = NULL;
	}
	
	if (text) {
		this->text = new char[ustrsizez(text)];
		ustrcpy(this->text, text);
	}
}


void MAS::Tooltip::ShowTooltip() {
	if (!lastWidget) return;

	// set the text
	const char *text = lastWidget->GetTooltipText();
	if (!text) return;
	SetText(text);

	// get size and position
	CalculateGeometry();

	if (Hidden()) {
		// bring it to top just in case
		GetParent()->BringToTop(*this);
	}
	else {
		GetParent()->Redraw();
	}
	
	// show it
	Unhide();
	Animate();
}


void MAS::Tooltip::HideTooltip() {
	counter = 0;
	Hide();
	ResetAnimation();
	GetParent()->Redraw();
	state = 0;
	lastWidget = GetParent()->GetMouseObject();
}


void MAS::Tooltip::CalculateGeometry() {
	CalculateSize();
	CalculatePosition();
}


void MAS::Tooltip::CalculatePosition() {
	int ww = w();
	int hh = h();
	
	int xx = lastMousePos.x() - GetParent()->x();
	if (xx + ww >= SCREEN_W) {
		xx = SCREEN_W - ww;
	}
	
	int yy = lastMousePos.y() - GetParent()->y() + 20;
	if (yy + hh >= SCREEN_H) {
		yy = lastMousePos.y() - GetParent()->y() - hh - 8;
	}
	
	// setup the tooltip
	Place(xx, yy);
}


void MAS::Tooltip::CalculateSize() {
	Font f = GetFont(0);
	int ww = f.GUITextLength(text) + 12;
	int hh = f.TextHeight() + 6;
	if (ww > SCREEN_W/2) {
		do {
			ww /= 2;
			hh = f.CountLines(text, ww - 12)*f.TextHeight() + 6;
		}
		while (ww/hh > 4);
	}
	
	Resize(ww, hh);
}
