#include "ship.h"
#include "melee/mview.h"

REGISTER_FILE

//#include "sc1ships.h"

class VezlagariMissile;
class VezlagariBulkhead;

class MissileCourse {
  public:
  VezlagariMissile* Missile;
  double Turn1, Turn2, Turn3, Turn4;
  double Range1, Range2, Range3, Range4;
  double MissileRange;
  double LaunchFacingAngle;
  double CurrentFacingAngle;
  MissileCourse(VezlagariMissile* omissile);
  MissileCourse(VezlagariMissile* omissile, int side); // 1 or -1
  void ScrambleCourse(int iterations);
  double GetCurrentFacing(double fractionTraveled);
};

class VezlagariBarge : public Ship {
public:
  double       shipSpecialRegen;

  double       weaponRange;
  double       weaponVelocity;
  double       weaponDamage;
  double       weaponArmour;
  double       weaponTurnRate;
  int          weaponRandomness;
  double       weaponSpreadDistance;
  double       weaponSpreadAngle;

  double       specialDamage;
  double       specialRepulse;
  double       specialArmourEfficiency;
  double       specialMass;

  double       specialRepulseResilience;

  double       shrapnelDamage;
  double       shrapnelArmour;
  double       shrapnelRange;
  double       shrapnelVelocity;

  int    justBorn;
  
  public:
  VezlagariBarge(Vector2 opos, double angle, ShipData *data, unsigned int code);
  VezlagariBulkhead* Bulkhead;

  protected:
  virtual void calculate();
  virtual int activate_weapon();
  virtual int activate_special();
};

class VezlagariMissile : public Missile {
	public:
	VezlagariMissile(VezlagariBarge* ocreator, Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
			int side);
  virtual ~VezlagariMissile(void);
  MissileCourse* Course;
  VezlagariBarge* creator;
	double startingAngle;
	virtual void calculate();
	};

class VezlagariBulkhead : public SpaceObject {
public:
  double damageAbsorbed, normal, direct;
  double armourEfficiency;
  double resilience;
  VezlagariBarge* creator;
  VezlagariBulkhead(VezlagariBarge* ocreator);
  ~VezlagariBulkhead(void);
  virtual void calculate(void);
  virtual void inflict_damage(SpaceObject *other);
  virtual int handle_damage(SpaceLocation *source, double normal, double direct);

};

VezlagariBarge::VezlagariBarge(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
	{
  shipSpecialRegen = get_config_float("Ship", "SpecialRegen", 1);

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_float("Weapon", "Damage", 0);
  weaponArmour   = get_config_float("Weapon", "Armour", 0);
  weaponTurnRate = scale_turning(get_config_float("Weapon", "TurnRate", 0));
  weaponRandomness = get_config_int("Weapon", "Randomness", 0);
  weaponSpreadDistance = get_config_float("Weapon", "SpreadDistance", 0);
  weaponSpreadAngle = get_config_float("Weapon", "SpreadAngle", 0);

  specialRepulse  = (get_config_float("Special", "Repulse", 0));
  specialArmourEfficiency = get_config_float("Special", "ArmourEfficiency", 1);
  specialDamage = get_config_float("Special", "Damage", 0);
  specialRepulseResilience  = get_config_float("Special", "RepulseResilience", 100);
  specialMass = get_config_float("Special", "Mass", 0);

  justBorn = TRUE;
  Bulkhead = NULL;
}

void VezlagariBarge::calculate(void) {
	STACKTRACE
  if(justBorn) {
    Bulkhead = new VezlagariBulkhead(this);
    Bulkhead->resilience = this->specialRepulseResilience;
    Bulkhead->mass = this->specialMass;
    game->add(Bulkhead);
    justBorn = false;
  }
  Ship::calculate();
}

int VezlagariBarge::activate_weapon() {
	STACKTRACE
  game->add(new VezlagariMissile(this, 
    Vector2(get_size().x * 0.32, get_size().y * -0.4), angle + PI2 / 2, weaponVelocity, weaponDamage, weaponRange,
    weaponArmour, this, data->spriteWeapon, -1));
  game->add(new VezlagariMissile(this,
    Vector2(get_size().x * -0.32, get_size().y * -0.4), angle+ PI2 / 2, weaponVelocity, weaponDamage, weaponRange,
    weaponArmour, this, data->spriteWeapon, 1));
  return(TRUE);
}

int VezlagariBarge::activate_special() {
	STACKTRACE
  if(Bulkhead && Bulkhead->exists()) {
    if(Bulkhead->damageAbsorbed < 1) return(FALSE);
    Bulkhead->damageAbsorbed -= shipSpecialRegen;
    if(Bulkhead->damageAbsorbed<0)Bulkhead->damageAbsorbed=0;
    return(TRUE);
  }
	else return(FALSE);
	}

VezlagariMissile::VezlagariMissile(VezlagariBarge* ocreator, Vector2 opos, double oangle, double ov,
	int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
	int side)
	:
	Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship,osprite)
	{
	startingAngle=oangle;
	explosionSprite     = data->spriteWeaponExplosion;
  creator = ocreator;
  Course = new MissileCourse(this, side);
  Course->ScrambleCourse(creator->weaponRandomness);
	}

VezlagariMissile::~VezlagariMissile(void) {
  if(Course!=NULL) delete(Course);
}


void VezlagariMissile::calculate()
	{
	STACKTRACE
		Missile::calculate();
		double totalDist;
		totalDist = d / range;
    this->changeDirection(Course->GetCurrentFacing(totalDist));
	}

MissileCourse::MissileCourse(VezlagariMissile* omissile) {
  Missile = omissile;
  MissileRange = omissile->range;
  LaunchFacingAngle = omissile->startingAngle;
  Turn1 = Turn2 = Turn3 = Turn4 = 0;
  Range1 = Range2 = Range3 = Range4 = 0;
}

MissileCourse::MissileCourse(VezlagariMissile* omissile, int side) {
  Missile = omissile;
  MissileRange = omissile->range;
  LaunchFacingAngle = omissile->startingAngle;
  if(side==1) {
    Turn1 = (180 - Missile->creator->weaponSpreadAngle) * ANGLE_RATIO;
    Turn2 = Turn3 = Turn4 = 0;
    Range1 = 0.01 * Missile->creator->weaponSpreadDistance;
    Range2 = 0.25;
    Range3 = 0.35;
    Range4 = 0.37;
  }
  else if(side==-1) {
    Turn1 = (-180 + Missile->creator->weaponSpreadAngle) * ANGLE_RATIO;
    Turn2 = Turn3 = Turn4 = 0;
    Range1 = 0.01 * Missile->creator->weaponSpreadDistance;
    Range2 = 0.25;
    Range3 = 0.35;
    Range4 = 0.37;
  }
  else {
    Turn1 = Turn2 = Turn3 = Turn4 = 0;
    Range1 = Range2 = Range3 = Range4 = 0.25;
  }
}

double MissileCourse::GetCurrentFacing(double fractionTraveled) {
	STACKTRACE
  double fracDist, totalDist;
  totalDist = 1;
		if(fractionTraveled<Range1){
			fracDist = fractionTraveled / Range1;
			return(LaunchFacingAngle + (Turn1 * fracDist));
		}
		else if(fractionTraveled<(Range1+Range2)) {
			fracDist = (fractionTraveled-Range1) / Range2;
			return(LaunchFacingAngle + Turn1 + (Turn2 * fracDist));
		}
		else if(fractionTraveled<(Range1+Range2+Range3)) {
			fracDist = (fractionTraveled-Range1-Range2) / Range3;
			return(LaunchFacingAngle + Turn1 + Turn2 + (Turn3 * fracDist));
		}
		else if(fractionTraveled<(Range1+Range2+Range3+Range4)) {
			fracDist = (fractionTraveled-Range1-Range2-Range3) / Range4;
			return(LaunchFacingAngle + Turn1 + Turn2 + Turn3 + (Turn4 * fracDist));
		}
    else {
      return(LaunchFacingAngle+Turn1+Turn2+Turn3+Turn4);
    }
		//Missile::calculate();

  return(0.0);
}

void MissileCourse::ScrambleCourse(int iterations) {
	STACKTRACE
  double R;
  int i;
  for(i=0; i<iterations; i++) {
    R = (double)(random() % 2001 - 1000) / 1000.0;
    Turn1 += R / 30;
    R = (double)(random() % 2001 - 1000) / 1000.0;
    Turn2 += R / 10;
    R = (double)(random() % 2001 - 1000) / 1000.0;
    Turn3 += R / 5;
    R = (double)(random() % 2001 - 1000) / 1000.0;
    Turn4 += R / 2;
    R = (double)(random() % 2001 - 1000) / 1000.0;
    Range1 += R / 500;
    R = (double)(random() % 2001 - 1000) / 1000.0;
    Range2 += R / 8;
    R = (double)(random() % 2001 - 1000) / 1000.0;
    Range3 += R / 8;
    R = (double)(random() % 2001 - 1000) / 1000.0;
    Range4 += R / 8;
  }
}

VezlagariBulkhead::VezlagariBulkhead(VezlagariBarge* ocreator):
  SpaceObject(ocreator, ocreator->pos, ocreator->angle, ocreator->data->spriteExtra)
{
//SpaceObject::SpaceObject(SpaceLocation *creator, Vector2 opos, 
//	double oangle, SpaceSprite *osprite) 
	  layer = LAYER_SPECIAL;
  creator = ocreator;
  sprite = (creator->data)->spriteExtra;
  sprite_index = 0;
  damageAbsorbed = 0;
  this->normal = creator->specialDamage;
  this->armourEfficiency = creator->specialArmourEfficiency;
}


VezlagariBulkhead::~VezlagariBulkhead(void) {
	STACKTRACE
  if(creator) creator->Bulkhead = NULL;
}

void VezlagariBulkhead::calculate(void) {
	STACKTRACE
  int x;

  if ( !(creator && creator->exists()) )
  {
    state = 0;
	creator = 0;
    return;
  }

  this->pos = creator->pos;
  this->vel = creator->vel;
  this->angle = creator->angle;
  x = (int) (damageAbsorbed / armourEfficiency);
  if(x<0)x=0;
  if(x>15)x=15;
	sprite_index = get_index(this->angle);
	sprite_index &= 63;
  sprite_index += (x * 64);
  
  return;
}

void VezlagariBulkhead::inflict_damage(SpaceObject *other) {
	STACKTRACE
  int x;
  if(other->isShip() || other->isAsteroid() || other->isPlanet()) {
    damage(other, normal, direct);
    //SpaceObject::inflict_damage(other);
    if(creator) {
      x = 1 - (damageAbsorbed / resilience);
      if(x<0)x=0;
		  if(!other->isPlanet()) other->accelerate (other, creator->trajectory_angle(other), creator->specialRepulse / other->mass, MAX_SPEED);
      creator->accelerate (creator, other->trajectory_angle(creator), creator->specialRepulse / creator->mass, MAX_SPEED);
    }
  }  
}

int VezlagariBulkhead::handle_damage(SpaceLocation *source, double normal, double direct) {
	STACKTRACE
  this->damageAbsorbed += normal + direct;
  //message.print(1500,9,"damageAbsorbed = %f",damageAbsorbed);

	state = 1;

  return(SpaceObject::handle_damage(source, normal, direct));
}


REGISTER_SHIP (VezlagariBarge)
