/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

#include "../sc2ships.h"

class ThraddashFlame : public Animation {
  int armour;

  public:
  ThraddashFlame(Vector2 opos, int odamage, int oarmour, Ship *oship,
    SpaceSprite *osprite, int ofcount, int ofsize);

  virtual void calculate();
  virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

ThraddashTorch::ThraddashTorch(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)
{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);

  specialThrust = scale_velocity(get_config_float("Special", "Thrust", 0));
  specialDamage = get_config_int("Special", "Damage", 0);
  specialArmour = get_config_int("Special", "Armour", 0);
	debug_id = 1044;
}

int ThraddashTorch::activate_weapon() {
  add(new Missile(this, Vector2(0.0, 0.5*get_size().y),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon));
  return(TRUE);
}

int ThraddashTorch::activate_special() {
	accelerate(this, angle, specialThrust, MAX_SPEED);
  add(new ThraddashFlame(
//    x ,//- cos(angle) * (width() / 2.0),
//    y ,//- sin(angle) * (height() / 2.0),
	pos - unit_vector(angle) * size.x/2.5,
    specialDamage, specialArmour, this, data->spriteSpecial, 39, 100));
  return(TRUE);
}

void ThraddashTorch::calculate_thrust()
{
	if(special_recharge <= 0) Ship::calculate_thrust();
}

ThraddashFlame::ThraddashFlame(Vector2 opos, int odamage, int oarmour,
  Ship *oship, SpaceSprite *osprite, int ofcount, int ofsize) :
  Animation(oship, opos, osprite, 0, ofcount, ofsize, DEPTH_SHOTS),
  armour(oarmour)
{
	  layer = LAYER_SHOTS;
	collide_flag_anyone = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
  damage_factor = odamage;

  attributes &= ~ATTRIB_UNDETECTABLE;

  if(!(ship && ship->exists()))
  {
	  ship = 0;
    state = 0;
  }
	debug_id = 1045;
}

void ThraddashFlame::calculate()
{
	Animation::calculate();
}

int ThraddashFlame::handle_damage(SpaceLocation *source, double normal, double direct) {
	int total = iround(normal + direct);
	armour -= total;

	if ((armour <= 0) || (source->isObject() && ((SpaceObject*)source)->mass)) {
		state = 0;
		add(new Animation(this, normal_pos(), //normal_x(), normal_y(), 
				meleedata.sparkSprite, 0, SPARK_FRAMES, 50, DEPTH_EXPLOSIONS));
		}
	return total;
	}



REGISTER_SHIP(ThraddashTorch)
