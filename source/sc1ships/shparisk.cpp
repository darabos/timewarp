#include "../ship.h"
REGISTER_FILE

#include "../sc1ships.h"

ArilouSkiff::ArilouSkiff(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code)
	{
	specialSprite = data->spriteSpecial;

	weaponColor  = get_config_int("Weapon", "Color", 0);
	weaponDamage = get_config_int("Weapon", "Damage", 0);
	weaponRange  = scale_range(get_config_float("Weapon", "Range", 0));
	weaponFrames = get_config_int("Weapon", "Frames", 0);

	specialFrames = get_config_float("Special", "Frames", 0);

	just_teleported = 0;
	debug_id = 1003;
	}

void ArilouSkiff::inflict_damage(SpaceObject *other) {
	if (just_teleported && other->mass) {
		damage(this, 0, 999);
		}
	else Ship::inflict_damage(other);
	return;
	}

int ArilouSkiff::activate_weapon() {
	SpaceObject *o = NULL;

	double r = 99999;  

	Query a;
	for (a.begin(this, bit(LAYER_SHIPS), weaponRange + 200); a.current; a.next()) {
		if ((distance(a.current) < r) && !a.current->isInvisible()) {
			o = a.currento;
			r = distance(o);
			}
		}

	if (o) r = trajectory_angle(o); else r = angle;
	game->add(new Laser(this, r, pallete_color[weaponColor], 
		weaponRange, weaponDamage, weaponFrames, this));
	return TRUE;
	}

int ArilouSkiff::activate_special() {

	game->add(new Animation(this, pos,
			specialSprite, 0, 40, iround(specialFrames/40), DEPTH_HOTSPOTS-0.1));
	
	Vector2 d = Vector2 ( 
		random(-1500.0, 1500.0),
		random(-1500.0, 1500.0)
	);
	translate(d);
	just_teleported = 1;

	game->add(new Animation(this, pos,
			specialSprite, 0, 40, iround(specialFrames/40), DEPTH_HOTSPOTS-0.1));
	return(TRUE);
	}

void ArilouSkiff::calculate() {
	just_teleported = 0;
	Ship::calculate();

	if(!thrust) {
		vel *= 1 - frame_time * 0.02;
		}
	}

void ArilouSkiff::calculate_gravity() {}

int ArilouSkiff::accelerate(SpaceLocation *source, double angle, double velocity, 
		double max_speed) {
	if (source == this) 
		return Ship::accelerate(source, angle, velocity, max_speed);
	return false;
	}

int ArilouSkiff::accelerate_gravwhip(SpaceLocation *source, double angle, double velocity, 
		double max_speed) {
	if (source == this) 
		return Ship::accelerate(source, angle, velocity, max_speed);
	return false;
	}

REGISTER_SHIP(ArilouSkiff)
