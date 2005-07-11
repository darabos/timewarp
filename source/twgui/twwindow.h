/* $Id$ */ 
/*
Twgui: GPL license - Rob Devilee, 2004.
*/

#ifndef __TWWINDOW_H__
#define __TWWINDOW_H__

/*

TAB = navigates between buttons.
LSHIFT+TAB = navigates in the opposite direction.
LCONTROL+TAB = navigates between windows.


  exclusive = when focused, all other windows are disabled. Default=false.
  layer = you can only change focus within your own layer. Default=0. In this way you can cause a set of windows to always appear above another set of windows.
  disabled = that window does no calculate (use ... to do this ?)
  hidden = that window does not animate (use hide() to stop activity, or show() to restore activity)

*/



#include "utils.h"

#include "twbutton.h"

void del_bitmap(BITMAP **bmp);


class EmptyButton;

// a menu superstructure ... to manage a bunch of related areas ... in a reserved area/ region

const int max_area = 128;


extern normalmouse Tmouse;
extern TKeyHandler keyhandler;

/** the window interface */
class TWindow
{
public:

	/** (default=true): caches the background bitmap into an rle sprite, which is a compressed version of the bitmap.
	This can be very useful for simple backgrounds with large areas of the same color, but disadvantageous
	for complex bitmaps. If set to true, this creates a cached .rledat file next to the .bmp file, so you
	can check how much you save. Even if you don't save much, the rle operation is usually faster than the
	normal bitmap blitting. Set the "userle" option to "0" if you don't want to use it. You've to delete
	it manually if the .bmp is changed.*/
	bool use_rle;

	// in buffered mode, the window has an extra memory bitmap, on which all
	// blit operations are done, before this whole bitmap is written to the
	// twscreen memory.
	bool bufferedmode;

	// use blit or masked_blit...
	bool backgr_masked;

	/** The "autoplace" option in the info.txt file:
	true: then the auto-search is used to place a bitmap somewhere, and also
	there's always a check to see if this is needed;
	false: then the ini positions are used without further questions.
	*/
	int autoplace;

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
	bool grabbedmouse, mousealreadygrabbed;
	twguiVector mpos;


	char			ident[128];

	int		default_W;
	double	scale;
	//BITMAP *originalscreen;	// to backup the original screen content
	bool screen;	// to prevent abuse of the global screen pointer ;)
	BITMAP *twscreen;	// pointer to (modifiable) original screen
	
	BITMAP *drawarea;		// intermediately used for drawing ... so that menu items won't draw off this area
	int ixoffs;		// extra coordinate to determine the true origin, if the drawarea is incomplete
	int iyoffs;

	BITMAP *backgr_forsearch;	// waste of memory perhaps ?!

	BITMAP *backgr;
	RLE_SPRITE *backgr_rle;

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

	int prevx, prevy;
	void handle_unbufferedarea();	// reinits the sub-drawarea each time the menu changes position

	// return a bmp from the data file
	BITMAP* bmp(char *bmpname, bool vidmem = false);
	RLE_SPRITE* load_rle_data(char *filename);
	void save_rle_data(RLE_SPRITE* data, char *filename);

	// this centers the bitmap on this position.
	void center_abs(int xcenter, int ycenter);
	void center(int xcenter, int ycenter);
	void center();

	void handle_focus();
	void handle_focus_loss();

	void scalepos(int *ax, int *ay);
	void scalepos(twguiVector *apos);
	
	char configfilename[256];
	bool search_bmp_location(BITMAP *bmp_default, twguiVector *apos);

	bool exclusive;
	int layer;		// you can only switch focus within a layer...
};



#endif // __TWWINDOW_H__
