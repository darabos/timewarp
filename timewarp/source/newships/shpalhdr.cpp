#include "../ship.h"
#include "../melee/mview.h"

REGISTER_FILE


class AlhordianLaserSweep;

class AlhordianDreadnought : public Ship {
public:
  AlhordianLaserSweep* ALS1;
  AlhordianLaserSweep* ALS2;

  int currentSweepTime;
  int sweepIsOn;

  double weaponRange;
  double weaponVelocity;
  int    weaponDamage;
  int    weaponArmour;
  double weaponRelativity;

  double flashAngle1, flashRange1, flashDamage1;
  double flashAngle2, flashRange2, flashDamage2;
  double flashAngle3, flashRange3, flashDamage3;
  double flashAngle4, flashRange4, flashDamage4;
  double flashAngle5, flashRange5, flashDamage5;
  double flashAngle6, flashRange6, flashDamage6;
  double flashAngle7, flashRange7, flashDamage7;
  int flashColor;

  double specialStartX, specialStartY;
  double specialEndX, specialEndY;
  double specialStartAngle, specialEndAngle;
  double specialStartRange, specialEndRange;
  //double specialStartLength, specialEndLength;
  double specialTime;
  double specialDamage;

  public:
  AlhordianDreadnought(Vector2 opos, double angle, ShipData *data, unsigned int code);
  ~AlhordianDreadnought(void);
  protected:
  virtual int activate_weapon();
  virtual int activate_special();
  virtual void calculate();
  void weapon_flash();
  void calculate_laser_sweep();

  private:

  int weaponSpriteNumber;
  int justBorn;
  double shipSpecialDrain;
  double shipWeaponDrain;
};

class AlhordianDrBolt : public Missile {
	public:
	AlhordianDrBolt(double ox, double oy, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship,
      SpaceSprite *osprite, double orelativity);
	int spriteToUse;
	virtual void calculate();
	};

class AlhordianLaserSweep : public SpaceLocation {
  int currentTime;
  AlhordianDreadnought* creator;
  double startX, startY;
  double endX, endY;
  double startAngle, endAngle;
  double startLength, endLength;
  double lifetime; double damage;
  double Height;
public:
  AlhordianLaserSweep** pointerToMe;
  AlhordianLaserSweep(AlhordianDreadnought* ocreator,
    double ostartX, double ostartY, double ostartAngle, double ostartLength,
    double oendX, double oendY, double oendAngle, double oendLength,
    int oTime, int oDamage);
  virtual void calculate();
};


AlhordianDreadnought::AlhordianDreadnought(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
	{
  shipWeaponDrain = get_config_int("Ship", "WeaponDrain",0);
  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  specialDamage = get_config_int("Special", "Damage", 0);
  shipSpecialDrain = get_config_int("Ship", "SpecialDrain",0);
  weaponRelativity = get_config_float("Weapon", "Relativity", 0);

  flashColor = get_config_int("Flash", "Color", 7);
  flashAngle1 = get_config_float("Flash", "Angle1", 0) * ANGLE_RATIO;
  flashRange1 = scale_range(get_config_float("Flash", "Range1", 0));
  flashAngle2 = get_config_float("Flash", "Angle2", 0) * ANGLE_RATIO;
  flashRange2 = scale_range(get_config_float("Flash", "Range2", 0));
  flashAngle3 = get_config_float("Flash", "Angle3", 0) * ANGLE_RATIO;
  flashRange3 = scale_range(get_config_float("Flash", "Range3", 0));
  flashAngle4 = get_config_float("Flash", "Angle4", 0) * ANGLE_RATIO;
  flashRange4 = scale_range(get_config_float("Flash", "Range4", 0));
  flashAngle5 = get_config_float("Flash", "Angle5", 0) * ANGLE_RATIO;
  flashRange5 = scale_range(get_config_float("Flash", "Range5", 0));
  flashAngle6 = get_config_float("Flash", "Angle6", 0) * ANGLE_RATIO;
  flashRange6 = scale_range(get_config_float("Flash", "Range6", 0));
  flashAngle7 = get_config_float("Flash", "Angle7", 0) * ANGLE_RATIO;
  flashRange7 = scale_range(get_config_float("Flash", "Range7", 0));
  flashDamage1 = get_config_int("Flash", "Damage1", 0);
  flashDamage2 = get_config_int("Flash", "Damage2", 0);
  flashDamage3 = get_config_int("Flash", "Damage3", 0);
  flashDamage4 = get_config_int("Flash", "Damage4", 0);
  flashDamage5 = get_config_int("Flash", "Damage5", 0);
  flashDamage6 = get_config_int("Flash", "Damage6", 0);
  flashDamage7 = get_config_int("Flash", "Damage7", 0);

  specialStartX = get_config_float("Special", "StartX", 0);
  specialStartY = get_config_float("Special", "StartY", 0);
  specialEndX = get_config_float("Special", "EndX", 0);
  specialEndY = get_config_float("Special", "EndY", 0);
  specialStartAngle = get_config_float("Special", "StartAngle", 0) * ANGLE_RATIO;
  specialEndAngle = get_config_float("Special", "EndAngle", 0) * ANGLE_RATIO;
  specialStartRange = scale_range(get_config_float("Special", "StartRange", 0));
  specialEndRange = scale_range(get_config_float("Special", "EndRange", 0));
  specialTime = get_config_int("Special", "Time", 0);
  specialDamage = get_config_int("Special", "Damage", 0);

  currentSweepTime = specialTime + 1;
  sweepIsOn = FALSE;
  //justBorn = TRUE;
}

AlhordianDreadnought::~AlhordianDreadnought(void) {
  ;
}

void AlhordianDreadnought::calculate() {
  Ship::calculate();
  calculate_laser_sweep();
}


int AlhordianDreadnought::activate_weapon() {
  weaponSpriteNumber = 2;
  if(weaponSpriteNumber>3) weaponSpriteNumber=3;
  if(weaponSpriteNumber<0) weaponSpriteNumber=0;
  game->add(new AlhordianDrBolt(size.y*(0.0), (size.y * 0.1),
    angle, weaponVelocity, weaponDamage, weaponRange, 
	  weaponArmour, this, data->spriteWeapon, weaponRelativity));
  this->weapon_flash();
  return(TRUE);
}

int AlhordianDreadnought::activate_special() {
  if((!sweepIsOn)&&fire_special) {
    sweepIsOn = TRUE;
    currentSweepTime = 0;
	  return(TRUE);
  }
  else
    return(FALSE);
}

void AlhordianDreadnought::weapon_flash() {
//Laser::Laser(SpaceLocation *creator, double langle, int lcolor, double lrange, int ldamage,
//  int lfcount, SpaceLocation *opos, double rel_x, double rel_y, bool osinc_angle) 
  game->add(new Laser(this, angle+flashAngle1, pallete_color[flashColor],
    flashRange1, flashDamage1, 50, this, Vector2(size.y *0.0, size.y*0.5)));
  game->add(new Laser(this, angle-flashAngle1, pallete_color[flashColor],
    flashRange1, flashDamage1, 50, this, Vector2(size.y *0.0, size.y*0.5)));

  game->add(new Laser(this, angle+flashAngle2, pallete_color[flashColor],
    flashRange2, flashDamage2, 50, this, Vector2(size.y *0.0, size.y*0.5)));
  game->add(new Laser(this, angle-flashAngle2, pallete_color[flashColor],
    flashRange2, flashDamage2, 50, this, Vector2(size.y *0.0, size.y*0.5)));

  game->add(new Laser(this, angle+flashAngle3, pallete_color[flashColor],
    flashRange3, flashDamage3, 50, this, Vector2(size.y *0.0, size.y*0.5)));
  game->add(new Laser(this, angle-flashAngle3, pallete_color[flashColor],
    flashRange3, flashDamage3, 50, this, Vector2(size.y *0.0, size.y*0.5)));

  game->add(new Laser(this, angle+flashAngle4, pallete_color[flashColor],
    flashRange4, flashDamage4, 50, this, Vector2(size.y *0.0, size.y*0.5)));
  game->add(new Laser(this, angle-flashAngle4, pallete_color[flashColor],
    flashRange4, flashDamage4, 50, this, Vector2(size.y *0.0, size.y*0.5)));

  game->add(new Laser(this, angle+flashAngle5, pallete_color[flashColor],
    flashRange5, flashDamage5, 50, this, Vector2(size.y *0.0, size.y*0.5)));
  game->add(new Laser(this, angle-flashAngle5, pallete_color[flashColor],
    flashRange5, flashDamage5, 50, this, Vector2(size.y *0.0, size.y*0.5)));

  game->add(new Laser(this, angle+flashAngle6, pallete_color[flashColor],
    flashRange6, flashDamage6, 50, this, Vector2(size.y *0.0, size.y*0.5)));
  game->add(new Laser(this, angle-flashAngle6, pallete_color[flashColor],
    flashRange6, flashDamage6, 50, this, Vector2(size.y *0.0, size.y*0.5)));

  game->add(new Laser(this, angle+flashAngle7, pallete_color[flashColor],
    flashRange7, flashDamage7, 50, this, Vector2(size.y *0.0, size.y*0.5)));
  game->add(new Laser(this, angle-flashAngle7, pallete_color[flashColor],
    flashRange7, flashDamage7, 50, this, Vector2(size.y *0.0, size.y*0.5)));
}

AlhordianDrBolt::AlhordianDrBolt(double ox, double oy, double oangle, double ov,
	int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite, double orelativity) 
	:
	Missile(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oship, osprite, orelativity),
	spriteToUse(odamage)
{
	explosionSprite     = data->spriteWeaponExplosion;
}


void AlhordianDrBolt::calculate()
{
	Missile::calculate();
}

AlhordianLaserSweep::AlhordianLaserSweep(AlhordianDreadnought* ocreator,
    double ostartX, double ostartY, double ostartAngle, double ostartLength,
    double oendX, double oendY, double oendAngle, double oendLength,
    int oTime, int oDamage) 
    :
    SpaceLocation(ocreator, ocreator->normal_pos(), ocreator->angle),
    creator(ocreator),
    startX(ostartX), startY(ostartY),
    endX(oendX), endY(oendY),
    startAngle(ostartAngle), endAngle(oendAngle),
    startLength(ostartLength), endLength(oendLength),
    lifetime(oTime), damage(oDamage)
{
  currentTime = 0;
  Height = creator->size.y;
}

void AlhordianLaserSweep::calculate(void) {
  double fractionDone;
  double X, Y, Angle, Length;
  if(currentTime>lifetime) {state = 0; return;}
  if(creator)
    if(creator->exists())
      {pos = creator->normal_pos(); angle = creator->get_angle();}
    else
      {state = 0; return;}
  else
    {state = 0; return;}
  fractionDone = currentTime / lifetime;
  X = startX * (1-fractionDone) + endX * fractionDone;
  Y = startY * (1-fractionDone) + endY * fractionDone;
  Angle = startAngle * (1-fractionDone) + endAngle * (fractionDone);
  Length = startLength * (1-fractionDone) + endLength * (fractionDone);
  //Length = scale_range(10); //debugging override
  game->add(new Laser(this, Angle+angle, palette_color[9], Length, 1, 25,
    this, Vector2(Height * X, Height * Y), TRUE));
//Laser::Laser(SpaceLocation *creator, double langle, int lcolor, double lrange, int ldamage,
//  int lfcount, SpaceLocation *opos, double rel_x, double rel_y, bool osinc_angle) 
  currentTime += frame_time;
  if(currentTime>lifetime) state=0;
}

void AlhordianDreadnought::calculate_laser_sweep(void) {
  double fractionDone;
  double X, Y, Angle, Length;
  //if(justBorn) {justBorn = FALSE; currentSweepTime = 0; sweepIsOn = FALSE; return;}
  if(currentSweepTime>specialTime) {sweepIsOn = FALSE; return;}
  fractionDone = currentSweepTime / specialTime;
  X = specialStartX * (1-fractionDone) + specialEndX * fractionDone;
  Y = specialStartY * (1-fractionDone) + specialEndY * fractionDone;
  Angle = specialStartAngle * (1-fractionDone) + specialEndAngle * (fractionDone);
  Length = specialStartRange * (1-fractionDone) + specialEndRange * (fractionDone);
  //Length = scale_range(10); //debugging override
  game->add(new Laser(this, Angle+angle, palette_color[9], Length, 1*frame_time/25.0, 25,
    this, Vector2(size.y * X, size.y * Y), TRUE));
  game->add(new Laser(this, -Angle+angle, palette_color[9], Length, 1*frame_time/25.0, 25,
    this, Vector2(-size.y * X, size.y * Y), TRUE));
//Laser::Laser(SpaceLocation *creator, double langle, int lcolor, double lrange, int ldamage,
//  int lfcount, SpaceLocation *opos, double rel_x, double rel_y, bool osinc_angle) 
  currentSweepTime += frame_time;
  if(currentSweepTime>specialTime) sweepIsOn = FALSE;
}




REGISTER_SHIP (AlhordianDreadnought)