#include "../ship.h"
REGISTER_FILE

#define turret_fire_frame_size 40

class AlaryBCTurret;


class AlaryBC : public Ship
{
	friend		AlaryBCTurret;

	int			death_frame, exp_frame;

	bool		dying;

	SpaceSprite	*dummy, *sprite_turret, *sprite_fire, *(sprite_engines[4]);

	bool		supersprite;
	int			ship_forceAA, turret_forceAA, fire_forceAA, engines_forceAA;

	int			engine_phase;

	double		weaponProximity, weaponVelocity, weaponAccel, weaponTR;
	double		weaponArmour;
	int			weaponLifetime;


	double		warheadRange, warheadVelocity, warheadTR;
	double		warheadDamage, warheadArmour;

	double		specialDamage, specialArmour, turretArmour, specialDamThr;
	double		specialRange, specialVelocity, specialTurnRate;
	double		specialMaxShots;

	bool		turrets_on, turrets_old;
	int			can_switch;

	double		specialRelativity;

	int			side;
	
	double		engines_armour;
	int			engines_death_frame, engines_death_count;
	int			old_shield_state;

	double		extraThreshold, extraCapacity, extraRelaxation;
	double		extraDamageReduction, extraFuelSapReduction, extraSpeedLossReduction;
	double		extraDirectDamageReduction;

	double		absorbed_damage;//, residual_damage;

	double		turn_step_128;

	AlaryBCTurret *turret[3];

public:
	AlaryBC (Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);
  
	virtual void calculate();
	virtual int  activate_weapon();
	virtual void calculate_fire_special();
	virtual void calculate_thrust();
	virtual void calculate_turn_left();
	virtual void calculate_turn_right();
	virtual void calculate_hotspots();
	virtual void animate(Frame *space);
	virtual	void animate_simple(Frame *space);
	virtual int  handle_damage(SpaceLocation* source, double normal, double direct);
	virtual int  handle_fuel_sap(SpaceLocation *source, double normal);
	virtual double handle_speed_loss(SpaceLocation *source, double normal);
	virtual		~AlaryBC();
  
};

class AlaryBCTorpedo : public SpaceObject
{
	int			armour, lifetime, smoke_frame, inactive;
	double		proximity, accel, maxspeed, turn_rate;

	int			wh_damage, wh_armour;
	double		wh_v, wh_turn_rate, wh_range;

public:

	AlaryBCTorpedo(SpaceLocation *creator, double ox, double oy, double oangle, int oinactive, double oaccel, double  omaxspeed,
		int olifetime, double oproximity, double oarmour, double otr, SpaceObject *otarget,
		SpaceSprite *osprite,
		double wdamage, double wrange, double warmour, double wv, double wtr);

	virtual void inflict_damage(SpaceObject *other);
	virtual void calculate();
	virtual int  handle_damage(SpaceLocation* source, double normal, double direct);
};


class AlaryBCWarhead : public HomingMissile
{
	int			smoke_frame;
public:
	AlaryBCWarhead(SpaceLocation *creator, double ox, double oy, double oangle, double ov, double odamage,
		double orange, double oarmour, double otrate, SpaceSprite *osprite, SpaceObject *otarget);
	virtual void calculate();
};


class AlaryBCTurret : public SpaceLocation
{
	friend		AlaryBC;
	AlaryBC		*ship;
	double		min_angle, max_angle, std_angle;
	int			recharge;
	bool		alive;
	int			fire_frame[2], fire_time[2];
	double		rel_x, rel_y;
	int			armour;

	int			barrel;
	int			shots_fired;

public:

	AlaryBCTurret (AlaryBC *oship, double or, double oa, double oangle,
		double omin_angle, double omax_angle, int team);
	double get_aim(SpaceObject *tgt);
	SpaceObject *get_target(SpaceObject *tgt);
	virtual void calculate();
	void sinc_it();
};

class AlaryBCTShot : public Missile
{
public:
	AlaryBCTShot(double ox, double oy, double oangle, double ov, double odamage, double orange, double oarmour,
		SpaceLocation *creator, SpaceLocation *opos, SpaceSprite *osprite, double relativity);
};


AlaryBC::AlaryBC (Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
        Ship(opos, shipAngle, shipData, code)
{
	engines_armour  = get_config_int("Ship", "EnginesArmour", 99);

	weaponVelocity	= scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponAccel		= scale_acceleration(get_config_float("Weapon", "Accel", 0), 0);
	weaponTR		= scale_turning(get_config_float("Weapon", "TurnRate", 0));
	weaponLifetime	= get_config_int("Weapon", "Lifetime", 0);
	weaponArmour	= get_config_float("Weapon", "Armour", 0);
	weaponProximity	= scale_range(get_config_float("Weapon", "Proximity", 0));

	warheadRange	= scale_range(get_config_float("Weapon", "WarheadRange", 0));
	warheadVelocity	= scale_velocity(get_config_float("Weapon", "WarheadVelocity", 0));
	warheadDamage	= get_config_float("Weapon", "WarheadDamage", 0);
	warheadArmour	= get_config_float("Weapon", "WarheadArmour", 0);
	warheadTR		= scale_turning(get_config_float("Weapon", "WarheadTurnRate", 0));
	
	specialRange    = scale_range(get_config_float("Special", "Range", 0));
    specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialDamage   = get_config_float("Special", "Damage", 0);
	specialArmour   = get_config_float("Special", "Armour", 0);
	specialTurnRate = scale_turning(get_config_float("Special", "TurnRate", 0));

	turrets_on = false;
	turrets_old = !turrets_on;
	can_switch = 0;

    turretArmour    = get_config_float("Special", "TurretArmour", 0);
	specialDamThr   = get_config_float("Special", "DamageFactorReactionThreshold", 0);
	specialMaxShots = get_config_float("Special", "MaxShots", 1);
	specialRelativity= get_config_float("Special", "Relativity", 0);

	side = -1;

	extraThreshold  = get_config_float("Extra", "Threshold", 0);
	extraCapacity   = get_config_float("Extra", "Capacity", 0);
	extraRelaxation = get_config_float("Extra", "Relaxation", 0) / 1000.0;
	extraDamageReduction     = get_config_float("Extra", "DamageReduction", 1);extraDirectDamageReduction = get_config_float("Extra", "DirectDamageReduction", 1);
	extraFuelSapReduction    = get_config_float("Extra", "FuelSapReduction", 1);
    extraSpeedLossReduction  = get_config_float("Extra", "SpeedLossReduction", 1);

	supersprite				= (get_config_int("GFX", "SuperSprite", 0) > 0);

	ship_forceAA			= get_config_int("GFX", "ShipAA", 0);
	turret_forceAA			= get_config_int("GFX", "TurretAA", 0);
	fire_forceAA			= get_config_int("GFX", "FireAA", 0);
	engines_forceAA			= get_config_int("GFX", "EnginesAA", 0);
	
	absorbed_damage = 0;
//	residual_damage = 0;
	old_shield_state= -1;
	engines_death_frame = -1;
	engines_death_count = -1;

	engine_phase = 0;

	death_frame = 3500 + tw_random()%2000;
	exp_frame = 0;
	dying = false;

	int i;
	for (i=0; i<3; i++) {
		turret[i] =  new AlaryBCTurret(this, 70.7, PI2/3*i, PI2/3*i, PI2/3*i-PI/2, PI2/3*i+PI/2, ally_flag);
		game->add(turret[i]);
	}

	turn_step_128 = 0;

	sprite_index = iround(angle / (PI2/128)) + 32;
	sprite_index &= 127;

	DATAFILE *rawdata = load_datafile("ships/shpalabs.dat");
	if(!rawdata) error("Error loading 'shpalabs.dat'", "xxx");

	dummy = NULL;

	if (supersprite) {
		dummy = new SpaceSprite(&rawdata[0], 1, SpaceSprite::MASKED);
		sprite = new SpaceSprite(&rawdata[0], 128, SpaceSprite::MASKED);
		sprite_turret = new SpaceSprite(&rawdata[128], 256, SpaceSprite::MASKED);
		sprite_fire = new SpaceSprite(&rawdata[384], 4, SpaceSprite::MASKED);
		for (i=0; i<4; i++)
			sprite_engines[i] = new SpaceSprite(&rawdata[388+i*2], 2, SpaceSprite::MASKED);
	}
	else {
		sprite = new SpaceSprite(&rawdata[0], 128, SpaceSprite::MASKED | SpaceSprite::MIPMAPED);
		sprite_turret = new SpaceSprite(&rawdata[128], 256, SpaceSprite::MASKED | SpaceSprite::MIPMAPED);
		sprite_fire = new SpaceSprite(&rawdata[384], 4, SpaceSprite::MASKED | SpaceSprite::MIPMAPED);
		for (i=0; i<4; i++)
			sprite_engines[i] = new SpaceSprite(&rawdata[388+i*2], 2, SpaceSprite::MASKED | SpaceSprite::MIPMAPED);
	}

	unload_datafile(rawdata);

}

void AlaryBC::calculate()
{
	update_panel = true;

	if (crew <= 0) {
		death_frame -= frame_time;
		if (death_frame <= 0) handle_damage(this,777,888);
		turn_step_128 += turn_rate * frame_time;
		
		double aaa, rrr;
		exp_frame -= frame_time;
		while (exp_frame <= 0) {
			exp_frame += 90+tw_random()%300;
			aaa = PI2 * ((tw_random()%1001)/1000.0);
			rrr = 66*sqrt((tw_random()%1001)/1000.0);
			if (tw_random()%2 == 0) {
				SpaceLocation *ani = new Animation(this, pos+rrr*unit_vector(aaa), data->spriteSpecialExplosion, 0, 10, 50, DEPTH_EXPLOSIONS);
				game->add(ani); }
//				ani->play_sound((SAMPLE *)(melee[MELEE_BOOM + 1].dat));}
			else {
				SpaceLocation *ani = new Animation(this, pos+rrr*unit_vector(aaa), data->spriteExtraExplosion, 0, 10, 50, DEPTH_EXPLOSIONS);
				game->add(ani);
				ani->play_sound(data->sampleWeapon[2]); }
//				ani->play_sound((SAMPLE *)(melee[MELEE_BOOM + 3].dat)); }
		}
	}


	if (!fire_special)
		can_switch = true;

	turn_step_128 += turn_step;
	turn_step = 0;

    Ship::calculate();

	while(fabs(turn_step_128) > (PI2/64) / 4) {
		if(turn_step_128 < 0.0) {
			angle -= (PI2/128);
			turn_step_128 += (PI2/128); }
		else
			if(turn_step_128 > 0.0) {
				angle += (PI2/128);
				turn_step_128 -= (PI2/128); }
		if(angle < 0.0)
			angle += PI2;
		if(angle >= PI2)
			angle -= PI2;
	}

	sprite_index = iround(angle / (PI2/128)) + 32;
	sprite_index &= 127;

	int i;
	for (i=0; i<3; i++)
		turret[i]->sinc_it();


	if (engines_death_frame > 0) {
		if (engines_death_count > 0) engines_death_count -= frame_time;
		else {
			engines_death_count += 100 +random()%200;
			engines_death_frame--;
			double tx = cos(angle), ty = sin(angle);
			double rrr= 42 - (random() % 85);
			double xx = ( -54*tx - rrr*ty);
			double yy = ( -54*ty + rrr*tx);
			game->add(new Animation(this, pos + Vector2(xx,yy), data->spriteSpecialExplosion, 0, 10, 50, DEPTH_SHIPS-0.25));
		}
	}

	int aaa = (int)floor(3.999 * absorbed_damage / extraCapacity);

	if ((aaa != old_shield_state) || (turrets_on != turrets_old)) {
		int i;
		for (i=6; i>0; i--)
			spritePanel->overlay(1, 7+aaa+(turrets_on?4:0), spritePanel->get_bitmap(i));
		update_panel = true;
		old_shield_state = aaa; 
		turrets_old = turrets_on;
	}

	absorbed_damage -= extraRelaxation * frame_time;
	if (absorbed_damage < 0) absorbed_damage = 0;

};

int AlaryBC::activate_weapon()
{
	if (crew <= 0) return false;

	game->add(new AlaryBCTorpedo(this, 30*side, 0, angle, 350/*oinactive*/, weaponAccel, weaponVelocity,
		weaponLifetime, weaponProximity, weaponArmour, weaponTR, target,
		data->spriteWeapon,
		warheadDamage,  warheadRange, warheadArmour, warheadVelocity, warheadTR));

	side *= -1;
	return true;
}

void AlaryBC::calculate_fire_special()
{
	if (crew <= 0) return;

	if (fire_special && can_switch) {
		can_switch = false;
		turrets_on = !turrets_on; }

}

void AlaryBC::calculate_thrust()
{
	if (crew <= 0) return;
    Ship::calculate_thrust();
}

void AlaryBC::calculate_turn_left()
{
	if (crew <= 0) return;
	if ((turn_left)&&(!turn_right))
		turn_step_128 -= turn_rate * frame_time;
}

void AlaryBC::calculate_turn_right()
{
	if (crew <= 0) return;
	if ((turn_right)&&(!turn_left))
		turn_step_128 += turn_rate * frame_time;
}

void AlaryBC::calculate_hotspots()
{
	if (crew <= 0) return;
	if (engines_armour > 0) return;
	if((thrust) && (hotspot_frame <= 0)) {
		game->add(new Animation(this,
			pos - unit_vector(angle)*size.x/3.8,
//			normal_x() - (cos(angle) * w / 3.8),
//			normal_y() - (sin(angle) * h / 3.8),
			game->hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, LAYER_HOTSPOTS));
		hotspot_frame += hotspot_rate; }
	if (hotspot_frame > 0) hotspot_frame -= frame_time;
	return;
}


void AlaryBC::animate_simple(Frame *space)
{
	if (state == 0) return;

	double tx = cos(angle), ty = sin(angle);

	int old_aa = get_tw_aa_mode();

	set_tw_aa_mode((engines_forceAA<0)?old_aa:engines_forceAA);
	if ((thrust)&&(engines_armour > 0)&&(crew>0)) {
		engine_phase = (engine_phase + 1) % 2;
		sprite_engines[0]->animate(Vector2(pos.x+0.5-50*tx, pos.y+0.5-50*ty), engine_phase, space);
		sprite_engines[1]->animate(Vector2(pos.x+0.5-49*tx-16*ty, pos.y+0.5-49*ty+16*tx), engine_phase, space);
		sprite_engines[1]->animate(Vector2(pos.x+0.5-49*tx+16*ty, pos.y+0.5-49*ty-16*tx), engine_phase, space);
		sprite_engines[2]->animate(Vector2(pos.x+0.5-49.5*tx-28.5*ty, pos.y+0.5-49.5*ty+28.5*tx), engine_phase, space);
		sprite_engines[2]->animate(Vector2(pos.x+0.5-49.5*tx+28.5*ty, pos.y+0.5-49.5*ty-28.5*tx), engine_phase, space);
		sprite_engines[3]->animate(Vector2(pos.x+0.5-48*tx-39*ty, pos.y+0.5-48*ty+39*tx), engine_phase, space);
		sprite_engines[3]->animate(Vector2(pos.x+0.5-48*tx+39*ty, pos.y+0.5-48*ty-39*tx), engine_phase, space);
	}

	set_tw_aa_mode((ship_forceAA<0)?old_aa:ship_forceAA);
	sprite->animate(pos,sprite_index, space);


	int i;
	for (i=0; i<3; i++) {
		double si, ta, ttx, tty, rx, ry;
		rx = turret[i]->rel_x; ry = turret[i]->rel_y;
		if (turret[i]->alive) {
			si = iround(normalize(angle+turret[i]->angle,PI2) / (PI2/128));
			ta = si * (PI2/128);
            ttx = cos(ta); tty = sin(ta);
			set_tw_aa_mode((fire_forceAA<0)?old_aa:fire_forceAA);
			if (turret[i]->fire_frame[0] < 4)
				sprite_fire->animate(Vector2(pos.x+0.5+ry*tx-rx*ty + 13*ttx+3*tty, pos.y+0.5+ry*ty+rx*tx + 13*tty-3*ttx), turret[i]->fire_frame[0], space);
			if (turret[i]->fire_frame[1] < 4)
				sprite_fire->animate(Vector2(pos.x+0.5+ry*tx-rx*ty + 13*ttx-3*tty, pos.y+0.5+ry*ty+rx*tx + 13*tty+3*ttx), turret[i]->fire_frame[1], space);
			set_tw_aa_mode((turret_forceAA<0)?old_aa:turret_forceAA);
			sprite_turret->animate(Vector2(pos.x+0.5+ry*tx-rx*ty, pos.y+0.5+ry*ty+rx*tx), (iround(si) + 32) & 127, space);
		}
		else	{
			set_tw_aa_mode((turret_forceAA<0)?old_aa:turret_forceAA);
			sprite_turret->animate(Vector2(pos.x+0.5+ry*tx-rx*ty, pos.y+0.5+ry*ty+rx*tx), ((iround(normalize(angle+turret[i]->angle,PI2)/(PI2/128)) + 32) & 127) + 128, space); }
	}

	set_tw_aa_mode(old_aa);
}

void AlaryBC::animate(Frame *space)
{
	if (!supersprite) {
		animate_simple(space);
		return; }

	int i;

	double tx = cos(angle), ty = sin(angle);
	BITMAP *bmp;
	bmp = dummy->get_bitmap(0);
	clear_to_color( bmp, bitmap_mask_color(bmp));


//  animate engines

	if ((thrust)&&(engines_armour > 0)&&(crew>0)) {
		engine_phase = (engine_phase + 1) % 2;
        sprite_engines[0]->draw(Vector2(91+0.5-50*tx, 91+0.5-50*ty), sprite_engines[0]->size(), engine_phase, bmp);
        sprite_engines[1]->draw(Vector2(92.5+0.5-49*tx-16*ty, 92.5+0.5-49*ty+16*tx), sprite_engines[1]->size(), engine_phase, bmp);
	    sprite_engines[1]->draw(Vector2(92.5+0.5-49*tx+16*ty, 92.5+0.5-49*ty-16*tx), sprite_engines[1]->size(), engine_phase, bmp);
        sprite_engines[2]->draw(Vector2(94+0.5-49.5*tx-28.5*ty, 94+0.5-49.5*ty+28.5*tx), sprite_engines[2]->size(), engine_phase, bmp);
	    sprite_engines[2]->draw(Vector2(94+0.5-49.5*tx+28.5*ty, 94+0.5-49.5*ty-28.5*tx), sprite_engines[2]->size(), engine_phase, bmp);
        sprite_engines[3]->draw(Vector2(95.5+0.5-48*tx-39*ty, 95.5+0.5-48*ty+39*tx), sprite_engines[3]->size(), engine_phase, bmp);
	    sprite_engines[3]->draw(Vector2(95.5+0.5-48*tx+39*ty, 95.5+0.5-48*ty-39*tx), sprite_engines[3]->size(), engine_phase, bmp);
	}


//	draw ship
	sprite->draw(Vector2(0, 0), size, sprite_index, bmp);

	
//	animate turrets

	for (i=0; i<3; i++) {

		double si, ta, ttx, tty, rx, ry;
		rx = turret[i]->rel_x; ry = turret[i]->rel_y;
		if (turret[i]->alive) {
			si = iround(normalize(angle+turret[i]->angle,PI2) / (PI2/128));
			ta = si * (PI2/128);
            ttx = cos(ta); tty = sin(ta);
			if (turret[i]->fire_frame[0] < 4)
				sprite_fire->draw(97+0.5+ry*tx-rx*ty + 13*ttx+3*tty, 97+0.5+ry*ty+rx*tx + 13*tty-3*ttx, turret[i]->fire_frame[0], bmp);
			if (turret[i]->fire_frame[1] < 4)
				sprite_fire->draw(97+0.5+ry*tx-rx*ty + 13*ttx-3*tty, 97+0.5+ry*ty+rx*tx + 13*tty+3*ttx, turret[i]->fire_frame[1], bmp);
			sprite_turret->draw(84+0.5+ry*tx-rx*ty, 84+0.5+ry*ty+rx*tx, (iround(si) + 32) & 127, bmp);
		}
		else
			sprite_turret->draw(84+0.5+ry*tx-rx*ty, 84+0.5+ry*ty+rx*tx, ((iround(normalize(angle+turret[i]->angle,PI2)/(PI2/128)) + 32) & 127) + 128, bmp);
	}

	
//  draw on screen

	int old_aa = get_tw_aa_mode();
	set_tw_aa_mode((ship_forceAA<0)?old_aa:ship_forceAA);
	dummy->animate(pos, 0, space);
	set_tw_aa_mode(old_aa);
};


int AlaryBC::handle_damage(SpaceLocation* source, double normal, double direct)
{
	double total = 0;

//	check for "repair"

	if (normal < 0) {
		crew -= normal;
		normal = 0; }
	if (direct < 0) {
		crew -= direct;
		direct = 0; }

	if (crew > crew_max) crew = crew_max;

	if (normal+direct <= 0) return 0;
	
//      damage reduction

	if (normal > extraThreshold) {
		absorbed_damage += extraThreshold;
		total += normal / extraDamageReduction;
		total += (normal - extraThreshold) / extraDirectDamageReduction; }
	else {
		absorbed_damage += normal;
		total += normal / extraDamageReduction; }

	if (absorbed_damage > extraCapacity) {
		total += (absorbed_damage - extraCapacity) / extraDirectDamageReduction;
		absorbed_damage = extraCapacity; }

//      direct_damage
	total += direct / extraDirectDamageReduction;

//      damage to deal right now
//	int total = floor(residual_damage);
//	residual_damage -= total;


//      hit zone calculation

	Vector2 dp = source->normal_pos();
	double dx = dp.x;//source->normal_x();
	double dy = dp.y;//source->normal_y();
	if (source->isLine()) {
		dx += ((SpaceLine*)source)->edge_x();
		dy += ((SpaceLine*)source)->edge_y(); }
	dx = min_delta(dx, pos.x, map_size.x);
	dy = min_delta(dy, pos.y, map_size.y);

	double alpha = normalize(atan3(dy, dx) - angle, PI2);

	if ((turret[0]->alive) && ((alpha > PI2-PI/12) || (alpha < PI/12)))
		turret[0]->armour -= total;
	if ((turret[2]->alive) && (alpha > 2*PI2/3-PI/12) && (alpha < 2*PI2/3 + PI/12))
		turret[2]->armour -= total;
	if ((turret[1]->alive) && (alpha > PI2/3-PI/12) && (alpha < PI2/3+PI/12))
		turret[1]->armour -= total;
	if ((engines_armour > 0) && (alpha >= PI-PI2/9) && (alpha <= PI+PI2/9)) {
		engines_armour -= total;
        if (engines_armour <=0) {
			play_sound(data->sampleExtra[0]);
			engines_death_frame = 8;
			engines_death_count = 500 +random()%100;
			speed_max /= 2.0;
			accel_rate /= 4.0;
			turn_rate /= 3.0; } }


	if ((source == this) && (dying)) {
//      remove turrets when dead;
		int i;
		for (i=0; i<3; i++)
			turret[i]->state = 0;
		play_sound((SAMPLE *)(melee[MELEE_BOOMSHIP].dat));
		game->add(new Animation(this, pos,
				game->kaboomSprite, 0, KABOOM_FRAMES, time_ratio, DEPTH_EXPLOSIONS));
		if (attributes & ATTRIB_NOTIFY_ON_DEATH) game->ship_died(this, source);
		state = 0; return 0;
	}

	crew -= total;

	if(crew > crew_max) {
		total += crew_max - crew;
		crew = crew_max;
	}
    if((crew <= 0)&&(!dying)) {
		total += crew;
		crew  = 0;
		dying = true;
//		state = 0;
//		play_sound((SAMPLE *)(melee[MELEE_BOOMSHIP].dat));
//		game->add(new Animation(this, x, y,
//			game->kaboomSprite, 0, KABOOM_FRAMES, time_ratio, DEPTH_EXPLOSIONS));
//		if (attributes & ATTRIB_NOTIFY_ON_DEATH) game->ship_died(this, source);
		turn_rate = scale_turning(12+tw_random()%80) * (1-2*(tw_random()%2)); 
		if (engines_armour > 0) {
			engines_armour = 0;
			play_sound(data->sampleExtra[0]);
			engines_death_frame = 8;
			engines_death_count = 500 +random()%100;
		}
	}

	if (crew < 0) {
		recharge_amount = 0;
		crew = 0; }

	return total;
}

int AlaryBC::handle_fuel_sap(SpaceLocation *source, double normal)
{
	normal = (normal / extraFuelSapReduction);
	return Ship::handle_fuel_sap(source, normal);
}

double AlaryBC::handle_speed_loss(SpaceLocation *source, double normal)
{
	normal = normal / extraSpeedLossReduction;
	return Ship::handle_speed_loss(source, normal);
}



AlaryBC::~AlaryBC()
{
	if (supersprite) delete dummy;
	delete sprite;
	delete sprite_turret;
	delete sprite_fire;
	int i;
	for (i=0; i<4; i++)
		delete sprite_engines[i];
}



AlaryBCTorpedo::AlaryBCTorpedo(SpaceLocation *creator, double ox, double oy, double oangle, int oinactive, double oaccel, double  omaxspeed,
		int olifetime, double oproximity, double oarmour, double otr, SpaceObject *otarget,
		SpaceSprite *osprite,
		double wdamage, double wrange, double warmour, double wv,
		double wtr) :
	SpaceObject(creator, 0, oangle, osprite),
	armour(oarmour), lifetime(olifetime), proximity(oproximity), accel(oaccel),
	maxspeed(omaxspeed), turn_rate(otr), inactive(oinactive),
	wh_damage(wdamage), wh_armour(warmour), wh_v(wv),
	wh_turn_rate(wtr), wh_range(wrange)
{
	target = otarget;
	layer = LAYER_SHOTS;
	set_depth(DEPTH_SHOTS);
//	attributes |= ATTRIB_SHOT;

//	double alpha, tx, ty;
//	alpha = (creator->get_angle());
//	tx = cos(alpha);
//	ty = sin(alpha);

//	x += creator->normal_x() + oy * tx - ox * ty;
//	y += creator->normal_y() + oy * ty + ox * tx;
	pos = normalize(creator->normal_pos() + rotate(Vector2(-ox, oy), -PI/2+creator->get_angle()));
//	vx = creator->get_vx();
//	vy = creator->get_vy();
	vel = creator->get_vel();

	smoke_frame = 0;
//        explosionSprite     = data->spriteWeaponExplosion;
//        explosionFrameCount = 10;
//        explosionFrameSize  = 50;
//        explosionSample = data->sampleWeapon[1];
}

void AlaryBCTorpedo::calculate()
{
	SpaceObject::calculate();
	lifetime -= frame_time;
	if (lifetime < 0) {

		//launch warheads or die
		die();
		return;

	}


	if (inactive > 0) inactive -= frame_time;

	if (target) if ((!target->isInvisible()) && (inactive <=0)) {
		double d_a = normalize(trajectory_angle(target) - angle, PI2);
		if (d_a > PI) d_a -= PI2;
		double ta = turn_rate * frame_time;
		if (fabs(d_a) < ta) ta = fabs(d_a);
		if (d_a > 0) angle += ta;
		else angle -= ta;
		angle = normalize(angle, PI2);

		if (distance(target) < proximity) {
			game->add(new Animation(this, pos, data->spriteWeaponExplosion, 0, 10, 50, DEPTH_EXPLOSIONS));
			
			game->add(new AlaryBCWarhead(this, 0, 10, angle,
				wh_v, wh_damage, wh_range*(1+0.0025*(tw_random()%101)), wh_armour, wh_turn_rate, data->spriteSpecial, target));
			game->add(new AlaryBCWarhead(this, 0, 10, angle - 50*PI/180,
				wh_v, wh_damage, wh_range*(1+0.0025*(tw_random()%101)), wh_armour, wh_turn_rate, data->spriteSpecial, target));
			game->add(new AlaryBCWarhead(this, 0, 10, angle + 50*PI/180,
				wh_v, wh_damage, wh_range*(1+0.0025*(tw_random()%101)), wh_armour, wh_turn_rate, data->spriteSpecial, target));
			game->add(new AlaryBCWarhead(this, 0, 10, angle - 75*PI/180,
				wh_v, wh_damage, wh_range*(1+0.0025*(tw_random()%101)), wh_armour, wh_turn_rate, data->spriteSpecial, target));
			game->add(new AlaryBCWarhead(this, 0, 10, angle + 75*PI/180,
				wh_v, wh_damage, wh_range*(1+0.0025*(tw_random()%101)), wh_armour, wh_turn_rate, data->spriteSpecial, target));
			
			state = 0;
		}
	}
	else {
		if (inactive <= 0) target = NULL;

		//find another target???????

	}
	
	sprite_index = iround(angle / (PI2/64)) + 16;
	sprite_index &= 63;

	accelerate(this, angle, accel*frame_time, maxspeed);

	while (smoke_frame <= 0) {
		smoke_frame += 50;
		game->add(new Animation(this, pos - 14*unit_vector(angle), data->spriteSpecial,
			128, 12, 50, LAYER_HOTSPOTS));
	}
	smoke_frame -= frame_time;


	return;

}

void AlaryBCTorpedo::inflict_damage(SpaceObject *other)
{
	if (!other->isShot()) {
		game->add(new Animation(this, pos, data->spriteWeaponExplosion, 0, 10, 50, DEPTH_EXPLOSIONS));
		state = 0; 
		play_sound((SAMPLE *)(melee[MELEE_BOOM].dat));
	}
};

int AlaryBCTorpedo::handle_damage(SpaceLocation *source, double normal, double direct)
{
	if (direct+normal > 0) armour -= direct+normal;
	if (armour <= 0) {
		game->add(new Animation(this, pos, data->spriteWeaponExplosion, 0, 10, 50, DEPTH_EXPLOSIONS));
		state = 0; }
	return direct+normal;
}


AlaryBCWarhead::AlaryBCWarhead(SpaceLocation *creator, double ox, double oy, double oangle, double ov, double odamage,
		double orange, double oarmour, double otrate, SpaceSprite *osprite, SpaceObject *otarget) :
	HomingMissile(creator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, 
	otrate, creator, osprite, otarget)
{
	sprite_index += 64;

	explosionSprite     = data->spriteExtraExplosion;
	explosionFrameCount = 10;
	explosionFrameSize  = 50;
	explosionSample = data->sampleWeapon[2];
	smoke_frame = 0;
	play_sound(data->sampleWeapon[1], 64);
}


void AlaryBCWarhead::calculate()
{
	HomingMissile::calculate();
	
	if (sprite_index < 64)
		sprite_index += 64;

	while (smoke_frame <= 0) {
		smoke_frame += 25;
		game->add(new Animation(this, pos -2*unit_vector(angle), data->spriteExtra,
			0, 12, 50, LAYER_HOTSPOTS));
	}
	smoke_frame -= frame_time;
}



void AlaryBCTurret::sinc_it()
{
	double tx = cos(ship->angle);
	double ty = sin(ship->angle);

//	x = ship->x + rel_y*tx - rel_x*ty;
//	y = ship->y + rel_y*ty + rel_x*tx;
	pos = normalize(ship->normal_pos() + rotate(Vector2(-rel_x, rel_y), -PI/2+ship->get_angle()));

//	vx = ship->vx;
//	vy = ship->vy;
	vel = ship->get_vel();
}

AlaryBCTurret::AlaryBCTurret (AlaryBC *oship, double or, double oa, double oangle,
				double omin_angle, double omax_angle, int team) :
	SpaceLocation(oship,0,oangle)
//	barrel(0), ship(oship), recharce(0)
						
{
	target = NULL;
	shots_fired = 0;
	barrel = 0;
	ship = oship;
	rel_x = or*sin(oa);
	rel_y = or*cos(oa);
	recharge = 0;
	min_angle = omin_angle; max_angle = omax_angle;
	std_angle = oangle;
	
	collide_flag_anyone = 0;
	collide_flag_sameteam = 0;
	collide_flag_sameship = 0;

	armour = ship->turretArmour;

	collide_flag_anyone = 0;
	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;
	sinc_it();

	fire_frame[0] = fire_frame[1] = 10;

	alive = true;
}

double AlaryBCTurret::get_aim(SpaceObject *tgt)
{

	if (tgt == NULL)
		return (-1);
		
	Vector2 tv = tgt->get_vel() - ship->specialRelativity * ship->get_vel();

	double tvx = tv.x;//tgt->get_vx() - ship->specialRelativity * ship->vx;
	double tvy = tv.y;//tgt->get_vy() - ship->specialRelativity * ship->vy;
//        double tv2 = tvx*tvx + tvy*tvy;
	tv = min_delta(tgt->normal_pos(), pos);
	double rx  = tv.x;//min_delta(tgt->normal_x(), normal_x(), X_MAX);
	double ry  = tv.y;//min_delta(tgt->normal_y(), normal_y(), Y_MAX);
	double r2  = rx*rx + ry*ry;
	double u2  = ship->specialVelocity;
	u2 *= u2;
	double d2v = u2 - (tvx*tvx + tvy*tvy);
	double t = (rx*tvx + ry*tvy);
	double q, p;
	if (fabs(d2v/u2) > 0.01 ) {
		q = t*t + r2*d2v;
		if (q > 0) q = sqrt(q);
		else    return (-1);
		p = (t+q)/d2v;
		q = (t-q)/d2v;
		if (p > 0) t = p;
		else       t = q;
		if (t < 0) return (-1);
	}
	else {
		if (fabs(t)<1e-6) return (-1);
		else    t = - 0.5 * r2 / t;
		if (t < 0) return (-1); }
	if (t * ship->specialVelocity > ship->specialRange) return(-1);
	t = normalize((atan3(tvy*t + ry, tvx*t + rx)) - ship->angle, PI2);
	double d_a = normalize(t - min_angle, PI2);
	if (d_a > PI) d_a -= PI2;
	if (d_a > 0) {
		d_a = normalize(t - max_angle, PI2);
		if (d_a > PI) d_a -= PI2;
		if (d_a < 0)
			return (t); }
	return (-1);
}

SpaceObject *AlaryBCTurret::get_target(SpaceObject *tgt)
{
	double d_a, prix=-1, prix_c, aim; //!!!
	Query q;
	SpaceObject *tgt0=tgt;

	for (q.begin(this, OBJECT_LAYERS, ship->specialRange); q.currento; q.next())
		if ((!q.currento->isInvisible()) && (!q.currento->sameTeam(this))
				&& (!q.currento->isPlanet())
				&& (q.currento->collide_flag_anyone&bit(LAYER_SHOTS))
				&& (q.currento != tgt) ) {
			aim = get_aim(q.currento);
			if (aim >= 0) {
				d_a = normalize(aim - angle, PI2);
				if (d_a > PI) d_a -= PI2;
				d_a = fabs(d_a);
			
				if (q.currento->isShip()) {
					if (q.currento == ship->target) prix_c = 4;
					else
						prix_c = 3; }
				else
					if (q.currento->damage_factor <= 0)
						prix_c = 2;
					else
						if (q.currento->damage_factor >= ship->specialDamThr)
							prix_c = 1;
						else
							prix_c = 0;
				prix_c -= d_a/PI;
				if (prix_c > prix) {
					prix = prix_c;
					tgt = q.currento; }
			}
		}


	if (tgt != tgt0)
		return tgt;
	else
		return NULL;

}



void AlaryBCTurret::calculate()
{
	if (!ship->exists()) {
		state = 0; return; }

	SpaceLocation::calculate();

	if (!alive) return;

	if (armour <= 0) {
		alive = false;
		play_sound((SAMPLE *)(melee[MELEE_BOOM + 1].dat));
//		play_sound(ship->data->sampleSpecial[1]);
//                        game->add(new Animation(this, turret[i]->normal_x(), turret[i]->normal_y(),
//                                          ship->data->spriteWeaponExplosion, 0, 10, 50, DEPTH_EXPLOSIONS));
		game->add(new FixedAnimation(this, this, ship->data->spriteSpecialExplosion,
			0, 10, 50, DEPTH_EXPLOSIONS));
		return; }

	int i;
	for (i=0; i<2; i++) {
		if ((fire_time[i] > 0) && (fire_frame[i] < 4)) fire_time[i] -= frame_time;
		else {
			fire_time[i] += turret_fire_frame_size;
			fire_frame[i]++; } }

                
	double d_a, aim; //!!!

	if (ship->turrets_on) {
		if (target) {
			if (!target->exists()) {
				shots_fired = 0;
				target = get_target(NULL); }
/*			else {
				if (target->isShot()) {
					if (shots_fired >= ((Shot*)target)->armour * ship->specialMaxShots / ship->specialDamage) {
						shots_fired = 0;
						target = get_target(NULL); } }
				else {
					if (target->isAsteroid()) 
						if (shots_fired >= ship->specialMaxShots) {
							shots_fired = 0;
							target = get_target(NULL); }
				}
			}
*/		
		}
		else {
			shots_fired = 0;
			target = get_target(NULL); }

		aim = get_aim(target);

		if (aim < 0) {
			shots_fired = 0;
			target = get_target(NULL); }
	}

	else {
		target = NULL;
		shots_fired = 0;
		aim = -1; }
        

	if (aim >= 0)
		d_a = normalize(aim - angle, PI2);
	else
		d_a = normalize(std_angle - angle, PI2);
	if (d_a > PI)
		d_a -= PI2;
	
	double delta = ship->specialTurnRate * frame_time;

	if (fabs(d_a) <= delta)
		delta = fabs(d_a);
	if (d_a > 0)
		angle += delta;
	else
		angle -= delta;

	if (recharge > 0) recharge -= frame_time;
	if (target && (recharge <= 0)) {
		if (fabs(d_a) <= (0.25 * (target->size.x+target->size.y)/2.0) / distance(target)) {
			if (ship->batt >= ship->special_drain) {
				recharge += ship->special_rate;
				ship->batt -= ship->special_drain;
				play_sound(ship->data->sampleSpecial[0]);
				shots_fired++;
				if (barrel == 0) {
					game->add(new AlaryBCTShot(-3, 14, normalize(angle+ship->angle, PI2), ship->specialVelocity,
						ship->specialDamage, ship->specialRange,
						ship->specialArmour, ship, this, ship->data->spriteSpecial, ship->specialRelativity));
					fire_frame[0] = 0; fire_time[0] =  turret_fire_frame_size;
					barrel = 1; }
				else {
					game->add(new AlaryBCTShot(3, 14, normalize(angle+ship->angle, PI2), ship->specialVelocity,
						ship->specialDamage, ship->specialRange,
						ship->specialArmour, ship, this, ship->data->spriteSpecial, ship->specialRelativity));
					fire_frame[1] = 0; fire_time[1] =  turret_fire_frame_size;
					barrel = 0; }
			}
		}
	}

}

AlaryBCTShot::AlaryBCTShot(double ox, double oy, double oangle, double ov, double odamage, double orange, double oarmour,
               SpaceLocation *creator, SpaceLocation *opos, SpaceSprite *osprite, double relativity) :
	Missile(creator, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, 
	creator, osprite, relativity)

{
	set_depth(DEPTH_SHIPS+0.25);
	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;

	explosionSprite     = data->spriteSpecialExplosion;
	explosionFrameCount = 10;
	explosionFrameSize  = 50;
	explosionSample = data->sampleSpecial[1];
	double tx = cos(angle);
	double ty = sin(angle);
//	x = opos->normal_x() + oy*tx - ox*ty;
//	y = opos->normal_y() + oy*ty + ox*tx;
	pos = normalize(opos->normal_pos() + rotate(Vector2(-ox, oy), -PI/2+angle));
}


REGISTER_SHIP(AlaryBC)
