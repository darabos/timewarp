
#ifndef __TWGUI_UTILS__
#define __TWGUI_UTILS__


BITMAP *find_datafile_bmp(DATAFILE *datafile, char *identif);

BITMAP *clone_bitmap(int bpp, BITMAP *src, double scale);


struct button_bitmaps
{
	BITMAP *bmp_defocus, *bmp_focus, *bmp_selected;

	~button_bitmaps();
	// simply sets the pointers to these bitmaps:
	void init(BITMAP *bdefocus, BITMAP *bfocus, BITMAP*bselected);
};



class normalmouse
{
protected:

public:

	struct posstr
	{
		int x, y, wheel;	// x,y = position, z = wheel position
		void set(int xnew, int ynew, int znew);
		void move(int dx, int dy);	// so that you can make those values relative?!
	} pos, oldpos;


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

	void update();
	void copyinfo(normalmouse *othermouse);


	int xpos() { return pos.x; };
	int ypos() { return pos.y; };
	int wheelpos() { return pos.wheel; };



	class bmpstr
	{
	public:
		BITMAP *original_mouse_sprite, *newmousebmp;
		void init(BITMAP *newbmp);
		void set();
		void restore();
	} bmp;

};



// discrete position (x,y = item number, not pixels)
struct scrollpos_str
{
	int	x, y;	// which Icon is visible top left.
	int Nx, Ny;				// number of Icons that are present.
	void set(int xscroll, int yscroll, int Nxscroll, int Nyscroll);
	void add(int dx, int dy);
};




#endif


