////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

#include "../include/MASkinG/button.h"
#include "../include/MASkinG/settings.h"


MAS::Button::Button()
	:Label(),
	counter(0),
	spinCounter(0)
{
	ClearFlag(D_AUTOSIZE);
}


void MAS::Button::MsgStart() {
	counter = 0;
	spinCounter = 0;
	UpdateSize();
	Widget::MsgStart();
}


void MAS::Button::MsgInitSkin() {
	Widget::MsgInitSkin();
	for (int i=0; i<4; i++) {
		SetFontColor(skin->fcol[Skin::INFO_BUTTON][i], skin->scol[Skin::INFO_BUTTON][i], i);
		SetFont(skin->fnt[Skin::INFO_BUTTON][i], i);
	}
	SetTextMode(-1);
	SetBitmap(Skin::BUTTON);
	SetSample(Skin::SAMPLE_GOTFOCUS, Skin::SAMPLE_GOTFOCUS);
	SetSample(Skin::SAMPLE_LOSTFOCUS, Skin::SAMPLE_LOSTFOCUS);
	SetSample(Skin::SAMPLE_ACTIVATE, Skin::SAMPLE_ACTIVATE);
	SetAnimationProperties(skin->buttonAnimationLength, skin->buttonAnimationType);
}


void MAS::Button::Draw(Bitmap &canvas) {
	// get the button state
	int state = Disabled() ? 2 : (Selected() ? 1 : (HasFocus() ? 3 : 0));
	
	// get the button colors and font
	Color fg = GetFontColor(state);
	Color bg = GetShadowColor(state);
	Font f = GetFont(state);

	// update the size if necessary
	UpdateSize();
	
	// Draw the button
	Bitmap bmp = GetBitmap();
	if (bmp) {
		if (TestFlag(D_AUTOSIZE)) {
			bmp.MaskedBlit(canvas, 0, state*h(), 0, 0, w(), h());
		}
		else {
			bmp.MaskedTiledBlit(canvas, 0, state, 0, 0, w(), h(), 1, 4);
		}
	}

	if (text) {
		// Calculate the position of the text
		int text_x = w()/2 - 1;
		int text_y = (h() - f.TextHeight())/2 - 1;
	
		// Displace the text a little when the button is down
		if (state == 1) {
			text_x += skin->buttonDisplacement;
			text_y += skin->buttonDisplacement;
		}
	
		// Print the text
		f.GUITextoutEx(canvas, text, text_x, text_y, fg, bg, GetTextMode(), 2);
	}

	// draw the dotted rectangle
	if (skin->drawDots && HasFocus() && !TestFlag(D_AUTOSIZE)) {
		canvas.DrawDottedRect(3, 3, w()-3, h()-3, fg);
	}
}


void MAS::Button::Toggle() {
	if (Selected())
		Deselect();
	else
		Select();
	
	Redraw();
	Animate();
}



void MAS::Button::MsgGotfocus() {
	Widget::MsgGotfocus();
	PlaySample(Skin::SAMPLE_GOTFOCUS);
	Redraw();
	Animate();
}


void MAS::Button::MsgLostfocus() {
	Widget::MsgLostfocus();
	PlaySample(Skin::SAMPLE_LOSTFOCUS);
	Redraw();
	Animate();
}


void MAS::Button::MsgKey() {
	Widget::MsgKey();
	if (!counter) {
		Toggle();
		SetFlag(D_PRESSED);
	}
	counter = 150*MAS::Settings::logicFrameRate/1000;
}


void MAS::Button::MsgLPress() {
	Widget::MsgLPress();
	Toggle();
	SetFlag(D_PRESSED);
}


void MAS::Button::MsgLRelease() {
	Widget::MsgLRelease();

	if (HasMouse()) {
		if ((Flags() & D_PRESSED)) {
			ClearFlag(D_PRESSED);
			if (!TestFlag(D_TOGGLE)) {
				Toggle();
			}
			else {
				Redraw();
				Animate();
			}
			if (!TestFlag(D_SPINNER)) {
				PlaySample(Skin::SAMPLE_ACTIVATE);
				GetParent()->HandleEvent(*this, MSG_ACTIVATE);
			}
		}
	}
	else {
		Toggle();
		ClearFlag(D_PRESSED);
	}
}


bool MAS::Button::MsgWantfocus() {
	Widget::MsgWantfocus();
	return true;
}


void MAS::Button::Setup(int x, int y, int w, int h, int key, int flags, const char *text) {
	SetText(text);
	Widget::Setup(x, y, w, h, key, flags);
}


void MAS::Button::UpdateSize() {
	if (TestFlag(D_AUTOSIZE)) {
		Bitmap bmp = GetBitmap();
		if (bmp) {
			Resize(bmp.w(), bmp.h()/4);
		}
	}
}


void MAS::Button::MsgTick() {
	Widget::MsgTick();

	if (counter) {
		--counter;
		if (!counter) {
			ClearFlag(D_PRESSED);
			if (!TestFlag(D_TOGGLE)) {
				Toggle();
			}
			else {
				Redraw();
				Animate();
			}
			if (!TestFlag(D_SPINNER)) {
				PlaySample(Skin::SAMPLE_ACTIVATE);
				GetParent()->HandleEvent(*this, MSG_ACTIVATE);
			}
		}
	}

	if (TestFlag(D_SPINNER | D_PRESSED)) {
		if (spinCounter) {
			--spinCounter;
		}
		else {
			PlaySample(Skin::SAMPLE_ACTIVATE);
			GetParent()->HandleEvent(*this, MSG_ACTIVATE);
			spinCounter = 60*MAS::Settings::logicFrameRate/1000;
		}
	}
}
