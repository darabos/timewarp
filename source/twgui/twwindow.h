
#ifndef __TWINDOW__
#define __TWINDOW__

/*

TAB = navigates between buttons.
LSHIFT+TAB = navigates in the opposite direction.
LCONTROL+TAB = navigates between windows.


*/



#include "utils.h"

#include "twbutton.h"

class EmptyButton;

// a menu superstructure ... to manage a bunch of related areas ... in a reserved area/ region

const int max_area = 128;


extern normalmouse Tmouse;
extern TKeyHandler keyhandler;


class TWindow
{
public:

	// managing a list of (related) windows
	TWindow	*prev, *next;

	TWindow *tree_root();
	TWindow *tree_last();

	void tree_calculate();
	void tree_animate();
	void tree_setscreen(BITMAP *scr);
	void tree_doneinit();
	void focus();

	void add(TWindow *newwindow);

	// status of the window (ready to handle input, or not ...)

	bool		disabled;
	bool		hidden;		// hide is "stronger" than disabled, since it also prevents drawing.

	void enable();
	void disable();	// changes the "disabled" flag, and possibly does other stuff as well?
	void hide();
	void show();

	bool hasfocus();	// so that items can check if the menu is active or not
						// (in case the menu calculations fail, occasionally happens)
						// eg., mouse can move out of the menu, in a separate iteration
						// so that incremental updates don't work.


	double		menu_time, menu_starttime;
	void		update_time();

//	int			keybuff[128];
//	int			keybuff_count;
//	void		add_key(int akey);
//	void		clear_keys();
//	void		add_keys();
//	normalmouse		mouse;

	int				x, y, W, H;

	bool checkmouse();
	bool grabbedmouse;
	Vector2 mpos;


	char			ident[128];

	int		default_W;
	double	scale;
	//BITMAP *originalscreen;	// to backup the original screen content
	BITMAP *screen;	// pointer to (modifiable) original screen
	BITMAP *drawarea;		// intermediately used for drawing ... so that menu items won't draw off this area
	BITMAP *backgr_forsearch;	// waste of memory perhaps ?!

	BITMAP *backgr;

//	DATAFILE *datafile;		// this file should contain info for all related areas
							// (used for initialization).

	EmptyButton *button_first, *button_last;
	void add(EmptyButton *newbutton);
	void rem(EmptyButton *newbutton);

	// the last button that has the focus!
	EmptyButton *button_focus;
	void setfocus(EmptyButton *newbutton);


	// vidwin: places the bitmaps used by the menu in video-memory, which is faster.
	TWindow(char *identbase, int dx, int dy, BITMAP *outputscreen, bool vidwin = false);
	virtual ~TWindow();

	void setscreen(BITMAP *scr);

	virtual void doneinit();


	// can be changed
	virtual void calculate();	// calls all menu items
	virtual void animate();

	// return a bmp from the data file
	BITMAP* bmp(char *bmpname, bool vidmem = false);

	// this centers the bitmap on this position.
	void center_abs(int xcenter, int ycenter);
	void center(int xcenter, int ycenter);
	void center();

	void handle_focus();
	void handle_focus_loss();

	void scalepos(int *ax, int *ay);
	void scalepos(Vector2 *apos);
	
	char configfilename[256];
	bool search_bmp_location(BITMAP *bmp_default, Vector2 *apos);

	bool exclusive;
};





#endif
