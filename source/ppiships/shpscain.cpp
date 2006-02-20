/* $Id$ */ 
#include "../ship.h"
#include "../scp.h"
REGISTER_FILE

/** Copy of the Korvian Sniper
Extra quirk is after the Narool Lurker
*/


class ScavengerInterloper : public Ship {
public:
IDENTITY(ScavengerInterloper);
public:
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	double specialRange, specialVelocity;
	int specialDamage, specialArmour, SkipperPeriod, VisiblePeriod;
	double SkipperMaxAngle;

	bool cloak;

	// visible after taking damage
	int visible_time, visible_period;

	// bright flare after shooting
	int flare_time;

	

	public:
	ScavengerInterloper(Vector2 opos, double angle, ShipData *data, unsigned int code);

	virtual double isInvisible() const;
	virtual int activate_weapon();
	virtual int activate_special();
	virtual void calculate_hotspots();
	virtual void calculate();
	virtual void animate(Frame *space);

	virtual void inflict_damage(SpaceObject *other);
};

class ScavengerInterloperMissile : public Missile {
public:
IDENTITY(ScavengerInterloperMissile);
	public:
	ScavengerInterloperMissile(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite);
};

class SkipperMissile : public Missile
{
IDENTITY(SkipperMissile);
	int timer, vtime;
	int visible_period, skipper_period;
	double max_angle_change;
public:
	SkipperMissile(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
			int askipper_period, int avisible_period, double max_angle_change);
	virtual void calculate();
	virtual double isInvisible() const;
	virtual void animate(Frame *space);
public:
};



ScavengerInterloper::ScavengerInterloper(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
{
  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);

  specialRange = scale_range( get_config_float("Special", "Range", 50) );
	specialVelocity = scale_velocity( get_config_float("Special", "Velocity", 30) );

	specialDamage = get_config_int("Special", "Damage", 1);
	specialArmour = get_config_int("Special", "Armour", 2);
	SkipperPeriod = get_config_int("Special", "SkipperPeriod", 1000);
	VisiblePeriod = get_config_int("Special", "VisiblePeriod", 500);
	SkipperMaxAngle = get_config_float("Special", "MaxAngle", 45.0) * PI / 180.0;


	cloak = true;
	debug_id = 1009;
	visible_time = 0;
	visible_period = get_config_int("Quirk", "VisiblePeriod", 1000);
	flare_time = get_config_int("Quirk", "FlareTime", 100);

}

double ScavengerInterloper::isInvisible() const
{
	return cloak;
}

int ScavengerInterloper::activate_weapon()
{

	Vector2 dP = Vector2(0.0, 0.7*size.y);

	ScavengerInterloperMissile *m;
	m = new ScavengerInterloperMissile( dP,
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon);
	game->add(m);

	play_sound2(data->sampleWeapon[0]);

	// when a weapon is fired, also add a bright flash to the game:
	SpaceSprite *spr = data->spriteWeaponExplosion;
	Animation *a = new Animation(this, m->pos, spr,
		0, spr->frames(), flare_time, DEPTH_EXPLOSIONS);
	
	targets->add(a);
	game->add(a);

	return TRUE;
}


int ScavengerInterloper::activate_special()
{
	SkipperMissile *m = new SkipperMissile( Vector2(0.0, 0.7*size.y),
		angle, specialVelocity, specialDamage, specialRange, specialArmour,
		this, data->spriteSpecial, SkipperPeriod, VisiblePeriod, SkipperMaxAngle);
	game->add(m);

	SpaceSprite *spr = data->spriteSpecialExplosion;
	Animation *a = new Animation(this, m->pos, spr,
		0, spr->frames(), flare_time, DEPTH_EXPLOSIONS);

	targets->add(a);
	game->add(a);

	return TRUE;
}


void ScavengerInterloper::calculate_hotspots()
{
    //Ship::calculate_hotspots();
}



void ScavengerInterloper::calculate()
{
	

	if (visible_time > 0)
	{
		cloak = false;

		visible_time -= frame_time;
		if (visible_time < 0)
			visible_time = 0;
	} else {
		cloak = true;
	}
	
	Ship::calculate();
}

void ScavengerInterloper::animate(Frame *space)
{

	if (!show_red_cloaker || (control && is_bot(control->channel)) || (control && !is_local(control->channel)) || (!game_networked && num_network>1))	// bots and remote players are "hidden"
	{
//#ifdef _DEBUG
//		// in debug mode, show where the enem
//		sprite->animate_character( pos, sprite_index, pallete_color[4], space);
//#else
		if (cloak)
			sprite->animate_character( pos, sprite_index, pallete_color[255], space);
		else
			sprite->animate_character( pos, sprite_index, pallete_color[15], space);
//#endif
	}
	else
	{
		sprite->animate_character( pos, sprite_index, pallete_color[4], space);
	}

	return;
}



void ScavengerInterloper::inflict_damage(SpaceObject *other)
{

	Ship::inflict_damage(other);

	// after hitting something, the ship should become visible.
	visible_time = visible_period;
	cloak = false;

	//you've hit something; (re)activate sparks.
	//lightning.reset();

	// but, where did you hit it ?
	// place the source somewhere... at the edge .. how ?
//	double a, R;
//	a = trajectory_angle(other);
//	R = 20;
	
//	lightning.locate(R * unit_vector(a));
}

ScavengerInterloperMissile::ScavengerInterloperMissile(Vector2 opos, double oangle, double ov,
	int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite) 
	:
	Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship ,osprite) 
	{
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = explosionSprite->frames();
	explosionFrameSize  = 50;
	debug_id = 1011;
}



SkipperMissile::SkipperMissile(Vector2 opos, double oangle, double ov,
	int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
	int askipper_period, int avisible_period, double amax_angle_change)
:
Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship ,osprite) 
{
	timer = 0;
	vtime = 0;
	visible_period = avisible_period;
	skipper_period = askipper_period;
	max_angle_change = amax_angle_change;
}

void SkipperMissile::calculate()
{

	timer += frame_time;

	if (vtime > 0)
		vtime -= frame_time;
	else
		vtime = 0;

	if (timer > skipper_period)
	{
		timer -= skipper_period;
		// re-orientate, and become visible for a short while

		vtime = visible_period;

		// desired change in angle.
		double a = trajectory_angle(target);
		double da = a - angle;
		while (da < -PI) da += PI2;
		while (da >  PI) da -= PI2;

		// change the angle by a maximum amount
		if (da < -max_angle_change)
			da = -max_angle_change;
		if (da >  max_angle_change)
			da = max_angle_change;

		angle += da;

		vel = v * unit_vector(angle);
	}

	Missile::calculate();

	if (!target)
		die();
}

double SkipperMissile::isInvisible() const
{
	if (vtime > 0)
		return 0;
	else
		return 1;
	
}

void SkipperMissile::animate(Frame *space)
{
	if (!isInvisible())
		Missile::animate(space);
}

REGISTER_SHIP(ScavengerInterloper)
