////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

#include "../include/MASkinG/slider.h"


////////////////////////////////////////////////////////////////////////////////
// Sets the orientation and updates bitmap indices
//	0 - vertical
//	1 - horizontal
void MAS::Slider::SetOrientation(int o) {
	orientation = o;
	SetBitmap(o == 1 ? Skin::HSLIDER_BACK : Skin::VSLIDER_BACK,
			  o == 1 ? Skin::HSLIDER_GRIP : Skin::VSLIDER_GRIP);
}


void MAS::Slider::Setup(int x, int y, int w, int h, int key, int flags, int m, int M, int p, int o) {
	SetRange(m, M);
	Widget::Setup(x, y, w, h, key, flags);
	SetPosition(p);
	SetOrientation(o);
}


void MAS::Slider::SetBitmap(int back, int button) {
	bitmap[0] = back;
	bitmap[1] = button;
	if (orientation == 1) {
		h(skin->GetBitmap(back).h()/4);
	}
	else {
		w(skin->GetBitmap(back).w()/4);
	}
	Redraw();
}


void MAS::Slider::MsgInitSkin() {
	SetOrientation(orientation);

	// The samples
	SetSample(Skin::SAMPLE_GOTFOCUS, Skin::SAMPLE_GOTFOCUS);
	SetSample(Skin::SAMPLE_LOSTFOCUS, Skin::SAMPLE_LOSTFOCUS);
	SetSample(Skin::SAMPLE_SCROLL, Skin::SAMPLE_SCROLL);

	Widget::MsgInitSkin();
}


void MAS::Slider::Draw(Bitmap &canvas) {
	if (orientation == 1) {
		DrawHorizontalSlider(canvas);
	}
	else {
		DrawVerticalSlider(canvas);
	}
}


void MAS::Slider::DrawVerticalSlider(Bitmap &canvas) {
	Bitmap bb = skin->GetBitmap(bitmap[0]);
	Bitmap bg = skin->GetBitmap(bitmap[1]);

	// Get the subbitmap offset
	int state = Disabled() ? 2 : (Selected() ? 1 : (HasFocus() ? 3 : 0));

	// Draw the background
	bb.VerticalTile(canvas, state, 0, 0, h(), 4);

	// Possibly draw the dotted rectangle
	if (skin->drawDots && HasFocus()) {
		canvas.DrawDottedRect(0, 0, w()-1, h()-1, skin->c_font);
	}

	// Draw the gripper
	int xx = (w() - bg.w()/4)/2;
	int yy = (int)((GetPosition() - GetMin())*(h() - bg.h())/GetRange());
	bg.MaskedBlit(canvas, state*bg.w()/4, 0, xx, yy, bg.w()/4, bg.h());
}


void MAS::Slider::DrawHorizontalSlider(Bitmap &canvas) {
	Bitmap bb = skin->GetBitmap(bitmap[0]);
	Bitmap bg = skin->GetBitmap(bitmap[1]);

	// Get the subbitmap offset
	int state = Disabled() ? 2 : (Selected() ? 1 : (HasFocus() ? 3 : 0));

	// Draw the background
	bb.HorizontalTile(canvas, state, 0, 0, w(), 4);

	// Possibly draw the dotted rectangle
	if (skin->drawDots && HasFocus()) {
		canvas.DrawDottedRect(0, 0, w()-1, h()-1, skin->c_font);
	}

	// Draw the gripper
	int xx = (int)((GetPosition() - GetMin())*(w() - bg.w())/GetRange());
	int yy = (h() - bg.h()/4)/2;
	bg.MaskedBlit(canvas, 0, (int)(0.25*state*bg.h()), xx, yy, bg.w(), bg.h()/4);
}


void MAS::Slider::Toggle() {
	if (Selected())
		Deselect();
	else
		Select();
	
	Redraw();
	Animate();
}


void MAS::Slider::MsgLPress() {
	Bitmap bmp = skin->GetBitmap(bitmap[1]);
	Point mp = GetMousePos();
	double newpos = pos;
	if (orientation == 1) {
		int spos = (int)((GetPosition() - GetMin())*(w() - bmp.w())/GetRange());
		if (!(mp.x() >= spos && mp.x() < spos + bmp.w())) {
			newpos = GetRange()*(mp.x() - bmp.w()/2)/(w() - bmp.w()) + GetMin();
			offset = bmp.w()/2;
		}
		else {
			int xx = (int)((GetPosition() - GetMin())*(w() - bmp.w())/GetRange());
			offset = mp.x() - (xx + bmp.w()/2);
		}
	}
	else {
		int spos = (int)((GetPosition() - GetMin())*(h() - bmp.h())/GetRange());
		if (!(mp.y() >= spos && mp.y() < spos + bmp.h())) {
			newpos = GetRange()*(mp.y() - bmp.h()/2)/(h() - bmp.h()) + GetMin();
			offset = bmp.h()/2;
		}
		else {
			int yy = (int)((GetPosition() - GetMin())*(h() - bmp.h())/GetRange());
			offset = mp.y() - (yy + bmp.h()/2);
		}
	}

	SetPosition((int)newpos);
	Toggle();
	SetFlag(D_PRESSED);
	Redraw();
	
	Widget::MsgLPress();
}


void MAS::Slider::MsgLRelease() {
	if (Flags() & D_PRESSED) {
		Toggle();
		ClearFlag(D_PRESSED);
	}
	Widget::MsgLRelease();
}


void MAS::Slider::MsgMousemove(const Point &d) {
	if (Selected()) {
		Bitmap bmp = skin->GetBitmap(bitmap[1]);
		Point mp = GetMousePos();
		int oldpos = (int)pos;
		double newpos = pos;
		if (orientation == 1) {
			int xx = mp.x() - offset;
			newpos = GetRange()*(xx - bmp.w()/2)/(w() - bmp.w()) + GetMin();
			//newpos = GetRange()*(mp.x() - bmp.w()/2)/(w() - bmp.w()) + GetMin();
		}
		else {
			int yy = mp.y() - offset;
			newpos = GetRange()*(yy - bmp.h()/2)/(h() - bmp.h()) + GetMin();
			//newpos = GetRange()*(mp.y() - bmp.h()/2)/(h() - bmp.h()) + GetMin();
		}

		newpos = (newpos < min) ? min : ((newpos > max) ? max : newpos);
		if ((int)newpos != oldpos) {
			SetPosition((int)newpos);
			Redraw();
			PlaySample(Skin::SAMPLE_SCROLL);
			GetParent()->HandleEvent(*this, MSG_SCROLL, (int)newpos);
		}
	}
	
	Widget::MsgMousemove(d);
}


void MAS::Slider::MsgWheel(int z) {
	Widget::MsgWheel(z);

	int newpos = (int)GetPosition();
	int oldpos = newpos;
	
	newpos += (orientation==1 ? z : -z);

	// Clip the new position if necessary
	if (newpos < min) newpos = (int)min;
	if (newpos > max) newpos = (int)max;

	// Position changed
	if (newpos != oldpos) {
		SetPosition(newpos);
		Redraw();
		PlaySample(Skin::SAMPLE_SCROLL);
		GetParent()->HandleEvent(*this, MSG_SCROLL, newpos);
	}
}


bool MAS::Slider::MsgChar(int c) {
	Widget::MsgChar(c);
	
	int newpos = (int)GetPosition();
	int oldpos = newpos;
	int r = (int)GetRange();
	int cc = c>>8;

	int INC, DEC;
	if (orientation == 1) {
		INC = KEY_RIGHT;
		DEC = KEY_LEFT;
	}
	else {
		INC = KEY_DOWN;
		DEC = KEY_UP;
	}

	// Change the position
	if (cc == INC)		newpos++;			else
	if (cc == DEC)		newpos--;			else
	if (cc == KEY_HOME) newpos = (int)min;	else
	if (cc == KEY_END)	newpos = (int)max;	else
	if (cc == KEY_PGUP)	newpos += r/10;		else
	if (cc == KEY_PGDN)	newpos -= r/10;		else
	return false;

	// Clip the new position if necessary
	if (newpos < min) newpos = (int)min;
	if (newpos > max) newpos = (int)max;

	// Position changed
	if (newpos != oldpos) {
		SetPosition(newpos);
		Redraw();
		PlaySample(Skin::SAMPLE_SCROLL);
		GetParent()->HandleEvent(*this, MSG_SCROLL, newpos);
	}

	clear_keybuf();

	return true;
}


bool MAS::Slider::MsgWantfocus() {
	return true;
}
