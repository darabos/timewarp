#include <allegro.h>
#include "../melee.h"
REGISTER_FILE


#include "../frame.h"
#include "mframe.h"
#include "mview.h"
#include "mitems.h"
#include "mgame.h"


Indicator::Indicator() : Presence() {STACKTRACE
	}

bool Indicator::coords(Frame *space, SpaceLocation *l, Vector2 *pos, Vector2 *a_pos) {STACKTRACE
	Vector2 p = corner(l->normal_pos());
	Vector2 op = p;

	p -= space_view_size / 2;

	if (p.x < -space->surface->w/2) {
		double a = p.x / -(space->surface->w/2 + 10);
		p /= a;
		}
	if (p.y < -space->surface->h/2) {
		double a = p.y / -(space->surface->h/2 + 10);
		p /= a;
		}
	if (p.x > space->surface->w/2) {
		double a = p.x / (space->surface->w/2 + 10);
		p /= a;
		}
	if (p.y > space->surface->h/2) {
		double a = p.y / (space->surface->h/2 + 10);
		p /= a;
		}
	p += space_view_size / 2;
	*pos = p;
	if (a_pos) *a_pos = op;

	if ((op.x > 0) && (op.x < space->surface->w) &&
		(op.y > 0) && (op.y < space->surface->h)) return false;
	return true;
	}

BlinkyIndicator::BlinkyIndicator(SpaceObject *target, int color) : Indicator() {STACKTRACE
	this->target = target;
	this->color = color;
	}

void BlinkyIndicator::animate(Frame *space) {
	if ((game->game_time >> 8) & 1) return;
	Vector2 p;
	int a = coords(space, target, &p);
	if (!a) return;
	SpaceSprite *s = target->get_sprite();
	p -= s->size() * space_zoom / 2;
	p -= space_view_size / 2;

	p.x -= sign(p.x) * s->width()  / 4 * sqrt(space_zoom);
	p.y -= sign(p.y) * s->height() / 4 * sqrt(space_zoom);
	//shouldn't be sqrt ... maybe there's a bug somewhere

	p += space_view_size / 2;
	if (color == -1) {
		s->draw(p, s->size() * space_zoom, 
			target->get_sprite_index(), space);
	}
	else {
		Vector2 size = s->size() * space_zoom;
		s->draw_character((int)p.x, (int)p.y, (int)size.x, (int)size.y, 
			target->get_sprite_index(), palette_color[color], space);
	}
	return;
	}
void BlinkyIndicator::calculate() {STACKTRACE
	if (!target->exists()) die();
	}

WedgeIndicator::WedgeIndicator(SpaceLocation *target, int length, int color) : Indicator() {STACKTRACE
	this->target = target;
	this->length = length;
	this->color = color;
	}

void WedgeIndicator::animate(Frame *space) {
	double a, a2;
	Vector2 p, p2, tmp;
	if (target->isInvisible() > 0.5) return;
	if (!coords(space, target, &p, &p2)) return;
	a = PI + atan(p - space_view_size / 2);
	int ix, iy;
	//ix = (int)p.x;
	//iy = (int)p.y;
	//line(space->frame, ix, iy, int(ix+cos(a)*length), int(iy+sin(a)*length), pallete_color[color]);
	//space->add_line  ( ix, iy, int(ix+cos(a)*length), int(iy+sin(a)*length));
	tmp = p + unit_vector(a+PI*.15) * length;
	a2 = PI + atan(tmp-p2);
	ix = int(tmp.x);
	iy = int(tmp.y);
	line(space->surface, ix, iy, int(ix+cos(a2)*length), int(iy+sin(a2)*length), pallete_color[color]);
	space->add_line  ( ix, iy, int(ix+cos(a2)*length), int(iy+sin(a2)*length));
	tmp = p + unit_vector(a-PI*.15) * length;
	a2 = PI + atan(tmp-p2);
	ix = int(tmp.x);
	iy = int(tmp.y);
	line(space->surface, ix, iy, int(ix+cos(a2)*length), int(iy+sin(a2)*length), pallete_color[color]);
	space->add_line  ( ix, iy, int(ix+cos(a2)*length), int(iy+sin(a2)*length));
	return;
	}
void WedgeIndicator::calculate() {
	if (!target->exists()) die();
	}



Orbiter::Orbiter ( SpaceSprite *pic, SpaceLocation *orbit_me, double distance) : 
		SpaceObject(NULL, orbit_me->normal_pos(), random(PI2), pic) 
	{STACKTRACE
	layer = LAYER_CBODIES;
	mass = 99;
	center = orbit_me;
	radius = distance;
	pos -= unit_vector(angle) * radius;
	accelerate(this, angle + PI/2 + PI*(random()&1), 0.15, MAX_SPEED);
	}

void Orbiter::calculate() {STACKTRACE
	angle = trajectory_angle(center) + PI;
	sprite_index = get_index(angle);
	double r = distance(center) / radius;


	if (r < 1) {
		accelerate(this, angle +PI, 0.0001 * (r-1) * frame_time, MAX_SPEED);
		}
	else {
		accelerate(this, angle, 0.0001 * (1-r) * frame_time, MAX_SPEED);
		if (r > 4) translate(unit_vector(angle) * radius*(4-r));
	}
	if (random() & 3) return;
	double  va, vb, a;
	Vector2 d;
	d = pos - nearest_pos(center);
	a = magnitude(d);
	d /= a;
	va = vel.x * d.x + vel.y * d.y;
	vb = vel.x * d.y - vel.y * d.x;
	if ((r > 1) && (va > 0)) {
		va *= 1 - 0.001 * frame_time;
		}
	else if ((r < 1) && (va < 0)) {
		vb *= 1 + 0.001 * frame_time;
		va *= 1 - 0.001 * frame_time;
		}
	vel.x = va * cos(angle) + vb * sin(angle);
	vel.y = va * sin(angle) - vb * cos(angle);
	return;
	}
