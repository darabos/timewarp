
#ifndef __TWGUI_GUI__
#define __TWGUI_GUI__


#include "twwindow.h"
#include "twbuttontypes.h"
#include "twhelpers.h"





// something which has a background, and its own drawing-area
class AreaTabletScrolled : public AreaTablet
{
protected:

	//scrollpos_str	scroll;
	ScrollControl	scroll;

public:

	AreaTabletScrolled(TWindow *menu, char *identbranch, int asciicode, bool akeepkey = 0);

	int gety();
	
	virtual void calculate();
};




// a text on top of a button.
// no interaction with environment, it's purely a message.

class TextButton : public AreaTablet
{
public:
	TextButton(TWindow *menu, char *identbranch, FONT *afont);
	virtual ~TextButton();

	FONT		*usefont;

	char *text;
	int text_color;

	virtual void subanimate();
	void set_text(char *txt, int color);
};



// Draws a list of text strings onto a background
// Returns the "item" (or line) number that's selected
// This must support a scroll-bar, in case the list does not fit in the box
class TextList : public AreaTabletScrolled
{

public:
	TextList(TWindow *menu, char *identbranch, FONT *afont);
	~TextList();

	//ScrollControl	scroll;

	char	**optionlist;
	int		N;				// number of options

	FONT	*usefont;
	int		Htxt, Nshow;

	//int		yselected;	// the selected item.

	void clear_optionlist();
	void set_optionlist(char **aoptionlist, int color);
	void set_optionlist(char **aoptionlist, int aN, int color);

	void set_selected(int iy);

	int		text_color;

	void initbackgr(bool autoplace);

	virtual void subanimate();
	
	virtual void handle_lpress();
	virtual void handle_rpress();
	virtual void calculate();

	int getk();
};



// Draw some text into a box... text can consist of many lines. No editing possible.
// If there's a lot of text, you could scroll.


class TextInfoArea : public AreaTabletScrolled
{
protected:
	int maxchars;
	char *localcopy;
	TextInfo *textinfo;
	//scrollpos_str	*scroll;
public:
	TextInfoArea(TWindow *menu, char *identbranch, FONT *afont, char *atext, int amaxtext);
	virtual ~TextInfoArea();

	int		linestart[maxlines];
	int		Nlines;

	FONT	*usefont;
	int		Htxt, Nshow;

	int		text_color;

	void set_textinfo(char *atextinfo);
	void set_textinfo(char *atextinfo, int Nchars);

	virtual void subanimate();
	
};





class TextEditBox : public TextInfoArea
{
protected:
	char *text;//[128];	// can hold 1 line of text.
	int charpos;

	// this is used to record when the last key-press was made (with a resolution
	// of 1 frame at best).
	bool	keypr[KEY_MAX];
	int		lastpressed;
	int		repeattime, lasttime;
	int		text_color;

	
	//TextInfo *textinfo;


public:
	TextEditBox(TWindow *menu, char *identbranch, FONT *afont, char *atext, int amaxtext);
	virtual ~TextEditBox();

	//FONT		*usefont;

	virtual void calculate();
	virtual void subanimate();

	virtual void handle_lpress();

	void clear_text();
	void show_text();

	// set the typematic delay to "atime" milliseconds (default = 100)
	//void set_repeattime(int atime);
	void set_textcolor(int c);

	void text_reset();
	void text_reset(char *newtext);
};





// implement a matrix of icons
// the box defines the area where the icons are visible;

class MatrixIcons : public AreaTabletScrolled
{
protected:
	int		Nx, Ny, Nxshow, Nyshow, maxitems;
	int		Wicon, Hicon;
	BITMAP	*overlay, *tmp;
	double	extrascale;
public:

	enum property {ENABLE=0x01, DISABLE=0x02};

//	int xselect, yselect;
//	int xold, yold;
	bool selected;

	int		*itemproperty;

	BITMAP	**listIcon;

	MatrixIcons(TWindow *menu, char *identbranch, int akey);
	virtual ~MatrixIcons();

	void set_iconinfo(BITMAP **alistIcon, double ascale);

	int getk();

	virtual void subanimate();
	virtual void subcalculate();

	virtual void handle_lpress();
	virtual void handle_rpress();	// for scrolling
	//virtual void handle_doubleclick();

};




#endif


