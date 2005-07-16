/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE


#include "../sc2ships.h"

// allows other ships to affect control over a ship.
class OverrideControlMelnorme : public OverrideControl
{
public:
	virtual void calculate(short *key);
};

class MelnormeShot : public Shot {
public:
IDENTITY(MelnormeShot);
  double v;
  int frame;
  int frame_step;
  int frame_size;
  int frame_count;
  int charge_frame;
  int charge_phase;
  int released;
	double RangeUp;

  public:
  MelnormeShot(Vector2 opos, double oangle, double ov, int odamage,
    double orange, double rangeup, int oarmour, Ship *oship, SpaceSprite *osprite,
    int ofcount, int ofsize);

  virtual void calculate();
  virtual void animateExplosion();
  virtual int handle_damage(SpaceLocation *source, double normal, double direct);
  virtual void inflict_damage(SpaceObject *other);
};

class MelnormeDisable : public SpaceObject {
public:
IDENTITY(MelnormeDisable);
	OverrideControlMelnorme *ocm;
  Ship *ship;
  int   disableframe;
  int   disableframe_count;
  int   frame_step;
  int   frame_size;
  int   frame_count;

  public:
  MelnormeDisable(Ship *creator, Ship *oship, SpaceSprite *osprite, int ofcount,
    int ofsize, int disableFrames);

  virtual void calculate();
};

class MelnormeSpecial : public AnimatedShot {
public:
IDENTITY(MelnormeSpecial);
  int disableFrames;
  SpaceSprite *disableSprite;

  public:
  MelnormeSpecial(Vector2 opos, double oangle, double ov,
    int oframes, double orange, int oarmour, Ship *oship,
    SpaceSprite *osprite, int ofsize, int ofcount);

  void inflict_damage(SpaceObject *other);
  void animateExplosion();
};

MelnormeTrader::MelnormeTrader(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)
{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponRangeUp  = scale_range(get_config_float("Weapon", "RangeUp", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  weaponObject   = NULL;

  specialRange    = scale_range(get_config_float("Special", "Range", 0));
  specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
  specialFrames   = get_config_int("Special", "Frames", 0);
  specialArmour   = get_config_int("Special", "Armour", 0);
	debug_id = 1034;
}

void MelnormeTrader::calculate()
{
  Ship::calculate();
  if (weaponObject) {
    if (!weaponObject->exists()) weaponObject = NULL;
    if (fire_weapon) recharge_step += frame_time; else weaponObject = NULL;
    }
}

int MelnormeTrader::activate_weapon()
{
  if(weaponObject)
    return(FALSE);
  add(weaponObject = new MelnormeShot(
    Vector2(0.0, (get_size().y /*height()*/ / 2.0)), angle, weaponVelocity, weaponDamage, weaponRange, weaponRangeUp, 
    weaponArmour, this, data->spriteWeapon, 10, 50));
  return(TRUE);
}

int MelnormeTrader::activate_special() {
  add( new MelnormeSpecial(
    Vector2(0.0, get_size().y /*(height()*/ / 2.0), angle, specialVelocity, specialFrames,
    specialRange, specialArmour, this, data->spriteSpecial, 20, 50));
  return(TRUE);
}

MelnormeShot::MelnormeShot(Vector2 opos, double oangle, double ov,
  int odamage, double orange, double rangeup, int oarmour, Ship *oship, SpaceSprite *osprite,
  int ofcount, int ofsize) :
  Shot(oship, opos, oangle, ov, odamage, orange, oarmour, oship, osprite),
  v(ov),
  frame(0),
  frame_step(0),
  frame_size(ofsize),
  frame_count(ofcount),
  charge_frame(0),
  charge_phase(0),
  released(FALSE),
	RangeUp(rangeup)
{  
//  vx = ship->get_vx();
//  vy = ship->get_vy();
  vel = ship->get_vel();
  set_depth(DEPTH_SHIPS+0.5);
	debug_id = 1035;
}

void MelnormeShot::calculate() {
	if(released) {
		Shot::calculate();
		}
	else {
		SpaceObject::calculate();
		}
	if (!exists()) return;
	frame_step += frame_time;
	if(frame_step >= frame_size) {
		frame_step -= frame_size;
		frame++;
		if(frame == frame_count) {
			frame = 0;
			if((!released) && (charge_phase < 3)) {
				charge_frame++;
				if(charge_frame == 5) {
					charge_frame = 0;
					charge_phase++;
					damage_factor *= 2;
					armour *= 2;
					range += RangeUp;
					play_sound(data->sampleWeapon[0]);
					}
				}
			}
		}
	sprite_index = (charge_phase * 10) + frame;
	if(!released) {
		if (!ship || !ship->fire_weapon) {
//			vx = cos(angle) * v;
//			vy = sin(angle) * v;
			vel = v * unit_vector(angle);
			released = TRUE;
			play_sound(data->sampleSpecial[0]);
			}
		else {
			angle = ship->get_angle();
//			x = ship->normal_x() + (cos(angle) * (ship->width() / 2.0));
//			y = ship->normal_y() + (sin(angle) * (ship->height() / 2.0));
			pos = ship->normal_pos() + 0.5 * product(unit_vector(angle), ship->get_size());
//			vx = ship->get_vx();
//			vy = ship->get_vy();
			vel = ship->get_vel();

			}
		}
	return;
	}

void MelnormeShot::animateExplosion()
{
  add( new Animation(this, pos,
    data->spriteWeaponExplosion, (charge_phase * 20), 20, 25, 
    DEPTH_EXPLOSIONS));
}

int MelnormeShot::handle_damage(SpaceLocation *source, double normal, double direct)
{
	if (source->isShip())
	{
		die();
		return 1;
	}
  int old = iround(armour);
  Shot::handle_damage(source, normal, direct);
  if (!released && (armour > 0)) armour = old;
  return iround(old - armour);
}

void MelnormeShot::inflict_damage(SpaceObject *other)
{
	if (!other->exists()) return;
	damage(other, damage_factor);

	// this can only die on enemy non-shots, if it's not red ...
	if (charge_phase < 3) 
		if (other->isblockingweapons) state = 0;

	if (state == 0) {
	animateExplosion();
	soundExplosion(); 
	}
	return;
}



void OverrideControlMelnorme::calculate(short *key)
{
	*key &= ~(keyflag::left | keyflag::right | keyflag::special);
	*key |= keyflag::right;
}



MelnormeDisable::MelnormeDisable(Ship *creator, Ship *oship, SpaceSprite *osprite,
  int ofcount, int ofsize, int disableFrames) :
  SpaceObject(creator, oship->normal_pos(), 0.0, osprite),
  ship(oship),
  disableframe(0),
  disableframe_count(disableFrames),
  frame_step(0),
  frame_size(ofsize),
  frame_count(ofcount)
{
	collide_flag_anyone = 0;
	set_depth(DEPTH_EXPLOSIONS);
	debug_id = 1036;

	ocm = new OverrideControlMelnorme();
	ship->set_override_control(ocm);
}

void MelnormeDisable::calculate()
{
	frame_step+= frame_time;
	while (frame_step >= frame_size) {
		frame_step -= frame_size;
		sprite_index++;
		if(sprite_index == frame_count)
			sprite_index = 0;
		}
	if(!(ship && ship->exists()))
	{
		ship->del_override_control(ocm);
		ship = 0;
		state = 0;
		return;
		}
//	x = ship->normal_x();
//	y = ship->normal_y();
	pos = ship->normal_pos();
	disableframe += frame_time;
	if (disableframe >= disableframe_count)
	{
		state = 0;
		ship->del_override_control(ocm);
	}

	SpaceObject::calculate();

	if (!exists())
	{
		ship->del_override_control(ocm);
	}

}

MelnormeSpecial::MelnormeSpecial(Vector2 opos, double oangle,
  double ov, int oframes, double orange, int oarmour, Ship *oship,
  SpaceSprite *osprite, int ofcount, int ofsize) :
  AnimatedShot(oship, opos, oangle, ov, 0, orange, oarmour, oship, osprite,
    ofcount, ofsize),
  disableFrames(oframes),
  disableSprite(data->spriteExtra)
{
	collide_flag_anyone = bit(LAYER_SHIPS);
	debug_id = 1037;
}

void MelnormeSpecial::animateExplosion() {}

void MelnormeSpecial::inflict_damage(SpaceObject *other)
{
	if (other->isShip()) add(new MelnormeDisable( ship, 
			(Ship *)(other), disableSprite, 20, 50, disableFrames));
	state = 0;
	return;
}



REGISTER_SHIP(MelnormeTrader)
