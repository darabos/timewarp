

#include <allegro.h>
#include <stdio.h>
#include <string.h>


#include "../melee.h"
//#include "../../melee/mview.h"

REGISTER_FILE

#include "twbutton.h"
#include "twwindow.h"
#include "twhelpers.h"



ScrollControl::ScrollControl()
{
	x = 0;
	y = 0;

	left = 0;
	right = 0;
	up = 0;
	down = 0;
	scrollhor = 0;
	scrollvert = 0;
}


void ScrollControl::set(int xscroll, int yscroll, int Nxscroll, int Nyscroll,
						int Nx_show, int Ny_show)
{
	x = xscroll;	// this is top-left corner item of the visible screen
	y = yscroll;

	xselect = x;	// this has some extra movement capacility within the visible screen
	yselect = y;

	Nxshow = Nx_show;	// the size (in items) of the visible screen
	Nyshow = Ny_show;

	Nx = Nxscroll - Nxshow + 1;	// the scrollable freedom
	Ny = Nyscroll - Nyshow + 1;

	// note: Nx = 1 indicates the "base" position; values >1 indicate out-of-sight positions.
	// or: Nx-Nxshow indicates the out-of-sight positions, +1 indicates the base value position.
	
	// check ranges.
	if (Nx < 1)
		Nx = 1;
	if (Ny < 1)
		Ny = 1;

	check_pos();
}


void ScrollControl::check_pos()
{
	// check the scroll position
	if (x > Nx-1)
		x = Nx-1;

	if (y > Ny-1)
		y = Ny-1;

	if (x < 0)
		x = 0;

	if (y < 0)
		y = 0;

	// check the select position
	// note, the selected item _must_ always lie within the visible area:

	if (xselect < x)
		xselect = x;

	if (xselect > x + Nxshow - 1)
		xselect = x + Nxshow - 1;

	if (yselect < y)
		yselect = y;

	if (yselect > y + Nyshow - 1)
		yselect = y + Nyshow - 1;
}


void ScrollControl::add(int dx, int dy)
{
	x += dx;
	y += dy;

	check_pos();
}



void ScrollControl::check_sel()
{
	if (xselect < x)
		x = xselect;

	if (xselect > x + Nxshow - 1)
		x = xselect-Nxshow+1;

	if (yselect < y)
		y = yselect;

	if (yselect > y + Nyshow - 1)
		y = yselect-Nyshow+1;
}



void ScrollControl::set_sel(int xsel, int ysel)
{
	xselect = xsel;
	yselect = ysel;

	check_sel();
	check_pos();
}



// alpha is a value between 0 and 1
void ScrollControl::set_percent_pos_x(double alpha)
{
	x = iround( (Nx-1) * alpha );
	check_pos();
}

void ScrollControl::set_percent_pos_y(double alpha)
{
	y = iround( (Ny-1) * alpha );
	check_pos();
}

void ScrollControl::set_pos(int xnew, int ynew)
{
	if ( xnew >= 0 && xnew < Nx && ynew >= 0 && ynew < Ny )
	{
		x = xnew;
		y = ynew;

		if (scrollhor)
			scrollhor->setrelpos(double(x)/Nx);
		if (scrollvert)
			scrollvert->setrelpos(double(y)/Ny);
	}

	check_pos();
}



void ScrollControl::calculate()
{
	int xold, yold;

	xold = x;
	yold = y;


	if (left && left->flag.left_mouse_press)
	{
		--xselect;
		if (xselect < x)
			x = xselect;

	}

	if (right && right->flag.left_mouse_press)
	{
		++xselect;
		if (xselect > x + Nxshow - 1)
			x = xselect-Nxshow+1;
	}

	if (up && up->flag.left_mouse_press)
	{
		--yselect;
		if (yselect < y)
			y = yselect;

	}

	if (down && down->flag.left_mouse_press)
	{
		++yselect;
		if (yselect > y + Nyshow - 1)
			y = yselect-Nyshow+1;
	}


	check_pos();

	if (scrollhor && xold != x)
		scrollhor->setrelpos(double(x)/Nx);

	if (scrollvert && yold != y)
		scrollvert->setrelpos(double(y)/Ny);
	

	if (scrollhor && scrollhor->flag.left_mouse_hold)
		set_percent_pos_x(scrollhor->relpos);

	if (scrollvert && scrollvert->flag.left_mouse_hold)
		set_percent_pos_y(scrollvert->relpos);

}




void ScrollControl::bind(EmptyButton *aleft, EmptyButton *aright,
								EmptyButton *aup, EmptyButton *adown,
								ScrollBar *ascrollhor, ScrollBar *ascrollvert)
{
	left = aleft;
	right = aright;
	up = aup;
	down = adown;
	scrollvert = ascrollvert;
	scrollhor = ascrollhor;
}



// This loads the required buttons.

void ScrollControl::setup_hor(TWindow *A, char *id, ScrollControl *scr)
{

	// create the buttons and bitmaps, finding graphics in some data file,
	// with strict name conventions having as base the general identifier "ident".
	//
	// note that the buttons required, and their layout, are determined by the
	// graphics in the datafile.
	char id2[128];


	strcpy(id2, id);
	strcat(id2, "left_");
	left	= new Button(A, id2);
	if (!left->isvalid()) {	A->rem(left); delete left; left = 0;	}

	strcpy(id2, id);
	strcat(id2, "right_");
	right	= new Button(A, id2);
	if (!right->isvalid())	{	A->rem(right); delete right; right = 0;	}

	// add a scroll bar
	strcpy(id2, id);
	strcat(id2, "barhor_");
	scrollhor = new ScrollBar(A, id2);
	if (!scrollhor->isvalid())	{	
		A->rem(scrollhor); delete scrollhor; scrollhor = 0;	
	}

	scr->left = left;
	scr->right = right;
	scr->scrollhor = scrollhor;
}

void ScrollControl::setup_ver(TWindow *A, char *id, ScrollControl *scr)
{
	char id2[128];

	strcpy(id2, id);
	strcat(id2, "up_");
	up		= new Button(A, id2);
	if (!up->isvalid())	{	A->rem(up); delete up; up = 0;	}

	strcpy(id2, id);
	strcat(id2, "down_");
	down	= new Button(A, id2);
	if (!down->isvalid())	{	A->rem(down); delete down; down = 0;	}

	strcpy(id2, id);
	strcat(id2, "barver_");
	scrollvert = new ScrollBar(A, id2);
	if (!scrollvert->isvalid())	{	A->rem(scrollvert); delete scrollvert; scrollvert = 0;	}

	scr->up = up;
	scr->down = down;
	scr->scrollvert = scrollvert;
}


void ScrollControl::setup_hor(EmptyButton *A, char *id, ScrollControl *scr)
{
	setup_hor(A->mainwindow, id, scr);
}

void ScrollControl::setup_ver(EmptyButton *A, char *id, ScrollControl *scr)
{
	setup_ver(A->mainwindow, id, scr);
}

void ScrollControl::setup(TWindow *A, char *id, ScrollControl *scr)
{
	setup_hor(A, id, scr);
	setup_ver(A, id, scr);
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

void TextInfo::reset(ScrollControl *scroll)
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
		n = linestart[iline] + x;
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




