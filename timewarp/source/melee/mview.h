#ifndef _MELEE_H
#include "../melee.h"
#endif
#ifndef _MFRAME_H
#include "mframe.h"
#endif

#ifndef _MVIEW_H
#define _MVIEW_H


//global flags: read/write
extern int camera_hides_cloakers;
extern int FULL_REDRAW;



//View stuff: READ ONLY!
                              // units
extern View  *space_view;

extern Vector2 space_view_size;// pixels (should be an integer value)

extern Vector2 space_size;     // game-pixels

extern double  space_zoom;     // 1
extern int     space_mip_i;    // ?
extern double  space_mip;      // ?

extern Vector2 space_corner; // game-pixels : uppper left corner
extern Vector2 space_center_nowrap; // game-pixels : center, non-wrapped
extern Vector2 space_vel;        // game-pixels / millisecond : center
extern Vector2 space_center;     // game-pixels : center

struct CameraPosition {
	Vector2 pos;   // game-pixels : unwrapped position of camera
	Vector2 vel; // game-pixels / ms : velocity of camera
	double z;      // game-pixels : zoom of camera position
};

class View;

/*

What we need:

  list of view types
  get view of default type
  set the default type
  get view of specific type

*/

extern int num_views;
extern char **view_name;

struct ViewType {
	View *create( View *old = NULL );
	const char *name;
	View *(*_create)(); //treate as private
};
extern ViewType *viewtypelist;

void __register_viewtype( ViewType n );
#define REGISTER_VIEW(a, b) class a;static View *__create_view ## a (){return new a;} static void __register_view_ ## a () { view_name=(char**)realloc(view_name,sizeof(char*)*(num_views+2));view_name[num_views+1]=NULL;view_name[num_views] = b; viewtypelist=(ViewType*)realloc(viewtypelist,sizeof(ViewType)*(num_views+1));viewtypelist[num_views].name = b; viewtypelist[num_views]._create = __create_view ## a; num_views += 1;} CALL_BEFORE_MAIN( __register_view_ ## a );

void set_view ( View *new_default ) ;
View *get_view( const char *name, View *match ) ;
int get_view_num ( const char * name );

class View : public BaseClass {
	public:
	ViewType *type;
	Frame *frame;
	VideoWindow *window;

	CameraPosition camera;
	Vector2 view_size;//should contain integer values

	virtual void preinit();
	virtual void init( View *old );
	virtual ~View();
	virtual void config();

	void replace ( View * v );
	protected:

	int focus (CameraPosition *pos, SpaceLocation *a, SpaceLocation *b = NULL);
//	void see_also (SpaceLocation *other);
	virtual void track ( const CameraPosition &target, CameraPosition *origin = NULL ) ;
	virtual void track ( const CameraPosition &target, double smooth_time, CameraPosition *origin = NULL) ;
	public:

	int key_zoom_in;
	int key_zoom_out;
	int key_alter1;
	int key_alter2;
	
	virtual void _event( Event *e);


//	virtual void animate_target(Frame *frame, SpaceLocation *t, int dx, int dy, int r, int c) ;
//	virtual void set_background (int color);
	//translates screen coordinates to game coordinates
	virtual bool screen2game(Vector2 *pos);
//	virtual bool game2screen(double *x, double *y);
	//called more or less every game tic, with the amount of time passed
	virtual void calculate(Game *game);


	virtual void prepare(Frame *frame, int time = 0);
	virtual void animate ( Game *game );
	virtual void animate_predict ( Game *game, int time );
	void refresh () ; 
};

class message_type {
	enum { max_messages = 16 };
	struct entry_type {
		char *string;
		int end_time;
		int color;
		} messages[max_messages];
	int ox, oy;
	int num_messages;
	void clean();
	public:
	message_type() {num_messages = ox = oy = 0;}
	void animate(Frame *frame);
	void flush();
	void out(char *string, int dur = 2000, int c = 15);
	void print(int dur, int c, const char *format, ...);
	} extern message;




#endif
