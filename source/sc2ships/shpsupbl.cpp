/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

#include "../sc2ships.h"

SupoxBlade::SupoxBlade(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)
{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
	debug_id = 1043;
}

void SupoxBlade::calculate_thrust()
{

  if(thrust || ((fire_special) && (turn_left || turn_right || thrust))) {
    if((fire_special) && (turn_left)) {
			accelerate_gravwhip(this, angle-PI/2, accel_rate * frame_time, speed_max);
    }
    if((fire_special) && (turn_right)) {
			accelerate_gravwhip(this, angle+PI/2, accel_rate * frame_time, speed_max);
    }
    if((fire_special) && (thrust)) {
			accelerate_gravwhip(this, angle+PI, accel_rate * frame_time, speed_max);
    }
    if((!fire_special) && (thrust)) {
			accelerate_gravwhip(this, angle, accel_rate * frame_time, speed_max);
    }

  }
}

void SupoxBlade::calculate_turn_left()
{
  if(!fire_special)
    Ship::calculate_turn_left();
}

void SupoxBlade::calculate_turn_right()
{
  if(!fire_special)
    Ship::calculate_turn_right();
}

void SupoxBlade::calculate_hotspots()
{
  if(!fire_special)
    Ship::calculate_hotspots();
}

int SupoxBlade::activate_weapon()
{
  add(new Missile(this, Vector2(0.0, 0.5*get_size().y / 2.0),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon));
  return(TRUE);
}

void SupoxBlade::calculate_fire_special() {}



REGISTER_SHIP(SupoxBlade)
