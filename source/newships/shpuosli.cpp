
/*
I used some code from the Orz Nemesis for this ship: for the turret
Rest is "original" - as far as I can be original anyway ;)
*/

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

class UoiSlicer : public Ship {
  double	weaponRange;
  double	weaponVelocity;
  int		weaponDamage;
  double	weaponArmour;

  double	specialRange;
  double	specialDamage;
  int		specialFrames;
  double	specialTurnRate;
  int		specialColor;

  double	turretAngle, turret_turn_rate, turret_turn_step;

  int		weapon_left_right;

  public:
  UoiSlicer(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  protected:
  virtual int activate_weapon();
  virtual int activate_special();
  virtual void animate(Frame *space);
  virtual void calculate();
  virtual void calculate_turn_left();
  virtual void calculate_turn_right();
  void calculate_turn_turret();
};




// a laser that makes a circle around the ship, then disappears
class RotatingLaser : public Laser
{
public:

	Ship	*mother;
	double	relangle, laser_turn_rate, default_length;
	int		default_color;
	Vector2	relpos;

	RotatingLaser(SpaceLocation *creator, double langle, int lcolor, double lrange, double ldamage,
				int lfcount, SpaceLocation *opos, Vector2 rpos,
				double oturnrate, bool osinc_angle);

	void calculate();
	void inflict_damage(SpaceObject *other);
};



UoiSlicer::UoiSlicer(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos,  shipAngle, shipData, code)
{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);

  specialRange    = scale_range(get_config_float("Special", "Range", 0));
  specialFrames   = scale_frames(get_config_int("Special", "Frames", 0));
  specialTurnRate = scale_turning(get_config_float("Special", "TurnRate", 0));
  specialColor    = get_config_int("Special", "Color", 0);
  specialDamage   = get_config_int("Special", "Damage", 0);
  

  turretAngle = 0.0;
  turret_turn_rate = scale_turning(get_config_float("Turret", "TurnRate", 0));
  turret_turn_step = 0;

  weapon_left_right = 1;
}


int UoiSlicer::activate_weapon()
{	

	if (fire_special)
		return FALSE;

	double a = angle+turretAngle;

	Vector2 rpos;
	rpos = Vector2(-0.0, 50.0);

	double angle_true = angle;
	angle += turretAngle;		// note, angle is used to rotate rpos by Missile;
	add(new Missile(this, rpos, a,
		weaponVelocity, weaponDamage, weaponRange, weaponArmour,
		this, data->spriteWeapon, 0.0));
	angle = angle_true;
	
	return TRUE;
}


int UoiSlicer::activate_special()
{

	if ( !fire_weapon )
		return FALSE;

		// activate the laser !
		
	add( new RotatingLaser(this, angle, specialColor, specialRange, specialDamage,
		specialFrames, this, Vector2(0,50), specialTurnRate, 0) );

	return TRUE;
}



void UoiSlicer::animate(Frame *space)
{
	int turret_index;
		
	Ship::animate(space);
	turret_index = get_index(angle + turretAngle);

	data->spriteSpecial->animate( pos, turret_index, space);
		
	return;
	}

void UoiSlicer::calculate()
{
	/*
	if ( !(nextkeys & keyflag::special) )
	{

//		if (turn_left)
		if (nextkeys & keyflag::left)
		{
			turret_turn_step -= frame_time * turret_turn_rate;
			nextkeys &= ~keyflag::left;	// don't use for normal turning
		}

//		if (turn_right)
		if (nextkeys & keyflag::right)
		{
			turret_turn_step += frame_time * turret_turn_rate;
			nextkeys &= ~keyflag::right;	// don't use for normal turning
		}
		
		while (fabs(turret_turn_step) > (PI2/64))
		{
			if (turret_turn_step < 0.0 )
			{
				turretAngle -= (PI2/64);
				turret_turn_step += (PI2/64);
			} else {
				turretAngle += (PI2/64);
				turret_turn_step -= (PI2/64);
			}
		}
		
	turretAngle = normalize(turretAngle, PI2);
	}
	*/

	Ship::calculate();
}


void UoiSlicer::calculate_turn_turret()
{

	if (turn_left)
		turret_turn_step -= frame_time * turret_turn_rate;

	if (turn_right)
		turret_turn_step += frame_time * turret_turn_rate;
	
	while (fabs(turret_turn_step) > (PI2/64))
	{
		if (turret_turn_step < 0.0 )
		{
			turretAngle -= (PI2/64);
			turret_turn_step += (PI2/64);
		} else {
			turretAngle += (PI2/64);
			turret_turn_step -= (PI2/64);
		}
	}
		
	turretAngle = normalize(turretAngle, PI2);
}

void UoiSlicer::calculate_turn_left()
{
	if ( fire_special )
		Ship::calculate_turn_left();
	else
		calculate_turn_turret();
}

void UoiSlicer::calculate_turn_right()
{
	if ( fire_special )
		Ship::calculate_turn_right();
	else
		calculate_turn_turret();
}



RotatingLaser::RotatingLaser(SpaceLocation *creator, double langle,
  int lcolor, double lrange, double ldamage,
  int lfcount, SpaceLocation *opos, Vector2 rpos,
  double oturnrate, bool osinc_angle)
  :
Laser(creator, langle, lcolor, lrange, ldamage,
  lfcount, opos, rpos, osinc_angle)
{
	mother = (Ship*) creator;
	relangle = 0;
	relpos = rpos;
	laser_turn_rate = oturnrate;

	default_length = length;
	default_color = lcolor;
}


void RotatingLaser::calculate()
{
	if ( !(mother && mother->exists()) )
	{
		state = 0;
		return;
	}

	relangle += laser_turn_rate * frame_time;
	relative_angle = relangle;
	rel_pos = rotate(relpos, relangle);

	angle = normalize(mother->angle + mother->turn_step + relangle, PI2);
//	pos = mother->pos + rotate(relpos, angle+relangle-PI/2);
//	vel = mother->vel;

	double a = 1.0 - double(frame) / frame_count;
	length = default_length * a;

	int r, g, b;
	r = getr(default_color);
	g = getg(default_color);
	b = getb(default_color);
	r *= a;
	b *= a;
	color = makecol(r,g,b);


	Laser::calculate();
}


void RotatingLaser::inflict_damage(SpaceObject *other)
{
	Laser::inflict_damage(other);
	state = 0;
}



REGISTER_SHIP(UoiSlicer)

