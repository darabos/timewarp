#include "../ship.h"
REGISTER_FILE

#include "../melee/mmain.h"

#define OWADISABLE_SPEC 0x36

class OwaVoyager : public Ship {

  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;
  double       weaponHoming;
  int          weaponArming;
  double       weaponLaunch;

  double       shrapnelRange;
  double       shrapnelVelocity;
  int          shrapnelDamage;
  int          shrapnelArmour;
  double       shrapnelHoming;
  int          shrapnelArming;
  int          shrapnelHotspot;

  double       specialRange;
  double       specialVelocity;
  int          specialArmour;
  int          specialFrames;

public:

  OwaVoyager(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  virtual int activate_weapon();
  virtual int activate_special();
};

class OwaMissile : public HomingMissile {

  int          missileArming;
  int          missileActive;
  double       launch;

  double       shrapnelRange;
  double       shrapnelTurn;
  double       shrapnelVelocity;
  int          shrapnelDamage;
  int          shrapnelArmour;
  double       shrapnelHoming;
  double       shrapnelArming;
  int          shrapnelHotspot;
  SpaceSprite  *shrapnelSprite;

  public:
  OwaMissile(Vector2 opos,double oangle, double ov,
    int odamage, double orange, int oarmour,double otrate , int oarming,
    double olaunch, Ship *oship, SpaceSprite *osprite, double sv, int sdamage,
    double srange, int sarmour, double strate, int sarming,
    SpaceSprite *ssprite,int shotspot);
  virtual void calculate();
};

class OwaShrapnel : public HomingMissile {

  int    missileArming;
  int    missileActive;
  int    hotspot_frame;
  int    hotspot_rate;
  double turnrate;
  public:
OwaShrapnel(Vector2 op,double oangle, double ov,
  int odamage, double orange, int oarmour, double otrate, int oarming,
  OwaMissile *opos, SpaceSprite *osprite, int ohotspot);
  virtual void calculate();
};

class OwaSpecial : public Missile {
  int disableFrames;

  public:
  OwaSpecial(Vector2 opos, double oangle, double ov,
    int oframes, double orange, int oarmour, Ship *oship,
    SpaceSprite *osprite);

  void inflict_damage(SpaceObject *other);
  void animateExplosion();
};

class OwaDisable : public SpaceObject {
  int   disableframe;
  int   disableframe_count;
  int   frame_step;
  int   frame_size;
  int   frame_count;
  int   lowerindex;

  public:
  OwaDisable(Ship *otarget, OwaSpecial *ocreator, SpaceSprite *osprite, int ofcount,
    int ofsize, int disableFrames, int lowerFrames);

	virtual void target_died();
  virtual void calculate();
  void reset_time();
  Ship *return_disable();
};

OwaVoyager::OwaVoyager(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)
{
  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  weaponHoming   = scale_turning(get_config_float("Weapon","Homing",0));
  weaponArming   = scale_frames(get_config_int("Weapon","Arming",0));
  weaponLaunch   = scale_range(get_config_float("Weapon", "Launch",0));

  shrapnelRange    = scale_range(get_config_float("Shrapnel", "Range", 0));
  shrapnelVelocity = scale_velocity(get_config_float("Shrapnel", "Velocity", 0));
  shrapnelDamage   = get_config_int("Shrapnel", "Damage", 0);
  shrapnelArmour   = get_config_int("Shrapnel", "Armour", 0);
  shrapnelHoming   = scale_turning(get_config_float("Shrapnel","Homing",0));
  shrapnelArming   = scale_frames(get_config_int("Shrapnel","Arming",0));
  shrapnelHotspot  = scale_frames(get_config_int("Shrapnel","Hotspot",0));

  specialRange    = scale_range(get_config_float("Special", "Range", 0));
  specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
  specialArmour   = get_config_int("Special", "Armour", 0);
  specialFrames   = scale_frames(get_config_int("Special", "Frames", 0));
}

int OwaVoyager::activate_weapon()
{
  add(new OwaMissile(Vector2(0,size.y/2),angle,weaponVelocity,
    weaponDamage,weaponRange,weaponArmour,weaponHoming,weaponArming,
    weaponLaunch,this, data->spriteWeapon, shrapnelVelocity,shrapnelDamage,
    shrapnelRange,shrapnelArmour,shrapnelHoming,shrapnelArming,data->spriteExtra,
    shrapnelHotspot));
  return(TRUE);
}

int OwaVoyager::activate_special()
{
  add( new OwaSpecial(Vector2(0.0, -(size.y / 2.0)), angle-PI,
    specialVelocity, specialFrames,specialRange, specialArmour, this,
    data->spriteSpecial));
  return(TRUE);
}

OwaMissile::OwaMissile(Vector2 opos,double oangle, double ov,
    int odamage, double orange, int oarmour,double otrate , int oarming,
    double olaunch, Ship *oship, SpaceSprite *osprite, double sv, int sdamage,
    double srange, int sarmour, double strate, int sarming,
    SpaceSprite *ssprite,int shotspot) : HomingMissile(oship, opos, oangle, ov,
    odamage, orange, oarmour, otrate, oship, osprite, oship->target)

{
  missileArming = oarming;
  missileActive = FALSE;
  launch = olaunch;
  shrapnelRange    = srange;
  shrapnelVelocity = sv;
  shrapnelDamage   = sdamage;
  shrapnelArmour   = sarmour;
  shrapnelHoming   = strate;
  shrapnelArming   = sarming;
  shrapnelSprite   = ssprite;
  shrapnelHotspot  = shotspot;
}

void OwaMissile::calculate() {
	HomingMissile::calculate();
	if (!missileActive) {
		missileArming -= frame_time;
		if (missileArming <= 0)
			missileActive = TRUE;
		} 
	else if (target) if ((distance(target) < launch) || (d >= range)) {
		double deg = 35 * ANGLE_RATIO;
		for (int i=0;i <8;i++)  {
			OwaShrapnel *shrap;
			shrap = (new OwaShrapnel(0,angle+deg ,shrapnelVelocity,
					shrapnelDamage, shrapnelRange, shrapnelArmour, shrapnelHoming,
					iround(shrapnelArming), this, shrapnelSprite, shrapnelHotspot));
			deg -= 10 * ANGLE_RATIO;
			add(shrap);
			}
		play_sound(data->sampleWeapon[1]);
		state = 0;
		}
	return;
	}

OwaShrapnel::OwaShrapnel(Vector2 op,double oangle, double ov,
    int odamage, double orange, int oarmour, double otrate, int oarming,
    OwaMissile *opos, SpaceSprite *osprite, int ohotspot) :
    HomingMissile(opos, op, oangle, ov, odamage, orange, oarmour, 0,
    opos, osprite, opos->target)
{
   missileArming = oarming;
   missileActive = FALSE;
   turnrate = otrate;
   hotspot_rate = ohotspot;
   hotspot_frame = 0;
}

void OwaShrapnel::calculate()
{
  if (!missileActive) {
    missileArming -= frame_time;
    if (missileArming <= 0) {
      missileActive = TRUE;
      turn_rate = turnrate; }}
  if (hotspot_frame <= 0) {
    add(new Animation(this, 
      normal_pos() - (unit_vector(angle ) * size.x / 2.5),
      meleedata.hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, LAYER_HOTSPOTS));
    hotspot_frame += hotspot_rate;
  }
  if (hotspot_frame > 0) hotspot_frame -= frame_time;
  HomingMissile::calculate();
  return;
}

OwaDisable::OwaDisable(Ship *otarget, OwaSpecial *ocreator, SpaceSprite *osprite,
  int ofcount, int ofsize, int disableFrames,int lowerFrames) :
  SpaceObject(ocreator, otarget->normal_pos(), 0.0, osprite),
  disableframe(lowerFrames),
  disableframe_count(disableFrames),
  frame_step(0),
  frame_size(ofsize),
  frame_count(ofcount),
  lowerindex(lowerFrames)
{
	target = otarget;
	id = OWADISABLE_SPEC;
	sprite_index = lowerFrames;
	collide_flag_anyone = 0;
	layer = LAYER_SPECIAL;
}
void OwaDisable::target_died() {
	state = 0;
	return;
	}
void OwaDisable::calculate() {

	if (!(ship && ship->exists()))
	{
		state = 0;
		return;
	}

	frame_step+= frame_time;
	while (frame_step >= frame_size) {
		frame_step -= frame_size;
		sprite_index++;
		if(sprite_index == (lowerindex + frame_count))
			sprite_index = lowerindex;
		}
	if (!lowerindex) {
		pos = target->normal_pos();
		((Ship*)target)->nextkeys &= ~(keyflag::left | keyflag::right | keyflag::thrust);
		}
	else {
		pos = target->normal_pos();
		}
	disableframe += frame_time;
	if (disableframe >= disableframe_count) state = 0;
	SpaceObject::calculate();
	return;
	}

Ship *OwaDisable::return_disable()
{
  return (ship);
}

void OwaDisable::reset_time()
{
  disableframe = 0;
}

OwaSpecial::OwaSpecial(Vector2 opos, double oangle,
  double ov, int oframes, double orange, int oarmour, Ship *oship,
  SpaceSprite *osprite) : Missile (oship, opos, oangle, ov, 0, orange, oarmour, oship, osprite),
  disableFrames(oframes)
{}

void OwaSpecial::animateExplosion() {}

void OwaSpecial::inflict_damage(SpaceObject *other) {
          if (other->isShip()) {
            play_sound(data->sampleExtra[0]);
            SpaceObject *o = NULL;
            Query a;
            int found = FALSE;
            for (a.begin(this, ALL_LAYERS ,distance(other)+ 10);
              a.current; a.next()) {
		o = a.currento;
		if ((o->getID()) == OWADISABLE_SPEC)
		  if ((((OwaDisable *)o)->return_disable()) == other) {
			((OwaDisable *)o)->reset_time();
                        found = TRUE;
	  }
          if (!found) {
          add(new OwaDisable(
            (Ship *)(other), this, data->spriteExtraExplosion, 32, 40, disableFrames, 0));
          add(new OwaDisable(
            (Ship *)(other), this, data->spriteExtraExplosion, 32, 40, disableFrames, 32));
          }  }   }
        state = 0;
	return;
}




REGISTER_SHIP(OwaVoyager)
