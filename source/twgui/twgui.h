
#ifndef __TWGUI_GUI__
#define __TWGUI_GUI__


#include "area.h"


const int maxlines = 1024;




// to implement a button, you add bitmaps-feedback to the box-area control

class Button : public AreaBox
{
protected:
	BITMAP *bmp_default, *bmp_focus, *bmp_selected;
public:
	// x, y, W, H are inside the draw area
	Button(AreaReserve *menu, char *identbranch, int ax, int ay, int asciicode, bool keepkey = 0);
	virtual ~Button();

//	virtual void init(AreaReserve *menu, char *identbranch, int ax, int ay, unsigned char asciicode);

	virtual void draw_default();
	virtual void draw_focus();
	virtual void draw_selected();

	virtual bool hasmouse();
};


// something which has a background, and its own drawing-area
class AreaTablet : public AreaBox
{
protected:
	BITMAP *backgr, *drawarea;

public:

	AreaTablet(AreaReserve *menu, char *identbranch, int ax, int ay, int asciicode, bool akeepkey = 0);
	virtual AreaTablet::~AreaTablet();

	void initbackgr(bool autoplace);
	void changebackgr(char *fname);

	void animate();		// shouldn't be changed.
	virtual void subanimate();

	virtual bool hasmouse();
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
	virtual ~ScrollBar();

	virtual void handle_lhold();

	virtual void subanimate();

	void setrelpos(double arelpos);
};

// creates a set of buttons for the control of a scrollpos_str
class ScrollControl
{
public:
	Button			*left, *right, *up, *down;
	ScrollBar		*scrollvert, *scrollhor;

	ScrollControl();

	void hor(AreaReserve *A, char *id, scrollpos_str *ascr);
	void hor(AreaGeneral *A, char *id, scrollpos_str *ascr);

	void ver(AreaReserve *A, char *id, scrollpos_str *ascr);
	void ver(AreaGeneral *A, char *id, scrollpos_str *ascr);
};


// something which has a background, and its own drawing-area
class AreaTabletScrolled : public AreaTablet
{
protected:

	scrollpos_str	scroll;
	ScrollControl	scrollcontrol;

public:

	AreaTabletScrolled(AreaReserve *menu, char *identbranch, int ax, int ay, int asciicode, bool akeepkey = 0);
};







class GhostButton : public AreaGeneral
{
public:
	GhostButton(AreaReserve *menu);
	virtual ~GhostButton();
};

// a text on top of a button.
// no interaction with environment, it's purely a message.

class TextButton : public AreaTablet
{
public:
	TextButton(AreaReserve *menu, char *identbranch, int ax, int ay, FONT *afont);
	virtual ~TextButton();

	FONT		*usefont;

	char text[64];
	int text_color;

	virtual void subanimate();
	void set_text(char *txt, int color);
};


// a class you can use to edit text

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

	void reset(scrollpos_str *scroll);
};

class TextEditBox : public AreaTabletScrolled
{
protected:
	char *text;//[128];	// can hold 1 line of text.
	int maxchars;
	int charpos;

	//BITMAP	*backgr;

	// this is used to record when the last key-press was made (with a resolution
	// of 1 frame at best).
	bool	keypr[KEY_MAX];
	int		lastpressed;
	int		repeattime, lasttime;
	int		text_color;

	
	TextInfo *textinfo;


public:
	TextEditBox(AreaReserve *menu, char *identbranch, int ax, int ay, FONT *afont, char *atext, int amaxtext);
	virtual ~TextEditBox();

	FONT		*usefont;

//	scrollpos_str	scroll;
//	ScrollControl	scrollcontrol;

	virtual void calculate();
	virtual void subanimate();

	virtual void handle_lpress();

	void clear_text();
	void show_text();

	// set the typematic delay to "atime" milliseconds (default = 100)
	void set_repeattime(int atime);
	void set_textcolor(int c);

	void text_reset();
	void text_reset(char *newtext);
};


// Draws a list of text strings onto a background
class TextList : public AreaTablet
{

public:
	TextList(AreaReserve *menu, char *identbranch, int ax, int ay, FONT *afont);
	~TextList();

	scrollpos_str	scroll;
	ScrollControl	scrollcontrol;

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
	virtual ~SwitchButton();

	virtual void calculate();

	virtual void draw_default();
	virtual void draw_focus();
	virtual void draw_selected();	// is the same as focus, cause a switch cannot be selected all the time !!

	void draw_on();
	void draw_off();

	virtual bool hasmouse();
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
	virtual ~TextButtonList();

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







// Draw some text into a box... text can consist of many lines. No editing possible.
// If there's a lot of text, you could scroll.


class TextInfoArea : public AreaTabletScrolled
{
	//scrollpos_str	*scroll;
public:
	TextInfoArea(AreaReserve *menu, char *identbranch, int ax, int ay, FONT *afont);
	virtual ~TextInfoArea();

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

//	Button			*left, *right, *up, *down;
//	ScrollBar		*scrollvert, *scrollhor;

	scrollpos_str	scroll;
	ScrollControl	scrollcontrol;

	// origin relative to the creators' position
	PopupGeneral(AreaGeneral *creator, char *identbranch, int axshift, int ayshift);
	PopupGeneral::PopupGeneral(char *identbranch, int axshift, int ayshift, BITMAP *outputscreen);
	~PopupGeneral();
	virtual void init_components(char *identbranch);

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

	PopupTextInfo(AreaGeneral *creator, char *ident, int axshift, int ayshift,
					FONT *afont, char *atext, int aNchar);
	virtual ~PopupTextInfo();
};



// the info screen, can be toggled on/off, by pressing the trigger-button.
// (uhm, well, that didn't work easily, so now it's simply turned on all the time).
class PopupTextInfo_toggle : public PopupTextInfo
{
public:

	PopupTextInfo_toggle(AreaGeneral *creator, char *ident, int axshift, int ayshift,
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
	PopupList(AreaGeneral *creator, char *ident, int axshift, int ayshift,
				FONT *afont, char **aaoptionslist);
	virtual ~PopupList();

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
	virtual ~MatrixIcons();

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

	virtual ~PopupFleetSelection();

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
	Popup(char *ident, int xcenter, int ycenter, BITMAP *outputscreen,
		bool inherited = false);
	virtual ~Popup();

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
						//char *datafilename,
						BITMAP *outputscreen,
							bool inherited = false);
	virtual ~PopupOk();
	virtual void check_end();
};


class PopupYN : public Popup
{
	Button		*yes, *no;
public:
	PopupYN(char *ident, int xcenter, int ycenter,
						//char *datafilename,
						BITMAP *outputscreen,
							bool inherited = false);
	virtual ~PopupYN();
	virtual void check_end();

};




#endif

