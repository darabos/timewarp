////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

#include "../include/MASkinG/clearscreen.h"


void MAS::ClearScreen::MsgStart() {
	//Resize(parent->size());
	Shape(0, 0, 100, 100, true);
	Widget::MsgStart();
}


void MAS::ClearScreen::Draw(Bitmap &canvas) {
	canvas.Clear(col);
}


void MAS::ClearScreen::MsgInitSkin() {
	col = skin->c_back;
	Widget::MsgInitSkin();
}


void MAS::ClearScreen::SetColor(const Color& col) {
	this->col = col;
}


MAS::Color &MAS::ClearScreen::GetColor () {
	return col;
}
