#include "../ship.h"
REGISTER_FILE
#include "../melee/mcbodies.h"

#include "../sc2ships.h"

PkunkFury::PkunkFury(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)
{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);

	reborn = 0;
}

int PkunkFury::handle_damage(SpaceLocation *source, double normal, double direct) {
	double r = normal + direct;
	crew -= r;
	if (crew > 0) return r;

	play_sound((SAMPLE *)(melee[MELEE_BOOMSHIP].dat));
	game->add(new Animation(this, pos,	game->kaboomSprite, 0, KABOOM_FRAMES, time_ratio, DEPTH_EXPLOSIONS));

	if (random() % 2) {
		if (attributes & ATTRIB_NOTIFY_ON_DEATH){
			game->ship_died(this, source);
			attributes &= ~ATTRIB_NOTIFY_ON_DEATH;
		}
		die();
		return r;
	}

	pos = random(Vector2(3000,3000)) - Vector2(1500,1500);
	SpaceLocation *spacePlanet = nearest_planet();
	if (spacePlanet && (distance(spacePlanet) < 1000.0)) {
//		x += cos(trajectory_angle(spacePlanet)) * 1000.0;
//		y += sin(trajectory_angle(spacePlanet)) * 1000.0;
		pos += 1000.0 * unit_vector(trajectory_angle(spacePlanet));
		}
	angle = random(PI2);
	sprite_index = get_index(angle);
//	vx = vy = 0.0;
	vel = 0;
	crew = crew_max;
	batt = batt_max;
	reborn = TRUE;
	update_panel = TRUE;
	play_sound(data->sampleExtra[0]);
	game->remove(this);
	add(new Phaser (this, 
//			x - cos(angle+0) * PHASE_MAX * w, 
//			y - sin(angle+0) * PHASE_MAX * h, 
			pos - PHASE_MAX * product(unit_vector(angle+0), get_size()),
//			cos(angle+0) * PHASE_MAX * w, 
//			sin(angle+0) * PHASE_MAX * h, 
			PHASE_MAX * product(unit_vector(angle+0), get_size()),
			this, sprite, sprite_index, hot_color, HOT_COLORS, 
			PHASE_DELAY, PHASE_MAX, PHASE_DELAY) );
	add(new Phaser (this, 
//			x - cos(angle+PI/2) * PHASE_MAX * w, 
//			y - sin(angle+PI/2) * PHASE_MAX * h, 
			pos - PHASE_MAX * product(unit_vector(angle+PI/2), get_size()),
//			cos(angle+PI/2) * PHASE_MAX * w, 
//			sin(angle+PI/2) * PHASE_MAX * h, 
			PHASE_MAX * product(unit_vector(angle+PI/2), get_size()),
			NULL, sprite, (sprite_index+0)&63, hot_color, HOT_COLORS, 
			PHASE_DELAY, PHASE_MAX, PHASE_DELAY) );
	add(new Phaser (this, 
//			x - cos(angle-PI/2) * PHASE_MAX * w, 
//			y - sin(angle-PI/2) * PHASE_MAX * h, 
			pos - PHASE_MAX * product(unit_vector(angle-PI/2), get_size()),
//			cos(angle-PI/2) * PHASE_MAX * w, 
//			sin(angle-PI/2) * PHASE_MAX * h, 
			PHASE_MAX * product(unit_vector(angle-PI/2), get_size()),
			NULL, sprite, (sprite_index-0)&63, hot_color, HOT_COLORS, 
			PHASE_DELAY, PHASE_MAX, PHASE_DELAY) );
	return r;
  }

int PkunkFury::activate_weapon()
{
  add(new AnimatedShot(this, 
    Vector2(0.0, (get_size().y / 2.0)), angle, weaponVelocity, weaponDamage, weaponRange,
    weaponArmour, this, data->spriteWeapon, 10, 1, 1.0));
  add(new AnimatedShot(this, 
    Vector2(-(get_size().x / 2.0), 0.0), angle - PI/2, weaponVelocity, weaponDamage,
    weaponRange, weaponArmour, this, data->spriteWeapon, 10, 1, 1.0));
  add(new AnimatedShot(this, 
    Vector2(get_size().x / 2.0, 0.0), angle + PI/2, weaponVelocity, weaponDamage,
    weaponRange, weaponArmour, this, data->spriteWeapon, 10, 1, 1.0));

  return(TRUE);
}

void PkunkFury::calculate_fire_special()
{

  if(fire_special) {
    if((special_recharge > 0) || (batt >= batt_max))
      return;

    batt += special_drain;
    if(batt > batt_max)
      batt = batt_max;

    special_recharge = special_rate;

    //sound.stop(data->sampleSpecial[special_sample]);

    special_sample = (special_sample + 1 + random(13)) % 14;
	play_sound2(data->sampleSpecial[special_sample]);
  }
}



REGISTER_SHIP(PkunkFury)
