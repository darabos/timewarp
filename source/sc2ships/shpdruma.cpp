#include "../ship.h"
REGISTER_FILE


#include "../sc2ships.h"

class DruugeMissile : public Missile {
  public:
  DruugeMissile(Vector2 opos, double oangle, double ov, int odamage,
      double weaponDriftVelocity, double orange, int oarmour, Ship *oship,
      SpaceSprite *osprite);
  virtual void inflict_damage (SpaceObject *other);
  double kick;
};

DruugeMauler::DruugeMauler(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code) {

  weaponRange         = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity      = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage        = get_config_int("Weapon", "Damage", 0);
  weaponArmour        = get_config_int("Weapon", "Armour", 0);
  weaponDriftVelocity = scale_velocity(get_config_float("Weapon", "DriftVelocity", 0));
}

int DruugeMauler::activate_weapon() {
	accelerate (this, angle + PI, weaponDriftVelocity / mass, MAX_SPEED);
	add(new DruugeMissile(
			Vector2(0.0, (get_size().y /*height()*/ / 2.0)), angle, weaponVelocity, weaponDamage,
			weaponDriftVelocity, weaponRange, weaponArmour, this,
			data->spriteWeapon));
	return(TRUE);
	}

void DruugeMauler::calculate_fire_special() {
  if((fire_special) && (crew > 1) && (batt < batt_max) &&
    (special_recharge <= 0)) {

    batt += special_drain;
    if(batt > batt_max)
      batt = batt_max;
    crew--;

    special_recharge = special_rate;

    play_sound2(data->sampleSpecial[0]);
  }
}

DruugeMissile::DruugeMissile(Vector2 opos, double oangle, double ov,
    int odamage, double weaponDriftVelocity, double orange, int oarmour,
    Ship *oship, SpaceSprite *osprite) :
    Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship, osprite),
    kick(weaponDriftVelocity)
{
  explosionSprite     = data->spriteWeaponExplosion;
  explosionFrameCount = 5;
  explosionFrameSize  = 50;
}
void DruugeMissile::inflict_damage (SpaceObject *other) {
//	if (other->getID() == SPACE_PLANET) other->accelerate (this, angle, kick/25., GLOBAL_MAXSPEED);
	if (other->mass) 
		other->accelerate (this, angle, kick / other->mass, MAX_SPEED);
	Missile::inflict_damage(other);
	}



REGISTER_SHIP(DruugeMauler)
