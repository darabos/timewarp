/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

#include "../sc1ships.h"

class CrewPod : public SpaceObject {
	int frame_count;
	int frame_size;
	int frame_step;

	double velocity;
	int    life;
	int    lifetime;

	public:
	CrewPod(Vector2 opos, double oVelocity, int oLifeTime,
		Ship *oship, SpaceSprite *osprite, int ofcount, int ofsize);

	virtual void calculate();
	virtual int sameTeam(SpaceLocation *other);

	virtual void inflict_damage(SpaceObject *other);
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
	};

SyreenPenetrator::SyreenPenetrator(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
{

	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);

	specialRange    = scale_range(get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialDamage   = get_config_int("Special", "Damage", 0);
	specialFrames   = get_config_int("Special", "Frames", 0);
	debug_id = 1018;
}

int SyreenPenetrator::activate_weapon()
{
	Missile *m;
	m = new Missile(this, Vector2(0.0, size.y / 2.0 + 10),
			angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
			this, data->spriteWeapon);
	add(m);
	m->collide_flag_sameship = ALL_LAYERS;

  return(TRUE);
}

int SyreenPenetrator::activate_special() {
	double minDist;
	minDist = specialRange + (size.x / 2.0);
	int j;
	for (j = 0; j < targets->N; j += 1) {
		Ship *target = (Ship*) targets->item[j];
		if (!target->isShip()) continue;
		if (!control->valid_target(target)) continue;
		int callDamage;
		double r;
		if (target) r = minDist - distance(target) - target->get_size().x / 2.0;
		else r = -1;
		
		RGB col = target->crewPanelColor();
		if ((col.g < 192) || (col.r > 16) || (col.b > 16)) continue;
		if (r > 0) {
			callDamage = (int)(r * (double)(specialDamage) / specialRange);
			if (callDamage > specialDamage) callDamage = specialDamage;
			callDamage += (random() % specialDamage);
			if ((target->getCrew() - callDamage) < 1) callDamage = iround(target->getCrew() - 1);
			int old = iround(target->getCrew());
			damage(target, 0, callDamage);
			callDamage = iround(old - target->getCrew());
			for(int i = 0; i < callDamage; i++) {
				add(new CrewPod(
						target->normal_pos() + (unit_vector(trajectory_angle(target) - PI) * 
						target->size.x) + random(Vector2(50,50)) - Vector2(25,25),
						specialVelocity, specialFrames, this, data->spriteSpecial, 32, 
						50));
				}
			}
		}
	return(TRUE);
	}

CrewPod::CrewPod(Vector2 opos, double oVelocity, int oLifeTime,
  Ship *oship, SpaceSprite *osprite, int ofcount, int ofsize) :
  SpaceObject(oship, opos, 0.0, osprite),
  frame_count(ofcount),
  frame_size(ofsize),
  frame_step(0),
  velocity(oVelocity),
  life(0),
  lifetime(oLifeTime)
{
	collide_flag_sameship = ALL_LAYERS;
	collide_flag_sameteam = ALL_LAYERS;
	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);

	isblockingweapons = false;
	debug_id = 1019;
}

int CrewPod::sameTeam(SpaceLocation *other) {
	return true;
	}

void CrewPod::calculate() {
	frame_step += frame_time;
	while (frame_step >= frame_size) {
		frame_step -= frame_size;
		sprite_index++;
		if(sprite_index == frame_count)	sprite_index = 0;
		}

	life += frame_time;
	if(life >= lifetime) {
		state = 0;
		return;
		}

  if (ship && ship->exists()) {
		vel = unit_vector(trajectory_angle(ship)) * velocity;
		}
	else ship = NULL;

	SpaceObject::calculate();
	}

void CrewPod::inflict_damage(SpaceObject *other)
{
  if (other->isShip() && other->damage_factor == 0) {
    sound.stop(data->sampleExtra[0]);
    sound.play(data->sampleExtra[0]);
	damage(other, 0, -1);
		state = 0;
  }
}

int CrewPod::handle_damage(SpaceLocation *source, double normal, double direct)
{
	state = 0;
	return 0;
}



REGISTER_SHIP(SyreenPenetrator)
