////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

#include "../include/MASkinG/menu.h"
#include "../include/MASkinG/settings.h"


MAS::MenuItem::MenuItem(const char *txt, Menu *ch)
	:Widget(),
	text(NULL),
	child(ch),
	callbackID(-1),
	topBar(true),
	delayTimer(0),
	allocated(false)
{
	SetText(txt);
}


MAS::MenuItem::MenuItem(const char *txt, int id)
	:Widget(),
	text(NULL),
	child(NULL),
	callbackID(id),
	topBar(true),
	delayTimer(0),
	allocated(false)
{
	SetText(txt);
}


MAS::MenuItem::~MenuItem() {
	if (text) {
		delete [] text;
		text = NULL;
	}
	
	child = NULL;
}


void MAS::MenuItem::MsgInitSkin() {
	Widget::MsgInitSkin();
	for (int i=0; i<4; i++) {
		SetFontColor(skin->fcol[Skin::INFO_MENU][i], skin->scol[Skin::INFO_MENU][i], i);
		SetFont(skin->fnt[Skin::INFO_MENU][i], i);
	}
	SetTextMode(-1);
	SetSample(Skin::SAMPLE_GOTFOCUS, Skin::SAMPLE_GOTFOCUS);
	SetSample(Skin::SAMPLE_LOSTFOCUS, Skin::SAMPLE_LOSTFOCUS);
}


bool MAS::MenuItem::MsgWantfocus() {
	Widget::MsgWantfocus();
	return text ? true : false;
}


bool MAS::MenuItem::MsgWantTab() {
	Widget::MsgWantTab();
	return false;
}


void MAS::MenuItem::MsgGotfocus() {
	Widget::MsgGotfocus();
	delayTimer = 0;
}


void MAS::MenuItem::MsgLostfocus() {
	Widget::MsgLostfocus();
	delayTimer = -1;
}


void MAS::MenuItem::MsgTick() {
	Widget::MsgTick();
	if (HasFocus() && !topBar && child && delayTimer >= 0) {
		delayTimer += 1000/MAS::Settings::logicFrameRate;
		if (delayTimer >= skin->menuDelay) {
			delayTimer = -1;
			GetParent()->HandleEvent(*this, MSG_ACTIVATE);
		}
	}
}


void MAS::MenuItem::MsgKey() {
	Widget::MsgKey();
	GetParent()->HandleEvent(*this, MSG_ACTIVATE);
}


void MAS::MenuItem::MsgLPress() {
	Widget::MsgLPress();
	SetFlag(D_PRESSED);
}


void MAS::MenuItem::MsgLRelease() {
	Widget::MsgLRelease();
	if (TestFlag(D_PRESSED)) {
		ClearFlag(D_PRESSED);
		if (HasMouse()) {
			GetParent()->HandleEvent(*this, MSG_ACTIVATE);
		}
	}
}


void MAS::MenuItem::Draw(Bitmap &canvas) {
	int off, my;
	char *buf, *tok, *last;
	char tmp[16];

	// get the button state
	int state;
	if (topBar) {
		state = Disabled() ? 2 : (Selected() ? 3 : 0);
	}
	else {
		state = Disabled() ? 2 : (HasFocus() ? 3 : (Selected() ? 1 : 0));
	}
 
	// get the button colors and font
	Color fg = GetFontColor(state);
	Color bg = GetShadowColor(state);
	Font f = GetFont(state);

	// Get the subbitmap count and offset for a particular type of menu item
	int vcount;
	if (topBar) {
		vcount = 2;
		off = (Selected() || HasFocus()) ? 1 : 0;
	}
	else {
		vcount = 7;
		if (HasFocus()) {
			if (child)				off = 4;
			else if (Selected())	off = 6;
			else					off = 1;
		}
		else {
			if (child)				off = 3;
			else if (Selected())	off = 5;
			else					off = 0;
		}
	}

	// Draw a normal text menu item
	SetBitmap(topBar ? Skin::MENU_BUTTON : Skin::MENU_ITEM);
	Bitmap bmp = GetBitmap();

	if (text) {
		my = (h() - f.TextHeight())/2;

		// The menu button
		bmp.MaskedTiledBlit(canvas, 0, off, 0, 0, w(), h(), 1, vcount);

		// The menu text (left part)
		buf = ustrdup(text);
		tok = ustrtok_r(buf, uconvert_ascii("\t", tmp), &last);
		if (topBar) {
			f.GUITextoutEx(canvas, tok, 4, my, fg, bg, GetTextMode(), 0);
		}
		else {
			f.GUITextoutEx(canvas, tok, 16, my, fg, bg, GetTextMode(), 0);
		}

		// The menu text (right part)
		tok = ustrtok_r(NULL, empty_string, &last);
		if (tok) {
			f.GUITextout(canvas, tok, w() - f.TextLength(tok) - 12, my, fg, bg, GetTextMode(), 0);
		}

		free(buf);
	}
	// It's not a text item, it's a separator
	else {
		bmp.MaskedTiledBlit(canvas, 0, 2, 0, 0, w(), h(), 1, vcount);
	}
}


char *MAS::MenuItem::GetText() {
	return text;
}


void MAS::MenuItem::SetText(const char *t) {
	if (text) {
		delete [] text;
		text = NULL;
	}
	
	if (t) {
		text = new char[ustrsizez(t)];
		ustrcpy(text, t);
	}
	
	Redraw();
}


void MAS::MenuItem::UpdateSize() {
	if (text) {
		Font f = GetFont(Skin::NORMAL);
		int l = f.TextLength(text) + (topBar ? 8 : 64);
		Resize(l, skin->menuHeight);
	}
}



//////////////////////////////////////////////////

MAS::Menu::Menu()
	:Dialog(),
	topBar(true),
	parentMenu(NULL),
	openChildMenu(NULL)
{
	Dialog::Add(back);
	w(8);
	back.w(8);
	minSize = MAS::Size(0,0);
}


MAS::Menu::~Menu() {
	while (widgets.size() > 1) {
		Remove(0);
	}
}


void MAS::Menu::MsgInitSkin() {
	Dialog::MsgInitSkin();
	SetBitmap(Skin::MENU_BACK);
	back.SetBitmap(Skin::MENU_BACK);
	UpdateSize();
	SetAnimationProperties(skin->menuAnimationLength, skin->menuAnimationType);
	SetSample(Skin::SAMPLE_ACTIVATE, Skin::SAMPLE_ACTIVATE);
	SetSample(Skin::SAMPLE_OPEN, Skin::SAMPLE_OPEN);
	SetSample(Skin::SAMPLE_CLOSE, Skin::SAMPLE_CLOSE);
}


void MAS::Menu::MsgLostmouse() {
	Dialog::MsgLostmouse();
	HandleEvent(*this, MSG_LOSTMOUSE);
}


void MAS::Menu::MsgLostfocus() {
	Dialog::MsgLostfocus();
	HandleEvent(*this, MSG_LOSTFOCUS);
}


bool MAS::Menu::MsgXChar(int c) {
	Dialog::MsgXChar(c);

	if (openChildMenu || !topBar) {
		return false;
	}
	else {
		int x = MenuAltKey(c);
		if (!x) return false;

		bool foundIt = false;
		for (int i=1; i<widgets.size(); i++) {
			if (MenuKeyShortcut(x, ((MenuItem *)widgets[i])->GetText())) {
				foundIt = true;
				HandleEvent(*widgets[i], MSG_ACTIVATE);
				MenuItem *item = (MenuItem *)widgets[i];
				parent->MoveFocusTo(item);
				break;
			}
		}
	}

	return true;
}


bool MAS::Menu::MsgChar(int c) {
	Dialog::MsgChar(c);

	switch (c>>8) {
		case KEY_ESC: {
			if (openChildMenu) {
				CloseChildMenu();
			}
			if (parentMenu) {
				GetParent()->MoveFocusTo(parentMenu);
				Close();
				parentMenu->GetParent()->SendMessage(MSG_CHAR, c);
			}
			if (topBar) {
				simulate_keypress(KEY_TAB<<8);
			}
		}
		break;
		
		case KEY_TAB: {
			if (openChildMenu) {
				CloseChildMenu();
			}
			if (parentMenu) {
				GetParent()->MoveFocusTo(parentMenu);
				Close();
				parentMenu->GetParent()->SendMessage(MSG_CHAR, KEY_ESC<<8);
			}
			return false;
		}
		break;
		
		case KEY_LEFT:
			if (topBar) {
				MoveFocus(3);
			}
			else {
				if (parentMenu) {
					Close();
					GetParent()->MoveFocusTo(parentMenu);
					if (parentMenu->topBar) {
						parentMenu->GetParent()->SendMessage(MSG_CHAR, c);
					}
				}
			}
			break;
		
		case KEY_RIGHT:
			if (topBar) {
				MoveFocus(2);
			}
			else {
				if (openChildMenu) {
					GetParent()->MoveFocusTo(openChildMenu->widgets[1]);
				}
				else if (parentMenu) {
					Close();
					GetParent()->MoveFocusTo(parentMenu);
					parentMenu->GetParent()->SendMessage(MSG_CHAR, c);
				}
			}
			break;
		
		case KEY_UP:
			if (!topBar) {
				MoveFocus(5);
			}
			break;
		
		case KEY_DOWN:
			if (!topBar) {
				MoveFocus(4);
			}
			else {
				if (openChildMenu) {
					GetParent()->MoveFocusTo(openChildMenu->widgets[1]);
				}
			}
			break;
		
		default: {
			if (openChildMenu) {
				return openChildMenu->MsgChar(c);
			}
			else {
				bool foundIt = false;
				for (int i=1; i<widgets.size(); i++) {
					if (MenuKeyShortcut(c, ((MenuItem *)widgets[i])->GetText())) {
						foundIt = true;
						MenuItem *item = (MenuItem *)widgets[i];
						GetParent()->MoveFocusTo(item);
						HandleEvent(*item, MSG_ACTIVATE);
						return true;
					}
				}
			}
			return false;
		}
	}

	return true;
}

// dir: 2 - right, 3 - left, 4 - down, 5 - up (0 and 1 are not used (TAB and shift+TAB)
void MAS::Menu::MoveFocus(int dir) {
	int i=1;
	while (true) {
		if (widgets[i]->HasFocus()) break;
		++i;
	}
	
	int di = -1-2*(dir%2-1);
	while (true) {
		i += di;
		if (i >= widgets.size()) {
			i = 1;
		}
		else if (i < 1) {
			i = widgets.size()-1;
		}
		//i = MID(1, i, widgets.size()-1);
		if (((MenuItem*)widgets[i])->text) {
			MoveFocusTo(widgets[i]);
			break;
		}
	}
}


bool MAS::Menu::MenuKeyShortcut(int c, const char *s) {
	int d;

	if (!s) return false;
	while ((d = ugetxc(&s)) != 0) {
		if (d == '&') {
			d = ugetc(s);
			if ((d != '&') && (utolower(d) == utolower(c & 0xFF)))
				return true;
		}
	}

	return false;
}


int MAS::Menu::MenuAltKey(int k) {
	static unsigned char alt_table[] =
	{
		KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, 
		KEY_J, KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, 
		KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z
	};

	AL_CONST char *s;
	int c, d;

	if (k & 0xFF)
		return 0;

	k >>= 8;

	c = scancode_to_ascii(k);
	if (c) {
		k = c;
	}
	else {
		for (c=0; c<(int)sizeof(alt_table); c++) {
			if (k == alt_table[c]) {
				k = c + 'a';
				break;
			}
		}

		if (c >= (int)sizeof(alt_table))
		return 0;
	}

	for (int i=1; i<widgets.size(); i++) {
		s = ((MenuItem *)widgets[i])->GetText();
		while ((d = ugetxc(&s)) != 0) {
			if (d == '&') {
				d = ugetc(s);
				if ((d != '&') && (utolower(d) == utolower(k))) {
					return k;
				}
			}
		}
	}

	return 0;
}


MAS::Dialog::ActionType MAS::Menu::GetAction() {
	if (mouseObject == &back) {
		return MOVE;
	}
	else {
		return NONE;
	}
}


void MAS::Menu::HandleEvent(Widget& obj, int msg, int arg1, int arg2) {
	Dialog::HandleEvent(obj, msg, arg1, arg2);

	switch (msg) {
		case MSG_ACTIVATE:
			if (&obj == openChildMenu) {
				Menu *m = GetRootMenu();
				if (!m->parentMenu) {
					m->Close();
				}
			}
			else {
				MenuItem *item = (MenuItem *)(&obj);
				if (item->child) {
					if (openChildMenu) {
						if (openChildMenu == item->child) {
							if (topBar && skin->focus != 2) {
								CloseChildMenu();
								obj.Deselect();
								PlaySample(Skin::SAMPLE_CLOSE);
							}
						}
						else {
							if (skin->focus != 2) {
								CloseChildMenu();
								OpenChildMenu(item);
								obj.Select();
								PlaySample(Skin::SAMPLE_OPEN);
							}
						}
					}
					else {
						OpenChildMenu(item);
						obj.Select();
						PlaySample(Skin::SAMPLE_OPEN);
					}
				}
				else if (item->callbackID >= 0) {
					if (parentMenu) {
						parentMenu->GetParent()->HandleEvent(*this, MSG_ACTIVATE);
						PlaySample(Skin::SAMPLE_ACTIVATE);
					}
					else {
						Close();
						PlaySample(Skin::SAMPLE_CLOSE);
					}
					//GetParent()->HandleEvent(obj, item->callbackID);
					GetParent()->HandleEvent(*GetRootMenu(), item->callbackID);
				}
			}
			break;

		case MSG_GOTFOCUS: {
			MenuItem *item = (MenuItem *)(&obj);
			if (openChildMenu && openChildMenu != item->child) {
				CloseChildMenu();
				
				if (item->child) {
					OpenChildMenu(item);
					PlaySample(Skin::SAMPLE_OPEN);
				}
				else {
					PlaySample(Skin::SAMPLE_CLOSE);
				}
				
				if (topBar) {
					obj.Select();
				}
			}
			break;
		}

		case MSG_LOSTMOUSE: {
			//if (skin->focus == 0) {
				if (!MouseInMenu() && !MouseInChildMenu()) {
					GetRootMenu()->Close();
				}
			//}
			break;
		}

		/*
		case MSG_LOSTFOCUS: {
			if (obj == *this) {
				if (!openChildMenu || !openChildMenu->HasFocus()) {
					GetRootMenu()->Close();
				}
			}
			break;
		}
		*/
	};
}


bool MAS::Menu::MouseInMenu() {
	Point mp = GetMousePos();
	Rect r = Rect(0, 0, w(), h());
	//if (HasMouse()) {
	if (mp <= r) {
		return true;
	}
	else if (parentMenu) {
		return ((MAS::Menu *)parentMenu->GetParent())->MouseInMenu();
	}
	else {
		return false;
	}
}


bool MAS::Menu::MouseInChildMenu() {
	if (!openChildMenu) {
		return false;
	}
	//else if (openChildMenu->HasMouse()) {
	else {
		Point mp = openChildMenu->GetMousePos();
		Rect r = Rect(0, 0, openChildMenu->w(), openChildMenu->h());
		if (mp <= r) {
			return true;
		}
		else {
			return openChildMenu->MouseInChildMenu();
		}
	}
}


void MAS::Menu::CloseChildMenu() {
	if (openChildMenu) {
		openChildMenu->CloseChildMenu();
	}
	else {
		return;
	}

	GetParent()->Remove(*openChildMenu);
	openChildMenu->parentMenu->Deselect();
	openChildMenu->ResetAnimation();
	openChildMenu = NULL;
}


void MAS::Menu::OpenChildMenu(MenuItem *item) {
	int cx = topBar ? item->x() : item->x() + item->w()*3/4;
	int cy = topBar ? y2() : item->y() + item->h()/2;
	item->child->Popup(GetParent(), item, cx, cy);
	openChildMenu = item->child;
}


MAS::Menu *MAS::Menu::GetRootMenu() {
	if (parentMenu) {
		return ((Menu *)parentMenu->GetParent())->GetRootMenu();
	}
	else {
		return this;
	}
}


void MAS::Menu::Add(MenuItem *item) {
	Dialog::Add(*item);
	UpdateSize();
}


void MAS::Menu::Add(const char *text, Menu &child) {
	MenuItem *item = new MenuItem(text, &child);
	item->allocated = true;
	Add(item);
}


void MAS::Menu::Add(const char *text, int id) {
	MenuItem *item = new MenuItem(text, id);
	item->allocated = true;
	Add(item);
}


void MAS::Menu::Remove(int i) {
	if (i >= 0 && i < widgets.size()-1) {
		MenuItem *item = (MenuItem *)widgets[i+1];
		Dialog::Remove(*widgets[i+1]);
		if (item->allocated) {
			delete item;
		}
	}
}


MAS::MenuItem *MAS::Menu::GetItem(int i) {
	if (i >= 0 && i < widgets.size()-1) {
		return (MenuItem *)widgets[i+1];
	}
	
	return NULL;
}


void MAS::Menu::UpdateSize() {
	Dialog::UpdateSize();

	int i;
	if (topBar) {
		int xx = 0;
		int yy = 0;
		int ww = 0;
		int hh = skin->menuHeight;
		
		for (i=1; i<widgets.size(); i++) {
			MenuItem *item = (MAS::MenuItem *)(widgets[i]);
			item->Place(xx, yy);
			item->topBar = true;
			item->UpdateSize();
			xx += item->w();
			ww += item->w();
		}
		
		Resize(ww+2, hh+2);
		back.Resize(w(), h());
	}
	else {
		if (bitmap < 0) return;
		//Bitmap bmp = back.GetBitmap();

		int xx = 2;
		int yy = 2;
		int ww = 0;
		int hh = 0;
		
		for (i=1; i<widgets.size(); i++) {
			MenuItem *item = (MAS::MenuItem *)(widgets[i]);
			item->topBar = false;
			item->UpdateSize();
			if (item->w() > ww) {
				ww = item->w();
			}
		}
		
		for (i=1; i<widgets.size(); i++) {
			MenuItem *item = (MAS::MenuItem *)(widgets[i]);
			item->Shape(xx, yy, ww, skin->menuHeight);
			yy += item->h();
			hh += item->h();
		}

		Rect r = *GetParent();
		xx = x() - parent->x();
		if (xx + ww > r.x2()) {
			if (parentMenu) {
				xx = parentMenu->GetParent()->x() + parentMenu->w()/4 - ww;
			}
			else {
				xx = r.w() - ww - 4;
			}
		}
		yy = y() - parent->y();
		if (yy + hh > r.y2()) {
			yy = r.h() - hh - 1;
		}
		Shape(xx, yy, ww + 4, hh + 4);
		back.Resize(ww + 4, hh + 4);
	}
}


void MAS::Menu::Popup(MAS::Dialog *parentDialog, MAS::MenuItem *parentMenu, int x, int y) {
	parentDialog->Remove(*this);
	topBar = false;
	this->parentMenu = parentMenu;
	parentDialog->Add(*this);
	Place(x - parent->x(), y - parent->y());
	TakeFocusFrom(focusObject);
	UpdateSize();
	Animate();
	Redraw();
	PlaySample(Skin::SAMPLE_OPEN);
}


void MAS::Menu::Popup(MAS::Dialog *parentDialog, MAS::MenuItem *parentMenu, const Point &p) {
	Popup(parentDialog, parentMenu, p.x(), p.y());
}


void MAS::Menu::Close() {
	CloseChildMenu();
	if (!topBar) {
		GetParent()->Remove(*this);
	}
	else {
		if (focusObject) {
			focusObject->Deselect();
			MoveFocusTo(NULL);
		}
	}
}


void MAS::Menu::Enable(int i)	{ widgets[i+1]->ClearFlag(D_DISABLED); }
void MAS::Menu::Disable(int i)	{ widgets[i+1]->SetFlag(D_DISABLED); }
void MAS::Menu::Check(int i)	{ widgets[i+1]->SetFlag(D_SELECTED); }
void MAS::Menu::Uncheck(int i)	{ widgets[i+1]->ClearFlag(D_SELECTED); }
bool MAS::Menu::Disabled(int i)	{ return (widgets[i+1]->TestFlag(D_DISABLED)); }
bool MAS::Menu::Checked(int i)	{ return (widgets[i+1]->TestFlag(D_SELECTED)); }
