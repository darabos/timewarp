
#ifndef __TWGUI_UTILS__
#define __TWGUI_UTILS__


int mapkey(int scancode_key, int scancode_ctrl = 0);


BITMAP *find_datafile_bmp(DATAFILE *datafile, char *identif);

BITMAP *clone_bitmap(int bpp, BITMAP *src, double scale);




class normalmouse
{
protected:

public:

	struct posstr
	{
		int x, y, wheel;	// x,y = position, z = wheel position
		void set(int xnew, int ynew, int znew);
		void move(int dx, int dy);	// so that you can make those values relative?!
	} pos, oldpos, pos2;


	class mousebutton
	{
		struct buttonstr
		{
			double time;
			bool status;			// button status
			bool change;
		} button, oldbutton;

	public:
		void update(bool newstatus);

		// mouse button is suddenly pressed
		bool press();
		// mouse button is suddenly released
		bool release();
		// left mouse button is double clicked
		bool dclick();
		// left mouse button is held
		bool hold();

	} left, mid, right;
	
	

	normalmouse();

	void reset();

	void update();
	void copyinfo(normalmouse *othermouse);


	int xpos() { return pos.x; };
	int ypos() { return pos.y; };
	int wheelpos() { return pos.wheel; };

	void move(int dx, int dy);
	int vx();
	int vy();

	class bmpstr
	{
	public:
		BITMAP *original_mouse_sprite, *newmousebmp;
		void init(BITMAP *newbmp);
		void set();
		void restore();
	} bmp;

};



class AreaGeneral;
class ScrollBar;

// discrete position (x,y = item number, not pixels)
struct scrollpos_str
{
	AreaGeneral *left, *right, *up, *down;
	ScrollBar *scrollhor, *scrollvert;

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
	void bind(AreaGeneral *left, AreaGeneral *right, AreaGeneral *up, AreaGeneral *down,
				ScrollBar *scrollhor, ScrollBar *scrollvert);
	void calculate();
};




#endif



