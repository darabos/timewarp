/* $Id$ */ 
#include "../ship.h"
#include "../scp.h"
REGISTER_FILE

#include "../other/lightning.h"

/** Copy of the Korvian Sniper
Lightning taken from Narool Lurker
*/

void draw_lightning(Vector2 lightningrelpos, int sparktime, int maxsparktime, int Rmax,
					BITMAP *lightningbmp, BITMAP *shpbmp, Frame *space,
					Vector2 plot_pos, Vector2 plot_size);

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

	int cloak;
	int cloak_frame;
	

	Lightning lightning;

	public:
	static int cloak_color[3];
	ScavengerInterloper(Vector2 opos, double angle, ShipData *data, unsigned int code);

	virtual double isInvisible() const;
	virtual int activate_weapon();
	virtual int activate_special();
	virtual void calculate_hotspots();
	virtual void calculate();
	virtual void animate(Frame *space);

	// add code to activate the sparks
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


  cloak = TRUE;
	debug_id = 1009;

	BITMAP *shpbmp = sprite->get_bitmap(0);
	double maxsparktime = get_config_float("Quirk", "maxsparktime", 2000);
	double Rmax = get_config_float("Quirk", "Rmax", 1);
	lightning.init(shpbmp, 0, maxsparktime, Rmax);
}

double ScavengerInterloper::isInvisible() const
{
	if (lightning.visible())
		return 0;
	else
		return 1;
	
}

int ScavengerInterloper::activate_weapon()
{

	game->add(new ScavengerInterloperMissile( Vector2(0.0, size.y / 2.0),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));

	play_sound2(data->sampleWeapon[0]);

	return TRUE;
}


int ScavengerInterloper::activate_special()
{
	game->add(new SkipperMissile( Vector2(0.0, size.y / 2.0),
		angle, specialVelocity, specialDamage, specialRange, specialArmour,
		this, data->spriteSpecial, SkipperPeriod, VisiblePeriod, SkipperMaxAngle));

	return TRUE;
}


void ScavengerInterloper::calculate_hotspots()
{
    //Ship::calculate_hotspots();
}



void ScavengerInterloper::calculate()
{
	
	lightning.update(frame_time);
	
	Ship::calculate();
}

void ScavengerInterloper::animate(Frame *space)
{

	if (!show_red_cloaker || (control && is_bot(control->channel)) || (control && !is_local(control->channel)) || (!game_networked && num_network>1))	// bots and remote players are "hidden"
	{
#ifdef _DEBUG
		sprite->animate_character( pos, sprite_index, pallete_color[4], space);
#else
		sprite->animate_character( pos, sprite_index, pallete_color[255], space);
#endif
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

	//you've hit something; (re)activate sparks.
	lightning.reset();

	// but, where did you hit it ?
	// place the source somewhere... at the edge .. how ?
	double a, R;
	a = trajectory_angle(other);
	R = 20;
	
	lightning.locate(R * unit_vector(a));
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
