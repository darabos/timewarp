/* $Id$ */ 
/*
Placed in public domain by Rob Devilee, 2004. Share and enjoy!
*/

#ifndef __TWBUTTON_H__
#define __TWBUTTON_H__

#include "utils.h"



class ButtonEvent
{
public:
	
	//virtual void handle_char(int c) {};
	//virtual void handle_uchar(int c) {};
	virtual void handle_main_press() {};
	virtual void handle_special_press() {};
	virtual void handle_main_hold() {};
	virtual void handle_special_hold() {};
};



class TWindow;



/** \brief The bare button type. Any buttons are derived from this. Basically,
only the i/o is defined here.

*/
class EmptyButton
{

public:
	EmptyButton *prev, *next;
	twguiVector pos, size;	// these are used to define the box area (top left corner and dimensions)

	ButtonEvent	*button_event;
	void bind(ButtonEvent *be) {button_event = be;};

	// ---------- MOUSE IO -----------
//	normalmouse	mouse;
//	void update_mouse();

	virtual bool hasmouse();

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

	// --------- keyboard IO -----------
	int		trigger_key;	// ascii (+control flags) key-code shortcut to select this area
	//bool	trigger_keepkey;	// determines if a key-press, or a key-hold is used
	bool	trigger_keydetection;	// stores whether the key's been held or not

	bool	passive;	// can it be selected, or not ...

	virtual void check_key();

	virtual void handle_char(int c) {};
	virtual void handle_uchar(int c) {};
	
	virtual bool haskey();
	virtual bool haskeypress();

	struct flag_struct
	{
		// these flags are updated by the area's calculate function.
		bool haskey, haskeypress;
		bool focus, lastfocus;
		bool left_mouse_press, left_mouse_release, left_mouse_hold;
		bool right_mouse_press, right_mouse_release, right_mouse_hold;
		void reset();
		flag_struct();
	} flag;

	// this should point to some master control structure
	// in order to synch different parts.
	TWindow	*mainwindow;

	char		ident[128];



	
	// a general-purpose flag; is set if left mouse is held; this can
	// be useful in simple applications.
	// it's supposed that several items can be selected at once.
	// this class shows whether the item is part of a selection or
	// not.
	bool selected;


	// whether this button is available for IO or not.
	int state;		// old: "disabled"


	virtual void calculate();
	virtual void animate();

	// should be changed; users can put stuff here that should come after the mouse/ key
	// update, but before the mouse handles (i.e., to pre-process input if needed).
	virtual void subcalculate() {};		// empty by default

	EmptyButton(TWindow *menu, char *identbranch = 0, int asciicode = 0, bool akeepkey = 0);
	virtual ~EmptyButton();
	//virtual void init(TWindow *menu, char *identbranch);


	//int lastfocus, focus;
	void check_focus();

	virtual void handle_menu_focus_loss();

	virtual void handle_focus();
	virtual void handle_defocus();

	virtual bool isvalid();
};


/** \brief A more specialized button type, which provides graphical routines
for display.

*/

class GraphicButton : public EmptyButton
{
public:

	virtual bool hasmouse(BITMAP *bmpref);

	void draw_rect();
	void draw_rect_fancy();
	void draw_boundaries(BITMAP *bmpref);
	// auto-locate button
	GraphicButton(TWindow *menu, char *identbranch, int asciicode, bool akeepkey = 0);

	// to override the auto-locate...
	virtual void locate(int ax, int ay);

//	virtual void init(TWindow *menu, char *identbranch, int ax, int ay);
	virtual ~GraphicButton();

	void init_pos_size(BITMAP **bmp_default, char *idstr);
	void locate_by_backgr(char *stron);
	// should be a function of the main window, really ...

	BITMAP *getbmp(char *name);
	BITMAP *getbmp_nobutton(char *name);

	virtual void animate();
	virtual void draw_default();
	virtual void draw_focus();
	virtual void draw_selected();

	// masked_blits the specified bitmap using the button coordinates.
	bool draw(BITMAP *b);
};

#endif  // __TWBUTTON_H__
