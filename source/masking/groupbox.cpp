////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

#include "../include/MASkinG/groupbox.h"


MAS::GroupBox::GroupBox() : Dialog() {
	Add(box);
	Add(title);
}


void MAS::GroupBox::MsgInitSkin() {
	Dialog::MsgInitSkin();

	box.SetBitmap(Skin::BOX);

	for (int i=0; i<4; i++) {
		title.SetFontColor(skin->fcol[Skin::INFO_BOX][i], skin->scol[Skin::INFO_BOX][i], i);
		title.SetFont(skin->fnt[Skin::INFO_BOX][i], i);
	}
	title.SetTextMode(skin->boxBack);
	title.SetAlignment(skin->boxAlign);
}


void MAS::GroupBox::SetTitle(const char *t) {
	title.SetText(t);
	UpdateSize();
}


void MAS::GroupBox::UpdateSize() {
	box.Resize(w(), h());

	switch (skin->boxAlign) {
		case 0: title.Place(skin->boxX, skin->boxY);						break;
		case 1: title.Place(w() - skin->boxX - title.w(), skin->boxY);		break;
		case 2:	title.Place((w() - title.w())/2 + skin->boxX, skin->boxY);	break;
	};

	Dialog::UpdateSize();
}
