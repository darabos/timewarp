

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class XaaarShot;

class Xaaar : public Ship
{
	double	weaponRange, weaponVelocity, weaponDamage, weaponArmour;
	XaaarShot *xs1, *xs2;
	
public:
	Xaaar(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);
		
	virtual int activate_weapon();
	virtual int activate_special();

	virtual void calculate();
	virtual void animate(Frame *f);
	
	Vector2 bombpos(int side);
};


class XaaarShot : public SpaceObject
{
	Xaaar *mother;
public:
	XaaarShot(Xaaar *mother, Vector2 rpos, double rangle, double v, double odamage,
								SpaceSprite *sprite);

	virtual void calculate();
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
	virtual void inflict_damage(SpaceObject *other);

	//virtual void animate(Frame *f);
};



Xaaar::Xaaar(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);

	xs1 = 0;
	xs2 = 0;
}


int Xaaar::activate_weapon()
{
	// destroy old bombs

	if (xs1)
	{
		xs1->die();
		xs1 = 0;
	}
	if (xs2)
	{
		xs2->die();
		xs2 = 0;
	}

	Vector2 dpos;
	double aoffs;
	
	// let the bombs fly !!
	dpos = bombpos(1);

	aoffs = 0.05 * PI;

	xs1 = new XaaarShot( this, dpos, aoffs,
			weaponVelocity, weaponDamage,
			data->spriteWeapon);
	add(xs1);

	dpos = bombpos(2);

	xs2 = new XaaarShot( this, dpos, -aoffs,
			weaponVelocity, weaponDamage,
			data->spriteWeapon);
	add(xs2);


	return TRUE;
}


int Xaaar::activate_special()
{
	// does nothing "special" here ; that's handled by the bomb.

	return TRUE;
}



Vector2 Xaaar::bombpos(int side)
{
	// place the bombs on either side.

	Vector2 P1, P2;

	P1 = -10 * unit_vector(angle);
	P2 = 25 * unit_vector(angle + 0.5*PI);

	if (side == 1)
		return P1 + P2;
	else
		return P1 - P2;
}


void Xaaar::animate(Frame *f)
{
	Ship::animate(f);

	Vector2 dpos;
	double a;
	int i;

	// show how much of the bomb is ready.
	a = (1.0 - double(weapon_recharge) / double(weapon_rate));	// counts down to 0; max = weapon_rate
	i = int(a * data->spriteExtra->frames());

	if (i >= data->spriteExtra->frames())
		i = data->spriteExtra->frames()-1;

	if (i < 0)
		i = 0;

	dpos = bombpos(1);
	data->spriteExtra->animate(pos+dpos, i, f);

	dpos = bombpos(2);
	data->spriteExtra->animate(pos+dpos, i, f);
}



void Xaaar::calculate()
{
	Ship::calculate();

	// check if the bombs still "exist"
	if (!(xs1 && xs1->exists()))
		xs1 = 0;
	if (!(xs2 && xs2->exists()))
		xs2 = 0;
}



XaaarShot::XaaarShot(Xaaar *omother, Vector2 rpos, double rangle, double v, double odamage,
								SpaceSprite *osprite)
:
SpaceObject(omother, omother->pos+rpos, omother->angle+rangle, osprite)
{
	damage_factor = odamage;
	mother = omother;

	vel = mother->vel + v * unit_vector(angle);

	sprite_index = 0;

	layer = LAYER_SHOTS;

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
}


void XaaarShot::calculate()
{
	SpaceObject::calculate();

	if (!(mother && mother->exists()))
	{
		mother = 0;
		state = 0;
		return;
	}

	if (!mother->fire_special)
		sprite_index = 0;
	else
		sprite_index = 1;
}


int XaaarShot::handle_damage(SpaceLocation *source, double normal, double direct)
{
	if (!mother->fire_special)
	{
		state = 0;
	}
	return 0;
}

void XaaarShot::inflict_damage(SpaceObject *other)
{
	SpaceObject::inflict_damage(other);

	// test if this bomb can be destroyed.
	XaaarShot::handle_damage(other, 1, 0);

	// it is _always_ destroyed by ships or planets.
	if (other->isShip() || other->isPlanet())
		state = 0;
}


REGISTER_SHIP(Xaaar)

