/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

#include "../sc1ships.h"

int IlwrathAvenger::cloak_color[3] = { 15, 11, 9 };

IlwrathAvenger::IlwrathAvenger(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
{
  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);

  cloak = FALSE;
  cloak_frame = 0;
	debug_id = 1009;
}

double IlwrathAvenger::isInvisible() const {
	if(cloak_frame >= 300)
		return 1.0;
	return false;
	}

int IlwrathAvenger::activate_weapon() {
	// note that target=0 is only set after this routine is called in ship::calculate
	// so we need to check if it exists ...
	if (cloak && target && target->exists()) {
		if (distance(target) < weaponRange * 3) {
			angle = 
				intercept_angle2(pos, vel * 1.0, weaponVelocity, 
					target->normal_pos(), target->get_vel() );
			}
		else angle = trajectory_angle(target);
		}
	cloak = FALSE;
	game->add(new AnimatedShot(this, Vector2(0.0, size.y / 2.0),
			angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour, 
			this, data->spriteWeapon, 50, 12, 1.0));
	return(TRUE);
	}

void IlwrathAvenger::calculate_fire_special()
{
  special_low = FALSE;

  if(fire_special) {
    if((batt < special_drain) && (!cloak)) {
      special_low = TRUE;
      return;
    }

    if(special_recharge > 0)
      return;

    if(cloak) {
      cloak = FALSE;

      play_sound2(data->sampleSpecial[1]);
    } else {
      cloak = TRUE;
      play_sound2(data->sampleSpecial[0]);
      batt -= special_drain;
    }

    special_recharge = special_rate;
  }
}

void IlwrathAvenger::calculate_hotspots()
{
  if(!cloak)
    Ship::calculate_hotspots();
}

void IlwrathAvenger::calculate()
{
  if((cloak) && (cloak_frame < 300))
    cloak_frame += frame_time;
  if((!cloak) && (cloak_frame > 0))
    cloak_frame -= frame_time;

  Ship::calculate();
}

void IlwrathAvenger::animate(Frame *space) {
	if((cloak_frame > 0) && (cloak_frame < 300))
		sprite->animate_character( pos, sprite_index, 
				pallete_color[cloak_color[cloak_frame / 100]], space);
	else if ((cloak_frame >= 300)) {
		sprite->animate_character( pos, sprite_index, pallete_color[255], space);
		}
	else Ship::animate(space);		
	return;
	}

REGISTER_SHIP(IlwrathAvenger)
