#include "../ship.h"

REGISTER_FILE

class BubalosBomber : public Ship {
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;
  int          flame_frame;
  int          weaponArming;
  double       weaponMinSplitRadius;
  int          weapon_offset;
  bool         can_switch;

  double       mirvRange;
  double       mirvVelocity;
  int          mirvDamage;
  int          mirvArmour;
  double       mirvTurnRate;

  double specialRange;
  int    specialDamage;
  int    specialDDamage;
  double specialVelocity;

  int    specialArmour;
  int    specialDriftVelocity;
  double specialHomingRate;

  double EAS;
  double ExtraRange;
  int    ExtraDamage;

  public:
  BubalosBomber (Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);

  protected:
  virtual void calculate();
  virtual int activate_weapon();
  virtual void calculate_hotspots();
  virtual int activate_special();
  virtual int handle_damage(SpaceLocation *source, double normal, double direct);
  virtual void death();
};

class BubalosBomberFlame : public PositionedAnimation
{
  int base_frame;
  public:
  BubalosBomberFlame (SpaceLocation *creator, double ox, double oy, SpaceSprite *osprite);
  virtual void calculate();
};


class BubalosMIRV : public Missile {

  int          missileArming;
  int          missileActive;
  double       MinSplitRadius;

  double       mirvRange;
  double       mirvVelocity;
  int          mirvDamage;
  int          mirvArmour;
  double       mirvTurnRate;
  SpaceSprite  *mirvSprite;

  public:
  BubalosMIRV(double ox,double oy,double oangle, double ov,
    int odamage, double orange, int oarmour, int oarming,
    double oMinSplitRadius, Ship *oship, SpaceSprite *osprite, 
	double mv, int mdamage, double mrange, int marmour, double mtrate,
    SpaceSprite *msprite);
  virtual void calculate();
};



class BubalosHMissile : public HomingMissile {
  public:
  BubalosHMissile(double ox, double oy, double oangle, double ov,
	int odamage, double orange, int oarmour, double otrate,
	BubalosMIRV *opos, SpaceSprite *osprite);
};

class BubalosEMPSlug : public HomingMissile {
  public:
  BubalosEMPSlug(double ox, double oy, double oangle, double ov,
      int odamage,
      int oddamage, double specialDriftVelocity, double otrate,
      double orange, int oarmour, Ship *oship,
      SpaceSprite *osprite);
  virtual void inflict_damage (SpaceObject *other);
  double kick;
  int    EMPSlug_Damage;
};


BubalosBomber::BubalosBomber  (Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
        Ship(opos, shipAngle, shipData, code)
{
  weaponRange		= scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity	= scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage		= get_config_int("Weapon", "Damage", 0);
  weaponArmour		= get_config_int("Weapon", "Armour", 0);
  weaponArming		= scale_frames(get_config_float("Weapon", "Arming", 0));
  weaponMinSplitRadius  = scale_range(get_config_float("Weapon", "MinSplitRadius", 0));
  weapon_offset		= 0;

  mirvRange     = scale_range(get_config_float("Weapon", "MIRVRange", 0));
  mirvVelocity  = scale_velocity(get_config_float("Weapon", "MIRVVelocity", 0));
  mirvDamage    = get_config_int("Weapon", "MIRVDamage", 0);
  mirvArmour    = get_config_int("Weapon", "MIRVArmour", 0);
  mirvTurnRate  = get_config_float("Weapon", "MIRVTurnRate", 0);


  specialRange    = scale_range(get_config_float("Special", "Range", 0));
  specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
  specialDamage   = get_config_int("Special", "Damage", 0);
  specialDDamage  = get_config_int("Special", "DDamage", 0);
  specialArmour    = get_config_int("Special","Armour",0);
  specialDriftVelocity = get_config_int("Special", "DriftVelocity", 0);
  specialHomingRate = get_config_float("Special","Homing",0);

  flame_frame = 0;
  can_switch = true;

  EAS  = get_config_float("Extra", "EAS", 0); //Enhanced Absorber shield
  ExtraRange      = scale_range(get_config_float("Extra", "Range", 0));
  ExtraDamage     = get_config_int("Extra", "Damage",0);

  sprite_index = sprite_index % 32;

}

int BubalosBomber::activate_weapon() {
        if (fire_special) return false;

        double FireAngle, Direction, DeltaA, DetonateRange;
        double r1, r;
        if (target) r = distance(target);
        else r = 1e40;
        SpaceObject *o, *t = NULL;
        SpaceObject *PrevTarget = target;

        Query a;
        for (a.begin(this,bit(LAYER_SPECIAL),ExtraRange); a.current; a.next()) {
		o = a.currento;  r1 = distance(o);
                if (r1 > size.y)
                if ((!o->sameTeam(this)) && (r1 < r) &&
	            canCollide(o) && o->exists() && (o->ally_flag != -1)) {
                        t = o; r = r1;  }
        }

        if (t != NULL) target = t;

        if ((target != NULL) && (!target->isInvisible())) {
                DeltaA = fabs(angle - trajectory_angle(target));
                if (DeltaA > PI*3/2) DeltaA = 0; }
//                DetonateRange = distance(target); }
        else {  DeltaA = 0;
                DetonateRange = weaponRange; }

        DetonateRange = weaponRange;
        if (DeltaA <= PI/2) {
                FireAngle = angle;
                Direction = 1; }
        else {    FireAngle = angle + PI;
		  Direction = -1; }

        int wo = (weapon_offset>3)?(6-weapon_offset):weapon_offset;
        weapon_offset = (weapon_offset + 1) % 6;

        double dx;
	switch (wo) {
	case 0: dx = -12; break;
        case 1: dx = -8; break;
        case 2: dx = 8; break;
        case 3: dx = 12; }

        add(new BubalosMIRV(dx, 54 * Direction, FireAngle,weaponVelocity,
                          weaponDamage,DetonateRange,weaponArmour,
			  weaponArming,weaponMinSplitRadius,this, data->spriteWeapon,
                          mirvVelocity,mirvDamage,mirvRange,mirvArmour,mirvTurnRate,data->spriteWeapon));

        target = PrevTarget;
        return(TRUE);
}



BubalosMIRV::BubalosMIRV(double ox,double oy,double oangle, double ov,
    int odamage, double orange, int oarmour, int oarming,
    double oMinSplitRadius, Ship *oship, SpaceSprite *osprite, 
    double mv, int mdamage, double mrange, int marmour, double mtrate,
    SpaceSprite *msprite) :
	Missile(oship, Vector2(ox,oy), oangle, ov,
    odamage, orange, oarmour, oship, osprite)
{
  missileArming = oarming;
  missileActive = FALSE;
  MinSplitRadius = oMinSplitRadius;

  mirvRange    = mrange;
  mirvVelocity = mv;
  mirvDamage   = mdamage;
  mirvArmour   = marmour;
  mirvSprite   = msprite;
  mirvTurnRate = mtrate;

  explosionSprite     = data->spriteWeaponExplosion;
  explosionFrameCount = 10;
  explosionFrameSize  = 50; 
  
}


void BubalosMIRV::calculate() {

	Missile::calculate();

	if (!missileActive) {
		missileArming -= frame_time;
		if (missileArming <= 0)
			missileActive = TRUE;
	}
	else    if (target) {
		if ((distance(target) < MinSplitRadius) || (d >= range)) {
			int ArcDeg = 35;
  		        state = 0;
			for (int i=0;i < 2;i++)  {
				BubalosHMissile *BHMissile;

                                BHMissile = (new BubalosHMissile(0,0,angle+ArcDeg ,mirvVelocity,
						mirvDamage, mirvRange, mirvArmour,mirvTurnRate,
						this,mirvSprite));
				
				ArcDeg -= 70;
				add(BHMissile);
			}
		}
	}
	
	return;
	}


void BubalosBomber::death() {
	if (random() % 3) return;
	int lastHurrah;
	Query q;
	for (q.begin(this,OBJECT_LAYERS, ExtraRange); q.currento; q.next()) 
		{
		lastHurrah = (int)ceil((ExtraRange - distance(q.currento)) /
		ExtraRange * ExtraDamage);
		damage( q.currento, 0, lastHurrah);
		} //for
	add(new Animation(this, pos, data->spriteExtra, 0, 6, 50, LAYER_EXPLOSIONS));
	return;
}

int BubalosBomber::handle_damage(SpaceLocation *source, double normal, double direct) {

 if (normal > 0) {
    normal = int(normal * EAS);
    if (normal == 0) normal = 1;
    batt += normal;
    if (batt > batt_max) batt = batt_max; 
  } //if  
  return Ship::handle_damage(source, normal, direct);
 } // handle_damage


BubalosEMPSlug::BubalosEMPSlug(double ox, double oy, double oangle, double ov,
    int odamage, int oddamage, double specialDriftVelocity, double otrate,
    double orange, int oarmour, Ship *oship, SpaceSprite *osprite) :
    HomingMissile(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, otrate,
	oship, osprite, oship->target),
	
	kick(specialDriftVelocity), 
	EMPSlug_Damage(oddamage)
{
  collide_flag_sameteam  = bit(LAYER_SHIPS);
  explosionSprite     = data->spriteWeaponExplosion;
  explosionFrameCount = 10;
  explosionFrameSize  = 50; 
}


BubalosHMissile::BubalosHMissile(double ox, double oy, double oangle,double ov, 
								 int odamage, double orange, int oarmour, double otrate,
								 BubalosMIRV *opos, SpaceSprite *osprite) :
  HomingMissile(opos, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, otrate,
  opos, osprite, opos->target)
{
  collide_flag_sameteam   = bit(LAYER_SHIPS);
  explosionSprite     = data->spriteWeaponExplosion;
  explosionFrameCount = 10;
  explosionFrameSize  = 50;

}


int BubalosBomber::activate_special() {

  if (!fire_weapon) return false;

  double FireAngle;
  double Direction; 
  double DeltaA;

    if ((target != NULL) && (!target->isInvisible())) {
      DeltaA = fabs(angle - trajectory_angle(target));
      if (DeltaA > PI*3/2) DeltaA = 0;
    } else DeltaA = 0;

    if (DeltaA <= PI/2) {
          FireAngle = angle;
          Direction = 1; }
    else {FireAngle = angle + PI;
          Direction = -1;
    }

  add(new BubalosEMPSlug(
                0.0, 55 * Direction, FireAngle, specialVelocity,
                specialDamage, specialDDamage, specialDriftVelocity, specialHomingRate,
                specialRange, specialArmour, this,data->spriteSpecial));

  return(TRUE);

}


void BubalosEMPSlug::inflict_damage (SpaceObject *other) {
	if (other->mass) 
		other->accelerate (this, angle, kick / other->mass, MAX_SPEED);
        other->damage (this, 0, EMPSlug_Damage);
        Missile::inflict_damage(other);
}

void BubalosBomber::calculate_hotspots() {

  Ship::calculate_hotspots();

  if ((thrust) && (flame_frame <= 0)) {
    add(new BubalosBomberFlame(this, 20, -42, data->spriteExtraExplosion));
    add(new BubalosBomberFlame(this, -20, -42, data->spriteExtraExplosion));
    flame_frame += 100; }
  if (flame_frame > 0) flame_frame -= frame_time;

}

BubalosBomberFlame::BubalosBomberFlame (SpaceLocation *creator, double ox, double oy, SpaceSprite *osprite) :
        PositionedAnimation(creator, creator, Vector2(ox,oy), osprite, 0, 1, 100, LAYER_SHOTS)
{
        base_frame = 64*(random()%4);
        sprite_index = base_frame + get_index(follow->get_angle());
}

void BubalosBomberFlame::calculate()
{
        PositionedAnimation::calculate();
        if (state != 0)
                sprite_index = base_frame + get_index(follow->get_angle());
}

void BubalosBomber::calculate()
{
        if (fire_special && thrust) {
                if (can_switch) {
                        can_switch = false;
                        angle = normalize (angle+PI, PI2); } }
        else    can_switch = true;
        Ship::calculate();
        sprite_index = sprite_index & 31;
}


REGISTER_SHIP(BubalosBomber)
