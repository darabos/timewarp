
#ifndef __TWGUI_AREA__
#define __TWGUI_AREA__

#include <list>
//using namespace std;

#include "utils.h"


class AreaReserve;

class AreaGeneral
{
protected:

public:

	bool selected;		// a general-purpose flag; is set if left mouse is held; this can
						// be useful in simple applications.

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
	void calculate();

	void init(AreaReserve *menu, char *identbranch);


	int lastfocus, focus;
	void check_focus();
	void check_defocus();

	virtual void draw_defocus();
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
};


// a menu superstructure ... to manage a bunch of related areas ... in a reserved area/ region

const int max_area = 32;

class AreaReserve
{
protected:
public:

	int				x, y, W, H;
	normalmouse		mouse;
	int				disabled;

	bool			hasfocus;		// so that items can check if the menu is active or not
									// (in case the menu calculations fail, occasionally happens)
									// eg., mouse can move out of the menu, in a separate iteration
									// so that incremental updates don't work.

	char			ident[128];

	int		default_W;
	double	scale;
	BITMAP *originalscreen;	// to backup the original screen content
	BITMAP *screenreserve;	// pointer to (modifiable) original screen
	BITMAP *drawarea;		// intermediately used for drawing ... so that menu items won't draw off this area

	BITMAP *backgr;

	DATAFILE *datafile;		// this file should contain info for all related areas
							// (used for initialization).

	int				Nareas;
	AreaGeneral		*area[max_area];


public:
	AreaReserve(char *identbase, int dx, int dy, int dW, int dH, int default_W, char *datafilename, BITMAP *outputscreen, char *backgrname = 0);
	~AreaReserve();

	// shouldn't be changed
	void add(AreaGeneral *newarea);

	// can be changed
	void calculate();	// calls all menu items
	void animate();

	// return a bmp from the data file
	BITMAP* bmp(char *bmpname);
};





class AreaBox : public AreaGeneral
{
public:
	int x, y, W, H;	// these are used to define the box area (top left corner and dimensions)

	virtual bool hasmouse();

	virtual void init(AreaReserve *menu, char *identbranch, int x, int y, int W, int H);


};




#endif
