#include "../ship.h"
REGISTER_FILE

#include "../sc1ships.h"


class PanCohAvian : public Ship
{
public:
	double       weaponTime;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	double       specialTime;
	double       specialVelocity;
	int          specialDamage;
	int          specialArmour;

	public:
	PanCohAvian(Vector2 opos, double angle, ShipData *data, unsigned int code);

	virtual int activate_weapon();
	virtual int activate_special();
};



class TimedShot : public SpaceObject
{
public:
	double armour;
	double existtime, maxtime;

	TimedShot(SpaceLocation *creator, Vector2 orelpos, double orelangle, SpaceSprite *osprite,
		double ovel, double otime, double oarmour, double odamage);

	virtual void calculate();

	virtual void inflict_damage(SpaceObject *other);
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};


class PanCohCrystal : public TimedShot
{
public:
	PanCohCrystal(SpaceLocation *creator, Vector2 orelpos, double orelangle, SpaceSprite *osprite,
		double ovel, double otime, double oarmour, double odamage);
	
	virtual void inflict_damage(SpaceObject *other);
};



class PanCohFeather : public TimedShot
{
	double vref, period, angle0;
	double alpha, dalpha;
public:
	PanCohFeather(SpaceLocation *creator, Vector2 orelpos, double orelangle, SpaceSprite *osprite,
		double ovel, double otime, double oarmour, double odamage, double perturbedangle);

	virtual void calculate();
};




TimedShot::TimedShot(SpaceLocation *creator, Vector2 orelpos, double orelangle, SpaceSprite *osprite,
		double ovel, double otime, double oarmour, double odamage)
:
SpaceObject(creator, 0, 0, osprite),
existtime(0),
maxtime(otime),
armour(oarmour)
{
	angle = creator->angle + orelangle;
	pos = creator->pos + rotate(orelpos, creator->angle - PI/2);
	vel = ovel * unit_vector(angle);

	layer = LAYER_SHOTS;

	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_sameship = 0;

	damage_factor = odamage;
}

void TimedShot::calculate()
{
	SpaceObject::calculate();

	if (!(ship && ship->exists()))
	{
		state = 0;
		return;
	}

	if (existtime >= maxtime)
	{
		state = 0;
		return;
	}

	existtime += frame_time * 1E-3;

	// always orient the shot in the direction of movement.
	angle = vel.angle();
	sprite_index = get_index(angle);
}

void TimedShot::inflict_damage(SpaceObject *other)
{
	// copied from Shot::infli...

	if (!other->exists()) return;

	damage(other, damage_factor);

	if (!other->isShot()) state = 0;

	if (state == 0)
	{
		//animateExplosion();
		//soundExplosion(); 
	}

}

int TimedShot::handle_damage(SpaceLocation *source, double normal, double direct)
{
	armour -= (normal + direct);

	if (armour <= 0)
	{
		armour = 0;
		state = 0;
	}

	return true;
}



PanCohCrystal::PanCohCrystal(SpaceLocation *creator, Vector2 orelpos, double orelangle, SpaceSprite *osprite,
		double ovel, double otime, double oarmour, double odamage)
:
TimedShot(creator, orelpos, orelangle, osprite, ovel, otime, oarmour, odamage)
{
	// it needs mass, otherwise it won't bounce off.
	mass = 0.1;
}
	
void PanCohCrystal::inflict_damage(SpaceObject *other)
{
	if (!state)
		return;

	TimedShot::inflict_damage(other);

	// don't die, just bounce (so, override the state=0 thingy in timedshot).
	state = 1;
}



PanCohFeather::PanCohFeather(SpaceLocation *creator, Vector2 orelpos, double orelangle, SpaceSprite *osprite,
		double ovel, double otime, double oarmour, double odamage, double perturbedangle)
:
TimedShot(creator, orelpos, orelangle, osprite, ovel, otime, oarmour, odamage)
{
	vref = ovel;

	angle0 = angle;

	period = 1.0;

	alpha = perturbedangle;	// perturbed position.
	dalpha = 0;			// at rest
}

void PanCohFeather::calculate()
{
	// the velocity of a falling feather is ... somewhat weird.

	/*
	Vector2 vchange;

	double si, co;
	si = sin(PI2 * existtime / period);
	co = cos(PI2 * existtime / period);
	vchange = vref * Vector2(2*(fabs(si)-0.2), co);

	int k;
	k = existtime / period;
	*/


	// pendulum?
	double dt, g, R;

	dt = frame_time * 1E-3;

	g = 1.0;
	R = 0.1;

	dalpha += (g/R) * sin(alpha) * dt;
	alpha += dalpha * dt;



	//vel = vref*unit_vector(angle0) + rotate( vchange, angle0 );
	vel = Vector2(0.1, 0);

	vel += R*dalpha * unit_vector(alpha-PI/2);

	vel = vref * rotate( vel, angle0 );


	TimedShot::calculate();
}



PanCohAvian::PanCohAvian(Vector2 opos, double angle, ShipData *data, unsigned int code) 
:
Ship(opos, angle, data, code)
{

	weaponTime     = get_config_float("Weapon", "Time", 0);
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);

	specialTime     = get_config_float("Special", "Time", 0);
	specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialDamage   = get_config_int("Special", "Damage", 0);
	specialArmour   = get_config_int("Special", "Armour", 0);
}


int PanCohAvian::activate_weapon()
{
	//creator, orelpos, orelangle, osprite, ovel, otime, oarmour, odamage
	TimedShot *tmp = new PanCohCrystal( this, 
		Vector2(0.0, size.y / 1.5), 0.0, data->spriteWeapon,
		weaponVelocity, weaponTime, weaponArmour, weaponDamage);
	add(tmp);

//	tmp->explosionSprite = data->spriteWeaponExplosion;
//	tmp->explosionSample = 0;
//	tmp->explosionFrameCount = data->spriteWeaponExplosion->frames();
//	tmp->explosionFrameSize = 100;

	return(TRUE);
}


int PanCohAvian::activate_special()
{
	int i;

	for ( i = -1; i < 2; ++i )
	{
		if (i == 0)
			continue;

		PanCohFeather *tmp = new PanCohFeather( this, 
			Vector2(0.0, -size.y / 1.5), PI*(1+0.1*i), data->spriteSpecial,
			specialVelocity, specialTime, specialArmour, specialDamage,
			i*0.4*PI);
		add(tmp);
	}

//	tmp->explosionSprite = data->spriteSpecialExplosion;
//	tmp->explosionSample = 0;
//	tmp->explosionFrameCount = data->spriteSpecialExplosion->frames();
//	tmp->explosionFrameSize = 100;

	return(TRUE);
}






REGISTER_SHIP ( PanCohAvian )
