#include "../ship.h"
#include "../util/aastr.h"
#include "../melee/mcbodies.h"


REGISTER_FILE

void TauArchonFreezeColorEffects (RGB *c)
{
	int alpha = (c->filler ^ 255) + 1; 
	c->r = (c->r << 8) / alpha;
	c->r = ((c->r * 3) / 4) + 256/4;
	c->r = (c->r * alpha) >> 8;
	c->g = (c->g << 8) / alpha;
	c->g = ((c->g * 3) / 4) + 256/4;
	c->g = (c->g * alpha) >> 8;
	c->b = (c->b << 8) / alpha;
	c->b = ((c->b * 1) / 2) + 256/2;
	c->b = (c->b * alpha) >> 8;
//	gamma_color_effects (c);
	return;
}

class TauArchon : public Ship
{
	double		weaponRange, weaponVelocity;
	double		weaponDamage, weaponDamageMin, weaponArmour;
	double		weaponFuelSap;
	bool		weaponDoFreeze;
	bool		weaponReactiveDamage;
	int			weaponSoundTimer;
	int			weapon_sound_timer;
	int			weaponChargeTime, weapon_charge_counter;
	double		specialVelocity, specialRange;
	double		specialRangeLimiter;
	double		specialMaxDivergence;
	int			base_recharge_rate;

public:
	TauArchon(Vector2 opos, double shipAngle,
		ShipData *shipData, unsigned int code);

	virtual void calculate();
	virtual void calculate_fire_weapon();
	virtual void calculate_fire_special();
	virtual void animate(Frame *space);
	virtual RGB crewPanelColor();
	virtual RGB battPanelColor();
	virtual void calculate_hotspots();

};

class TauArchonShot : public Shot
{
	bool		do_freeze;
	int			index_base;
	double		min_damage;//, ast_damage;
	double		fuel_sap;
	double		divergence_factor;
	double		old_range;
	SpaceLocation	*rotation_base;
	bool		do_reactive_damage;

public:
	TauArchonShot(SpaceLocation *creator, Vector2 opos, double oangle,
		double ov, double odamage, double omindamage, double orange,
		double oarmour, double ofsap, SpaceSprite *osprite,
		bool ofreeze, double max_divergence, bool oreactive, double rangelimit);
	virtual void calculate();
	virtual void animate(Frame *space);
	virtual void inflict_damage(SpaceObject *other);
};


class TauArchonFrozen : public Shot
{
	Vector2		old_vel;
	bool		exploded_already;
	int			lifetime;

public:

	TauArchonFrozen(SpaceLocation *creator, SpaceObject *source, SpaceSprite *osprite);
	~TauArchonFrozen();
	virtual void inflict_damage(SpaceObject *other);
	virtual void calculate();
	virtual void animateExplosion();
};



TauArchon::TauArchon(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code) :
	Ship(opos, shipAngle, shipData, code)
{
	weaponChargeTime	= (int)(get_config_float("Weapon", "ChargeTime", 0) * 1000);
	weapon_charge_counter	= 0;
	weaponVelocity		= scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponRange			= scale_range(get_config_float("Weapon", "Range", 0));
	weaponDamage		= get_config_float("Weapon", "Damage", 0);
	weaponDamageMin		= get_config_float("Weapon", "DamageMin", 0);
//	weaponDamageAst		= get_config_float("Weapon", "DamageAst", 0);
	weaponArmour		= get_config_float("Weapon", "Armour", 0);
	weaponFuelSap		= get_config_float("Weapon", "FuelSap", 0);
	weaponDoFreeze		= (get_config_int("Weapon", "DoFreeze", 0) > 0);
	weaponSoundTimer	= get_config_int("Weapon", "SoundTimer", 0);
	weaponReactiveDamage= (get_config_int("Weapon", "DoReactiveDamage", 0) > 0);
	weapon_sound_timer	= 0;
	specialVelocity		= scale_velocity(get_config_float("Special", "Velocity", 0));
	specialRange		= scale_range(get_config_float("Special", "Range", 0));
	specialMaxDivergence= get_config_float("Special", "MaxDivergence", 0);
	specialRangeLimiter	= fabs(get_config_float("Special", "RangeLimiter", 0));
	if (specialRangeLimiter > 1) specialRangeLimiter = 1;
	base_recharge_rate	= recharge_rate;
}


void TauArchon::calculate()
{
	recharge_rate = base_recharge_rate * (1 - batt / (double)batt_max);
	if (recharge_rate <= frame_time)
		recharge_rate = frame_time + 1;
	if (recharge_rate <= 2*weapon_rate)
		recharge_rate = 2*weapon_rate + 1;
	recharge_amount = batt_max;

	Ship::calculate();
	
	if ((fire_weapon || fire_special) && (batt >= weapon_drain/(double)special_drain)/* && (weapon_recharge <= 0)*/) {
		weapon_charge_counter += frame_time;
		if (weapon_charge_counter > weaponChargeTime)
			weapon_charge_counter = weaponChargeTime;
//		recharge_step = recharge_rate;
	}
	else {
		weapon_charge_counter -= frame_time;
		if (weapon_charge_counter < 0)
			weapon_charge_counter = 0; }

	if (weapon_sound_timer > 0)
		weapon_sound_timer -= frame_time;

	return;
}


void TauArchon::calculate_fire_weapon()
{
	weapon_low = FALSE;

	if ((fire_weapon || fire_special) && (weapon_charge_counter >= weaponChargeTime))

		while (weapon_recharge <= 0) {
			
			if (batt < weapon_drain/(double)special_drain) {
				weapon_low = true;
				return; }

			double rx = tw_random(-12.0, 12.0);
			double ax = (rx/3.0) * ANGLE_RATIO;

			game->add(new TauArchonShot(this, Vector2(rx/2,11), angle+ax,
					weaponVelocity * tw_random(0.96, 1.08), weaponDamage,
					weaponDamageMin, (fire_special?specialRange:weaponRange) * tw_random(0.77, 1.17),
					weaponArmour, weaponFuelSap,
					data->spriteWeapon, weaponDoFreeze,
					fire_special?specialMaxDivergence:(-1),
					weaponReactiveDamage, specialRangeLimiter));

			batt -= (weapon_drain/(double)special_drain);//*0.9999;

			if (batt < 0) batt = 0;

			recharge_step = recharge_rate;

			weapon_recharge += weapon_rate;

			if (weapon_sound_timer <= 0) {
				if (fire_special)
					play_sound(data->sampleSpecial[0]);
				else
					play_sound(data->sampleWeapon[0]);
				weapon_sound_timer = weaponSoundTimer * tw_random(0.49, 1.63); }
		}
	return;
}


void TauArchon::calculate_fire_special()
{
	special_low = false;
	return;
}


void TauArchon::animate(Frame *space)
{
	int aa = get_tw_aa_mode();
	if ((weapon_charge_counter > 0) && (aa & AA_BLEND) && !(aa & AA_NO_AA)) {
		int	_old_trans = aa_get_trans();
		sprite->animate(pos, sprite_index, space);
		aa_set_trans(255.0*(1-weapon_charge_counter/(double)weaponChargeTime));
		if (aa_get_trans() < 255)
			data->more_sprites[0]->animate(pos, sprite_index, space);
		aa_set_trans(_old_trans); }
	else
		if (weapon_charge_counter > weaponChargeTime/2.0)
			data->more_sprites[0]->animate(pos, sprite_index, space);
		else
			sprite->animate(pos, sprite_index, space);
}


RGB TauArchon::crewPanelColor()
{
	RGB c = {255, 255, 255};
	return c;
}


RGB TauArchon::battPanelColor()
{
	RGB c = {85, 85, 255};
	return c;
}


void TauArchon::calculate_hotspots()
{
	if((thrust) && (hotspot_frame <= 0)) {
		game->add(new Animation( this, 
				normal_pos() - unit_vector(angle) * 15,
				game->hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, DEPTH_HOTSPOTS));
		hotspot_frame += hotspot_rate; }
	if (hotspot_frame > 0) hotspot_frame -= frame_time;
	return;
}



TauArchonShot::TauArchonShot(SpaceLocation *creator, Vector2 opos, double oangle,
		double ov, double odamage, double omindamage, double orange,
		double oarmour, double ofsap, SpaceSprite *osprite,
		bool ofreeze, double max_divergence, bool oreactive, double range_limit) :
	Shot(creator, opos, oangle, ov, odamage, orange, oarmour, creator, osprite),
	min_damage(omindamage), fuel_sap(ofsap)
{
	do_reactive_damage = oreactive;
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 10;
	explosionFrameSize  = 50;
	explosionSample = data->sampleWeapon[1+tw_random()%3];
	sprite_index = 80;
	do_freeze = ofreeze;
	index_base = (tw_random()%4)*20;
	attributes &= ~ATTRIB_STANDARD_INDEX;

	if (max_divergence >= 0) {
		
		int aa = get_tw_aa_mode();
		if ((tw_random()%2) && (aa & AA_ALPHA) && (aa & AA_BLEND) && !(aa & AA_NO_AA))
			set_depth(creator->get_depth() + 0.1);

		rotation_base = creator;
		double rnd = tw_random(0.0,1.0);
//		rnd *= rnd * rnd;
//		rnd = sqrt(rnd);
//		rnd *= rnd;

		divergence_factor = max_divergence * rnd;
	
		if (tw_random()%2)
			divergence_factor *= -1;

		old_range = distance(creator);

		if (max_divergence > 1e-20)
			range *= (1 - range_limit) + range_limit * (1-fabs(divergence_factor)/max_divergence);
	}
	else
		rotation_base = NULL;

}


void TauArchonShot::calculate()
{
	Shot::calculate();

	if (rotation_base) {
		if (rotation_base->exists()) {
			double t_a = trajectory_angle(rotation_base);
			pos = rotation_base->normal_pos() - unit_vector(t_a) * old_range;
			if (divergence_factor > 0)
				angle = normalize(t_a + PI/2, PI2);
			else
				angle = normalize(t_a - PI/2, PI2);
			changeDirection(angle);
			old_range += fabs(divergence_factor) * v * frame_time;
		}
		else
			rotation_base = NULL;
	}
}



void TauArchonShot::animate(Frame *space)
{
	if (exists())
		sprite->animate(pos, index_base + (int)(19.89 * d / range), space);
}

void TauArchonShot::inflict_damage(SpaceObject *other)
{
	double d_f = damage_factor;

	if (other->isShip()) {
		double bt = ((Ship*)other)->batt;
		other->handle_fuel_sap(this, fuel_sap);
		bt -= ((Ship*)other)->batt;
		if ((bt > 0) && (fuel_sap > 0)) {
			bt /= fuel_sap;
			if (bt < 1)
				damage_factor *= 1 - bt;
			else 
				damage_factor = 0;}
		if (damage_factor < min_damage)
			damage_factor = min_damage;
	}
	else {
		other->handle_fuel_sap(this, fuel_sap);
		damage_factor = min_damage; }

	bool freeze = false;

	if (other->isShip() && do_freeze) {
		if ((((Ship*)other)->crew <= damage_factor ) && (!other->isProtected())) {
			freeze = true;
			other->die();
			game->ship_died(((Ship*)other), this); }
	}
/*	else
		if (other->isAsteroid()) {
//			damage_factor = ast_damage;
//			if ( ((Asteroid*)other)->armour <= damage_factor) && (!other->isProtected())) {
//				do_freeze = true;
//				other->die(); }
			other->die();
			freeze = true;
		}*/

	if (freeze) {
		BITMAP* bmp = other->get_sprite()->get_bitmap_readonly( other->get_sprite_index() );
		BITMAP* tmp;
		int mcol = bitmap_mask_color( bmp );
		DATAFILE* image = new DATAFILE;
		image->type = DAT_RLE_SPRITE;
		tmp = create_bitmap_ex( bitmap_color_depth( bmp ), bmp->w, bmp->h );
		//void* old_color_effects = videosystem.color_effects;
		void* old_color_effects = (void *)videosystem.color_effects; //added (void *) 7/1/2003 Culture20
		videosystem.color_effects = TauArchonFreezeColorEffects;
	//	videosystem.update_colors();
		blit( bmp, tmp, 0, 0, 0, 0, bmp->w, bmp->h );
		image->dat = get_rle_sprite( tmp );
		SpaceSprite* frozen_sprite = new SpaceSprite( image, 1 );
		destroy_bitmap( tmp );
		//videosystem.color_effects = (void (__cdecl *)(struct RGB *))old_color_effects;
		videosystem.color_effects = (void (*)(struct RGB *))old_color_effects; //removed __cdecl 7/1/2003 Culture20
	//	videosystem.update_colors();
		delete image;
		game->add(new TauArchonFrozen(this, other, frozen_sprite));
		return;
	}

	if (do_reactive_damage)
		damage_factor += other->damage_factor;

	Shot::inflict_damage(other);
	damage_factor = d_f;
}



TauArchonFrozen::TauArchonFrozen(SpaceLocation *creator, SpaceObject *source, SpaceSprite *osprite) :
	Shot(creator, 0, source->get_angle(), 0, 0, 999, source->mass, source, osprite, 1.0)
{
	collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = ALL_LAYERS;
	ally_flag = 0;
	mass = source->mass;
	sprite_index = 0;
	attributes &= ~ATTRIB_STANDARD_INDEX;
	set_depth(source->get_depth());
	attributes &= ~ATTRIB_SHOT;
	old_vel = vel;
	explosionSample = data->sampleExtra[tw_random()%4];
	exploded_already = false;

	lifetime = 3000 + tw_random()%(15*60000);

	if (tw_random()%2)
		play_sound(data->sampleWeapon[4]);
	else
		play_sound(data->sampleWeapon[5]);
}

TauArchonFrozen::~TauArchonFrozen()
{
	delete sprite;
}

void TauArchonFrozen::inflict_damage(SpaceObject *other)
{
}

void TauArchonFrozen::calculate()
{
	Shot::calculate();
	if (length(vel-old_vel) > scale_velocity(15+tw_random()%20))
		handle_damage(this,armour,armour);
	else
		if ((lifetime -= frame_time) <= 0)
			handle_damage(this,armour,armour);
	old_vel = vel;
}

void TauArchonFrozen::animateExplosion()
{
	if (!exploded_already) {
		exploded_already = true;	
		game->add(new Animation(this, normal_pos(),
				data->spriteExtraExplosion, 0, 20,
				25, DEPTH_EXPLOSIONS, size.x/66.0));
/*		Query q;
		double blast_damage = mass;
		double blast_range = 150 * sqrt(mass/20);
		for (q.begin(this, OBJECT_LAYERS, blast_range); q.currento; q.next()) {
			if (q.currento->canCollide(this)) {
				damage(q.current, blast_damage * (blast_range - distance(q.currento)) / blast_range, 0);
				}
			}
		q.end();*/
	}
	return;
}




REGISTER_SHIP(TauArchon)
