/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

#include "../melee/mview.h"
#include "../melee/mcbodies.h"

#define WEAPON_MASS .25

class KorvianSidekick : public Ship {
public:
IDENTITY(KorvianSidekick);
	public:

	double absorption; //added for gob

        double       Sidekick_turn_step;

	double       weaponRange;
	double       weaponVelocity;
	int          weaponDamage;
	int          weaponArmour;

	double       specialAccelRate;
	double       specialSpeedMax;
	int          specialHotspotRate;
	int          specialArmour;

	double       SidekickAngle;

	int          recoil;
	int          recoil_rate;
	int          recoil_range;
	double       Sidekick_turn_rate;

	SpaceObject* tugger;
	double tuggerDistance;
	double maxTuggerSpeed;
	double k;

	KorvianSidekick(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

	virtual void calculate();
	virtual void animate(Frame *space);

	virtual void calculate_turn_left();
	virtual void calculate_turn_right();
	virtual int activate_weapon();
	virtual int activate_special();
	virtual void calculate_hotspots();
};

class SidekickLaser : public PointLaser {
public:
IDENTITY(SidekickLaser);
public:
	SidekickLaser(SpaceLocation *creator, 
		SpaceLocation *lsource, SpaceObject *ltarget, Vector2 rel_pos = Vector2(0,0)) ;
	virtual void inflict_damage(SpaceObject *other);
};

SidekickLaser::SidekickLaser(SpaceLocation *creator, 
						 SpaceLocation *lsource, SpaceObject *ltarget, Vector2 rel_pos) :
	PointLaser(creator, pallete_color[15], 0, 1, lsource, ltarget, rel_pos)
{
}

void SidekickLaser::inflict_damage(SpaceObject *other) {

}

class SidekickMissile : public Missile {
public:
IDENTITY(SidekickMissile);
  public:
  SidekickMissile(double oangle, double ov, int odamage, double orange,
    int oarmour, Ship *oship, SpaceSprite *osprite);
    virtual void inflict_damage(SpaceObject *other);
};

KorvianSidekick::KorvianSidekick(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)
{
	absorption = 0;

	collide_flag_sameship = bit(LAYER_SPECIAL);
  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);

  specialSpeedMax    = scale_velocity(get_config_float("Special", "SpeedMax", 0));
  double raw_specialHotspotRate = get_config_int("Special", "HotspotRate", 0);
	specialHotspotRate = scale_frames(raw_specialHotspotRate);
  specialAccelRate   = scale_acceleration(get_config_float("Special", "AccelRate", 0), raw_specialHotspotRate);
  specialArmour      = get_config_int("Special", "Armour", 0);

  SidekickAngle = 0.0;
  recoil = 0;

  recoil_rate = scale_frames(get_config_float("Turret", "RecoilRate",0));
  if (recoil_rate > weapon_rate) recoil_rate = weapon_rate;
  recoil_range = get_config_int("Turret", "Recoil", 0);
  if (recoil_range < 0) recoil_range = 0;
  Sidekick_turn_rate = scale_turning(get_config_float("Turret", "TurnRate", 0));

  Sidekick_turn_step = 0;

  tugger = NULL;
  tuggerDistance = scale_range(get_config_float("Special", "TowDistance", 0));
  maxTuggerSpeed = specialSpeedMax;
  k = .01; //'springiness' constant of the Korvian's special

	debug_id = 1038;
}

int KorvianSidekick::activate_weapon()
{
  if(fire_special)
    return(FALSE);
  add(new SidekickMissile(
	SidekickAngle, weaponVelocity,
    weaponDamage, weaponRange, weaponArmour, this, data->spriteWeapon));

  if (mass > 0)
	accelerate(this, SidekickAngle, -WEAPON_MASS / mass * weaponVelocity, MAX_SPEED);
  recoil += recoil_rate;

  return(TRUE);
}

int KorvianSidekick::activate_special()
{
	SpaceObject *o;
	double closestDistance = tuggerDistance, d;
	int foundTugger = FALSE;
	Query a;
	if(tugger == NULL) {
		for (a.begin(this, bit(LAYER_SHIPS) | bit(LAYER_SHOTS) | bit(LAYER_SPECIAL) | bit(LAYER_CBODIES),
			tuggerDistance, QUERY_OBJECT); a.current; a.next()) 
		{
			o = a.currento;
			if( (!o->isInvisible()) && (o->collide_flag_anyone & bit(LAYER_LINES))
					&& abs(get_vel() - o->get_vel()) < maxTuggerSpeed) {
				d = distance(o);
				if(d < closestDistance) {
					foundTugger = TRUE;
					closestDistance = d;
					tugger = o;
				}
			}
		}
		if(tugger == NULL)
			return FALSE;
			//play_sound2(data->sampleExtra[0]);
	}
	else
	{
		tugger = NULL;
		//play_sound2(data->sampleExtra[0]);
	}
	return TRUE;
/*  if(turn_left && (recoil<=0))
    Sidekick_turn_step -= frame_time * Sidekick_turn_rate;
  if(turn_right && (recoil<=0))
    Sidekick_turn_step += frame_time * Sidekick_turn_rate;

  while (fabs(Sidekick_turn_step) > (PI2/64)/2) {
    if (Sidekick_turn_step < 0.0 ) {
      SidekickAngle -= (PI2/64);
      Sidekick_turn_step += (PI2/64); }
    else {
      SidekickAngle += (PI2/64);
      Sidekick_turn_step -= (PI2/64); }
  }
  SidekickAngle = normalize(SidekickAngle, PI2);

  return(FALSE);*/
}

void KorvianSidekick::calculate_turn_left()
{
	 if(turn_left /*&& (recoil<=0)*/)
		Sidekick_turn_step -= frame_time * Sidekick_turn_rate;
	//Ship::calculate_turn_left();
}

void KorvianSidekick::calculate_turn_right()
{
	  if(turn_right /*&& (recoil<=0)*/)
    Sidekick_turn_step += frame_time * Sidekick_turn_rate;
	  //Ship::calculate_turn_right();
}

void KorvianSidekick::calculate()
{
	if(tugger != NULL && !tugger->exists()) tugger = NULL;
	if(tugger != NULL && tugger->exists()) {
		double x = distance(tugger)-tuggerDistance;
		//Vector2 v = tugger->pos-pos;
		if(x > 0) {
			if (mass > 0)
				accelerate(tugger, trajectory_angle(tugger), k*x / mass, abs(tugger->get_vel()));

			if (tugger->mass > 0)
				tugger->accelerate(this, tugger->trajectory_angle(this), k*x / tugger->mass, MAX_SPEED);
		}
	}
	if(tugger != NULL && tugger->exists() && !tugger->isInvisible()) {
		game->add(new SidekickLaser(this, this, tugger));
	}
	turn_right = TRUE;
	Ship::calculate_turn_right();
	while (fabs(Sidekick_turn_step) > (PI2/64)/2) {
    if (Sidekick_turn_step < 0.0 ) {
      SidekickAngle -= (PI2/64);
      Sidekick_turn_step += (PI2/64); }
    else {
      SidekickAngle += (PI2/64);
      Sidekick_turn_step -= (PI2/64); }
  }
  SidekickAngle = normalize(SidekickAngle, PI2);

  if(recoil > 0) {
	  recoil -= frame_time;
	if (recoil < 0) recoil = 0;
  }
  Ship::calculate();
}

void KorvianSidekick::animate(Frame *space)
{
	double rec;
	int Sidekick_index;

	/*
	ra = normalize(angle + SidekickAngle, PI2);
	Sidekick_index = get_index(ra);
	bmp = data->spriteShip->get_bitmap(64);
	clear_to_color( bmp, makecol(255,0,255));
	sprite->draw(0, 0, sprite_index, bmp);
	rec = (double)recoil/recoil_rate;
	rec *= rec * recoil_range;
	data->spriteExtra->draw( -cos(ra)*rec, -sin(ra)*rec, Sidekick_index, bmp);
	data->spriteShip->animate(x,y,64, space);
	*/
		
	Ship::animate(space);

	Sidekick_index = get_index(SidekickAngle);
	rec = (double)recoil/recoil_rate;
	rec *= rec * recoil_range;
	data->spriteExtra->animate( pos - rec*unit_vector(SidekickAngle), //  x - cos(angle+SidekickAngle)*rec, y - sin(angle+SidekickAngle)*rec
								Sidekick_index, space);

	return;
	}

void KorvianSidekick::calculate_hotspots() {

}

SidekickMissile::SidekickMissile(double oangle, double ov, int odamage, double orange,
	  int oarmour, Ship *oship, SpaceSprite *osprite) :
  Missile(oship, Vector2(0.0, 0.0), oangle, ov, odamage, orange, oarmour, oship,
    osprite)
{
//  x += cos(angle) * 30.0;
//  y += sin(angle) * 30.0;
  pos += 30.0 * unit_vector(angle);

  explosionSprite     = data->spriteWeaponExplosion;
  explosionFrameCount = 10;
  explosionFrameSize  = 50;
  /*add(new Animation(this, pos,
    data->spriteExtraExplosion, 0, 10, 30, DEPTH_EXPLOSIONS));*/
	debug_id = 1039;
}

void SidekickMissile::inflict_damage(SpaceObject* other) {
	if (other->mass > 0) 
		other->accelerate (this, this->angle, (WEAPON_MASS / other->mass) * abs(this->get_vel()), MAX_SPEED);
	Missile::inflict_damage(other);
}



REGISTER_SHIP(KorvianSidekick)