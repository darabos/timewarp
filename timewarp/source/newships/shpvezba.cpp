#include "../ship.h"
REGISTER_FILE

//#include "../sc1ships.h"

class VezlagariBarge : public Ship {
public:
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;
  double       weaponTurnRate;
  double weaponMinRangeBracket;
  double weaponMaxTurn1;
  double weaponMaxRangeBracket1;
  double weaponMaxTurn2;
  double weaponMaxRangeBracket2;
  double weaponMaxTurn3;
  double weaponMaxRangeBracket3;
  double weaponMaxTurn4;
  double weaponMaxRangeBracket4;
  double weaponPercentForward;
  double weaponPercentLeft;

  int    specialColor;
  double specialRange;
  double specialMinRange;
  double specialAddRange;
  int    specialFrames;
  int    specialMinFrames;
  int    specialAddFrames;
  int    specialDamage;
  int    specialMinDamage;
  int    specialAddDamage;

  public:
  VezlagariBarge(Vector2 opos, double angle, ShipData *data, unsigned int code);

  protected:
  virtual int activate_weapon();
  virtual int activate_special();
};

class VezlagariMissile : public Missile {
	public:
	VezlagariMissile(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
			double minRangeBracket, double maxTurn1, double maxRangeBracket1, 
			double maxTurn2, double maxRangeBracket2,
			double maxTurn3, double maxRangeBracket3,
			double maxTurn4, double maxRangeBracket4);
	double startingAngle;
	virtual void calculate();
	  double Turn1;
  double RangeBreak1;
  double Turn2;
  double RangeBreak2;
  double Turn3;
  double RangeBreak3;
  double Turn4;
  double RangeBreak4;
  double MinRangeBracket, StartingAngle;
  double MaxTurn1, MaxTurn2, MaxTurn3, MaxTurn4;
  double MaxRangeBracket1, MaxRangeBracket2, MaxRangeBracket3, MaxRangeBracket4;
	};


VezlagariBarge::VezlagariBarge(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
	{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  weaponTurnRate = scale_turning(get_config_float("Weapon", "TurnRate", 0));
  weaponMinRangeBracket = get_config_float("Weapon", "MinRangeBracket", 0);
  weaponMaxTurn1 = get_config_float("Weapon", "TurnRange1", 0);
  weaponMaxRangeBracket1 = get_config_float("Weapon", "MaxRangeBracket1", 0);
  weaponMaxTurn2 = get_config_float("Weapon", "TurnRange2", 0);
  weaponMaxRangeBracket2 = get_config_float("Weapon", "MaxRangeBracket2", 0);
  weaponMaxTurn3 = get_config_float("Weapon", "TurnRange3", 0);
  weaponMaxRangeBracket3 = get_config_float("Weapon", "MaxRangeBracket3", 0);
  weaponMaxTurn4 = get_config_float("Weapon", "TurnRange4", 0);
  weaponMaxRangeBracket4 = get_config_float("Weapon", "MaxRangeBracket4", 0);
  weaponPercentForward = get_config_int("Weapon", "PercentForward", 0);
  weaponPercentLeft = get_config_int("Weapon", "PercentLeft", 0);


  specialColor  = get_config_int("Special", "Color", 0);
  specialRange  = scale_range(get_config_float("Special", "Range", 0));
  specialFrames = get_config_int("Special", "Frames", 0);
  specialDamage = get_config_int("Special", "Damage", 0);
  specialMinRange  = scale_range(get_config_float("Special", "MinRange", 0));
  specialMinFrames = get_config_int("Special", "MinFrames", 0);
  specialMinDamage = get_config_int("Special", "MinDamage", 0);
  specialAddRange  = scale_range(get_config_float("Special", "AddRange", 0));
  specialAddFrames = get_config_int("Special", "AddFrames", 0);
  specialAddDamage = get_config_int("Special", "AddDamage", 0);

}

int VezlagariBarge::activate_weapon() {
	double xMult,yMult;
	double angleMod;
	if(random()%100<weaponPercentForward) {yMult=1;angleMod=0;} 
	else {yMult=-1; angleMod=PI;}
	if(random()%100>=weaponPercentLeft) xMult=1; else xMult=-1;
  game->add(new VezlagariMissile(
    Vector2(get_size().x * 0.30 * xMult, get_size().y * 0.2 * yMult), angle+angleMod, weaponVelocity, weaponDamage, weaponRange,
    weaponArmour, this, data->spriteWeapon, weaponMinRangeBracket,
	weaponMaxTurn1, weaponMaxRangeBracket1,
	weaponMaxTurn2, weaponMaxRangeBracket2,
	weaponMaxTurn3, weaponMaxRangeBracket3,
	weaponMaxTurn4, weaponMaxRangeBracket4));
  return(TRUE);
}

int VezlagariBarge::activate_special() {
	double rangeL, damageL, framesL;
	rangeL = (double) (random()%(int)specialAddRange) + specialMinRange;
	damageL = (double) (random()%specialAddDamage) + specialMinDamage;
	framesL = (double) (random()%specialAddFrames) + specialMinFrames;

	game->add(new Laser(this, random(PI2), pallete_color[specialColor], 
    rangeL, damageL, framesL, this, Vector2(0, get_size().y*(-0.3)), true));
	return(TRUE);
	}

VezlagariMissile::VezlagariMissile(Vector2 opos, double oangle, double ov,
	int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
	double minRangeBracket, double maxTurn1, double maxRangeBracket1, 
	double maxTurn2, double maxRangeBracket2,
	double maxTurn3, double maxRangeBracket3,
	double maxTurn4, double maxRangeBracket4) 
	:
	Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship,osprite),
	StartingAngle(oangle),
	MinRangeBracket(minRangeBracket),
	MaxTurn1(maxTurn1), MaxRangeBracket1(maxRangeBracket1),
	MaxTurn2(maxTurn2), MaxRangeBracket2(maxRangeBracket2),
	MaxTurn3(maxTurn3), MaxRangeBracket3(maxRangeBracket3),
	MaxTurn4(maxTurn4), MaxRangeBracket4(maxRangeBracket4)

	{
	double MaxTurn1, MaxTurn2, MaxTurn3, MaxTurn4;
	double MaxRangeBracket1, MaxRangeBracket2, MaxRangeBracket3, MaxRangeBracket4;
	startingAngle=oangle;
	MaxRangeBracket1=maxRangeBracket1;MaxTurn1=maxTurn1;
	MaxRangeBracket2=maxRangeBracket2;MaxTurn2=maxTurn2;
	MaxRangeBracket3=maxRangeBracket3;MaxTurn3=maxTurn3;
	MaxRangeBracket4=maxRangeBracket4;MaxTurn4=maxTurn4;
	explosionSprite     = data->spriteWeaponExplosion;
	RangeBreak1 = (double)(random() % (int)(MaxRangeBracket1 * 1000)) / 1000;
	RangeBreak2 = (double)(random() % (int)(MaxRangeBracket2 * 1000)) / 1000;
	RangeBreak3 = (double)(random() % (int)(MaxRangeBracket3 * 1000)) / 1000;
	RangeBreak4 = (double)(random() % (int)(MaxRangeBracket4 * 1000)) / 1000;
	Turn1 = (double)(random() % (int)(MaxTurn1 * 2)) - MaxTurn1;
	Turn2 = (double)(random() % (int)(MaxTurn2 * 2)) - MaxTurn2;
	Turn3 = (double)(random() % (int)(MaxTurn3 * 2)) - MaxTurn3;
	Turn4 = (double)(random() % (int)(MaxTurn4 * 2)) - MaxTurn4;

	Turn1 *= ANGLE_RATIO;
	Turn2 *= ANGLE_RATIO;
	Turn3 *= ANGLE_RATIO;
	Turn4 *= ANGLE_RATIO;

	}

	void VezlagariMissile::calculate()
	{
		Missile::calculate();
		double totalDist;
		double fracDist;
		totalDist = d / range;
		//Missile::changeDirection(PI/2);
		if(totalDist<RangeBreak1){
			fracDist = totalDist / RangeBreak1;
			Missile::changeDirection(startingAngle + (Turn1 * fracDist));
		}
		else if(totalDist<(RangeBreak1+RangeBreak2)) {
			fracDist = (totalDist-RangeBreak1) / RangeBreak2;
			Missile::changeDirection(startingAngle + Turn1 + (Turn2 * fracDist));
		}
		else if(totalDist<(RangeBreak1+RangeBreak2+RangeBreak3)) {
			fracDist = (totalDist-RangeBreak1-RangeBreak2) / RangeBreak3;
			Missile::changeDirection(startingAngle + Turn1 + Turn2 + (Turn3 * fracDist));
		}
		else if(totalDist<(RangeBreak1+RangeBreak2+RangeBreak3+RangeBreak4)) {
			fracDist = (totalDist-RangeBreak1-RangeBreak2-RangeBreak3) / RangeBreak4;
			Missile::changeDirection(startingAngle + Turn1 + Turn2 + Turn3 + (Turn4 * fracDist));
		}
		else Missile::changeDirection(startingAngle+Turn1+Turn2+Turn3+Turn4);
		//Missile::calculate();
	}

REGISTER_SHIP (VezlagariBarge)
