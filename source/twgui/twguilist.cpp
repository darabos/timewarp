/*
Placed in public domain by Rob Devilee, 2004. Share and enjoy!
*/

#include <allegro.h>
#ifndef LINUX
#include <winalleg.h>
#endif
#include <stdio.h>
#include <string.h>

#include "twgui/twgui.h"
#include "twwindow.h"

#include "util/round.h"

// ------------ AND NOW THE GRAPHICAL PART ---------------


AreaTabletScrolled::AreaTabletScrolled(TWindow *menu, char *identbranch, int asciicode, bool akeepkey)
:
AreaTablet(menu, identbranch, asciicode, akeepkey)
{
	scroll.setup(mainwindow, identbranch);//, &scroll);
}


int AreaTabletScrolled::gety()
{
	return scroll.yselect;
}


void AreaTabletScrolled::calculate()
{
	AreaTablet::calculate();

	scroll.calculate();
}









TextButton::TextButton(TWindow *menu, char *identbranch, FONT *afont)
:
AreaTablet(menu, identbranch, 255)
{
	usefont = afont;
	text = 0;

	passive = true;	// just displays some text, nothing else (by default at least).
}

TextButton::~TextButton()
{
	if (text)
		delete text;
}

void TextButton::set_text(char *newtext, int color)
{
	if (newtext)
	{
		if (text && strlen(text) < strlen(newtext))
			delete text;

		text = new char [strlen(newtext) + 1];
		strcpy(text, newtext);
	}
	else
	{
		if (!text)
			text = new char [1];
		text[0] = 0;
	}
	text_color = color;
}

void TextButton::subanimate()
{
	int xcentre, ycentre;

	xcentre = iround(size.x / 2);
	ycentre = iround(size.y / 2 - text_height(usefont)/2);

	text_mode(-1);
	if (text)
		textout_centre(drawarea, usefont, text, xcentre, ycentre, text_color);
}












TextList::TextList(TWindow *menu, char *identbranch, FONT *afont)
:
AreaTabletScrolled(menu, identbranch, 255)
{
	usefont = afont;
	Htxt = text_height(usefont);
	text_color = makecol(0,0,0);

//	yselected = 0;

	optionlist = 0;
	N = 0;
	Nreserved = N;

	selected = false;

	scroll.setup(mainwindow, identbranch);//, &scroll);
	scroll.set(0, 0, 1, 0, 1, 1);

	Nshow = int(size.y / Htxt) - 1;		// -1, because item 0 is also shown...

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
	Nreserved = N;

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
	Nreserved = N;

	if (N == 0)
		return;				// in case there's an empty list


	if (N > 0)
		optionlist = new char* [Nreserved];	// reserve space for that many pointers to strings.
	else
		optionlist = 0;

	if (optionlist)
	{
		for ( i = 0; i < N; ++i )
		{
			optionlist[i] = new char[strlen(aoptionlist[i]) + 1 ];
			strcpy(optionlist[i], aoptionlist[i]);
		}
	}

	scroll.set(0, 0, 1, N , 1, Nshow);

	text_color = color;
}


void TextList::add_optionlist(char *newstr)
{
	if (!optionlist)
	{
		Nreserved = 128;
		optionlist = new char* [Nreserved];
	}

	if (N >= Nreserved)
	{
		// re-allocate memory ...
		char **tmp;
		Nreserved += 128;
		tmp = new char* [Nreserved];		// reserve new space
		for ( int i = 0; i < N; ++i )
			tmp[i] = optionlist[i];			// copy content
		delete optionlist;					// delete old stuff
		optionlist = tmp;					// point to the new space
	}

	optionlist[N] = new char[strlen(newstr) + 1 ];
	strcpy(optionlist[N], newstr);
	++N;
	scroll.set(0, 0, 1, N , 1, Nshow);
}


// if the mouse is clicked within the window:
void TextList::handle_lpress()
{

	int iy;
	
	iy = iround(mainwindow->mpos.y - pos.y);

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
	
	iy = iround(mainwindow->mpos.y - pos.y);

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
			//c = makecol(255,255,255);
			c = text_color;
		}

		if (optionlist[i])// && strlen(optionlist[i]) < 20)
			textout(drawarea, usefont, optionlist[i], ix, iy, c);
	}
	

}















TextInfoArea::TextInfoArea(TWindow *menu, char *identbranch, FONT *afont, char *atext, int amaxtext)
:
AreaTabletScrolled(menu, identbranch, 255)
{
	usefont = afont;
//	Htxt = text_height(usefont);
	text_color = makecol(0,0,0);

//	maxchars = amaxtext;

	textinfo = 0;
	localcopy = 0;

	set_textinfo(atext, amaxtext);
	
	//scroll = ascroll;

//	scroll.set(0, 0, 1, 0, 1, 1);

//	Nshow = int(size.y / Htxt) - 1;		// -1, because item 0 is also shown...

	// just display some passive information. Clicking doesn't need to give action by default.
	passive = true;
}

TextInfoArea::~TextInfoArea()
{
	if (textinfo)
		delete textinfo;

	if (localcopy)
		delete localcopy;
}

// the following could be used for editing text that's stored elsewhere
void TextInfoArea::set_textinfo_unbuffered(char *newtext, int Nchars)
{
	if (textinfo)
		delete textinfo;

	textinfo = new TextInfo(usefont, drawarea, newtext, Nchars);

	textinfo->reset(&scroll);
	scroll.set_sel(0, 0);
}


// the following is used to display text, and keep it safe from harm by
// other external factors.
void TextInfoArea::set_textinfo(char *newtext, int Nchars)
{

	if (localcopy)
		delete localcopy;

	localcopy = new char [Nchars+1];
	if (newtext)
		strncpy(localcopy, newtext, Nchars);
	else
		localcopy[0] = 0;
	localcopy[Nchars] = 0;

	set_textinfo_unbuffered(localcopy, Nchars);
}



void TextInfoArea::set_textinfo(char *atextinfo)
{
	set_textinfo(atextinfo, strlen(atextinfo));
}



void TextInfoArea::subanimate()
{

	text_mode(-1);

	int i;
	for ( i = 0; i < textinfo->Nshow; ++i )
	{
		int iline;
		iline = scroll.yselect + i;

		if ( iline < 0 || iline > textinfo->Nlines-1 )	// is the following correct? -> Nline-1 is not a line; it indicates the last byte + 1
			continue;

		// first character of the line
		int n, L;
		n = textinfo->linestart[iline];

		// number of characters till the start of the next line
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
			if (txt[k] < 20 || txt[k] > 127 )
				txt[k] = ' ';
		}

		textout(drawarea, usefont, txt, 0, (iline - scroll.yselect)*textinfo->Htxt, text_color);
	}
	
}















TextEditBox::TextEditBox(TWindow *menu, char *identbranch, FONT *afont,	char *atext, int amaxtext)
:
TextInfoArea(menu, identbranch, afont, atext, amaxtext)
{
	//usefont = afont;
	text = atext;
	maxchars = amaxtext;	// a short line?

	textinfo = 0;
	//textinfo = new TextInfo(afont, drawarea, text, amaxtext);
	//textinfo->reset(&scroll);
	this->set_textinfo_unbuffered(text, amaxtext);

	if (textinfo->textinfo)
		charpos = strlen(textinfo->textinfo);
	else
		charpos = 0;

	int i;
	for ( i = 0; i < KEY_MAX; ++i )
		keypr[i] = (key[i] != 0);

	repeattime = 250;

	lasttime = 0;
	lastpressed = -1;


	int x, y;
	textinfo->getxy(charpos, &x, &y);
	y /= textinfo->Htxt;
	scroll.yselect = y;

	text_color = 0;

	passive = false;
}



TextEditBox::~TextEditBox()
{
	// the following is deleted in the mother function.
	//if (textinfo)
	//	delete textinfo;
}


void TextEditBox::set_textcolor(int c)
{
	text_color = c;
}



// if the mouse is clicked within the window:
// if the mouse button was pressed .. update text pos to current mouse cursor pos.
void TextEditBox::handle_lpress()
{
	// but only if the enter-key wasn't pressed (that can also intiate this?)
	if (keyhandler.keyhit[KEY_ENTER])
		return;

	int ix, iy;
	
	iy = iround(mainwindow->mpos.y - pos.y);
	ix = iround(mainwindow->mpos.x - pos.x);

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


void TextEditBox::text_reset(char *newtext, int N)
{
	textinfo->textinfo = newtext;
	text = newtext;
	textinfo->Nchars = N;
	maxchars = N;
	text_reset();
}

void TextEditBox::text_reset()
{

	textinfo->reset(&scroll);

	if (charpos > (int)strlen(text))
		charpos = strlen(text)-1;

	int xs, ys;
	textinfo->getxy(charpos, &xs, &ys);
	ys /= textinfo->Htxt;

	scroll.set_sel(0, ys);
	
}



// this is, where text is detected and entered ... I think ....
void TextEditBox::calculate()
{
	if (textinfo->textinfo != text) {twgui_error("text mismatch");}

	TextInfoArea::calculate();

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


	int ikey;
	// check all the keys that were pressed ...
	for ( ikey = 0; ikey < keyhandler.Nbuf; ++ikey )
	{

		// normal character format
		int k = keyhandler.keybuf[ikey] & 0x0FF;

		if (keyhandler.keynew[KEY_LCONTROL])
			continue;
		// don't use any keyboard presses when the control key is being held...

		if ( (k >= 0x020 && k < 0x080) || k == '\r' || k == '\n' )
		{
			if (k == '\r')
				k = '\n';	// return should yield a newline!

			// insert a character
			memmove(&text[charpos+1], &text[charpos], (maxchars-1)-charpos-1);
			text[charpos] = k;
			if (charpos < (int)strlen(text))
				++charpos;
			//text[charpos] = 0;
		}
	}

	// Insert text from the clipboard (?)
#ifdef LINUX

#else
	if (keyhandler.keyhit[KEY_V] && keyhandler.keynew[KEY_LCONTROL])
	{
		//Test: this can show contents of the clipboard ...
		
		HWND w;
		w = win_get_window();
		
		OpenClipboard(w);
		
		char *txt = (char*) ::GetClipboardData(CF_TEXT);

		int L;
		L = strlen(txt);
		if (L + charpos < maxchars)
		{
			memmove(&text[charpos+L], &text[charpos], (maxchars-1)-charpos-L);

			memcpy(&text[charpos], txt, L);

			// update the editor linestarts and stuff...
			text_reset();
		}
		
		CloseClipboard();
	}
#endif
			
	// check the special keys ?
	if ( keyhandler.keyhit[KEY_BACKSPACE] && charpos > 0 )
	{
		// delete the previous character 
		int m;
		m = 1;
		
		// if it's a return, then delete that as well
//		if (text[charpos-1] == '\n' && charpos > 1)
//			++m;
		// nah, don't delete, otherwise you can't undelete a line, purely

		memmove(&text[charpos-m], &text[charpos], (maxchars-m)-charpos);
		text[maxchars-m] = 0;
		
		charpos -= m;

	}
	
	if ( keyhandler.keyhit[KEY_DEL] && text[charpos] != 0 )
	{
		// delete the current character (if it's not the closing zero)
		int m;
		m = 1;
		
		// if it's a return, then delete that as well
		if (text[charpos+1] == '\n')
			++m;
		
		memmove(&text[charpos], &text[charpos+m], (maxchars-m)-charpos);
		text[maxchars-m] = 0;

	}
	
	if ( keyhandler.keyhit[KEY_LEFT] )
	{
		if (charpos > 0 )
			--charpos;
		
		//if (charpos > 0 && text[charpos] == '\n')
		//	--charpos;
	}
	
	if ( keyhandler.keyhit[KEY_RIGHT] )
	{
		if (charpos < (int)strlen(textinfo->textinfo))
			++charpos;
		
		//if (charpos  < strlen(textinfo->textinfo) && text[charpos] == '\n')
		//	++charpos;
	}
	
	if ( keyhandler.keyhit[KEY_UP] )
	{
		// update the charpos ...
		int ys;
		ys = scroll.yselect - 1;
		if (ys < 0)
			ys = 0;
		textinfo->changeline(&charpos, scroll.yselect, ys);
	}
	
	if ( keyhandler.keyhit[KEY_DOWN] )
	{
		// update the charpos ...
		int ys;
		ys = scroll.yselect + 1;
		if (ys > textinfo->Nlines-1)
			ys = textinfo->Nlines-1;
		textinfo->changeline(&charpos, scroll.yselect, ys);
	}

	if (ikey > 0)
		text_reset();

}


void TextEditBox::subanimate()
{

//	TextInfoArea::subanimate();

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

		// number of characters till the start of the next line
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
			if (txt[k] < 20 || txt[k] > 127 )
				txt[k] = ' ';
		}

		textout(drawarea, usefont, txt, 0, (iline - scroll.y)*textinfo->Htxt, text_color);
	}


	// draw a line at "charpos" ... 

	int xc, yc;
	textinfo->getxy(charpos, &xc, &yc);
	yc /= textinfo->Htxt;
	//yc = scroll.yselect;
	
	int h;
	h = textinfo->Htxt;
	yc -= scroll.y;

	if ( (int(mainwindow->menu_time * 1000) % 100) < 50)
		line (drawarea, xc, yc*h, xc, (yc+1)*h, makecol(0,0,0));
	
}


char *TextEditBox::get_text()
{
	return text;
}





















MatrixIcons::MatrixIcons(TWindow *menu, char *identbranch, int akey)
:
AreaTabletScrolled(menu, identbranch, akey)
{

	scroll.set(0, 0, 1, 1, 1, 1);

	// obtain the overlay ... this defines the width/height of each matrix area

	overlay = getbmp("overlay");
	if (!overlay)
	{
		twgui_error("MatrixIcons : overlay is missing");
	}

	tmp = create_bitmap(overlay->w, overlay->h);

	maxitems = 0;
	Nx = 0;
	Ny = 0;

	Wicon = overlay->w;
	Hicon = overlay->h;

	Nxshow = iround( size.x / double(Wicon) );
	Nyshow = iround( size.y / double(Hicon) );

	itemproperty = 0;
}


MatrixIcons::~MatrixIcons()
{
	del_bitmap(&overlay);
	del_bitmap(&tmp);

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
	Nx = iround( sqrt((double)maxitems) );
	Ny = maxitems/Nx + 1;

	if (Nx*Ny >= maxitems + Nx)
		--Ny;

	if ( Nx == 0 || Ny == 0 )
	{
		twgui_error("PopupFleetSelection : no bitmap data");
	}

	scroll.set(0, 0, Nx, Ny, Nxshow, Nyshow);

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
	if (hasmouse() && Tmouse.vx() != 0 && Tmouse.vy() != 0)
	{
		// control is handled by the mouse
		scroll.xselect = iround(scroll.x + (mainwindow->mpos.x - pos.x) / Wicon);	// mouse coordinate within the matrix area
		scroll.yselect = iround(scroll.y + (mainwindow->mpos.y - pos.y) / Hicon);
	}

}

void MatrixIcons::subanimate()
{

	int i, j;
	int ix, iy;

	for (iy = scroll.y; iy < scroll.y + Nyshow && iy < Ny; ++iy )
	{

		for (ix = scroll.x; ix < scroll.x + Nxshow && ix < Nx; ++ix )
		{

			int xoverlay, yoverlay;
			int xicon, yicon, k;

			k = iy*Nx + ix;

			if (k >= maxitems)	// if you've run out of bitmaps that you can plot...
				break;

			xoverlay = (ix - scroll.x) * Wicon;
			yoverlay = (iy - scroll.y) * Hicon;

			int w0, h0;
			w0 = iround(listIcon[k]->w * mainwindow->scale * extrascale);
			h0 = iround(listIcon[k]->h * mainwindow->scale * extrascale);

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
	i = scroll.xselect - scroll.x;//mx / Wicon;
	j = scroll.yselect - scroll.y;//my / Hicon;
	double a;
	//a = 0.5 + 0.5 * sin(areareserve->menu_time * 1E-3 * 2*PI / 10);
	a = 0.5;
	rect(drawarea, i*Wicon, j*Hicon, (i+1)*Wicon-1, (j+1)*Hicon-1, makecol(iround(20*a),iround(100*a),iround(200*a)));
}


// this action is used to scroll left/right/up/down
void MatrixIcons::handle_rpress()
{
	int mx, my;
	
	// mouse position relative to the center of the item window:
	mx = iround(mainwindow->mpos.x - pos.x - size.x / 2);
	my = iround(mainwindow->mpos.y - pos.y - size.y / 2);

	// velocity depends on how far you're away from the center.
	scroll.add(iround(mx / (size.x/8)), iround(my / (size.y/8)));
}


// select some icon :
void MatrixIcons::handle_lpress()
{
	selected = true;
}


int MatrixIcons::getk()
{
	int k;
	k = scroll.xselect + scroll.yselect * Nx;

	if ( k > maxitems-1 )
		k = maxitems-1;

	return k;
}






