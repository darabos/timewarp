#ifndef __TWGUI_GUI__
#define __TWGUI_GUI__


#include "area.h"


// to implement a button, you add bitmaps-feedback to the box-area control

class Button : public AreaBox
{
protected:
	button_bitmaps	bbmps;
public:
	// x, y, W, H are inside the draw area
	Button();

	virtual void init(AreaReserve *menu, char *identbranch, int x, int y);

	virtual void draw_defocus();
	virtual void draw_focus();
	virtual void draw_selected();
};



// a text on top of a button.

class TextButton : public Button
{
public:
	char text[64];
	void animate();
	void set_text(char *txt);
};



// implement a matrix of icons
// the box defines the area where the icons are visible;
// if there are too many, you can scroll perhaps --> so, you need to know the mouse position inside this box...

class MatrixIcons : public AreaBox
{
protected:
	scrollpos_str	scroll;
	BITMAP	*areaMatrix, *backgr;
	BITMAP	**listIcon;
	int D;		// D = default width of an icon.
public:
	// x, y, W, H are inside the draw area
	MatrixIcons();
	~MatrixIcons();

	virtual void init(AreaReserve *menu, char *identbranch, int ax, int ay,
						BITMAP **alistIcon, int aNx, int aNy, int aD);

	virtual void draw_defocus();
	virtual void draw_focus();
	virtual void draw_selected();

	virtual void handle_rpress();

	void draw_matrix();
};



#endif
