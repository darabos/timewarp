#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

#include "../sc2ships.h"

UtwigJugger::UtwigJugger(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)
{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  fortitude = 0;
	debug_id = 1046;
}

void UtwigJugger::calculate()
{
        Ship::calculate();
        if (!fire_special)
                special_recharge = 0;
}

void UtwigJugger::calculate_fire_weapon()
{
  weapon_low = FALSE;

  if(fire_weapon) {
    if((weapon_recharge > 0) || (special_recharge > 0))
      return;
    if (fire_special && batt) return;
    
    add(new Missile(this, Vector2(34.0, 11.0),
      angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
      this, data->spriteWeapon));
    add(new Missile(this, Vector2(-34.0, 11.0),
      angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
      this, data->spriteWeapon));
    add(new Missile(this, Vector2(18.0, 20.0),
      angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
      this, data->spriteWeapon));
    add(new Missile(this, Vector2(-18.0, 20.0),
      angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
      this, data->spriteWeapon));
    add(new Missile(this, Vector2(6.0, 27.0),
      angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
      this, data->spriteWeapon));
    add(new Missile(this, Vector2(-6.0, 27.0),
      angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
      this, data->spriteWeapon));

    weapon_recharge = weapon_rate;

    play_sound2(data->sampleWeapon[0]);
  }
}

void UtwigJugger::animate(Frame *space) {
	if (special_recharge > 0) {
		sprite->animate_character(pos, sprite_index, pallete_color[hot_color[((int)(special_recharge/10)) % HOT_COLORS]], space);
		}
	else sprite->animate( pos, sprite_index, space);
	return;
}

int UtwigJugger::handle_damage(SpaceLocation *source, double normal, double direct) {
	if (special_recharge > 0) {
		batt += normal;
		if (batt > batt_max) batt = batt_max;
		normal = 0;
	}
	if (fortitude && (normal > 0)) {
		if (normal > random(3)) normal -= fortitude;
		if (normal < 0) normal = 0;
	}
	return Ship::handle_damage(source, normal, direct);
}

double UtwigJugger::isProtected() const
{
	return (special_recharge > 0) ? 1 : 0;
}



REGISTER_SHIP(UtwigJugger)
