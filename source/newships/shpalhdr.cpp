#include "../ship.h"
#include "../melee/mview.h"

REGISTER_FILE


// combine Klisru main and Alhordian special into this ship ...
// also make special more useful by giving more control over it.

class AlhordianLaserSweep;

class AlhordianDreadnought : public Ship {
public:
	AlhordianLaserSweep* ALS1;
	AlhordianLaserSweep* ALS2;
	
	int currentSweepTime;
	int sweepIsOn;
	
	double       weaponStartVelocity;
	double       weaponStartDamage;
	double       weaponEndDamage;
	double       weaponStartArmour;
	double       weaponEndArmour;
	double       weaponRelativity;
	double       weaponStartFriction;
	double       weaponEndFriction;
	int          weaponLifetime;
	
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
	int specialSustain;
	
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
	
//	int weaponSpriteNumber;
//	int justBorn;
};



class AlhordianTorpedo : public Shot {
public:
	AlhordianTorpedo(AlhordianDreadnought* ocreator, Vector2 rpos,
		double oangle, double oStartVelocity, double oEndVelocity, 
		double oStartDamage, double oEndDamage,
		double orange, double oStartArmour, double oEndArmour,
		SpaceLocation* opos, SpaceSprite* osprite, double orelativity);

	AlhordianDreadnought* creator;

	double startDamage;
	double endDamage;
	double startArmour;
	double endArmour;
	double startVelocity;
	double endVelocity;
	double lifetimeCounter;
	double lifetimeMax;
	double startFriction, endFriction;
	double friction;

	virtual void calculate(void);
	//virtual void animateExplosion(void);
	void inflict_damage(SpaceObject *other);
};




AlhordianDreadnought::AlhordianDreadnought(Vector2 opos, double angle,  ShipData *data, unsigned int code)
:
Ship(opos, angle, data, code)
{
	
	weaponStartVelocity = scale_velocity(get_config_float("Weapon", "StartVelocity", 0));
	weaponStartDamage   = get_config_float("Weapon", "StartDamage", 0);
	weaponStartArmour   = get_config_float("Weapon", "StartArmour", 0);
	weaponEndDamage     = get_config_float("Weapon", "EndDamage", 0);
	weaponEndArmour     = get_config_float("Weapon", "EndArmour", 0);
	weaponRelativity    = get_config_float("Weapon", "Relativity", 0);
	weaponStartFriction = get_config_float("Weapon", "StartFriction", 0);
	weaponEndFriction   = get_config_float("Weapon", "EndFriction", 0);
	weaponLifetime      = get_config_int("Weapon", "Lifetime", 0);
	
	
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
	specialSustain = get_config_int("Special", "Sustain", 0);
	
	//currentSweepTime = (int)specialTime + 1;
	currentSweepTime = 0; //pulling up on
	sweepIsOn = FALSE;
}

AlhordianDreadnought::~AlhordianDreadnought(void) {
  ;
}

void AlhordianDreadnought::calculate()
{
	STACKTRACE;
	if(this->batt<1 || !fire_special) {
		//currentSweepTime = 0; //pulling up on
		sweepIsOn = FALSE;
	}
	Ship::calculate();

	if (sweepIsOn)
	{
		if (!fire_weapon && fire_special)
			currentSweepTime += frame_time;

		else if (fire_weapon && fire_special)
			currentSweepTime -= frame_time;
	}
}


int AlhordianDreadnought::activate_weapon()
{
	STACKTRACE;

	if (fire_special)
		return false;

	AlhordianTorpedo* KT;
	KT = new AlhordianTorpedo(this, Vector2(0, size.y * 0.4), this->angle,
		this->weaponStartVelocity, this->weaponStartVelocity, 
		this->weaponStartDamage, this->weaponEndDamage,
		-1, this->weaponStartArmour, this->weaponEndArmour,
		this, this->data->spriteWeapon, 0.0);
	KT->startFriction = this->weaponStartFriction;
	KT->endFriction = this->weaponEndFriction;
	KT->lifetimeCounter = 0;
	KT->lifetimeMax = this->weaponLifetime;
	game->add(KT);

	this->weapon_flash();

	return(TRUE);
}


int AlhordianDreadnought::activate_special() {
	STACKTRACE;
	
	sweepIsOn = TRUE;
	
	calculate_laser_sweep();
	
	return(TRUE);
}

void AlhordianDreadnought::weapon_flash() {
	STACKTRACE
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


void AlhordianDreadnought::calculate_laser_sweep(void)
{
	STACKTRACE;
	
	double fractionDone;
	double X, Y, Angle, Length;
	
	if ( currentSweepTime < 0 )
		currentSweepTime = 0;

	if ( currentSweepTime > specialTime )
		currentSweepTime = (int)specialTime;

	if ( currentSweepTime > specialTime )
	{
		sweepIsOn = FALSE;
		return;
	}
	
	fractionDone = currentSweepTime / specialTime;
	X = specialStartX * (1-fractionDone) + specialEndX * fractionDone;
	Y = specialStartY * (1-fractionDone) + specialEndY * fractionDone;

	Angle = specialStartAngle * (1-fractionDone) + specialEndAngle * (fractionDone);
	Length = specialStartRange * (1-fractionDone) + specialEndRange * (fractionDone);
	
	game->add(new Laser(this,Angle+angle, palette_color[9], Length, specialDamage, specialSustain,
		this, Vector2(size.y * X, size.y * Y), TRUE));
	
	game->add(new Laser(this,-Angle+angle, palette_color[9], Length, specialDamage, specialSustain,
		this, Vector2(-size.y * X, size.y * Y), TRUE));
}



AlhordianTorpedo::AlhordianTorpedo(AlhordianDreadnought* ocreator, Vector2 rpos,
							 double oangle, double oStartVelocity, double oEndVelocity,
							 double oStartDamage, double oEndDamage, double orange,
							 double oStartArmour, double oEndArmour, SpaceLocation* opos, SpaceSprite* osprite,
							 double orelativity)
:
Shot(ocreator, rpos, oangle, oStartVelocity, oStartDamage, orange, 
	 oStartArmour, ocreator, ocreator->data->spriteWeapon, orelativity)
{
	this->explosionSprite = ocreator->data->spriteWeapon;
	this->explosionFrameCount = 24;
	creator = ocreator;
	startDamage = oStartDamage;
	endDamage = oEndDamage;
	startArmour = oStartArmour;
	endArmour = oEndArmour;
	startVelocity = oStartVelocity;
	endVelocity = oEndVelocity;
	sprite_index = 0;
	this->lifetimeCounter = 0;
}

void AlhordianTorpedo::calculate(void)
{
	STACKTRACE;
	double fractionDone = 0.0;
	int spriteToUse = 0;
	Shot::calculate();
	d = -1;	// to override shot-range

	lifetimeCounter += frame_time;
	fractionDone = lifetimeCounter / lifetimeMax;
	friction = startFriction * (1 - fractionDone) + endFriction * fractionDone;

	if(fractionDone<0) fractionDone = 0.0;

	if(fractionDone>=0.999) {
		fractionDone = 0.999;
		state = 0;
	}
	spriteToUse = (int)(fractionDone * 16.0);
	sprite_index = spriteToUse;
	damage_factor = (1 - fractionDone) * startDamage + fractionDone * endDamage;
	armour = (1 - fractionDone) * startArmour + fractionDone * endArmour;
	this->v *= (1 - this->friction * frame_time);
	this->vel = unit_vector(this->angle) * this->v;
	
}

void AlhordianTorpedo::inflict_damage(SpaceObject *other)
{
	STACKTRACE;
	
	int x = this->sprite_index;
	
	Shot::inflict_damage(other);
	
	if (other->mass <= 0)
		return;
	
	if(other->isShip())
		game->add(new FixedAnimation(this, other,
				explosionSprite, x, explosionFrameCount - x,
				explosionFrameSize, DEPTH_EXPLOSIONS)
				);
	else
		game->add(new Animation(this, pos,
				explosionSprite, x, explosionFrameCount - x,
				explosionFrameSize, DEPTH_EXPLOSIONS)
				);
	
}



REGISTER_SHIP (AlhordianDreadnought)

