#include "../ship.h"
REGISTER_FILE

//#include "../sc1ships.h"

class PhedarPaTorpedo : public AnimatedShot {
  //int       passive;
  //double    passiveRange;
  double    Relativity;
  int oorange;

  public:
  PhedarPaTorpedo(Vector2 opos, double oangle, double ov, int odamage,
    double orange, int oarmour, Ship *oship,
    SpaceSprite *osprite, int ofcount, int ofsize, Vector2 Dd);

  virtual void calculate();
  //void disengage();
};


class PhedarPatrolShip : public Ship {
public:
  double       weaponRange;
  double       weaponVelocity1;
  int          weaponDamage;
  int          weaponArmour;
  double       weaponTurnRate;
  double       weaponRelativity;

  int    specialColor;
  double specialRange;
  int    specialFrames;

  double       specialVelocity;
  int          specialDamage;
  int          specialArmour;
  double       specialMaxLaserDeflection;


  public:
  PhedarPatrolShip(Vector2 opos, double angle, ShipData *data, unsigned int code);

  protected:
  virtual int activate_weapon();
  virtual int activate_special();
};


PhedarPatrolShip::PhedarPatrolShip(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
	{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity1 = scale_velocity(get_config_float("Weapon", "Velocity1", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  weaponTurnRate = scale_turning(get_config_float("Weapon", "TurnRate", 0));
  weaponRelativity = get_config_float("Weapon", "Relativity", 0.5);

  specialColor  = get_config_int("Special", "Color", 0);
  specialFrames = get_config_int("Special", "Frames", 0);
  specialRange    = scale_range(get_config_float("Special", "Range", 0));
  specialDamage   = get_config_int("Special", "Damage", 0);
  specialMaxLaserDeflection = get_config_int("Special", "MaxLaserDeflection", 0);
}



int PhedarPatrolShip::activate_special() {
  SpaceObject *o = NULL;

  double r = 99999;  

	Query a;
	for (a.begin(this, bit(LAYER_SHIPS), specialRange + 20); a.current; a.next()) {
		if ((distance(a.current) < r) && !a.current->isInvisible() ) {
			o = a.currento;
			r = distance(o);
			}
		}

  if (o) r = trajectory_angle(o); else r = angle;
  if(abs(r-angle)>specialMaxLaserDeflection) r = angle;

  game->add(new Laser(this, r, pallete_color[specialColor], 
    specialRange, specialDamage, specialFrames, this, Vector2(0, 0) ));
  return TRUE;
	}

int PhedarPatrolShip::activate_weapon() {
	game->add(new PhedarPaTorpedo(Vector2(0.0, get_size().y*0.5), angle, weaponVelocity1,
			weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon,
			16, 10, vel * weaponRelativity));
	return(TRUE);
	}

PhedarPaTorpedo::PhedarPaTorpedo(Vector2 opos, double oangle, double ov,
  int odamage, double orange, int oarmour, Ship *oship,
  SpaceSprite *osprite, int ofcount, int ofsize, Vector2 Dd) :
  AnimatedShot(oship, opos, oangle, ov, odamage, -1.0, oarmour, oship,
    osprite, ofcount, ofsize)
{
  explosionSprite     = data->spriteWeaponExplosion;
  explosionFrameCount = 20;
  explosionFrameSize  = 40;
  oorange=orange;
//  vx = vx + ddx;
//  vy = vy + ddy;
  vel += Dd;
}


  void PhedarPaTorpedo::calculate()
  {
	SpaceObject::calculate();
	d += v * frame_time;
	if(d >= oorange) state = 0;
  }


REGISTER_SHIP ( PhedarPatrolShip )
