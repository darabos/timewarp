

#include "ship.h"
REGISTER_FILE

#include "frame.h"

const int ID_WOLLYPLASMA = 0x0f98a301;


class Wolly : public Ship
{
	double	weaponRange, weaponVelocity, weaponDamage, weaponArmour;
	double	specialDamage, specialLifeTime;
	
public:
	Wolly(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);
	
protected:
	
	virtual int activate_weapon();
	virtual int activate_special();
	
};


class WollyPlasma : public Shot
{
public:

	double default_range;

	WollyPlasma(SpaceLocation *creator, Vector2 rpos, double oangle, double ov, 
			double odamage, double orange, double oarmour, SpaceLocation *opos, 
			SpaceSprite *osprite, double relativity);

	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
	virtual void calculate();

	virtual int canCollide(SpaceLocation *other);
};



class WollyMine : public SpaceObject
{
	double lifetime, existtime;
public:
	WollyMine(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite,
				double odamage, double olifetime);

	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
	virtual void calculate();
};



Wolly::Wolly(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);

	specialDamage   = get_config_int("Special", "Damage", 0);
	specialLifeTime = get_config_float("Special", "LifeTime", 1);

}


int Wolly::activate_weapon()
{	
	STACKTRACE;
	

	Vector2 rpos;

	rpos = Vector2(0, 100);

	add ( new WollyPlasma( this, rpos, angle, weaponVelocity,
		weaponDamage, weaponRange, weaponArmour, this,
		data->spriteWeapon, 0));
	
	return TRUE;
}


int Wolly::activate_special()
{
	STACKTRACE;

	WollyMine *o;
	Vector2 rpos;

	rpos = Vector2(-50, 0);
	rpos = rotate(rpos, angle);

	o = new WollyMine(this, pos+rpos, 0, data->spriteSpecial, specialDamage, specialLifeTime);

	add( o );


	return TRUE;
}



WollyPlasma::WollyPlasma(SpaceLocation *creator, Vector2 rpos, double oangle, double ov, 
			double odamage, double orange, double oarmour, SpaceLocation *opos, 
			SpaceSprite *osprite, double relativity)
:
Shot(creator, rpos, oangle, ov, odamage, orange, oarmour, opos, osprite, relativity)
{
	default_range = orange;

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;

	id = ID_WOLLYPLASMA;
}

int WollyPlasma::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE;

	armour += normal + direct;

	if (armour > 6)	// note, there are only 6 sprites ...
	{
		state = 0;
		
		Vector2 rpos;
		rpos = Vector2(0, 20);
		rotate(rpos, angle);

		add ( new WollyPlasma( this, rpos, angle + PI/4, v,
			3, range, 3, this, data->spriteWeapon, 0));

		add ( new WollyPlasma( this, -rpos, angle - PI/4, v,
			3, range, 3, this, data->spriteWeapon, 0));
	}

	return 0;
}

void WollyPlasma::calculate()
{
	STACKTRACE;
	
	Shot::calculate();

	if (d > default_range)
	{
		state = 1;

		d = 0;
		range = default_range;
		--armour;

		if (armour <= 0)
		{
			state = 0;
			return;
		}

	}

	sprite_index = iround(armour-1);
	damage_factor = armour;
}


int WollyPlasma::canCollide(SpaceLocation *other)
{
	if (other->id == ID_WOLLYPLASMA)
		return FALSE;
		
	
	return SpaceObject::canCollide(other);	
}




WollyMine::WollyMine(SpaceLocation *creator, Vector2 opos, double oangle, SpaceSprite *osprite,
				double odamage, double olifetime)
:
SpaceObject(creator, opos, oangle, osprite)
{
	lifetime = olifetime;
	existtime = 0;

	damage_factor = odamage;

	layer = LAYER_SPECIAL;

	sprite_index = 0;

	isblockingweapons = false;

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = ALL_LAYERS;
}


void WollyMine::calculate()
{
	SpaceObject::calculate();

	existtime += frame_time * 1E-3;

	if (existtime > lifetime || !ship)
	{
		state = 0;
		return;
	}
}


int WollyMine::handle_damage(SpaceLocation *source, double normal, double direct)
{
	state = 0;
	return true;
}


REGISTER_SHIP(Wolly)

