////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

#include "../include/MASkinG/combobox.h"


void MAS::ComboButton::SetOverlay(int ovr) {
	overlay = ovr;
}


void MAS::ComboButton::Draw(Bitmap &canvas) {
	Button::Draw(canvas);
	Bitmap ovr = skin->GetBitmap(overlay);
	int state = Disabled() ? 2 : (Selected() ? 1 : (HasFocus() ? 3 : 0));
	ovr.MaskedBlit(canvas, 0, state*ovr.h()/4, (w() - ovr.w())/2, (h() - ovr.h()/4)/2, ovr.w(), ovr.h()/4);
	
	//canvas.DrawSprite(ovr, (w() - ovr.w())/2, (h() - ovr.h()/4)/2);
}


MAS::ComboBox::ComboBox() : MAS::Dialog() {
	Add(editBox);
	Add(button);
	Add(list);
	list.Hide();
	list.h(80);
}


void MAS::ComboBox::MsgInitSkin() {
	Dialog::MsgInitSkin();
	
	button.SetOverlay(Skin::COMBO_BUTTON);
	list.SetAnimationProperties(skin->comboAnimationLength, skin->comboAnimationType);
	button.SetSample(Skin::SAMPLE_OPEN, Skin::SAMPLE_ACTIVATE);
}


void MAS::ComboBox::UpdateSize() {
	Bitmap bmp = skin->GetBitmap(Skin::COMBO_BUTTON);
	if (!bmp) return;
	if (list.Hidden()) {
		button.Shape(w() - bmp.w(), 0, bmp.w(), h());
		editBox.Resize(w() - bmp.w(), h());
	}
	else {
		button.Shape(w() - bmp.w(), 0, bmp.w(), h() - list.h());
		editBox.Resize(w() - bmp.w(), h() - list.h());
	}
	if (!list.GetSelectedItem()) {
		editBox.SetText(NULL);
	}
	else {
		editBox.SetText(list.GetSelectedItem()->GetText());
	}
	list.Place(0, editBox.h());
	list.w(w());
	Dialog::UpdateSize();
}


void MAS::ComboBox::Setup(int x, int y, int w, int h, int key, int flags, int i) {
	Dialog::Setup(x, y, w, h, key, flags);
	if (i<0) i = 0; else if (i>=list.Size()) i=list.Size()-1;
	list.Select(i);
	MAS::ListItem *li = list.GetItem(i);
	if (li) {
		editBox.Setup(0, 0, w, h, key, flags, li->GetText());
	}
	else {
		editBox.Setup(0, 0, w, h, key, flags, NULL);
	}
	list.w(w);
}


void MAS::ComboBox::HandleEvent(Widget& obj, int msg, int arg1, int arg2) {
	if (msg == MSG_ACTIVATE) {
		if (obj == button) {
			if (list.Hidden()) {
				ShowList();
			}
			else {
				HideList();
			}
		}
		else if (obj == list) {
			editBox.SetText(list.GetSelectedItem()->GetText());
			HideList();
			parent->HandleEvent(*this, MSG_ACTIVATE, arg1);
		}
		else if (obj == editBox) {
			if (!list.Hidden()) {
				HideList();
			}
			parent->HandleEvent(*this, MSG_ACTIVATE, list.Selection());
		}
	}
	else if (msg == MSG_LOSTMOUSE) {
		Point mp = GetMousePos() + GetOffset();
		if (!(mp <= *this) && !(mp <= list)) {
			HideList();
		}
	}
}


void MAS::ComboBox::HideList() {
	button.SetSample(Skin::SAMPLE_OPEN, Skin::SAMPLE_ACTIVATE);
	list.ResetAnimation();
	list.Hide();
	h(editBox.h());
	Root()->Redraw();
}


void MAS::ComboBox::ShowList() {
	button.SetSample(Skin::SAMPLE_ACTIVATE, Skin::SAMPLE_ACTIVATE);
	h(editBox.h() + list.h());
	list.Animate();
	list.Unhide();
	//parent->BringToTop(*this);
	Redraw();
}
