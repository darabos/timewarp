/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

#include "../sc1ships.h"

#define MAX_DOGIS 4

class ChenjesuShot : public Missile
{
public:
	ChenjesuShot(Vector2 opos, double oangle, double ov, int odamage,
		int oarmour, SpaceLocation *creator, SpaceSprite *osprite);
	virtual void inflict_damage(SpaceObject *other);
	virtual void animateExplosion();
};

class ChenjesuDOGI : public AnimatedShot
{
	int		sap_factor;
	double	accel_rate;
	int		*num_dogis;

public:
	ChenjesuDOGI(Vector2 opos, double oangle, double ov, int fuel_sap,
		int oarmour, double oaccel, double omass, Ship *oship,
		SpaceSprite *osprite, int *onum_dogis);
	double	avoidanceAngle;
	double	avoidanceFactor;
	virtual void calculate();
	virtual void inflict_damage(SpaceObject *other);
	virtual void death();
	virtual void ship_died();
	virtual void animateExplosion();
	virtual void soundExplosion();
};

ChenjesuBroodhome::ChenjesuBroodhome(Vector2 opos, double angle, ShipData *data, unsigned int code) :
	Ship(opos, angle, data, code)
{
	weaponDamage		= get_config_int("Weapon", "Damage", 0);
	shardRange			= scale_range(get_config_int("Weapon", "ShardRange", 0));
	shardDamage			= get_config_int("Weapon", "ShardDamage", 0);
	shardArmour			= get_config_int("Weapon", "ShardArmour", 0);
	shardRelativity		= get_config_float("Weapon", "ShardRelativity", 0);
	shardRotation		= (get_config_int("Weapon", "ShardRotation", 0) > 0);
	weaponArmour		= get_config_int("Weapon", "Armour", 0);
	weaponVelocity		= scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponFired			= FALSE;
	weaponObject		= NULL;

	specialVelocity		= scale_velocity(get_config_float("Special", "Velocity", 0));
	specialFuelSap		= get_config_int("Special", "FuelSap", 0);
	specialArmour		= get_config_int("Special", "Armour", 0);
	specialAccelRate	= scale_acceleration(get_config_float("Special", "AccelRate", 0), 1);
	specialMass			= get_config_float("Special", "Mass", 0);
	specialNumDOGIs		= 0;
	specialAvoidanceAngle	= get_config_float("Special", "AvoidanceAngle", 0) * ANGLE_RATIO;
	specialAvoidanceFactor	= get_config_float("Special", "AvoidanceFactor", 0);
	debug_id = 1004;
}

int ChenjesuBroodhome::activate_weapon()
{
	if(weaponFired)
		return(FALSE);
	weaponObject = new ChenjesuShot(Vector2(0.0, (size.y / 2.0)), angle,
		weaponVelocity, iround(weaponDamage), iround(weaponArmour), this, data->spriteWeapon);
	game->add(weaponObject);
	weaponFired = TRUE;
	return(TRUE);
}

int ChenjesuBroodhome::activate_special()
{
	if(specialNumDOGIs >= MAX_DOGIS)
		return(FALSE);
	ChenjesuDOGI* DOGI; //added by Varith
	DOGI = (new ChenjesuDOGI( Vector2(0.0, -size.y / 1.5),
		angle + PI, specialVelocity, iround(specialFuelSap), iround(specialArmour),
		specialAccelRate, specialMass, this, data->spriteSpecial,
		&specialNumDOGIs));
	DOGI->avoidanceFactor = specialAvoidanceFactor;
	DOGI->avoidanceAngle = specialAvoidanceAngle;
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
		for(i = 0; i < 8; i++) {
			Shot *shot = new Missile(weaponObject, Vector2(0.0, 0.0),
				(shardRotation?weaponObject->get_angle():0) +	PI/4 * (double)(i),
				weaponVelocity, shardDamage, shardRange, shardArmour,
				weaponObject, data->spriteExtra, shardRelativity);
			shot->collide_flag_sameship = bit(LAYER_SPECIAL);
			game->add(shot);
		}
//		weaponObject->play_sound2(data->sampleExtra[0]);
		weaponObject->play_sound(data->sampleExtra[0]);
		weaponObject->destroy();
		weaponObject = NULL;
	}
	return;
}

ChenjesuShot::ChenjesuShot(Vector2 opos, double oangle, double ov,
	int odamage, int oarmour, SpaceLocation *creator, SpaceSprite *osprite) :
	Missile(creator, opos, oangle, ov, odamage, -1.0, oarmour, creator, osprite)
{
	collide_flag_sameship = bit(LAYER_SPECIAL);
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 20;
	explosionFrameSize  = 50;
	debug_id = 1005;
}

void ChenjesuShot::inflict_damage(SpaceObject *other)
{
	Shot::inflict_damage(other);
	if ((other->mass > 0) && !exists())
		if (other-exists())
			game->add(new FixedAnimation(this, other,
				explosionSprite, 0, explosionFrameCount,
				explosionFrameSize, DEPTH_EXPLOSIONS));
		else
			game->add(new Animation(this, pos,
				explosionSprite, 0, explosionFrameCount,
				explosionFrameSize, DEPTH_EXPLOSIONS));
}

void ChenjesuShot::animateExplosion()
{
	return;
}

ChenjesuDOGI::ChenjesuDOGI(Vector2 opos, double ov, double oangle,
		int fuel_sap, int oarmour, double accel, double omass, Ship *oship,
		SpaceSprite *osprite, int *onum_dogis) :
	AnimatedShot(oship, opos, ov, oangle, 0, -1.0, oarmour, oship, osprite, 64, 50),
	sap_factor(fuel_sap), accel_rate(accel), num_dogis(onum_dogis)
{
	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);
	mass = omass;
	collide_flag_sameship = ALL_LAYERS;
	(*num_dogis)++;
	debug_id = 1006;
}

void ChenjesuDOGI::calculate()
{
	AnimatedShot::calculate();

	if (ship && ship->exists()) {
		target = ship->target; }
	else {
		state = 0;
		return;  }

	if (target && !target->isInvisible()) {
		angle = trajectory_angle(target);

		double ra = normalize(target->get_angle() - (angle - PI), PI2);
		if (ra > PI) ra -= PI2;

		if (fabs(ra) < avoidanceAngle)
			angle += PI/2 * sign(ra) * pow((1 - fabs(ra)/avoidanceAngle), 1/avoidanceFactor);

		normalize(angle, PI2);
	}

	accelerate (this, angle, accel_rate * frame_time, v);
}

void ChenjesuDOGI::inflict_damage(SpaceObject *other)
{
	play_sound(data->sampleExtra[1]);

	if(sameTeam(other)) return;

	other->handle_fuel_sap(this, sap_factor);
	double a = trajectory_angle(other);
	accelerate(this, a, -v, MAX_SPEED);
	if (other->mass > mass/100.0)
		other->accelerate(this, a, v*3*mass/other->mass, v);
	return;
}

void ChenjesuDOGI::death()
{
	if (num_dogis) (*num_dogis)--;
}

void ChenjesuDOGI::ship_died()
{
	num_dogis = NULL;
	SpaceObject::ship_died();
	state = 0;
	return;
}

void ChenjesuDOGI::animateExplosion()
{
	game->add(new Animation(this, pos,
		data->spriteSpecialExplosion, 0, 20, 25, DEPTH_EXPLOSIONS));
}

void ChenjesuDOGI::soundExplosion()
{
	play_sound2(data->sampleExtra[2]);
	return;
}


REGISTER_SHIP(ChenjesuBroodhome)
