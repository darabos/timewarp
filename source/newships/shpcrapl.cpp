

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"
#include "../melee/mcbodies.h"


class Crash : public Ship
{
public:
	double	weaponRange, weaponVelocity, weaponDamage, weaponArmour, weaponNdebris, weaponActionRange, weaponSpreadAngle;
	double	specialVelocity, specialDamage, specialArmour, specialAngVel, specialActionRange;
	double	turn_rate_right;
	
	Crash(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);
	
protected:
	
	virtual int activate_weapon();
	virtual int activate_special();

	Asteroid *closest_asteroid(double maxrange);
	virtual void calculate_turn_right();
	virtual void animate(Frame *f);
};


class CrashAsteroid : public Asteroid
{
public:
	Crash *master;

	CrashAsteroid(Crash *amaster, Asteroid *ast);
	virtual void calculate();
	virtual void inflict_damage(SpaceObject *other);
	virtual void death();
};




Crash::Crash(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	turn_rate_right	= scale_turning(get_config_float("Ship", "TurnRateRight", 0));
	
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	weaponNdebris  = get_config_int("Weapon", "Ndebris", 0);
	weaponActionRange   = get_config_float("Weapon", "ActionRange", 0);
	weaponSpreadAngle = get_config_float("Weapon", "SpreadAngle", 0) * PI / 180;

	specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialDamage   = get_config_int("Special", "Damage", 0);
	specialArmour   = get_config_int("Special", "Armour", 0);
	specialAngVel   = get_config_float("Special", "AngVel", 0) * PI / 180;
	specialActionRange   = get_config_float("Special", "ActionRange", 0);
}



Asteroid *Crash::closest_asteroid(double maxrange)
{
	Asteroid *closest = 0;

	int layers = bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL) +
		bit(LAYER_CBODIES);
	double Range = maxrange;
	double R, Rmin = 1E9;

	Query a;
	for (a.begin(this, layers, Range); a.current; a.next())
	{
		SpaceObject *o = a.currento;
		if (o->isAsteroid())
		{
			R = distance(o);

			if (R < maxrange)
			{
				if (!closest || R < Rmin)
				{
					Rmin = R;
					closest = (Asteroid*) o;
				}
			}
		}
	}

	return closest;
}


// blow an asteroid to bits !
int Crash::activate_weapon()
{

	Asteroid *ast;

	ast = closest_asteroid(weaponActionRange);

	if (!ast)
		return FALSE;

	// direction ...
	double a0;
	a0 = trajectory_angle(ast);

	// create N pieces of debris
	int i;

	for ( i = 0; i < weaponNdebris; ++i )
	{
		double a;
		double da = weaponSpreadAngle / weaponNdebris;

		a = a0 + (i - 0.5*weaponNdebris) * da;

		Shot *s;
		s = new Shot(this, 0, a, weaponVelocity, 
			weaponDamage, weaponRange, weaponArmour, ast, 
			data->spriteWeapon, 0.0);

		add(s);
	}

	// well, you blow it up after all, don't you.
	ast->die();

	return TRUE;
}


int Crash::activate_special()
{

	Asteroid *ast;

	ast = closest_asteroid(specialActionRange);

	if (!ast)
		return FALSE;

	// create a missile control which guides the asteroid ...

	add( new CrashAsteroid(this, ast) );
	ast->die();

	return TRUE;
}


void Crash::calculate_turn_right()
{
	if(turn_right)
		turn_step += turn_rate_right * frame_time;
}


void Crash::animate(Frame *f)
{
	SpaceSprite *spr;
	Asteroid *ast;

	Ship::animate(f);

	ast = closest_asteroid(specialActionRange);

	if (ast)
	{
		spr = ast->get_sprite();
		spr->animate_character(ast->pos, ast->get_sprite_index(), makecol(0,128,0), f);
	} else {
		ast = closest_asteroid(weaponActionRange);

		if (ast)
		{
			spr = ast->get_sprite();
			spr->animate_character(ast->pos, ast->get_sprite_index(), makecol(128,0,0), f);
		}
	}
}


CrashAsteroid::CrashAsteroid(Crash *amaster, Asteroid *ast)
:
Asteroid()
{
	master = amaster;

	damage_factor = master->specialDamage;
	armour = master->specialArmour;

	pos = ast->pos;
	vel = ast->vel;
	angle = ast->vel.atan();

}

void CrashAsteroid::calculate()
{
	Presence::calculate();

	if (!(master && master->exists()))
	{
		master = 0;
		state = 0;
		return;
	}

	if (!(master->target && master->target->exists()))
		return;

	// fly to the target.

	double a, da, damax;
	a = trajectory_angle(master->target);

	da = a - angle;
	while (da > PI)		da -= PI;
	while (da < -PI)	da += PI;

	damax = master->specialAngVel * frame_time * 1E-3;
	if (da > damax)		da = damax;
	if (da < -damax)	da = -damax;

	angle += da;

	vel = master->specialVelocity * unit_vector(angle);
}


void CrashAsteroid::inflict_damage(SpaceObject *other)
{
	if ( other != master && other->isShip() )
	{
		Asteroid::inflict_damage(other);

		if (exists())
			die();
	}
}

void CrashAsteroid::death()
{
	// do nothing, prevent making a new asteroid !
	return;
}


REGISTER_SHIP(Crash)

