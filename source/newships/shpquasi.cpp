/* $Id$ */ 
#include "../ship.h"
#include "../scp.h"
REGISTER_FILE

#include "../sc1ships.h"


class Quai : public Ship
{
public:
IDENTITY(Quai);
public:
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;
	double		weaponDelay;

	double       specialTime;
	double       specialVelocity;
	int          specialDamage;
	int          specialArmour;
	double		specialPush;

	int			enginereleased;

	public:
	Quai(Vector2 opos, double angle, ShipData *data, unsigned int code);

	virtual int activate_weapon();
	virtual int activate_special();

	virtual void calculate();
	virtual void animate(Frame *f);

	virtual void calculate_thrust();
	virtual double isInvisible() const;
};



class QuaiShot : public Missile
{
public:
IDENTITY(QuaiShot);
	double		timedelay, twait;

public:

	QuaiShot(SpaceLocation *creator, Vector2 orelpos, double orelangle,
		double ovel, double odamage, double orange, double oarmour, double odelay,
		SpaceSprite *osprite);
	
	virtual void calculate();
	virtual void animate(Frame *f);
};



class QuaiEngine : public SpaceObject
{
public:
IDENTITY(QuaiEngine);
	Quai *mother;

public:


	double armour;


	QuaiEngine(Quai *creator, Vector2 opos, double oangle, SpaceSprite *osprite);

	virtual void calculate();
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);

};



QuaiShot::QuaiShot(SpaceLocation *creator, Vector2 orelpos, double orelangle,
		double ovel, double odamage, double orange, double oarmour, double odelay,
		SpaceSprite *osprite)
:
Missile(creator, orelpos, orelangle, ovel, odamage, orange, oarmour, creator, osprite, 0 )
{
	angle = creator->angle + orelangle;

	timedelay = odelay;
	twait = 0;

	vel = 0;	// wait.
}
	
void QuaiShot::calculate()
{
	STACKTRACE
	if (!(ship && ship->exists()))
	{
		state = 0;
		ship = 0;
		return;
	}

	if (twait < timedelay)
	{
		twait += frame_time * 1E-3;

	} else {

		vel = v * unit_vector(angle);

		Missile::calculate();
		sprite_index = get_index(angle);
	}

}

void QuaiShot::animate(Frame *f)
{
	STACKTRACE
	if (twait >= timedelay)
		Missile::animate(f);
}



QuaiEngine::QuaiEngine(Quai *creator, Vector2 opos, double oangle, SpaceSprite *osprite)
:
SpaceObject(creator, opos, oangle, osprite)
{
	layer = LAYER_SPECIAL;

	mother = creator;
	vel = mother->vel;



	damage_factor = mother->specialDamage;

	armour = mother->specialArmour;

	isblockingweapons = false;
}

void QuaiEngine::calculate()
{
	STACKTRACE
	if (!(mother && mother->exists()))
	{
		mother = 0;
		state = 0;
		return;
	}

	if (mother->thrust)
	{
		// ???
	}

	if (!mother->enginereleased)
	{

		// cease to exist
		mother = 0;
		state = 0;



		// also cause a small explosion, perhaps ?

		

		// ...


		return;
	}

	SpaceObject::calculate();
}



int QuaiEngine::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE
	armour -= normal + direct;

	if ( armour < 0 || source->isShip() )
		state = 0;

	return 0;

}





Quai::Quai(Vector2 opos, double angle, ShipData *data, unsigned int code) 
:
Ship(opos, angle, data, code)
{

	weaponRange     = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	weaponDelay    = get_config_float("Weapon", "Delay", 0);

	specialTime     = get_config_float("Special", "Time", 0);
	specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialDamage   = get_config_int("Special", "Damage", 0);
	specialArmour   = get_config_int("Special", "Armour", 0);
	specialPush     = get_config_float("Special", "Push", 0);

	enginereleased = 0;
}


int Quai::activate_weapon()
{
	STACKTRACE

//	QuaiShot(SpaceLocation *creator, Vector2 orelpos, double orelangle,
//		double ovel, double odamage, double orange, double oarmour,
//		SpaceSprite *osprite);
	add(new QuaiShot(this, Vector2(0, 30), 0.0,
		weaponVelocity, weaponDamage, weaponRange, weaponArmour, weaponDelay,
		data->spriteWeapon));

	return(TRUE);
}


int Quai::activate_special()
{

	STACKTRACE
	if (!enginereleased)
	{
		QuaiEngine *tmp;

		tmp = new QuaiEngine(this, pos, angle, data->spriteSpecial);
		enginereleased = 1;
		add(tmp);

		targets->add(tmp);

		// separation from the engine pushes the ship it away a little

		// but the engine stays on its original course as if nothing happened
		vel += specialPush * speed_max * unit_vector(angle);

		// remove this thing from the target list
		targets->rem(this);

		// check all existing objects, and if they're targetting
		// this ship, let them target the engine instead.
		int i;
		for ( i = 0; i < physics->num_items; ++i )
		{
			if (physics->item[i]->target == this)
				physics->item[i]->target = tmp;
		}
	} else {
		enginereleased = 0;

		// add this thing to the targetlist
		targets->add(this);

	}

	return(TRUE);
}


void Quai::calculate()
{
	STACKTRACE
	Ship::calculate();

	if (!state)
		return;

	sprite_index = get_index(angle);

	if (enginereleased)
		sprite_index += 64;
}

void Quai::calculate_thrust()
{
	STACKTRACE
	if (!enginereleased)
		Ship::calculate_thrust();
}


double Quai::isInvisible() const
{
	return enginereleased;
}

void Quai::animate(Frame *space)
{
	if (isInvisible() && !is_bot(control->channel) && is_local(control->channel) || (!game_networked && num_network>1))	// bots and remote players are "hidden"
		sprite->animate_character( pos, sprite_index, pallete_color[4], space);
	else
		Ship::animate(space);
}


REGISTER_SHIP ( Quai )
