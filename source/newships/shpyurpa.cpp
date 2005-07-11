/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

//#include "../sc1ships.h"

#include "../melee.h"
#include "../melee/mshot.h"
#include "../melee/mship.h"
#include "../melee/manim.h"


class YuryulPatriot;

class YuryulMissile : public HomingMissile {
public:
IDENTITY(YuryulMissile);
public:
	YuryulMissile(YuryulPatriot* ocreator, double ox, double oy, double oangle, double ov,
		int odamage, double orange, int oarmour, double oturnrate, SpaceLocation* opos, SpaceSprite* osprite, SpaceObject* otarget);
	Ship* creator;
	int lifetimeTimer;
	double relativity;
	double facingAngle;
	double framesToIgnition;
	int isActivated;
	int burnFrames1;
	int burnFrames2;
	int coastFrames;
	double acceleration;
	int wasCoasting;
	int thrustOn;
	double coastVelocity;
	double burnVelocity1;
	double burnVelocity2;
	void thrust_on(void);
	virtual void calculate(void);
	virtual void inflict_damage(SpaceObject *other);
};

class YuryulRam : public Missile {
public:
IDENTITY(YuryulRam);
public:
	YuryulPatriot* creator;
	Vector2 relativePosition;
	double angleOffset;
	int RamNumber;
	int decayFrames;
	int decayAmount;
	int decayCount;
	YuryulRam(YuryulPatriot* creator, Vector2 relativePosition,
		double oangle, double angleOffset,
		double ov, double odamage, double orange, double oarmour);
	~YuryulRam();
	virtual void calculate(void);
};


class YuryulPatriot : public Ship {
public:
IDENTITY(YuryulPatriot);
public:
	double       shipTurnAccelRate;
	double       shipTurnSpeedMax;
	
	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;
	double       weaponRelativity;
	
	int          specialDamage;
	int          specialArmour;
	double       specialReleaseAngle;
	double       specialReleaseAngleRad;
	int          specialDecayFrames;
	
	double       thrustRange;
	double       thrustVelocity;
	int          thrustDamage;
	int          thrustArmour;
	double       thrustRelativity;
	
	//int          gunToFire;
	Vector2		ramrelpos;
	
public:
	
	YuryulRam* Ram1;
	YuryulRam* Ram2;
	
	YuryulPatriot(Vector2 opos, double angle, ShipData *data, unsigned int code);
	
protected:
	~YuryulPatriot();
	virtual void calculate();
	virtual int activate_weapon();
	virtual int activate_special();
	virtual void death();
	virtual void calculate_turn_left();
	virtual void calculate_turn_right();
	virtual void calculate_thrust();
};


YuryulPatriot::YuryulPatriot(Vector2 opos, double angle, ShipData *data, unsigned int code) 
:
Ship(opos, angle, data, code)
{
	shipTurnAccelRate = scale_acceleration(get_config_float("Ship", "TurnAccelRate", 0));
	shipTurnSpeedMax = scale_velocity(get_config_float("Ship", "TurnSpeedMax", 0));
	
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	weaponRelativity = get_config_int("Weapon", "Relativity", 0);
	
	
	specialDamage   = get_config_int("Special", "Damage", 0);
	specialArmour   = get_config_int("Special", "Armour", 0);
	specialReleaseAngle = get_config_float("Special", "ReleaseAngle", 0);
	specialReleaseAngleRad = specialReleaseAngle * ANGLE_RATIO;
	specialDecayFrames = get_config_int("Special", "DecayFrames", 0);
	
	thrustRange    = scale_range(get_config_float("Thrust", "Range", 0));
	thrustVelocity = scale_velocity(get_config_float("Thrust", "Velocity", 0));
	thrustDamage   = get_config_int("Thrust", "Damage", 0);
	thrustArmour   = get_config_int("Thrust", "Armour", 0);
	thrustRelativity = get_config_float("Thrust", "Relativity", 0);
	
	Ram1 = NULL;
	Ram2 = NULL;

	ramrelpos.x = get_config_float("Special", "RamXrel", 0);
	ramrelpos.y = get_config_float("Special", "RamYrel", 0);
}

YuryulPatriot::~YuryulPatriot(void) {
	if(Ram1!=NULL)
		Ram1->creator = NULL;
	if(Ram2!=NULL)
		Ram2->creator = NULL;
}


void YuryulPatriot::death(void) {
	STACKTRACE
		Ship::death();
}

int YuryulPatriot::activate_weapon() {
	STACKTRACE
		game->add(new Missile(this, Vector2(size.y*(0.00), (size.y * +0.00)),
		angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon));
	return(TRUE);
}

int YuryulPatriot::activate_special() {
	STACKTRACE
		if(Ram1!=NULL) Ram1->state = 0;
		if(Ram2!=NULL) Ram2->state = 0;
		Ram1 = new YuryulRam(this, Vector2(this->size.x * (ramrelpos.x), this->size.x * (ramrelpos.y)), this->angle, -1.0 * this->specialReleaseAngleRad,
			this->vel.magnitude(), this->specialDamage, -1.0, this->specialArmour);
		Ram1->angleOffset = this->specialReleaseAngleRad * (-1.0);
		Ram1->RamNumber = 1;
		Ram2 = new YuryulRam(this, Vector2(this->size.x * (-ramrelpos.x), this->size.x * (ramrelpos.y)), this->angle, +1.0 * this->specialReleaseAngleRad,
			this->vel.magnitude(), this->specialDamage, -1.0, this->specialArmour);
		Ram2->angleOffset = this->specialReleaseAngleRad * (1.0);
		Ram2->RamNumber = 2;
		game->add(Ram1);
		game->add(Ram2);
		return(TRUE);
}

void YuryulPatriot::calculate(void) {
	STACKTRACE
		if(Ram1!=NULL) {
			Ram1->decayCount += frame_time;
			if(Ram1->decayCount > this->specialDecayFrames) {
				Ram1->state = 0;
				Ram1=NULL;
			}
		}
		if(Ram2!=NULL) {
			Ram2->decayCount += frame_time;
			if(Ram2->decayCount > this->specialDecayFrames) {
				Ram2->state = 0;
				Ram2=NULL;
			}
		}
		Ship::calculate();
}

void YuryulPatriot::calculate_turn_left(void) {
	STACKTRACE
		Ship::calculate_turn_left();
	//if(turn_left)accelerate_gravwhip(this, angle, shipTurnAccelRate * frame_time, shipTurnSpeedMax);
	
}

void YuryulPatriot::calculate_turn_right(void) {
	STACKTRACE
		Ship::calculate_turn_right();
	//if(turn_right)accelerate_gravwhip(this, angle, shipTurnAccelRate * frame_time, shipTurnSpeedMax);
	
}

void YuryulPatriot::calculate_thrust(void) {
	STACKTRACE
		Ship::calculate_thrust();
}

YuryulRam::YuryulRam(YuryulPatriot* ocreator, Vector2 relPos, double oangle, double oangleOffset,
                     double ov, double odamage, double orange, double oarmour)
					 :
//Missile::Missile(SpaceLocation *creator, Vector2 rpos, double oangle, 
//	double ov, double odamage, double orange, double oarmour, 
//	SpaceLocation *opos, SpaceSprite *osprite, double relativity) 

//Missile(ocreator, ocreator->pos, ocreator->angle,
//        (ocreator->vel).magnitude(), ocreator->specialDamage, -1, ocreator->specialArmour,
//        ocreator, data->spriteExtra, 0.0)
Missile(ocreator, ocreator->pos, oangle,
        ov, odamage, orange, oarmour,
        ocreator, ocreator->data->spriteExtra, 0.0)
		
{
	relativePosition = relPos;
	creator = ocreator;
	this->pos = creator->pos;
	angleOffset = oangleOffset;
	this->angle = oangle + oangleOffset;
	decayFrames = 0;
	decayAmount = 0;
	decayCount = 0;
}

YuryulRam::~YuryulRam(void) {
	if(creator!=NULL) {
		if(RamNumber == 1)
			creator->Ram1 = NULL;
		if(RamNumber == 2)
			creator->Ram2 = NULL;
	}
}


void YuryulRam::calculate(void) {
	STACKTRACE
		// changed GEO
		if(!(creator && creator->exists()) )
		{
			creator = 0;
			state = 0;
			return;
		}
		this->changeDirection(creator->angle + this->angleOffset);
		this->pos = creator->pos;
		this->vel = creator->vel;
		pos = normalize(pos + rotate(relativePosition, -PI/2+creator->get_angle()));
		Missile::calculate();
}

YuryulMissile::YuryulMissile(YuryulPatriot* ocreator, double ox, double oy, double oangle, double ov,
							 int odamage, double orange, int oarmour, double oturnrate, SpaceLocation* opos, SpaceSprite* osprite, SpaceObject* otarget) 
							 :
HomingMissile(ocreator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oturnrate, opos, osprite, otarget) 

{
	explosionSprite     = data->spriteWeaponExplosion;
	facingAngle=oangle;
	isActivated=0;
	this->relativity = 1.0;
	thrustOn = FALSE;
	lifetimeTimer = 0;
}

void YuryulMissile::calculate(void) {
	STACKTRACE
		Missile::calculate();
}

void YuryulMissile::inflict_damage(SpaceObject *other) {
	STACKTRACE
		Shot::inflict_damage(other);
}

void YuryulMissile::thrust_on(void) {
	;}

REGISTER_SHIP ( YuryulPatriot )
