#include "../ship.h"
//#include "../melee/mview.h"
REGISTER_FILE

class GarashTyrant : public Ship {
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;
  int          weaponChargeIncrement;
  double       weaponRange;
	int					 weaponChargeSpeed;
	double       weaponChargingDistance;
	int          weaponChargingDistanceDamage;
  SpaceObject *weaponObject;

  double       specialDamage;
  double       specialRange;
  int          specialRepulse;
	int					 specialStunFrames;
	int          panelDelay;
	int					 panelFrame;
	int					 origPanelDelay;

	bool         repulse;

	int					 weaponMinTimeLimit;
	int					 weaponMaxTimeLimit;

  public:
  GarashTyrant(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);
	friend class GarashShot;

	virtual void calculate_fire_weapon();

  virtual int  activate_weapon();
  virtual int  activate_special();

  virtual void calculate();
	virtual void stun_ship(SpaceObject *other);

};

class GarashShot : public Shot {
  double v;
  int  frame;
  int  frame_step;
  int  frame_size;
  int  frame_count;
  int  charge_frame;
  int  charge_phase;
  int  released;
	bool ship_hit;
	bool done_once;

	int		 Timer;
	int		 MinTL;
	int		 MaxTL;
	int		 Charge;

	Vector2		StartC;
	double CD;
	int    CDD;



  public:
  GarashShot(Vector2 opos, double oangle, double ov, int odamage, double orange,
    int oarmour, Ship *oship, SpaceSprite *osprite, int ofsize, int ofcount, 
		int oCI, int oMinTL, int oMaxTL, double oChargeDistance, int oChargingDistanceDamage);

  virtual void calculate();
  virtual void animateExplosion();
  virtual void inflict_damage(SpaceObject *other);
  virtual int handle_damage(SpaceLocation *source, double normal, double direct);
	virtual int  canCollide(SpaceLocation *other);
};

class GarashRepulsarStun: public SpaceObject {
  Ship *ship;
  int   stunframe;
  int   stunframe_count;
  int   frame_step;
  int   frame_size;
  int   frame_count;

  public:
  GarashRepulsarStun(Ship *oship, SpaceSprite *osprite, int ofcount,
    int ofsize, int disableFrames);

  virtual void calculate();
};


GarashTyrant::GarashTyrant(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)
{

	weaponDamage					 = get_config_int("Weapon", "Damage", 8);
	weaponArmour					 = get_config_int("Weapon", "Armour", 8);
	weaponChargeIncrement	 = get_config_int("Weapon", "ChargeIncrement", 3);
	weaponVelocity				 = scale_velocity(get_config_float("Weapon", "Velocity", 200));
	weaponRange						 = scale_range(get_config_float("Weapon", "Range", 54));
	weaponChargeSpeed      = scale_frames(get_config_int("Weapon", "ChargeSpeed",2));
	weaponMinTimeLimit		 = scale_frames(get_config_int("Weapon","MinTimeLimit",152));
	weaponMaxTimeLimit		 = scale_frames(get_config_int("Weapon","MaxTimeLimit",430));
	weaponChargingDistance = scale_range(get_config_float("Weapon", "ChargingDistance", 9));
	weaponChargingDistanceDamage = get_config_int("Weapon", "ChargingDistanceDamage", 0);

	specialDamage			= get_config_int("Special", "Damage", 4);
	specialRange			= scale_range(get_config_int("Special", "Range", 9));
	specialRepulse		= get_config_int("Special","Repulse",80);
	specialStunFrames = scale_frames(get_config_int("Special","StunFrames",24));
	repulse						= FALSE;

	weaponObject			= NULL;
	origPanelDelay		= scale_frames(get_config_int("Extra","PanelDelay",0));
	panelDelay        = origPanelDelay;
	panelFrame				= 0;


}

void GarashTyrant::calculate()
{

if (!fire_weapon  &&
		!fire_special &&
		!turn_left    &&
		!turn_right   &&
		!thrust)
		{
		panelDelay -= frame_time;
		if (panelDelay==0)
			{
			BITMAP *b=spritePanel->get_bitmap(1);
			switch (panelFrame) {
				case 0:
					ship->spritePanel->overlay(1, 1, b);
					break;
				case 1:
					ship->spritePanel->overlay(1, 2, b);
					break;
				case 2:
					ship->spritePanel->overlay(1, 5, b);
					break;
				case 3:
					ship->spritePanel->overlay(1, 6, b);
					break;
				}
		  blit(b,spritePanel->get_bitmap(1), 0, 0, 0, 70, 55, 30);
			ship->update_panel = TRUE;
			
			panelFrame++;
			if (panelFrame > 3)
				panelFrame = 0;
			panelDelay = origPanelDelay;
			}
		}

if (repulse)
	{

		Query q;
		for (q.begin(this, OBJECT_LAYERS, specialRange); q.currento; q.next()) {

			stun_ship(q.currento);

			if (q.currento->mass) 
				q.currento->accelerate (q.currento, trajectory_angle(q.currento), specialRepulse / q.currento->mass, MAX_SPEED);
			damage(q.currento, 0, specialDamage);




			}
		add(new FixedAnimation(this,this, data->spriteSpecial, 0, 7, 70, LAYER_EXPLOSIONS));

		damage(this, 0);

		repulse=false;

	}
	

/**** ****/
  Ship::calculate();
  if (weaponObject) {
    if (!weaponObject->exists()) weaponObject = NULL;
    if (!fire_weapon)  weaponObject = NULL;
    }
}

void GarashTyrant::stun_ship(SpaceObject *other){
	if (other->isShip()) {
		add(new GarashRepulsarStun(
		(Ship *) (other->ship ), data->spriteExtra, 
		10, 31, specialStunFrames) );
		}
	}

void GarashTyrant::calculate_fire_weapon()
	{
	weapon_low = FALSE;

	if (fire_weapon) {
		if (batt < weapon_drain) {
			weapon_low = true;
			return;
			}

		if (weapon_recharge > 0)
				return;

		if (!activate_weapon())
				return;

		batt -= weapon_drain;
		if (recharge_amount > 1) recharge_step = recharge_rate;
		weapon_recharge += weapon_rate;
		}
	return;

	}


int GarashTyrant::activate_weapon()
{
  if(weaponObject)
    return(FALSE);

  add(weaponObject = new GarashShot(
    Vector2(0.0,size.y/2.60), angle, weaponVelocity, weaponDamage, weaponRange,
    weaponArmour, this, data->spriteWeapon, 10, weaponChargeSpeed,
		weaponChargeIncrement,weaponMinTimeLimit,weaponMaxTimeLimit,
		weaponChargingDistance, weaponChargingDistanceDamage));
  return(TRUE);
}

int GarashTyrant::activate_special() {

	repulse = true;

  return(TRUE);
}

GarashRepulsarStun::GarashRepulsarStun(Ship *oship,
  SpaceSprite *osprite, int ofcount, int ofsize, int stunFrames) 
	:
  SpaceObject(oship, oship->normal_pos(), 0.0, osprite),
  ship(oship), stunframe(0), stunframe_count(50),
  frame_step(0), frame_size(ofsize), frame_count(ofcount)
{
	collide_flag_anyone = 0;
	layer = LAYER_EXPLOSIONS;
}

void GarashRepulsarStun::calculate() {
	frame_step+= frame_time;
	while (frame_step >= frame_size) {
		frame_step -= frame_size;
		sprite_index++;
		if(sprite_index == frame_count)
			sprite_index = 0;
		}
	if(!ship->exists()) {
		state = 0;
		return;
		}

  ship->nextkeys &= ~(keyflag::left | keyflag::right | keyflag::thrust | keyflag::special);

  stunframe += frame_time;
  if (stunframe >= stunframe_count) state = 0;
	SpaceObject::calculate();
	}



GarashShot::GarashShot(Vector2 opos, double oangle, double ov,
  int odamage, double orange, int oarmour, Ship *oship, SpaceSprite *osprite,
  int ofcount, int ofsize, int oCI, int oMinTL, int oMaxTL,
	double oChargingDistance, int oChargingDistanceDamage) :
  Shot(oship, opos, oangle, ov, odamage, orange, oarmour, oship, osprite),
  v(ov),
  frame(0),
  frame_step(0),
  frame_size(ofsize),
  frame_count(ofcount),
  charge_frame(0),
  charge_phase(0),
  released(FALSE),
	ship_hit(FALSE),
	done_once(FALSE),
	StartC(opos),
	CD(oChargingDistance),
	CDD(oChargingDistanceDamage)
{  
	  vel = ship->get_vel();

	Timer  = 0;
	MinTL  = oMinTL;
	MaxTL  = oMaxTL;
	Charge = oCI;

  collide_flag_anyone   = (ALL_LAYERS);
	collide_flag_sameship = 0;

  explosionSprite     = data->spriteWeaponExplosion;
  explosionFrameCount = 7;
  explosionFrameSize  = 35;

}

void GarashShot::calculate() {

	if(released) Shot::calculate();
	else SpaceObject::calculate();

	if (!exists()) return;

	if (!done_once)
		{
			play_sound(data->sampleExtra[0]); //charge
			done_once=TRUE;
		}


	frame_step += frame_time;
	if(frame_step >= frame_size) { //1
		frame_step -= frame_size;
		frame++;
		if(frame == frame_count) { //2
			frame = 0;

			if(!released)  { //3
				charge_frame++;
				if(charge_frame == 5) { //4
			    play_sound(data->sampleExtra[0]); //charge
					charge_frame = 0;
					charge_phase++;
					damage_factor += Charge;
					armour += Charge;
					range  += scale_range(Charge);
					} //4
				} //3
			} //2
		} //1

	if(!released) {

		if (!ship || !ship->fire_weapon || Timer > MaxTL) {
			vel = unit_vector(angle) * v;
			released = TRUE;
			if (Timer > MinTL)
 				play_sound(data->sampleWeapon[0]); //shoot
			}
		else {
			angle = ship->get_angle();
			pos = ship->normal_pos() + unit_vector(angle) * ship->size.y/2.6;
			vel = ship->get_vel();
		}

		StartC = pos;

		if (ship)	
			{ int sprite_phase = charge_phase;
		  if (sprite_phase > 2)	sprite_phase = 2;
			sprite_index = (get_index(ship->get_angle()) +
										 (sprite_phase * 64)); 
			}
		 else released = TRUE;



			Timer += frame_time;
		} else if (Timer < MinTL) {
		    released=TRUE;
				sound.stop(data->sampleExtra[0]);
				play_sound2(data->sampleExtra[1]); //cancel if less than min time limit
				state=0;
				}


	return;
	}

void GarashShot::animateExplosion()
{
	return;
}

void GarashShot::inflict_damage(SpaceObject *other)
	{

	int Distance_Travelled = distance_from(StartC, other->normal_pos());
	int AddDamage = (Distance_Travelled / CD) * CDD;

	
	if (!ship_hit || !released)
	  damage (other ,damage_factor + AddDamage);

	if (!other->isShip() &&
		  !other->isPlanet())
			other->state=0;


	if (other->isShip()) {
		ship_hit=TRUE;
		if (other->mass > 20 || !released)	
			 state=0;
		}
	 else ship_hit=FALSE;


	if (other->isShip() && other->mass > 0) {
		animateExplosion();
		soundExplosion(); 
	}

		if (other->mass <= 0) return;

		if(other->isShip())
			add(new FixedAnimation(this, other,
				explosionSprite, 0, explosionFrameCount,
				explosionFrameSize, LAYER_EXPLOSIONS));

		else
			add(new Animation(this, pos,
				explosionSprite, 0, explosionFrameCount,
				explosionFrameSize, LAYER_EXPLOSIONS));
		return;
	}



int GarashShot::handle_damage(SpaceLocation *source, double normal, double direct)
{
	armour += damage_factor + direct ; // nullify damage
	if (((!ship_hit && !source->isShip()) ||
			(ship_hit && source->isShip())) && !released)
			play_sound(data->sampleExtra[2]);
	Shot::handle_damage(source, normal, direct);
	return 0;
}

int GarashShot::canCollide(SpaceLocation *other)
	{
		return(TRUE);
	}
REGISTER_SHIP(GarashTyrant)