#include "../ship.h"
REGISTER_FILE

#include "../sc1ships.h"

#define MAX_DOGIS 4

class ChenjesuShot : public AnimatedShot {
	public:
	ChenjesuShot(Vector2 opos, double oangle, double ov, int odamage,
		int oarmour, Ship *oship, SpaceSprite *osprite, int ofcount, int ofsize);

	virtual void inflict_damage(SpaceObject *other);
	virtual void animateExplosion();
	};

class ChenjesuDOGI : public AnimatedShot {
  int    sap_factor;
  double accel_rate;
  int   *num_dogis;

  public:
  ChenjesuDOGI(Vector2 opos, double oangle, double ov, int fuel_sap,
    int oarmour, double oaccel, double omass, Ship *oship,
    SpaceSprite *osprite, int *onum_dogis);
  double avoidanceAngle;
  double avoidanceFactor;
  virtual void calculate();
  virtual void inflict_damage(SpaceObject *other);
  virtual void death();
  virtual void ship_died();
  virtual void animateExplosion();
  virtual void soundExplosion();
};

ChenjesuBroodhome::ChenjesuBroodhome(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
{
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	shardRange     = scale_range(get_config_int("Weapon", "ShardRange", 0));
	shardDamage    = get_config_int("Weapon", "ShardDamage", 0);
	shardArmour    = get_config_int("Weapon", "ShardArmour", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponFired    = FALSE;
	weaponObject   = NULL;

	specialVelocity    = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialFuelSap     = get_config_int("Special", "FuelSap", 0);
	specialArmour      = get_config_int("Special", "Armour", 0);
	specialAccelRate   = scale_acceleration(get_config_float("Special", "AccelRate", 0), 1);
	specialMass        = get_config_float("Special", "Mass", 0);
	specialNumDOGIs    = 0;
	specialAvoidanceAngle = get_config_float("Special", "AvoidanceAngle", 0); //added by Varith
	specialAvoidanceFactor = get_config_float("Special", "AvoidanceFactor", 0); //added by Varith
}

int ChenjesuBroodhome::activate_weapon()
{
	if(weaponFired)
		return(FALSE);
	weaponObject = new ChenjesuShot(Vector2(0.0, (size.y / 2.0)), angle,
		weaponVelocity, weaponDamage, weaponArmour, this,
		data->spriteWeapon, 64, 50);
	game->add(weaponObject);
	weaponFired = TRUE;
	return(TRUE);
}

int ChenjesuBroodhome::activate_special()
{
  ChenjesuDOGI* DOGI; //added by Varith
  if(specialNumDOGIs >= MAX_DOGIS)
    return(FALSE);

	DOGI = (new ChenjesuDOGI( Vector2(0.0, -size.y / 1.5),
		angle + PI, specialVelocity, specialFuelSap, specialArmour,
		specialAccelRate, specialMass, this, data->spriteSpecial,
		&specialNumDOGIs));
	DOGI->avoidanceFactor = specialAvoidanceFactor;
	DOGI->avoidanceAngle = specialAvoidanceAngle; //added by Varith
	game->add(DOGI);
	return(TRUE);
}

void ChenjesuBroodhome::calculate()
{
	int i;

	Ship::calculate();

	if((weaponObject != NULL) && (!weaponObject->exists()))
		weaponObject = NULL;
	if(weaponFired && (!fire_weapon))
		weaponFired = FALSE;
	if((weaponObject != NULL) && (!fire_weapon)) {
		weaponObject->stop();
		for(i = 0; i < 8; i++) {
//			AnimatedShot::AnimatedShot(
			Shot *shot = new AnimatedShot(weaponObject, Vector2(0.0, 0.0),
				PI/4 * (double)(i), weaponVelocity, shardDamage, shardRange,
				shardArmour, weaponObject, data->spriteExtra, 64, 50);
			shot->collide_flag_sameship = bit(LAYER_SPECIAL);
			game->add(shot);
		}
		weaponObject->destroy();
		weaponObject = NULL;
		play_sound2(data->sampleExtra[0]);
	}
	return;
}

ChenjesuShot::ChenjesuShot(Vector2 opos, double oangle, double ov,
  int odamage, int oarmour, Ship *oship, SpaceSprite *osprite,
  int ofcount, int ofsize) :
  AnimatedShot(oship, opos, oangle, ov, odamage, -1.0, oarmour, oship,
    osprite, ofcount, ofsize)
{
	collide_flag_sameship = bit(LAYER_SPECIAL);
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 30;
	explosionFrameSize  = 50;
}

void ChenjesuShot::inflict_damage(SpaceObject *other)
{
	Shot::inflict_damage(other);
	if (other->mass <= 0) return;
	if(other->isShip())
		game->add(new FixedAnimation(this, other,
			explosionSprite, 0, explosionFrameCount,
			explosionFrameSize, DEPTH_EXPLOSIONS)
		);
	else
		game->add(new Animation(this, pos,
			explosionSprite, 0, explosionFrameCount,
			explosionFrameSize, DEPTH_EXPLOSIONS)
		);
}
void ChenjesuShot::animateExplosion() {
	return;
}

ChenjesuDOGI::ChenjesuDOGI(Vector2 opos, double ov, double oangle,
  int fuel_sap, int oarmour, double accel, double omass, Ship *oship,
  SpaceSprite *osprite, int *onum_dogis) :
  AnimatedShot(oship, opos, ov, oangle, 0, -1.0, oarmour, oship,
    osprite, 64, 50),
  sap_factor(fuel_sap),
  accel_rate(accel),
  num_dogis(onum_dogis)
{
	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);
  mass = omass;
  collide_flag_sameship = ALL_LAYERS;
  (*num_dogis)++;
}

void ChenjesuDOGI::calculate() {
	AnimatedShot::calculate();

	if (ship) {
		target = ship->target;
	}
	if (target && !target->isInvisible()) {
		angle = trajectory_angle(target);

		double ra = target->get_angle() - (angle - PI);
		while (ra > PI) ra -= PI2;
		while (ra < -PI) ra += PI2;
		//if (fabs(ra) < PI/4) {
		if (fabs(ra) < avoidanceAngle) {
			double d = distance(target);
			if (d > 100) angle += (100 * PI * sign(ra) / d) * avoidanceFactor;
			//else if (d < 1000) angle -= PI;
		}
		while(angle > PI2) angle -= PI2;
		while(angle < 0.0) angle += PI2;
	}

	accelerate (this, angle, accel_rate * frame_time, v);
}

void ChenjesuDOGI::inflict_damage(SpaceObject *other) {
	play_sound(data->sampleExtra[1]);
	if(sameTeam(other)) return;

	other->handle_fuel_sap(this, sap_factor);
	double a = trajectory_angle(other);
	accelerate(this, a, v*2, MAX_SPEED);
	return;
}

void ChenjesuDOGI::death() {
	if (num_dogis) (*num_dogis)--;
}
void ChenjesuDOGI::ship_died() {
	num_dogis = NULL;
	SpaceObject::ship_died();
	return;
}

void ChenjesuDOGI::animateExplosion()
{
	game->add(new Animation(this, pos,
		data->spriteSpecialExplosion, 0, 16, 50, DEPTH_EXPLOSIONS));
}

void ChenjesuDOGI::soundExplosion() {
	play_sound2(data->sampleExtra[2]);
	return;
}


REGISTER_SHIP(ChenjesuBroodhome)
