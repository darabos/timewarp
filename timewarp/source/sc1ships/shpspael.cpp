#include "../ship.h"
REGISTER_FILE

#include "../sc1ships.h"

SpathiEluder::SpathiEluder(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
{

	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);

	specialRange    = scale_range(get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialDamage   = get_config_int("Special", "Damage", 0);
	specialArmour   = get_config_int("Special", "Armour", 0);
	specialTurnRate = scale_turning(get_config_float("Special", "TurnRate", 0));
}

int SpathiEluder::activate_weapon()
{
	add(new Missile( this, Vector2(0.0, size.y / 2.0),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));
	return(TRUE);
}

int SpathiEluder::activate_special()
{
	SpaceLocation *tmp = new HomingMissile( this, 
		Vector2(0.0, -size.y / 1.5), angle + PI, specialVelocity, specialDamage, specialRange, 
		specialArmour, specialTurnRate, this, data->spriteSpecial, target);
		tmp->collide_flag_sameship |= bit(LAYER_SHIPS);
		tmp->collide_flag_sameteam |= bit(LAYER_SHIPS);
	add(tmp);
	return(TRUE);
}




REGISTER_SHIP ( SpathiEluder )
