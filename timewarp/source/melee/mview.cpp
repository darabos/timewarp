
#include <string.h>
#include <stdio.h>
#include <float.h>
#include <allegro.h>


#include "../melee.h"
REGISTER_FILE
#include "../scp.h"
#include "../frame.h"

#include "mview.h"
#include "mgame.h"
#include "mcbodies.h"
#include "mshppan.h"
#include "mcontrol.h"
#include "mship.h"

#include "../util/aastr.h"

int FULL_REDRAW = 0;
int camera_hides_cloakers = 0;

                              // units
View  *space_view;

Vector2 space_view_size;   // pixels (should be an integer value)

double space_zoom;     // 1
int    space_mip_i;    // ?
double space_mip;      // ?

Vector2 space_size;         // game-pixels

Vector2 space_corner;       // game-pixels : uppper left corner
Vector2 space_center_nowrap;// game-pixels : center, non-wrapped
Vector2 space_vel;          // game-pixels / millisecond : center
Vector2 space_center;       // game-pixels : center



View * ViewType::create( View * old ) {STACKTRACE
	View * r = _create();
	r->preinit();
	r->type = this;
	r->init ( old );
	return r;
}


static View *_default_view = NULL;

int num_views = 0;
char **view_name = NULL;
ViewType *viewtypelist = NULL;


void set_view ( View * new_default ) {STACKTRACE
	if (!new_default) tw_error( "new default view is NULL");
	if (!new_default->type) tw_error("new default view has no type info");
	if (_default_view) delete _default_view;
	_default_view = new_default;
	return;
}




View *get_view ( const char *name, View *old ) {STACKTRACE
	int i;
	if ((name == NULL) && _default_view) {
		name = _default_view->type->name;
	}
	if (!old) old = _default_view;

	i = get_view_num ( name );
	if ( i < 0 ) return NULL;
	View *v = viewtypelist[i].create(old);
	return v;
}

int get_view_num ( const char *name ) {STACKTRACE
	int i;
	if (!name) return -1;
	for (i = 0; i < num_views; i += 1) {
		if (strncmp(viewtypelist[i].name, name, 64) == 0) {
			return i;
		}
	}
	return -1;
}



void View::preinit() {STACKTRACE
	frame = NULL;
	window = NULL;
	type = NULL;
}

void View::refresh () { STACKTRACE
	if (frame) frame->full_redraw = true; 
	return; 
}

void View::prepare ( Frame *frame, int time ) {STACKTRACE

	Vector2 oc = camera.pos;
	camera.pos += camera.vel * time;

	frame->prepare();

	::space_view = this;

	view_size.x = window->w;
	view_size.y = window->h;

	::space_view_size = view_size;
	double tz = magnitude(view_size) / 1.41421356237309504880168872;
	::space_zoom = tz / camera.z;
	::space_mip = -log(space_zoom) / log(2);
	::space_mip_i = iround_down(::space_mip);
	::space_size  = view_size * space_zoom;

	::space_corner = normalize2(camera.pos - space_size /2, map_size);
	::space_center = normalize2(camera.pos, map_size);
	::space_vel = camera.vel;
	::space_center_nowrap = camera.pos;
	
	camera.pos = oc;

	return;
}

void View::animate(Game *game) {STACKTRACE
	if (FULL_REDRAW) frame->full_redraw = true;
	frame->erase();
	prepare(frame);

	if (frame->surface) {
		if (frame->surface) game->animate(frame);
		//WTF???!!!???
		//if (frame->surface) ((Physics*)game)->animate(frame);
		message.animate(frame);
	}

	scare_mouse();
	frame->draw();
	unscare_mouse();
	return;
	}

void View::animate_predict(Game *game, int time) {STACKTRACE
	if (FULL_REDRAW) frame->full_redraw = true;
	frame->erase();
	prepare(frame, time);

	if (frame->surface) {
		if (frame->surface) game->animate_predict(frame, time);
		message.animate(frame);
	}

	scare_mouse();
	frame->draw();
	unscare_mouse();
	return;
	}

void View::config() {STACKTRACE
	return;
	}
bool View::screen2game(Vector2 *_pos) {
	STACKTRACE
	Vector2 pos = *_pos;
	Vector2 opos = pos;

	pos.x -= window->x;
	pos.y -= window->y;

	pos /= space_zoom;

	pos += camera.pos - space_size / 2;
	pos = normalize (pos, map_size);
	*_pos = pos;
	if ((opos.x < window->x) || 
		(opos.x >= window->x + window->w) || 
		(opos.y < window->y) || 
		(opos.y >= window->y + window->h))
		return false;
	return true;
	}
int View::focus(CameraPosition *pos, SpaceLocation *la, SpaceLocation *lb) {
	STACKTRACE
	if (!la && !lb) return 0;
	if (!la) la = lb;
	if (!lb) lb = la;
	Vector2 p, p2;
	p = la->normal_pos();
	p2 = lb->normal_pos();
	double c, d;

	Vector2 a = Vector2(
		min_delta(p.x, p2.x, map_size.x),
		min_delta(p.y, p2.y, map_size.y)
		);

	p = normalize(p-a/2, map_size);

	c = fabs(a.x) + 1;
	d = fabs(a.y) + 1;
	//if (c < d) c = d;
	c = sqrt(c*c + d*d);

	pos->pos = p;
	if (lb != la) {
		pos->z = c;
		return 2;
	}
	else return 1;
}

/*void View::see_also(SpaceLocation *o) {
	if (!o) return;
	double x2, y2;
	x2 = b->normal_x();
	y2 = b->normal_y();
	double a, b, c, d;
	a = min_delta(x, x2, X_MAX);
	b = min_delta(y, y2, Y_MAX);
	c = view_x / (fabs(a) + 1);
	d = view_y / (fabs(b) + 1);
	if (c > d) c = d;
	x = normalize(x-a/2, X_MAX);
	y = normalize(y-b/2, Y_MAX);
	z = c;
	return;
	}*/
//make these static...

void View::track (const CameraPosition &target, CameraPosition *origin) {
	STACKTRACE
	if (!origin) origin = &this->camera;
	Vector2 d;
	d = target.pos - origin->pos;

	d = normalize2(d + map_size/2, map_size) - map_size/2;

	origin->pos += d;
	origin->vel = d / frame_time;
	origin->z = target.z;
}
void View::track (const CameraPosition &target, double smooth_time, CameraPosition *origin) {
	STACKTRACE
	if (!origin) origin = &this->camera;
	Vector2 d;
	d.x = -min_delta( origin->pos.x, target.pos.x, map_size.x);
	d.y = -min_delta( origin->pos.y, target.pos.y, map_size.y);
	Vector2 dd;
	dd = d - origin->vel * frame_time;

	origin->z = target.z;
	//if (origin->z < 100) origin->z = 100;
	//if (origin->z > 10000) origin->z = 10000;

/*	double r1 = (dx * dx + dy * dy) / (frame_time * frame_time);
	double r2 = (ddx * ddx + ddy * ddy) / (frame_time * frame_time);

	if ( 1 ) {
		double d = (log(origin->z) / log(10) -2) /2;
		d = 0.5 - 0.4 * d;
		d = 0.5;
		dx = origin->vx * d + dx * (1.0 - d);
		dy = origin->vy * d + dy * (1.0 - d);
/*		dx = ddx * (1.0 - d);
		dx += origin->vx * (1.0 - d);
		dy -= ddy * (1.0 - d);
		dy += origin->vy * d;*/
/*	}*/

	origin->vel = d / frame_time;
	origin->pos += d;

	return;
}

void View::init(View *old) {STACKTRACE
	if (old) {
		camera = old->camera;
		key_zoom_in = old->key_zoom_in;
		key_zoom_out = old->key_zoom_out;
		key_alter1 = old->key_alter1;
		key_alter2 = old->key_alter2;
		frame = new Frame(1024);
		window = frame->window;
	}
	else {
		frame = new Frame(1024);
		window = frame->window;
		camera.pos = Vector2(0,0);
		camera.z = 960;
		camera.vel = Vector2(0,0);

		set_config_file ( "client.ini" );
		key_zoom_in  = name_to_key(get_config_string("View", "Key_zoomin",  "EQUALS"));
		key_zoom_out = name_to_key(get_config_string("View", "Key_zoomout", "MINUS"));
		key_alter1   = name_to_key(get_config_string("View", "Key_alter1",  "0"));
		key_alter2   = name_to_key(get_config_string("View", "Key_alter2",  "BACKSLASH"));
	}
	window->add_callback(this);
	if (window->surface) ;
	return;
}
void View::replace ( View * v ) {STACKTRACE
	if (frame) delete frame;
	frame = v->frame;
	window = frame->window;
	view_size = v->view_size;

	window->remove_callback(v);
	window->add_callback(this);
	
	v->frame = NULL;
	v->window = NULL;
	delete v;

	return;
}
View::~View() {STACKTRACE
	}
void View::calculate(Game *game) {}

void message_type::out(char *string, int dur, int c) {STACKTRACE
	ASSERT (c < 256);
	if (num_messages == max_messages - 1) {
		messages[0].end_time = -1;
		clean();
		}
	if (num_messages >= max_messages - 1) throw "bad dog!";
	messages[num_messages].string = strdup(string);
	if (game) messages[num_messages].end_time = game->game_time + dur;
	else messages[num_messages].end_time = 0 + dur;
	messages[num_messages].color = palette_color[c];
	num_messages += 1;
	//animate(NULL);clean();return;
	if (!game) animate(NULL);
	else if (game->is_paused()) animate(NULL);
	clean();
	return;
	}
void message_type::print(int dur, int c, const char *format, ...) {STACKTRACE
	char buf[1024];
	va_list those_dots;
	va_start (those_dots, format);
#ifdef ALLEGRO_MSVC
	_vsnprintf(buf, 1000, format, those_dots);
//#elif NO_VSNPRINTF
#else 
	vsprintf(buf, format, those_dots);
	//vsnprintf(buf, 1000, format, those_dots); //it would be nice to use this line...
#endif
	va_end (those_dots);
	out(buf, dur, c);
	return;
	}
void message_type::clean() {STACKTRACE
	int kill_time;
	if (game) kill_time = game->game_time;
	else kill_time = 0;
	for (int i = 0; i < num_messages; i += 1) {
		if (messages[i].end_time <= kill_time) {
			free (messages[i].string);
			num_messages -= 1;
			memmove (&messages[i], &messages[i+1], (num_messages - i) * sizeof(entry_type));
			i -= 1;
			}
		}
	return;
	}
void message_type::flush() {STACKTRACE
	for (int i = 0; i < num_messages; i += 1) {
		free (messages[i].string);
		}
	num_messages = ox = oy = 0;
	return;
	}

void message_type::animate(Frame *frame) {STACKTRACE
	int i, x = 0, y = 0, tmp;
	BITMAP *bmp;
	if (frame) {
		text_mode(-1);
		bmp = frame->surface;
		}
	else {
		text_mode(0);
		videosystem.window.lock();
		bmp = videosystem.window.surface;
		}
	clean();
	if (!frame) rectfill(bmp, 0, 0, ox, oy, 0);
	for (i = 0; i < num_messages; i += 1) {
		textprintf(bmp, font, 0, y, messages[i].color, "%s", messages[i].string);
		tmp = text_length(font, messages[i].string);
		if (x < tmp) x = tmp;
		y += text_height(font);
		}
	if (frame) frame->add_box(0, 0, x, y);	
	else videosystem.window.unlock();
	ox = x;
	oy = y;
	return;
	}
message_type message;

void View::_event( Event *e ) {STACKTRACE
	if ( e->type == Event::VIDEO ) {
		const VideoEvent *ve = (const VideoEvent*) e;
		const VideoWindow *w = ve->window;
		if (w != window) return;
		//if (w->surface) set_window(w->surface, w->x, w->y, w->w, w->h);
	}
}






class View_Everything : public View {
	public:
	virtual void calculate(Game *game);
	};
void View_Everything::calculate (Game *game) {STACKTRACE
	double a, b, c;
	//sqrt(view_w * view_w + view_h * view_h) / 1.41421356237309504880168872
	c = 1.414 / magnitude(view_size);
	a = map_size.x * view_size.x * c;
	b = map_size.y * view_size.y * c;
	if (b > a) a = b;
	camera.z = a;
	return;
	}




class View_Hero : public View {
	double f;
	double max, min;
	public:
	virtual void calculate(Game *game);
	virtual void init(View *old);
	//virtual void set_window (BITMAP *dest, int x, int y, int w, int h);
	};
void View_Hero::init(View *old) {STACKTRACE
	View::init(old);
	f = 0;
	min = 30;//480;
	max = 480000;
	return;
	}

void View_Hero::calculate (Game *game) {STACKTRACE
	CameraPosition n = camera;
	if (key_pressed(key_zoom_in))  n.z /= 1 + 0.002 * frame_time;
	if (key_pressed(key_zoom_out)) n.z *= 1 + 0.002 * frame_time;
	if (n.z < min) n.z = min;
	if (n.z > max) n.z = max;
	if (key_pressed(key_alter1)) f += 0.006 * frame_time;
	else f -= 0.006 * frame_time;
	if (f < 0) f = 0;
	if (f > 1.2) f = 1.2;
	SpaceLocation *c = NULL;
	if (game->num_focuses) c = game->focus[game->focus_index]->get_focus();
	focus ( &n, c );
	if (c) {
		n.pos += (f) * n.z / 4 * unit_vector(c->get_angle_ex());
	}
	track ( n, frame_time );
	return;
}




class View_Enemy : public View {
	public:
	virtual void calculate(Game *game);
	};
void View_Enemy::calculate (Game *game) {STACKTRACE
	SpaceLocation *c = NULL;
	if (game->num_focuses) c = game->focus[game->focus_index]->get_focus();
	if (!c) return;
	CameraPosition n = camera;
	if (c->target && !(camera_hides_cloakers && c->target->isInvisible())) {
		if (c->distance(c->target) < 3000) {
			focus(&n, c, c->target);
			n.z *= 1.4;
		}
		else focus(&n, c);
	}
	else focus(&n, c);
	if (n.z < 480) n.z = 480;
	track(n);
	return;
}




class View_Enemy_Discrete : public View {
	public:
	virtual void calculate(Game *game);
	};
void View_Enemy_Discrete::calculate (Game *game) {STACKTRACE
	SpaceLocation *c = NULL;
	if (game->num_focuses) c = game->focus[game->focus_index]->get_focus();
	if (!c) return;
	CameraPosition n = camera;
	if (c->target && !(camera_hides_cloakers && c->target->isInvisible())) {
		focus(&n, c, c->target);
		n.z *= 1.4;
		}
	else focus(&n, c);

	if (n.z < 480) n.z = 480;

	double ref_size = 480;
	n.z = ref_size * pow(2, ceil(log(n.z/ref_size) / log(2)));
	track(n);
	return;
	}












class View_Split : public View {
	double max, min;
	CameraPosition c1, c2;
	VideoWindow *sub[2];
	Frame *frame2;
	public:
	virtual void calculate(Game *game);
	virtual void init(View *old);
	virtual void animate(Game *game);
	//virtual void set_window (BITMAP *dest, int x, int y, int w, int h);
	};
void View_Split::init(View *old) {STACKTRACE
	View::init(old);

	c1 = camera;
	c2 = camera;
	min = 480;
	max = 4800;
	frame2 = new Frame(1024);
	sub[0] = window;
	sub[1] = frame2->window;
	sub[1]->preinit();
	window = new VideoWindow();
	window->preinit();
	window->match(sub[0]);
	sub[0]->init(window);
	sub[1]->init(window);
	sub[0]->locate(0,0.0, 0,0, 0,0.5, 0, 1);
	sub[1]->locate(0,0.5, 0,0, 0,0.5, 0, 1);
	return;
	}
void View_Split::animate(Game *game) {STACKTRACE
	VideoWindow *tmpw;
	Frame *tmpf;

	tmpw = window;
	tmpf = frame;

	window = sub[0];
	camera = c1;
	View::animate(game);

	frame = frame2;
	window = sub[1];
	camera = c2;
	View::animate(game);

	frame = tmpf;
	window = tmpw;
	view_size.x = window->w;
	view_size.y = window->w;
}

void View_Split::calculate (Game *game) {STACKTRACE
	CameraPosition n;
	SpaceLocation *c;

	n = c1;
	if (key_pressed(key_zoom_in)) n.z *= 1 + 0.002 * frame_time;
	if (key_pressed(key_zoom_out))  n.z /= 1 + 0.002 * frame_time;
	if (n.z < min) n.z = min;
	if (n.z > max) n.z = max;
	c = NULL;
	if (game->num_focuses) c = game->focus[game->focus_index]->get_focus();
	focus ( &n, c );
	track ( n, frame_time, &c1 );

	n = c2;
	if (key_pressed(key_alter1)) n.z *= 1 + 0.002 * frame_time;
	if (key_pressed(key_alter2)) n.z /= 1 + 0.002 * frame_time;
	if (n.z < min) n.z = min;
	if (n.z > max) n.z = max;
	c = NULL;
	if (game->num_focuses) c = game->focus[(game->focus_index + 1) % game->num_focuses]->get_focus();
	focus ( &n, c );
	track ( n, frame_time, &c1 );

	camera = c1;

/*	if (key_pressed(key_zoom_in)) z *= 1 + 0.002 * frame_time;
	if (key_pressed(key_zoom_out))  z /= 1 + 0.002 * frame_time;
	if (z < min) z = min;
	if (z > max) z = max;
	if (key_pressed(key_alter2)) z2 *= 1 + 0.002 * frame_time;
	if (key_pressed(key_alter1))  z2 /= 1 + 0.002 * frame_time;
	if (z2 < min) z2 = min;
	if (z2 > max) z2 = max;
	SpaceLocation *c;
	double tx = x, ty = y;	

	c = NULL;
	if (game->num_focuses) 
		c = game->focus[(game->focus_index+1) % game->num_focuses]->get_focus();
	if (c) {
		focus ( c );
		x += (f) * view_w / z2 * cos(c->get_angle_ex()) / 4;
		y += (f) * view_h / z2 * sin(c->get_angle_ex()) / 4;
		}
	x2 = normalize(x, X_MAX);
	y2 = normalize(y, Y_MAX);
	
	x = tx; y = ty;
	
	c = NULL;
	if (game->num_focuses) c = game->focus[game->focus_index]->get_focus();
	if (c) {
		focus ( c );
		x += (f) * view_w / z * cos(c->get_angle_ex()) / 4;
		y += (f) * view_h / z * sin(c->get_angle_ex()) / 4;
		}
	x = normalize(x, X_MAX);
	y = normalize(y, Y_MAX);*/
	return;
	}



REGISTER_VIEW ( View_Hero, "Hero" )
REGISTER_VIEW ( View_Everything, "Everything" )
REGISTER_VIEW ( View_Enemy, "Enemy" )
REGISTER_VIEW ( View_Enemy_Discrete, "Enemy_Discrete" )
REGISTER_VIEW ( View_Split, "Split" )

