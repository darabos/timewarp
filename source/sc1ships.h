/* $Id$ */ 
class AndrosynthGuardian : public Ship {
public:
	IDENTITY(AndrosynthGuardian);
public:
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	int bounce_status;

	SpaceSprite *specialSprite;
	SpaceSprite *shipSprite;
	double       specialVelocity;
	double       specialTurnRate;
	int          specialDamage;
	int          specialBounceDistance;
	int          specialBounceTime;
	int          specialActive;
	double       shipTurnRate;
	int          shipRechargeAmount;
	double specialMass;
	double normalMass;

	public:
	AndrosynthGuardian(Vector2 opos, double angle, ShipData *data, unsigned int code);

	virtual void calculate();
	virtual void calculate_thrust();
	virtual void calculate_hotspots();

	virtual void inflict_damage(SpaceObject *other);

	virtual int activate_weapon();
	virtual int activate_special();
};



class ArilouSkiff : public Ship {
public:
	IDENTITY(ArilouSkiff);
public:
	int    weaponColor;
	double weaponRange;
	int    weaponFrames;
	int    weaponDamage;

	int just_teleported;

	SpaceSprite *specialSprite;
	double       specialFrames;

	public:
	ArilouSkiff(Vector2 opos, double angle, ShipData *data, unsigned int code);

	protected:
	virtual void inflict_damage(SpaceObject *other);
	virtual int activate_weapon();
	virtual int activate_special();
	virtual void calculate();
	virtual void calculate_gravity();
	virtual int accelerate(SpaceLocation *source, double angle, double velocity, 
			double max_speed);
	virtual int accelerate_gravwhip(SpaceLocation *source, double angle, double velocity, 
			double max_speed);
	};


class ChenjesuDOGI ;
class ChenjesuBroodhome : public Ship
{
public:
	IDENTITY(ChenjesuBroodhome);
protected:
	double		weaponVelocity, shardRange;
	double		shardDamage, shardArmour;
	double		shardRelativity;
	double		weaponDamage, weaponArmour;
	int			shardRotation;
	int			weaponFired;
	Shot		*weaponObject;

	double		specialVelocity;
	double		specialFuelSap;
	double		specialArmour;
	double		specialAccelRate;
	double		specialMass;
	double		specialAvoidanceAngle;
	double		specialAvoidanceFactor;
	int			specialNumDOGIs;

public:
	ChenjesuBroodhome(Vector2 opos, double angle, ShipData *data, unsigned int code);
	virtual int activate_weapon();
	virtual int activate_special();
	virtual void calculate();
};


class EarthlingCruiser : public Ship {
public:
	IDENTITY(EarthlingCruiser);
public:
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;
  double       weaponTurnRate;

  int    specialColor;
  double specialRange;
  int    specialFrames;
  int    specialDamage;

  public:
  EarthlingCruiser(Vector2 opos, double angle, ShipData *data, unsigned int code);

  protected:
  virtual int activate_weapon();
  virtual int activate_special();
};


class IlwrathAvenger : public Ship {
public:
	IDENTITY(IlwrathAvenger);
public:
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	int cloak;
	int cloak_frame;

	public:
	static int cloak_color[3];
	IlwrathAvenger(Vector2 opos, double angle, ShipData *data, unsigned int code);

	virtual double isInvisible() const;
	virtual int activate_weapon();
	virtual void calculate_fire_special();
	virtual void calculate_hotspots();
	virtual void calculate();
	virtual void animate(Frame *space);
};


class KzerZaDreadnought : public Ship {
public:
	IDENTITY(KzerZaDreadnought);
public:
	double weaponRange;
	double weaponVelocity;
	int    weaponDamage;
	int    weaponArmour;

	int    specialFrames;
	int    specialLaserDamage;
	int    specialLaserColor;
	double specialLaserRange;
	int    specialLaserFrames;
	int    specialLaserDrain;
	double specialVelocity;
	double specialRange;
	int    specialArmour;

	public:
	KzerZaDreadnought(Vector2 opos, double angle, ShipData *data, unsigned int code);

	int activate_weapon();
	int activate_special();
	};


class MmrnmhrmXForm : public Ship {
public:
	IDENTITY(MmrnmhrmXForm);
public:
	struct {
		public:
		double       speed_max;
		double       accel_rate;
		double       turn_rate;
		int          recharge_amount;
		int          recharge_rate;
		int          weapon_drain;
		int          weapon_rate;
		int          hotspot_rate;
		SpaceSprite *sprite;
	} form_data[2];

	int form;

	int        laserColor;
	double     laserRange;
	double     laserAngle;
	int        laserDamage;
	SpaceLine *laser1;
	SpaceLine *laser2;
	int        laserSpark;

	double missileRange;
	double missileVelocity;
	int    missileDamage;
	int    missileArmour;
	double missileTurnRate;

	public:
	MmrnmhrmXForm(Vector2 opos, double shipAngle, 
		ShipData *shipData, unsigned int code);

	void calculate();
	double handle_speed_loss(SpaceLocation *source, double normal);

	int activate_weapon();
	int activate_special();
};


class MyconPodship : public Ship {
public:
	IDENTITY(MyconPodship);
public:
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	double       weaponHome;
	int          specialRepair;

	public:
	MyconPodship(Vector2 opos, double angle, ShipData *data, unsigned int code);

	virtual int activate_weapon();
	virtual int activate_special();
};

class ShofixtiScout : public Ship {
public:
	IDENTITY(ShofixtiScout);
	public:
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	double       specialRange;
	double       specialScale;
	int          specialFrames;
	int          specialDamage;
	int          flipSwitch;
	int          glory;

	ShofixtiScout(Vector2 opos, double angle, ShipData *data, unsigned int code);

	virtual int activate_weapon();
	virtual void calculate_fire_special();
};


class SpathiEluder : public Ship {
public:
	IDENTITY(SpathiEluder);
public:
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	double       specialRange;
	double       specialVelocity;
	int          specialDamage;
	int          specialArmour;
	double       specialTurnRate;

	public:
	SpathiEluder(Vector2 opos, double angle, ShipData *data, unsigned int code);

	virtual int activate_weapon();
	virtual int activate_special();
};


class SyreenPenetrator : public Ship {
public:
	IDENTITY(SyreenPenetrator);
public:
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	double       specialRange;
	double       specialVelocity;
	int          specialDamage;
	int          specialFrames;

	public:
	SyreenPenetrator(Vector2 opos, double angle, ShipData *data, unsigned int code);

	virtual int activate_weapon();
	virtual int activate_special();
};


class UmgahDrone : public Ship {
public:
	IDENTITY(UmgahDrone);
public:
	int          weaponDamage;
	int          weaponCone;
	int          specialRate;

	public:

	int          firing;
	int          damage_type;

	UmgahDrone(Vector2 opos, double angle, ShipData *data, unsigned int code);

	virtual int activate_weapon();
	virtual int activate_special();
	virtual void calculate_thrust();
	virtual void calculate();
};


class VuxIntruder : public Ship {
public:
	IDENTITY(VuxIntruder);
public:
	int    weaponColor;
	double weaponRange;
	int    weaponDamage;

	double specialRange;
	double specialVelocity;
	double specialSlowdown;
	int    specialArmour;

	public:
	VuxIntruder(Vector2 opos, double angle, ShipData *data, unsigned int code);

	virtual int activate_weapon();
	virtual int activate_special();
	virtual SpaceLocation *get_ship_phaser() ;
	void relocate() ;
  virtual void animate(Frame *space);
};


class YehatTerminator : public Ship {
public:
	IDENTITY(YehatTerminator);
public:
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	int          specialFrames;
	int          shieldFrames;

	public:
	YehatTerminator(Vector2 opos, double angle, ShipData *data, unsigned int code);
	virtual double isProtected() const;

	protected:
	virtual int activate_weapon();
	virtual int activate_special();
	virtual void calculate();
	virtual void animate(Frame *space);
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
};

