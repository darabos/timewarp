#include "../ship.h"
#include "../melee/mview.h"
REGISTER_FILE

class ArchTempest : public Ship {
public:
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;
	int			 weaponWaves;
	int			 ready_to_fire;

	double       specialRange;
	double       specialVelocity;
	int          specialDamage;
	int          specialArmour;
	double       specialTurnRate;

	public:
	ArchTempest(Vector2 pos, double angle, ShipData *data, unsigned int code);

	virtual int activate_weapon();
	virtual int activate_special();
	virtual void calculate();
	virtual void calculate_fire_weapon();
	virtual void calculate_fire_special();
	virtual void calculate_thrust();
	virtual void calculate_turn_left();
	virtual void calculate_turn_right();
	virtual void calculate_hotspots();
};

class TempestWave : public Missile 
{

public:

	ArchTempest *tempest;
	int wave_range;

  TempestWave(Vector2 pos, double oangle, double ov, int odamage,
      double orange, int oarmour, ArchTempest *oship, SpaceSprite *osprite);


  virtual void calculate();
  virtual void death();

};

ArchTempest::ArchTempest(Vector2 pos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(pos, angle, data, code)
{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  weaponWaves	 = get_config_int("Weapon", "Waves", 0);
  ready_to_fire = true;

  specialRange    = scale_range(get_config_float("Special", "Range", 0));
  specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
  specialDamage   = get_config_int("Special", "Damage", 0);
  specialArmour   = get_config_int("Special", "Armour", 0);
  specialTurnRate = scale_turning(get_config_float("Special", "TurnRate", 0));
}

void ArchTempest::calculate()
{
	STACKTRACE

	Ship::calculate();

}

int ArchTempest::activate_weapon()
{
	STACKTRACE

	if (ready_to_fire) {
		add(new TempestWave(Vector2(0.0, (size.y / 2.0)), angle,
		weaponVelocity, weaponDamage, weaponRange, weaponArmour, 
		this, data->spriteWeapon) );
		ready_to_fire = false;
	}
		
  return(TRUE);
}

int ArchTempest::activate_special()
{
	STACKTRACE

    if (target && !target->isInvisible()) //&& !fire_weapon) // Requires more skill to fly
		angle = trajectory_angle(target);

  return(TRUE);
}

void ArchTempest::calculate_fire_weapon() {
	STACKTRACE

	if (ready_to_fire)
		Ship::calculate_fire_weapon();
}

void ArchTempest::calculate_fire_special()
{
	STACKTRACE

	// This is just to override any ini changes.  Special MUST be free with a constant rate.

	if (fire_special) activate_special();
/*
    special_low = FALSE;

    if(fire_special) 
	{

      if(batt < special_drain) {
        special_low = TRUE;
        return;
	  }

	   if(special_recharge > 0)
		   return;

    if(!activate_special())
      return;

      batt -= special_drain;
      special_recharge += special_rate;

	} 
*/

}

void ArchTempest::calculate_thrust() {
	STACKTRACE

	if (thrust && !fire_special) // Thrust like normal
		accelerate_gravwhip(this, angle, accel_rate * frame_time, speed_max);

	else if (thrust && fire_special) {

		if (turn_left && !turn_right) // Activate bottom-right jet
			accelerate_gravwhip(this, angle - (PI/4), accel_rate * frame_time, speed_max);

		else if (turn_right && !turn_left) // Activate bottom-left jet
			accelerate_gravwhip(this, angle + (PI/4), accel_rate * frame_time, speed_max);

		else // Thrust like normal
			accelerate_gravwhip(this, angle, accel_rate * frame_time, speed_max);

	}

	return;
}

void ArchTempest::calculate_turn_left()
{
	STACKTRACE

	if (fire_special) {

		if (turn_left && !turn_right) // Activate right thruster
			accelerate_gravwhip(this, angle - (PI/2), accel_rate * frame_time, speed_max);

		else if (turn_left && turn_right && !thrust)
		{ vel = 0; } // Debatable

	}

	else if (turn_left)
		turn_step -= turn_rate * frame_time;

}

void ArchTempest::calculate_turn_right()
{
	STACKTRACE

	if (fire_special) {

		if (turn_right && !turn_left) // Activate left thruster
			accelerate_gravwhip(this, angle + (PI/2), accel_rate * frame_time, speed_max);

		else if (turn_right && turn_left && !thrust)
		{ vel = 0; } // Debatable

	}

	else if (turn_right)
		turn_step += turn_rate * frame_time;

}

void ArchTempest::calculate_hotspots() { // My ultra-cool hotspot code.  It 0wnz you.
	STACKTRACE

	if (hotspot_frame <= 0) {

		if (fire_special) {

			if (turn_right && !turn_left) {

				game->add(new Animation(this,
					normal_pos() - (unit_vector(angle + (PI/2)) * size.y / 2.5),
					game->hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));

				if (thrust) {

					game->add(new Animation(this,
						normal_pos() - (unit_vector(angle + (PI/4)) * size.y / 2.5),
						game->hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));

				}

				hotspot_frame += hotspot_rate;

			}

			else if (turn_left && !turn_right) {

				game->add(new Animation(this,
					normal_pos() - (unit_vector(angle - (PI/2)) * size.y / 2.5),
					game->hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));

				if (thrust) {
					game->add(new Animation(this,
						normal_pos() - (unit_vector(angle - (PI/4)) * size.y / 2.5),
						game->hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));
				}

				hotspot_frame += hotspot_rate;

			}

			else if (thrust) {

				game->add(new Animation(this,
					normal_pos() - (unit_vector(angle) * size.x / 2.5),
					game->hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));
				hotspot_frame += hotspot_rate;
			}
		}	

		else if (thrust) {
			game->add(new Animation(this,
				normal_pos() - (unit_vector(angle) * size.x / 2.5),
				game->hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));

			hotspot_frame += hotspot_rate;
		}



	}

	if (hotspot_frame > 0)
		hotspot_frame -= frame_time;

	return;
}

TempestWave::TempestWave(Vector2 pos, double oangle, double ov,
    int odamage, double orange, int oarmour,
    ArchTempest *oship, SpaceSprite *osprite) :
    Missile(oship, pos, oangle, ov, odamage, orange, oarmour, oship, osprite)
{
  //explosionSprite     = data->spriteWeaponExplosion;
  //explosionFrameCount = 5;
  //explosionFrameSize  = 50;

	tempest = oship;
	wave_range = false;

}

void TempestWave::calculate()
{
	STACKTRACE
	double shit_angle = angle * (180 / PI); // FIXME: Goddamn radians

	if ( (d / range) > (.25) )
		sprite_index = (shit_angle / 5.625) + 64;
	if ( (d / range) > (.5) )
		sprite_index = (shit_angle / 5.625) + 128;
	if ( (d / range) > (.75) )
		sprite_index = (shit_angle / 5.625) + 192;

	if (!wave_range && tempest->exists()) {

		if (d > (range / tempest->weaponWaves) ) {
			tempest->ready_to_fire = true;
			wave_range = true;
		}

	}

	Missile::calculate();

}

void TempestWave::death()
{
	STACKTRACE

	if (!wave_range && tempest->exists())
		tempest->ready_to_fire = true;

}

REGISTER_SHIP ( ArchTempest )
