#include "../ship.h"
REGISTER_FILE

//#include "../sc1ships.h"

class LyrmristuWaBolt1 : public Missile {
	public:
	LyrmristuWaBolt1(double ox, double oy, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite);
};

class LyrmristuWaBolt2 : public Missile {
	public:
	LyrmristuWaBolt2(double ox, double oy, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite);
};


class LyrmristuWaSphere : public Shot {
	public:
	LyrmristuWaSphere **PP;
	LyrmristuWaSphere(double ox, double oy, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite, LyrmristuWaSphere **P);
	virtual void calculate();
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
	virtual void inflict_damage(SpaceObject *other);
	virtual void destroy();
	virtual ~LyrmristuWaSphere();
	private:


};

class LyrmristuWarDestroyer : public Ship {
public:
  double       weaponRange1;
  double       weaponVelocity1;
  int          weaponDamage1;
  int          weaponArmour1;
  double	   weaponAngleSpread1;
  double       weaponRange2;
  double       weaponVelocity2;
  int          weaponDamage2;
  int          weaponArmour2;

  int    specialColor;
  double specialRange;
  int    specialFrames;

  double       specialVelocity;
  int          specialDamage;
  int          specialArmour;
  double       specialTurnRate;
  int          specialMaxDamage;
  LyrmristuWaSphere *weaponObject;


  public:
  LyrmristuWarDestroyer(Vector2 opos, double angle, ShipData *data, unsigned int code);

  protected:
  virtual int activate_weapon();
  virtual int activate_special();
  virtual int handle_damage(SpaceLocation *source, double normal, double direct);

};


LyrmristuWarDestroyer::LyrmristuWarDestroyer(Vector2 opos, double angle, ShipData *data, unsigned int code)
	:
	Ship(opos, angle, data, code)
	{
  weaponRange1    = scale_range(get_config_float("Weapon", "Range1", 0));
  weaponVelocity1 = scale_velocity(get_config_float("Weapon", "Velocity1", 0));
  weaponDamage1   = get_config_int("Weapon", "Damage1", 0);
  weaponArmour1   = get_config_int("Weapon", "Armour1", 0);
  weaponAngleSpread1 = get_config_float("Weapon","AngleSpread1", 0) * ANGLE_RATIO;
  weaponRange2    = scale_range(get_config_float("Weapon", "Range2", 0));
  weaponVelocity2 = scale_velocity(get_config_float("Weapon", "Velocity2", 0));
  weaponDamage2   = get_config_int("Weapon", "Damage2", 0);
  weaponArmour2   = get_config_int("Weapon", "Armour2", 0);

  specialColor  = get_config_int("Special", "Color", 0);
  specialRange  = scale_range(get_config_float("Special", "Range", 0));
  specialFrames = get_config_int("Special", "Frames", 0);
  specialRange    = scale_range(get_config_float("Special", "Range", 0));
  specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
  specialDamage   = get_config_int("Special", "DamageArmour", 0);
  specialArmour   = get_config_int("Special", "DamageArmour", 0);
  specialTurnRate = scale_turning(get_config_float("Special", "TurnRate", 0));
  specialMaxDamage = get_config_int("Special","MaxDamage",0);
  weaponObject=NULL;
}

int LyrmristuWarDestroyer::activate_weapon() {
  game->add(new LyrmristuWaBolt2(size.y*(0.0), (size.y * 0.6),
    angle, weaponVelocity2, weaponDamage2, weaponRange2, weaponArmour2,
    this, data->spriteWeapon));
  game->add(new LyrmristuWaBolt1(size.y*(-0.4), (size.y * 0.6),
    angle-weaponAngleSpread1, weaponVelocity1, weaponDamage1, weaponRange1, weaponArmour1,
    this, data->spriteWeapon));
  game->add(new LyrmristuWaBolt1(size.y*(0.4), (size.y * 0.6),
    angle+weaponAngleSpread1, weaponVelocity1, weaponDamage1, weaponRange1, weaponArmour1,
    this, data->spriteWeapon));

  return(TRUE);
}

int LyrmristuWarDestroyer::activate_special() {
if(weaponObject==NULL)
		{weaponObject = (new LyrmristuWaSphere(
			0.0, 0.0, angle, specialVelocity, specialDamage, specialRange,
			specialArmour, this, data->spriteSpecial, &weaponObject));
		game->add(weaponObject);
		}
	else if (weaponObject->damage_factor >= specialMaxDamage) return(FALSE);
	else
	{
		weaponObject->damage_factor += specialDamage;
		weaponObject->armour += specialArmour;
	}
	return(TRUE);	}

int LyrmristuWarDestroyer::handle_damage(SpaceLocation *source, double normal, double direct) {
	if(weaponObject==NULL)
	  return Ship::handle_damage(source, normal, direct);
	else if(weaponObject->state==0)
	  return Ship::handle_damage(source, normal, direct);
    else {
		if(weaponObject->armour < normal) {
			normal -= weaponObject->armour;
			weaponObject->armour = 0;
			weaponObject->damage_factor = 0;
			weaponObject->state = 0;
		}
		if(weaponObject->armour = normal) {
			weaponObject->armour = 0;
			weaponObject->damage_factor = 0;
			normal = 0;
			weaponObject->state = 0;
		}
		if(weaponObject->armour > normal) {
			weaponObject->armour -= normal;
			weaponObject->damage_factor = weaponObject->armour;
			normal = 0;
		}
		return Ship::handle_damage(source, normal, direct);
	}
}


LyrmristuWaSphere::LyrmristuWaSphere(double ox, double oy, double oangle, 
double ov, int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite, 
LyrmristuWaSphere **P)
	:
	Shot(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oship, osprite)
	{
	explosionSprite     = data->spriteWeaponExplosion;
	//explosionFrameCount = 20;
	//explosionFrameSize  = 50;
	PP=P;
	if(armour<damage_factor) damage_factor=armour;
	if(damage_factor<armour) armour=damage_factor;
    sprite_index = (damage_factor - 1)/ 2;
	if(sprite_index>9) sprite_index=9;
	if(sprite_index<0) sprite_index=0;
	}

	void LyrmristuWaSphere::calculate(){
	if(!ship->exists()) {
		state = 0;
		return;
		}

	if(ship && (ship->target) && (!ship->target->isInvisible())) {
	angle = ship->get_angle() -0;
		vel = v * unit_vector(angle);
		}
	pos = ship->normal_pos();
	angle = ship->angle;
	vel = ship->get_vel();


	Shot::calculate();
	if(state!=0) {
		sprite_index = damage_factor - 1;
		if(sprite_index>15) sprite_index=15;
		if(sprite_index<0) sprite_index=0;
	}

	}

int LyrmristuWaSphere::handle_damage(SpaceLocation *source, double normal, double direct) {
	int x;
	x = Shot::handle_damage(source, normal, direct);
	if(source->isShot()){
		// empty statement... not sure how to handle it yet
	}
	if(state==0) *PP=NULL;
	if(damage_factor<=0) *PP=NULL;
	if(armour<=0) *PP=NULL;
	if(armour<damage_factor) damage_factor=armour;
	return x;
	}

void LyrmristuWaSphere::inflict_damage(SpaceObject *other) {
  int startingTargetArmour, endingTargetArmour;
	int originalCrew=-1;
	int originalStrength;
	originalStrength = this->damage_factor;
	Ship* S1;
	if(other->isShip()) {
		S1 = (Ship*)other;
		originalCrew = S1->crew;
	}
  if(other->isShot()) startingTargetArmour = ((Shot*)other)->armour;
	Shot::inflict_damage(other);
  if(other->isShot()) {
    endingTargetArmour = ((Shot*)other)->armour;
    ((Shot*)other)->damage_factor -= (startingTargetArmour - 
endingTargetArmour);
    if(((Shot*)other)->damage_factor<0) ((Shot*)other)->damage_factor = 0;
  }
	if(other->isAsteroid()&&damage_factor>1) {
		state=1; damage_factor--;
	}
	if(originalCrew>0 && originalCrew<originalStrength) {
		damage_factor = originalStrength-originalCrew;
		armour = damage_factor;
		state = 1;
	}
		if(state == 0) *PP=NULL;
		if(damage_factor<=0) *PP=NULL;
		if(armour<=0) *PP=NULL;

	return;
}

void LyrmristuWaSphere::destroy()
{
	*PP=NULL;
	Shot::destroy();
}

LyrmristuWaSphere::~LyrmristuWaSphere(){
	*PP=NULL;
	//LyrmristuWaSPhere::~LyrmristuWaSphere()
}

LyrmristuWaBolt1::LyrmristuWaBolt1(double ox, double oy, double oangle, 
double ov,
	int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite)
	:
	Missile(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oship,osprite)
	{
	explosionSprite     = data->spriteWeaponExplosion;
	//explosionFrameCount = 20;
	//explosionFrameSize  = 50;
	//sprite_index=0;
    sprite_index = (get_index(ship->get_angle()) +
										 (0 * 64));

	}

LyrmristuWaBolt2::LyrmristuWaBolt2(double ox, double oy, double oangle, 
double ov,
	int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite)
	:
	Missile(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oship,osprite)
	{
	explosionSprite     = data->spriteWeaponExplosion;
	//explosionFrameCount = 20;
	//explosionFrameSize  = 50;
	//sprite_index=1;
    sprite_index = (get_index(ship->get_angle()) +
										 (1 * 64));

	}


REGISTER_SHIP ( LyrmristuWarDestroyer )

