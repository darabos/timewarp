#include "../ship.h"
REGISTER_FILE


#include "../sc2ships.h"


class ChmmrLaser : public Laser {
  public:
  ChmmrLaser(double langle, double lrange, int ldamage, int lfcount,
    SpaceLocation *opos, Vector2 rel_pos);
};

class ChmmrBeam : public SpaceObject {
  int frame;
  int frame_count;
  Ship *ship;
  SpaceObject *target;

  public:
  ChmmrBeam(Ship *oship, int oframes);

  virtual void calculate();
  virtual void animate(Frame *space);
};

class ChmmrZapSat : public SpaceObject {
	int frame;

	double lRange;
	int    lDamage;
	int    lFrames;
	int    lRechargeRate;
	int    lRecharge;
	int    lColor;
	int    armour;

	public:
	ChmmrZapSat(double oangle, double orange, int odamage, int oframes,
			int orechargerate, int ocolor, int oarmour, Ship *oship,
			SpaceSprite *osprite);

	virtual void calculate();
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
	virtual int canCollide(SpaceLocation *other);
};

ChmmrAvatar::ChmmrAvatar(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code) 
	:
	Ship(opos, shipAngle, shipData, code) 
	{

	weaponRange  = scale_range(get_config_float("Weapon", "Range", 0));
	weaponDamage = get_config_int("Weapon", "Damage", 0);

	specialForce = scale_velocity(get_config_float("Special", "Force", 0));
	specialRange = scale_range(get_config_float("Special", "Range", 0));

	extraRange        = scale_range(get_config_float("Extra", "Range", 0));
	extraFrames       = get_config_int("Extra", "Frames", 0);
	extraDamage       = get_config_int("Extra", "Damage", 0);
	extraRechargeRate = get_config_int("Extra", "RechargeRate", 0);
	extraColor        = get_config_int("Extra", "Color", 0);
	extraArmour       = get_config_int("Extra", "Armour", 0);
	}

int ChmmrAvatar::activate_weapon() {
  add(new ChmmrLaser(angle, weaponRange,
    weaponDamage, weapon_rate, this, Vector2(0.0, 25.0) ));
	if ((random() % 200) < frame_time) {
		Vector2 f, c;
		//double f, o, c, s;
		f.x = random(1.0);
		f.y = random(-1.0, 1.0) * (f.x+.5) / 5 ;
//		f = (((double)random()) / MAXINT);
//		o = ((((double)random()) / MAXINT) - 0.5) * (f+.5) / 2.5;
//		c = Vector2 ( cos(angle), sin(angle) );
//		c = unit_vector(angle);
//		c = cos(angle);
//		s = sin(angle);
		add(new Animation(this, 
			pos + rotate(f, angle) * weaponRange,
//				x + (c * f + s * o) * weaponRange, 
//				y + (s * f + c * o) * weaponRange, 
				data->spriteWeaponExplosion, (random()&1)*6, 6, 50, DEPTH_HOTSPOTS));

//		f = (((double)random()) / MAXINT);
//		o = ((((double)random()) / MAXINT) - 0.5) * (f+.5) / 2.5;
		f.x = random(1.0);
		f.y = random(-1.0, 1.0) * (f.x+.5) / 5 ;
//		c = cos(angle);
//		s = sin(angle);
		add(new Animation(this, 
			pos + rotate(f, angle) * weaponRange,
//				x + (c * f + s * o) * weaponRange, 
//				y + (s * f + c * o) * weaponRange, 
				data->spriteWeaponExplosion, (random()&1)*6, 6, 50, DEPTH_HOTSPOTS));
//		f = (((double)random()) / MAXINT);
//		o = ((((double)random()) / MAXINT) - 0.5) * (f+.5) / 2.5;
		f.x = random(1.0);
		f.y = random(-1.0, 1.0) * (f.x+.5) / 5 ;
//		c = cos(angle);
//		s = sin(angle);
		add(new Animation(this, 
			pos + rotate(f, angle) * weaponRange,
//				x + (c * f + s * o) * weaponRange, 
//				y + (s * f + c * o) * weaponRange, 
				data->spriteWeapon, (random()&1)*6, 6, 100, DEPTH_HOTSPOTS));
		}
	return(TRUE);
	}

int ChmmrAvatar::activate_special()
{
	if ((target != NULL) && (!target->isInvisible()) && (target->mass > 0) && (distance(target) < specialRange)) {
		add(new ChmmrBeam(this, special_rate));
		target->accelerate(this, target->trajectory_angle(this), specialForce / target->mass, MAX_SPEED);
		return (true);
		}
	return false;
}

void ChmmrAvatar::materialize()
{
	Ship::materialize();
	add(new ChmmrZapSat(0.0, extraRange,
		extraDamage, extraFrames, extraRechargeRate, extraColor, extraArmour, this,
		data->spriteExtra));
	add(new ChmmrZapSat(PI2/3, extraRange,
		extraDamage, extraFrames, extraRechargeRate, extraColor, extraArmour, this,
		data->spriteExtra));
	add(new ChmmrZapSat(PI2*2/3, extraRange,
		extraDamage, extraFrames, extraRechargeRate, extraColor, extraArmour, this,
		data->spriteExtra));
}

ChmmrLaser::ChmmrLaser(double langle, double lrange, int ldamage,
	int lfcount, SpaceLocation *opos, Vector2 rel_pos) 
:
	Laser(opos, langle, pallete_color[hot_color[random() % HOT_COLORS]], lrange,
	ldamage, lfcount, opos, rel_pos, true)
{
}

ChmmrBeam::ChmmrBeam(Ship *oship, int oframes) :
	SpaceObject(oship, oship->normal_pos(), oship->get_angle(), 
    game->sparkSprite),
	frame(0),
	frame_count(oframes),
	ship(oship),
	target(oship->target)
{
	set_depth(DEPTH_HOTSPOTS);
	collide_flag_anyone = 0;
	if(!ship->exists()) {
		state = 0;
		return;
	}

	target = ship->target;
	if((target == NULL) || (target->isInvisible())) {
		state = 0;
		return;
	}
}

void ChmmrBeam::calculate()
{

	SpaceLocation::calculate();

	if(!ship) {
		state = 0;
		return;
	}
	target = ship->target;

	if((!target) || (target->isInvisible())) {
		state = 0;
		return;
	}

//	x = ship->normal_x();
//	y = ship->normal_y();
	pos = ship->normal_pos();
	frame += frame_time;
	if (frame > frame_count) state = 0;
}

void ChmmrBeam::animate(Frame *space)
{
  const int beam_color[5] = { 80, 81, 82, 83, 84 };
  int i;
  double length;

//  length = sqrt((target->get_vx() * target->get_vx()) +
//    (target->get_vy() * target->get_vy())) + (target->width() / 4.0);
  length = target->get_vel().length() + (target->get_size().x / 4.0);

	for(i = 3; i >= 0 ; i--)
		target->get_sprite()->animate_character(
		target->normal_pos() + (i+1) * unit_vector(trajectory_angle(target) - PI) * length,
//				target->normal_x() + (i + 1) * 
//				cos(trajectory_angle(target) - PI) * length,
//				target->normal_y() + (i + 1) *
//				sin(trajectory_angle(target) - PI) * length,
				target->get_sprite_index(), pallete_color[beam_color[i]], space);
}

ChmmrZapSat::ChmmrZapSat(double oangle, double orange, int odamage,
  int oframes, int orechargerate, int ocolor, int oarmour, Ship *oship,
  SpaceSprite *osprite) :
  SpaceObject(oship, Vector2(0.0, 0.0), 0.0, osprite),
  lRange(orange),
  lDamage(odamage),
  lFrames(oframes),
  lRechargeRate(orechargerate),
  lRecharge(0),
  lColor(ocolor),
  armour(oarmour)
{
	layer = LAYER_SPECIAL;
	set_depth(DEPTH_SPECIAL);
	collide_flag_anyone = ALL_LAYERS &~ bit(LAYER_CBODIES);
	id |= CHMMR_SPEC;
	angle = oangle;
//	x = ship->normal_x() + cos(angle) * 100.0;
//	y = ship->normal_y() + sin(angle) * 100.0;
	pos = ship->normal_pos() + unit_vector(angle) * 100.0;

	if(!ship->exists()) state = 0;
}

void ChmmrZapSat::calculate() {

	SpaceObject::calculate();

	if (!ship) {
		state = 0;
		return;
		}

//	x = ship->normal_x() + (cos(angle) * 100.0);
//	y = ship->normal_y() + (sin(angle) * 100.0);
	pos = ship->normal_pos() + unit_vector(angle) * 100.0;

	double da = 0.002;

	angle += da * frame_time;
	
//	vx = (ship->normal_x() + (cos(angle) * 100.0) - x) / frame_time;
//	vy = (ship->normal_y() + (sin(angle) * 100.0) - y) / frame_time;
	vel = (ship->normal_pos() + unit_vector(angle) * 100.0 - pos) / frame_time;


	if(angle >= PI2) angle -= PI2;
	sprite_index = get_index(angle);

	if(lRecharge > 0) {
		lRecharge -= frame_time;
		return;
		}

	Query q;
	for (q.begin(this, OBJECT_LAYERS &~ bit(LAYER_CBODIES), lRange); q.currento; q.next()) {
		if (!q.currento->isInvisible() && !q.currento->sameTeam(this) && (q.currento->collide_flag_anyone&bit(LAYER_LINES))) {
			add(new PointLaser(this, pallete_color[lColor], 1, lFrames,
                        		this, q.currento, Vector2(0.0, 0.0) ));
			sound.play((SAMPLE *)(melee[MELEE_BOOM + 0].dat));
			lRecharge += lRechargeRate;
			break;
			}
		}
	return;
	}
int ChmmrZapSat::canCollide(SpaceLocation *other) {
	if (!other->damage_factor) return false;
	return SpaceObject::canCollide(other);
	}

int ChmmrZapSat::handle_damage(SpaceLocation *source, double normal, double direct) {
	int total = normal + direct;
	if (total) {
		armour -= total;
		if(armour <= 0) {
			armour = 0;
			state = 0;
			add(new Animation(this, pos,
					game->kaboomSprite, 0, KABOOM_FRAMES, 50, DEPTH_EXPLOSIONS));
			sound.stop(data->sampleExtra[0]);
			sound.play(data->sampleExtra[0]);
			}
		}
	return total;
	}




REGISTER_SHIP(ChmmrAvatar)
