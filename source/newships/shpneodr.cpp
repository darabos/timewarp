

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"


class NeoggDrain : public Ship
{
	double	weaponRange, weaponVelocity, weaponDamage, weaponArmour;
	double	specialRange, specialDamage;
	int		specialColor;
	
	bool shipmode;
	
public:
	NeoggDrain(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);
	
protected:
	
	virtual int activate_weapon();
	virtual int activate_special();
	
};

class LaserDrain : public Laser
{
	void inflict_damage(SpaceObject *other);
public:
	LaserDrain(SpaceLocation *creator, double langle, int lcolor, double lrange, double ldamage,
  int lfcount, SpaceLocation *opos, Vector2 rpos, bool osinc_angle);
};




NeoggDrain::NeoggDrain(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	
	specialColor  = get_config_int("Special", "Color", 0);
	specialRange  = scale_range(get_config_float("Special", "Range", 0));
	specialDamage = get_config_int("Special", "Damage", 0);
	
}


int NeoggDrain::activate_weapon()
{	

	add(new Missile(this, Vector2(36,20), angle,
		weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon, 0.0));

	return TRUE;
}


int NeoggDrain::activate_special()
{

	add(new LaserDrain(this, angle,
		pallete_color[specialColor], specialRange, specialDamage, special_rate,
		this, Vector2(-10,42), true));
	
	return TRUE;
}






LaserDrain::LaserDrain(SpaceLocation *creator, double langle, int lcolor, double lrange, double ldamage,
  int lfcount, SpaceLocation *opos, Vector2 rpos, bool osinc_angle)
:
Laser(creator, langle, lcolor, lrange, ldamage, lfcount, opos, rpos, osinc_angle)
{
	// nother extra needed.
}

void LaserDrain::inflict_damage(SpaceObject *other)
{
	other->handle_fuel_sap(this, damage_factor);
}



REGISTER_SHIP(NeoggDrain)
