
#ifndef __TWGUI_GUI__
#define __TWGUI_GUI__


#include "area.h"


// to implement a button, you add bitmaps-feedback to the box-area control

class Button : public AreaBox
{
protected:
	BITMAP *bmp_default, *bmp_focus, *bmp_selected;
public:
	// x, y, W, H are inside the draw area
	Button(AreaReserve *menu, char *identbranch, int ax, int ay, int asciicode, bool keepkey = 0);
	~Button();

//	virtual void init(AreaReserve *menu, char *identbranch, int ax, int ay, unsigned char asciicode);

	virtual void draw_default();
	virtual void draw_focus();
	virtual void draw_selected();
};


class GhostButton : public AreaGeneral
{
public:
	GhostButton(AreaReserve *menu);
};

// a text on top of a button.
// no interaction with environment, it's purely a message.

class TextButton : public AreaTablet
{
public:
	TextButton(AreaReserve *menu, char *identbranch, int ax, int ay, FONT *afont);

	FONT		*usefont;

	char text[64];
	int text_color;

	virtual void subanimate();
	void set_text(char *txt, int color);
};



// a class you can use to edit text

class TextEditBox : public AreaTablet
{
protected:
	char text[128];	// can hold 1 line of text.
	int maxchars;
	int charpos;

	//BITMAP	*backgr;

	// this is used to record when the last key-press was made (with a resolution
	// of 1 frame at best).
	int key_time[KEY_MAX];
	bool keypr[KEY_MAX];
	int repeattime;

public:
	TextEditBox(AreaReserve *menu, char *identbranch, int ax, int ay, FONT *afont);

	FONT		*usefont;

	virtual void calculate();
	virtual void subanimate();

	void clear_text();
	void show_text();

	// set the typematic delay to "atime" milliseconds (default = 100)
	void set_repeattime(int atime);
};





// a switch, is a button that is always in 2 of states (on or off) :
// (a "focus" is always indicated by some box around it).

class SwitchButton : public AreaBox
{
protected:
	BITMAP	*bmp_on, *bmp_off;
public:
	bool	state;	// true=on, false=off
	// x, y, W, H are inside the draw area
	SwitchButton(AreaReserve *menu, char *identbranch, int ax, int ay, int asciicode);
	~SwitchButton();

	virtual void calculate();

	virtual void draw_default();
	virtual void draw_focus();
	virtual void draw_selected();	// is the same as focus, cause a switch cannot be selected all the time !!

	void draw_on();
	void draw_off();

	//void locate_by_backgr(char *stron);
};



// Draws a list of text strings onto a background
// This is useful for a popup menu.
class TextButtonList : public AreaTablet
{
	scrollpos_str	*scroll;
public:
	TextButtonList(AreaReserve *menu, char *identbranch, int ax, int ay, FONT *afont,
					scrollpos_str *ascroll);
	~TextButtonList();

	char	**optionlist;
	int		N;				// number of options

	FONT	*usefont;
	int		Htxt, Nshow;

	int		yselected;	// the selected item.

	void clear_optionlist();
	void set_optionlist(char **aoptionlist, int color);
	void set_optionlist(char **aoptionlist, int aN, int color);

	void set_selected(int iy);

	int		text_color;

	void initbackgr(bool autoplace);

	virtual void subanimate();
	
	virtual void handle_lpress();
	virtual void handle_rpress();

	int getk();
};




class ScrollBar : public AreaTablet
{
protected:
	BITMAP	*button;
public:

	enum {hor = 1, ver= 2}	direction ;

	double relpos;

	int pmin, pmax, bwhalf, bhhalf;	// p = position (can be x or y)
	int pbutton;

	// x, y, W, H are inside the draw area
	ScrollBar(AreaReserve *menu, char *identbranch, int ax, int ay);
	~ScrollBar();

	virtual void handle_lhold();

	virtual void subanimate();

	void setrelpos(double arelpos);
};



// Draw some text into a box... text can consist of many lines. No editing possible.
// If there's a lot of text, you could scroll.

const int maxlines = 1024;

class TextInfoArea : public AreaTablet
{
	scrollpos_str	*scroll;
public:
	TextInfoArea(AreaReserve *menu, char *identbranch, int ax, int ay, FONT *afont,
					scrollpos_str *ascroll);
	~TextInfoArea();

	char	*textinfo;

	int		linestart[maxlines];
	int		Nlines;

	FONT	*usefont;
	int		Htxt, Nshow;

	int		text_color;

	void set_textinfo(char *atextinfo, int Nchars);

	virtual void subanimate();
	
};






// A popup-list consists of several components, which you've to initialize:
// (most of it is automatically initialized from the shape of the menu screen-
// the button images should be present on it as well)
class PopupGeneral : public AreaReserve
{
public:
	int returnstatus;

	struct option_str
	{
		bool disable_othermenu,		// disables all i/o of the host menu (true)
			place_relative2mouse;	// places relative to mouse (true), or the host menu (false)
	} option;


	bool movingthereserve;	// in case you want to move the menu area somewhere else

	bool returnvalueready;

	int xshift, yshift;

	AreaGeneral		*trigger;	// this controls the on/off of the menu

	Button			*left, *right, *up, *down;
	scrollpos_str	*scroll_control;
	ScrollBar		*scrollvert, *scrollhor;

	// origin relative to the creators' position
	PopupGeneral(AreaGeneral *creator, char *ident, int axshift, int ayshift, char *datafilename);
	PopupGeneral::PopupGeneral(char *ident, int axshift, int ayshift,
								char *datafilename, BITMAP *outputscreen);
	~PopupGeneral();
	virtual void init_components();

	// returnstatus can be (usually is) the array index of a selected item in a list.
	virtual void close(int areturnstatus);
	virtual void enable();
	virtual void newscan();

	virtual void calculate();
	virtual void handle_focus_loss();

	// this should call "close" ; you should replace this with your own code, to get the
	// submenu-exit behaviour you want. Default is that it returns value -1 when the mouse
	// pointer leaves the submenu area.
	virtual void check_end();

	// this allows you to acces the return value, but only once after the submenu was
	// closed; otherwise it returns -1.
	int getvalue();
};


class PopupTextInfo : public PopupGeneral
{
public:
	TextInfoArea	*tia;

	PopupTextInfo(AreaGeneral *creator, char *ident, int axshift, int ayshift, char *datafilename,
					FONT *afont, char *atext, int aNchar);
	~PopupTextInfo();
};



// the info screen, can be toggled on/off, by pressing the trigger-button.
// (uhm, well, that didn't work easily, so now it's simply turned on all the time).
class PopupTextInfo_toggle : public PopupTextInfo
{
public:

	PopupTextInfo_toggle(AreaGeneral *creator, char *ident, int axshift, int ayshift, char *datafilename,
		FONT *afont, char *atext, int aNchar);

//	~PopupTextInfo();

	virtual void calculate();
	virtual void check_end();

};




// A popup-list consists of several components, which you've to initialize:
// (most of it is automatically initialized from the shape of the menu screen-
// the button images should be present on it as well)
class PopupList : public PopupGeneral
{
public:

	int xshift, yshift;

//	AreaGeneral		*trigger;	// this controls the on/off of the menu

//	Button			*left, *right, *up, *down;
//	scrollpos_str	*scroll_control;
	TextButtonList	*tbl;
//	ScrollBar		*scrollvert, *scrollhor;

//	char			**optionslist;
	
	// origin relative to the creators' position
	PopupList(AreaGeneral *creator, char *ident, int axshift, int ayshift, char *datafilename,
				FONT *afont, char **aaoptionslist);
	~PopupList();

	virtual void check_end();
	//virtual void calculate();
	//virtual void handle_focus_loss();

};





// implement a matrix of icons
// the box defines the area where the icons are visible;
// if there are too many, you can scroll perhaps --> so, you need to know the mouse position inside this box...

class MatrixIcons : public AreaTablet
{
protected:
	scrollpos_str	*scroll;
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

	MatrixIcons(AreaReserve *menu, char *identbranch, int ax, int ay,
						scrollpos_str *ascroll, int akey);
	~MatrixIcons();

	void set_iconinfo(BITMAP **alistIcon, double ascale);

	int getk();

	virtual void subanimate();
	virtual void subcalculate();

	virtual void handle_lpress();
	virtual void handle_rpress();	// for scrolling
	//virtual void handle_doubleclick();

};


class PopupFleetSelection : public PopupGeneral
{
public:
	MatrixIcons		*icons;
	TextButton		*info;
	Button			*oncerandom, *alwaysrandom;

	PopupFleetSelection(AreaGeneral *creator, char *ident, int axshift, int ayshift, char *datafilename,
				BITMAP **alistIcon, double ascale, FONT *afont );

	PopupFleetSelection(char *ident, int axshift, int ayshift,
								char *datafilename, BITMAP *outputscreen,
								BITMAP **alistIcon, double ascale, FONT *afont );

	~PopupFleetSelection();

	//virtual void close(int areturnstatus);
	virtual void check_end();

	virtual void newscan(BITMAP **alistIcon, double ascale, char *txt);

	//virtual void init_components();
	
};






class Popup : public AreaReserve
{
	int		returnstatus;
	bool	inherited;

public:
	Popup(char *ident, int xcenter, int ycenter, char *datafilename, BITMAP *outputscreen,
		bool inherited = false);

	bool returnvalueready;

	virtual void calculate();
	virtual void check_end();

	virtual void close(int areturnstatus);
	virtual int getvalue();
	virtual void enable();

};



class PopupOk : public Popup
{
	Button		*ok;
public:
	PopupOk(char *ident, int xcenter, int ycenter,
						char *datafilename, BITMAP *outputscreen,
							bool inherited = false);

	virtual void check_end();
};


class PopupYN : public Popup
{
	Button		*yes, *no;
public:
	PopupYN(char *ident, int xcenter, int ycenter,
						char *datafilename, BITMAP *outputscreen,
							bool inherited = false);

	virtual void check_end();

};



#endif


