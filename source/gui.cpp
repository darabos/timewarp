#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <allegro.h>
#include <allegro/internal/aintern.h>
#include "melee.h"
REGISTER_FILE

#include "gui.h"
#include "melee/mgame.h"
#include "melee/mfleet.h"
#include "util/aastr.h"

/*

TimeWarp Dialog

like Allegro's popup-dialog, except
1.  scales for screen resolution, based upon a 
default of 640x480
2.  automatically sets background/forground colors 
to black and white
3.  like popup, only automatic

*/

#define TW_DIALOG_DEFAULT_RES_X 640
#define TW_DIALOG_DEFAULT_RES_Y 480

static int tw_gui_color_transform ( int c ) {
	if (0) ;
	else if (c ==255) c = makecol(0,0,0);
	else if (c ==  0) c = makecol(255,255,255);
	else if (c < 256) c = palette_color[c];
	else c = tw_color(c&255,(c>>8)&255,(c>>16)&255) | (c &0xff000000);
	return c;
}

static FONT *tw_gui_get_font ( int c ) {
	int f, g, h;
	if (0) ;
	else if (videosystem.width <= 250) g = 0;
	else if (videosystem.width <= 320) g = 1;
	else if (videosystem.width <= 400) g = 2;
	else if (videosystem.width <= 640) g = 3;
	else if (videosystem.width <= 800) g = 4;
	else if (videosystem.width <= 1024) g = 5;
	else if (videosystem.width <= 1280) g = 6;
	else if (videosystem.width <= 1600) g = 7;
	else g = 8;
	if (0) ;
	else if (videosystem.height <= 180) h = 0;
	else if (videosystem.height <= 240) h = 1;
	else if (videosystem.height <= 300) h = 2;
	else if (videosystem.height <= 480) h = 3;
	else if (videosystem.height <= 600) h = 4;
	else if (videosystem.height <= 768) h = 5;
	else if (videosystem.height <= 1024) h = 6;
	else if (videosystem.height <= 1280) h = 7;
	else h = 8;
	f = (c + g + h + 1) / 2;
	return videosystem.get_font(f);
}

void TW_Dialog_Player::redraw() {
	int i;
	for (i = 0; player->dialog[i].proc; i++) player->dialog[i].flags |= D_DIRTY;
	update();
	return;
}

void TW_Dialog_Player::_event( Event * e) {
	switch (e->type) {
		case Event::VIDEO: {
			if (e->subtype == VideoEvent::REDRAW)
				this->redraw();
		} break;
	}
}

void TW_Dialog_Player::init(VideoWindow *w, DIALOG *d, int focus) {
	dialog = d;
	window = w;
	ifocus = focus;
	player = NULL;
	if (!window) window = &videosystem.window;
	window->add_callback(this);
	int i;
	for (i = 0; d[i].proc; i += 1) ;
	length = i;

	level = d[i].d2;
	d[length].d2 += 1;

	prev_level = (TW_Dialog_Player*) d[length].dp;
	d[length].dp = this;

	old_sizes = new int[length * 6];

	int ox = d[length].x, oy = d[length].y;
	int ow = d[length].w, oh = d[length].h;
	if (!ox) ox = 0;
	if (!oy) oy = 0;
	if (!ow) ow = TW_DIALOG_DEFAULT_RES_X;
	if (!oh) oh = TW_DIALOG_DEFAULT_RES_Y;

	for (i = 0; i < length; i += 1) {
		old_sizes[i*6+0] = d[i].x;
		old_sizes[i*6+1] = d[i].y;
		old_sizes[i*6+2] = d[i].w;
		old_sizes[i*6+3] = d[i].h;
		old_sizes[i*6+4] = d[i].fg;
		old_sizes[i*6+5] = d[i].bg;
	}

	for (i = 0; i < length; i += 1) {
		d[i].fg = tw_gui_color_transform(d[i].fg);
		d[i].bg = tw_gui_color_transform(d[i].bg);
		d[i].x = window->x + window->w * (d[i].x - ox) / ow;
		d[i].y = window->y + window->h * (d[i].y - oy) / oh;
		d[i].w = window->w * d[i].w / ow;
		d[i].h = window->h * d[i].h / oh;
	}

	d[length].dp2 = font;
	font = tw_gui_get_font(d[length].d1);

	subscreen = create_sub_bitmap( window->surface, window->x, window->y, window->w, window->h);

	return;
}
void TW_Dialog_Player::deinit() {
	int i;
	window->remove_callback(this);
	dialog[length].d2 -= 1;
	if (dialog[length].d2 != level) {
		tw_error("TW_Dialog_Player::deinit - inconsistent GUI order");
	}
	dialog[length].dp = prev_level;
	font = (FONT*) dialog[length].dp2;
	for (i = 0; i < length; i += 1) {
		dialog[i].x  = old_sizes[i*6+0];
		dialog[i].y  = old_sizes[i*6+1];
		dialog[i].w  = old_sizes[i*6+2];
		dialog[i].h  = old_sizes[i*6+3];
		dialog[i].fg = old_sizes[i*6+4];
		dialog[i].bg = old_sizes[i*6+5];
	}
	delete[] old_sizes;
	if (player) i = shutdown_dialog ( player );
	else i = -1;
	destroy_bitmap(subscreen);
	return;
}
int TW_Dialog_Player::update() {
	videosystem.poll_redraw();
	BITMAP *old = screen;
	screen = subscreen;
	if (!player) player = init_dialog(dialog, ifocus);
	int i = update_dialog ( player );
	screen = old;
	if (!i) {
		return player->obj;
	}
	return -2;
}

/*
static void tw_dialog_post ( VideoWindow *window, DIALOG *d, int n ) {
}*/

int tw_do_dialog ( VideoWindow *window, DIALOG *d, int index ) {
	int return_value;
	TW_Dialog_Player bob;
	if (!window) window = &videosystem.window;
	bob.init(window, d, index);

	while (keypressed()) readkey();

	show_mouse(window->surface);
	while ((-2 == (return_value = bob.update())));
	bob.deinit();
	show_mouse(NULL);

	return return_value;
}
int tw_popup_dialog ( VideoWindow *window, DIALOG *d, int index ) {
	BITMAP *tmp;
	int n, i, return_value;
	if (!window) window = &videosystem.window;
	for (n = 0; d[n].proc; n += 1) ;
	if (!window->surface) error ("tw_dialog_pre - no drawing surface");
	if ((index >= n) || (index < 0)) 
		error("tw_dialog - index invalid");

	TW_Dialog_Player bob;
	bob.init( window, d, index);

	int x=9999, y=9999, w=0, h=0;
	for (i = 0; i < n; i += 1) if (d[i].w && d[i].h) {
		if (d[i].x < x) x = d[i].x;
		if (d[i].y < y) y = d[i].y;
		if (d[i].x + d[i].w > w) w = d[i].x + d[i].w;
		if (d[i].y + d[i].h > h) h = d[i].y + d[i].h;
	}
	w -= x;
	h -= y;
	tmp = create_bitmap_ex(bitmap_color_depth(window->surface), w, h);
	window->lock();
	blit(window->surface, tmp, x, y, 0, 0, w, h);
	window->unlock();

	while (keypressed()) readkey();

	show_mouse(window->surface);
	while ((-2 == (return_value = bob.update())));
	bob.deinit();
	show_mouse(NULL);

	window->lock();
	blit(tmp, window->surface, 0, 0, x, y, w, h);
	window->unlock();
	destroy_bitmap(tmp);
	return return_value;
}

/*
Configuration string format

a/b/c/d == current config option a/b/c/d
/ini/a/b/c/d == ini option d in section c in file b in directory a
/dev/a/b == option b in device a
/cfg/a/b/c/d == read from ini, write to ini & config if they match

*/
const char *_get_ini_string ( const char *name ) {
	char *item;
	char *section;
	char file[1024];
	if (strlen(name) > 1000) error("_get_ini_string - name too long");
	strncpy (file, name, 1000);
	char *_slash[2];
	_slash[0] = strchr(file, '/');
	if (!_slash[0]) {
		tw_error("_get_ini_string - bad name (%s)", name);
		return NULL;
	}
	_slash[1] = strchr(_slash[0] + 1, '/');
	if (!_slash[1]) {
		tw_error("_get_ini_string - bad name (%s)", name);
		return NULL;
	}
	
	int i = 1;

	char *tmp = strchr(_slash[i] + 1, '/');
	while ( tmp )
	{
		i ^= 1;
		_slash[i] = tmp;
		tmp = strchr(_slash[i] + 1, '/');
	}

	item = _slash[i] + 1;
	item[-1] = 0;
	section = _slash[i^1] + 1;
	section[-1] = 0;
	set_config_file(file);
	return get_config_string(section, item, NULL);
}
void _set_ini_string ( const char *name, const char *value ) {
	char *item;
	char *section;
	char file[1024];
	if (strlen(name) > 1000) error("_set_ini_string - name too long");
	strncpy (file, name, 1000);
	char *_slash[2];
	_slash[0] = strchr(file, '/');
	if (!_slash[0]) {
		tw_error("_set_ini_string - bad name (%s)", name);
		return;
	}
	_slash[1] = strchr(_slash[0]+1, '/');
	if (!_slash[1] ) {
		tw_error("_set_ini_string - bad name (%s)", name);
		return;
	}
	
	int i = 1;
	char *tmp = strchr(_slash[i] + 1, '/');
	while (tmp ) {
		i ^= 1;
		_slash[i] = tmp;
		tmp = strchr(_slash[i] + 1, '/');
	}

	item = _slash[i] + 1;
	item[-1] = 0;
	section = _slash[i^1] + 1;
	section[-1] = 0;
	set_config_file(file);
	set_config_string(section, item, value);
	return;
}
const char * twconfig_get_string (const char *item) {
	char buffy[256];
	static char result[512];
	enum { INI, CFG, DEV, NORMAL};
	int type = -1;
	strcpy(buffy, item);
	if (*item == '/') {
		if (0) ;
		//else if (!strncmp(item, "/dev/", 5)) type = DEV;
		else if (!strncmp(item, "/ini/", 5)) type = INI;
		else if (!strncmp(item, "/cfg/", 5)) type = CFG;
		else error("twconfig_get_string - unknown prefix");
	}
	else type = NORMAL;
	ConfigEvent ce;

	ce.value = NULL;
	ce.subtype = ConfigEvent::GET;

	ASSERT(type!=-1);
	switch (type) {
		case NORMAL: {
			ce.name = buffy;
			if (game) game->_event(&ce);
		} break;
		case INI: {
			ce.value = strdup(_get_ini_string(&buffy[5]));
		} break;
		case CFG: {
			ce.name = &buffy[5];
			if (game) game->_event(&ce);
			if (!ce.value) ce.value = strdup(_get_ini_string(&buffy[5]));
		} break;
		case DEV: {
			//ce.name = buffy;
			//if (game) game->_event(&ce);
		} break;
	}
	if (ce.value) {
		strncpy(result, ce.value, 500);
		free(ce.value);
	}
	return result;
}

void twconfig_set_string (const char *item, const char *value){
	char buffy[256];
	char buffy2[256];
	enum { INI, CFG, DEV, NORMAL};
	int type = -1;
	strcpy(buffy, item);
	if (*item == '/') {
		if (0) ;
		//else if (!strncmp(item, "/dev/", 5)) type = DEV;
		else if (!strncmp(item, "/ini/", 5)) type = INI;
		else if (!strncmp(item, "/cfg/", 5)) type = CFG;
		else error("twconfig_get_string - unknown prefix");
	}
	else type = NORMAL;
	ConfigEvent ce;

	ce.name = buffy2;
	ce.value = (char*) value;
	ce.type = Event::TW_CONFIG;
	ce.subtype = ConfigEvent::SET;

	ASSERT(type!=-1);
	switch (type) {
		case NORMAL: {
			ce.name = buffy;
			if (game) game->_event(&ce);
		} break;
		case INI: {
			_set_ini_string(&buffy[5], value);
		} break;
		case CFG: {
			_set_ini_string(&buffy[5], value);
			ce.name = &buffy[5];
			if (game) game->_event(&ce);
		} break;
		case DEV: {
			//ce.name = buffy;
			//if (game) game->_event(&ce);
		} break;
	}
	return;
}

int twconfig_get_int (const char *item) {
	const char *v = twconfig_get_string(item);
	if (!v) return 0;
	return atoi(v);;
}

void twconfig_set_int (const char *item, int value){
	char buffy[32];
	sprintf(buffy, "%d", value);
	twconfig_set_string(item, buffy);
	return;
}

double twconfig_get_float ( const char *item ) {
	const char *v = twconfig_get_string(item);
	if (!v) return 0;
	return atof(v);
}

void twconfig_set_float ( const char *item, double value ) {
	char buffy[64];
	sprintf(buffy, "%f", value);
	twconfig_set_string(item, buffy);
	return;
}

/*
commands:
	checkbox
	slider
	text
	button
	textbox
	intbox
	floatbox
	list

they specify
	window region
	range
	item (i.e. "client/audio/volume")
*/

/*MenuItem * read_item( char *blah );

void Menu::preinit() {
	window = NULL;
	item = NULL;
	num_items = 0;
}

void Menu::init (const char *fname) {
	num_items = 0;
	window = new VideoWindow;
	window->preinit();
	load(fname);
}

void Menu::load ( const char * fname ) {
	if (!fname) return;
	FILE *f = fopen ( fname, "rt" );
	char buffy[4096];
	while ( fgets ( buffy, 4096, f) ) {
		MenuItem *m = read_item(buffy);
		m->window->init(window);
		if (m) add(m);
	}

	fclose (f);
	return;
}

void Menu::add ( MenuItem *a ) {
	if ((num_items & 7) == 0) {
		int new_max = num_items + 8;
		item = (MenuItem**) realloc (item, sizeof(MenuItem*) * new_max);
	}
	item[num_items] = a;
	num_items += 1;
	a->window->init(window);
}

void MenuItem::preinit() {
	window = NULL;
	menu = NULL;
	item = NULL;
	value.i = -42;
	return;
}
void MenuItem::animate() {
	return;
}
void MenuItem::left_click() {
	return;
}
void MenuItem::left_unclick() {
	return;
}
void MenuItem::read() {
	return;
}
void MenuItem::write() {
}
void MI_bool::read() {
	const char *dufus = twconfig_get_string(item);
	if (!dufus) {
		value.i = 0;
		return;
	}
	else value.i = 1;
	//strtolwr(dufus);
	if (!strcmp(dufus, "0")) value.i = 0;
	return;
}
void MI_bool::write() {
	if (value.i) twconfig_set_string(item, "1");
	else twconfig_set_string(item, "0");
	return;
}
void MI_int::read() {
	const char *dufus = twconfig_get_string(item);
	if (!dufus) {
		value.i = 0;
		return;
	}
	else value.i = atoi(dufus);
	return;
}
void MI_int::write() {
	char buffy[64];
	sprintf(buffy, "%d", value.i);
	twconfig_set_string(item, buffy);
	return;
}
void MI_bool_checkbox::animate() {
	rect(window->surface, window->x, window->y, window->x+window->w, window->y+window->h, palette_color[15]);
	if (value.i) {
		line(window->surface, 
			window->x, 
			window->y,
			window->x + window->w, 
			window->y + window->h, 
			palette_color[15]);
		line(window->surface, 
			window->x + window->w, 
			window->y,
			window->x, 
			window->y + window->h, 
			palette_color[15]);
	}
	return;
}
void MI_int_slider::animate() {
	rect(window->surface, window->x, window->y, window->x+window->w, window->y+window->h, palette_color[15]);
	int i = value.i;
	if (i < min) i = min;
	if (i > max) i = max;
	if (max - min > 0) {
		int pos = ((i - min) * (window->w - 2) ) / (max-min);
		rect (window->surface, 
			window->x + pos, window->y, 
			window->x + pos + 2, window->y + window->h, 
			palette_color[15]
			);
	}

	return;	
}
char *skip_white ( char * input ) {
	for (;; input += 1) {
		if (*input == ' ') continue;
		if (*input == '\t') continue;
		break;
	}
	return input;
}


void get_token (char * input, char **token, char **remainder) {
	*token = NULL;
	*remainder = NULL;
	if (!input) error ("gui.cpp - get_token had NULL input");
	input = skip_white(input);
	if (*input == 0) return;
	*token = input;
	while (true) {
		input += 1;
		if (*input == 0) break;
		if (*input == ' ') break;
		if (*input == '\t') break;
	}
	char * end = input;
	input = skip_white(input);
	if (*input != 0) *remainder = input;
	*end = 0;
	return;
}

MenuItem * read_item ( char *blah ) {
	MenuItem *r = NULL;
	char *word;
	get_token (blah, &word, &blah);
	if (0) {
	} else if (!strcmp (word, "checkbox")) {
		r = new MI_bool_checkbox;
	} else ;//invalid command
	if (r) r->init(blah);
	return r;
}


void MI_bool_checkbox::init ( char * params ) {
	char *t;
	double cx, px, cy, py, cw, pw, ch, ph;
	get_token (params, &t, &params);
	cx = atof(t);
	get_token (params, &t, &params);
	px = atof(t);
	get_token (params, &t, &params);
	cy = atof(t);
	get_token (params, &t, &params);
	py = atof(t);
	get_token (params, &t, &params);
	cw = atof(t);
	get_token (params, &t, &params);
	pw = atof(t);
	get_token (params, &t, &params);
	ch = atof(t);
	get_token (params, &t, &params);
	ph = atof(t);
	
	window = new VideoWindow();
	window->preinit();
	window->locate(cx, px, cy, py, cw, pw, ch, ph);
	
	get_token (params, &t, &params);
	item = strdup(t);
}


void MI_int_slider::init ( char * params ) {
	char *t;
	double cx, px, cy, py, cw, pw, ch, ph;
	get_token (params, &t, &params);
	cx = atof(t);
	get_token (params, &t, &params);
	px = atof(t);
	get_token (params, &t, &params);
	cy = atof(t);
	get_token (params, &t, &params);
	py = atof(t);
	get_token (params, &t, &params);
	cw = atof(t);
	get_token (params, &t, &params);
	pw = atof(t);
	get_token (params, &t, &params);
	ch = atof(t);
	get_token (params, &t, &params);
	ph = atof(t);
	
	window = new VideoWindow();
	window->preinit();
	window->locate(cx, px, cy, py, cw, pw, ch, ph);
	
	get_token (params, &t, &params);
	item = strdup(t);
}
*/



/* 

Just some minor extensions to Allegros GUI 
It looks much more complicated than it is
or something

*/


/* d_bitmap_proc:
 *  Simple dialog procedure: draws the bitmap which is pointed to by dp.
 */
int d_tw_bitmap_proc(int msg, DIALOG *d, int c)
{
	BITMAP *b = (BITMAP *)d->dp;

/*	if (b && (msg==MSG_DRAW))
		_aa2_stretch_blit(b, screen, d->x, d->y, d->w, d->h, true);*/
	if (b && ((msg==MSG_IDLE) || (msg==MSG_DRAW))) {
		int ocl, oct, ocr, ocb;
		ocl = screen->cl; oct = screen->ct; ocr = screen->cr; ocb = screen->cb;
		screen->cl = d->x; screen->ct = d->y; screen->cr = d->x+d->w-1; screen->cb = d->y+d->h+1;
		rotate_sprite ( screen, b, d->x+d->w/2 - b->w/2, d->y+d->h/2 - b->h/2, get_time() * 4096 );
		screen->cl = ocl; screen->ct = oct; screen->cr = ocr; screen->cb = ocb;
		//_aa2_stretch_blit(b, screen, d->x, d->y, d->w, d->h, true);
	}
	return D_O_K;
}


/* d_yield_proc:
 *  Simple dialog procedure which just yields the timeslice when the dialog
 *  is idle.
 * changed: calls idle instead of yield
 * added: checks for redraw, handles it if necessary
 */
#include "melee/mview.h"
int d_tw_yield_proc(int msg, DIALOG *d, int c)
{

	if (msg == MSG_IDLE) {
		rest(20);
	}

	return D_O_K;
}



char *genericListboxGetter(int index, int *list_size, char **_list) 
{
  if(index < 0) {
		index = 0;
		while (_list[index]) index += 1;
    *list_size = index;
    return NULL;
  } else {
    return(_list[index]);
  }
}
char *shipListboxGetter(int index, int *list_size)
{
  if(index < 0) {
    *list_size = reference_fleet->getSize();
    return NULL;
  } else
	  if (reference_fleet->getShipType(index)) return (char *) reference_fleet->getShipType(index)->name;
	  else return "(none)";
}
char *shippointsListboxGetter(int index, int *list_size)
{
	static char buffy[256];
	if(index < 0) {
		*list_size = reference_fleet->getSize();
		return NULL;
	} else {
        ShipType * type = reference_fleet->getShipType(index);
		sprintf(buffy, "%3d %s", type->cost, type->name);
	}
	return buffy;
}
char *fleetListboxGetter(int index, int *list_size, Fleet *fleet)
{
  if(index < 0) {
    *list_size = fleet->getSize();
    return NULL;
  } else
	  if (fleet->getShipType(index)) return (char *) fleet->getShipType(index)->name;
	  else return "(none)";
}
char *fleetpointsListboxGetter(int index, int *list_size, Fleet *fleet)
{
    //return "Disco!";
   ASSERT(fleet);
   static char buffy[256];
   
   if(index < 0) {
      *list_size = fleet->getSize();
      return NULL;
   } else
	  if (fleet->getShipType(index)) {
		  sprintf(buffy, "%3d %s", fleet->getShipType(index)->cost, fleet->getShipType(index)->name);
		  return buffy;
	  }
	  else return "(none)";
}

/* idle_cb:
 *  rest_callback() routine to keep dialogs animating nice and smoothly.
 */
static void idle_cb(void)
{
   broadcast_dialog_message(MSG_IDLE, 0);
}

typedef char *(*getfuncptr2)(int, int *, char **);

/* _handle_listbox_click:
 *  Helper to process a click on a listbox, doing hit-testing and moving
 *  the selection.
 */
void _handle_listbox_click2(DIALOG *d)
{
   char *sel = (char*)d->dp2;
   int listsize, height;
   int i, j;

   (*(getfuncptr2)d->dp)(-1, &listsize, (char**)d->dp3);
   if (!listsize)
      return;

   height = (d->h-4) / text_height(font);

   i = MID(0, ((gui_mouse_y() - d->y - 2) / text_height(font)), 
	      ((d->h-4) / text_height(font) - 1));
   i += d->d2;
   if (i < d->d2)
      i = d->d2;
   else {
      if (i > d->d2 + height-1)
	 i = d->d2 + height-1;
      if (i >= listsize)
	 i = listsize-1;
   }

   if (gui_mouse_y() <= d->y)
      i = MAX(i-1, 0);
   else if (gui_mouse_y() >= d->y+d->h-1)
      i = MIN(i+1, listsize-1);

   if (i != d->d1) {
      if (sel) {
	 if (key_shifts & (KB_SHIFT_FLAG | KB_CTRL_FLAG)) {
	    if ((key_shifts & KB_SHIFT_FLAG) || (d->flags & D_INTERNAL)) {
	       for (j=MIN(i, d->d1); j<=MAX(i, d->d1); j++)
		  sel[j] = TRUE;
	    }
	    else
	       sel[i] = !sel[i];
	 }
      }

      d->d1 = i;
      i = d->d2;
      _handle_scrollable_scroll(d, listsize, &d->d1, &d->d2);

      d->flags |= D_DIRTY;

      if (i != d->d2)
	 rest_callback(MID(10, text_height(font)*16-d->h-1, 100), idle_cb);
   }
}

/* draw_listbox:
 *  Helper function to draw a listbox object.
 */
void _draw_listbox2(DIALOG *d)
{
    int height, listsize, i, len, bar, x, y, w;
    int fg_color, fg, bg;
    char *sel = (char*) d->dp2;
    char s[1024];
    int rtm;
    
    (*(getfuncptr2)d->dp)(-1, &listsize, (char**) d->dp3);
    height = (d->h-4) / text_height(font);
    bar = (listsize > height);
    w = (bar ? d->w-15 : d->w-3);
    fg_color = (d->flags & D_DISABLED) ? gui_mg_color : d->fg;
    
    /* draw box contents */
    for (i=0; i<height; i++) {
        if (d->d2+i < listsize) {
            if (d->d2+i == d->d1) {
                fg = d->bg;
                bg = fg_color;
            } 
            else if ((sel) && (sel[d->d2+i])) { 
                fg = d->bg;
                bg = gui_mg_color;
            }
            else {
                fg = fg_color;
                bg = d->bg;
            }
            usetc(s, 0);
            ustrncat(s, (*(getfuncptr2)d->dp)(i+d->d2, NULL, (char**)d->dp3), sizeof(s)-ucwidth(0));
            x = d->x + 2;
            y = d->y + 2 + i*text_height(font);
            rtm = text_mode(bg);
            rectfill(screen, x, y, x+7, y+text_height(font)-1, bg); 
            x += 8;
            len = ustrlen(s);
            while (text_length(font, s) >= MAX(d->w - 1 - (bar ? 22 : 10), 1)) {
                len--;
                usetat(s, len, 0);
            }
            textout(screen, font, s, x, y, fg);
            text_mode(rtm);
            x += text_length(font, s);
            if (x <= d->x+w) 
                rectfill(screen, x, y, d->x+w, y+text_height(font)-1, bg);
        }
        else {
            rectfill(screen, d->x+2,  d->y+2+i*text_height(font), 
                d->x+w, d->y+1+(i+1)*text_height(font), d->bg);
        }
    }
    
    if (d->y+2+i*text_height(font) <= d->y+d->h-3)
        rectfill(screen, d->x+2, d->y+2+i*text_height(font), 
        d->x+w, d->y+d->h-3, d->bg);/**/

    //rectfill(screen, 100,100,200,200,d->bg);
    _draw_scrollable_frame(d, listsize, d->d2, height, fg_color, d->bg);
    /* draw frame, maybe with scrollbar */
    //_draw_scrollable_frame(d, listsize, 4, height, fg_color, d->bg);
    //_draw_scrollable_frame(d, listsize, d->d2, height, fg_color, d->bg);
}


/* d_list_proc:
 *  A list box object. The dp field points to a function which it will call
 *  to obtain information about the list. This should follow the form:
 *     const char *<list_func_name> (int index, int *list_size);
 *  If index is zero or positive, the function should return a pointer to
 *  the string which is to be displayed at position index in the list. If
 *  index is  negative, it should return null and list_size should be set
 *  to the number of items in the list. The list box object will allow the
 *  user to scroll through the list and to select items list by clicking
 *  on them, and if it has the input focus also by using the arrow keys. If 
 *  the D_EXIT flag is set, double clicking on a list item will cause it to 
 *  close the dialog. The index of the selected item is held in the d1 
 *  field, and d2 is used to store how far it has scrolled through the list.
 */
int d_list_proc2(int msg, DIALOG *d, int c)
{
   int listsize, i, bottom, height, bar, orig;
   char *sel = (char*) d->dp2;
   int redraw = FALSE;

   switch (msg) {

      case MSG_START:
	 (*(getfuncptr2)d->dp)(-1, &listsize, (char**)d->dp3);
	 _handle_scrollable_scroll(d, listsize, &d->d1, &d->d2);
	 break;

      case MSG_DRAW:
	 _draw_listbox2(d);
	 break;

      case MSG_CLICK:
	 (*(getfuncptr2)d->dp)(-1, &listsize, (char **) d->dp3);
	 height = (d->h-4) / text_height(font);
	 bar = (listsize > height);
	 if ((!bar) || (gui_mouse_x() < d->x+d->w-13)) {
	    if ((sel) && (!(key_shifts & KB_CTRL_FLAG))) {
	       for (i=0; i<listsize; i++) {
		  if (sel[i]) {
		     redraw = TRUE;
		     sel[i] = FALSE;
		  }
	       }
	       if (redraw) {
		  scare_mouse();
		  SEND_MESSAGE(d, MSG_DRAW, 0);
		  unscare_mouse();
	       }
	    }
	    _handle_listbox_click2(d);
	    while (gui_mouse_b()) {
	       broadcast_dialog_message(MSG_IDLE, 0);
	       d->flags |= D_INTERNAL;
	       _handle_listbox_click2(d);
	       d->flags &= ~D_INTERNAL;
	    }
         }
	 else {
	    _handle_scrollable_scroll_click(d, listsize, &d->d2, height);
	 }
	 break;

      case MSG_DCLICK:
	 (*(getfuncptr2)d->dp)(-1, &listsize, (char **) d->dp3);
	 height = (d->h-4) / text_height(font);
	 bar = (listsize > height);
	 if ((!bar) || (gui_mouse_x() < d->x+d->w-13)) {
	    if (d->flags & D_EXIT) {
	       if (listsize) {
		  i = d->d1;
		  SEND_MESSAGE(d, MSG_CLICK, 0);
		  if (i == d->d1) 
		     return D_CLOSE;
	       }
	    }
	 }
	 break;

      case MSG_WHEEL:
	 (*(getfuncptr2)d->dp)(-1, &listsize, (char **) d->dp3);
	 height = (d->h-4) / text_height(font);
	 if (height < listsize) {
	    int delta = (height > 3) ? 3 : 1;
	    if (c > 0) 
	       i = MAX(0, d->d2-delta);
	    else
	       i = MIN(listsize-height, d->d2+delta);
	    if (i != d->d2) {
	       d->d2 = i;
	       scare_mouse();
	       SEND_MESSAGE(d, MSG_DRAW, 0);
	       unscare_mouse(); 
	    }
	 }
	 break;

      case MSG_KEY:
	 (*(getfuncptr2)d->dp)(-1, &listsize, (char **) d->dp3);
	 if ((listsize) && (d->flags & D_EXIT))
	    return D_CLOSE;
	 break;

      case MSG_WANTFOCUS:
	 return D_WANTFOCUS;

      case MSG_CHAR:
	 (*(getfuncptr2)d->dp)(-1, &listsize, (char **) d->dp3);

	 if (listsize) {
	    c >>= 8;

	    bottom = d->d2 + (d->h-4)/text_height(font) - 1;
	    if (bottom >= listsize-1)
	       bottom = listsize-1;

	    orig = d->d1;

	    if (c == KEY_UP)
	       d->d1--;
	    else if (c == KEY_DOWN)
	       d->d1++;
	    else if (c == KEY_HOME)
	       d->d1 = 0;
	    else if (c == KEY_END)
	       d->d1 = listsize-1;
	    else if (c == KEY_PGUP) {
	       if (d->d1 > d->d2)
		  d->d1 = d->d2;
	       else
		  d->d1 -= (bottom - d->d2) ? bottom - d->d2 : 1;
	    }
	    else if (c == KEY_PGDN) {
	       if (d->d1 < bottom)
		  d->d1 = bottom;
	       else
		  d->d1 += (bottom - d->d2) ? bottom - d->d2 : 1;
	    } 
	    else 
	       return D_O_K;

	    if (sel) {
	       if (!(key_shifts & (KB_SHIFT_FLAG | KB_CTRL_FLAG))) {
		  for (i=0; i<listsize; i++)
		     sel[i] = FALSE;
	       }
	       else if (key_shifts & KB_SHIFT_FLAG) {
		  for (i=MIN(orig, d->d1); i<=MAX(orig, d->d1); i++) {
		     if (key_shifts & KB_CTRL_FLAG)
			sel[i] = (i != d->d1);
		     else
			sel[i] = TRUE;
		  }
	       }
	    }

	    /* if we changed something, better redraw... */ 
	    _handle_scrollable_scroll(d, listsize, &d->d1, &d->d2);
	    d->flags |= D_DIRTY;
	    return D_USED_CHAR;
	 }
	 break;
   }

   return D_O_K;
}
