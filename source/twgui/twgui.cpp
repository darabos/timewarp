
#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
#include "../melee/mview.h"
REGISTER_FILE

#include "twgui.h"



bool check_pixel(BITMAP *bmp, int x, int y)
{
	if (!bmp)
		return false;
	
	int col;
	col = getpixel(bmp, x, y);
	if (col != makecol(255,0,255))
		return true;
	else
		return false;
}



TextList::TextList(AreaReserve *menu, char *identbranch, int ax, int ay, FONT *afont)
:
AreaTablet(menu, identbranch, ax, ay, 255)
{
	usefont = afont;
	Htxt = text_height(usefont);
	text_color = makecol(0,0,0);

//	yselected = 0;

	optionlist = 0;
	N = 0;

	selected = false;

	scrollcontrol.ver(this, identbranch, &scroll);
	scroll.set(0, 0, 1, 0, 1, 1);

	Nshow = int(H / Htxt) - 1;		// -1, because item 0 is also shown...

}


TextList::~TextList()
{
	
	clear_optionlist();
}


void TextList::calculate()
{
	AreaTablet::calculate();

	scroll.calculate();

//	if (scroll.yselect > N-1)
//		scroll.yselect = N-1;

	
	if (!flag.focus)
		return;

}


void TextList::set_selected(int iy)
{
//	yselected = iy;
	if ( iy >= 0 && iy < N )
		scroll.yselect = iy;
	else
		scroll.yselect = -1;
}

void TextList::clear_optionlist()
{
	int i;

	if (optionlist)			// delete an existing set of strings first.
	{
		for ( i = 0; i < N; ++i )
			delete optionlist[i];	// delete the strings pointed at

		delete optionlist;			// delete the pointers
		optionlist = 0;
	}

	N = 0;
	scroll.Ny = 0;
	scroll.set(0, 0, 1, 0, 1, 1);
}


void TextList::set_optionlist(char **aoptionlist, int color)
{
	int aN;

	// in this case, where the number of elements in the list isn't explicitly
	// specified, I search for a null-element that indicates the last element
	// in the list:
	aN = 0;

	if (aoptionlist)
		while ( aoptionlist[aN] != 0 )
			++aN;

	set_optionlist(aoptionlist, aN, color);
}

void TextList::set_optionlist(char **aoptionlist, int aN, int color)
{
	int i;

	clear_optionlist();		// note that this resets N .

	N = aN;
	if (N == 0)
		return;				// in case there's an empty list

	if (N > 0)
		optionlist = new char* [N];	// reserve space for that many pointers to strings.
	else
		optionlist = 0;

	for ( i = 0; i < N; ++i )
	{
		optionlist[i] = new char[strlen(aoptionlist[i]) + 1 ];
		strcpy(optionlist[i], aoptionlist[i]);
	}

	scroll.set(0, 0, 1, N , 1, Nshow);
}


// if the mouse is clicked within the window:
void TextList::handle_lpress()
{

	int iy;
	
	iy = mouse.pos.y - y;

	iy /= Htxt;		// # of item relative to the top

	iy += scroll.y;	// scroll->y = 1st item at the top

//	if (iy < N)
//	{
//		scroll.yselect = iy;
//		selected = true;
//	}

	
	selected = true;
	if (iy < N)
		scroll.yselect = iy;
	else
		scroll.yselect = -1;

}


int TextList::getk()
{
	return scroll.yselect;
}

// select and center the list on "that" item
void TextList::handle_rpress()
{
	
	int iy;
	
	iy = mouse.pos.y - y;

	iy /= Htxt;		// # of item relative to the top

	iy += scroll.y;	// yshift = 1st item at the top

	iy -= Nshow/2;	// center in the window
	if (iy <= 0)
		iy = 0;

	scroll.y = iy;
}


void TextList::subanimate()
{

	if (!optionlist || N == 0)
		return;				// if it's an empty list

	int ix, iy;
	int i;


	// check if there's need for shifting the list, i.e., if the scroll is off
	// the map:
	
	// note, a positive "shift" means, a number of items are skipped at the top,
	// when drawing them:


	// else, do nothing ;)					// selected item is somewhere in the middle.


	for ( i = scroll.y; i < N; ++i )
	{
		if (i > scroll.y + scroll.Nyshow)
			break;

		ix = 0;
		iy = Htxt * (i - scroll.y);

		int c;

		if ( i != scroll.yselect )
		{
			text_mode(-1);
			c = text_color;
		} else {
			text_mode( makecol(0,0,0) );
			c = makecol(255,255,255);
		}

		if (optionlist[i] && strlen(optionlist[i]) < 20)
			textout(drawarea, usefont, optionlist[i], ix, iy, c);
	}
	

}










ScrollControl::ScrollControl()
{
	left = 0;
	right = 0;
	up = 0;
	down = 0;
	scrollhor = 0;
	scrollvert = 0;
}

void ScrollControl::hor(AreaReserve *A, char *id, scrollpos_str *scr)
{

	// create the buttons and bitmaps, finding graphics in some data file,
	// with strict name conventions having as base the general identifier "ident".
	//
	// note that the buttons required, and their layout, are determined by the
	// graphics in the datafile.
	char id2[128];


	strcpy(id2, id);
	strcat(id2, "/left");
	left	= new Button(A, id2, -1, -1, 0);
	if (!left->W) {	delete left;	left = 0;	}

	strcpy(id2, id);
	strcat(id2, "/right");
	right	= new Button(A, id2, -1, -1, 0);
	if (!right->W)	{	delete right;	right = 0;	}

	// add a scroll bar
	strcpy(id2, id);
	strcat(id2, "/scrollbar_hor");
	scrollhor = new ScrollBar(A, id2, -1, -1);
	if (!scrollhor->W)	{	delete scrollhor;	scrollhor = 0;	}

	scr->left = left;
	scr->right = right;
	scr->scrollhor = scrollhor;
}

void ScrollControl::ver(AreaReserve *A, char *id, scrollpos_str *scr)
{
	char id2[128];

	strcpy(id2, id);
	strcat(id2, "/up");
	up		= new Button(A, id2, -1, -1, 0);
	if (!up->W)	{	delete up;	up = 0;	}

	strcpy(id2, id);
	strcat(id2, "/down");
	down	= new Button(A, id2, -1, -1, 0);
	if (!down->W)	{	delete down;	down = 0;	}

	strcpy(id2, id);
	strcat(id2, "/scrollbar_vert");
	scrollvert = new ScrollBar(A, id2, -1, -1);
	if (!scrollvert->W)	{	delete scrollvert;	scrollvert = 0;	}

	scr->up = up;
	scr->down = down;
	scr->scrollvert = scrollvert;
}


void ScrollControl::hor(AreaGeneral *A, char *id, scrollpos_str *scr)
{
	ScrollControl::hor(A->areareserve, id, scr);
}

void ScrollControl::ver(AreaGeneral *A, char *id, scrollpos_str *scr)
{
	ScrollControl::ver(A->areareserve, id, scr);
}



// ------------ AND NOW THE GRAPHICAL PART ---------------


// to implement a button, you add bitmaps-feedback to the box-area control


Button::~Button()
{
	if (bmp_default)
		destroy_bitmap(bmp_default);
	if (bmp_focus)
		destroy_bitmap(bmp_focus);
	if (bmp_selected)
		destroy_bitmap(bmp_selected);
}


Button::Button(AreaReserve *menu, char *identbranch, int ax, int ay, int asciicode, bool akeepkey)
:
AreaBox(menu, identbranch, ax, ay, asciicode, akeepkey)
{
	char strdefault[128], strfocus[128], strselected[128];

	strcpy(strdefault,  ident);
	strcpy(strfocus,    ident);
	strcpy(strselected, ident);

	strcat(strdefault,  "/default");
	strcat(strfocus,    "/focus");
	strcat(strselected, "/selected");

	
	bmp_default = areareserve->bmp(strdefault);
	bmp_focus = areareserve->bmp(strfocus);
	bmp_selected = areareserve->bmp(strselected);
	// if one of these bitmaps is 0, the "defocus" (or default) will be used

	if (bmp_default)
	{
		W = bmp_default->w;		// note: it's already scaled on initialization.
		H = bmp_default->h;
	} else {
		//tw_error("Could not initialize button bitmap");
		W = 0;
		H = 0;
	}


	if ( ax == -1 && ay == -1 && bmp_default )
		locate_by_backgr(strdefault);

}


// this is the default drawing (at rest):
void Button::draw_default()
{
	BITMAP *b = bmp_default;
	if (b)
		blit(b, areareserve->drawarea, 0, 0, x, y, b->w, b->h);
}

void Button::draw_focus()
{
	BITMAP *b = bmp_focus;
	if (b)
		blit(b, areareserve->drawarea, 0, 0, x, y, b->w, b->h);
	else
	{
		draw_default();
		draw_rect_fancy();
	}
}

void Button::draw_selected()
{
	BITMAP *b = bmp_selected;
	if (b)
		blit(b, areareserve->drawarea, 0, 0, x, y, b->w, b->h);
	else
		draw_default();
}


bool Button::hasmouse()
{
	// the first rough check whether it's in the boxed bitmap area
	if (AreaBox::hasmouse())
	{
		// now check the bitmap, if the mouse is touching a non-transparent sprite.
		// only check the default bitmap (buttons that change size aren't cool)
		return check_pixel(bmp_default, mouse.xpos()-x, mouse.ypos()-y);

	} else
		return false;
}




GhostButton::GhostButton(AreaReserve *menu)
:
AreaGeneral(menu)
{
}
GhostButton::~GhostButton()
{
}


TextButton::TextButton(AreaReserve *menu, char *identbranch, int ax, int ay, FONT *afont)
:
AreaTablet(menu, identbranch, ax, ay, 255)
{
	usefont = afont;
}

TextButton::~TextButton()
{
}

void TextButton::set_text(char *txt, int color)
{
	if (txt)
		strncpy(text, txt, 63);
	else
		text[0] = 0;
	text_color = color;
}

void TextButton::subanimate()
{
	int xcentre, ycentre;

	xcentre = W/2;
	ycentre = H/2 - text_height(usefont)/2;

	text_mode(-1);
	textout_centre(drawarea, usefont, text, xcentre, ycentre, text_color);
}






TextInfo::TextInfo(FONT *afont, BITMAP *abmp, char *atextinfo, int aNchars)
{
	bmp = abmp;
	usefont = afont;
	textinfo = atextinfo;
	Nchars = aNchars;


	Htxt = text_height(usefont);
	text_color = makecol(0,0,0);
	Nshow = int(bmp->h / Htxt) - 1;		// -1, because item 0 is also shown...
}

TextInfo::~TextInfo()
{
}

void TextInfo::reset(scrollpos_str *scroll)
{

	// "initialize" the text:
	int n, len;
	n = 0;
	len = 0;

	Nlines = 0;
	linestart[0] = 0;
	if (textinfo[0] == 0)	// empty text
	{
		Nlines = 1;	// even empty text has 1 line to hold the cursor.
		linestart[1] = 0;

		scroll->y = 0;
		scroll->yselect = 0;
		scroll->set(0, scroll->y, 1, Nlines, 1, Nshow);
		return;
	}

	++Nlines;
	++n;

	// check (again) how long the text is...
	Nchars = strlen(textinfo);

	while ( n < Nchars )
	{
		if (textinfo[n] == 0 || textinfo[n] == '\n')
		{
			linestart[Nlines] = n+1;
			++Nlines;

			len = 0;

			if (textinfo[n] == 0)
				break;
			// value 0 indicates the end of the text.

		} else {
			char txt[2];
			txt[0] = textinfo[n];
			txt[1] = 0;

			len += text_length(usefont, txt);

			// if the line exceeds the window width, then you've to look
			// back for the last space
			if (len >= bmp->w-1)
			{
				while (n > 0 && textinfo[n] != ' ' && textinfo[n] != '.' && textinfo[n] != ',' &&
						textinfo[n] != '!' && textinfo[n] != '?' && textinfo[n] != ';' &&
						textinfo[n] != ':' && textinfo[n] != '-' && textinfo[n] != '/' &&
						 textinfo[n] != '\n' && textinfo[n] != 0 )
					--n;

				linestart[Nlines] = n+1;
				++Nlines;

				len = 0;
			}
		}

		if (Nlines >= maxlines)
			break;

		++n;
	}

	// check if the scroll position is valid.
	if (scroll->y > Nlines-1)		scroll->y = Nlines-1;
	if (scroll->yselect > Nlines-1)	scroll->yselect = Nlines-1;


	// is maybe better off in a separate routine but well ...
//	if ( Nlines > Nshow )
		scroll->set(0, scroll->y, 1, Nlines, 1, Nshow);
//	else
//		scroll->set(0, scroll->y, 1, 1, 1, 1);

}

// go from line-coordinate to bitmap coordinate
void TextInfo::getxy(int charpos, int *x, int *y)
{
	int iline;

	iline = 0;
	while (iline+1 < Nlines && linestart[iline+1] <= charpos)
		++iline;

	*y = iline * Htxt;

	// on this line, find the x-position...

	int n, len;

	n = linestart[iline];
	len = 0;

	while (n < charpos)
	{

		char txt[2];
		txt[0] = textinfo[n];
		txt[1] = 0;

		len += text_length(usefont, txt);

		++n;
	}

	*x = len;
}



// map coordinate to character number
int TextInfo::getcharpos(int x, int y)
{
	int iline;

	iline = y / Htxt;
	if (iline > Nlines-1)
		iline = Nlines-1;

	if (iline < 0)
	{
		tw_error("getcharpos : Nlines < 0 should not happen");
	}

	// the last line should be handled with care ... (you don't know it's length, only that it stops)
	if (iline == Nlines-1)
	{
		int n;
		n = linestart[Nlines] + x;
		if (n > (int)strlen(textinfo))
			n = strlen(textinfo);

		return n;
	}

	// on this line, find the char-position left-closest to the x value

	int n, len;

	len = 0;

	for ( n = linestart[iline]; n < linestart[iline+1]-1; ++n )
	{
		char txt[2];
		txt[0] = textinfo[n];
		txt[1] = 0;

		len += text_length(usefont, txt);

		if (len > x)
			return n;
	}


	return n;
}



void TextInfo::changeline(int *charpos, int line1, int line2)
{

	if (line2 > Nlines-1)
		line2 = Nlines-1;
	if (line1 > Nlines-1)
		line1 = Nlines-1;

	int d;
	d = *charpos - linestart[line1];
	*charpos = linestart[line2] + d;

		
	if (*charpos > (int)strlen(textinfo))
		*charpos = strlen(textinfo);
	else
	if (line2+1 < Nlines)
		if (*charpos >= linestart[line2+1])
				*charpos = linestart[line2+1] - 1;
}





TextEditBox::TextEditBox(AreaReserve *menu, char *identbranch, int ax, int ay, FONT *afont,
							char *atext, int amaxtext)
:
AreaTabletScrolled(menu, identbranch, ax, ay, 255)
{
	usefont = afont;
	text = atext;
	maxchars = amaxtext;	// a short line?

	textinfo = new TextInfo(afont, drawarea, text, maxchars);
	textinfo->reset(&scroll);

	charpos = strlen(textinfo->textinfo);

	int i;
	for ( i = 0; i < KEY_MAX; ++i )
		keypr[i] = key[i];

	repeattime = 250;

	lasttime = 0;
	lastpressed = -1;

	//scroll.set(0, 0, 1, 0, 1, 1);

	//scrollcontrol.ver(this, identbranch, &scroll);


	int x, y;
	textinfo->getxy(charpos, &x, &y);
	y /= textinfo->Htxt;
	scroll.yselect = y;

	text_color = 0;
}



TextEditBox::~TextEditBox()
{
	delete textinfo;
}


void TextEditBox::set_textcolor(int c)
{
	text_color = c;
}




void TextEditBox::set_repeattime(int atime)
{
	repeattime = atime;
}


// if the mouse is clicked within the window:
// if the mouse button was pressed .. update text pos to current mouse cursor pos.
void TextEditBox::handle_lpress()
{

	int ix, iy;
	
	iy = mouse.pos.y - y;
	ix = mouse.pos.x - x;

	iy += scroll.y * textinfo->Htxt;	// scroll->y = 1st item at the top

	// shouldn't occur:
	if (iy < 0)
		iy = 0;
	if (ix < 0)
		ix = 0;

	if (iy < textinfo->Nlines * textinfo->Htxt)
	{
		//scroll.yselect = iy;
		charpos = textinfo->getcharpos(ix, iy);
	}

	int xs, ys;
	textinfo->getxy(charpos, &xs, &ys);
	ys /= textinfo->Htxt;
	scroll.set_sel(0, ys);
}


void TextEditBox::text_reset(char *newtext)
{
	textinfo->textinfo = newtext;
	text = newtext;
	text_reset();
}

void TextEditBox::text_reset()
{

	textinfo->reset(&scroll);
	
	// check if charpos still has an acceptable position.
	// note that Nchars does not include the zero at the end of the line...
	if (charpos > textinfo->Nchars)
		charpos = textinfo->Nchars;

	int xs, ys;
	textinfo->getxy(charpos, &xs, &ys);
	ys /= textinfo->Htxt;
	//scroll.yselect = ys;
	scroll.set_sel(0, ys);
	//scroll.calculate(); dangerous; buttons are handled TWICE in that case !!
	
}



// this is, where text is detected and entered ... I think ....
void TextEditBox::calculate()
{
	//message.print(1500, 12, "Checking keys");
	//strcpy(text, "hello");

	AreaTablet::calculate();

	int ys;
	ys = scroll.yselect;

	// perhaps this could happen if someone changes the text pointer.
	if (ys < 0)
		ys = 0;
	if (ys > textinfo->Nlines-1)
		ys = textinfo->Nlines-1;

	scroll.calculate();

	// if scrolled, then update the position in the text
	if (scroll.yselect > textinfo->Nlines-1)
		scroll.yselect = textinfo->Nlines-1;

	if (scroll.yselect < 0)
		scroll.yselect = 0;

	if (scroll.yselect != ys)
	{
		// update the charpos ...
		textinfo->changeline(&charpos, ys, scroll.yselect);
	}


	if (!flag.focus)
		return;


	int i;

	int prevlastpressed;
	prevlastpressed = lastpressed;

	// find the last key that was pressed
	for ( i = 0; i < KEY_MAX; ++i )
	{
		if (!keypr[i] && key[i])
		{
			lastpressed = i;
			lasttime = 0;
		}
	}

	// is the last known key still pressed?
	if (lastpressed >= 0 && !key[lastpressed])
		lastpressed = -1;

	// if a key is pressed, then
	selected = false;
	if (lastpressed >= 0)
	{
		// use this key to enter text
		int k = scancode_to_ascii(lastpressed);		// normal character format

		// force immediate response to a new key, and reset repeattime for the key
		if (lastpressed != prevlastpressed)
			lasttime = 0;

		if (areareserve->menu_time - lasttime > repeattime)	// 100 ms
		{
			lasttime = areareserve->menu_time;


			if ( (k >= 0x020 && k < 0x080) || (lastpressed == KEY_ENTER) )
			{
				if (key[KEY_LSHIFT])
				{
					// create the capital letter.
					char txt[2];
					txt[0] = k;
					txt[1] = 0;
					strupr(txt);
					k = txt[0];

					if (k == '1') k = '!';
					if (k == '2') k = '@';
					if (k == '3') k = '#';
					if (k == '4') k = '$';
					if (k == '5') k = '%';
					if (k == '6') k = '^';
					if (k == '7') k = '&';
					if (k == '8') k = '*';
					if (k == '9') k = '(';
					if (k == '0') k = ')';

					if (k == '-') k = '_';
					if (k == '=') k = '+';
					if (k == ';') k = ':';
					if (k == '\'') k = '\"';
					if (k == ',') k = '<';
					if (k == '.') k = '>';
					if (k == '/') k = '?';
					if (k == '`') k = '~';
				}

				if (lastpressed == KEY_ENTER)
					k = '\n';
				
				//message.print(1500, 12, "adding key [%c]", k);

				// insert a character
				memmove(&text[charpos+1], &text[charpos], (maxchars-1)-charpos-1);
				text[charpos] = k;
				if (charpos < (int)strlen(text))
					++charpos;
				//text[charpos] = 0;
				
			} else if ( lastpressed == KEY_BACKSPACE && charpos > 0 )
			{
				// delete the previous character 
				int m;
				m = 1;

				// if it's a return, then delete that as well
				if (text[charpos-1] == '\n' && charpos > 1)
					++m;

				memmove(&text[charpos-m], &text[charpos], (maxchars-m)-charpos);
				text[maxchars-m] = 0;

				charpos -= m;
				
			} else if ( lastpressed == KEY_DEL && text[charpos] != 0 )
			{
				// delete the current character (if it's not the closing zero)
				int m;
				m = 1;

				// if it's a return, then delete that as well
				if (text[charpos+1] == '\n')
					++m;

				memmove(&text[charpos], &text[charpos+m], (maxchars-m)-charpos);
				text[maxchars-m] = 0;
				
			} else if ( lastpressed == KEY_LEFT )
			{
				if (charpos > 0 )
					--charpos;

				//if (charpos > 0 && text[charpos] == '\n')
				//	--charpos;
			} else if ( lastpressed == KEY_RIGHT )
			{
				if (charpos < (int)strlen(textinfo->textinfo))
					++charpos;

				//if (charpos  < strlen(textinfo->textinfo) && text[charpos] == '\n')
				//	++charpos;
			} else if ( lastpressed == KEY_UP )
			{
				// update the charpos ...
				int ys;
				ys = scroll.yselect - 1;
				if (ys < 0)
					ys = 0;
				textinfo->changeline(&charpos, scroll.yselect, ys);
			} else if ( lastpressed == KEY_DOWN )
			{
				// update the charpos ...
				int ys;
				ys = scroll.yselect + 1;
				if (ys > textinfo->Nlines-1)
					ys = textinfo->Nlines-1;
				textinfo->changeline(&charpos, scroll.yselect, ys);
			}
			// else if ( lastpressed == KEY_ENTER)
			//	selected = true;
			
		}

		text_reset();
			
	}


	for ( i = 0; i < KEY_MAX; ++i )
		keypr[i] = key[i];



}


void TextEditBox::subanimate()
{
	// background?
	//blit(backgr, areareserve->drawarea, 0, 0, x, y, W, H);

		// well, draw the text ;) Use auto-indent ?
	// or well, use the lines information ?

	// starting line ...



//	if (scroll.yselect > scroll.Ny)
//		scroll.yselect = scroll.Ny;

	// also do some manipulation of scroll-y, and scroll-??, so that you
	// get immediate response when using a button:
	//scroll.y = scroll.yselect;
	//if (scroll.yselect > scroll.y)
//		scroll.y = scroll.yselect;	// this removes the extra amount of freedom you had

	text_mode(-1);

	int i;
	for ( i = 0; i < textinfo->Nshow; ++i )
	{
		int iline;
		iline = scroll.y + i;

		if ( iline < 0 || iline > textinfo->Nlines-1 )	// is the following correct? -> Nline-1 is not a line; it indicates the last byte + 1
			continue;

		// first character of the line
		int n, L;
		n = textinfo->linestart[iline];

		// number of character till the start of the next line
		if ( iline < textinfo->Nlines-1 )
			L = textinfo->linestart[iline+1] - n - 1;
		else
			L = strlen(&(textinfo->textinfo[n]));


		// make a copy of this line (and add a 0 ?)
		char txt[128];

		if (L > 127)
			L = 127;

		strncpy(txt, &(textinfo->textinfo[n]), L);
		txt[L] = 0;

		// filter the text a little...

		int k;
		for ( k = 0; k < L; ++k )
		{
			if (txt[k] < 20 || txt[k] > 128 )
				txt[k] = ' ';
		}

		textout(drawarea, usefont, txt, 0, (iline - scroll.y)*textinfo->Htxt, text_color);
	}


	// draw a line at "charpos" ... but how ??

	int xc, yc;
	textinfo->getxy(charpos, &xc, &yc);
	yc /= textinfo->Htxt;
	//yc = scroll.yselect;
	
	int h;
	h = textinfo->Htxt;
	yc -= scroll.y;

	if ( (int(areareserve->menu_time * 1000) % 100) < 50)
		line (drawarea, xc, yc*h, xc, (yc+1)*h, makecol(0,0,0));
	
}





SwitchButton::SwitchButton(AreaReserve *menu, char *identbranch, int ax, int ay, int asciicode)
:
AreaBox(menu, identbranch, ax, ay, asciicode)
{
	char stron[128], stroff[128];

	strcpy(stron,  ident);
	strcpy(stroff, ident);

	strcat(stron, "/on");
	strcat(stroff,"/off");


	// both of these bitmaps must be present !!
	bmp_on = areareserve->bmp(stron);
	bmp_off = areareserve->bmp(stroff);
	if (!(bmp_on || bmp_off))
	{
		tw_error("Switchbutton misses one or more bitmaps");
	}

	W = bmp_on->w;		// note: it's already scaled on initialization.
	H = bmp_on->h;

	// search for the location of the "on" bitmap
	if ( ax == -1 && ay == -1 && bmp_on )
		locate_by_backgr(stron);

	state = false;
}

SwitchButton::~SwitchButton()
{
	if (bmp_on)
		destroy_bitmap(bmp_on);
	if (bmp_off)
		destroy_bitmap(bmp_off);
}

void SwitchButton::draw_default()
{
	if (state)
		draw_on();
	else
		draw_off();
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

void SwitchButton::draw_on()
{
	BITMAP *b = bmp_on;
	if (b)
		blit(b, areareserve->drawarea, 0, 0, x, y, b->w, b->h);
}

void SwitchButton::draw_off()
{
	BITMAP *b = bmp_off;
	if (b)
		blit(b, areareserve->drawarea, 0, 0, x, y, b->w, b->h);
}

void SwitchButton::calculate()
{
	AreaBox::calculate();

	// determine if the state of the button is being changed by (some) interaction:
	if (flag.left_mouse_press)
		state = !state;	// switch state.
}


bool SwitchButton::hasmouse()
{
	// the first rough check whether it's in the boxed bitmap area
	if (AreaBox::hasmouse())
	{
		// now check the bitmap, if the mouse is touching a non-transparent sprite.
		// only check the default bitmap (buttons that change size aren't cool)
		return check_pixel(bmp_on, mouse.xpos()-x, mouse.ypos()-y);

	} else
		return false;
}









TextButtonList::TextButtonList(AreaReserve *menu, char *identbranch, int ax, int ay, FONT *afont,
								scrollpos_str *ascroll = 0)
:
AreaTablet(menu, identbranch, ax, ay, 255)
{
	usefont = afont;
	Htxt = text_height(usefont);
	text_color = makecol(0,0,0);

	yselected = 0;

	optionlist = 0;
	N = 0;

	selected = false;

	scroll = ascroll;

	scroll->set(0, 0, 1, 0, 1, 1);

	Nshow = int(H / Htxt) - 1;		// -1, because item 0 is also shown...

}


TextButtonList::~TextButtonList()
{
	
	clear_optionlist();
}


void TextButtonList::set_selected(int iy)
{
	yselected = iy;
}

void TextButtonList::clear_optionlist()
{
	int i;

	if (optionlist)			// delete an existing set of strings first.
	{
		for ( i = 0; i < scroll->Ny; ++i )
			delete optionlist[i];	// delete the strings pointed at

		delete optionlist;			// delete the pointers
	}

	scroll->Ny = 0;
}


void TextButtonList::set_optionlist(char **aoptionlist, int color)
{
	int aN;

	// in this case, where the number of elements in the list isn't explicitly
	// specified, I search for a null-element that indicates the last element
	// in the list:
	aN = 0;

	if (aoptionlist)
		while ( aoptionlist[aN] != 0 )
			++aN;

	set_optionlist(aoptionlist, aN, color);
}

void TextButtonList::set_optionlist(char **aoptionlist, int aN, int color)
{
	int i;

	clear_optionlist();		// note that this resets N .

	N = aN;
	if (N == 0)
		return;				// in case there's an empty list

	optionlist = new char* [N];	// reserve space for that many pointers to strings.

	for ( i = 0; i < N; ++i )
	{
		optionlist[i] = new char[strlen(aoptionlist[i]) + 1 ];
		strcpy(optionlist[i], aoptionlist[i]);
	}

	//if (N > Nshow)
	scroll->set(0, 0, 1, N , 1, Nshow);
	//else
	//	scroll->set(0, 0, 1, 1, 1, Nshow);

}


// if the mouse is clicked within the window:
void TextButtonList::handle_lpress()
{
	selected = true;
}


int TextButtonList::getk()
{
	// select the corresponding item:

	int iy;
	
	iy = mouse.pos.y - y;

	iy /= Htxt;		// # of item relative to the top

	iy += scroll->y;	// scroll->y = 1st item at the top

	if (iy < N)
	{
		yselected = iy;
	}

	return yselected;
}

// select and center the list on "that" item
void TextButtonList::handle_rpress()
{
	
	int iy;
	
	iy = mouse.pos.y - y;

	iy /= Htxt;		// # of item relative to the top

	iy += scroll->y;	// yshift = 1st item at the top

	iy -= Nshow/2;	// center in the window
	if (iy <= 0)
		iy = 0;

	scroll->y = iy;
}


void TextButtonList::subanimate()
{

	if (!optionlist || N == 0)
		return;				// if it's an empty list

	int ix, iy;
	int i;


	// check if there's need for shifting the list, i.e., if the scroll is off
	// the map:
	
	// note, a positive "shift" means, a number of items are skipped at the top,
	// when drawing them:


	// else, do nothing ;)					// selected item is somewhere in the middle.


	for ( i = scroll->y; i < N; ++i )
	{
		ix = 0;
		iy = Htxt * (i - scroll->y);

		int c;

		if ( i != yselected )
		{
			text_mode(-1);
			c = text_color;
		} else {
			text_mode( makecol(0,0,0) );
			c = makecol(255,255,255);
		}

		if (optionlist[i])// && strlen(optionlist[i]) < 20)
			textout(drawarea, usefont, optionlist[i], ix, iy, c);
	}
	

}








ScrollBar::ScrollBar(AreaReserve *menu, char *identbranch, int ax, int ay)
:
AreaTablet(menu, identbranch, ax, ay, 255)
{
	relpos = 0.0;	// between 0 and 1

	button = getbmp("/button");

	if (button)
	{
		bwhalf = button->w/2; 
		bhhalf = button->h/2;
	} else {
		bwhalf = 0;
		bhhalf = 0;
	}

	if (H >= W)
		direction = ver;	// vertically oriented
	else
		direction = hor;	// horizontally oriented

	if (direction == ver)
	{
		pmin = bhhalf;
		pmax = H - bhhalf;
	} else {
		pmin = bwhalf;
		pmax = W - bwhalf;
	}

	pbutton = pmin;
}


ScrollBar::~ScrollBar()
{
	if (button)
		destroy_bitmap(button);
}


void ScrollBar::handle_lhold()
{
	if (direction == ver)
		pbutton = mouse.ypos() - y;		// mouse pos relative in the little bar area
	else
		pbutton = mouse.xpos() - x;
	
	if (pbutton < pmin)
		pbutton = pmin;
	
	if (pbutton > pmax)
		pbutton = pmax;
	
	relpos = double(pbutton - pmin) / double(pmax - pmin);
}


void ScrollBar::subanimate()
{
	if (direction == ver)
		masked_blit(button, drawarea, 0, 0, W/2 - bwhalf, pbutton-bhhalf, button->w, button->h);
	else
		masked_blit(button, drawarea, 0, 0, pbutton-bwhalf, H/2 - bhhalf, button->w, button->h);
}


void ScrollBar::setrelpos(double arelpos)
{
	if (relpos == arelpos)
		return;

	relpos = arelpos;

	// also update the button position to reflect this change
	pbutton = pmin + iround(relpos * (pmax - pmin));
}











TextInfoArea::TextInfoArea(AreaReserve *menu, char *identbranch, int ax, int ay, FONT *afont)
:
AreaTabletScrolled(menu, identbranch, ax, ay, 255)
{
	usefont = afont;
	Htxt = text_height(usefont);
	text_color = makecol(0,0,0);

	//scroll = ascroll;

	scroll.set(0, 0, 1, 0, 1, 1);

	Nshow = int(H / Htxt) - 1;		// -1, because item 0 is also shown...

	textinfo = 0;
}

TextInfoArea::~TextInfoArea()
{
	delete textinfo;
}

void TextInfoArea::set_textinfo(char *atextinfo, int Nchars)
{
	// make a copy of the textinfo ...

	if (textinfo)
		delete textinfo;	// but first, delete existing text

	textinfo = new char [Nchars+1];
	memcpy(textinfo, atextinfo, Nchars);
	textinfo[Nchars] = 0;

	// "initialize" the text:
	int n, len;
	n = 0;
	len = 0;

	Nlines = 0;

	linestart[0] = 0;
	++Nlines;
	++n;

	while ( n < Nchars )
	{
		if (textinfo[n] == 0 || textinfo[n] == '\n')
		{
			linestart[Nlines] = n+1;
			++Nlines;

			len = 0;

		} else {
			char txt[2];
			txt[0] = textinfo[n];
			txt[1] = 0;

			len += text_length(usefont, txt);

			// if the line exceeds the window width, then you've to look
			// back for the last space
			if (len >= W)
			{
				while (n > 0 && textinfo[n] != ' ' && textinfo[n] != '.' && textinfo[n] != ',' &&
						textinfo[n] != '!' && textinfo[n] != '?' && textinfo[n] != ';' &&
						textinfo[n] != ':' && textinfo[n] != '-' && textinfo[n] != '/' &&
						 textinfo[n] != '\n' && textinfo[n] != 0 )
					--n;

				linestart[Nlines] = n+1;
				++Nlines;

				len = 0;
			}
		}

		if (Nlines >= maxlines)
			break;

		++n;
	}


	if ( Nlines > Nshow )
		scroll.set(0, 0, 1, Nlines, 1, Nshow);
	else
		scroll.set(0, 0, 1, 1, 1, 1);
}



void TextInfoArea::subanimate()
{
	// well, draw the text ;) Use auto-indent ?
	// or well, use the lines information ?

	// starting line ...

	// also do some manipulation of scroll-y, and scroll-??, so that you
	// get immediate response when using a button:

	if (scroll.yselect > scroll.Ny)
		scroll.yselect = scroll.Ny;

	if (scroll.yselect > scroll.y)
		scroll.y = scroll.yselect;	// this removes the extra amount of freedom you had

	int i;
	for ( i = 0; i < Nshow; ++i )
	{
		int iline;
		iline = scroll.y + i;

		if ( iline < 0 || iline > Nlines-1 )	// is the following correct? -> Nline-1 is not a line; it indicates the last byte + 1
			continue;

		// first character of the line
		int n, L;
		n = linestart[iline];

		// number of character till the start of the next line
		if ( iline < Nlines-1 )
			L = linestart[iline+1] - n - 1;
		else
			L = strlen(&textinfo[n]);


		// make a copy of this line (and add a 0 ?)
		char txt[128];

		if (L > 127)
			L = 127;

		strncpy(txt, &textinfo[n], L);
		txt[L] = 0;

		// filter the text a little...

		int k;
		for ( k = 0; k < L; ++k )
		{
			if (txt[k] < 20 || txt[k] > 128 )
				txt[k] = ' ';
		}

		textout(drawarea, usefont, txt, 0, (iline - scroll.y)*Htxt, text_color);
	}

}







// This popup is invoked by a push-button somewhere. I'm not going to create some
// special push button type for that, I can just as well let this (new) class
// access the state of that general button type.


PopupGeneral::PopupGeneral(char *identbranch, int axshift, int ayshift, BITMAP *outputscreen)
:
AreaReserve(identbranch,
			0, 0,
			outputscreen)
{
	trigger = 0;

	init_components(identbranch);

	xshift = iround(axshift * scale);
	yshift = iround(ayshift * scale);
}



PopupGeneral::PopupGeneral(AreaGeneral *atrigger, char *identbranch, int axshift, int ayshift)
:
AreaReserve(identbranch,
			0.0, 0.0,
			atrigger->areareserve->screenreserve)
{
	trigger = atrigger;

	init_components(identbranch);

	xshift = iround(axshift * scale);
	yshift = iround(ayshift * scale);
}



void PopupGeneral::init_components(char *id)
{
	movingthereserve = false;
	returnvalueready = false;

	// create the buttons and bitmaps, finding graphics in some data file,
	// with strict name conventions having as base the general identifier "ident".
	//
	// note that the buttons required, and their layout, are determined by the
	// graphics in the datafile.

	//scroll_control = new scrollpos_str();
	scrollcontrol.hor(this, "scroll", &scroll);
	scrollcontrol.ver(this, "scroll", &scroll);

	/*
	left = 0;
	right = 0;

	left	= new Button(this, "left", -1, -1, KEY_LEFT);
	if (!left->W) {	delete left;	left = 0;	}

	right	= new Button(this, "right", -1, -1, KEY_RIGHT);
	if (!right->W)	{	delete right;	right = 0;	}

	up		= new Button(this, "up", -1, -1, KEY_UP);
	if (!up->W)	{	delete up;	up = 0;	}

	down	= new Button(this, "down", -1, -1, KEY_DOWN);
	if (!down->W)	{	delete down;	down = 0;	}


	// add a scroll bar
	scrollhor = new ScrollBar(this, "scrollbar_hor", -1, -1);
	if (!scrollhor->W)	{	delete scrollhor;	scrollhor = 0;	}

	scrollvert = new ScrollBar(this, "scrollbar_vert", -1, -1);
	if (!scrollvert->W)	{	delete scrollvert;	scrollvert = 0;	}

	// add scroll control
	scroll_control->bind(left, right, up, down, scrollhor, scrollvert);
	*/

	// and default option settings:
	option.disable_othermenu = true;
	option.place_relative2mouse = true;
}


PopupGeneral::~PopupGeneral()
{
	/*
	if (left)
		delete left;
	if (right)
		delete right;
	if (up)
		delete up;
	if (down)
		delete down;

	if (scrollhor)
		delete scrollhor;
	if (scrollvert)
		delete scrollvert;
	if (scroll_control)
		delete scroll_control;
		*/

}


void PopupGeneral::calculate()
{
	// must be called before the "return", otherwise focus-loss isn't called
	AreaReserve::calculate();

	// first, do some tests for activation, that should always be done.
	// if you press the trigger, this menu is activated
	// (if a trigger isn't needed, but some more forcefull external control,
	// trigger==0)
	if ( trigger && disabled && trigger->flag.left_mouse_hold )
	{
		if (option.disable_othermenu)
		{
			trigger->areareserve->disable();
			trigger->areareserve->handle_focus_loss();
		}
		this->show();		// this also resets the mouse position and old position, so that it has
							// the most recent value (since it hasn't been updated by the
							// calculate funtion for a while).

		// but where exactly : near the mouse !!
		if (option.place_relative2mouse)
		{
			x = trigger->areareserve->x + trigger->mouse.pos.x + xshift;
			y = trigger->areareserve->y + trigger->mouse.pos.y + yshift;
		} else {
			x = trigger->areareserve->x + xshift;
			y = trigger->areareserve->y + yshift;
		}

		// don't let the menu go off-screen:
		
		if ( x+W > screenreserve->w )
			x = screenreserve->w - W;
		if ( x < 0 )
			x = 0;
		
		if ( y+H > screenreserve->h )
			y = screenreserve->h - H;
		if ( y < 0 )
			y = 0;
		
		
	}

	if (disabled)
		return;

	// also, check if the mouse is clicked, and dragged - in that case, move the menu:
	// but, only if it's clicked in a part that is not a button ;)
	
	if ( mouse.left.press() && hasmouse() )
	{
		int i;
		for ( i = 0; i < Nareas; ++i )
			if (area[i]->hasmouse())
				break;

		if ( i == Nareas )
			movingthereserve = true;
	}

	if (!mouse.left.hold())
		movingthereserve = false;


	if ( movingthereserve )
	{
		int dx, dy;
		dx = mouse.vx();
		dy = mouse.vy();
		x += dx;
		y += dy;
		mouse.move(-dx, -dy);

	}

	// must be called before the "return", otherwise focus-loss isn't called
//	AreaReserve::calculate();



	// check the scroll controls

	scroll.calculate();
	

	check_end();
}


// if you leave the area, it's auto-hiding -- but only if you're not keeping the
// left-key pressed, which means you're moving the thing - when you move *fast*, the
// mouse pointer can exit the area before the area follows the mouse ...

void PopupGeneral::handle_focus_loss()
{
	if (mouse.left.hold())
	{
		hasfocus = true;
		return;
	}

	check_end();
}

void PopupGeneral::close(int areturnstatus)
{
	if (trigger)
		trigger->areareserve->enable();

	this->hide();

	returnstatus = areturnstatus;
	returnvalueready = true;
}


// you can (and should) replace this routine with one suited for your own purposes
void PopupGeneral::check_end()
{
	if (!hidden && !hasfocus)	// to prevent this from being called twice, due to recursive call to handle_focus() in hide()
	{
		//tw_error("PopupList : Losing focus !!");
		AreaReserve::handle_focus_loss();
		close(-1);		// nothing ;)
	}
}


int PopupGeneral::getvalue()
{
	if (!returnvalueready)
		return -1;				// otherwise it's undefined

	returnvalueready = false;
	return returnstatus;
}


void PopupGeneral::enable()
{
	AreaReserve::enable();
	returnvalueready = false;
}



void PopupGeneral::newscan()
{
	show();	// (more general than enable)
	returnvalueready = false;
}






// make use of this textinfo box, in some "controlled" menu space:

PopupTextInfo::PopupTextInfo(AreaGeneral *creator, char *ident, int axshift, int ayshift,
							 FONT *afont, char *atext, int aNchar)
:
PopupGeneral(creator, ident, axshift, ayshift)
{
	// add a text list box
	tia = new TextInfoArea(this, "text", -1, -1, afont);//, &scroll);
	tia->set_textinfo(atext, aNchar);

	doneinit();	// closes the datafile
	hide();		// cause it needs a trigger to activate
}

PopupTextInfo::~PopupTextInfo()
{
	if (tia)
		delete tia;
}







PopupTextInfo_toggle::PopupTextInfo_toggle(AreaGeneral *creator, char *ident, int axshift, int ayshift,
	FONT *afont, char *atext, int aNchar)
:
PopupTextInfo(creator, ident, axshift, ayshift, afont, atext, aNchar)
{
};



void PopupTextInfo_toggle::calculate()
{
	PopupTextInfo::calculate();
}

void PopupTextInfo_toggle::check_end()
{
};






// This popup is invoked by a push-button somewhere. I'm not going to create some
// special push button type for that, I can just as well let this (new) class
// access the state of that general button type.

PopupList::PopupList(AreaGeneral *atrigger, char *ident, int axshift, int ayshift,
						FONT *afont, char **aoptionslist)
:
PopupGeneral(atrigger, ident, axshift, ayshift)
{
	//optionslist = aoptionslist;

	// add a text list box
	tbl = new TextButtonList(this, "TEXT", -1, -1, afont, &scroll);
	tbl->set_optionlist(aoptionslist, makecol(0,0,0));

	doneinit();	// closes the datafile
	hide();		// cause it needs a trigger to activate
}


PopupList::~PopupList()
{

	// delete the list
	if (tbl)
		delete tbl;
}


// this calls close with return value
void PopupList::check_end()
{
	PopupGeneral::check_end();		// this is also a way to end (without choosing anything)

	if (tbl->selected)
	{
		tbl->selected = false;
		close(tbl->getk());
	}
}






PopupFleetSelection::PopupFleetSelection(char *ident, int axshift, int ayshift,
						char *datafilename, BITMAP *outputscreen,
						BITMAP **alistIcon, double ascale, FONT *afont )
:
PopupGeneral(ident, axshift, ayshift, outputscreen)
{

	icons = new MatrixIcons(this, "ICON", -1, -1, &scroll, KEY_ENTER);

	// this auto-configures the data as well ...
	icons->set_iconinfo(alistIcon, ascale);

	info = new TextButton(this, "INFO", -1, -1, afont);
	oncerandom = new Button(this, "RANDOM", -1, -1, KEY_R);
	alwaysrandom = new Button(this, "ALWAYSRANDOM", -1, -1, KEY_A);

	doneinit();	// closes the datafile
	hide();		// cause it needs a trigger to activate
}








PopupFleetSelection::~PopupFleetSelection()
{
	if (icons)
		delete icons;
	if (info)
		delete info;
	if (oncerandom)
		delete oncerandom;
	if (alwaysrandom)
		delete alwaysrandom;
}

// this calls close with return value
// (the general close is ok, it closes and hides the menu).
void PopupFleetSelection::check_end()
{
	if (icons->selected)
		close(icons->getk());
}



void PopupFleetSelection::newscan(BITMAP **alistIcon, double ascale, char *txt)
{
	PopupGeneral::newscan();
	icons->set_iconinfo(alistIcon, ascale);
	info->set_text(txt, makecol(255,255,0));
}





MatrixIcons::MatrixIcons(AreaReserve *menu, char *identbranch, int ax, int ay, scrollpos_str *ascroll, int akey)
:
AreaTablet(menu, identbranch, ax, ay, akey)
{

	scroll = ascroll;

	scroll->set(0, 0, 1, 1, 1, 1);

	// obtain the overlay ... this defines the width/height of each matrix area

	overlay = getbmp("/overlay");
	if (!overlay)
	{
		tw_error("MatrixIcons : overlay is missing");
	}

	tmp = create_bitmap(overlay->w, overlay->h);

	maxitems = 0;
	Nx = 0;
	Ny = 0;

	Wicon = overlay->w;
	Hicon = overlay->h;

	Nxshow = iround( double(W) / double(Wicon) );
	Nyshow = iround( double(H) / double(Hicon) );

	itemproperty = 0;
}


MatrixIcons::~MatrixIcons()
{
	if (overlay)
		destroy_bitmap(overlay);
	if (tmp)
		destroy_bitmap(tmp);
	if (itemproperty)
		delete itemproperty;
}


void MatrixIcons::set_iconinfo(BITMAP **alistIcon, double ascale)
{
	listIcon = alistIcon;

	// do nothing, if there are no data (doh)
	if (!listIcon)
		return;

	// should be null-terminated; find out how many there are
	maxitems = 0;
	while (listIcon[maxitems])
		++maxitems;
	
	// create the most "optimal" square map (minimum x,y positions means least search trouble (I think)).
	Nx = iround( sqrt(maxitems) );
	Ny = maxitems/Nx + 1;

	if (Nx*Ny >= maxitems + Nx)
		--Ny;

	if ( Nx == 0 || Ny == 0 )
	{
		tw_error("PopupFleetSelection : no bitmap data");
	}

	scroll->set(0, 0, Nx, Ny, Nxshow, Nyshow);

	selected = false;

	extrascale = ascale;

	if (itemproperty)
		delete itemproperty;

	itemproperty = new int [maxitems];
	int i;
	for ( i = 0; i < maxitems; ++i )
	{
		itemproperty[i] = ENABLE;
	}

}


// this is done just after mouse/key update, but before the handle* routines.
void MatrixIcons::subcalculate()
{

	// additionally, if there's mouse movement within this region, you should
	// override settings of the scroll menu, namely, which particular icon is
	// selected
	if (hasmouse() && mouse.vx() && mouse.vy())
	{
		// control is handled by the mouse
		scroll->xselect = scroll->x + (mouse.xpos() - x) / Wicon;	// mouse coordinate within the matrix area
		scroll->yselect = scroll->y + (mouse.ypos() - y) / Hicon;
	}

}

void MatrixIcons::subanimate()
{

	int i, j;
	int ix, iy;

	for (iy = scroll->y; iy < scroll->y + Nyshow && iy < Ny; ++iy )
	{

		for (ix = scroll->x; ix < scroll->x + Nxshow && ix < Nx; ++ix )
		{

			int xoverlay, yoverlay;
			int xicon, yicon, k;

			k = iy*Nx + ix;

			if (k >= maxitems)	// if you've run out of bitmaps that you can plot...
				break;

			xoverlay = (ix - scroll->x) * Wicon;
			yoverlay = (iy - scroll->y) * Hicon;

			int w0, h0;
			w0 = iround(listIcon[k]->w * areareserve->scale * extrascale);
			h0 = iround(listIcon[k]->h * areareserve->scale * extrascale);

			// create a intermediate icon
			xicon =  (Wicon - w0) / 2;
			yicon =  (Hicon - h0) / 2;

			clear_to_color(tmp, makecol(255,0,255));

			if (listIcon[k] && (itemproperty[k] & ENABLE) )
			{

				masked_stretch_blit(listIcon[k], tmp,
					0, 0, listIcon[k]->w, listIcon[k]->h,
					xicon, yicon, w0, h0 );
			}

			masked_blit(overlay, tmp, 0, 0, 0, 0, overlay->w, overlay->h );

			// blit the combined image onto the panel area
			masked_blit(tmp, drawarea, 0, 0, xoverlay, yoverlay, overlay->w, overlay->h );
		}
	}

	// one of them is highlighted ... the one which the mouse is pointed at: draw
	// a box around it ?
	i = scroll->xselect - scroll->x;//mx / Wicon;
	j = scroll->yselect - scroll->y;//my / Hicon;
	double a;
	//a = 0.5 + 0.5 * sin(areareserve->menu_time * 1E-3 * 2*PI / 10);
	a = 0.5;
	rect(drawarea, i*Wicon, j*Hicon, (i+1)*Wicon-1, (j+1)*Hicon-1, makecol(20*a,100*a,200*a));
}


// this action is used to scroll left/right/up/down
void MatrixIcons::handle_rpress()
{
	int mx, my;
	
	// mouse position relative to the center of the item window:
	mx = mouse.xpos() - x - W / 2;
	my = mouse.ypos() - y - H / 2;

	// velocity depends on how far you're away from the center.
	scroll->add(mx / (W/8), my / (H/8));
}


// select some icon :
void MatrixIcons::handle_lpress()
{
	selected = true;
}


int MatrixIcons::getk()
{
	int k;
	k = scroll->xselect + scroll->yselect * Nx;

	if ( k > maxitems-1 )
		k = maxitems-1;

	return k;
}




/*
// let the "main" routine determine whether to stop or not ;)
void PopupFleetSelection::check_end()
{
	if (icons->selected)
		close(icons->getk());
}
*/





// here, the background is initialized (by areareserve)
Popup::Popup(char *ident, int xcenter, int ycenter, BITMAP *outputscreen,
						bool inherited)
:
AreaReserve(ident, xcenter, ycenter, outputscreen)
{
	center(xcenter, ycenter);		// center around this position, in relative coordinates

	returnvalueready = false;

	if (!inherited)		// cause closing the data file should only be done at the highest level.
	{					// but it should be done, always...
		doneinit();		// with this check, you can build new classes on top of this (always do that check)
		hide();
	}
}

Popup::~Popup()
{
}

// this calls close with return value
// (the general close is ok, it closes and hides the menu).
void Popup::check_end()
{
	// nothing
}


void Popup::calculate()
{
	// must be called before the "return", otherwise focus-loss isn't called
	AreaReserve::calculate();

	check_end();
}




void Popup::close(int areturnstatus)
{
	hide();

	returnstatus = areturnstatus;
	returnvalueready = true;
}


int Popup::getvalue()
{
	if (!returnvalueready)
		return -1;				// otherwise it's undefined

	returnvalueready = false;
	return returnstatus;
}


void Popup::enable()
{
	AreaReserve::enable();
	returnvalueready = false;
}







// here, buttons are initialized
PopupYN::PopupYN(char *ident, int xcenter, int ycenter,
							BITMAP *outputscreen,
								bool inherited)
:
Popup(ident,
			xcenter, ycenter,
			outputscreen,
			true)	// it has got a child
{
	yes = new Button(this, "YES", -1, -1, KEY_Y);

	no = new Button(this, "NO", -1, -1, KEY_N);

	if (!inherited)
	{
		doneinit();
		hide();
	}
}

PopupYN::~PopupYN()
{
}

void PopupYN::check_end()
{
	if (yes->flag.left_mouse_press)
		close(1);
	if (no->flag.left_mouse_press)
		close(0);
}







// here, buttons are initialized
PopupOk::PopupOk(char *ident, int xcenter, int ycenter,
							BITMAP *outputscreen,
								bool inherited)
:
Popup(ident,
			xcenter, ycenter,
			outputscreen,
			true)	// it has got a child
{
	ok = new Button(this, "OK", -1, -1, KEY_ENTER);

	if (!inherited)
	{
		doneinit();
		hide();
	}
}

PopupOk::~PopupOk()
{
}

void PopupOk::check_end()
{
	if (ok->flag.left_mouse_press)
		close(1);
}









// an additional class, which has its own background and drawing area, which
// can be used to create custom representations of information (eg., text or smaller
// bitmaps))



AreaTablet::AreaTablet(AreaReserve *menu, char *identbranch, int ax, int ay, int asciicode, bool akeepkey)
:
AreaBox(menu, identbranch, ax, ay, asciicode, akeepkey)
{

	// (auto-) initialize the background and create a drawing area

	bool autoplace = false;
	if (ax == -1 && ay == -1)
		autoplace = true;

	initbackgr(autoplace);		// this also sets W and H.
								// and x,y in case of "autoplace"

	if (W != 0)
		drawarea = create_bitmap_ex(bitmap_color_depth(areareserve->drawarea), W, H);
	else
		drawarea = 0;

}




AreaTablet::~AreaTablet()
{
	if (backgr)
		destroy_bitmap(backgr);
	if (drawarea)
		destroy_bitmap(drawarea);
}


void AreaTablet::initbackgr(bool autoplace)
{
	backgr = getbmp("/backgr");
	
	if (backgr)
	{
		W = backgr->w;		// note: background is already scaled when it's initialized
		H = backgr->h;
	} else {
		//tw_error("Could not initialize background of matrix");
		W = 0;
		H = 0;
	}

	if (autoplace && W != 0)
	{
		char streditbox[128];
		strcpy(streditbox,  ident);
		strcat(streditbox,  "/backgr");

		locate_by_backgr(streditbox);
	}

}


void AreaTablet::changebackgr(char *fname)
{
	//int W, H;
	//W = backgr->w;
	//H = backgr->h;
	
	//destroy_bitmap(backgr);
	// nah, it's only overwritten !!

	//blit(bmp, backgr, 0, 0, 0, 0, bmp->w, bmp->h);
	BITMAP *newb;
	newb = areareserve->bmp(fname);

	if (newb)
	{
		destroy_bitmap(backgr);
		backgr = newb;
	}
}


void AreaTablet::animate()
{
	blit(backgr, drawarea, 0, 0, 0, 0, W, H);

	subanimate();

	blit(drawarea, areareserve->drawarea, 0, 0, x, y, W, H);
}


void AreaTablet::subanimate()
{
	// nothing; you can put extra drawing commands here, stuff that's drawn onto
	// the background before being blitted onto the reserved area.
}


bool AreaTablet::hasmouse()
{
	// the first rough check whether it's in the boxed bitmap area
	if (AreaBox::hasmouse())
	{
		// now check the bitmap, if the mouse is touching a non-transparent sprite.
		// only check the default bitmap (buttons that change size aren't cool)
		return check_pixel(backgr, mouse.xpos()-x, mouse.ypos()-y);

	} else
		return false;
}



AreaTabletScrolled::AreaTabletScrolled(AreaReserve *menu, char *identbranch, int ax, int ay, int asciicode, bool akeepkey)
:AreaTablet(menu, identbranch, ax, ay, asciicode, akeepkey)
{
	scrollcontrol.ver(this, identbranch, &scroll);
}

