#include "../ship.h"
REGISTER_FILE

//#include "../sc1ships.h"

class GlavriaCrTorpedo : public AnimatedShot {
  //int       passive;
  //double    passiveRange;
  double    Relativity;
  int oorange;

  public:
  GlavriaCrTorpedo(Vector2 opos, double oangle, double ov, int odamage,
    double orange, int oarmour, Ship *oship,
    SpaceSprite *osprite, int ofcount, int ofsize, double orelativity);
  virtual void calculate();
};

class GlavriaCruiser : public Ship {
public:
  double       weaponRange;
  double       weaponVelocity1;
  double       weaponVelocity2;
  double       weaponVelocity3;
  double       weaponVelocity4;
  double       weaponVelocity5;
  double       weaponAngle1;
  double       weaponAngle2;
  double       weaponAngle3;
  double       weaponAngle4;
  double       weaponAngle5;
  double       weaponRelativity;

  int          weaponDamage;
  int          weaponArmour;
  double       weaponTurnRate;

  double specialRange;
  int    specialFrames;

  double       specialVelocity;
  int          specialDamage;
  int          specialArmour;
  double       specialRelativity;

  public:
  GlavriaCruiser(Vector2 opos, double angle, ShipData *data, unsigned int code);

  protected:
  virtual int activate_weapon();
  virtual int activate_special();
};


GlavriaCruiser::GlavriaCruiser(Vector2 opos, double angle, ShipData *data, unsigned int code)
	:
	Ship(opos, angle, data, code)
{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity1 = scale_velocity(get_config_float("Weapon", "Velocity1", 0));
  weaponVelocity2 = scale_velocity(get_config_float("Weapon", "Velocity2", 0));
  weaponVelocity3 = scale_velocity(get_config_float("Weapon", "Velocity3", 0));
  weaponVelocity4 = scale_velocity(get_config_float("Weapon", "Velocity4", 0));
  weaponVelocity5 = scale_velocity(get_config_float("Weapon", "Velocity5", 0));
  weaponAngle1 = get_config_float("Weapon", "Angle1", 0) * ANGLE_RATIO;
  weaponAngle2 = get_config_float("Weapon", "Angle2", 0) * ANGLE_RATIO;
  weaponAngle3 = get_config_float("Weapon", "Angle3", 0) * ANGLE_RATIO;
  weaponAngle4 = get_config_float("Weapon", "Angle4", 0) * ANGLE_RATIO;
  weaponAngle5 = get_config_float("Weapon", "Angle5", 0) * ANGLE_RATIO;

  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  weaponTurnRate = scale_turning(get_config_float("Weapon", "TurnRate", 0));
  weaponRelativity = get_config_float("Weapon", "Relativity", 0.5);

  specialFrames = get_config_int("Special", "Frames", 0);
  specialRange    = scale_range(get_config_float("Special", "Range", 0));
  specialDamage   = get_config_int("Special", "Damage", 0);
  specialArmour =   get_config_int("Special", "Armour", 0);
  specialRelativity = get_config_float("Special", "Relativity", 0.5);
  specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));

}



int GlavriaCruiser::activate_special() {
	game->add(new GlavriaCrTorpedo(Vector2(0.0, size.y *(-0.3)), angle+PI, specialVelocity,
			specialDamage, specialRange, specialArmour, this, data->spriteWeapon,
			100, 16, specialRelativity));
	return(TRUE);
	}

int GlavriaCruiser::activate_weapon() {
	game->add(new GlavriaCrTorpedo(Vector2(0.0,size.y*0.3), angle+weaponAngle1, weaponVelocity1,
			weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon,
			100, 16, weaponRelativity));
	game->add(new GlavriaCrTorpedo(Vector2(0.0, size.y *0.3), angle+weaponAngle2, weaponVelocity2,
			weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon,
			100, 16, weaponRelativity));
	game->add(new GlavriaCrTorpedo(Vector2(0.0, size.y *0.3), angle+weaponAngle3, weaponVelocity3,
			weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon,
			100, 16, weaponRelativity));
	game->add(new GlavriaCrTorpedo(Vector2(0.0, size.y *0.3), angle+weaponAngle4, weaponVelocity4,
			weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon,
			100, 16, weaponRelativity));
	game->add(new GlavriaCrTorpedo(Vector2(0.0, size.y *0.3), angle+weaponAngle5, weaponVelocity5,
			weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon,
			100, 16, weaponRelativity));
	return(TRUE);
	}

GlavriaCrTorpedo::GlavriaCrTorpedo(Vector2 opos, double oangle, double ov,
  int odamage, double orange, int oarmour, Ship *oship,
  SpaceSprite *osprite, int ofcount, int ofsize, double orelativity) :
  AnimatedShot(oship, opos, oangle, ov, odamage, orange, oarmour, oship,
    osprite, 16, 45, orelativity)
{
  explosionSprite     = data->spriteWeaponExplosion;
  explosionFrameCount = 20;
  explosionFrameSize  = 40;
  oorange=orange;
  //vx = vx + ddx;
  //vy = vy + ddy;
}


void GlavriaCrTorpedo::calculate()
  {
	AnimatedShot::calculate();
	//d += v * frame_time;
	//if(d >= oorange) state = 0;
  }

REGISTER_SHIP ( GlavriaCruiser )

