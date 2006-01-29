/* $Id$ */ 
#include <assert.h>
#include "../ship.h"
#include "../melee/mview.h"
#include "../frame.h"

REGISTER_FILE

/* Copy of the Tau Bomber
*/

class AlaryBomber : public Ship
{
public:
IDENTITY(AlaryBomber);
	int			bombLifetime;
	double		bombDamageMin, bombDamageMax, bombArmour;
	double		bombProximity, bombBlastRange, bombKick;

	double		special_speedmax, special_accelrate;
		
	bool		can_launch_bomb;

public:
	AlaryBomber	(Vector2 opos, double shipAngle,
				ShipData *shipData, unsigned int code);

	virtual int  activate_weapon();
	virtual int  activate_special();
	virtual void calculate();
	virtual void calculate_hotspots();

};


class AlaryBomberBomb : public Missile
{
public:
IDENTITY(AlaryBomberBomb);
	double		blast_range, proximity_range, old_range, kick;
	//int		rotation_index;
	double		blast_damage_max, blast_damage_min, lifetime;
	//double		rotation_angle;
	SpaceObject *tgt;
	bool		active;

public:
	AlaryBomberBomb (SpaceLocation *creator, double ox, double oy, double oangle, double odamage_min, double odamage_max,
				double oarmour, SpaceSprite *osprite, double oblast_range, double oproximity,
				int olifetime, double okick);
	virtual void calculate();
	virtual void animate(Frame *space);
	virtual void animateExplosion();
};


class AlaryBomberBombExplosion : public Presence
{
public:
IDENTITY(AlaryBomberBombExplosion);
	Vector2 *xp, *xv;
	int		num, lifetime, life_counter, color;
public:
	AlaryBomberBombExplosion(Vector2 opos, double ov, int onum, int olife, int ocolor);
	virtual void calculate();
	virtual void animate(Frame *space);
	virtual ~AlaryBomberBombExplosion();
};




AlaryBomber::AlaryBomber(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
	Ship(opos, shipAngle, shipData, code)
{
	bombLifetime	= int(get_config_float("Bomb", "Lifetime", 0) * 1000);
	bombDamageMin	= get_config_float("Bomb", "DamageMin", 1);
	bombDamageMax	= get_config_float("Bomb", "DamageMax", 3);
	bombArmour		= get_config_float("Bomb", "Armour", 0);
	bombProximity	= scale_range(get_config_float("Bomb", "Proximity", 0));
	bombBlastRange	= scale_range(get_config_float("Bomb", "BlastRange", 0));
	bombKick		= scale_velocity(get_config_float("Bomb", "Kick", 0));

	special_speedmax = scale_velocity(get_config_float("Afterburn", "SpeedMax", 10.0));
	special_accelrate = scale_acceleration(get_config_float("Afterburn", "AccelRate", 10.0));

	can_launch_bomb = true;

}

int AlaryBomber::activate_weapon()
{
	STACKTRACE
	if (!can_launch_bomb) return false;
	add(new AlaryBomberBomb(this, 0, 0, angle, bombDamageMin, bombDamageMax, bombArmour, data->spriteWeapon,
		bombBlastRange, bombProximity, bombLifetime, bombKick));
	can_launch_bomb = false;
	return true;
}

int AlaryBomber::activate_special()
{
	STACKTRACE;

	// just some kind of afterburn
	accelerate(this, angle, special_accelrate * frame_time, special_speedmax);

	return true;
}


void AlaryBomber::calculate_hotspots()
{

	Ship::calculate_hotspots();

	if (this->fire_special)
	{
		Vector2 D = -17*unit_vector(angle+0.5*PI);
		game->add(new Animation(this, pos + D,
			meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));
		game->add(new Animation(this, pos - D,
			meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));
	}
	return;
}

void AlaryBomber::calculate()
{
	STACKTRACE;
	
	if (!fire_weapon)
		can_launch_bomb = true;

	Ship::calculate();
}





AlaryBomberBomb::AlaryBomberBomb (SpaceLocation *creator, double ox, double oy, double oangle, double odamage_min,
								  double odamage_max,
							double oarmour, SpaceSprite *osprite, double oblast_range, double oproximity,
							int olifetime, double okick)
:
Missile(creator, Vector2(ox, oy), oangle, 0, odamage_max, 1e40, oarmour, creator, osprite, 1.0),
blast_range(oblast_range),
proximity_range(oproximity),
old_range(1e40),
kick(okick),
blast_damage_min(odamage_min),
blast_damage_max(odamage_max),
lifetime(olifetime),
tgt(NULL),
active(false)
{
	id = SPACE_OBJECT;
    collide_flag_sameteam = 0;
    mass = 0.01;

//	rotation_angle = 0;
//	attributes &= ~ATTRIB_SHOT;

	attributes &= ~ATTRIB_STANDARD_INDEX;

}

void AlaryBomberBomb::calculate()
{
	STACKTRACE;

	SpaceLocation::calculate();	// to clean up pointers

	if (ship && !active)
	{
		if ((!ship->exists()) || (!ship->fire_weapon)) {
			ship = NULL;
            active = true;
		}
	}

	double d_a = normalize(atan(vel) - angle, PI2);
	if (d_a > PI) d_a -= PI2;
	d_a *= 1 - exp(-0.004*magnitude(vel)*frame_time);
	angle = normalize(angle + d_a, PI2);

	sprite_index = iround(angle / (PI2/64)) + 16;
	sprite_index &= 63;
	
	// disappear if the target is gone
	if (!(target && target->exists()))
	{
		die();
		return;
	}

	// explode at the end of its life
	if (lifetime > 0)
		lifetime -= frame_time;
	else
		damage(this, 999);


	// the original Tau Bomber checked it neighbourhood in order to detonate the bomb, however, it's far more
	// efficient to scan only the target.

	double r0;
	r0 = distance(target);
	if (active)
	{
		if ( r0 > old_range || target->isInvisible() )
		{
			// however, do not detonate if you're still too close to your own ship ...
			if (distance(ship) > blast_range)
			{
				damage(this, 9999);
			}
		}
	}
	old_range = r0;


}


void AlaryBomberBomb::animate(Frame *space)
{
	STACKTRACE
	sprite->animate(pos, sprite_index,space);
}


void AlaryBomberBomb::animateExplosion()
{
	
	STACKTRACE;

	if (active)
	{

		explosionSample = data->sampleWeapon[1];
        explosionSprite = data->spriteWeaponExplosion;
        explosionFrameCount = 10;
        explosionFrameSize = 50;

		Query q;
		double r;
		int d;

		// apply damage to anything that is close enough.
		for (q.begin(this, OBJECT_LAYERS, blast_range, QUERY_OBJECT); q.currento; q.next())
		{
			double dist;
			dist = distance(q.currento);
            if (dist > blast_range) continue;

			r = (blast_range - dist) / blast_range;
			// distance=0, then r=1
			// distance=max, then r=0

			// the damage...
			d = int( blast_damage_min + r * (blast_damage_max - blast_damage_min) );

            damage(q.currento, d);
            if ((q.currento->mass > 0) && (!q.currento->isPlanet()))
				q.currento->accelerate(this, trajectory_angle(q.currento), kick * r / ((q.currento->mass > 1)?sqrt(q.currento->mass):1), MAX_SPEED);
		}

		add(new AlaryBomberBombExplosion(pos, scale_velocity(70), 150, 450, makecol(255,240,140)));
	}

    Missile::animateExplosion();
}



AlaryBomberBombExplosion::AlaryBomberBombExplosion(Vector2 opos, double ov, int onum, int olife, int ocolor) :
	Presence(), num(onum), lifetime(olife), life_counter(0), color(ocolor)
{
	if (onum <= 0) {
		state = 0; return; }
	set_depth(DEPTH_EXPLOSIONS);
	xp = new Vector2[num];
	xv = new Vector2[num];
	int i;
	for (i=0; i<num; i++) {
		xp[i] = opos;
		xv[i] = ov * (0.5+sqrt(sqrt((tw_random(1.0))))) * unit_vector(tw_random(PI2));
	}
}


void AlaryBomberBombExplosion::calculate()
{
	STACKTRACE
	life_counter += frame_time;
	if (life_counter >= lifetime) {
		state = 0; return; }
	int i;
	for (i=0; i<num; i++)
		xp[i] += xv[i] * frame_time;
}

void AlaryBomberBombExplosion::animate(Frame *space)
{
	STACKTRACE
	if (state == 0)
		return;
	int i, j;
	double t = 1 - life_counter/(double)lifetime;
	double  x0, y0, dx, dy;
	int xi, yi;
	Vector2 p0;
	drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	for (i=0; i<num; i++) {
		p0 = corner(xp[i]);
		x0 = p0.x;
		y0 = p0.y;
		p0 = unit_vector(xv[i]) * 3 * space_zoom;
		dx = p0.x;
		dy = p0.y;
		for (j=3; j>=0; j--) {
			if (space_zoom <= 1)
				set_trans_blender(0, 0, 0, iround(space_zoom * 255 * t * (4-j) / 4.0));
			else
				set_trans_blender(0, 0, 0, iround(1* 255 * t * (4-j) / 4.0));
			xi = iround(x0 - dx * j);
			yi = iround(y0 - dy * j);
			putpixel(space->surface, xi, yi, color);
			space->add_pixel(xi, yi);
		}
	}
	drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}


AlaryBomberBombExplosion::~AlaryBomberBombExplosion()
{
	if (num > 0) {
		delete xp;
		delete xv; }
}




REGISTER_SHIP(AlaryBomber)
