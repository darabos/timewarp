#include "../ship.h"
REGISTER_FILE

#include <math.h>

#include "../sc2ships.h"

class ZoqFotPikShot : public Shot {
  int frame_count;

  public:
  ZoqFotPikShot(Vector2 opos, double oangle, double ov, int odamage,
    double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
    int num_frames);

  virtual void calculate();
};

class ZoqFotPikTongue : public SpaceObject {
  double dist;
  Ship  *ship;
  int    lick_factor;

  int frame;
  int frame_size;
  int frame_count;
  int frame_step;

  public:
  ZoqFotPikTongue(double odist, int odamage, Ship *oship,
    SpaceSprite *osprite, int ofcount, int ofsize);

  virtual void calculate();
  virtual void inflict_damage(SpaceObject *other);
};

ZoqFotPikStinger::ZoqFotPikStinger(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)
{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  specialDamage = get_config_int("Special", "Damage", 0);
  if (get_config_int("Special", "Licking", 0) == 0) specialDamage *= -1;
	debug_id = 1047;
}

int ZoqFotPikStinger::activate_weapon() {
  add(new ZoqFotPikShot(
    Vector2(0.0, 0.5*get_size().y), angle + ANGLE_RATIO * random(-10.0, 10.0),
    weaponVelocity, weaponDamage, weaponRange, weaponArmour, this,
    data->spriteWeapon, 6));
  return(TRUE);
}

int ZoqFotPikStinger::activate_special()
{
  add(new ZoqFotPikTongue(
    39, specialDamage, this, data->spriteSpecial, 6, 50));
  return(TRUE);
}

ZoqFotPikShot::ZoqFotPikShot(Vector2 opos, double oangle,
  double ov, int odamage, double orange, int oarmour, Ship *oship,
  SpaceSprite *osprite, int num_frames) :
  Shot(oship, opos, oangle, ov, odamage, orange, oarmour, oship, osprite),
  frame_count(num_frames)
{
	debug_id = 1048;
}

void ZoqFotPikShot::calculate()
{
  Shot::calculate();
  sprite_index = (int)((d / range) * (double)(frame_count - 1));
}

ZoqFotPikTongue::ZoqFotPikTongue(double odist, int odamage, Ship *oship,
  SpaceSprite *osprite, int ofcount, int ofsize) :
  SpaceObject(oship, oship->normal_pos()/*, oship->normal_y()*/, 0.0,
    osprite),
  dist(odist),
  ship(oship),
  lick_factor(odamage),
  frame(0),
  frame_size(ofsize),
  frame_count(ofcount),
  frame_step(0)
{
  layer = LAYER_SHOTS;
  set_depth(DEPTH_SHOTS);
  damage_factor = abs(lick_factor);
//  x = ship->normal_x() + (cos(ship->get_angle()) * dist);
//  y = ship->normal_y() + (sin(ship->get_angle()) * dist);
  pos = ship->normal_pos() + dist * unit_vector(ship->get_angle());
//  vx = ship->get_vx(); vy = ship->get_vy();
  vel = ship->get_vel();
  sprite_index = get_index(ship->get_angle());
  sprite_index += (64 * frame);

  isblockingweapons = true;
	debug_id = 1049;
}

void ZoqFotPikTongue::calculate()
{
  int current_frame = frame;

  if (!(ship && ship->exists()))
    {
	  ship = 0;
	  state = 0;
	  return;
  }

  frame_step+= frame_time;
  while(frame_step >= frame_size) {
    frame_step -= frame_size;
    frame++;
    if(frame == frame_count)
      state = 0;
  }
  if((current_frame != frame) && (lick_factor > 0))
    damage_factor = lick_factor;

//  x = ship->normal_x() + cos(ship->get_angle()) * dist;
//  y = ship->normal_y() + sin(ship->get_angle()) * dist;
  pos = ship->normal_pos() + dist * unit_vector(ship->get_angle());
//  vx = ship->get_vx(); vy = ship->get_vy();
  vel = ship->get_vel();
  sprite_index = get_index(ship->get_angle());
  sprite_index += (64 * frame);

  SpaceObject::calculate();
}

void ZoqFotPikTongue::inflict_damage(SpaceObject *other)
{
  SpaceObject::inflict_damage(other); 
  damage_factor = 0;

  // in order to remove the variable damage:
  lick_factor = 0;
}



REGISTER_SHIP(ZoqFotPikStinger)
