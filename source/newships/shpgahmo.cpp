#include "../ship.h"
#include "../melee/mview.h"

REGISTER_FILE


class GahmurMonitor : public Ship {
public:
  double       shipWeaponDrainPerSecond;
  double       shipSpecialInitialDrain;
  double       shipSpecialDrainPerSecond;
  double       shipSpecialLockedDrainPerSecond;

  int          weaponMinChargeTime;
  int          weaponMaxChargeTime;
  double       weaponMinChargeRange;
  double       weaponMaxChargeRange;
  double       weaponMinChargeBeamRange;
  double       weaponMaxChargeBeamRange;
  double       weaponMinChargeVelocity;
  double       weaponMaxChargeVelocity;
  double       weaponSpeedChangeFactor;
  double       weaponMinChargeDamage;
  double       weaponMaxChargeDamage;
  double       weaponMinChargeBeamDamage;
  double       weaponMaxChargeBeamDamage;
	double       weaponRange;
	double       weaponVelocity;
	double       weaponDamage;
	double       weaponHome;
  double       weaponMinBatteryToCharge;
  double       weaponMinChargeBeamDivergenceAngle;
  double       weaponMaxChargeBeamDivergenceAngle;
  int          weaponBeamEnabled;
  int          weaponStopsDynamo;
  double       weaponVulnerabilityFactor;
  int          weaponAlwaysFireOnEmptyBattery;
  int          weaponAlwaysFireOnMaxChargeTime;

	double       specialNormalDamagePerSecond;
  double       specialDirectDamagePerSecond;
  double       specialLengthMultiplier;
  double       specialPowerMultiplier;
  double       specialDecayMultiplier;
  int          specialStopsDynamo;
  double       specialInitialEnergy;
  double       specialMaxLength;
  double       specialMaxEnergy;
  double       specialVelocityCouplingFactor;

  bool         isCharging;
  bool         wasFiredTooEarly;
  bool         isLaunching;
  bool         isBeaming;
  int          chargingTime;
  int          oldChargingTime;
  int          tractorLockTime;
  int          oldTractorLockTime;
  bool         isTractoring;
  double       tractorPower;
  double       tractorLength;
  double       tractorAngle;
  int          tractorSpriteIndex;
  int          last_fire_special;
  int          last_fire_weapon;
  SpaceObject* tractorTarget;
  SpaceObject* oldTractorTarget;
  SpaceObject* holdoverTractorTarget;
  SpaceLocation* tractorSource;
  double       defaultFiringAngle;

	public:
	GahmurMonitor(Vector2 opos, double angle, ShipData *data, unsigned int code);

  virtual void PowerTractor();
  virtual void DecayTractor();
  virtual void CalculateTractor();
  virtual void CalculateTractorEffect();
  virtual void LaunchPlasma();
  virtual void BeamPlasma();
	virtual int activate_weapon();
	virtual int activate_special();
  virtual void calculate();
  virtual void death();
  virtual bool die();
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);

};

class GahmurPlasma : public HomingMissile {

	static SpaceSprite *spriteWeaponExplosion;
	int frame_count;
	int max_damage;
  int sprite_index_override;

	public:
  double vulnerabilityFactor;

  double speedChangeFactor;
	GahmurPlasma(Vector2 opos, double oangle, double ov, int odamage,
		double orange, double otrate, Ship *oship, SpaceSprite *osprite, int ofcount);

  virtual void SetMaxDamage(double omaxDamage);
	virtual void calculate();
  
	virtual void inflict_damage(SpaceObject *other);
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
  void animate (Frame *frame);

};

class GahmurTractor:public Laser {
public:
  GahmurTractor(GahmurMonitor *ocreator, double langle, int lcolor, double lrange, double ldamage,
  int lfcount, SpaceLocation *opos, Vector2 rpos, bool osinc_angle);
	virtual void inflict_damage(SpaceObject *other);
  virtual void collide(SpaceObject *other);
  GahmurMonitor* creator;
  int explosionSpriteIndex;
  double normalDamagePerSecond;
  double directDamagePerSecond;
};

GahmurTractor::GahmurTractor(GahmurMonitor *ocreator, double langle, int lcolor, double lrange, double ldamage,
  int lfcount, SpaceLocation *opos, Vector2 rpos, bool osinc_angle)
:
Laser(ocreator, langle, lcolor, lrange, ldamage,
  lfcount, opos, rpos, osinc_angle)
{
  this->creator = ocreator;
  this->normalDamagePerSecond = ocreator->specialNormalDamagePerSecond;
  this->directDamagePerSecond = ocreator->specialDirectDamagePerSecond;
	this->set_depth(DEPTH_SPECIAL);
  this->data->spriteSpecialExplosion = creator->data->spriteSpecialExplosion;
  this->explosionSpriteIndex = 0;
}

void GahmurTractor::inflict_damage(SpaceObject* other) {
	STACKTRACE
  double x1, x2;
  x1 = (this->normalDamagePerSecond / 1000.0) * (double)frame_time;
  x2 = (this->directDamagePerSecond / 1000.0) * (double)frame_time;
  damage(other, x1, x2);
  if(creator->exists())
    if(creator->tractorTarget==NULL) {
      creator->tractorTarget = other;
      creator->tractorLength = other->distance(creator->tractorSource);
      creator->tractorAngle = creator->tractorSource->trajectory_angle(other);
    }
    else {
      if(creator->distance(creator->tractorTarget) > creator->distance(other)) {
        creator->tractorTarget = other;
        creator->holdoverTractorTarget = other;
        if(other->state==0) creator->isTractoring = false;
      }
      else {
        creator->holdoverTractorTarget = creator->tractorTarget;;
        if(!creator->tractorTarget->exists()) creator->isTractoring = false;
      }
    }
	collide_flag_anyone = collide_flag_sameship = collide_flag_sameteam = 0;
	physics->add(new Animation( this, 
			pos + edge(), this->data->spriteSpecialExplosion, this->explosionSpriteIndex, 
  		1, 15, DEPTH_EXPLOSIONS));

	return;
  //note:  Laser::inflict_damage is NOT called!
}

void GahmurTractor::collide(SpaceObject* other) {
	STACKTRACE
  if(other->isShot()) return;
  if(other->isLine()) return;
  Laser::collide(other);
}



GahmurMonitor::GahmurMonitor(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
{
  shipWeaponDrainPerSecond =  get_config_float("Ship", "WeaponDrainPerSecond", 0);
  shipSpecialInitialDrain = get_config_float("Ship", "SpecialInitialDrain", 0);
  shipSpecialDrainPerSecond = get_config_float("Ship", "SpecialDrainPerSecond", 0);
  shipSpecialLockedDrainPerSecond = get_config_float("Ship", "SpecialLockedDrainPerSecond", 0);

  weaponMinChargeRange    = scale_range(get_config_float("Weapon", "MinChargeRange", 0));
  weaponMaxChargeRange    = scale_range(get_config_float("Weapon", "MaxChargeRange", 0));
  weaponMinChargeBeamRange= scale_range(get_config_float("Weapon", "MinChargeBeamRange", 0));
  weaponMaxChargeBeamRange= scale_range(get_config_float("Weapon", "MaxChargeBeamRange", 0));

  weaponMinChargeVelocity = scale_velocity(get_config_float("Weapon", "MinChargeVelocity", 0));
  weaponMaxChargeVelocity = scale_velocity(get_config_float("Weapon", "MaxChargeVelocity", 0));

  weaponMinChargeDamage =   get_config_float("Weapon", "MinChargeDamage", 0);
  weaponMaxChargeDamage =   get_config_float("Weapon", "MaxChargeDamage", 0);
  weaponMinChargeBeamDamage = get_config_float("Weapon", "MinChargeBeamDamage", 0);
  weaponMaxChargeBeamDamage = get_config_float("Weapon", "MaxChargeBeamDamage", 0);

  weaponSpeedChangeFactor = get_config_float("Weapon", "SpeedChangeFactor", 0.0);
  weaponMinChargeTime =     get_config_int("Weapon", "MinChargeTime", 0);
  weaponMaxChargeTime =     get_config_int("Weapon", "MaxChargeTime", 0);
  weaponMinBatteryToCharge = get_config_float("Weapon", "MinBatteryToCharge", 0);
  weaponMinChargeBeamDivergenceAngle = get_config_float("Weapon", "MinChargeBeamDivergenceAngle", 0) * PI / 180;
  weaponMaxChargeBeamDivergenceAngle = get_config_float("Weapon", "MaxChargeBeamDivergenceAngle", 0) * PI / 180;
  weaponBeamEnabled = get_config_int("Weapon", "BeamEnabled", 0);
  weaponStopsDynamo =       get_config_int("Weapon", "StopsDynamo", 0);
  weaponHome     =          scale_turning(get_config_float("Weapon", "Homing", 0));
  weaponVulnerabilityFactor = get_config_float("Weapon", "VulnerabilityFactor", 0);
  weaponAlwaysFireOnEmptyBattery = get_config_int("Weapon", "AlwaysFireOnEmptyBattery", 0);
  weaponAlwaysFireOnMaxChargeTime = get_config_int("Weapon", "AlwaysFireOnMaxChargeTime", 0);


  specialNormalDamagePerSecond  = get_config_float("Special", "NormalDamagePerSecond", 0);
  specialDirectDamagePerSecond  = get_config_float("Special", "DirectDamagePerSecond", 0);
  specialLengthMultiplier = get_config_float("Special", "LengthMultiplier", 1.0);
  specialPowerMultiplier =  get_config_float("Special", "PowerMultiplier", 1.0);
  specialDecayMultiplier =  get_config_float("Special", "DecayMultiplier", 1.0);
  specialStopsDynamo =      get_config_int("Special", "StopsDynamo", 0);
  specialInitialEnergy =   (get_config_float("Special", "InitialEnergy", 0));
  specialMaxLength =        scale_range(get_config_float("Special", "MaxLength", 0));
  specialMaxEnergy =        get_config_float("Special", "MaxEnergy", 0);
  specialVelocityCouplingFactor = get_config_float("Special", "VelocityCouplingFactor", 0);

  isCharging = false;
  isLaunching = false;
  isBeaming = false;
  isTractoring = false;
  chargingTime = 0;
  oldChargingTime = 0;
  tractorLockTime = 0;
  oldTractorLockTime = 0;
  tractorPower = 0;
  tractorLength = 0;
  tractorAngle = 0;
  tractorTarget = NULL;
  oldTractorTarget = NULL;
  holdoverTractorTarget = NULL;
  tractorSource = new SpaceLocation(this, Vector2(0,0),0);
  tractorSpriteIndex = 0;
  last_fire_special = 0;
  last_fire_weapon = 0;
  wasFiredTooEarly = false;
  defaultFiringAngle = 0.0001;
}

int GahmurMonitor::activate_weapon() {
  return(FALSE); //entirely handled in the calculate section.
}

int GahmurMonitor::activate_special()
{
  return(FALSE); //entirely handled in the calculate section.
}


void GahmurMonitor::calculate() {
	STACKTRACE
  double requiredWeaponEnergy;
  double requiredSpecialEnergy;
  if(this->state==0) {
    this->LaunchPlasma();
    Ship::calculate();
    return;
  }
  requiredWeaponEnergy = (((double)this->shipWeaponDrainPerSecond / 1000.0) * (double)frame_time);
  requiredSpecialEnergy = (((double)this->shipSpecialDrainPerSecond / 1000.0) * (double)frame_time);

  if(((fire_weapon==0)||(weaponBeamEnabled==0))&&(fire_special==1)&&(last_fire_special==0)) {
    if(this->batt>=this->shipSpecialInitialDrain || isTractoring) {
      isTractoring = !isTractoring;
      if(isTractoring) {
        this->batt -= this->shipSpecialInitialDrain;
        this->tractorPower = this->specialInitialEnergy;
        this->tractorAngle = this->angle;
      }
    }
  }

  if((fire_weapon==1) && (last_fire_weapon==0) && (this->batt>this->weaponMinBatteryToCharge)) {
    isCharging = true;
    isLaunching = false;
    isBeaming = false;
    if(!wasFiredTooEarly) chargingTime = 0; //maybe need THIS line!
  }

  if(((fire_weapon==0) && (last_fire_weapon==1))||wasFiredTooEarly) {
    if((this->chargingTime>=this->weaponMinChargeTime)&&isCharging) {
      isCharging = false;
      if(fire_special && weaponBeamEnabled) {
        isBeaming = true;
        isLaunching = false;
        wasFiredTooEarly = false;
      }
      else {
        isBeaming = false;
        isLaunching = true;
        wasFiredTooEarly = false;
      }
    }
    else
    {
      if(isCharging) {
        wasFiredTooEarly = true;
        isCharging = true;
      }
    }
  }

  if(isCharging)
    if(this->batt>requiredWeaponEnergy && this->chargingTime < this->weaponMaxChargeTime) {
      chargingTime += frame_time;
      this->batt -= requiredWeaponEnergy;
      this->update_panel = TRUE;
    }
    else {
      if(fire_weapon==0) { // a kludge
        isCharging = false;
        wasFiredTooEarly = false;
      }
      if(weaponAlwaysFireOnEmptyBattery) {
        isCharging = false;
        wasFiredTooEarly = false;
        if(fire_special) {
          isLaunching = false;
          isBeaming = true;
        }
        else {
          isLaunching = true;
          isBeaming = false;
        }
      }
    }
    else { //not charging
      //chargingTime = 0;
    }
    if((int)chargingTime/(int)350 != (int)oldChargingTime/(int)350) {
      double fracDone = (double)this->chargingTime / (double)this->weaponMaxChargeTime;
      play_sound2((this->data->sampleWeapon[0]), 150 + 150 * fracDone, 1000.0 + 300 * (fracDone-.5));
    }
  if(chargingTime>=this->weaponMaxChargeTime) {
    chargingTime = this->weaponMaxChargeTime;
    if(weaponAlwaysFireOnMaxChargeTime) {
      isCharging = false;
      wasFiredTooEarly = false;
      if(fire_special) {
        isLaunching = false;
        isBeaming = true;
      }
      else {
        isLaunching = true;
        isBeaming = false;
      }
    }
  }

  if(isLaunching) {
    this->LaunchPlasma();
    this->isLaunching = false;
    this->isBeaming = false;
    this->isCharging = false;
  }
  if(isBeaming) {
    this->BeamPlasma();
    this->isLaunching = false;
    this->isBeaming = false;
    this->isCharging = false;
  }
  if(isCharging==false) {
    chargingTime = 0.0;
    oldChargingTime = -1000.0;
  }
  if(isCharging&&weaponStopsDynamo)recharge_step = recharge_rate; //should freeze the dynamo
  if(isTractoring&&specialStopsDynamo)recharge_step = recharge_rate; //should freeze the dynamo

  if(isTractoring)
    this->PowerTractor();
  else
    this->DecayTractor();

  this->CalculateTractor();
  this->CalculateTractorEffect();
  last_fire_special = fire_special;
  last_fire_weapon = fire_weapon;
  oldChargingTime = chargingTime;
  holdoverTractorTarget = NULL;
  Ship::calculate();
}

void GahmurMonitor::LaunchPlasma()
{
	STACKTRACE
  GahmurPlasma* GP;
  double ta, rta, fracDone;
  Vector2 SV, TV;
  if(this->chargingTime<this->weaponMinChargeTime) {
    this->chargingTime = 0;
    return; //flushed...
  }
  if(this->weaponMinChargeTime == this->weaponMaxChargeTime) fracDone = 0.5;
  else fracDone = (double)(chargingTime - this->weaponMinChargeTime) / (double)(this->weaponMaxChargeTime - this->weaponMinChargeTime);
  if(fracDone<0.00) fracDone = 0.00;
  if(fracDone>1.00) fracDone = 1.00;
  weaponDamage = (1.0-fracDone) * this->weaponMinChargeDamage + fracDone * this->weaponMaxChargeDamage;
  weaponRange = (1.0-fracDone) * this->weaponMinChargeRange + fracDone * this->weaponMaxChargeRange;
  weaponVelocity = (1.0-fracDone) * this->weaponMinChargeVelocity + fracDone * this->weaponMaxChargeVelocity;

  if(target)ta = this->trajectory_angle(this->target);
  else {
    ta = this->angle + defaultFiringAngle;
    defaultFiringAngle = -defaultFiringAngle;
  }
  rta = this->angle - ta;
  while(rta<0) rta += PI2;
  while(rta>PI2) rta-= PI2;
  if(rta>PI) SV = unit_vector(0) * size.y * 0.24;
  else SV = unit_vector(PI) * size.y * 0.24;
  TV = unit_vector(-ta+this->angle+PI/2) * size.y * 0.20; 
  GP = (new GahmurPlasma(TV+SV, ta, 
    weaponVelocity, weaponDamage, weaponRange, weaponHome, this,
    data->spriteWeapon, 64));
  GP->speedChangeFactor = this->weaponSpeedChangeFactor;
  GP->SetMaxDamage(weaponDamage);
  GP->vulnerabilityFactor = this->weaponVulnerabilityFactor;
  game->add(GP);
  this->chargingTime = 0;
  this->oldChargingTime = -1000.0;
  this->isCharging = false;
  play_sound2((this->data->sampleWeapon[1]), 120.0 + 400.0 * fracDone, 1500.0 - (1000.0 * fracDone));

}

void GahmurMonitor::BeamPlasma()
{
	STACKTRACE
  double fracDone;
  double weaponBeamDivergenceAngle;
  Vector2 SV, TV;
  if(this->chargingTime<this->weaponMinChargeTime) {
    this->chargingTime = 0;
    return; //flushed...
  }
  if(this->weaponMinChargeTime == this->weaponMaxChargeTime) fracDone = 0.5;
  else fracDone = (double)(chargingTime - this->weaponMinChargeTime) / (double)(this->weaponMaxChargeTime - this->weaponMinChargeTime);
  if(fracDone<0.00) fracDone = 0.00;
  if(fracDone>1.00) fracDone = 1.00;
  weaponDamage = (1.0-fracDone) * this->weaponMinChargeBeamDamage + fracDone * this->weaponMaxChargeBeamDamage;
  weaponRange = (1.0-fracDone) * this->weaponMinChargeBeamRange + fracDone * this->weaponMaxChargeBeamRange;
  weaponBeamDivergenceAngle = (1.0-fracDone) * this->weaponMinChargeBeamDivergenceAngle + fracDone * this->weaponMaxChargeBeamDivergenceAngle;
  game->add(new Laser(this, this->angle+weaponBeamDivergenceAngle, pallete_color[5], weaponRange, weaponDamage,
    100, this, Vector2(size.x * 0.27, size.x * 0.13), true));
  game->add(new Laser(this, this->angle-weaponBeamDivergenceAngle, pallete_color[5], weaponRange, weaponDamage,
    100, this, Vector2(size.x * -0.27, size.x * 0.13), true));
  this->chargingTime = 0;
  this->oldChargingTime = -1000.0;
  this->isCharging = false;
  play_sound2((this->data->sampleExtra[0]), 150 + 150 * fracDone, 1000.0 * (1.5 - fracDone));
}



void GahmurMonitor::PowerTractor() {
	STACKTRACE
  double energy;
  if(this->tractorTarget) {
    this->tractorLockTime += frame_time;
    energy = (this->shipSpecialLockedDrainPerSecond / 1000.0) * (double)frame_time;
  }
  else {
    energy = (this->shipSpecialDrainPerSecond / 1000.0) * (double)frame_time;
    this->tractorLockTime = 0;  this->oldTractorLockTime = 0;
  }
  if(this->batt>energy) {
    this->tractorPower += (this->specialPowerMultiplier / 1000.0) * (double)frame_time;
    this->batt -= energy;
  }
  else
    isTractoring = false;
  if(this->tractorPower>this->specialMaxEnergy)
    this->tractorPower=this->specialMaxEnergy;
}

void GahmurMonitor::DecayTractor() {
	STACKTRACE
  this->tractorPower -= (this->specialDecayMultiplier / 1000.0) * (double)frame_time;
  if(this->tractorPower<0.0) {
    this->tractorLockTime = 0;
    this->tractorLength = 0;
    this->tractorPower = 0.0;
    this->oldTractorTarget = NULL;
    this->holdoverTractorTarget = NULL;
    this->tractorTarget = NULL;
    this->tractorAngle = 0;
    this->isTractoring = false;
  }
}

void GahmurMonitor::CalculateTractor() {
	STACKTRACE
  GahmurTractor* GT;
  this->tractorSource->pos = this->pos + unit_vector(this->angle) * this->size.y * 0.3;
  if(this->tractorPower<0.0001) return;
  tractorLength = scale_range(1) * this->tractorPower * this->specialLengthMultiplier;
  if(tractorLength>this->specialMaxLength)
    tractorLength=this->specialMaxLength;
  if(tractorTarget!=NULL) {
    if(tractorTarget->exists())tractorAngle = this->tractorSource->trajectory_angle(tractorTarget);
    else isTractoring = false;
    GT = new GahmurTractor(this, this->tractorAngle, pallete_color[6+(int)isTractoring], this->tractorLength, 0,
      50, this->tractorSource, Vector2(0,0), true);
    if(tractorTarget->exists())
      if(this->tractorSource->distance(this->tractorTarget)>this->tractorLength + scale_range(0.5))
        this->tractorTarget = NULL;
    game->add(GT);
    if(oldTractorTarget!=tractorTarget)
      play_sound2((this->data->sampleSpecial[0]));
    if((++this->tractorSpriteIndex)>5) this->tractorSpriteIndex = 0;
    GT->explosionSpriteIndex = this->tractorSpriteIndex;
  }
  else { //tractorTarget == NULL here
    GT = new GahmurTractor(this, this->angle, pallete_color[9+(int)isTractoring], this->tractorLength, 0,
      50, this->tractorSource, Vector2(0,0), true);
    tractorAngle = this->angle;
    game->add(GT);
    if(oldTractorTarget) this->isTractoring = false;
  }
  GT->normalDamagePerSecond = this->specialNormalDamagePerSecond;
  GT->directDamagePerSecond = this->specialDirectDamagePerSecond;
  oldTractorTarget = tractorTarget;
  if((this->oldTractorLockTime/1000 != this->tractorLockTime/1000) &&(holdoverTractorTarget)) {
      play_sound2((this->data->sampleSpecial[1]), 200);
  }
  this->oldTractorLockTime = this->tractorLockTime;
}

void GahmurMonitor::CalculateTractorEffect() {
	STACKTRACE
  Vector2 TM;
  double tmass;
  double vtransfer;
  if(tractorTarget==NULL || (!tractorTarget->exists())) {
    //tractorLockTime = 0; //added?
    //oldTractorLockTime = 0; //added???
    //oldTractorTarget = NULL; //added??
    //holdoverTractorTarget = NULL;
    //tractorTarget = NULL;
    //isTractoring = false;
    return;
  }
  if(tractorTarget->isShot()) {
    tractorTarget->pos = this->pos + unit_vector(tractorAngle) * tractorLength;
    tractorTarget->vel = this->vel;
    return;
  }
  if(tractorTarget->mass>0.01) {
    //tractorTarget->pos = this->pos + unit_vector(tractorAngle) * tractorLength;
    tmass = this->mass + tractorTarget->mass;
    TM = (this->vel * this->mass + tractorTarget->vel * tractorTarget->mass)/tmass;
    vtransfer = (this->specialVelocityCouplingFactor * frame_time) / (1+(this->specialVelocityCouplingFactor * frame_time));
    this->vel = this->vel * (1-vtransfer) + TM * vtransfer;
    tractorTarget->vel = tractorTarget->vel * (1-vtransfer) + TM * vtransfer;
    return;
  }
}

int GahmurMonitor::handle_damage(SpaceLocation* source, double normal, double direct) {
	STACKTRACE
  int x;
  x = Ship::handle_damage(source, normal, direct);
  if(this->crew<=0.0) {
    this->LaunchPlasma();
  }
  return x;
}

void GahmurMonitor::death() {
	STACKTRACE
  //message.print(1000,10,"death!!!!!");
  //this->LaunchPlasma();
  Ship::death();
}

bool GahmurMonitor::die() {
	STACKTRACE
  //message.print(1000,11,"die!!!!!");
  //this->LaunchPlasma();
  return Ship::die();
}


SpaceSprite *GahmurPlasma::spriteWeaponExplosion = NULL;
GahmurPlasma::GahmurPlasma(Vector2 opos, double oangle, double ov,
    int odamage, double orange, double otrate, Ship *oship,
    SpaceSprite *osprite, int ofcount) :
  HomingMissile( oship, opos, oangle, ov, odamage, orange, 0, otrate, oship, 
      osprite, oship->target),
  frame_count(ofcount),
  max_damage(odamage)
{
	spriteWeaponExplosion = data->spriteWeaponExplosion;
  speedChangeFactor = 0.0;
  this->attributes &= ~ATTRIB_STANDARD_INDEX;
  sprite_index_override = 0;
  vulnerabilityFactor = 0.0;
}

void GahmurPlasma::calculate()
{
	STACKTRACE
  int damageFactorSprite;
  int base_sprite_index;
  HomingMissile::calculate();
	this->v *= 1 + this->speedChangeFactor * game->frame_time; //half of the friction formula... no range change, though.
	this->range = this->d + (this->range - this->d) * (1 + this->speedChangeFactor * game->frame_time); //here's the friction modifier for range changes.
  //note, a positive value is acceleration, a negative value deceleration!
  base_sprite_index = (int)(angle / (PI2/64)) + 16;
	base_sprite_index &= 63;
  damage_factor = max_damage - (int)((d / range) * (double)(max_damage));
  damageFactorSprite = (int)(damage_factor + 0.5);
  if(damageFactorSprite>14) damageFactorSprite = 14;
  if(damageFactorSprite<0) damageFactorSprite = 0;
  sprite_index_override = damageFactorSprite * 64 + base_sprite_index;
  sprite_index = sprite_index_override;
}

void GahmurPlasma::inflict_damage(SpaceObject *other)
{
	STACKTRACE
  SpaceObject::inflict_damage(other);
  if (!other->isShot()) {
	  if (other->exists()) {
		  add(new FixedAnimation(this, other,
				spriteWeaponExplosion, 0, 20, 50, DEPTH_EXPLOSIONS));
	  }
	  else {
		  add(new Animation(this, other->normal_pos(), 
				spriteWeaponExplosion, 0, 20, 50, DEPTH_EXPLOSIONS));
	  }
    state = 0;
  }
}

int GahmurPlasma::handle_damage(SpaceLocation *source, double normal, double direct) {
	STACKTRACE
	double total = (normal + direct)*this->vulnerabilityFactor;

	if (total) {
		d += total / max_damage * range;
		if (d >= range) state = 0;
		}
	return 1;
	}


void GahmurPlasma::animate (Frame *frame) {
	STACKTRACE
  sprite_index = sprite_index_override;
  //if there is a better way, TELL ME!!!
  Shot::animate(frame);
}

void GahmurPlasma::SetMaxDamage(double omaxDamage) {
  this->max_damage = omaxDamage;
}

REGISTER_SHIP(GahmurMonitor)
