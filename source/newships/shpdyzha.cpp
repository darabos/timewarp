#include "../ship.h"
REGISTER_FILE

//#include "../sc1ships.h"

#include "../melee.h"
#include "../melee/mshot.h"
#include "../melee/mship.h"
#include "../melee/manim.h"


class DyzunHarbringer;

class DyzunMissile : public HomingMissile {
	public:
  DyzunMissile(DyzunHarbringer* ocreator, double ox, double oy, double oangle, double ov,
	int odamage, double orange, int oarmour, double oturnrate, SpaceLocation* opos, SpaceSprite* osprite, SpaceObject* otarget);
	Ship* creator;
  int lifetimeTimer;
	double relativity;
	double facingAngle;
	double framesToIgnition;
  int isActivated;
	int burnFrames1;
  int burnFrames2;
	int coastFrames;
	double acceleration;
  int wasCoasting;
  int thrustOn;
  double coastVelocity;
  double burnVelocity1;
  double burnVelocity2;
  void thrust_on(void);
	virtual void calculate(void);
  virtual void inflict_damage(SpaceObject *other);
	};


class DyzunHarbringer : public Ship {
public:
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;
  int          weaponTandemFire;

  double       specialVelocityCoast;
  double       specialVelocityBurn1;
  double       specialVelocityBurn2;
  int          specialDamage;
  int          specialArmour;
  double       specialReleaseAngle;
  double       specialReleaseFacingAngle;
  double       specialTurnRate;

  int          specialCoastFrames;
  int          specialBurnFrames1;
  int          specialBurnFrames2;

  int          gunToFire;

  public:
  DyzunHarbringer(Vector2 opos, double angle, ShipData *data, unsigned int code);

  protected:
  virtual void calculate();
  virtual int activate_weapon();
  virtual int activate_special();
  virtual void death();
};


DyzunHarbringer::DyzunHarbringer(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
	{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  weaponTandemFire = get_config_int("Weapon", "TandemFire", 0);

  specialVelocityCoast = scale_velocity(get_config_float("Special", "VelocityCoast", 5));
  specialVelocityBurn1 = scale_velocity(get_config_float("Special", "VelocityBurn1", 10));
  specialVelocityBurn2 = scale_velocity(get_config_float("Special", "VelocityBurn2", 20));

  specialDamage   = get_config_int("Special", "Damage", 0);
  specialArmour   = get_config_int("Special", "Armour", 0);
  specialReleaseAngle = get_config_float("Special", "ReleaseAngle", 0) * ANGLE_RATIO;
  specialReleaseFacingAngle = get_config_float("Special", "ReleaseFacingAngle", 0) * ANGLE_RATIO;
  specialTurnRate = scale_turning(get_config_float("Special", "TurnRate", 0));

  specialCoastFrames = get_config_int("Special", "CoastFrames", 0);
  specialBurnFrames1 = get_config_int("Special", "BurnFrames1", 0);
  specialBurnFrames2 = get_config_int("Special", "BurnFrames2", 0);
  gunToFire = 1;
 }

void DyzunHarbringer::death(void) {
  Ship::death();
}

int DyzunHarbringer::activate_weapon() {
  if(gunToFire==1 || weaponTandemFire) {
  game->add(new Missile(this, Vector2(size.y*(0.24), (size.y * +0.25)),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon));
  }
  if(gunToFire==2 || weaponTandemFire) {
  game->add(new Missile(this, Vector2(size.y*(-0.24), (size.y * +0.25)),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon));
  }
  if(gunToFire==1)gunToFire=2;
  else gunToFire=1;
  return(TRUE);
}

int DyzunHarbringer::activate_special() {
  DyzunMissile* NM;
  NM = new DyzunMissile(this, size.y*(0.25), (size.y * 0.6),
    angle+specialReleaseAngle, specialVelocityCoast, specialDamage, -1, specialArmour, specialTurnRate,
    this, data->spriteSpecial, this->target);
  NM->facingAngle = angle+specialReleaseFacingAngle;
  NM->creator=this;
  NM->coastFrames = specialCoastFrames;
  NM->burnFrames1 = specialBurnFrames1;
  NM->burnFrames2 = specialBurnFrames2;

  NM->coastVelocity = specialVelocityCoast;
  NM->burnVelocity1 = specialVelocityBurn1;
  NM->burnVelocity2 = specialVelocityBurn2;
  game->add(NM);

  NM = new DyzunMissile(this, size.y*(-0.25), (size.y * 0.6),
    angle-specialReleaseAngle, specialVelocityCoast, specialDamage, -1, specialArmour, specialTurnRate,
    this, data->spriteSpecial, this->target);
  NM->facingAngle = angle-specialReleaseFacingAngle;
  NM->creator=this;
  NM->coastFrames = specialCoastFrames;
  NM->burnFrames1 = specialBurnFrames1;
  NM->burnFrames2 = specialBurnFrames2;
  NM->coastVelocity = specialVelocityCoast;
  NM->burnVelocity1 = specialVelocityBurn1;
  NM->burnVelocity2 = specialVelocityBurn2;
  game->add(NM);

  return(TRUE);
}

void DyzunHarbringer::calculate(void) {
  Ship::calculate();
}


DyzunMissile::DyzunMissile(DyzunHarbringer* ocreator, double ox, double oy, double oangle, double ov,
	int odamage, double orange, int oarmour, double oturnrate, SpaceLocation* opos, SpaceSprite* osprite, SpaceObject* otarget) 
	:
	HomingMissile(ocreator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oturnrate, opos, osprite, otarget) 

	{
	explosionSprite     = data->spriteWeaponExplosion;
	facingAngle=oangle;
  isActivated=0;
  this->relativity = 1.0;
  thrustOn = FALSE;
  lifetimeTimer = 0;
	}

void DyzunMissile::calculate(void) {
  lifetimeTimer += frame_time;
  if(lifetimeTimer<coastFrames) {
    wasCoasting = TRUE;
    Shot::calculate();
  }
  else if(lifetimeTimer<(coastFrames+burnFrames1)) {
  	this->sprite_index = get_index(this->angle) + 64;
    HomingMissile::calculate();
  	this->sprite_index = get_index(this->angle) + 64;
    this->v = burnVelocity1;
    this->relativity = 0.0;
  	this->sprite_index = get_index(this->angle) + 64;
  }
  else if(lifetimeTimer<(coastFrames+burnFrames1+burnFrames2)) {
  	sprite_index = get_index(this->angle) + 128;
    HomingMissile::calculate();
  	this->sprite_index = get_index(this->angle) + 128;
    this->v =  burnVelocity2;
    this->relativity = 0.0;
  	this->sprite_index = get_index(this->angle) + 128;
  }
  else {
    state = 0;
  	this->sprite_index = get_index(this->angle) + 128;
    HomingMissile::calculate();
  }
}

void DyzunMissile::inflict_damage(SpaceObject *other) {
  Shot::inflict_damage(other);
}

void DyzunMissile::thrust_on(void) {
;}

REGISTER_SHIP ( DyzunHarbringer )
