
/*
This is a variation on the Xchagger exclave, to see if the confusion
concept can be used in a different version of gameplay.
*/

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class IceciConfusion : public Ship {
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  double       weaponArmour, weaponTurnrate;

  double       specialRange;
  double       specialVelocity;
  double       specialArmour;
  int          specialFrames;
  double		specialTurnRate;

  double		confusionLifeTime;

  public:
  IceciConfusion(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  protected:
  virtual int activate_weapon();
  virtual int activate_special();
};


class Confusionator : public Presence
{
	Ship	*t;
	double	lifetime;
	int		key_order[5];
	int		key_flags[5];

public:
	Confusionator(Ship *target, double olifetime);
	virtual void calculate();
};



class ConfusionDart : public HomingMissile {

	double confusionLifeTime;

  public:
  ConfusionDart(Vector2 opos, double oangle,
  double ov, int oframes, double orange, double oarmour, double otrate, Ship *oship,
  SpaceSprite *osprite, double confusionLifeTime);

  void inflict_damage(SpaceObject *other);

};



IceciConfusion::IceciConfusion(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos,  shipAngle, shipData, code)
{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  weaponTurnrate = scale_turning(get_config_int("Weapon", "TurnRate", 0));

  specialRange    = scale_range(get_config_float("Special", "Range", 0));
  specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
  specialArmour   = get_config_int("Special", "Armour", 0);
  specialFrames   = scale_frames(get_config_int("Special", "Frames", 0));
  specialTurnRate = scale_turning(get_config_float("Special", "TurnRate", 0));

  confusionLifeTime = get_config_float("Confusion", "LifeTime", 0);

}


//	HomingMissile(SpaceLocation *creator, Vector2 rpos, double oangle, double ov, double odamage,
//			double orange, double oarmour, double otrate, SpaceLocation *opos,
//			SpaceSprite *osprite, SpaceObject *target);
int IceciConfusion::activate_weapon()
{
	STACKTRACE

	double da = 60 * ANGLE_RATIO;

	/*
	add(new Missile(this, Vector2(16.0, 0.0), angle+da,
		weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon, 1.0));
	*/
	
	add(new HomingMissile(this, Vector2(0.0, 16.0), angle,
		weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		weaponTurnrate,
		this, data->spriteWeapon, target));

  return(TRUE);
}


int IceciConfusion::activate_special()
{
	STACKTRACE

	double da = 60 * ANGLE_RATIO;

	add( new ConfusionDart(Vector2(0.0, 16.0), angle+da,
		specialVelocity, specialFrames, specialRange, specialArmour,
		specialTurnRate, this, data->spriteSpecial, confusionLifeTime));

	add( new ConfusionDart(Vector2(0.0, 16.0), angle-da,
		specialVelocity, specialFrames, specialRange, specialArmour,
		specialTurnRate, this, data->spriteSpecial, confusionLifeTime));

	
	return(TRUE);
}



ConfusionDart::ConfusionDart(Vector2 opos, double oangle,
  double ov, int oframes, double orange, double oarmour, double otrate, Ship *oship,
  SpaceSprite *osprite, double oLifeTime)
:
HomingMissile (oship, opos,  oangle, ov, 0, orange, oarmour, otrate, oship, osprite,
			   oship->target)
{
	confusionLifeTime = oLifeTime;
}

void ConfusionDart::inflict_damage(SpaceObject *other)
{
	STACKTRACE
	
    if ( other->isShip() )
	{
		game->add(new Confusionator((Ship*)other, confusionLifeTime));
	}

    state = 0;
	return;
}


Confusionator::Confusionator(Ship *target, double olifetime)
{
	t = target;
	lifetime = olifetime;

	// new key ordering:

	int key_available[5];

	key_flags[0] = keyflag::left;
	key_flags[1] = keyflag::right;
	key_flags[2] = keyflag::thrust;
	key_flags[3] = keyflag::fire;
	key_flags[4] = keyflag::special;

	int i, k;
	for ( k = 0; k < 5; ++k )
		key_available[k] = k;

	for ( i = 0; i < 5; ++i )
	{
		k = random() % (5-i);
		key_order[i] = key_available[k];
		key_available[k] = key_available[5-i-1];
	}
}


void Confusionator::calculate()
{
	STACKTRACE
	lifetime -= frame_time * 1E-3;	// in seconds

	if ( !(t && t->exists()) )
	{
		t = 0;
		state = 0;
		return;
	}

	if ( lifetime < 0 )
	{
		state = 0;
		return;
	}

	// randomize the (most) important keys ... i.e., randomize the bits in the KeyCode ?!

	// how ?
	KeyCode newkeys;

	newkeys = 0;

	int i;
	for ( i = 0; i < 5; ++i )
		newkeys |= t->nextkeys & key_flags[ key_order[i] ];

	t->nextkeys = newkeys;
	if (t->control)
		t->control->keys = newkeys;

}



REGISTER_SHIP(IceciConfusion)

