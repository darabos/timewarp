#include "../ship.h"
REGISTER_FILE

#include "../sc1ships.h"

class MyconPlasma : public HomingMissile {
	//double v;

	static SpaceSprite *spriteWeaponExplosion;
	int frame_count;
	int max_damage;

	public:
	MyconPlasma(Vector2 opos, double oangle, double ov, int odamage,
		double orange, double otrate, Ship *oship, SpaceSprite *osprite, int ofcount);

	virtual void calculate();

	virtual void inflict_damage(SpaceObject *other);
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};


MyconPodship::MyconPodship(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
{
  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponHome     = scale_turning(get_config_float("Weapon", "Homing", 0));
  specialRepair  = get_config_int("Special", "Repair", 0);
}

int MyconPodship::activate_weapon()
{
  add(new MyconPlasma(Vector2(0.0, size.y),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponHome, this,
    data->spriteWeapon, 64));
  return(TRUE);
}

int MyconPodship::activate_special()
{
  if(crew >= crew_max)
    return(FALSE);
  damage(this, 0, -4);
  return(TRUE);
}

SpaceSprite *MyconPlasma::spriteWeaponExplosion = NULL;
MyconPlasma::MyconPlasma(Vector2 opos, double oangle, double ov,
    int odamage, double orange, double otrate, Ship *oship,
    SpaceSprite *osprite, int ofcount) :
  HomingMissile( oship, opos, oangle, ov, odamage, orange, 0, otrate, oship, 
      osprite, oship->target),
//  v(ov),
  frame_count(ofcount),
  max_damage(odamage)
{
	spriteWeaponExplosion = data->spriteWeaponExplosion;
	collide_flag_sameship |= bit(LAYER_SHIPS);
	collide_flag_sameteam |= bit(LAYER_SHIPS);
}

void MyconPlasma::calculate()
{
  HomingMissile::calculate();

  sprite_index = (int)((d / range) * (double)(frame_count - 1));
  damage_factor = max_damage - (int)((d / range) * (double)(max_damage));
}

void MyconPlasma::inflict_damage(SpaceObject *other)
{
  SpaceObject::inflict_damage(other);
  if (!other->isShot()) {
	  if (other->exists()) {
		  add(new FixedAnimation(this, other,
				spriteWeaponExplosion, 0, 20, 50, DEPTH_EXPLOSIONS));
	  }
	  else {
		  add(new Animation(this, other->normal_pos(), 
				spriteWeaponExplosion, 0, 20, 50, DEPTH_EXPLOSIONS));
	  }
    state = 0;
  }
}

int MyconPlasma::handle_damage(SpaceLocation *source, double normal, double direct) {
	double total = normal + direct;

	if (total) {
		total += normal;
		total += direct;
		d += total / max_damage * range;
		if (d >= range) state = 0;
		}
	return 1;
	}


REGISTER_SHIP(MyconPodship)
