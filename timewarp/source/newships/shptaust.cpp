#include "../ship.h"

REGISTER_FILE

class TauStorm : public Ship
{
	double		weaponVelocity, weaponAccel, weaponTurnRate, weaponThrust, weaponFuel, weaponStart;
	double		specialVelocity, specialAccel, specialTurnRate, specialThrust, specialFuel, specialStart;
	double		weaponBoosterSpeed, weaponRotation, weaponKick, weaponKickMaxspeed;
	double		specialBoosterSpeed, specialRotation, specialKick, specialKickMaxspeed;
	double		weaponMass;
	double		weaponRandom, specialRandom;

	int			slot;

public:
	TauStorm(Vector2 opos, double shipAngle,
		ShipData *shipData, unsigned int code);

	virtual int  activate_weapon();
	virtual int  activate_special();
	virtual void animate(Frame *space);
};


class TauStormMissile : public HomingMissile
{
	int	fuel;

	double ra, pa, rr, rt;

	SpaceObject *latched;
	
	double accel, thrust, booster_speed, rotation;
	int fire_index, fire_frame, smoke_frame, first_frame;

public:

	TauStormMissile (SpaceLocation *creator, double ox, double oy, double oangle, double oaccel,
				double ov, double otr, SpaceObject *otarget,
				int ofuel, double othrust, double omass, double oboosterspeed,
				SpaceSprite *osprite, double omr, double osv, SAMPLE *s);				
	virtual void calculate();
	virtual void animate(Frame *space);
	virtual void inflict_damage (SpaceObject *other);
	virtual void handle_damage (SpaceObject *other, double normal, double direct = 0);
	virtual int  canCollide(SpaceLocation *other);
};



TauStorm::TauStorm(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
	Ship(opos, shipAngle, shipData, code)
{

	weaponMass = get_config_float("Weapon", "Mass", 0.01);

	weaponVelocity		= scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponAccel			= scale_acceleration(get_config_float("Weapon", "Accel", 0), 0);
	weaponTurnRate		= scale_turning(get_config_float("Weapon", "TurnRate", 0));
	weaponFuel			= get_config_int("Weapon", "Fuel", 0);
	weaponThrust		= scale_acceleration(get_config_int("Weapon", "Thrust", 0), 0);
	weaponBoosterSpeed	= scale_velocity(get_config_float("Weapon", "BoosterSpeed", 0));
	weaponRotation		= get_config_float("Weapon", "Rotation", 0)*PI/180;
	weaponKick			= scale_velocity(get_config_float("Weapon", "Kick", 0));
	weaponKickMaxspeed	= scale_velocity(get_config_float("Weapon", "KickMaxspeed", 0));
	weaponStart			= get_config_int("Weapon", "StartSpeed", 0);
	weaponRandom		= get_config_float("Weapon", "Random", 0);

	specialVelocity		= scale_velocity(get_config_float("Special", "Velocity", 0));
	specialAccel		= scale_acceleration(get_config_float("Special", "Accel", 0), 0);
	specialTurnRate		= scale_turning(get_config_float("Special", "TurnRate", 0));
	specialFuel			= get_config_int("Special", "Fuel", 0);
	specialThrust		= scale_acceleration(get_config_int("Special", "Thrust", 0), 0);
	specialBoosterSpeed	= scale_velocity(get_config_float("Special", "BoosterSpeed", 0));
	specialRotation		= get_config_float("Special", "Rotation", 0)*PI/180;
	specialKick			= scale_velocity(get_config_float("Special", "Kick", 0));
	specialKickMaxspeed	= scale_velocity(get_config_float("Special", "KickMaxspeed", 0));
	specialStart		= get_config_int("Special", "StartSpeed", 0);
	specialRandom		= get_config_float("Special", "Random", 0);

	slot = 0;
}

int TauStorm::activate_weapon()
{
	int rx;
	if (slot<2)	rx = 9;
	else rx = 13;
	if (slot%2)	rx = -rx;
	game->add(new TauStormMissile (this, rx, 10, angle, weaponAccel,
				weaponVelocity, weaponTurnRate, target,
				weaponFuel*(1+weaponRandom*(100-random()%201)/100.0), weaponThrust, weaponMass,
				weaponBoosterSpeed, data->spriteWeapon,
				weaponRotation, weaponStart, data->sampleExtra[0]));
	slot = (slot +1) % 6;
	accelerate(this, angle+PI, weaponKick, weaponKickMaxspeed);
    return true;
}

int TauStorm::activate_special()
{
	int rx;
	if (slot<2)	rx = 9;
	else rx = 13;
	if (slot%2)	rx = -rx;
	game->add(new TauStormMissile (this, rx, 10, angle, specialAccel,
			specialVelocity, specialTurnRate, target,
			specialFuel*(1+specialRandom*(100-random()%201)/100.0), specialThrust, weaponMass,
			specialBoosterSpeed, data->spriteWeapon,
			specialRotation, specialStart, data->sampleExtra[1]));
	slot = (slot + 1) % 6;
	accelerate(this, angle+PI, specialKick, specialKickMaxspeed);
    return true;
}


void TauStorm::animate(Frame *space)
{
	if (thrust)
		sprite->animate(pos, sprite_index + 64, space);
	else
		sprite->animate(pos, sprite_index, space);

}


TauStormMissile::TauStormMissile (SpaceLocation *creator, double ox, double oy, double oangle, double oaccel,
				double ov, double otr, SpaceObject *otarget,
				int ofuel, double othrust, double omass, double oboosterspeed,
				SpaceSprite *osprite, double omr, double osv, SAMPLE *s) :
	HomingMissile(creator, Vector2(ox,oy), oangle, ov, 1, 1e40, 1, otr, creator,  osprite, otarget),
	fuel(ofuel), booster_speed(oboosterspeed),
	thrust(othrust), accel(oaccel), rotation(omr)

{

	latched = NULL;
	mass = omass;

	vel = vel*osv + creator->get_vel();

	fire_index = random() % 5;
	fire_frame = 50;

	explosionSprite = data->spriteWeaponExplosion;
	explosionFrameCount = 10;
	explosionFrameSize = 50;
	explosionSample = data->sampleWeapon[1];

	smoke_frame = 0;
	first_frame = 12;
	fire_index = 0;

	play_sound(s);
}


void TauStormMissile::calculate()
{
	if (latched) {

		if (!latched->exists())
			handle_damage(this, 999);

		double alpha;
		alpha = ((latched->get_angle() + pa));
		pos = latched->normal_pos() + rr * unit_vector(latched->get_angle() + pa);

		vel = latched->get_vel();

		angle = normalize(ra + latched->get_angle(), PI2);
		sprite_index = iround(angle / (PI2/64)) + 16;
		sprite_index &= 63;

		double jr = (latched->size.x/2.0);
		if (latched->mass > 1) {
			latched->accelerate(this, angle, thrust * frame_time / latched->mass, booster_speed);
			jr *= jr * 0.5 * latched->mass; }
		else {
			latched->accelerate(this, angle, thrust * frame_time, booster_speed);
			jr *= jr * 0.5; }

		if (latched->isShip())
			((Ship*)latched)->turn_step += rotation * 1000 * rt * frame_time / jr;
		else
			latched->angle += rotation * 1000 * rt * frame_time / jr;
	};

	if (fire_frame > 0) fire_frame -= frame_time;
	else {
		while (fire_frame <= 0)
			fire_frame +=50;
		fire_index = (fire_index + 1) % 2; }
               
	if (smoke_frame > 0) smoke_frame -= frame_time;
	else {
		while (smoke_frame <= 0)
			smoke_frame += 25;
		game->add(new Animation(this, pos, data->spriteSpecial,
			first_frame, 20-first_frame, 50, LAYER_HOTSPOTS));
		if (first_frame > 0) first_frame -= 3; }


	if (fuel > 0) {
		fuel -= frame_time;	
		Vector2 ov = vel;
		HomingMissile::calculate();
		vel = ov;
		accelerate(this, angle, accel*frame_time, v); }
	else {
		if (latched)
			damage(latched, 0, 1);
		handle_damage(this, 999); }
}

void TauStormMissile::animate(Frame *space)
{
	sprite->animate(pos, sprite_index + fire_index*64, space);
}

void TauStormMissile::inflict_damage (SpaceObject *other)
{

	if (latched || other->isShot()) return;
	latched = other;
	rr = distance(other);
	ra = angle - other->get_angle();
	pa = other->trajectory_angle(this) - other->get_angle();
	rt = thrust * rr * sin((angle-other->trajectory_angle(this)));

//	play_sound
}

void TauStormMissile::handle_damage (SpaceObject *other, double normal, double direct)
{
	Shot::handle_damage(other, normal, direct);
	if ((state == 0) && (other->isShot()))
		damage(other, 1);
}

int TauStormMissile::canCollide(SpaceLocation *other)
{
	if (other == latched) return false;
	return Shot::canCollide(other);
}



REGISTER_SHIP(TauStorm)
