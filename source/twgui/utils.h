/*
Placed in public domain by Rob Devilee, 2004. Share and enjoy!
*/

#ifndef __TWGUI_UTILS_H__
#define __TWGUI_UTILS_H__


// INTERFACE NECESSARY TO SET UP !!

// point to a routine that displays error messages
typedef volatile int (twgui_time_handler_type());
typedef void (twgui_err_handler_type(const char *txt, ...));

extern twgui_time_handler_type *twgui_time;
extern twgui_err_handler_type *twgui_error;



void twgui_init(twgui_time_handler_type t, twgui_err_handler_type f);

int mapkey(int scancode_key, int scancode_ctrl = 0);
int unmapkey1(int k);
int unmapkey2(int k);



BITMAP *find_datafile_bmp(DATAFILE *datafile, char *identif);

BITMAP *clone_bitmap(int bpp, BITMAP *src, double scale, bool vidmem);






// copied from timewarp code (Orz).
struct twguiVector
{
	double x, y;
	twguiVector () {}
	twguiVector (double a, double b) : x(a), y(b) {}
	twguiVector (void *nothing) : x(0), y(0) {}
	twguiVector &operator= (void *nothing) {x = y = 0; return *this;}
	twguiVector &operator+=(twguiVector a) { x+=a.x;y+=a.y; return *this;}
	twguiVector &operator-=(twguiVector a) { x-=a.x;y-=a.y; return *this;}
	twguiVector &operator*=(double a) { x*=a; y*=a; return *this;}
	twguiVector &operator/=(double a) { double b = 1/a; x*=b; y*=b; return *this;}
	twguiVector operator-() const { return twguiVector(-x,-y); }
	bool operator==(twguiVector a) const { return ((x==a.x) && (y==a.y)); }
	bool operator!=(twguiVector a) const { return ((x!=a.x) || (y!=a.y)); }
};

inline twguiVector operator+ (twguiVector a, twguiVector b) { return twguiVector(a.x+b.x,a.y+b.y); }
inline twguiVector operator- (twguiVector a, twguiVector b) { return twguiVector(a.x-b.x,a.y-b.y); }
inline twguiVector operator* (twguiVector a, double b) { return twguiVector(a.x*b,a.y*b); }
inline twguiVector operator* (double b, twguiVector a) { return twguiVector(a.x*b,a.y*b); }
inline twguiVector operator/ (twguiVector a, double b) { double c = 1/b; return twguiVector(a.x*c,a.y*c); }


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






static const int buffmax = 128;

class TKeyHandler
{
private:
	int Nbackbuf;
	int keybackbuf[buffmax];

public:
	int keynew[KEY_MAX];
	int keyold[KEY_MAX];
	int keyhit[KEY_MAX];
	int keyreleased[KEY_MAX];

	int Nbuf;
	int keybuf[buffmax];

	TKeyHandler();
	void update();
	void add(int key);
	void clear();
	void clearbuf();

	bool pressed(char key);
};



#endif // __TWGUI_UTILS_H__



