/* $Id$ */ 
#include "../ship.h"
#include "../frame.h"
#include "../melee/mview.h"
REGISTER_FILE

class GlathrielDestroyer : public Ship
{
public:
IDENTITY(GlathrielDestroyer);
	double		weaponRange, weaponVelocity, weaponInactive;
	int			weaponDamage, weaponArmour;
	int			laserDamage;
	int			laserColor;
	int			laserNumber;
	double		laserRange, laserSweep, sensorSweep;

	double		specialRange, specialVelocity, specialSweep;
	int			specialDamage, specialArmour, specialNumber;

public:
	GlathrielDestroyer(Vector2 opos, double angle, ShipData *shipData, unsigned int code);

protected:
	virtual int activate_weapon();
	virtual int activate_special();
};

class GlathrielMissile : public Missile
{
public:
IDENTITY(GlathrielMissile);
    double	inactive_range;
    int		laser_num;
    int		laser_damage;
    int		laser_color;
    double	laser_range, laser_sweep;

public:
	GlathrielMissile(SpaceLocation *creator, double ox, double oy, double oangle, double ov,
		int odamage, double orange, int oarmour, SpaceSprite *osprite,int olasernum,
		double oinactive, int olaserdamage,int olasercolor,double olaserrange,
		double olasersweep);

  virtual void calculate();
};

class GlathrielSpot : public SpaceLocation
{
public:
IDENTITY(GlathrielSpot);
	int		life_counter, lifetime;

public:
	GlathrielSpot(SpaceLocation *creator, double oangle, SpaceSprite *osprite,
		int olasernum, int olaserdamage, int olasercolor, double olaserRange, double olasersweep);

	virtual void calculate();
	virtual void animate(Frame *space);

};

class GlathrielMiniMissile : public Missile
{
public:
IDENTITY(GlathrielMiniMissile);
public:
	GlathrielMiniMissile(double ox, double oy, double oangle, double ov,
		int odamage, double orange, int oarmour, Ship *oship,
		SpaceSprite *osprite);
};

GlathrielDestroyer::GlathrielDestroyer(Vector2 opos, double angle, ShipData *shipData, unsigned int code) :
	Ship(opos, angle, shipData, code)
{
	weaponRange		= scale_range(get_config_float("Weapon", "Range", 0));
	weaponInactive	= scale_range(get_config_float("Weapon", "InactiveRange", 0));
	weaponVelocity	= scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage	= get_config_int("Weapon", "Damage", 0);
	weaponArmour	= get_config_int("Weapon", "Armour", 0);
	laserDamage		= get_config_int("Weapon","LaserDamage",0);
	laserColor		= get_config_int("Weapon","LaserColor",0);
	laserRange		= scale_range(get_config_float("Weapon","LaserRange",0));
	laserSweep		= get_config_float("Weapon","LaserSweep",0)*PI/180;
	laserNumber		= get_config_int("Weapon", "LaserNumber",0);

	specialRange	= scale_range(get_config_float("Special", "Range", 0));
	specialVelocity	= scale_velocity(get_config_float("Special", "Velocity", 0));
	specialDamage	= get_config_int("Special", "Damage", 0);
	specialArmour	= get_config_int("Special", "Armour", 0);
	specialSweep	= get_config_float("Special","Sweep",0)*PI/180;
	specialNumber	= get_config_int("Special", "Number",0);
}

int GlathrielDestroyer::activate_weapon()
{
	STACKTRACE
	add(new GlathrielMissile(this, 0.0, 14, angle, weaponVelocity, weaponDamage, weaponRange,
		weaponArmour, data->spriteWeapon, laserNumber, weaponInactive,
		laserDamage,laserColor,laserRange,laserSweep));
  return true;
}

int GlathrielDestroyer::activate_special()
{
	STACKTRACE
	int i;
	for (i=0; i<specialNumber; i++)
		add(new GlathrielMiniMissile(
			0.0, 26, angle + tw_random(-specialSweep, specialSweep), specialVelocity,
			specialDamage, specialRange, specialArmour, this, data->spriteSpecial));
	return true;
}

GlathrielMissile::GlathrielMissile(SpaceLocation *creator, double ox, double oy, double oangle, double ov,
		int odamage, double orange, int oarmour, SpaceSprite *osprite,int olasernum,
		double oinactive, int olaserdamage,int olasercolor,double olaserrange,
		double olasersweep) :
	Missile(creator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, creator, osprite),
	inactive_range(oinactive), laser_num(olasernum), laser_damage(olaserdamage),
	laser_color(olasercolor), laser_range(olaserrange), laser_sweep(olasersweep)
{
}

void GlathrielMissile::calculate()
{
	STACKTRACE
	Missile::calculate();
	if (!exists()) return;
	if (d > inactive_range) {
		SpaceObject *o;
		double a;
		Query q;
		for (q.begin(this, bit(LAYER_SHIPS),(laser_range *.80), QUERY_OBJECT); q.current; q.next()) {
			o = q.currento;
			a = normalize(trajectory_angle(o)-angle,360);
			if ((!o->sameTeam(this)) && (!o->isInvisible()) &&
				((a < laser_sweep) || (a > (360 - laser_sweep))) ){

				add(new GlathrielSpot(this,angle,data->spriteExtra,
					laser_num,laser_damage,laser_color,laser_range,laser_sweep));
				//play_sound(data->sampleWeapon[1]);//using wrong datafile?
				state = 0;
			}
		}
		q.end();
	}
}

GlathrielSpot::GlathrielSpot(SpaceLocation *creator, double oangle, SpaceSprite *osprite,
		int olasernum, int olaserdamage, int olasercolor, double olaserRange, double olasersweep) :
	SpaceLocation(creator,0,oangle)
{
	pos = creator->normal_pos();
	collide_flag_anyone = 0;
	collide_flag_sameteam = 0;
	collide_flag_sameship = 0;
	int i;
	for (i=0; i<olasernum; i++)
		add(new Laser(this, angle+2*olasersweep*(-(olasernum-1)/2.0 + i)/(double)olasernum,
			pallete_color[olasercolor], olaserRange,olaserdamage,50,this,0,0));
	lifetime = 500;
	life_counter = 0;
}

void GlathrielSpot::calculate()
{
	STACKTRACE
	SpaceLocation::calculate();
	life_counter += frame_time;
	if (life_counter >= lifetime)
		state = 0;
}

void GlathrielSpot::animate(Frame *space)
{
	STACKTRACE
	if (state == 0) return;
	double r = life_counter/(double)lifetime;
	int rr = iround(80 * r * space_zoom);
	set_trans_blender(0,0,0,iround(255*(1-r)*(1-r)));
	drawing_mode(DRAW_MODE_TRANS, NULL, 0, 0);
	Vector2 p0 = corner(pos);
	circlefill(space->surface,iround(p0.x),iround(p0.y),rr,makecol(255,255,0+iround(floor(255*(1-r)))));
	space->add_box(iround(p0.x)-rr-1, iround(p0.y)-rr-1, 2*rr+2, 2*rr+2);
	drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}



GlathrielMiniMissile::GlathrielMiniMissile(double ox, double oy, double oangle,
		double ov, int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite) :
	Missile(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour,  oship, osprite)
{
}



REGISTER_SHIP(GlathrielDestroyer)
