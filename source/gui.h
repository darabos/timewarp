
/*
Modifications to the Allegro GUI
*/
//for Allegro's d_text_list_proc
char *shipListboxGetter(int index, int *list_size) ;
char *shippointsListboxGetter(int index, int *list_size) ;

int d_list_proc2(int msg, DIALOG *d, int c);
//for the new d_list_proc2
char *genericListboxGetter(int index, int *list_size, char **list) ;
char *fleetListboxGetter(int index, int *list_size, class Fleet *fleet) ;
char *fleetpointsListboxGetter(int index, int *list_size, class Fleet *fleet) ;


/*
TimeWarps own custom GUI
*/
const char *twconfig_get_string (const char *item);
//string destroyed by next call to twconfig_get_*
void   twconfig_set_string (const char *item, const char *value);
int    twconfig_get_int ( const char *item );
void   twconfig_set_int ( const char *item, int value );
void   twconfig_set_float ( const char *item, double value );
double twconfig_get_float ( const char *item );
/*int  twconfig_get_bool ( const char *item );
void twconfig_set_bool ( const char *item, int value );*/




/*struct {
	enum {
		type_void, type_int, type_float, type_string
	};
	int type;
	union {
		void  *v;
		int   *i;
		float *f;
		char  *str;
		VideoWindow *window;
	};
};*/

/*
class MenuItem;

MenuItem * read_item ( char *blah ) ;

class Menu : public BaseClass {
	public:
	VideoWindow * window;
	MenuItem **item;
	int num_items;
	void load ( const char *fname );
	void init ( const char * fname = NULL);
	void add (MenuItem *a);
	virtual void preinit();
};
class MenuItem : public BaseClass {
	public:
	enum {
		STATUS_NEEDS_REDRAW = 1, 
		STATUS_NEEDS_MOUSE = 2
	};
	int status; 
	VideoWindow *window;
	Menu *menu;
	char *item;
	struct {
		int i;
		float f;
		char *s;
		double d;
	} value;

	virtual void animate();
	virtual void left_click();
	virtual void left_unclick();
	virtual void preinit();
	virtual void init ( char *params) = 0;
	virtual void read();
	virtual void write();
};
class MI_bool : public MenuItem {
	public:
	virtual void read();
	virtual void write();
};
class MI_int : public MenuItem {
	public:
	virtual void read();
	virtual void write();
};
class MI_float : public MenuItem {
	public:
	virtual void read();
	virtual void write();
};
class MI_bool_checkbox : public MI_bool {
	public:
	virtual void animate();
	virtual void init( char * params ) ; 
};
class MI_int_slider : public MI_int {
	public:
	virtual void animate();
	virtual void init( char * params ) ; 
	int max, min;
};
*/
