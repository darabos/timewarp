////////////////////////////////////////////////////////////////////////////////
//         MASkinG - Miran Amon's Skinnable GUI Library for Allegro           //
//    Copyright (c) 2002-2003 Miran Amon (miranamon@users.sourceforge.net)    //
//          Project websites: http://ferisrv5.uni-mb.si/~ma0747               //
//                  http://sourceforge.net/projects/masking                   //
////////////////////////////////////////////////////////////////////////////////

#include "../include/MASkinG/editbox.h"


MAS::EditBox::EditBox()
	:Label(),
	maxLength(0),
	cursorpos(0),
	offsetpos(0),
	selStart(0),
	selEnd(0),
	firstSel(0),
	oldSel(0)
{
	ClearFlag(D_AUTOSIZE);
}


void MAS::EditBox::Setup(int x, int y, int w, int h, int key, int flags, const char *text, int len) {
	Widget::Setup(x, y, w, h, key, flags);
	if (len >= 0) {
		Setup(text, len);
	}
	else {
		Setup(text, 256);
	}
}


void MAS::EditBox::MsgInitSkin() {
	SetBitmap(Skin::PANEL_SUNKEN);
	SetSample(Skin::SAMPLE_GOTFOCUS, Skin::SAMPLE_GOTFOCUS);
	SetSample(Skin::SAMPLE_LOSTFOCUS, Skin::SAMPLE_LOSTFOCUS);
	SetSample(Skin::SAMPLE_ACTIVATE, Skin::SAMPLE_ACTIVATE);
	SetSample(Skin::SAMPLE_KEY, Skin::SAMPLE_KEY);
	ClearFlag(D_AUTOSIZE);
	Label::MsgInitSkin();
}


int MAS::EditBox::FindCursorPos() {
	int p;
	int x1 = 4;
	char s[12];
	usprintf(s, uconvert_ascii(".", s));
	int l = ustrlen(text);

	int mx = GetMousePos().x() + skin->GetCursor(Skin::MOUSE_TEXTSELECT).GetFocus().x();
	Font f = GetFont(HasFocus() ? Skin::FOCUS : Skin::NORMAL);
	
	for (p = offsetpos; p < l; p++) {
		usetat(s, 0, ugetat(text, p));
		x1 += f.TextLength(s);
		if (x1 > mx) {
			break;
		}
	}

	return p;
}


void MAS::EditBox::MsgLPress() {
	int oldCursorpos = cursorpos;

	Label::Select();
	selStart = FindCursorPos();
	selEnd = selStart;
	cursorpos = selStart;
	firstSel = selStart;
	if (cursorpos != oldCursorpos) {
		UpdateOffset();
		Redraw();
	}
	oldSel = 0;
	SetFlag(D_PRESSED);
	
	Widget::MsgLPress();
}


void MAS::EditBox::MsgMousemove(const Point &d) {
	if (Flags() & D_PRESSED) {
		selEnd = cursorpos = FindCursorPos();
		if (selEnd <= selStart) {
			selStart = selEnd;
			selEnd = firstSel;
		}
		if (selEnd - selStart != oldSel) {
			oldSel = selEnd - selStart;
		}
		UpdateOffset();
		Redraw();
	}
	
	Widget::MsgMousemove(d);
}


void MAS::EditBox::MsgLRelease() {
	if (Flags() & D_PRESSED) {
		cursorpos = FindCursorPos();
		if (selStart == selEnd) {
			Label::Deselect();
		}
		ClearFlag(D_PRESSED);
		UpdateOffset();
		Redraw();
	}
	
	Widget::MsgLRelease();
}


void MAS::EditBox::MsgDClick() {
	selStart = 0;
	selEnd = ustrlen(text);
	Label::Select();
	Redraw();
	Widget::MsgDClick();
}


bool MAS::EditBox::MsgChar(int c) {
	bool ret = true;
	bool has_changed = false;
	int l = ustrlen(text);
	int ch = c>>8;

	if (ch == KEY_LEFT)	{
		if (cursorpos > 0) {
			cursorpos--;
			if (key_shifts & KB_SHIFT_FLAG) {
				if (!Selected()) {
					Label::Select();
					selStart = cursorpos+1;
					selEnd = selStart;
				}
				if (selStart > cursorpos) {
					selStart = cursorpos;
				}
				else {
					selEnd = cursorpos;
				}
			}
			else {
				Label::Deselect();
			}
		}
		else if (!(key_shifts & KB_SHIFT_FLAG)) {
			Label::Deselect();
		}
	}
	else if (ch == KEY_RIGHT)	{
		if (cursorpos < l) {
			cursorpos++;
			if (key_shifts & KB_SHIFT_FLAG) {
				if (!Selected()) {
					Label::Select();
					selStart = cursorpos-1;
					selEnd = selStart;
				}
				if (selEnd < cursorpos) {
					selEnd = cursorpos;
				}
				else {
					selStart = cursorpos;
				}
			}
			else {
				Label::Deselect();
			}
		}
		else if (!(key_shifts & KB_SHIFT_FLAG)) {
			Label::Deselect();
		}
	}
	else if (ch == KEY_HOME) {
		if (key_shifts & KB_SHIFT_FLAG) {
			if (!Selected()) {
				selStart = 0;
				selEnd = cursorpos;
				Label::Select();
			}
			else {
				selEnd = selStart;
				selStart = 0;
				Label::Select();
			}
			cursorpos = 0;
		}
		else {
			Label::Deselect();
			cursorpos = 0;
		}
	}
	else if (ch == KEY_END)	{
		if (key_shifts & KB_SHIFT_FLAG) {
			if (!Selected()) {
				selStart = cursorpos;
				selEnd = l;
				Label::Select();
			}
			else {
				selStart = selEnd;
				selEnd = l;
				Label::Select();
			}
			cursorpos = l;
		}
		else {
			Label::Deselect();
			cursorpos = l;
		}
	}
	else if (ch == KEY_DEL)	{
		if (Selected()) {
			int i=0;
			for (int p=selEnd; ugetat(text, p); p++) {
				usetat(text, selStart+i, ugetat(text, p));
				i++;
			}
			usetat(text, selStart+i, '\0');
			cursorpos = selStart;
			Label::Deselect();
			PlaySample(Skin::SAMPLE_KEY);
		}
		else if (cursorpos < l) {
			uremove(text, cursorpos);

			PlaySample(Skin::SAMPLE_KEY);
			has_changed = true;
		}
	}
	else if (ch == KEY_BACKSPACE)	{
		if (Selected()) {
			int i=0;
			for (int p=selEnd; ugetat(text, p); p++) {
				usetat(text, selStart+i, ugetat(text, p));
				i++;
			}
			usetat(text, selStart+i, '\0');
			cursorpos = selStart;
			Label::Deselect();
			PlaySample(Skin::SAMPLE_KEY);
		}
		else if (cursorpos > 0)		{
			--cursorpos;
			uremove(text, cursorpos);
			if (cursorpos <= offsetpos) offsetpos = 0;

			PlaySample(Skin::SAMPLE_KEY);
			has_changed = true;
		}
	}
	else if (ch == KEY_ENTER) {
		Label::Deselect();
		Redraw();
		PlaySample(Skin::SAMPLE_ACTIVATE);
		GetParent()->HandleEvent(*this, MSG_ACTIVATE);
	}
	else {
		ret = false;
	}

	UpdateOffset();
	Redraw();

	Widget::MsgChar(c);
	return ret;
}


bool MAS::EditBox::MsgUChar(int c) {
	bool ret = false;
	int l = ustrlen(text);
	if (c >= ' ' && uisok(c)) {
		if (Selected()) {
			int i=0;
			for (int p=selEnd; ugetat(text,p); p++) {
				usetat(text, selStart+i, ugetat(text, p));
				i++;
			}
			usetat(text, selStart+i, '\0');
			cursorpos = selStart;
			Label::Deselect();
		}
		if (l < maxLength) {
			uinsert(text, cursorpos, c);
			cursorpos++;
			PlaySample(Skin::SAMPLE_KEY);
		}

		UpdateOffset();
		Redraw();
		ret = true;
	}

	Widget::MsgUChar(c);
	return ret;
}


void MAS::EditBox::MsgGotmouse() {
	SetCursor(Skin::MOUSE_TEXTSELECT);
	Widget::MsgGotmouse();
}


void MAS::EditBox::MsgLostmouse() {
	SetCursor(Skin::MOUSE_NORMAL);
	Widget::MsgLostmouse();
}


void MAS::EditBox::Setup(const char *tex, int len) {
	int texLen = 0;
	if (tex && ustrcmp(tex, empty_string) != 0) texLen = ustrlen(tex);

	int l = len;
	if (l < texLen) l = texLen;
	if (l <= 0) l = 256;

	// Make room and copy the text
	if (text) delete text;
	text = new char[6*(l+1)];

	if (tex != NULL) ustrcpy(text, tex);
	else ustrcpy(text, empty_string);

	maxLength = l;
	//usetat(text, uoffset(text, -1), '\0');

	cursorpos = ustrlen(text);
	offsetpos = 0;
}


void MAS::EditBox::SetText(const char *tex, int len) {
	const char *tmp_txt = empty_string;
	if (tex != NULL) tmp_txt = tex;

	if (len == -1) {
		len = maxLength == 0 ? 256 : maxLength;
	}
	if (len != -1 && len != maxLength) {
		Setup(tmp_txt, len);
	}
	else {
		// Copy the stuff over
		ustrncpy(text, tmp_txt, maxLength);
		cursorpos = ustrsize(text);
	}

	offsetpos = 0;
	Redraw();
}


void MAS::EditBox::UpdateOffset() {
	int lastchar;
	Bitmap null;
	while (true) {
		// Try with the current offset, but don't really draw
		lastchar = DrawText(offsetpos, null);

		// need to decrease offset?
		if (offsetpos > cursorpos) {
			offsetpos--;
			if (offsetpos < 0) {
				offsetpos = 0;
				break;
			}
		}
		// need to increase offset?
		else if (cursorpos > lastchar+1) {
			offsetpos++;
		}
		// or is it just right?
		else {
			break;
		}
	}
}


void MAS::EditBox::Draw(Bitmap &canvas) {
	// Draw the text on the canvas
	GetBitmap().MaskedTiledBlit(canvas, 0, 0, 0, 0, w(), h(), 1, 1);
	DrawText(offsetpos, canvas);

	// Possibly draw the dotted rectangle
	if (skin->drawDots && HasFocus()) {
		canvas.DrawDottedRect(2, 2, w()-3, h()-3, skin->c_font);
	}
}


// Helper to draw the editable text
// Returned value is last character number drawn
int MAS::EditBox::DrawText(int offsetpos, Bitmap &canvas) {
	// get the button state
	int state = Disabled() ? 2 : (HasFocus() ? 3 : 0);
	
	// get the button colors and font
	Color fg = GetFontColor(state);
	Color bg = GetShadowColor(state);
	Font f = GetFont(state);
	Color fgs = GetFontColor(Skin::SELECT);
	Color bgs = GetShadowColor(Skin::SELECT);

	int	x1 = 4;
	int	y1 = (h()>>1) - (f.TextHeight()>>1) + 1;
	int	xx = w()-10;
	char s[12];
	usprintf(s, uconvert_ascii(".", s));
	int	i = offsetpos;
	int	len;
	int cursorx = -1;
	int ii;

	// Assume that we will go to the end
	int there_is_more = 0;

	// Possibly remember where the cursor should go
	if (i == cursorpos) cursorx = x1;

	while (ugetat(text, i) != 0) {
		// Find the length of the character
		usetat(s, 0, ugetat(text, i));
		len = f.TextLength(s);

		// Draw the character, possibly with cursor
		if (canvas) {
			if (Selected() && i>=selStart && i<selEnd) {
				if (HasFocus()) {
					f.GUITextout(canvas, s, x1, y1, fgs, bgs, skin->c_select, 0);
				}
				else {
					f.GUITextout(canvas, s, x1, y1, fg, bg, skin->c_deselect, 0);
				}
			}
			else {
				f.GUITextout(canvas, s, x1, y1, fg, bg, -1, 0);
			}
		}

		x1 += len;

		// Stop drawing if we are out of bounds
		if (x1 > xx) {
			// Remember that there is more to draw
			there_is_more = 1;
			break;
		}
		i++;

		// Possibly remember where the cursor should go
		if (i == cursorpos) cursorx = x1;
	}
	// If we ran off the end before finding the cursor, then its at the end
	if ((there_is_more) && (cursorx < 0)) cursorx = x1;

	// Draw the cursor if we have the focus
	if ((cursorx >= 0) && HasFocus() && canvas) {
		for (ii=0; ii<f.TextHeight(); ii++) {
			canvas.Putpixel(cursorx-1, y1+ii, fg);
		}
	}
	return i;
}


void MAS::EditBox::Select() {
	MsgDClick();
}


void MAS::EditBox::Deselect() {
	selStart = selEnd = firstSel = oldSel = 0;
	offsetpos = cursorpos = 0;
	Label::Deselect();
	Redraw();
}


void MAS::EditBox::ScrollTo(int offset) {
	if (!text) return;
	Deselect();
	offset = MID(0, offset, ustrlen(text));
	cursorpos = offset;
	UpdateOffset();
}


bool MAS::EditBox::MsgWantfocus() {
	return true;
}
