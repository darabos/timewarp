
#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE

#include "twgui.h"




// ------------ AND NOW THE GRAPHICAL PART ---------------


// to implement a button, you add bitmaps-feedback to the box-area control


Button::Button()
{
}


void Button::init(AreaReserve *menu, char *identbranch, int ax, int ay)
{
	char strdefocus[128], strfocus[128], strselected[128], strmouse[128];

	AreaBox::init(menu, identbranch, ax, ay, 0, 0);

	// what's the meaning of W and H ??

	BITMAP *tmp;

	strcpy(strdefocus,  ident);
	strcpy(strfocus,    ident);
	strcpy(strselected, ident);
	strcpy(strmouse,	ident);

	strcat(strdefocus,  "_DEFOCUS");
	strcat(strfocus,    "_FOCUS");
	strcat(strselected, "_SELECTED");
	strcat(strmouse,	"_MOUSE");

	bbmps.init(	//bitmap_color_depth(areareserve->drawarea),
				areareserve->bmp(strdefocus),
				areareserve->bmp(strfocus),
				areareserve->bmp(strselected));

	if (bbmps.bmp_focus)
	{
		W = bbmps.bmp_focus->w;		// note: it's already scaled on initialization.
		H = bbmps.bmp_focus->h;
	}


	mouse.bmp.init(areareserve->bmp(strmouse));
}


void Button::draw_defocus()
{
	BITMAP *b = bbmps.bmp_defocus;
	if (b)
		blit(b, areareserve->drawarea, 0, 0, x, y, b->w, b->h);
}

void Button::draw_focus()
{
	BITMAP *b = bbmps.bmp_focus;
	if (b)
		blit(b, areareserve->drawarea, 0, 0, x, y, b->w, b->h);
}

void Button::draw_selected()
{
	BITMAP *b = bbmps.bmp_selected;
	if (b)
		blit(b, areareserve->drawarea, 0, 0, x, y, b->w, b->h);
}




// implement a matrix of icons
// the box defines the area where the icons are visible;
// if there are too many, you can scroll perhaps --> so, you need to know the mouse position inside this box...



MatrixIcons::MatrixIcons()
{
}

MatrixIcons::~MatrixIcons()
{
	if (backgr)
		destroy_bitmap(backgr);
	if (areaMatrix)
		destroy_bitmap(areaMatrix);
}

void MatrixIcons::init(AreaReserve *menu, char *identbranch, int ax, int ay,
						BITMAP **alistIcon, int aNx, int aNy, int aD)
{
	scroll.set(0, 0, aNx, aNy);
	D = aD;

	listIcon = alistIcon;

	AreaBox::init(menu, identbranch, ax, ay, 0, 0);


	char strmatrix[128];
	strcpy(strmatrix,  ident);
	strcat(strmatrix,  "_BACKGR");

	// a background image is needed of course.
	backgr = areareserve->bmp(strmatrix);

	if (backgr)
	{
		W = backgr->w;		// note: background is already scaled when it's initialized
		H = backgr->h;
	} else {
		tw_error("Could not initialize background of matrix");
	}

	areaMatrix = create_bitmap_ex(bitmap_color_depth(areareserve->drawarea), W, H);
}


void MatrixIcons::draw_focus()
{
	draw_matrix();
}
void MatrixIcons::draw_defocus()
{
	draw_matrix();
}
void MatrixIcons::draw_selected()
{
	draw_matrix();
	// possibly with something extra
}

void MatrixIcons::draw_matrix()
{
	int mx, my;
	mx = mouse.xpos() - x;	// mouse coordinate within the matrix area
	my = mouse.ypos() - y;

	blit ( backgr, areaMatrix, 0, 0, 0, 0, backgr->w, backgr->h);

	int ix, iy;
	for (iy = 0; iy < scroll.Ny; ++iy )
	{
		for (ix = 0; ix < scroll.Nx; ++ix )
		{
			int xicon, yicon, k;

			k = iy*scroll.Nx + ix;

			xicon = (ix - scroll.x + 0.5) * D - 0.5 * listIcon[k]->w;
			yicon = (iy - scroll.y + 0.5) * D - 0.5 * listIcon[k]->h;

			// no stretching is applied to these things ?
			if (listIcon[k])
				masked_blit(listIcon[k], areaMatrix, 0, 0, xicon, yicon, listIcon[k]->w, listIcon[k]->h );
		}
	}

	// one of them is highlighted ... the one which the mouse is pointed at: draw
	// a box around it ?
	int i, j;
	i = mx / D;
	j = my / D;
	rect(areaMatrix, i*D, j*D, (i+1)*D, (j+1)*D, makecol(200,200,200));

	BITMAP *b = areaMatrix;
	blit(b, areareserve->drawarea, 0, 0, x, y, b->w, b->h);

}


// this action is used to scroll left/right/up/down
void MatrixIcons::handle_rpress()
{
	int mx, my;
	
	// mouse position relative to the center of the item window:
	mx = mouse.xpos() - x - 0.5 * W;
	my = mouse.ypos() - y - 0.5 * H;

	// velocity depends on how far you're away from the center.
	scroll.add(mx / (W/8), my / (H/8));
}






void TextButton::set_text(char *txt)
{
	strncpy(text, txt, 63);
}

void TextButton::animate()
{
	Button::animate();

	int xcentre, ycentre;

	xcentre = x + W/2;
	ycentre = y + H/2 - text_height(font)/2;

	text_mode(-1);
	textout_centre(areareserve->drawarea, font, text, xcentre, ycentre, makecol(200,200,200));
}






// a class you can use to edit text

class TextEditBox : public AreaBox
{
protected:
	char text[128];	// can hold 1 line of text.
	int maxchars;
	int charpos;

	BITMAP	*backgr;

public:
	virtual void init(AreaReserve *menu, char *identbranch, int ax, int ay);
	void animate();
	void calculate();

	virtual void draw_defocus();
	virtual void draw_focus();
	virtual void draw_selected();

	void clear_text();
	void show_text();
};

void TextEditBox::init(AreaReserve *menu, char *identbranch, int ax, int ay)
{
	AreaBox::init(menu, identbranch, ax, ay, 0, 0);

	char streditbox[128];
	strcpy(streditbox,  ident);
	strcat(streditbox,  "_BACKGR");

	// a background image is needed of course.
	backgr = areareserve->bmp(streditbox);

	if (backgr)
	{
		W = backgr->w;		// note: background is already scaled when it's initialized
		H = backgr->h;
	} else {
		tw_error("Could not initialize background of TextEditBox");
	}

	text[0] = 0;
	charpos = 0;
	maxchars = 30;	// a short line?

}

// this is, where text is detected and entered ... I think ....
void TextEditBox::calculate()
{
	AreaBox::calculate();

	if (!focus)
		return;

	// enter text.

	selected = false;

	while (keypressed())
	{
		int key_all = readkey();
		unsigned char k = key_all & 0x0ff;		// normal character format
		unsigned char m = key_all >> 8;			// scan code format

		if ( k > 0x020 && k < 0x040 )
		{
			text[charpos] = k;
			++charpos;
			text[charpos] = 0;

		} else if ( m == KEY_BACKSPACE )
		{
			--charpos;
			text[charpos] = 0;

		} else if ( m == KEY_ENTER)
			selected = true;

	}


}


void TextEditBox::animate()
{
	// background?
	blit(backgr, areareserve->drawarea, 0, 0, x, y, W, H);

	textout(areareserve->drawarea, font, text, x, y-0.5*text_height(font), makecol(200,200,200));
}


