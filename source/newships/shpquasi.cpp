#include "../ship.h"
REGISTER_FILE

#include "../sc1ships.h"


class Quai : public Ship
{
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
//	virtual void animate(Frame *f);

	virtual void calculate_thrust();
	virtual double isInvisible() const;
};



class QuaiShot : public Missile
{
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
	Quai *mother;

public:

	QuaiEngine(Quai *creator, Vector2 opos, double oangle, SpaceSprite *osprite);

	virtual void calculate();
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
}

void QuaiEngine::calculate()
{
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
		mother = 0;
		state = 0;
		return;
	}

	SpaceObject::calculate();
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

	if (!enginereleased)
	{
		QuaiEngine *tmp;

		tmp = new QuaiEngine(this, pos, angle, data->spriteSpecial);
		enginereleased = 1;
		add(tmp);

		game->add_target(tmp);

		// separation from the engine pushes it away a little
		vel += 0.1 * speed_max * unit_vector(angle);

	} else {
		enginereleased = 0;
	}

	return(TRUE);
}


void Quai::calculate()
{
	Ship::calculate();

	if (!state)
		return;

	sprite_index = get_index(angle);

	if (enginereleased)
		sprite_index += 64;
}

void Quai::calculate_thrust()
{
	if (!enginereleased)
		Ship::calculate_thrust();
}


double Quai::isInvisible() const
{
	return enginereleased;
}



REGISTER_SHIP ( Quai )
