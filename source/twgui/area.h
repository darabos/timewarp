
#ifndef __TWGUI_AREA__
#define __TWGUI_AREA__

#include "utils.h"



class AreaReserve;

class AreaGeneral
{
protected:

public:

	bool selected;		// a general-purpose flag; is set if left mouse is held; this can
						// be useful in simple applications.

	int		trigger_key;	// ascii (+control flags) key-code shortcut to select this area
	//bool	trigger_keepkey;	// determines if a key-press, or a key-hold is used
	bool	trigger_keydetection;	// stores whether the key's been held or not

	void update_key();
	bool haskey();
	bool haskeypress();

	struct flag_struct
	{
		// these flags are updated by the area's calculate function.
		bool haskey, haskeypress;
		bool focus, lastfocus;
		bool left_mouse_press, left_mouse_release, left_mouse_hold;
		bool right_mouse_press, right_mouse_release, right_mouse_hold;
	        //reset();
	        void reset();  //added void 7/1/2003 Culture20
		flag_struct();
	} flag;

	// this should point to some master control structure
	// in order to synch different parts.
	AreaReserve	*areareserve;

	char		ident[128];


	normalmouse	mouse;
	void update_mouse();


	class Key
	{
	public:
		virtual int check_key();
	};
	
	
	
	// not supported yet, except as declaration:
	// it's supposed that several items can be selected at once.
	// this class shows whether the item is part of a selection or
	// not.
	class Selections
	{
	protected:
		bool currentstatus;
	public:
		
		bool status()
		{
			return currentstatus;
		}
		
		void set()
		{
			currentstatus = true;
		}
		
		void clear()
		{
			currentstatus = false;
		}
	};


	int disabled;

	// called only when mouse enters or leaves the button area first time
	// these call handle_focus and handle_defocus, which you can use to update
	// the menu whenever the mouse enters or leaves the area that belongs to
	// this item.

	virtual bool hasmouse();


//	AreaMouse mouse;

//	Key key;

	// check if it is part of a larger selection (so that the key or
	// mouse doesn't need to point at it for it to be active).
	

//	Selections selection;

	// checks mouse I/O inside the menu item, and calls calculate and check_focus/defocus
	// routines.
	// is always calculated
	// and calls more specialized routines, what to do with the area in different cases

	// THERE CAN BE A LIST OF SUCH AREAS, EACH HAVING A DIFFERENT FUNCTION ?

	// shouldn't be changed.
	virtual void calculate();
	// should be changed; users can put stuff here that should come after the mouse/ key
	// update, but before the mouse handles (i.e., to pre-process input if needed).
	virtual void subcalculate() {};		// empty by default

	AreaGeneral(AreaReserve *menu);
	AreaGeneral(AreaReserve *menu, char *identbranch, int asciicode, bool akeepkey = 0);
	virtual ~AreaGeneral();
	//virtual void init(AreaReserve *menu, char *identbranch);


	//int lastfocus, focus;
	void check_focus();

	virtual void draw_default();
	virtual void draw_focus();
	virtual void draw_selected();
	virtual void animate();

	virtual void handle_menu_focus_loss();

	virtual void handle_char(int c) {};
	virtual void handle_uchar(int c) {};
	virtual void handle_focus() {};
	virtual void handle_defocus() {};
	virtual void handle_click() {};
	virtual void handle_doubleclick() {};
	virtual void handle_gotmouse() {};
	virtual void handle_lostmouse() {};
	virtual void handle_wheel(int c) {};
	virtual void handle_lpress() {};
	virtual void handle_mpress() {};
	virtual void handle_rpress() {};
	virtual void handle_lrelease() {};
	virtual void handle_mrelease() {};
	virtual void handle_rrelease() {};
	virtual void handle_lhold() {};
	virtual void handle_mhold() {};
	virtual void handle_rhold() {};

};


// a menu superstructure ... to manage a bunch of related areas ... in a reserved area/ region

const int max_area = 32;

class AreaReserve
{
protected:
	AreaReserve	*prev;
public:
	bool		disabled;
	bool		hidden;		// hide is "stronger" than disabled, since it also prevents drawing.


	double		menu_time, menu_starttime;
	void		update_time();

	int			keybuff[128];
	int			keybuff_count;
	void		add_key(int akey);
	void		clear_keys();
	void		add_keys();

	int				x, y, W, H;
	normalmouse		mouse;

	bool hasmouse();

	void enable();
	void disable();	// changes the "disabled" flag, and possibly does other stuff as well?
	void hide();
	void show();

	void focus2other(AreaReserve *other);
	void back2other();

	bool			hasfocus;		// so that items can check if the menu is active or not
									// (in case the menu calculations fail, occasionally happens)
									// eg., mouse can move out of the menu, in a separate iteration
									// so that incremental updates don't work.

	char			ident[128];

	int		default_W;
	double	scale;
	//BITMAP *originalscreen;	// to backup the original screen content
	BITMAP *screenreserve;	// pointer to (modifiable) original screen
	BITMAP *drawarea;		// intermediately used for drawing ... so that menu items won't draw off this area
	BITMAP *backgr_forsearch;	// waste of memory perhaps ?!

	BITMAP *backgr;

	DATAFILE *datafile;		// this file should contain info for all related areas
							// (used for initialization).

	int				Nareas;
	AreaGeneral		*area[max_area];


	AreaReserve(char *identbase, int dx, int dy, char *datafilename, BITMAP *outputscreen);
	virtual ~AreaReserve();

	void setscreen(BITMAP *scr);

	virtual void doneinit();

	// shouldn't be changed
	void add(AreaGeneral *newarea);
	void remove(AreaGeneral *remarea);

	// can be changed
	virtual void calculate();	// calls all menu items
	virtual void animate();

	// return a bmp from the data file
	BITMAP* bmp(char *bmpname);

	// this centers the bitmap on this position.
	void center(int xcenter, int ycenter);

	virtual void handle_focus_loss();

	void scalepos(int *ax, int *ay);
	bool search_bmp_location(BITMAP *bmp_default, int *ax, int *ay);
};





class AreaBox : public AreaGeneral
{
public:
	int x, y, W, H;	// these are used to define the box area (top left corner and dimensions)

	virtual bool hasmouse();

	virtual void draw_rect();
	virtual void draw_rect_fancy();

	AreaBox(AreaReserve *menu, char *identbranch, int ax, int ay, int asciicode, bool akeepkey = 0);
//	virtual void init(AreaReserve *menu, char *identbranch, int ax, int ay);
	virtual ~AreaBox();

	void locate_by_backgr(char *stron);

	BITMAP *getbmp(char *name);
};



// something which has a background, and its own drawing-area
class AreaTablet : public AreaBox
{
protected:
	BITMAP *backgr, *drawarea;

public:

	AreaTablet(AreaReserve *menu, char *identbranch, int ax, int ay, int asciicode, bool akeepkey = 0);
	virtual ~AreaTablet();

	void initbackgr(bool autoplace);

	void animate();		// shouldn't be changed.
	virtual void subanimate();
};







class WindowManagerElement
{
	public:
	WindowManagerElement		*prev;
	AreaReserve					*p;			// so much trouble for just this thingy here ;)
	WindowManagerElement		*next;

	WindowManagerElement(WindowManagerElement *aprev,
										   AreaReserve *ap, WindowManagerElement *anext);
	~WindowManagerElement();
};


class WindowManager
{
protected:
	WindowManagerElement		*head, *tail;
	WindowManagerElement		*iter;		// used for first()/ next() iteration.
public:

	WindowManager();
	~WindowManager();

	void add(AreaReserve *newreserve);
	void remove(WindowManagerElement *w);

	AreaReserve *first();
	AreaReserve *next();

	AreaReserve					*a;		// general purpose, so that you don't have to declare it everywhere.
	WindowManagerElement		*w;		// (same here)

	void calculate();
	void animate();
	void setscreen(BITMAP *scr);
};









#endif
