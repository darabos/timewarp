/* $Id$ */ 
class ChmmrAvatar : public Ship {
public:
  double weaponRange;
  int    weaponDamage;

  double       specialForce;
  double       specialRange;

  double extraRange;
  int    extraDamage;
  int    extraFrames;
  int    extraRechargeRate;
  int    extraColor;
  int    extraArmour;

  bool   uninterrupted_fire;

  public:
  ChmmrAvatar(Vector2 opos, double shipAngle, 
    ShipData *shipData, unsigned int code);

  virtual int activate_weapon();
  virtual int activate_special();
  virtual void calculate();
  virtual void materialize();
};

class DruugeMauler : public Ship {
public:
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;
  double       weaponDriftVelocity;

  public:
  DruugeMauler(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  protected:
  virtual int activate_weapon();
  virtual void calculate_fire_special();
};


class KohrAhBlade;
class KohrAhMarauder : public Ship
{
  public:
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;
  int          weaponFired;
  KohrAhBlade **weaponObject;
  bool         bladesPersist;

  double       specialVelocity;
  int          specialDamage;
  int          specialArmour;
  double       specialRange;


  KohrAhMarauder(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  virtual int activate_weapon();
  virtual int activate_special();
  virtual void calculate();

  int numblades;
  int maxblades;
};


class MelnormeTrader : public Ship {
public:
  double       weaponRange;
	double       weaponRangeUp;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;
  SpaceObject *weaponObject;

  double       specialRange;
  double       specialVelocity;
  int          specialFrames;
  int          specialArmour;

  public:
  MelnormeTrader(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  virtual void calculate();

  virtual int activate_weapon();
  virtual int activate_special();
};




#define MAX_MARINES 8

class OrzNemesis;
class OrzMissile;
class OrzMarine;

class OrzNemesis : public Ship {
	public:

	double absorption; //added for gob

        double       turret_turn_step;

	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	double       specialAccelRate;
	double       specialSpeedMax;
	int          specialHotspotRate;
	int          specialArmour;
	OrzMarine   *marine[MAX_MARINES];

	double       turretAngle;

	int          recoil;
	int          recoil_rate;
	int          recoil_range;
	double       turret_turn_rate;

	OrzNemesis(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

	virtual void calculate();
	virtual void animate(Frame *space);

	virtual void calculate_turn_left();
	virtual void calculate_turn_right();

	virtual int activate_weapon();
	virtual int activate_special();
	};



class PkunkFury : public Ship {
public:
  int reborn;

  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;

  public:
  PkunkFury(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  virtual int handle_damage(SpaceLocation *source, double normal, double direct);
  virtual int activate_weapon();
  virtual void calculate_fire_special();
};



class	SlylandroLaserNew;

class SlylandroProbe : public Ship {
public:
	int frame;
	int thrustActive;
	int thrustForward;
	int segments;
	int segment_length, segment_dispersion;
	int rnd_angle, aiming, dispersion;

	SlylandroLaserNew	*SlyLaser;

	double realturnstep;

	int sprite_index2;

	int last_turn_left, last_turn_right;

public:
	SlylandroProbe(Vector2 opos, double shipAngle,
		ShipData *shipData, unsigned int code);

	RGB crewPanelColor(int k = 0);

	virtual void calculate_thrust();
	virtual void calculate_turn_left();
	virtual void calculate_turn_right();
	virtual void calculate_fire_special();
	virtual void calculate_hotspots();
	virtual int activate_weapon();
	virtual void calculate();
	virtual int accelerate_gravwhip(SpaceLocation *source, double angle, double velocity, double max_speed);

};



class SupoxBlade : public Ship {
  public:
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;

  SupoxBlade(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  virtual void calculate_thrust();
  virtual void calculate_turn_left();
  virtual void calculate_turn_right();
  virtual void calculate_hotspots();

  virtual int activate_weapon();
  virtual void calculate_fire_special();
};



class ThraddashTorch : public Ship {
public:
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;

  double       specialThrust;
  int          specialDamage;
  int          specialArmour;

  public:
  ThraddashTorch(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  virtual int activate_weapon();
  virtual int activate_special();
  virtual void calculate_thrust();
};



class UtwigJugger : public Ship {
  public:

	int fortitude; //added for gob

  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;

  UtwigJugger(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);
  virtual void calculate();
  virtual double isProtected() const;

  protected:
  virtual void calculate_fire_weapon();
  virtual void animate(Frame *space);
  virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};



class ZoqFotPikStinger : public Ship {
  public:
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;

  int          specialDamage;


  ZoqFotPikStinger(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  virtual int activate_weapon();
  virtual int activate_special();
};

