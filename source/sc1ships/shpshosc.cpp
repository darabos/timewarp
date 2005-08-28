/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

#include "../sc1ships.h"

ShofixtiScout::ShofixtiScout(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
{
  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);

  specialRange  = scale_range(get_config_float("Special", "Range", 0));
  specialScale = get_config_float("Special", "Scale", 1.0);
  specialFrames = get_config_int("Special", "Frames", 0);
  specialDamage = get_config_int("Special", "Damage", 0);
  flipSwitch    = FALSE;
  glory         = 0;
	debug_id = 1016;
}

int ShofixtiScout::activate_weapon()
{
  add(new Missile(this, Vector2(0.0, size.y / 2.0),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon));
  return(TRUE);
}

void ShofixtiScout::calculate_fire_special() {
	int gloryDamage;

	if(fire_special) {
		if(!flipSwitch) {
			flipSwitch = TRUE;
			glory++;
			if(glory == 3) {
				Query q;
				for (q.begin(this, OBJECT_LAYERS, specialRange, QUERY_OBJECT); q.currento; q.next()) {
					if (q.currento->canCollide(this)) {
						gloryDamage = (int)ceil((specialRange - distance(q.currento)) / specialRange * specialDamage);
						damage(q.current, 0, gloryDamage);
						}
					}
				q.end();
				add(new Animation(this, pos, data->spriteSpecial, 
						0, specialFrames, 50, DEPTH_EXPLOSIONS, specialScale));
				play_sound2(data->sampleSpecial[0]);
				damage(this, 0, 999);
				}
			else {
				spritePanel->overlay(1, 5+glory, spritePanel->get_bitmap(2));
				spritePanel->overlay(1, 5+glory, spritePanel->get_bitmap(3));
				spritePanel->overlay(1, 5+glory, spritePanel->get_bitmap(4));
				spritePanel->overlay(1, 5+glory, spritePanel->get_bitmap(5));
				spritePanel->overlay(1, 5+glory, spritePanel->get_bitmap(1));
/*				blit(data->bitmapPanel[5 + glory], panelBitmap[1], 0, 23, 0, 23,
						CAPTAIN_WIDTH, 7);
				blit(data->bitmapPanel[5 + glory], panelBitmap[2], 0, 23, 0, 23,
						CAPTAIN_WIDTH, 7);
				blit(data->bitmapPanel[5 + glory], panelBitmap[3], 0, 23, 0, 23,
						CAPTAIN_WIDTH, 7);
				blit(data->bitmapPanel[5 + glory], panelBitmap[4], 0, 23, 0, 23,
						CAPTAIN_WIDTH, 7);
				blit(data->bitmapPanel[5 + glory], panelBitmap[5], 0, 23, 0, 23,
						CAPTAIN_WIDTH, 7);*/
				}
			}
		}
	else flipSwitch = FALSE;
	return;
	}



REGISTER_SHIP ( ShofixtiScout )
