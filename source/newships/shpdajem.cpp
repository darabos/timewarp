
#include "../ship.h"
REGISTER_FILE

#include "../melee/mframe.h"
#include "../melee/mview.h"

class DajielkaCruiser;
class DajielkaTendril;
class DajielkaSanctuary;

class DajielkaCrTorpedo : public AnimatedShot {
  double    Relativity;
  //int oorange;

  public:
  DajielkaCrTorpedo(DajielkaCruiser* ocreator, double ox, double oy, double oangle, double ov, int odamage,
    double orange, int oarmour, Ship *oship,
    SpaceSprite *osprite, double orelativity);
  double relativity;

  DajielkaCruiser* creator;
  virtual void calculate();
  virtual void inflict_damage(SpaceObject* other);
};

class DajielkaSanctuary : public SpaceObject {
  public:
  int absorbtionEnergyToAdd;
  int absorbtionEnergyIsEnabled;
  int absorbtionRegenerationIsEnabled;
  int armour;
  int maxArmour;
  int regenerationThreshhold;
  int regenerationCount;
  DajielkaCruiser* creator;
  DajielkaTendril* tendril[30];
  DajielkaSanctuary(DajielkaCruiser* ocreator);
  ~DajielkaSanctuary(void);
  DajielkaTendril* RecreateTendril(DajielkaTendril* DT);
  virtual void calculate();
  void addEnergy(int energy);
  double deltaAngle1;
  double deltaAngle2;
  double deltaAngle3;
  double deltaAngle4;
  int energyPerFrame;
  double range1;
  double range2;
  double range3;
  double range4;
  int energyLevelPerDamagePoint1;
  int energyLevelPerDamagePoint2;
  int energyLevelPerDamagePoint3;
  int energyLevelPerDamagePoint4;
  int energyLevelMax1;
  int energyLevelMax2;
  int energyLevelMax3;
  int energyLevelMax4;

  double defenseEfficiency;
  int regenerationExponent;
  int regenerationIsTendrilBased;
  virtual void inflict_damage(SpaceObject *other);
  virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

class DajielkaTendril : public SpaceLine {

public:
  int recreateMe;
  double originalLength;
  int isActive;
  int damage;
  double range;
  double rotation; // degrees per 1000 frames
  double startingAngle;
  int energyLevel;
  int energyLevelMax;
  int energyLevelPerDamagePoint;
  double defenseEfficiency;
  double regenerationIsTendrilBased;
  DajielkaCruiser* creator;
  DajielkaSanctuary* sanctuary;
  DajielkaTendril** pointerToMe;
  DajielkaTendril(DajielkaSanctuary* osanctuary, int odamage, int orange,
    double ostartingAngle, double orotation);
  ~DajielkaTendril(void);
  virtual void animate(Frame* space);
  virtual void calculate(void);
  virtual void inflict_damage(SpaceObject *other);
};


class DajielkaCruiser : public Ship {
public:
  int          accumulatedCharge;
  int          shipChargeThreshhold;
  int          accumulatedRegeneration;
  int          shipRegenerationThreshhold;
  int          redeployTime;

  double       weaponRange;
  double       weaponVelocity1;
  double       weaponVelocity2;

  double       weaponAngle1;
  double       weaponAngle2;

  double       weaponRelativity;

  int          weaponDamage;
  int          weaponArmour;
  double       weaponTurnRate;

  int          weaponToFire;

  double       specialRange1;
  double       specialRange2;
  double       specialRange3;
  double       specialRange4;
  int          specialEnergyPerFrame;

  double       specialAngle1;
  double       specialAngle2;
  double       specialAngle3;
  double       specialAngle4;

  int          specialEnergyLevelPerDamagePoint1;
  int          specialEnergyLevelPerDamagePoint2;
  int          specialEnergyLevelPerDamagePoint3;
  int          specialEnergyLevelPerDamagePoint4;

  int          specialEnergyLevelMax1;
  int          specialEnergyLevelMax2;
  int          specialEnergyLevelMax3;
  int          specialEnergyLevelMax4;
  double       specialDefenseEfficiency;
  double       specialHarvestEfficiency;
  double       specialAbsorbEfficiency;
  int          specialAbsorbEnergyEnabled;
  int          specialAbsorbRegenEnabled;
  int          specialHarvestEnergyEnabled;
  int          specialHarvestRegenEnabled;

  int          sanctuaryDamage;
  int          sanctuaryStartArmour;
  int          sanctuaryMaxArmour;
  int          sanctuaryMass;
  int          sanctuaryRegenerationThreshhold;
  int          sanctuaryRegenerationExponent;
  int          sanctuaryRegenerationIsTendrilBased;
  int          sanctuaryRedeployTime;

  public:
  DajielkaCruiser(Vector2 opos, double angle, ShipData *data, unsigned int code);
  ~DajielkaCruiser(void);
  DajielkaSanctuary* sanctuary;
  protected:
  virtual void calculate();
  virtual int activate_weapon();
  virtual int activate_special();
};


DajielkaCruiser::DajielkaCruiser(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code),
  weaponToFire(1)
	{
  shipChargeThreshhold = get_config_int("Ship", "ChargeThreshhold", 0);
  shipRegenerationThreshhold = get_config_int("Ship", "RegenerationThreshhold", 0);

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity1 = scale_velocity(get_config_float("Weapon", "Velocity1", 0));
  weaponVelocity2 = scale_velocity(get_config_float("Weapon", "Velocity2", 0));

  weaponAngle1 = get_config_float("Weapon", "Angle1", 0) * ANGLE_RATIO;
  weaponAngle2 = get_config_float("Weapon", "Angle2", 0) * ANGLE_RATIO;
  
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  weaponTurnRate = scale_turning(get_config_float("Weapon", "TurnRate", 0));
  weaponRelativity = get_config_float("Weapon", "Relativity", 0.5);

  specialEnergyPerFrame = get_config_int("Special", "EnergyPerFrame", 0);
  specialRange1    = scale_range(get_config_float("Special", "Range1", 1));
  specialRange2    = scale_range(get_config_float("Special", "Range2", 2));
  specialRange3    = scale_range(get_config_float("Special", "Range3", 3));
  specialRange4    = scale_range(get_config_float("Special", "Range4", 4));

  specialAngle1 = get_config_float("Special", "Angle1", 1);
  specialAngle2 = get_config_float("Special", "Angle2", 2);
  specialAngle3 = get_config_float("Special", "Angle3", 3);
  specialAngle4 = get_config_float("Special", "Angle4", 4);

  specialEnergyLevelPerDamagePoint1 = get_config_int("Special", "EnergyLevelPerDamagePoint1",0);
  specialEnergyLevelPerDamagePoint2 = get_config_int("Special", "EnergyLevelPerDamagePoint2",0);
  specialEnergyLevelPerDamagePoint3 = get_config_int("Special", "EnergyLevelPerDamagePoint3",0);
  specialEnergyLevelPerDamagePoint4 = get_config_int("Special", "EnergyLevelPerDamagePoint4",0);

  specialEnergyLevelMax1 = get_config_int("Special", "EnergyLevelMax1", 0);
  specialEnergyLevelMax2 = get_config_int("Special", "EnergyLevelMax2", 0);
  specialEnergyLevelMax3 = get_config_int("Special", "EnergyLevelMax3", 0);
  specialEnergyLevelMax4 = get_config_int("Special", "EnergyLevelMax4", 0);
  specialDefenseEfficiency = get_config_float("Special", "DefenseEfficiency",0);
  specialHarvestEfficiency = get_config_float("Special", "HarvestEfficiency",0);
  specialAbsorbEfficiency = get_config_float("Special", "AbsorbEfficiency", 0);

  specialAbsorbEnergyEnabled = get_config_int("Special", "AbsorbEnergyEnabled", 0);
  specialAbsorbRegenEnabled = get_config_int("Special", "AbsorbRegenEnabled", 0);
  specialHarvestEnergyEnabled = get_config_int("Special", "HarvestEnergyEnabled", 0);
  specialHarvestRegenEnabled = get_config_int("Special", "HarvestRegenEnabled" ,0);

  sanctuaryDamage = get_config_int("Sanctuary", "Damage", 0);
  sanctuaryStartArmour = get_config_int("Sanctuary", "StartArmour", 0);
  sanctuaryMaxArmour = get_config_int("Sanctuary", "MaxArmour", 0);
  sanctuaryMass = get_config_int("Sanctuary", "Mass", 0);
  sanctuaryRegenerationThreshhold = get_config_int("Sanctuary", "RegenerationThreshhold", 0);
  sanctuaryRegenerationExponent = get_config_int("Sanctuary", "RegenerationExponent",0);
  sanctuaryRegenerationIsTendrilBased = get_config_int("Sanctuary", "RegenerationIsTendrilBased", 0);
  sanctuaryRedeployTime = get_config_int("Sanctuary", "RedeployTime", 0);

  sanctuary = NULL;
  accumulatedCharge = 0;
  accumulatedRegeneration = 0;
  redeployTime = 0;
}

DajielkaCruiser::~DajielkaCruiser(void) {
	STACKTRACE
  if(sanctuary!=NULL) {
    sanctuary->state = 0;
    sanctuary->creator = NULL;
  }
}

int DajielkaCruiser::activate_special() {
	STACKTRACE
  if(sanctuary!=NULL) {
    redeployTime += frame_time;
    //message.print (1500, 9, "RDT = %d SRDT = %d", redeployTime, sanctuaryRedeployTime);
    //tw_error("Redeploy increment!");
    if(redeployTime>sanctuaryRedeployTime) {
    //if(TRUE) {
      sanctuary->state = 0; // line OK.
      redeployTime = 0;
    }
  }
  if(sanctuary==NULL || sanctuary->state==0) {
    sanctuary = new DajielkaSanctuary(this);
    sanctuary->energyPerFrame = specialEnergyPerFrame;
    sanctuary->regenerationExponent = sanctuaryRegenerationExponent;
  //sanctuary->regenerationIsTendrilBased = sanctuaryRegenerationIsTendrilBased;
	  game->add(sanctuary);
  }
	return(FALSE);
}

void DajielkaCruiser::calculate(void) {
	STACKTRACE
  if(!fire_special) redeployTime=0;
  if(accumulatedCharge>shipChargeThreshhold) {
    if(batt<batt_max) {
      batt++;
      update_panel = TRUE;
      accumulatedCharge -= shipChargeThreshhold;
      if(batt>batt_max)
        batt=batt_max;
    }
    else
      accumulatedCharge = shipChargeThreshhold;
  }
  if(accumulatedRegeneration>shipRegenerationThreshhold) {
    if(crew<crew_max) {
      crew++;
      update_panel = TRUE;
      accumulatedRegeneration -= shipRegenerationThreshhold;
      if(crew>crew_max)
        crew=crew_max;
    }
    else
      accumulatedRegeneration = shipRegenerationThreshhold;
  }

  Ship::calculate();
}

int DajielkaCruiser::activate_weapon() {
	STACKTRACE
  switch(weaponToFire)
  {
  case 1:
	game->add(new DajielkaCrTorpedo(this, size.y * 0.15, size.y *0.7, angle-weaponAngle1, weaponVelocity1,
			weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon,
			weaponRelativity));
  break;
  case 2:
	game->add(new DajielkaCrTorpedo(this, size.y * -0.3, size.y *0.6, angle+weaponAngle2, weaponVelocity2,
			weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon,
			weaponRelativity));
  break;
  case 3:
	game->add(new DajielkaCrTorpedo(this, size.y * -0.15, size.y *0.7, angle+weaponAngle2, weaponVelocity2,
			weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon,
			weaponRelativity));
  break;
  case 4:
	game->add(new DajielkaCrTorpedo(this, size.y * 0.3, size.y *0.6,angle-weaponAngle1, weaponVelocity1,
			weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon,
			weaponRelativity));
  break;
  default:
    weaponToFire = 1;
  }
  if(++weaponToFire > 4) weaponToFire = 1;
	return(TRUE);
}

DajielkaCrTorpedo::DajielkaCrTorpedo(DajielkaCruiser* ocreator, double ox, double oy, double oangle, double ov,
  int odamage, double orange, int oarmour, Ship *oship,
  SpaceSprite *osprite, double orelativity) :
  AnimatedShot(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oship,
    osprite, 27, 15, orelativity),
  relativity(orelativity),
  creator(ocreator)
{
  explosionSprite     = data->spriteWeaponExplosion;
  collide_flag_anyone = ALL_LAYERS;
  collide_flag_sameteam = bit(LAYER_LINES);
  collide_flag_sameship = bit(LAYER_LINES);

  //oorange=orange;
}

void DajielkaCrTorpedo::calculate()
{
	STACKTRACE
	AnimatedShot::calculate();
}

void DajielkaCrTorpedo::inflict_damage(SpaceObject* other) {
	STACKTRACE
  //if(other==creator)return;
  AnimatedShot::inflict_damage(other);
}

DajielkaSanctuary::DajielkaSanctuary(DajielkaCruiser* ocreator)
  :   
  creator(ocreator),
  SpaceObject((SpaceLocation*) ocreator, 
    ocreator->normal_pos(), ocreator->angle, ocreator->data->spriteSpecial)
{
  DajielkaTendril* DT;
  int i,j;
  collide_flag_anyone = ALL_LAYERS;
  collide_flag_sameteam = bit(LAYER_SHIPS)|bit(LAYER_SHOTS);
  collide_flag_sameship = bit(LAYER_SHIPS)|bit(LAYER_SHOTS);
  defenseEfficiency = creator->specialDefenseEfficiency;
  damage_factor = creator->sanctuaryDamage;
  mass = creator->sanctuaryMass;
  pos -= unit_vector(creator->get_angle()) * scale_range(1);
  armour = creator->sanctuaryStartArmour;
  maxArmour = creator->sanctuaryMaxArmour;
  regenerationThreshhold = creator->sanctuaryRegenerationThreshhold;
  regenerationCount = 0;
  range1 = creator->specialRange1;
  range2 = creator->specialRange2;
  range3 = creator->specialRange3;
  range4 = creator->specialRange4;
  deltaAngle1 = creator->specialAngle1;
  deltaAngle2 = creator->specialAngle2;
  deltaAngle3 = creator->specialAngle3;
  deltaAngle4 = creator->specialAngle4;
  energyLevelMax1 = creator->specialEnergyLevelMax1;
  energyLevelMax2 = creator->specialEnergyLevelMax2;
  energyLevelMax3 = creator->specialEnergyLevelMax3;
  energyLevelMax4 = creator->specialEnergyLevelMax4;
  energyLevelPerDamagePoint1 = creator->specialEnergyLevelPerDamagePoint1;
  energyLevelPerDamagePoint2 = creator->specialEnergyLevelPerDamagePoint2;
  energyLevelPerDamagePoint3 = creator->specialEnergyLevelPerDamagePoint3;
  energyLevelPerDamagePoint4 = creator->specialEnergyLevelPerDamagePoint4;
  regenerationIsTendrilBased = creator->sanctuaryRegenerationIsTendrilBased;

  for(i=0; i<3; i++) {
    DT = new DajielkaTendril(this, 1, range1, (PI2/3) * i, (double)deltaAngle1);
    DT->energyLevelMax = this->energyLevelMax1;
    DT->energyLevelPerDamagePoint = this->energyLevelPerDamagePoint1;
    DT->defenseEfficiency = this->defenseEfficiency;
    tendril[i]=DT;
    game->add(DT);
  }
  for(i=3, j=0; i<8; i++,j++) {
    DT = new DajielkaTendril(this, 1, range2, (PI2/5) * (double)(2*j), (double)deltaAngle2);
    DT->energyLevelMax = this->energyLevelMax2;
    DT->energyLevelPerDamagePoint = this->energyLevelPerDamagePoint2;
    tendril[i]=DT;
    DT->defenseEfficiency = this->defenseEfficiency;
    game->add(DT);
  }
  for(i=8, j=0; i<15; i++,j++) {
    DT = new DajielkaTendril(this, 1, range3, (PI2 / 7.0) * (double)(3*j), deltaAngle3);
    DT->energyLevelMax = this->energyLevelMax3;
    DT->energyLevelPerDamagePoint = this->energyLevelPerDamagePoint3;
    tendril[i]=DT;
    DT->defenseEfficiency = this->defenseEfficiency;
    game->add(DT);
  }
  for(i=15, j=0; i<30; i++,j++) {
    DT = new DajielkaTendril(this, 1, range4, (PI2 / 15.0) * (double)(4*j), deltaAngle4);
    DT->energyLevelMax = this->energyLevelMax4;
    DT->energyLevelPerDamagePoint = this->energyLevelPerDamagePoint4;
    tendril[i]=DT;
    DT->defenseEfficiency = this->defenseEfficiency;
    game->add(DT);
  }
  for(i=0;i<30;i++) {
    tendril[i]->regenerationIsTendrilBased = this->regenerationIsTendrilBased;
    tendril[i]->pointerToMe = &tendril[i];
  }
}

DajielkaSanctuary::~DajielkaSanctuary(void) {
	STACKTRACE
  int i;
  for(i=0; i<30; i++) {
    if(tendril[i]!=NULL) {
      tendril[i]->state = 0;
      tendril[i]->sanctuary = NULL;
    }
  }
}

void DajielkaSanctuary::calculate(void) {
	STACKTRACE
  int i,j;
  int oldSpriteIndex;
  int regenMultiplier;
  DajielkaSanctuary::addEnergy(energyPerFrame*frame_time/25.0); //corrected for click-dependant regeneration. Tau.
  for(i=0;i<30;i++)
    if(tendril[i]!=NULL)
      if(tendril[i]->recreateMe = TRUE) {
        //tw_error("Want to recreate!");
        //tendril[i] = this->RecreateTendril(tendril[i]);
      }
  vel = 0;
  SpaceObject::calculate();
  regenMultiplier=1;
  for(j=0;j<regenerationExponent;j++)
    regenMultiplier *= armour;
  if(!regenerationIsTendrilBased)
    regenerationCount += frame_time;
  if(regenerationCount>(regenerationThreshhold*regenMultiplier))
    if(armour<maxArmour && creator!=NULL) {
      //tw_error("armour=%d regenMultiplier=%d",armour,regenMultiplier);
      armour++;
      regenerationCount -= (regenerationThreshhold*regenMultiplier);
    }
    else
      regenerationCount=0;
    oldSpriteIndex = sprite_index;
    sprite_index = (int)((((double)armour / (double)maxArmour) * 10)+0.2);
    if(sprite_index<0) sprite_index=0;
    if(sprite_index>9) sprite_index=9;
    if(oldSpriteIndex!=sprite_index&&oldSpriteIndex<sprite_index)
      creator->play_sound2(creator->data->sampleSpecial[creator->special_sample]);

}

void DajielkaSanctuary::inflict_damage(SpaceObject *other){
	STACKTRACE
  int old_damage_factor;
  old_damage_factor = damage_factor;
  if(other==creator) damage_factor=0;;
  SpaceObject::inflict_damage(other);
  if(other==creator) damage_factor = old_damage_factor;
  //else AnimatedShot::inflict_damage(other);
  vel=0;
}

int DajielkaSanctuary::handle_damage(SpaceLocation *source, double normal, double direct){
	STACKTRACE
  armour -= normal + direct;
  if(armour<=0) state=0;
  if(normal+direct>0) regenerationCount /= 2;
  return(normal+direct);
}


void DajielkaSanctuary::addEnergy(int energy) {
	STACKTRACE
  int i, j;
  int damageBefore, damageAfter;
  for(j=1;j<10 && energy>0;j++)
    for(i=0;i<30 && energy>0;i++) {
      damageBefore = (int)(tendril[i]->energyLevel/tendril[i]->energyLevelPerDamagePoint);
      if(tendril[i]!=NULL && tendril[i]->state !=0)
        if(tendril[i]->energyLevelPerDamagePoint * j > tendril[i]->energyLevel
          && tendril[i]->energyLevel < tendril[i]->energyLevelMax) {
          tendril[i]->energyLevel += energy;
          energy = 0;
        }
      damageAfter = (int)(tendril[i]->energyLevel/tendril[i]->energyLevelPerDamagePoint);
      if(damageBefore!=damageAfter && tendril[i]->get_length()==0) {
        //tendril[i]=this->RecreateTendril(tendril[i]);
        tendril[i]->isActive = TRUE;
      }
    }
}

DajielkaTendril* DajielkaSanctuary::RecreateTendril(DajielkaTendril* DT) {
	STACKTRACE
  DajielkaTendril* DTN;
  if(DT==NULL) {
    tw_error("Error: Null Tendril.");
    return NULL;
  }
  if(DT->state = 0)
    return NULL;
  DTN = new DajielkaTendril(this, DT->damage, DT->originalLength,
    DT->angle, DT->rotation);
  DTN->creator = DT->creator;
  DTN->energyLevel = DT->energyLevel;
  DTN->energyLevelMax = DT->energyLevelMax;
  DTN->energyLevelPerDamagePoint = DT->energyLevelPerDamagePoint;
  DTN->originalLength = DT->originalLength;
  DTN->pointerToMe = DT->pointerToMe;
  //*(DTN->pointerToMe) = DTN; //shouldn't be needed... handled by the calling routine.
  DTN->range = DT->range;
  DTN->recreateMe = FALSE;
  DTN->rotation = DT->rotation;
  DTN->sanctuary = DT->sanctuary;
  DT->sanctuary=NULL; DT->creator = NULL; DT->state = 0;
  return(DTN);
}

DajielkaTendril::DajielkaTendril(DajielkaSanctuary* osanctuary, int odamage, int orange,
    double ostartingAngle, double orotation) :
  sanctuary(osanctuary),
  creator(osanctuary->creator),
  damage(odamage),
  range(orange),
  originalLength(orange),
  rotation(orotation*ANGLE_RATIO),
  startingAngle(ostartingAngle),
  SpaceLine(osanctuary->creator, osanctuary->normal_pos(),ostartingAngle, orange, palette_color[8])
{
  isActive = FALSE;
  energyLevel = 0;
  energyLevelPerDamagePoint = 100;
  energyLevelMax = 500;
  recreateMe = FALSE;
	collide_flag_sameship = 0;//bit(LAYER_SHIPS) | bit(LAYER_SHOTS);
  collide_flag_sameteam = 0;
  collide_flag_sameship = 0;

}

void DajielkaTendril::calculate(void) {
	STACKTRACE
  if(energyLevel<energyLevelPerDamagePoint) {
    isActive = FALSE;
    length = 0;
    damage_factor = 0;
    collide_flag_anyone = 0;
    collide_flag_sameteam = 0;
    collide_flag_sameship = 0;
  }
  else {
    isActive = TRUE;
    length = originalLength;
    damage_factor = 1;
    collide_flag_anyone = ALL_LAYERS;
    collide_flag_sameteam = bit(LAYER_SHIPS)|bit(LAYER_SHOTS);
    collide_flag_sameship = bit(LAYER_SHIPS)|bit(LAYER_SHOTS);
  }
  angle = angle + frame_time / 1000.0 * rotation;
  while(angle<0) angle+=PI2;
  while(angle>PI2) angle-=PI2;
  SpaceLine::calculate();
  color=palette_color[(int)(energyLevel/energyLevelPerDamagePoint)+7];
  if(creator!=NULL)
    if(isActive)
      if(regenerationIsTendrilBased==1)
        sanctuary->regenerationCount += frame_time;
}

void DajielkaTendril::animate(Frame *space) {
	STACKTRACE
  if(!isActive) return;
  //length = originalLength;
  SpaceLine::animate(space);
}

void DajielkaTendril::inflict_damage(SpaceObject *other) {
	STACKTRACE
  int oldStats, newStats;
  if(other==creator && other!=NULL) {
    creator->redeployTime = 0;
    if(creator->batt<creator->batt_max && creator->specialAbsorbEnergyEnabled) {
      creator->redeployTime = 0;
      creator->accumulatedCharge += frame_time * creator->specialAbsorbEfficiency;
    }
    if(creator->crew<creator->crew_max && creator->batt>=creator->batt_max && creator->specialAbsorbRegenEnabled) { //only when batteries full
      creator->accumulatedRegeneration += frame_time * creator->specialAbsorbEfficiency;
      creator->redeployTime = 0;
    }
    //if(creator->batt<creator->batt_max && creator->specialHarvestEnergyEnabled && creator->fire_special) {
    if(creator->fire_special && (creator->specialHarvestEnergyEnabled||creator->specialHarvestRegenEnabled)) {
      if(creator->batt<creator->batt_max && creator->specialHarvestEnergyEnabled) {
        creator->accumulatedCharge += this->energyLevel * creator->specialHarvestEfficiency;
        creator->redeployTime = 0;
        this->energyLevel = 0;
        this->isActive = 0;
      }
      else if (creator->crew<creator->crew_max && creator->specialHarvestRegenEnabled) {
        creator->accumulatedRegeneration += this->energyLevel * creator->specialHarvestEfficiency;
        creator->redeployTime = 0;
        this->energyLevel = 0;
        this->isActive = 0;
      }
    }
    //if(creator->crew<creator->crew_max && creator->batt>=creator->batt_max && creator->specialHarvestRegenEnabled && creator->fire_special) {//only when batteries full
    if(FALSE) {//only when batteries full
      //creator->redeployTime = 0;
      this->energyLevel = 0;
      this->isActive = 0;
    }
    collide_flag_anyone = ALL_LAYERS;
    collide_flag_sameteam = bit(LAYER_SHIPS);
    collide_flag_sameship = bit(LAYER_SHIPS);
    return;
  }
  if(other->isShip()) oldStats = ((Ship*)other)->crew + ((Ship*)other)->batt;
  else if(other->isShot()) oldStats = ((Shot*)other)->armour + ((Shot*)other)->damage_factor;
  else oldStats = 1;
  if(energyLevel>=energyLevelPerDamagePoint) {
    damage_factor = 1;
    SpaceLine::inflict_damage(other);
    if(other->isShip()) newStats = ((Ship*)other)->crew + ((Ship*)other)->batt;
    else if(other->isShot()) newStats = ((Shot*)other)->armour + ((Shot*)other)->damage_factor;
    else newStats = 0;
    if(other->isShip() && oldStats!=newStats)
      energyLevel -= energyLevelPerDamagePoint;
    else if(oldStats!=newStats)
      energyLevel -= (int)((double)energyLevelPerDamagePoint / (double)defenseEfficiency);
  }
  length = originalLength;
  if(energyLevel>=energyLevelPerDamagePoint) {
    isActive = TRUE;
    recreateMe = TRUE; //maybe this line needs killing???
  }
  else
    isActive = FALSE;
}

DajielkaTendril::~DajielkaTendril(void) {
  if(sanctuary!=NULL)
    if(creator!=NULL)
      *pointerToMe = NULL;
}


REGISTER_SHIP ( DajielkaCruiser )
