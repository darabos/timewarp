#include "../ship.h"
REGISTER_FILE



class Viogen : public Ship
{
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;
	
public:
	Viogen(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);
	  
protected:
	virtual int activate_weapon();
	virtual int activate_special();
};



class ViogenMissile : public Missile
{
public:

	double	period, rotatetime, accel, aoffs;

	ViogenMissile(Vector2 opos, double oaoffs, double oangle, double ov,
		int odamage, double orange, int oarmour, Ship *oship,
		SpaceSprite *osprite);

	virtual void calculate();
};



Viogen::Viogen(Vector2 opos, double shipAngle,
			   ShipData *shipData, unsigned int code) 
			   :
Ship(opos, shipAngle, shipData, code) {
	
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	
}



int Viogen::activate_weapon()
{
	STACKTRACE
	add(new ViogenMissile(
		Vector2( 20.0, 0.5*get_size().y), -0.5*PI, angle, weaponVelocity, weaponDamage, weaponRange,
		weaponArmour, this, data->spriteWeapon));

	add(new ViogenMissile(
		Vector2(-20.0, 0.5*get_size().y), 0.5*PI, angle, weaponVelocity, weaponDamage, weaponRange,
		weaponArmour, this, data->spriteWeapon));

	return(TRUE);
}



int Viogen::activate_special()
{
	STACKTRACE

	if (batt - special_drain > batt_max)
		return false;

	double a, v, dv;
	v = vel.length();
	a = vel.atan();

	dv = 2 * speed_max / batt_max;
	// full speed gives half batt.

	if (v - dv < 0)
		return false;

	
	v -= dv;
	vel = v * unit_vector(a);
	
	
	return true;
}



ViogenMissile::ViogenMissile(Vector2 opos, double oaoffs, double oangle,
							 double ov, int odamage, double orange, int oarmour,
							 Ship *oship, SpaceSprite *osprite) :
Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship, osprite)
{
	period = 1.0;
	rotatetime = 0;

	accel = scale_acceleration(7);

	aoffs = oaoffs;
}


void ViogenMissile::calculate()
{
	STACKTRACE
	Missile::calculate();

	if (!state)
		return;

	// sorta rotate ?
	Vector2 normal;
	normal = Vector2(-vel.y, vel.x);
	normalize(normal);

	rotatetime += frame_time * 1E-3;
	
	double a;
	a = sin(PI2 * rotatetime / period + aoffs);

	vel += (accel*frame_time) * a * normal;
	angle = vel.atan();

	// release some smoke trail.
	Vector2 relpos;

	relpos = Vector2( random(10.0)-5.0, -40.0 + random(10.0) );
	Animation *anim;
	anim = new Animation(this, pos + rotate(relpos, angle-0.5*PI),
					data->spriteSpecial, 0, data->spriteSpecial->frames(),
					100, LAYER_HOTSPOTS);
	add(anim);

//Animation::Animation(SpaceLocation *creator, Vector2 opos, 
//	SpaceSprite *osprite, int first_frame, int num_frames, int frame_length, 
//	double depth, double _scale) 
}




REGISTER_SHIP(Viogen)

