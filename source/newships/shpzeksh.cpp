/* $Id$ */ 
//ZekfahanShocker lousy code by Richardyzo@ig.com.br

#include "../ship.h"
REGISTER_FILE

class ZekfahanShocker : public Ship {
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;

  double	specialRange;
  double	specialVelocity;
  int		specialDamage;
  int		specialArmour;
  int		specialFrames;
  int           shockingFrames;
  int		shockVar;

  public:
  ZekfahanShocker(Vector2 opos, double shipAngle,
		ShipData *shipData, unsigned int code);

  protected:
  virtual int activate_weapon();
  virtual int activate_special();
  virtual void calculate();

};

class Shockwave : public AnimatedShot {

  public:
  Shockwave(Vector2 oposvec, double oangle, double ov,
  int odamage, double orange, int oarmour, Ship *oship, SpaceLocation *opos,
  SpaceSprite *osprite, int ofcount, int ofsize);

  protected:
  virtual void inflict_damage(SpaceObject *other);
};

ZekfahanShocker::ZekfahanShocker(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code) 
	:
  Ship(opos, shipAngle, shipData, code)
{
  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);

  specialRange    = scale_range(get_config_float("Special", "Range", 0));
  specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
  specialDamage   = get_config_int("Special", "Damage", 0);
  specialArmour   = get_config_int("Special", "Armour", 0);
  specialFrames   = get_config_int("Special", "Frames", 0);
  shockingFrames  = 0;
  shockVar        = 0;
}

int ZekfahanShocker::activate_weapon()
{
	STACKTRACE
	play_sound2(data->sampleWeapon[0]);
  add(new AnimatedShot(this, Vector2(42.0, 15.0), angle , weaponVelocity,
    weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon, 5, 12, 1.0));
  add(new AnimatedShot(this, Vector2(-42.0, 15.0), angle , weaponVelocity,
    weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon, 5, 12, 1.0));
  return(TRUE);
}

int ZekfahanShocker::activate_special() {
	STACKTRACE
  if(shockingFrames == 0)
  {
	play_sound2(data->sampleSpecial[0]);
        shockingFrames = specialFrames;
	shockVar = 1;
	recharge_amount = 0;
  }
  return(TRUE);
}

void ZekfahanShocker::calculate()
{ 
	STACKTRACE
  if(shockVar == 1)
  {
   if(shockingFrames > 0) {
     shockingFrames -= frame_time;
     }
   if(shockingFrames % 500 == 0 ) {
     add( new FixedAnimation(this, this, data->spriteSpecial, 0, 6, 30, LAYER_EXPLOSIONS));
     }
   /*
   if(shockingFrames == 2000 ) {
	play_sound2(data->sampleSpecial[0]);
     add( new FixedAnimation(this, this, data->spriteSpecial, 0, 6, 30, LAYER_EXPLOSIONS));
     }
   if(shockingFrames == 1500 ) {
	play_sound2(data->sampleSpecial[0]);
     add( new FixedAnimation(this, this, data->spriteSpecial, 0, 6, 30, LAYER_EXPLOSIONS));
     }
   if(shockingFrames == 1000 ) {
	play_sound2(data->sampleSpecial[0]);
     add( new FixedAnimation(this, this, data->spriteSpecial, 0, 6, 30, LAYER_EXPLOSIONS));
     }
   if(shockingFrames == 500 ) {
	play_sound2(data->sampleSpecial[0]);
     add( new FixedAnimation(this, this, data->spriteSpecial, 0, 6, 30, LAYER_EXPLOSIONS));
     }
	 */
   if(shockVar && shockingFrames <= 50 )
   {
	play_sound2(data->sampleSpecial[1]);
     add( new Shockwave(Vector2(0, 70), angle , specialVelocity, specialDamage, specialRange, specialArmour, this, this, data->spriteSpecial, 2, 50));
     shockingFrames = 0;
     shockVar = 0;
     recharge_amount = 1;
     }
  }
   Ship::calculate();
}

Shockwave::Shockwave(Vector2 oposvec, double oangle, double ov,
  int odamage, double orange, int oarmour, Ship *oship, SpaceLocation *opos,
  SpaceSprite *osprite, int ofcount, int ofsize) :
  AnimatedShot(oship, oposvec, oangle, ov, odamage, orange, oarmour, oship, osprite, ofcount, ofsize)
{
  collide_flag_anyone = (ALL_LAYERS);
}

void Shockwave::inflict_damage(SpaceObject *other) {
	STACKTRACE
	play_sound2(data->sampleSpecial[2]);
	damage(other, 0, damage_factor);
	add( new Animation(this, pos, data->spriteSpecial, 0, 6, 30, LAYER_EXPLOSIONS));
	return;
	}




REGISTER_SHIP(ZekfahanShocker)
