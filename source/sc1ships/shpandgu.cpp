#include "../ship.h"
REGISTER_FILE

#include "../sc1ships.h"

class AndrosynthBubble : public AnimatedShot {
  int courseFrames;

  public:
  AndrosynthBubble(Vector2 opos, double oangle, double ov,
    int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
    int ofcount, int ofsize);

  void calculate();
};

AndrosynthGuardian::AndrosynthGuardian(Vector2 opos, double angle, ShipData *data, unsigned int code) :
  Ship(opos, angle, data, code)
{
	weaponDamage       = get_config_int("Weapon", "Damage", 1);
	weaponArmour       = get_config_int("Weapon", "Armour", 1);
	weaponRange        = scale_range(get_config_float("Weapon", "Range", 10));
	weaponVelocity     = scale_velocity(get_config_float("Weapon", "Velocity", 100));

	specialSprite   = data->spriteSpecial;
	shipSprite      = sprite;
	specialDamage   = get_config_int("Special", "Damage", 0);
	specialBounceDistance   = get_config_int("Special", "BounceDistance", 0);
	specialBounceTime       = get_config_int("Special", "BounceTime",     0);
	specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialTurnRate = scale_turning(get_config_float("Special", "TurnRate", 0));
	specialActive   = FALSE;

	normalMass = mass;
	specialMass = get_config_float("Special", "Mass", 1);

	shipTurnRate    = turn_rate;
	shipRechargeAmount = recharge_amount;
	bounce_status = 0;
}

void AndrosynthGuardian::calculate()
{
	Ship::calculate();
	if(specialActive && (batt == -1)) {
		set_sprite(shipSprite);
		damage_factor = 0;
		specialActive = FALSE;
		vel = 0;
		turn_rate = shipTurnRate;
		recharge_amount = shipRechargeAmount;
		batt = 0;
		mass = normalMass;
		}
	return;
	}

void AndrosynthGuardian::calculate_thrust() {
	if(specialActive) {
		double dv;
		if (bounce_status > 0) {
			if (bounce_status == specialBounceTime) 
				accelerate(this, angle - PI, .1, MAX_SPEED);
			bounce_status -= frame_time;
			dv = specialVelocity * frame_time / specialBounceTime;
			accelerate(this, angle, dv, specialVelocity);
			}
		else {
			dv = specialVelocity / 2 * frame_time / 50.;
			accelerate(this, angle, dv, specialVelocity);
			accelerate(this, angle, dv, specialVelocity);
			}
		}
	else Ship::calculate_thrust();
	}

void AndrosynthGuardian::calculate_hotspots()
{
  if(specialActive)
    return;
  Ship::calculate_hotspots();
}

void AndrosynthGuardian::inflict_damage(SpaceObject *other) {
	if (damage_factor > 0) {
		if (other->mass) {
			game->add(new Animation(this, 
					pos + unit_vector(trajectory_angle(other)) * 20.0,
					meleedata.sparkSprite, 0, SPARK_FRAMES, 50, DEPTH_EXPLOSIONS));
			translate(-specialBounceDistance*unit_vector(angle));
			bounce_status = specialBounceTime;
			int i = iround_down(damage_factor / 2);
			if (i >= BOOM_SAMPLES) i = BOOM_SAMPLES - 1;
			play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
			}
		if (!other->isProtected()) damage(other, damage_factor);
		}
	else damage(other, 0);
	return;
	}

int AndrosynthGuardian::activate_weapon() {
  
  if(specialActive)
	  return FALSE;

  game->add(new AndrosynthBubble(
    Vector2(0.0, size.y / 2.0), angle, weaponVelocity, weaponDamage, weaponRange,
    weaponArmour, this, data->spriteWeapon, 10, 50));
  return(TRUE);
}

int AndrosynthGuardian::activate_special() {
	if(specialActive)
		return(FALSE);

	if (batt < 1) return(FALSE);

	set_sprite(specialSprite);

	normalMass = mass;
	shipTurnRate    = turn_rate;
	shipRechargeAmount = recharge_amount;

	damage_factor = specialDamage;
	specialActive = TRUE;
	turn_rate     = specialTurnRate;
  
	recharge_step = recharge_rate;
	recharge_amount = -1;
	mass = specialMass;

	return(TRUE);
}

AndrosynthBubble::AndrosynthBubble(Vector2 opos, double oangle,
  double ov, int odamage, double orange, int oarmour, Ship *oship,
  SpaceSprite *osprite, int ofcount, int ofsize) :
  AnimatedShot(oship, opos, oangle, ov, odamage, orange, oarmour, oship, 
    osprite, ofcount, ofsize),
  courseFrames(0)
{
  explosionSprite     = data->spriteWeaponExplosion;
  explosionFrameCount = 6;
  explosionFrameSize  = 50;
}

void AndrosynthBubble::calculate()
{
	double newAngle;

	AnimatedShot::calculate();
	courseFrames += frame_time;
	if(courseFrames >= 150) {
		courseFrames -= 150;

		newAngle = random(PI2);
		vel = v * unit_vector(newAngle);

		int i;
		double r = 99999;
		newAngle = 99999;
		
		// ship->control->ship == 0 (can happen ?!)
		// ... yeah ... look at control->select_ship. There you can see
		// that control->ship=0 if the ship does not exist. Perhaps there's
		// a de-synch between the two calculations (!exists() lags the control->ship) ?

		if (ship && ship->exists() && ship->control && ship->control->ship) {
			for (i = 0; i < game->num_targets; i += 1) {
				SpaceObject *s = game->target[i];
				if (ship->control->valid_target(s) && (distance(s) < r) && !s->isInvisible()) {
					r = distance(s);
					newAngle = trajectory_angle(s);
					}
				}
			}
		else {
			for (i = 0; i < game->num_targets; i += 1) {
				SpaceObject *s = game->target[i];
				if (s->sameTeam(this) && (distance(s) < r) && !s->isInvisible()) {
					r = distance(s);
					newAngle = trajectory_angle(s);
					}
				}
			}
		if(newAngle < PI * 4) {
			vel += unit_vector(newAngle) * v / 2;
			}      
		}
	return;
	}


REGISTER_SHIP(AndrosynthGuardian)

