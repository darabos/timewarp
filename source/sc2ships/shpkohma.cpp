#include "../ship.h"
REGISTER_FILE


#include "../sc2ships.h"

class KohrAhBlade : public AnimatedShot {
	bool persist;
	int       passive;
	double    passiveRange;
	double    passiveVelocity;

	public:
	KohrAhBlade(Vector2 opos, double oangle, double ov, int odamage,
		double orange, int oarmour, Ship *oship,
		SpaceSprite *osprite, int ofcount, int ofsize, bool persist);

	virtual void calculate();
	virtual void animateExplosion();

	void disengage();
};

class KohrAhBladeDecay : public Animation {
	public:
	KohrAhBladeDecay(SpaceLocation *creator, Vector2 opos, Vector2 ovel,
		SpaceSprite *osprite, int ofcount, int ofsize);
};

class KohrAhFRIED : public Shot {
  int frame_count;

  public:
  KohrAhFRIED(double oangle, double ov, int odamage, double orange,
    int oarmour, Ship *oship, SpaceSprite *osprite, int ofcount);

  virtual void calculate();
	virtual void inflict_damage(SpaceObject *other);
};

void KohrAhFRIED::inflict_damage(SpaceObject *other) {
	if ((other->getID() &~ SPACE_MASK) == CHMMR_SPEC) other->state = 0;
	Shot::inflict_damage(other);
	return;
	}

KohrAhMarauder::KohrAhMarauder(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code) {
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	bladesPersist = (bool)get_config_int("Weapon", "Persists", 0);
	weaponFired    = 0;

	specialRange    = scale_range(get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialDamage   = get_config_int("Special", "Damage", 0);
	specialArmour   = get_config_int("Special", "Armour", 0);

	numblades = 0;
	maxblades = get_config_int("Weapon", "MaxBlades", 1);
	weaponObject = new KohrAhBlade*[maxblades];
	for (int i = 0; i < maxblades; i += 1) {
		weaponObject[i] = NULL;
		}
	debug_id = 1031;
	}

int KohrAhMarauder::activate_weapon() {
	if (weaponFired) return(FALSE);
	if (numblades == maxblades) {
		weaponObject[0]->state = 0;
		numblades -= 1;
		for (int i = 0; i < numblades; i += 1) {
			weaponObject[i] = weaponObject[i + 1];
			}
		weaponObject[numblades] = NULL;
		}
	weaponObject[numblades] = new KohrAhBlade(Vector2(0.0, (get_size().y / 2.0)), angle, weaponVelocity,
			weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon,
			10, 40, bladesPersist);
	add(weaponObject[numblades]);
	numblades += 1;
	weaponFired = 1;
	return(TRUE);
	}

int KohrAhMarauder::activate_special() {
	int i;
	for(i = 0; i < 16; i++) {
		add(new KohrAhFRIED(
				i * PI2/16 - PI, specialVelocity, specialDamage,
				specialRange, specialArmour, this, data->spriteSpecial, 20));
		}
	return(TRUE);
	}

void KohrAhMarauder::calculate() {
	Ship::calculate();
	
	if (!fire_weapon) {
		if(weaponFired == 1) {
			weaponObject[numblades-1]->disengage();
			}
		weaponFired = 0;
		}
	
	else if ((weaponFired == 1) && !weaponObject[numblades-1]->exists()) {
		weaponFired = 2;
		}
	int j = 0;
	for (int i = 0; i < numblades; i += 1) {
		weaponObject[i-j] = weaponObject[i];
		if (!weaponObject[i]->exists()) j += 1;
		if (j) weaponObject[i] = NULL;
		}
	numblades -= j;		
	return;
	}

KohrAhBlade::KohrAhBlade(Vector2 opos, double oangle, double ov,
  int odamage, double orange, int oarmour, Ship *oship,
  SpaceSprite *osprite, int ofcount, int ofsize, bool _persists) :
  AnimatedShot(oship, opos, oangle, ov, odamage, -1.0, oarmour, oship,
    osprite, ofcount, ofsize),
  persist(_persists),
  passive(FALSE),
  passiveRange(orange)
{
  explosionSprite     = data->spriteWeaponExplosion;
  explosionFrameCount = 20;
  explosionFrameSize  = 40;
	debug_id = 1032;
}

void KohrAhBlade::calculate() {
	SpaceObject *o, *t = NULL;
	double oldrange = 999999;

	AnimatedShot::calculate();

	if (!ship && !persist) die();

	if(passive) {
		Query a;
		for (a.begin(this, bit(LAYER_SHIPS) + bit(LAYER_SPECIAL), 
				passiveRange); a.current; a.next()) {
			o = a.currento;
			if (!o->sameTeam(this) && o->canCollide(this) && !(o->isAsteroid() || o->isPlanet()) && (distance(o) < oldrange) && !o->isInvisible() && (distance(o) != 0) ) {
				t = o;
				oldrange = distance(o);
				}
			}
		if (t) {
			angle = trajectory_angle(t);
//			vx = (v / 10.0) * cos(angle);
//			vy = (v / 10.0) * sin(angle);
			vel = (v / 10.0) * unit_vector(angle);
			}
		else {
//			vx = 0;
//			vy = 0;
			vel = 0;
			}
		}
	return;
	}

void KohrAhBlade::animateExplosion()
{
  add(new KohrAhBladeDecay(this, pos,
    vel, explosionSprite, explosionFrameCount, explosionFrameSize));
}

void KohrAhBlade::disengage()
{
  passive = TRUE;
}

KohrAhBladeDecay::KohrAhBladeDecay(SpaceLocation *creator, Vector2 opos, Vector2 ovel,
  SpaceSprite *osprite, int ofcount, int ofsize) :
  Animation(creator, opos, osprite, 0, ofcount, ofsize, DEPTH_EXPLOSIONS)
{
//  vx = ovx / 2;
//  vy = ovy / 2;
  vel = 0.5 * ovel;
}

KohrAhFRIED::KohrAhFRIED(double oangle, double ov, int odamage,
  double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
  int ofcount) :
  Shot(oship, Vector2(0.0, 0.0), oangle, ov, odamage, orange, oarmour, oship, osprite, 1.0),
  frame_count(ofcount)
{
//  x += cos(angle) * 50.0;
//  y += sin(angle) * 50.0;
  pos += 50.0 * unit_vector(angle);
	debug_id = 1033;
}

void KohrAhFRIED::calculate() {
  sprite_index = (int)((d / range) * (double)(frame_count - 1));
  Shot::calculate();
}



REGISTER_SHIP(KohrAhMarauder)
