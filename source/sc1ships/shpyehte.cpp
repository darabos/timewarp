#include "ship.h"
REGISTER_FILE

#include "frame.h"

#include "sc1ships.h"

YehatTerminator::YehatTerminator(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);

  specialFrames = get_config_int("Special", "Frames", 0);
  shieldFrames  = 0;
}

int YehatTerminator::activate_weapon()
{
  add(new Missile(this, Vector2(-24.0, 14),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon));
  add(new Missile(this, Vector2(24.0, 14),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon));
  return(TRUE);
}

int YehatTerminator::activate_special()
{
  shieldFrames = (shieldFrames % frame_time) + specialFrames;
  return(TRUE);
}

void YehatTerminator::calculate()
{
   if(shieldFrames > 0)
     shieldFrames-= frame_time;
   Ship::calculate();
}

void YehatTerminator::animate(Frame *space)
{
  if(shieldFrames > 0)
    data->spriteSpecial->animate( pos, sprite_index, space);
  else
    sprite->animate( pos, sprite_index, space);
}

int YehatTerminator::handle_damage(SpaceLocation *source, double normal, double direct) {
	if (shieldFrames > 0) normal = 0;
	return Ship::handle_damage(source, normal, direct);
	}

double YehatTerminator::isProtected() const
{
  return (shieldFrames > 0);
}


REGISTER_SHIP (YehatTerminator)
