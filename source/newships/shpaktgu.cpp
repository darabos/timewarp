/* $Id$ */ 
#include "../ship.h"
#include "../melee/mshot.h"
REGISTER_FILE
#include <string.h>

class AktunGunner;

class AktunComSat : public SpaceObject
{
public:
IDENTITY(AktunComSat);
	int frame;
	AktunGunner *owner;
	
	double lRange;
	int    lDamage;
	int    lFrames;
	int    lRechargeRate;
	int    lRecharge;
	int    lColor;
	int    armour;
	
  public:
	  AktunComSat(double oangle, double orange, int odamage, int oframes,
		  int orechargerate, int ocolor, int oarmour, AktunGunner *oship,
		  SpaceSprite *osprite);
	  
	  virtual void calculate();
	  virtual int handle_damage(SpaceLocation* source, double normal, double direct);
	  //virtual void handle_damage(SpaceLocation *source);
	  virtual int canCollide(SpaceLocation *other);
	  virtual void death();
};

class AktunGunner : public Ship {
public:
IDENTITY(AktunGunner);
	int    weaponColor;
	double weaponRange1, weaponRange2, weaponDrain2;
	int    weaponDamage;
	
	double specialRange;
	double specialVelocity;
	int    specialDamage;
	int    specialArmour;
	
	double extraRange;
	int    extraDamage;
	int    extraFrames;
	int    extraRechargeRate;
	int    extraColor;
	int    extraArmour;
	int    extraDrain;
	
public:
	AktunGunner(Vector2 opos, double shipAngle, ShipData *shipData, int shipCollideFlag);
	
	int num_ComSats;
	int max_ComSats;
	AktunComSat **ComSat;
	
	virtual int activate_weapon();
	virtual int activate_special();
	
};

class AktunLaser : public Laser {
public:
IDENTITY(AktunLaser);
	Ship *owner;
	
public:
	AktunLaser(SpaceLocation *creator, double langle, int lweaponColor, double lrange, int ldamage, int lfcount,
		SpaceLocation *opos, Vector2 relpos, Ship *lowner);
	
	virtual void inflict_damage(SpaceObject *other);
};

AktunGunner::AktunGunner(Vector2 opos, double shipAngle, ShipData *shipData, int shipCollideFlag) :
Ship(opos, shipAngle, shipData, shipCollideFlag)

{
	weaponColor  = get_config_int("Weapon", "Color", 0);
	weaponRange1  = scale_range(get_config_float("Weapon", "Range1", 0));
	weaponDamage = get_config_int("Weapon", "Damage", 0);

	weaponDrain2 = get_config_float("Weapon", "WeaponDrain2", 0);
	weaponRange2 = scale_range(get_config_float("Weapon", "Range2", 0));
	
	specialRange         = scale_range(get_config_float("Special", "Range", 0));
	specialVelocity      = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialDamage        = get_config_int("Special", "Damage", 0);
	specialArmour        = get_config_int("Special", "Armour", 0);
	
	extraRange        = scale_range(get_config_float("Extra", "Range", 0));
	extraFrames       = get_config_int("Extra", "Frames", 0);
	extraDamage       = get_config_int("Extra", "Damage", 0);
	extraDrain        = get_config_int("Extra", "Drain", 0);
	extraRechargeRate = get_config_int("Extra", "RechargeRate", 0);
	extraColor        = get_config_int("Extra", "Color", 0);
	extraArmour       = get_config_int("Extra", "Armour", 0);
	
	max_ComSats = get_config_int( "Extra", "Number", 0);
	num_ComSats = 0;
	ComSat = new AktunComSat*[max_ComSats];
	for (int i = 0; i < max_ComSats; i += 1) {
		ComSat[i] = NULL;
	}
	// for debugging purpose
	debug_id = 5;
}

int AktunGunner::activate_weapon()
{
	if(fire_special)
		return(FALSE);

	// the simplest weapon
	game->add(new AktunLaser(this, angle,
		weaponColor, weaponRange1, weaponDamage, weapon_rate,
		this, Vector2(-10, 8), this));

	// the more expensive weapon: test if there's enough energy left after the first
	// drain, then subtract the extra drain (the default drain is subracted earlier).
	if (batt >= weapon_drain + weaponDrain2)
	{
		game->add(new AktunLaser(this, angle,
			weaponColor, weaponRange2, weaponDamage, weapon_rate,
			this, Vector2(10, 8), this));
		batt -= weaponDrain2;
	}
	return(TRUE);
}

int AktunGunner::activate_special()
{
//	if ((fire_weapon) && (batt >= extraDrain))
//	{
		if (num_ComSats == max_ComSats) {
			num_ComSats -= 1;
			ComSat[0]->state = 0;
			memcpy(&ComSat[0], &ComSat[1], sizeof(AktunComSat*) * num_ComSats);
			ComSat[num_ComSats] = NULL;
		}
		AktunComSat *tmp = new AktunComSat(0.0, extraRange,
			extraDamage, extraFrames, extraRechargeRate, extraColor, extraArmour, this,
			data->spriteExtra);
		ComSat[num_ComSats] = tmp;
		num_ComSats += 1;
//		batt -= extraDrain;
		game->add( tmp );
//		return(FALSE);
//	}

		/*
	game->add(new Missile(this,
		Vector2(0.0, size.y / 2.0), angle, specialVelocity, specialDamage, specialRange,
		specialArmour, this, data->spriteSpecial) );
		*/
	return(TRUE);
}


AktunLaser::AktunLaser(SpaceLocation *creator, double langle, int lweaponColor, double lrange, int ldamage,
					   int lfcount, SpaceLocation *opos, Vector2 relpos, Ship *lowner) :
Laser(creator, langle, pallete_color[lweaponColor], lrange,
	  ldamage, lfcount, opos, relpos, true),
	  owner(lowner)
{
	// for debugging purpose
	debug_id = 6;
}

void AktunLaser::inflict_damage(SpaceObject *other)
{
	sound.stop(owner->data->sampleWeapon[0]);
	Laser::inflict_damage(other);
	//sound.stop((SAMPLE *)(melee[1].dat));
	sound.play(owner->data->sampleWeapon[1]);
}

AktunComSat::AktunComSat(double oangle, double orange, int odamage,
						 int oframes, int orechargerate, int ocolor, int oarmour, AktunGunner *oship,
						 SpaceSprite *osprite) :
SpaceObject(oship, 0, 0, osprite),
lRange(orange),
lDamage(odamage),
lFrames(oframes),
lRechargeRate(orechargerate),
lRecharge(0),
lColor(ocolor),
armour(oarmour)
{
	owner = oship;
	layer = LAYER_SPECIAL;
	collide_flag_anyone = ALL_LAYERS - bit(LAYER_CBODIES);
	angle = oangle;
	pos = ship->normal_pos();
	if(!(ship && ship->exists())) state = 0;
	// for debugging purpose
	debug_id = 7;
}

void AktunComSat::calculate()
{
	if (owner && !owner->exists())
		owner = 0;
	
	if(!(ship && ship->exists()))	// this is not necessarily the same as owner.
	{
		state = 0;
		return;
	}
	
	SpaceObject::calculate();

	sprite_index++;
	if(sprite_index == 40)
		sprite_index = 0;
	
	if(lRecharge > 0) {
		lRecharge -= frame_time;
		return;
	}
	
	vel *= 1 - .0005 * frame_time;
	
	if (magnitude_sqr(vel) < 0.05 * 0.05) {
		vel = 0;
	}
	
	Query q;
	for (q.begin(this, OBJECT_LAYERS &~ bit(LAYER_CBODIES), lRange); q.currento; q.next()) {
		if (!q.currento->isInvisible() && !q.currento->sameTeam(this)) {
			SpaceLocation *l;
			l = new PointLaser(this, pallete_color[lColor], 1, lFrames, 
				this, q.currento, 0);
			game->add(l);
			if (l->exists()) {
				sound.play(ship->data->sampleExtra[0]);
				lRecharge += lRechargeRate;
				break;
			}
			else l->state = 0;
		}
	}
	return;
}

int AktunComSat::canCollide(SpaceLocation *other) {
	return SpaceObject::canCollide(other);
}

int AktunComSat::handle_damage(SpaceLocation* source, double normal, double direct)
{
	double tot;
	tot = normal+direct;
	if ( tot > 0 ) {
		armour -= iround(tot);
		
		if(armour <= 0) {
			armour = 0;
			state = 0;
			game->add(new Animation(this, pos,
				meleedata.kaboomSprite, 0, KABOOM_FRAMES, 50, LAYER_EXPLOSIONS));
			sound.stop(data->sampleExtra[0]);
			sound.play(data->sampleExtra[0]);
		}
	}
	return 1;
}

void AktunComSat::death()
{
	if (owner)
	{
		for (int i = 0; i < owner->num_ComSats; i++ ) {
			if (owner->ComSat[i] == this)
			{
				owner->ComSat[i] = NULL;
				owner->num_ComSats -= 1;
				memmove(&owner->ComSat[i], &owner->ComSat[i+1], (owner->num_ComSats-i) * sizeof(AktunComSat*));
				return;
			}
		}
	}
}



REGISTER_SHIP(AktunGunner)
