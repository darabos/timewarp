

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"


class MeknikChainsaw;
class LaserInform;

class MeknikPincer : public Ship
{
	double	rockRange, rockVelocity, rockDamage, rockArmour;
	double	weaponColor, weaponRange, weaponFrames, weaponDamage;
	double specialTurnperiod, specialDamage, specialDamageperiod;

	MeknikChainsaw* saw[2];
	double sawangle[2];
	
public:
	MeknikPincer(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

	void getinformed(int itype, SpaceLocation *other);
	int special_state();
	
protected:
	
	virtual void calculate();
	//virtual void animate(Frame *f);
	
	virtual int activate_weapon();
	virtual int activate_special();

	virtual int handle_damage(SpaceLocation* source, double normal, double direct);

};



class MeknikChainsaw : public SpaceObject
{
	double	turnperiod, damage, damageperiod, dist;
	double	lifetime, sawtime, inflict_time;
	MeknikPincer *refship;
	
public:
	MeknikChainsaw(MeknikPincer *oship, double odist, double a,
		double turnperiod, double damage, double damageperiod,
		SpaceSprite *ospr);
	
protected:
	
	virtual void calculate();
//	virtual void animate(Frame *f);
	
//	virtual int handle_damage(SpaceLocation* source, double normal, double direct);
	virtual void inflict_damage(SpaceObject *other);

};



class LaserInform : public Laser
{
	MeknikPincer *mother;
public:
	LaserInform(MeknikPincer *creator, double langle, int lcolor, double lrange, double ldamage,
			int lfcount, SpaceLocation *opos, Vector2 rpos, bool osinc_angle);

	virtual void inflict_damage(SpaceObject *other);
	virtual void calculate();
};



MeknikPincer::MeknikPincer(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	weaponColor  = get_config_int("Weapon", "Color", 0);
	weaponRange  = scale_range(get_config_float("Weapon", "Range", 0));
	weaponDamage = get_config_int("Weapon", "Damage", 0);
	
	rockRange    = scale_range(get_config_float("Quirk", "Range", 0));
	rockVelocity = scale_velocity(get_config_float("Quirk", "Velocity", 0));
	rockDamage   = get_config_int("Quirk", "Damage", 0);
	rockArmour   = get_config_int("Quirk", "Armour", 0);

	specialTurnperiod = get_config_float("Special", "Turnperiod", 1.0);
	specialDamage = get_config_float("Special", "Damage", 1.0);
	specialDamageperiod = get_config_float("Special", "Damageperiod", 1.0);

	saw[0] = 0;
	saw[1] = 0;

	sawangle[0] = 0;
	sawangle[1] = 0;
}



void MeknikPincer::getinformed(int itype, SpaceLocation *other)
{
	// if you've destroyed (i=1) an asteroid, then...
	if (itype == 1 && other->isAsteroid())
	{
		//damage(source, 1000, 1000);
		add(new Shot(this, Vector2(0, -size.y/2),
			angle+PI, rockVelocity, rockDamage, rockRange, rockArmour,
			this, data->spriteWeapon));

		vel += rockVelocity * unit_vector(angle);
	}
}

int MeknikPincer::special_state()
{
	return special_low;
}


int MeknikPincer::activate_weapon()
{	
	// activate laser (s)

	int i;
	double D = 0.125 * PI;

	for ( i = -2; i <= 2; ++i)
	{
		double R, da;
		Vector2 rpos;

		da = i * D;

		R = 50;
		rpos = Vector2(0, 5) + R * unit_vector(PI/2-da);

		add(new LaserInform(this, angle + da,
			makecol(128,0,64), weaponRange / (1+fabs(i)), weaponDamage, weapon_rate,
			this, rpos, true));
	}

	
	return TRUE;
}


int MeknikPincer::activate_special()
{
	// for re-activation, require minimum batt.
	if (!saw[0] || !saw[1])
		if (batt < 4)
			return FALSE;

	int i;
	for ( i = 0; i < 2; ++i )
	{
		int d;

		if ( i == 0 )
			d = 1;
		else
			d = -1;

		if (!saw[i])
		{
			saw[i] = new MeknikChainsaw(this, 60.0, sawangle[i],
				specialTurnperiod * d, specialDamage, specialDamageperiod,
				data->spriteSpecial);
			add(saw[i]);
		}
	}

	return TRUE;
}


void MeknikPincer::calculate()
{

	int i;
	for ( i = 0; i < 2; ++i )
	{
		if ( !(saw[i] && saw[i]->exists()) )
			saw[i] = 0;
		else
			sawangle[i] = saw[i]->angle;
	}

	Ship::calculate();
}


int MeknikPincer::handle_damage(SpaceLocation* source, double normal, double direct)
{
	/*
	if (source->isAsteroid())
	{
		damage(source, 1000, 1000);
		add(new Shot(this, Vector2(0, -size.y/2),
			angle+PI, rockVelocity, rockDamage, rockRange, rockArmour,
			this, data->spriteWeapon));

		vel += rockVelocity * unit_vector(angle);
	}
	*/

	return Ship::handle_damage(source, normal, direct);
}


MeknikChainsaw::MeknikChainsaw(MeknikPincer *oship, double odist, double a,
		double oturnperiod, double odamage, double odamageperiod,
		SpaceSprite *ospr)
:
SpaceObject(oship, oship->pos+odist*unit_vector(a), a, ospr)
{
	refship = oship;
	turnperiod = oturnperiod;
	damage = odamage;
	damageperiod = odamageperiod;
	dist = odist;
	angle = a;

	lifetime = 0;
	sawtime = 0;
	inflict_time = 0;

	mass = 0;	// so that it can slice through without pushing it away
	layer = LAYER_SPECIAL;
}


void MeknikChainsaw::calculate()
{
	SpaceObject::calculate();

	if ( !(refship && refship->exists()) )
	{
		refship = 0;
		state = 0;
		return;
	}

	// if special is pressed, and batt isn't low.
	if ( !refship->fire_special && !refship->special_state() )
	{
		state = 0;
		return;
	}


	double dt;

	dt = frame_time * 1E-3;
	sawtime += dt;

	double w;
	int i;

	w = 0.2;
	i = int(sawtime / w);

	if (i < 0 || i >= 3)
	{
		i = 0;
		//sawtime -= 3 * w;
		sawtime = 0;
	}

	angle += PI2 * dt / turnperiod;
	if (angle > PI)
		angle -= PI2;
	if (angle < -PI)
		angle += PI2;

	sprite_index = get_index(angle) + i*64;
	//sprite_index = i * 64;
	i = sprite->frames();

	pos = refship->pos + dist * unit_vector(angle+PI/2);
	vel = refship->vel;	// not really needed ?
}


void MeknikChainsaw::inflict_damage(SpaceObject *other)
{
	//SpaceObject::inflict_damage(other);
	inflict_time += frame_time * 1E-3;

	while (inflict_time > damageperiod)
	{
		inflict_time -= damageperiod;
		SpaceObject::damage(other, damage, 0);

		if (!other || !other->state)
		{
			refship->getinformed(1, other);
			break;
		}
	}
}



LaserInform::LaserInform(MeknikPincer *creator, double langle, int lcolor, double lrange, double ldamage,
			int lfcount, SpaceLocation *opos, Vector2 rpos, bool osinc_angle)
:
Laser(creator, langle, lcolor, lrange, ldamage, lfcount, opos, rpos, osinc_angle)
{
	mother = creator;
}


void LaserInform::calculate()
{
	if (!(mother && mother->exists()))
	{
		mother = 0;
		state = 0;
		return;
	}

	Laser::calculate();
}

void LaserInform::inflict_damage(SpaceObject *other)
{
	Laser::inflict_damage(other);

	if (!other || !other->exists())
	{
		mother->getinformed(1, other);
	}
}


REGISTER_SHIP(MeknikPincer)

