/* $Id$ */ 

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"


class CrewSupplyship : public Ship
{
public:
IDENTITY(CrewSupplyship);
	double	weaponRange, weaponVelocity, weaponDamage, weaponArmour, weaponTurnRate;
	double	specialPeriod;
	int		specialNlights;

	bool	lightcolor;
	
public:
	CrewSupplyship(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);
	
protected:
	
	virtual void calculate();
	
	virtual int activate_weapon();
	virtual int activate_special();
	
	virtual int handle_damage(SpaceLocation* source, double normal, double direct);
	virtual void animate(Frame *f);
};


class SupplyShuttle : public HomingMissile
{
public:
IDENTITY(SupplyShuttle);
	SpaceSprite *sprRG;
	bool colorstate;
	double flashperiod, flashtime;

public:
	SupplyShuttle(SpaceLocation *creator, Vector2 rpos, 
			double oangle, double ov, double odamage, double orange, double oarmour, 
			double otrate, SpaceLocation *opos, SpaceSprite *osprite, SpaceObject *otarget,
			SpaceSprite *osprRG, double operiod);

	virtual void calculate();

	virtual void animate(Frame *f);
};




CrewSupplyship::CrewSupplyship(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	weaponTurnRate = scale_turning(get_config_float("Weapon", "TurnRate", 0));

	specialPeriod  = get_config_float("Special", "T", 0);
	specialNlights = get_config_int("Special", "N", 0);

	lightcolor = 0;
}


int CrewSupplyship::activate_weapon()
{	
	STACKTRACE
	// find the planet
	int i;
	for (i = 0; i < physics->num_items; ++i )
		if (physics->item[i]->isPlanet())
			break;

	// add a little ship that flies to the planet
	if (i != physics->num_items)
		add(new SupplyShuttle(this, Vector2(0,50), angle,
					weaponVelocity,	weaponDamage, weaponRange, weaponArmour, weaponTurnRate,
					this, data->spriteWeapon, (SpaceObject*) physics->item[i],
					data->spriteSpecial, specialPeriod) );

	return TRUE;
}


int CrewSupplyship::activate_special()
{

	STACKTRACE
	// change the ships lights

	lightcolor = !lightcolor;

	return TRUE;
}



void CrewSupplyship::calculate()
{
	STACKTRACE
	Ship::calculate();

	sprite_index = get_sprite_index();
}


// delete lights (for a while), and/or create extra ships ?
int CrewSupplyship::handle_damage(SpaceLocation* source, double normal, double direct)
{
	Ship::handle_damage(source, normal, direct);

	/* actually, never mind. They'll die with the ship anyway.
	if (crew <= 0)
	{
		int n = 10 + random(15);

		int i;
		for ( i = 0; i < n; ++i )
		{
			double a = tw_random(PI2);
			double r = 50 + tw_random(50.0);
			Vector2 P = r * unit_vector(a);

			add(new SupplyShuttle(this, P, a,
						weaponVelocity,	weaponDamage, weaponRange, weaponArmour, weaponTurnRate,
						this, data->spriteWeapon, (SpaceObject*) physics->item[i],
						data->spriteSpecial, specialPeriod) );
		}
	}
	*/
	return 1;
}

void CrewSupplyship::animate(Frame *f)
{

	STACKTRACE
	sprite->animate(pos, sprite_index, f);

	int i;
	Vector2 relpos;
	for ( i = 0; i < specialNlights; ++i )
	{
		relpos = (100 - i*10) * unit_vector(angle);
		
		int k;
		if (lightcolor)
			k = 0;
		else
			k = 1;
		
		data->spriteSpecial->animate(pos + relpos, (i+k) % 2, f);
		//sprRG->animate(pos - relpos, 1, f);
	}
	
}



SupplyShuttle::SupplyShuttle(SpaceLocation *creator, Vector2 rpos, 
			double oangle, double ov, double odamage, double orange, double oarmour, 
			double otrate, SpaceLocation *opos, SpaceSprite *osprite, SpaceObject *otarget,
			SpaceSprite *osprRG, double operiod)
:
HomingMissile(creator, rpos, oangle, ov, odamage, orange, oarmour, otrate,
			  opos, osprite, otarget)
{
	sprRG = osprRG;
	colorstate = 1;

	flashperiod = operiod;
	flashtime = 0;
}



void SupplyShuttle::calculate()
{
	STACKTRACE;

	/*
	// if the mothership dies, you lose the sprites...
	// well... no, cause they fly to the planet anyway.
	if (!(ship && ship->exists()))
	{
		ship = 0;
		state = 0;
		return;
	}
	*/


	flashtime += frame_time * 1E-3;
	if (flashtime > flashperiod)
	{
		flashtime -= flashperiod;
		colorstate = !colorstate;
	}
	

	HomingMissile::calculate();

}



void SupplyShuttle::animate(Frame *f)
{
	STACKTRACE
	Vector2 relpos, rel2;

	relpos = 10 * unit_vector(angle-PI/2);

	rel2 = 5 * unit_vector(angle+PI);


	sprite->animate(pos, sprite_index, f);

	if (colorstate)
	{
		sprRG->animate(pos + rel2 + relpos, 0, f);
		sprRG->animate(pos + rel2 - relpos, 1, f);
	}
}





REGISTER_SHIP(CrewSupplyship)

