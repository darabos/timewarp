#include "../ship.h"
REGISTER_FILE
#include "../melee/mview.h"

#include "../sc1ships.h"

class KzerZaMissile : public Missile {
	public:
	KzerZaMissile(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, Ship *oship, SpaceSprite *osprite);
	};

class KzerZaFighter : public Missile {
	int    air_frames;
	int    max_air_frames;
	int    laser_damage;
	int    laser_color;
	double laser_range;
	int    laser_frames;
	int    batt;
	int    recharge_frames;

	public:
	KzerZaFighter(Vector2 opos, double oangle, double ov,
		double orange, int oarmour, int oair_frames,
		int olaser_damage, int olaser_color, double olaser_range,
		int olaser_frames, int odrain, Ship *oship, SpaceSprite *osprite);

	virtual void calculate();
	virtual void inflict_damage(SpaceObject *other);
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
	};

KzerZaDreadnought::KzerZaDreadnought(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code) 
	{
	weaponRange         = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity      = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage        = get_config_int("Weapon", "Damage", 0);
	weaponArmour        = get_config_int("Weapon", "Armour", 0);

	specialFrames          = get_config_int("Special", "Frames", 0);
	specialLaserDamage     = get_config_int("Special", "LaserDamage", 0);
	specialLaserColor      = get_config_int("Special", "LaserColor", 0);
	specialLaserRange      = scale_range(get_config_float("Special", "LaserRange", 0));
	specialLaserFrames     = get_config_int("Special", "LaserFrames", 0);
	specialLaserDrain      = get_config_int("Special", "LaserDrain", 0);
	specialVelocity        = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialRange           = scale_range(get_config_float("Special", "Range", 0));
	specialArmour          = get_config_int("Special", "Armour", 0);
	}

int KzerZaDreadnought::activate_weapon() {
	game->add( new KzerZaMissile(
			Vector2(0.0, (size.y / 2.0)), angle, weaponVelocity, weaponDamage, weaponRange,
			weaponArmour, this, data->spriteWeapon) );
	return(TRUE);
	}

int KzerZaDreadnought::activate_special() {
	if(crew > 1) {
		game->add( new KzerZaFighter(Vector2(-25.0, -50.0),
				angle - PI*7.0/8 , specialVelocity, specialRange, specialArmour,
				specialFrames, specialLaserDamage,
				specialLaserColor, specialLaserRange, specialLaserFrames,
				specialLaserDrain, this, data->spriteSpecial));
		crew--;
		if(crew > 1) {
			add( new KzerZaFighter(Vector2(25.0, -50.0),
					angle + PI*7.0/8, specialVelocity, specialRange, specialArmour,
					specialFrames, specialLaserDamage,
					specialLaserColor, specialLaserRange, specialLaserFrames,
					specialLaserDrain, this, data->spriteSpecial));
			crew--;
			}
		return(TRUE);
		}
	else 
		return(FALSE);
	}

KzerZaMissile::KzerZaMissile(Vector2 opos, double oangle, double ov,
	int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite) 
	:
	Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship,osprite) 
	{
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 20;
	explosionFrameSize  = 50;
	}

KzerZaFighter::KzerZaFighter (Vector2 opos, double oangle, double ov,
	double orange, int oarmour, int oair_frames,
	int olaser_damage, int olaser_color, double olaser_range, int olaser_frames,
	int odrain, Ship *oship, SpaceSprite *osprite) 
	:
	Missile(oship, opos, oangle, ov, 1, orange, oarmour, oship, osprite),
	air_frames(oair_frames),
	max_air_frames(oair_frames),
	laser_damage(olaser_damage),
	laser_color(olaser_color),
	laser_range(olaser_range),
	laser_frames(olaser_frames),
	batt(0),
	recharge_frames(odrain)
	{
	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);
	}

void KzerZaFighter::calculate() {
	Missile::calculate();

	if (!ship) {
		state = 0;
		return;
		}
	air_frames -= frame_time;
	if (air_frames <= 0) {
		state = 0;
		return;
		}
	if (air_frames > max_air_frames - 350) return;

	target = ship->target;
	if((target == NULL) || (air_frames < (max_air_frames / 2)) || target->isInvisible()) {
		collide_flag_sameship |= bit(LAYER_SHIPS);
		changeDirection(trajectory_angle(ship));
		return;
		}
	collide_flag_sameship &= ~bit(LAYER_SHIPS);

	if ((distance(target) < laser_range) && (batt <= 0)) {
		collide_flag_sameship = 0;
		vel = 0;
		play_sound2(data->sampleExtra[0]);
		add(new Laser(this, trajectory_angle(target),
				pallete_color[laser_color], laser_range, laser_damage,
				laser_frames, this, Vector2(0.0, -size.y / 2.0)));
		batt = recharge_frames;
		}
	else {
		Vector2 t = target->normal_pos();
		double ta = target->get_angle();
		double a = target->trajectory_angle(this) - target->get_angle();
		a = normalize(a,PI2);
		Vector2 l = t + unit_vector(ta+PI/2) * laser_range * 0.8;
		Vector2 r = t + unit_vector(ta+PI*3/2) * laser_range * 0.8;
		double d_l = distance_from(pos, l);
		double d_r = distance_from(pos, r);
		double d2t;
		if (d_l < d_r) {
			angle = atan(min_delta(l, pos));
			d2t = d_l;
			}
		else {
			angle = atan(min_delta(r, pos));
			d2t = d_r;
			}
		if (d2t > 20) d2t = 20;
		vel = v * unit_vector(angle) * d2t / 20;
		sprite_index = get_index(angle);
		}

	if (batt) batt -= frame_time;
	}


int KzerZaFighter::handle_damage(SpaceLocation *source, double normal, double direct){
	if (source->isPlanet()) {
		state = 1;
	}
	else state = 0;
	return 0;
}

void KzerZaFighter::inflict_damage(SpaceObject *other) {
	if (other == ship) {
		play_sound2(data->sampleExtra[1]);
		damage(ship, 0, -1);
	} 
	if (!other->isPlanet()) state = 0;
	else {
		double a = trajectory_angle(other);
		translate(unit_vector(a) * -10);
	}
	return;
}


REGISTER_SHIP ( KzerZaDreadnought )
