/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

#include "../sc1ships.h"

class UmgahCone : public SpaceObject {
public:
IDENTITY(UmgahCone);
  double dist;

  double damage;
  double residual_damage;
  int damage_type;

  UmgahDrone *umgahship;

  public:
  UmgahCone(double odist, int odamage, UmgahDrone *oship, SpaceSprite *osprite);

	virtual bool change_owner(SpaceLocation *new_owner);
  virtual void calculate();
  virtual void inflict_damage(SpaceObject *other);
  virtual void UmgahCone::animate(Frame* space);
  virtual int canCollide(SpaceLocation* other);

};

UmgahDrone::UmgahDrone(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
{

  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  specialRate    = special_rate;
  damage_type    = get_config_int("Weapon", "DamageType", 0);

  firing = false;
  add(new UmgahCone(81, weaponDamage, this, data->spriteWeapon));

	debug_id = 1020;
}

int UmgahDrone::activate_weapon() {
  firing = true;
  return(TRUE);
}

int UmgahDrone::activate_special()
{
  special_rate = specialRate;
  vel = 0;
  pos -= (unit_vector(angle) * size.x * 2.0);
  if (!thrust) special_rate = 50;
  return(TRUE);
}

void UmgahDrone::calculate_thrust()
{
  if(special_recharge <= 0)
    Ship::calculate_thrust();
}

void UmgahDrone::calculate()
{
  if (fire_weapon)
    recharge_step = recharge_rate;
  else
    firing = false;

  Ship::calculate();
}

UmgahCone::UmgahCone(double odist, int odamage, UmgahDrone *oship,
  SpaceSprite *osprite) :
  SpaceObject(oship, oship->normal_pos(), 0.0, osprite),
  dist(odist)
{
	layer = LAYER_SHOTS;
	set_depth(DEPTH_SHIPS - 0.1);

	id = SPACE_SHOT;
	damage_factor = 1;

	damage_type = oship->damage_type;
	damage = (odamage/1000.0);
	residual_damage = 1e-5;

	pos = ship->normal_pos() + (unit_vector(ship->get_angle()) * dist);
	angle = ship->get_angle();
	sprite_index = get_index(angle);

	collide_flag_sameship = 0;

	isblockingweapons = false;

	umgahship = oship;
	debug_id = 1021;
}

bool UmgahCone::change_owner(SpaceLocation *new_owner) {
	return false;
	}

void UmgahCone::calculate()
{
	SpaceObject::calculate();

	if(!(ship && ship->exists()))
	{
		ship = 0;
		state = 0; 
		return; 
	}

	if (!(umgahship && umgahship->exists()))
		umgahship = NULL;

	pos = ship->normal_pos() + (unit_vector(ship->get_angle()) * dist);
	vel = ship->get_vel();
	angle = ship->get_angle();
	sprite_index = get_index(angle);


	if (damage_type == 2) {
		residual_damage = fabs(residual_damage);
		if (residual_damage <= 0) residual_damage = 1e-5;
	}
}

void UmgahCone::inflict_damage(SpaceObject *other)
{
  switch (damage_type) {
    case 0: {
            residual_damage = damage * frame_time / 2.0;
            while (residual_damage >= 1) {
              residual_damage -= 1;
              SpaceObject::inflict_damage(other); };
            residual_damage *= 1e4;
            if (residual_damage > random(10000))
              SpaceObject::inflict_damage(other);
            }; break;
    case 1: {
            residual_damage += damage * frame_time / 2.0;
            while (residual_damage >= 1) {
              residual_damage -= 1;
              SpaceObject::inflict_damage(other); };
            }; break;
    case 2: if (residual_damage > 0) {
              residual_damage += damage * frame_time;
              while (residual_damage >= 1) {
                residual_damage -= 1;
                SpaceObject::inflict_damage(other); };
              residual_damage *= -1;
            }; break;
  }

}

int UmgahCone::canCollide(SpaceLocation* other) {
//  calc_base();
  if (umgahship && umgahship->exists())
	return (umgahship->firing & !other->isPlanet());
  else
	  return false;
}

void UmgahCone::animate(Frame* space) {
//  calc_base();
  if (!umgahship || !umgahship->exists() || !umgahship->firing) return;
  int si = sprite_index;
  sprite_index += ((rand()%6) << 6);	//graphics
  SpaceObject::animate(space);
  sprite_index = si;
}



REGISTER_SHIP(UmgahDrone)
