/* $Id$ */ 
//#include <allegro.h>
#include <string.h>

#include "../melee.h"
REGISTER_FILE
#include "../id.h"
#include "../frame.h"
#include "../libs.h"
#include "../util/aastr.h"

#include "mframe.h"
#include "mgame.h"
#include "manim.h"
#include "mview.h"
#include "mship.h"
#include "mcbodies.h"

#include "ais/ext_ai.h"

int total_presences;

#define ANIMATE_BUFFER_SIZE 2048
//setting this too low can cause things to not get drawn
//setting it too high will waste RAM

#define DEATH_FRAMES 4
//setting this too low will cause crashes
//setting it too high will waste CPU power and RAM
//the recommended value is 4

#define QUADS_X 8
#define QUADS_Y 8
//setting these too high waste CPU power & RAM in in small games
//setting these too low can waste CPU power in large games

#define QUADS_TOTAL (QUADS_X * QUADS_Y)
#define QUAD_X (map_size.x / QUADS_X)
#define QUAD_Y (map_size.y / QUADS_Y)
#define QUADI_X (1.0 / QUAD_X)
#define QUADI_Y (1.0 / QUAD_Y)
//don't fuck with these


Physics *physics = NULL;
Game *&game = (Game *&) physics;
NormalGame *&normalgame = (NormalGame *&) physics;
GobGame *&gobgame = (GobGame *&) physics;

int physics_time = 0;
int render_time = 0;
int frame_time = 1;
DATAFILE *melee = NULL;
Vector2 map_size;
double MAX_SPEED = 0;


void Query::begin (SpaceLocation *qtarget, int qlayers, double qrange) {STACKTRACE
	if (qrange < 0) {tw_error("Query::begin - negative range");}
	layers = qlayers;
	range_sqr = qrange * qrange;
	target = qtarget;
	current = NULL;
	target_pos = target->normal_pos();
	if ((qrange < map_size.x/2.5) && (qrange < map_size.y/2.5)) {
		qx_min = (int)floor((target->normal_pos().x - qrange) * QUADI_X);
		qx_max = (int)ceil ((target->normal_pos().x + qrange) * QUADI_X) + 1;
		if (qx_min < 0) qx_min += QUADS_X;
		if (qx_max >= QUADS_X) qx_max -= QUADS_X;
		qy_min = (int)floor((target->normal_pos().y - qrange) * QUADI_Y);
		qy_max = (int)ceil ((target->normal_pos().y + qrange) * QUADI_Y) + 1;
		if (qy_min < 0) qy_min += QUADS_Y;
		if (qy_max >= QUADS_Y) qy_max -= QUADS_Y;
		}
	else {
		qx_min = 0;
		qx_max = 0;
		qy_min = 0;
		qy_max = 0;
		}
	qy = qy_min;
	qx = qx_min;
	current = physics->quadrant[qy * QUADS_X + qx];
	if (!current) next_quadrant();
	if (!current) return;
	if (current_invalid()) next();
	return;
	}

void Query::begin (SpaceLocation *qtarget, Vector2 center, int qlayers, double qrange) {STACKTRACE
	layers = qlayers;
	range_sqr = qrange * qrange;
	target = qtarget;
	current = NULL;
	target_pos = center;
	if ((qrange < map_size.x/2.5) && (qrange < map_size.y/2.5)) {
		qx_min = (int)floor((target_pos.x - qrange) * QUADI_X);
		qx_max = (int)ceil ((target_pos.x + qrange) * QUADI_X) + 1;
		if (qx_min < 0) qx_min += QUADS_X;
		if (qx_max >= QUADS_X) qx_max -= QUADS_X;
		qy_min = (int)floor((target_pos.y - qrange) * QUADI_Y);
		qy_max = (int)ceil ((target_pos.y + qrange) * QUADI_Y) + 1;
		if (qy_min < 0) qy_min += QUADS_Y;
		if (qy_max >= QUADS_Y) qy_max -= QUADS_Y;
		}
	else {
		qx_min = 0;
		qx_max = 0;
		qy_min = 0;
		qy_max = 0;
		}
	qy = qy_min;
	qx = qx_min;

	int k;
	k = qx + qy * QUADS_Y;
	if (k < 0 || k >= QUADS_X*QUADS_Y)
		tw_error("Quad error");

	current = physics->quadrant[qy * QUADS_X + qx];
	if (!current) next_quadrant();
	if (!current) return;
	if (current_invalid()) next();
	return;
	}


void Query::next_quadrant () {
	STACKTRACE

tail_recurse4:
	qx += 1;
	if (qx == QUADS_X) qx = 0;
	if (qx == qx_max) {
		qy += 1;
		if (qy == QUADS_Y) qy = 0;
		if (qy == qy_max) {
			current = NULL;
			qy -= 1;
			qx -= 1;
			return;
			}
		qx = qx_min;
		}
	int tmp = qy * QUADS_X + qx;
	if (tmp < 0) {tw_error ("tmp was less than 0");}
	if (tmp > QUADS_TOTAL) {tw_error ("tmp was too large");}
	current = physics->quadrant[tmp];
	if (!current) goto tail_recurse4;
	return;
	}

void Query::next () {STACKTRACE
tail_recurse3:
	if (current == current->qnext) {tw_error ("Query::next - current = next");}
	current = current->qnext;
	if (!current) {
		next_quadrant();
		if (!current) return;
		}
	if (current_invalid()) goto tail_recurse3;
	return;
	}

void Query::end() {STACKTRACE
	}









void Query2::begin (SpaceLocation *qtarget, Uint64 attribute_filter, double qrange) {
	STACKTRACE;
	if (qrange < 0) {tw_error("Query::begin - negative range");}
	if (Uint32(attribute_filter) & ~Uint32(attribute_filter >> 32)) {
		tw_error("incorrect Query attributes");
	}
	attributes_mask = attribute_filter >> 32;
	attributes_desired = attribute_filter;
	range_sqr = qrange * qrange;
	target = qtarget;
	current = NULL;
	target_pos = target->normal_pos();
	if ((qrange < map_size.x/2.5) && (qrange < map_size.y/2.5)) {
		qx_min = (int)floor((target->normal_pos().x - qrange) * QUADI_X);
		qx_max = (int)ceil ((target->normal_pos().x + qrange) * QUADI_X) + 1;
		if (qx_min < 0) qx_min += QUADS_X;
		if (qx_max >= QUADS_X) qx_max -= QUADS_X;
		qy_min = (int)floor((target->normal_pos().y - qrange) * QUADI_Y);
		qy_max = (int)ceil ((target->normal_pos().y + qrange) * QUADI_Y) + 1;
		if (qy_min < 0) qy_min += QUADS_Y;
		if (qy_max >= QUADS_Y) qy_max -= QUADS_Y;
		}
	else {
		qx_min = 0;
		qx_max = 0;
		qy_min = 0;
		qy_max = 0;
		}
	qy = qy_min;
	qx = qx_min;
	current = physics->quadrant[qy * QUADS_X + qx];
	if (!current) next_quadrant();
	if (!current) return;
	if (current_invalid()) next();
	return;
	}

void Query2::begin (SpaceLocation *qtarget, Vector2 center, Uint64 attribute_filter, double qrange) {STACKTRACE
	if (Uint32(attribute_filter) & ~Uint32(attribute_filter >> 32)) {
		tw_error("incorrect Query attributes");
	}
	attributes_mask = attribute_filter >> 32;
	attributes_desired = attribute_filter;
	range_sqr = qrange * qrange;
	target = qtarget;
	current = NULL;
	target_pos = center;
	if ((qrange < map_size.x/2.5) && (qrange < map_size.y/2.5)) {
		qx_min = (int)floor((target_pos.x - qrange) * QUADI_X);
		qx_max = (int)ceil ((target_pos.x + qrange) * QUADI_X) + 1;
		if (qx_min < 0) qx_min += QUADS_X;
		if (qx_max >= QUADS_X) qx_max -= QUADS_X;
		qy_min = (int)floor((target_pos.y - qrange) * QUADI_Y);
		qy_max = (int)ceil ((target_pos.y + qrange) * QUADI_Y) + 1;
		if (qy_min < 0) qy_min += QUADS_Y;
		if (qy_max >= QUADS_Y) qy_max -= QUADS_Y;
		}
	else {
		qx_min = 0;
		qx_max = 0;
		qy_min = 0;
		qy_max = 0;
		}
	qy = qy_min;
	qx = qx_min;
	current = physics->quadrant[qy * QUADS_X + qx];
	if (!current) next_quadrant();
	if (!current) return;
	if (current_invalid()) next();
	return;
	}


void Query2::next_quadrant () {
tail_recurse4:
	qx += 1;
	if (qx == QUADS_X) qx = 0;
	if (qx == qx_max) {
		qy += 1;
		if (qy == QUADS_Y) qy = 0;
		if (qy == qy_max) {
			current = NULL;
			qy -= 1;
			qx -= 1;
			return;
			}
		qx = qx_min;
		}
	int tmp = qy * QUADS_X + qx;
	if (tmp < 0) {tw_error ("tmp was less than 0");}
	if (tmp > QUADS_TOTAL) {tw_error ("tmp was too large");}
	current = physics->quadrant[tmp];
	if (!current) goto tail_recurse4;
	return;
	}

void Query2::next () {
	STACKTRACE;
tail_recurse3:
	if (current == current->qnext) {tw_error ("Query::next - current = next");}
	current = current->qnext;
	if (!current) {
		next_quadrant();
		if (!current) return;
		}
	if (current_invalid()) goto tail_recurse3;
	return;
	}

void Query2::end() {STACKTRACE
	}


Presence::Presence() {STACKTRACE
	total_presences += 1;
	attributes = 0;
	state = 1;
	//_serial = 0;
	id = 0;
	_depth = 0;
	set_depth(DEPTH_PRESENCE);
	}

void Presence::animate(Frame *space) {STACKTRACE
	return;
	}
void Presence::animate_predict(Frame *space, int time) {STACKTRACE
	animate(space);
	}
void Presence::calculate() {STACKTRACE
	}
bool Presence::die() {STACKTRACE
	if (!exists())
		return true;	//tw_error("Presence::die - already dead"); [note: this is not an error]
	state = 0;
	return true;
	}
Presence::~Presence() {STACKTRACE
	total_presences -= 1;
	}

bool Presence::isLocation() const {
	return ((attributes & ATTRIB_LOCATION) != 0);
	}

bool Presence::isLine() const {
	return ((attributes & ATTRIB_LINE) != 0);
	}

bool Presence::isObject() const {
	return ((attributes & ATTRIB_OBJECT) != 0);
	}

bool Presence::isPlanet() const {
	return (id == ID_PLANET);
	}

bool Presence::isAsteroid() const {
	return (id == ID_ASTEROID);
	}

bool Presence::isShip() const {
	return ((attributes & ATTRIB_SHIP) != 0);
	}

bool Presence::isShot() const {
	return ((attributes & ATTRIB_SHOT) != 0);
	}
bool Presence::isSynched() const {
	return ((attributes & ATTRIB_SYNCHED) != 0);
	}

SpaceLocation *Presence::get_focus() {STACKTRACE
	return NULL;
	}
SpaceLocation *SpaceLocation::get_focus() {STACKTRACE
	return this;
	}

SpaceLocation::SpaceLocation(SpaceLocation *creator, Vector2 lpos, double langle) :
	pos(lpos),
	qnext(NULL),
	layer(LAYER_LOCATIONS),
	vel(0,0),
	angle(langle),
	damage_factor(0),
	collide_flag_anyone(ALL_LAYERS),
	collide_flag_sameteam(0),
	collide_flag_sameship(0)

{STACKTRACE
	id |= SPACE_LOCATION;
	attributes |= ATTRIB_SYNCHED;
	attributes |= ATTRIB_LOCATION;
	if (creator) {
		ally_flag = creator->ally_flag;

		ship = creator->ship;
		parent = creator;

		data = creator->data;

		if (data) data->lock();
		target = creator->target;
		}
	else {
		ally_flag = 0;
		ship = NULL;
		parent = NULL;
		data = NULL;
		target = NULL;
		}
}


SpaceLocation::~SpaceLocation() {STACKTRACE
	if (data) data->unlock();

	}

bool SpaceLocation::change_owner(SpaceLocation *new_owner) {STACKTRACE
	if (new_owner) {
		ally_flag = new_owner->ally_flag;
		ship = new_owner->ship;
		target = new_owner->target;
		// but, parent stays the same.
		}
	else {
		ally_flag = 0;
		ship = NULL;
		target = NULL;
		}
	return true;
	}

void SpaceLocation::death() {STACKTRACE
}

double SpaceLocation::get_angle_ex() const
{
	return get_angle();
}
double SpaceLocation::get_angle() const
{
	return(normalize(angle, PI2));
}

int SpaceLocation::getID() const
{
  return id;
}

Vector2 SpaceLocation::normal_pos() const
{STACKTRACE
  return(normalize(pos, map_size));
}

Vector2 SpaceLocation::nearest_pos(SpaceLocation *l) const 
{STACKTRACE
	Vector2 p1, p2;
	p1 = normal_pos();
	p2 = l->normal_pos();
  return Vector2(
	  nearest_coord(p1.x, p2.x, map_size.x),
	  nearest_coord(p1.y, p2.y, map_size.y)
	  );
}

double SpaceLocation::distance(SpaceLocation *l)
{STACKTRACE
  return(distance_from(normal_pos(), l->normal_pos()));
}

int SpaceLocation::handle_damage (SpaceLocation *source, double normal, double direct) {STACKTRACE
	return 0;
}

int SpaceLocation::handle_fuel_sap (SpaceLocation *source, double normal) {STACKTRACE
	return 0;
}

double SpaceLocation::handle_speed_loss (SpaceLocation *source, double normal) {STACKTRACE
	return 0;
}

void SpaceLocation::change_vel(Vector2 dvel)
{
	vel += dvel;
}

void SpaceLocation::scale_vel(double scale)
{
	vel *= scale;
}

/*** Change a location by translation
*/
void SpaceLocation::change_pos(Vector2 dpos)
{
	pos = normalize(pos + dpos);
}

/*** Change a location by scaling
*/
void SpaceLocation::change_pos(double scale)
{
	pos *= scale;
}

void SpaceLocation::ship_died() {STACKTRACE
	ship = NULL;
}
void SpaceLocation::target_died() {
	target = NULL;
}

double SpaceLocation::trajectory_angle(SpaceLocation *l) {STACKTRACE
	return ::trajectory_angle(pos, l->normal_pos());
}


bool inline SpaceLocation::detectable()
{
	int i;
	i = attributes & ATTRIB_UNDETECTABLE;

	// it's detectable if the undetectable flag isn't set
	return (i == 0);
};



int SpaceLocation::canCollide(SpaceLocation *other) {
	if (!detectable()) return 0;
	if (sameShip(other)) return ((1 << other->layer) & collide_flag_sameship);
	else if (sameTeam(other)) return ((1 << other->layer) & collide_flag_sameteam);
	return ((1 << other->layer) & collide_flag_anyone);
}

TeamCode SpaceLocation::get_team() const
{
	return (ally_flag & team_mask) >> team_shift;
}

void SpaceLocation::set_team(TeamCode k)
{
	ally_flag &= ~team_mask;
	ally_flag |= k << team_shift;
}

bool SpaceLocation::sameTeam(const SpaceLocation *other) const {
	return !((ally_flag ^ other->ally_flag) & (team_mask));
}

double SpaceLocation::isProtected() const {
	return 0;
}

double SpaceLocation::isInvisible() const {
	return 0;
}

void Presence::set_depth(double d) {
	_depth = int(floor(ldexp(d, 8)));
}

double Presence::get_depth() {
	return ldexp((double)_depth, -8);
}

Planet *SpaceLocation::nearest_planet() {STACKTRACE
	Planet *p = NULL;
	double r = 99999999;
	Query q;
	q.begin(this, bit(LAYER_CBODIES), 1600);
	while (q.current) {
		if (q.current->isPlanet()) {
			double t = distance(q.current);
			if (t < r) {
				r = t;
				p = (Planet *) q.current;
			}
		}
		q.next();
	}
	return p;
}
void SpaceLocation::play_sound (SAMPLE *sample, int vol, int freq) {STACKTRACE
	physics->play_sound(sample, this, vol, freq);
	return;
}
void SpaceLocation::play_sound2 (SAMPLE *sample, int vol, int freq) {STACKTRACE
	physics->play_sound2(sample, this, vol, freq);
	return;
}
int SpaceLocation::translate( Vector2 delta) {STACKTRACE
	pos = normalize ( pos + delta, map_size );
	return true;
}

int SpaceLocation::accelerate(SpaceLocation *source, double angle, double velocity, double max_speed) {STACKTRACE
	_accelerate(angle, velocity, max_speed);
	return true;
}
int SpaceLocation::accelerate(SpaceLocation *source, Vector2 delta_v, double max_speed) {STACKTRACE
	_accelerate(delta_v, max_speed);
	return true;
}
void SpaceLocation::_accelerate(double angle, double velocity, double max_speed) {STACKTRACE
	double ovm, nvm;
	Vector2 nv;

	ovm = magnitude_sqr(vel);
	nv = vel + unit_vector(angle) * velocity;
	nvm = magnitude_sqr(nv);
	if ((nvm <= max_speed * max_speed) || (nvm <= ovm)) {
		vel = nv;
	}
	else {
		if (ovm <= max_speed * max_speed) ovm = max_speed;
		else ovm = sqrt(ovm);
		vel = nv * ovm / (ovm + velocity);
	}
	return;
}
void SpaceLocation::_accelerate(Vector2 delta_v, double max_speed) {STACKTRACE
	double ovm, nvm;
	Vector2 nv;

	ovm = magnitude_sqr(vel);
	nv = vel + delta_v;
	nvm = magnitude_sqr(nv);
	if ((nvm <= max_speed * max_speed) || (nvm <= ovm)) {
		//if new velocity is slow, handle normally
		//if new velocity is fast, but we're decelerating, still handle normally
		vel = nv;
	}
	else {
		if (ovm <= max_speed * max_speed) ovm = max_speed;
		else ovm = sqrt(ovm);
		//otherwise, slow down closer to the maximum speed
		//but only when turning, particularly turning fast
		vel = nv * ovm / (ovm + magnitude(delta_v));
	}
	return;
}

int SpaceLocation::accelerate_gravwhip(SpaceLocation *source, double angle, double velocity, double max_speed) {STACKTRACE
	Planet *p = nearest_planet();
	if (!p) return SpaceLocation::accelerate(source, angle, velocity, max_speed);
	double tmp;
	tmp = distance(p) / p->gravity_range;
	if (tmp > 1) return SpaceLocation::accelerate(source, angle, velocity, max_speed);
	return SpaceLocation::accelerate(source, angle, velocity, max_speed * (p->gravity_whip * tmp + 1) + tmp * p->gravity_whip2);
}

void SpaceLocation::animate(Frame *space) {STACKTRACE
	return;
}

void SpaceLocation::animate_predict(Frame *space, int time) {STACKTRACE
	Vector2 opos = pos;
	pos += vel * time;
	animate(space);
	pos = opos;
	return;
}

void SpaceLocation::calculate() {STACKTRACE
	if (target && !target->exists()) {
		target_died();
	}
	if (ship && !ship->exists()) {
		ship_died();
	}
	if (parent && !parent->exists()) {
		parent = 0;
	}
	return;
}

void SpaceObject::set_sprite(SpaceSprite *new_sprite) {STACKTRACE
	sprite = new_sprite;
	size = new_sprite->size();
}

void SpaceObject::calculate()
{
	STACKTRACE;

	SpaceLocation::calculate();

	if (sprite)
	{
		if (attributes & ATTRIB_STANDARD_INDEX)
		{
			sprite_index = get_index(angle, PI/2, sprite->frames());
		}

	// error check:
		if (sprite_index >= sprite->frames() || sprite_index < 0)
		{
			tw_error("sprite index overflow in %s", get_identity());
		}
	}

	return;
}

void SpaceObject::calculate_index()
{
	if (sprite->frames() > 64)
		sprite_index = get_index(angle);
	else
		sprite_index = get_index(angle, PI/2, sprite->frames());
}

SpaceObject::SpaceObject(SpaceLocation *creator, Vector2 opos, 
	double oangle, SpaceSprite *osprite) 
	:
	SpaceLocation(creator, opos, oangle),
	size(osprite->width(), osprite->height()),
	mass(0),
	sprite(osprite),
	sprite_index(0)
	{STACKTRACE
	attributes |= ATTRIB_OBJECT;
	if (game && game->friendly_fire) collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = 0;
	collide_flag_anyone = ALL_LAYERS;
	id = SPACE_OBJECT;

	isblockingweapons = true;
//	ext_ai = NULL;
	}
SpaceObject::~SpaceObject()
{
//	destroy_external_ai();
}

void SpaceObject::animate(Frame *space) {STACKTRACE
	sprite->animate(pos, sprite_index, space);
	return;
	}

void SpaceObject::collide(SpaceObject *other) {STACKTRACE
	double tmp;

	if (this == other) {tw_error("SpaceObject::collide - self!");}
	if((!canCollide(other)) || (!other->canCollide(this))) return;
	if (!exists() || !other->exists()) return;

	pos = normal_pos();
	Vector2 p1, p2, dp, dv;

	p1 = pos;
	p2 = other->normal_pos();
	dp.x = min_delta(p1.x, p2.x, map_size.x);
	dp.y = min_delta(p1.y, p2.y, map_size.y);
	p2 = p1 - dp - other->size / 2;
	p1 = p1 - size / 2;

	if (!sprite->collide((int)p1.x, (int)p1.y, sprite_index, (int)p2.x, (int)p2.y, 
			other->sprite_index, other->sprite)) 
		return;
	//sprite->collide(x1, y1, sprite_index, x2, y2, other->sprite_index, other->sprite);

	inflict_damage(other);
	other->inflict_damage(this);

	if (!mass || !other->mass) return;
	
	dv = vel - other->vel;

	p1 = pos;
	p2 = other->normal_pos();
	dp.x = min_delta(p1.x, p2.x, map_size.x);
	dp.y = min_delta(p1.y, p2.y, map_size.y);
	p2 = p1 - dp - other->size / 2;
	p1 = p1 - size / 2;

	while ((dp.x == 0) && (dp.y == 0)) {
		dp.x = (tw_random(5) - 2) / 99.0;
		dp.y = (tw_random(5) - 2) / 99.0;
	}


	Vector2 _dp = unit_vector(dp);
	tmp = dot_product(dv, _dp);
	tmp = ( -2 * tmp );

	double a = mass + other->mass;
	if (a > 0)
		tmp = tmp * (mass * other->mass) / (a);

	if (tmp >= 0) {
		//vel += _dp * tmp / mass;
		//other->vel -= _dp * tmp / other->mass;
		if (mass > 0)
			change_vel(_dp * tmp / mass);

		if (other->mass > 0)
			other->change_vel( -_dp * tmp / other->mass);
	}
	
	Vector2 nd;
	nd = unit_vector(dp);
	nd /= (mass + other->mass);
	while (sprite->collide((int)p1.x, (int)p1.y, sprite_index, (int)p2.x, (int)p2.y, 
			other->sprite_index, other->sprite)) {
		//pos = normalize(pos + nd * other->mass);
		//other->pos = normalize(other->pos - nd * mass);
		change_pos(nd * other->mass);
		other->change_pos(-nd * mass);

		p1 = pos;
		p2 = other->normal_pos();
		dp.x = min_delta(p1.x, p2.x, map_size.x);
		dp.y = min_delta(p1.y, p2.y, map_size.y);
		p2 = p1 - dp - other->size / 2;
		p1 = p1 - size / 2;
	}

	return;
}

double SpaceObject::collide_ray(Vector2 lp1, Vector2 lp2, double llength)
{STACKTRACE
	int collide_x = (int)(lp2.x);
	int collide_y = (int)(lp2.y);
	Vector2 d;

	if (sprite->collide_ray(
			(int)(lp1.x), (int)(lp1.y), &collide_x, &collide_y,
			(int)(lp1.x - min_delta(lp1.x, pos.x, map_size.x)),
			(int)(lp1.y - min_delta(lp1.y, pos.y, map_size.y)), 
			sprite_index)) {
		d = lp2 - Vector2(collide_x, collide_y);
		llength = llength - magnitude(d);
	}

	return(llength);
}

void SpaceObject::inflict_damage(SpaceObject *other) {STACKTRACE
	int i;
	if (damage_factor > 0) {
		i = iround_down(damage_factor / 2);
		if(i >= BOOM_SAMPLES) i = BOOM_SAMPLES - 1;
		play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
		damage(other, damage_factor);
		}
	else damage(other, 0);
	return;
	}

void SpaceObject::death() {STACKTRACE
	if (attributes & ATTRIB_NOTIFY_ON_DEATH) {
		physics->object_died(this, NULL);
		attributes &= ~ ATTRIB_NOTIFY_ON_DEATH;
	}
}

/*
void SpaceObject::install_external_ai(const char* script)
{
	if (ext_ai != NULL)
	{
		delete ext_ai;
	}
	if (script == NULL) 
		return;
	ext_ai = new ExternalAI(this, script );
}

void SpaceObject::destroy_external_ai()
{
	if (ext_ai != NULL)
		delete ext_ai;
	ext_ai = NULL;
}
*/

SpaceLine::SpaceLine(SpaceLocation *creator, Vector2 lpos, double langle, 
	double llength, int lcolor) 
	:
	SpaceLocation(creator, lpos, langle),
	length(llength),
	color(lcolor)
	{STACKTRACE
	id = SPACE_LINE;
	attributes |= ATTRIB_LINE;// | ATTRIB_COLLIDE_STATIC;
	layer = LAYER_LINES;
	set_depth(DEPTH_LINES);
	collide_flag_anyone   &= OBJECT_LAYERS;
	collide_flag_sameteam &= OBJECT_LAYERS;
	collide_flag_sameship &= OBJECT_LAYERS;
	}

double SpaceLine::edge_x() const
{
  return(cos(angle) * length);
}

double SpaceLine::edge_y() const
{
  return(sin(angle) * length);
}

Vector2 SpaceLine::edge() const
{
  return unit_vector(angle) * length;
}

double SpaceLine::get_length() const
{
  return(length);
}

void SpaceLine::set_length(double d)
{
	length = d;
}

void SpaceLine::inflict_damage(SpaceObject *other) {STACKTRACE
	int i;
	i = iround_down(damage_factor / 2);
	if(i >= BOOM_SAMPLES)
		i = BOOM_SAMPLES - 1;
	play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
	damage(other, damage_factor);
	collide_flag_anyone = collide_flag_sameship = collide_flag_sameteam = 0;
	physics->add(new Animation( this, 
			pos + edge(), meleedata.sparkSprite, 0, 
			SPARK_FRAMES, 50, DEPTH_EXPLOSIONS));
	return;
	}

void SpaceLine::animate(Frame *space) {STACKTRACE

	Vector2 p1 = corner( pos );
	Vector2 p2 = p1 + edge() * space_zoom;

	line(space, p1, p2, color);
	}

double SpaceLine::collide_testdistance(SpaceObject *o)
{
	STACKTRACE;
	//double testlength;
  
	if((!canCollide(o)) || (!o->canCollide(this)))
		return length;

	return o->collide_ray(normal_pos(), normal_pos() + edge(), length);

	//if(length != old_length)
	//	inflict_damage(o);
	//return;
}

void SpaceLine::collide(SpaceObject *o)
{
	STACKTRACE;
	double old_length = length;
	/*
  
	if((!canCollide(o)) || (!o->canCollide(this)))
		return;

	length = o->collide_ray(normal_pos(), normal_pos() + edge(), length);
	*/
	length = collide_testdistance(o);
	if (length != old_length)
		inflict_damage(o);
	return;
}

void Physics::destroy_all() {
	STACKTRACE
	int i;
	for (i = 0; i < num_presences; i += 1) {
		Presence *tmp = presence[i];
		presence[i] = NULL;
		delete tmp;
	}
	if (presence) free(presence);
	presence = NULL;
	num_presences = 0;
	max_presences = 0;
	for (i = 0; i < num_items; i += 1) {
		SpaceLocation *tmp = item[i];
		item[i] = NULL;
		delete tmp;
	}
	if (item) free(item);
	item = NULL;
	num_items = 0;
	max_items = 0;
}

Physics::~Physics() {STACKTRACE
	destroy_all();
	if (quadrant) delete[] quadrant;
	quadrant = NULL;
}

void Physics::preinit() {STACKTRACE
	quadrant = NULL;
	num_items = max_items = 0;
	item = NULL;
	num_presences = max_presences = 0;
	presence = NULL;
	last_ship = 0;
	last_team = 0;
	return;
	}

unsigned int Physics::get_code(unsigned int ship, TeamCode team) {STACKTRACE
	return (ship << SpaceLocation::ship_shift) | (team << SpaceLocation::team_shift);
	}

unsigned int Physics::new_ship() {
	last_ship += 1;
	return last_ship;
	}
TeamCode Physics::new_team() {
	last_team += 1;
	return last_team;
	}
void Physics::switch_team(unsigned int ship, TeamCode team) {STACKTRACE
	int i, j;
	j = (ship & SpaceLocation::ship_mask) | (team << SpaceLocation::team_shift);
	for (i = 0; i < num_items; i += 1) {
		if ((item[i]->ally_flag & SpaceLocation::ship_mask) == (ship << SpaceLocation::ship_shift)) item[i]->ally_flag = j;
		}
	return;
	}
void Physics::merge_teams(TeamCode team1, TeamCode team2) {STACKTRACE
	int i;
	for (i = 0; i < num_items; i += 1) {
		if ((item[i]->ally_flag & SpaceLocation::team_mask) == (team2 << SpaceLocation::team_shift)) 
			item[i]->ally_flag = (item[i]->ally_flag & ~ SpaceLocation::team_mask) | (team1 << SpaceLocation::team_shift);
		}
	return;
	}

void Physics::init() {STACKTRACE
	int i;
	size = Vector2(3840.0, 3840.0);
	frame_time = 25;
	turbo = 1.0;
	max_speed = 1.0;
	quadrant = new SpaceLocation*[QUADS_TOTAL];
	for(i = 0; i < QUADS_TOTAL; i += 1) {
		quadrant[i] = NULL;
		}
	max_items += 1024; 
	item = (SpaceLocation**) realloc(item, sizeof(SpaceLocation*) * max_items);
	max_presences += 64; 
	presence = (Presence**) realloc(presence, sizeof(Presence*) * max_presences);
	return;
	}

void Physics::add(SpaceLocation *o) {
	STACKTRACE;
	if (o->attributes & ATTRIB_INGAME) {tw_error("addItem - already added");}
	if (!o->isLocation()) {tw_error("addItem - catastrophic");}
	//if (!o->_serial) _list(o);

	o->attributes |= ATTRIB_INGAME;

	if (num_items == max_items) {
		max_items += 1024;
		item = (SpaceLocation**) realloc(item, sizeof(SpaceLocation*) * max_items);
		}
	item[num_items] = o;
	num_items += 1;

	if (o->detectable())
	{
		Vector2 n = o->normal_pos();
		int q = int(n.x * QUADI_X) + 
			int(n.y * QUADI_Y) * QUADS_X;
		if ((q < 0) || (q > QUADS_TOTAL)) {tw_error("bad quadrant");}
		o->qnext = quadrant[q];
		quadrant[q] = o;
	}

	return;
	}

bool Physics::remove(SpaceLocation *o) {
	STACKTRACE;
	int i;
	if (!(o->attributes & ATTRIB_INGAME)) {tw_error("removeItem - not added");}
	o->attributes &= ~ATTRIB_INGAME;
	if (!o->isLocation()) {tw_error("removeItem - catastrophic");}
	for (i = 0; i < num_items; i += 1) {
		if (item[i] == o) {
			num_items -= 1;
			//item[i] = item[num_items];
			memmove(&item[i], &item[i+1], (num_items-i) * sizeof(SpaceLocation*));
			//o->qnext = NULL;
			return true;
			}
		}
	return false;
	}

void Physics::add(Presence *p) {
	STACKTRACE;
	if (p->attributes & ATTRIB_INGAME) {tw_error("addPresence - already added");}
	if (p->isLocation()) {
		add((SpaceLocation*)p);
		return;
		}
	//if (!p->_serial) _list(p);
	if (num_presences == max_presences) {
		max_presences += 256; 
		presence = (Presence**) realloc(presence, sizeof(Presence*) * max_presences);
		}
	presence[num_presences] = p;
	num_presences += 1;
	return;
	}

bool Physics::remove(Presence *o) {
	STACKTRACE;
	int i;
	if (!(o->attributes & ATTRIB_INGAME)) {tw_error("removePresence - not added");}
	if (o->isLocation()) return remove((SpaceLocation*)o);
	o->attributes &= ~ATTRIB_INGAME;
	for (i = 0; i < num_presences; i += 1) {
		if (presence[i] == o) {
			num_presences -= 1;
			memmove(&presence[i], &presence[i+1], (num_presences-i) * sizeof(Presence*));
			return true;
			}
		}
	return false;
	}

void Physics::calculate() {_STACKTRACE("Physics::calculate()")
	int i;

	//adjust time
	frame_number += 1;
	game_time += frame_time;

	//prepare global variables
	prepare();

	debug_value = num_items + num_presences;


checksync();
{_STACKTRACE("Physics::calculate() - item movement")
	//move objects
	for (i = 0; i < num_items; i += 1) {
		if (!item[i]->exists()) continue;
		//if (i == 1 && game_time == 100) tw_error("debug me!");
		item[i]->pos = normalize(item[i]->pos + item[i]->vel * frame_time, map_size);
		}
}
checksync();


{_STACKTRACE("Physics::calculate() - presence calculation")
	//call Presence calculate functions
	for (i = 0; i < num_presences; i += 1) {
		if (presence[i]->exists()) presence[i]->calculate();
checksync();
		}
}

	//call objects calculate functions
{_STACKTRACE("Physics::calculate() - item calculation")
	for (i = 0; i < num_items; i += 1) {
		if (item[i]->exists()) item[i]->calculate();
checksync();
		}
}

	//prepare quadrants stuff
{_STACKTRACE("Physics::calculate() - quadrants stuff")
	for(i = 0; i < QUADS_TOTAL; i += 1) {
		quadrant[i] = NULL;
		}
	for(i = 0; i < num_items; i += 1) {
		if (item[i]->exists() && item[i]->detectable()) {
			Vector2 n = item[i]->normal_pos();
			int q = iround_down(n.x * QUADI_X) + 
					iround_down(n.y * QUADI_Y) * QUADS_X;
			if ((q < 0) || (q > QUADS_TOTAL)) {tw_error("bad quadrant");}
			item[i]->qnext = quadrant[q];
			quadrant[q] = item[i];
			}
		else item[i]->qnext = NULL;
		}
}

checksync();
	//check for collisions
{_STACKTRACE("Physics::calculate() - collisions")
	collide();
}

checksync();

{_STACKTRACE("Physics::calculate() - presence destruction")
	//remove presences that have been dead long enough
	int deleted = 0;
	for(i = 0; i < num_presences; i ++) {
		presence[i] = presence[i+deleted];
		if (presence[i]->state == -DEATH_FRAMES) {
			Presence *tmp = presence[i];
			num_presences -= 1;
			memmove(&presence[i], &presence[i+1], sizeof(Presence*) * (num_presences-i));
			//presence[i] = presence[num_presences-1];
			i -= 1;
			//deleted += 1;
			delete tmp;
		}
		else {
			if (!presence[i]->exists()) {
				//if (presence[i]->state == 0) presence[i]->death();
				presence[i]->state -= 1;
			}
		}
	}
}

checksync();

	//remove objects that have been dead long enough
{_STACKTRACE("Physics::calculate() - item destruction")
	int deleted = 0;
	for(i = 0; i < num_items; i ++) {
		item[i] = item[i+deleted];
		if (item[i]->state == -DEATH_FRAMES) {
			SpaceLocation *tmp = item[i];
			num_items -= 1;
			//item[i] = item[num_items];
			memmove(&item[i], &item[i+1], (num_items-i) * sizeof(SpaceLocation*));
			//deleted += 1;
			i -= 1;
			delete tmp;
		}
	}

	for(i = 0; i < num_items; i ++)
	{
		// checking if this can happen? (curious ; GEO)
		if (item[i]->state < -DEATH_FRAMES)
		{tw_error("too many death-frames - should not happen !!");}
	}

	for(i = 0; i < num_items; i ++) {
		if (!item[i]->exists()) {
			if (item[i]->state == 0) {
				item[i]->death();
			}
			item[i]->state -= 1;
		}
	}
}
checksync();

	//remove dead listings
/*{STACKTRACE
	int deleted = 0;
	for (i = 0; i + deleted < num_listed; i += 1) {
		if (listed[i].serial == 0) deleted += 1;
		if (deleted) listed[i] = listed[i+deleted];
	}
	num_listed -= deleted;

}*/

checksync();

	return;
	}

int compare_depth (const void *_a, const void *_b) {
	int a = (*(const Presence**)_a)->_depth;
	int b = (*(const Presence**)_b)->_depth;
	if (a-b) return a-b;
        else return 0; // TODO test this line
	//else return (*(const SpaceLocation**)_a)->get_serial() - (*(const SpaceLocation**)_b)->get_serial();
}
static Presence *animate_buffer[ANIMATE_BUFFER_SIZE];

void Physics::animate (Frame *frame) {STACKTRACE
	int i, j;

	::render_time = this->game_time;

	//add presences to list
	j = num_presences;
	if (j > ANIMATE_BUFFER_SIZE / 2) j = ANIMATE_BUFFER_SIZE / 2;
	memcpy(&animate_buffer[0], presence, sizeof(Presence *) * j);

	//add items to list
	i = num_items;
	if (i > ANIMATE_BUFFER_SIZE - j) i = ANIMATE_BUFFER_SIZE - j;
	memcpy(&animate_buffer[j], item, sizeof(Presence *) * i);

	j += i;
	qsort(animate_buffer, j, sizeof(SpaceLocation*), compare_depth);
	prepare();
	RGB back = { frame->background_red, frame->background_green, frame->background_blue };
	aa_set_background ( back );
	for (i = 0; i < j; i += 1) {
		if (animate_buffer[i]->exists()) animate_buffer[i]->animate(frame);
	}
	return;
}

void Physics::animate_predict(Frame *frame, int time) {STACKTRACE
	int i, j;

	if (time == 0) {
		animate(frame);
		return;
	}

	::render_time = this->game_time + time;

	//add presences to list
	j = num_presences;
	if (j > ANIMATE_BUFFER_SIZE / 2) j = ANIMATE_BUFFER_SIZE / 2;
	memcpy(&animate_buffer[0], presence, sizeof(Presence *) * j);

	//add items to list
	i = num_items;
	if (i > ANIMATE_BUFFER_SIZE - j) i = ANIMATE_BUFFER_SIZE - j;
	memcpy(&animate_buffer[j], item, sizeof(Presence *) * i);

	j += i;
	qsort(animate_buffer, j, sizeof(SpaceLocation*), compare_depth);
	prepare();
	RGB back = { frame->background_red, frame->background_green, frame->background_blue };
	aa_set_background ( back );
	for (i = 0; i < j; i += 1) {
		if (animate_buffer[i]->exists()) animate_buffer[i]->animate_predict(frame, time);
	}
	return;
}

/** \brief Check all objects within range (length) of a line, and damage only the
one that has a point of impact that is closest to the line origin (ie reduces line
length most).
*/
void Physics::check_linecollision(SpaceLine *l)
{
	// not very efficient, ideally you'd just check the quadrants that the line traverses,
	// not all of them in a circle, but it'll do fine for short lines.

	double distance = l->get_length();
	SpaceObject *o = 0;

	Query q;
	for (q.begin(l, normalize(l->pos+l->edge()/2, map_size), OBJECT_LAYERS, 96 + l->get_length()/2); q.current && l->exists(); q.next())
	{
		double d;
		d = l->collide_testdistance(q.currento);
		// this is the distance to the point of impact

		// for long lines (eg chmmr laser), and several objects closeby, there can be >1 points of impact.
		// the point that's closest by makes most sense.s
		if (d < distance)
		{
			o = q.currento;
			distance = d;
		}
	}
	q.end();

	// note that o==null, in case that distance==length, meaning that there are no
	// colliding objects within range.
	if (o)
	{
		l->set_length(distance);
		l->inflict_damage(o);
	}

}

#include "../util/pmask.h"
void Physics::collide() {_STACKTRACE("Physics::collide()")
	int i;
	PMASKDATA_FLOAT *tmp;
	int l = 0;
	tmp = new PMASKDATA_FLOAT[num_items];
	for (i = 0; i < num_items; i += 1) {
		if (item[i] && item[i]->exists() && item[i]->isObject()
				&& item[i]->detectable() ) {
			SpaceObject *o = (SpaceObject *) item[i];
			Vector2 p = o->pos - o->size / 2;
			tmp[l].x = p.x;
			tmp[l].y = p.y;
			if (tmp[l].y < 0) tmp[l].y += size.y;
			tmp[l].pmask = o->get_sprite()->get_pmask(o->get_sprite_index());
			tmp[l].data = o;
			l += 1;
		}
	}
	SpaceObject *col[128 * 2 + 1];
	int nc = check_pmask_collision_list_float_wrap(size.x, size.y, tmp, l, (const void**)&col[0], 128);
	delete tmp;
//	return;
	for (i = 0; i < nc; i += 1) {
		col[i*2]->collide(col[i*2+1]);
	}//*/
	Query q;
	for (i = 0; i < num_items; i += 1) {
		if (item[i]->exists() && item[i]->detectable() &&
			(item[i]->collide_flag_sameship | item[i]->collide_flag_sameteam | item[i]->collide_flag_anyone) && 
			!(item[i]->attributes & ATTRIB_COLLIDE_STATIC)) {
			if (item[i]->isObject()) {
/*
				for (q.begin(item[i], OBJECT_LAYERS, (192+
						((SpaceObject*)item[i])->get_size().x+
						((SpaceObject*)item[i])->get_size().y)/2); q.current && item[i]->exists(); q.next()) {
					
					if ((q.currento->get_serial() < item[i]->get_serial()) ||
						(q.currento->attributes & ATTRIB_COLLIDE_STATIC)) {
						((SpaceObject*)item[i])->collide(q.currento);
					}
				}
				q.end();//*/
			}
			else if (item[i]->isLine())
			{
				check_linecollision((SpaceLine*)item[i]);
				/*
				SpaceLine *l = (SpaceLine*)item[i];
				for (q.begin(l, l->normal_pos()+l->edge()/2, OBJECT_LAYERS, 96 + l->get_length()/2); q.current && l->exists(); q.next())
				{
					((SpaceLine*)item[i])->collide(q.currento);
				}
				q.end();
				*/
			}
		}
	}
//*/
	return;
}
void Physics::prepare() {STACKTRACE
	::physics_time = this->game_time;
	::render_time = this->game_time;
	::frame_time = this->frame_time;
	::map_size   = this->size;
	::MAX_SPEED  = this->max_speed;
	return;
}
int Physics::checksum() {_STACKTRACE("Physics::checksum")
	int i;
	Uint32 g = 0;
	//prepare();
	for (i = 0; i < num_items; i += 1)
	{
		//if (!item[i]->detectable()) continue;
		if (!(item[i]->attributes & ATTRIB_SYNCHED)) continue;

		Vector2 n = item[i]->normal_pos();
		g += iround(n.x * sqrt(47.0+i+g));
		g += iround(n.y * sqrt(71.0+i+g));
		//g += g * item[i]->get_vy() * sqrt(73+i+g/32768.0);
		//g -= g * item[i]->get_vx() * sqrt(1+i+g/65536.0);
	}
//	return (((unsigned int)(fmod(floor(g * 1024), 0xFFffFFffUL))) & 255);
	return g;// + (tw_random_state_checksum()) & 255));
}
void Physics::dump_state ( const char *file_name ) {STACKTRACE
	//unimplemented
}
void Physics::play_sound (SAMPLE *sample, SpaceLocation *source, int vol, int freq) {STACKTRACE
	sound.play(sample, vol, 128, iround(freq * turbo));
	return;
}
void Physics::play_sound2 (SAMPLE *sample, SpaceLocation *source, int vol, int freq) {STACKTRACE
	sound.stop(sample);
	play_sound(sample, source, vol, freq);
	return;
}


void Physics::log_file (const char *fname)
{
	set_config_file(fname);
};


void Physics::ship_died(Ship *who, SpaceLocation *source)
{
	// nothing ...
}


void Physics::object_died(SpaceObject *who, SpaceLocation *source)
{
	if (who && who->isShip())
	{
		ship_died((Ship*)who, source);
	}
}
