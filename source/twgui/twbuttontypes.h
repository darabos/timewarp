
#ifndef __TW_BUTTONTYPES__
#define __TW_BUTTONTYPES__


#include "twbutton.h"



// to implement a button, you add bitmaps-feedback to the box-area control

class Button : public GraphicButton
{
protected:

public:
	BITMAP *bmp_default, *bmp_focus, *bmp_selected;

	Button(TWindow *menu, char *identbranch, int asciicode = 0, bool keepkey = 0);
	virtual ~Button();

	virtual void draw_default();
	virtual void draw_focus();
	virtual void draw_selected();

	virtual bool hasmouse();
	virtual bool isvalid();
};



class Area : public GraphicButton
{
protected:

public:
	BITMAP *backgr;
	bool markfordeletion;

	Area(TWindow *menu, char *identbranch, int asciicode = 0, bool akeepkey = 0);
	virtual ~Area();

	virtual void changebackgr(char *fname);
	virtual void changebackgr(BITMAP *newb);

	virtual void animate();		// shouldn't be changed.

	virtual bool hasmouse();
	virtual bool isvalid();
};



// something which has a background, and its own drawing-area
class AreaTablet : public Area
{
protected:

public:
	BITMAP *drawarea;

	AreaTablet(TWindow *menu, char *identbranch, int asciicode = 0, bool akeepkey = 0);
	virtual ~AreaTablet();

	virtual void animate();		// shouldn't be changed.
	virtual void subanimate();

	virtual bool hasmouse();
	virtual bool isvalid();
};





// a switch, is a button that is always in 2 of states (on or off) :
// (a "focus" is always indicated by some box around it).

class SwitchButton : public GraphicButton
{
protected:
public:
	BITMAP	*bmp_on, *bmp_off;

	bool	state;	// true=on, false=off
	// x, y, W, H are inside the draw area
	SwitchButton(TWindow *menu, char *identbranch, int asciicode = 0, bool initialstate = false);
	virtual ~SwitchButton();

	virtual void calculate();

	virtual void draw_default();
	virtual void draw_focus();
	virtual void draw_selected();	// is the same as focus, cause a switch cannot be selected all the time !!

	virtual bool hasmouse();
	//void locate_by_backgr(char *stron);
	virtual bool isvalid();
};



class GhostButton : public EmptyButton
{
public:
	GhostButton(TWindow *menu);
	virtual ~GhostButton();
};





class ScrollBar : public AreaTablet
{
protected:
public:
	BITMAP	*button;

	enum {hor = 1, ver= 2}	direction ;

	double relpos;

	int pmin, pmax, bwhalf, bhhalf;	// p = position (can be x or y)
	int pbutton;

	// x, y, W, H are inside the draw area
	ScrollBar(TWindow *menu, char *identbranch);
	virtual ~ScrollBar();

	virtual void handle_lhold();

	virtual void subanimate();

	void setrelpos(double arelpos);

	virtual void calculate();
};


#endif


