
#ifndef __TWHELPERS__
#define __TWHELPERS__

#include "twbuttontypes.h"

// discrete position (x,y = item number, not pixels)
//class ScrollBar;
//class EmptyButton;

class ScrollControl
{
public:
	EmptyButton *left, *right, *up, *down;
	ScrollBar *scrollhor, *scrollvert;

	ScrollControl();

	int	x, y;	// which Icon is visible top left.
	int Nx, Ny;				// number of Icons that are present.
	int xselect, yselect;
	int Nxshow, Nyshow;

	void set(int xscroll, int yscroll, int Nxscroll, int Nyscroll, int Nx_show, int Ny_show);

	void set_pos(int xnew, int ynew);
	void set_percent_pos_x(double alpha);
	void set_percent_pos_y(double alpha);
	void add(int dx, int dy);
	void check_pos();
	void calculate();

	void set_sel(int xsel, int ysel);
	void check_sel();

	void bind(EmptyButton *left, EmptyButton *right, EmptyButton *up, EmptyButton *down,
				ScrollBar *scrollhor, ScrollBar *scrollvert);

	void setup_hor(TWindow *A, char *id, ScrollControl *scr);
	void setup_ver(TWindow *A, char *id, ScrollControl *scr);
	void setup_hor(EmptyButton *A, char *id, ScrollControl *scr);
	void setup_ver(EmptyButton *A, char *id, ScrollControl *scr);
	// set up both the horizontal and vertical bar (provided the graphics exist)
	void setup(TWindow *A, char *id, ScrollControl *scr);

};



// a class you can use to edit text

const int maxlines = 1024;


class TextInfo
{
public:
	char	*textinfo;

	int		linestart[maxlines];
	int		Nlines, Nchars;

	FONT	*usefont;
	int		W, Htxt, Nshow;

	int		text_color;

	BITMAP	*bmp;

	TextInfo(FONT *afont, BITMAP *abmp, char *atextinfo, int Nchars);
	~TextInfo();

	//void set_textinfo(char *atextinfo, int Nchars);

	void getxy(int charpos, int *x, int *y);
	int getcharpos(int x, int y);
	void changeline(int *charpos, int line1, int line2);

	void reset(ScrollControl *scroll);
};



#endif