////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

#include "../include/MASkinG/panel.h"


void MAS::Panel::MsgInitSkin() {
	SetBitmap(Skin::PANEL_RAISED);
	Widget::MsgInitSkin();
}


void MAS::Panel::Draw(Bitmap &canvas) {
	GetBitmap().TiledBlit(canvas, 0, 0, 0, 0, w(), h(), 1, 1);
}


void MAS::PanelRaised::MsgInitSkin() {
	SetBitmap(Skin::PANEL_RAISED);
	Widget::MsgInitSkin();
}


void MAS::PanelSunken::MsgInitSkin() {
	SetBitmap(Skin::PANEL_SUNKEN);
	Widget::MsgInitSkin();
}


void MAS::PanelRidge::MsgInitSkin() {
	SetBitmap(Skin::PANEL_RIDGE);
	Widget::MsgInitSkin();
}


void MAS::PanelGroove::MsgInitSkin() {
	SetBitmap(Skin::PANEL_GROOVE);
	Widget::MsgInitSkin();
}


void MAS::BoxShadow::MsgInitSkin() {
	SetBitmap(Skin::BOX_SHADOW);
	Widget::MsgInitSkin();
}
