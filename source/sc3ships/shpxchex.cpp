#include "../ship.h"
REGISTER_FILE

#include "../frame.h"

#define XCHAGGERDISABLE_SPEC 0x37

class XchaggerExclave : public Ship {
  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;

  double       specialRange;
  double       specialVelocity;
  int          specialArmour;
  int          specialFrames;

  public:
  XchaggerExclave(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  protected:
  virtual int activate_weapon();
  virtual int activate_special();
};
class XchaggerDisable : public SpaceObject {
  int   disableframe;
  int   disableframe_count;
  int   frame_step;
  int   frame_size;
  int   frame_count;
  int   lowerindex;

  public:
  XchaggerDisable(SpaceObject *creator, Ship *oship, SpaceSprite *osprite, int ofcount,
    int ofsize, int disableFrames, int lowerFrames);

  virtual void calculate();
  void reset_time();
  Ship *return_disable();
};

class XchaggerSpecial : public Missile {
  int disableFrames;
  SpaceSprite *disableSprite;
  SAMPLE *disableSound;

  public:
  XchaggerSpecial(Vector2 opos, double oangle, double ov,
    int oframes, double orange, int oarmour, Ship *oship,
    SpaceSprite *osprite);

  void inflict_damage(SpaceObject *other);
  void animateExplosion();
};
XchaggerExclave::XchaggerExclave(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos,  shipAngle, shipData, code)
{

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);

  specialRange    = scale_range(get_config_float("Special", "Range", 0));
  specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
  specialArmour   = get_config_int("Special", "Armour", 0);
  specialFrames   = scale_frames(get_config_int("Special", "Frames", 0));
}

int XchaggerExclave::activate_weapon()
{
  add(new Missile(this, Vector2(-20.0, 28),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon));
  add(new Missile(this, Vector2(20.0, 28),
    angle, weaponVelocity, weaponDamage, weaponRange, weaponArmour,
    this, data->spriteWeapon));
  return(TRUE);
}

int XchaggerExclave::activate_special()
{
  add( new XchaggerSpecial(Vector2(0.0, -(size.y / 2.0)), angle-PI,
    specialVelocity, specialFrames,specialRange, specialArmour, this,
    data->spriteSpecial));
  return(TRUE);
}

XchaggerDisable::XchaggerDisable(SpaceObject *creator, Ship *oship, SpaceSprite *osprite,
  int ofcount, int ofsize, int disableFrames,int lowerFrames) :
  SpaceObject(creator, oship->normal_pos(), 0.0, osprite),
  disableframe(lowerFrames),
  disableframe_count(disableFrames),
  frame_step(0),
  frame_size(ofsize),
  frame_count(ofcount),
  lowerindex(lowerFrames)
{
  id = XCHAGGERDISABLE_SPEC;
  sprite_index = lowerFrames;
  collide_flag_anyone = 0;
  if (!lowerindex) layer = 5;
  target = oship;
}

void XchaggerDisable::calculate() {
	int keyleft,keyright;
	frame_step+= frame_time;
	while (frame_step >= frame_size) {
		frame_step -= frame_size;
		sprite_index++;
		if(sprite_index == (lowerindex + frame_count))
			sprite_index = lowerindex;
		}
	if(!(ship && ship->exists())) {
		ship = 0;		// should be here cause you return before the other call
		state = 0;
		return;
		}
	if(!(target && target->exists() && target->isShip())) {
		target = 0;
		state = 0;
		return;
		}

	Ship *t = (Ship*)target;

	if (!lowerindex) {
		pos = t->normal_pos();
		keyleft = t->nextkeys & (keyflag::left);
		keyright = t->nextkeys & (keyflag::right);
		t->nextkeys &= ~(keyflag::left | keyflag::right);
		if (keyleft)
			t->nextkeys |= (keyflag::right);
		if (keyright)
			t->nextkeys |= (keyflag::left);
        } 
	else {
        pos = t->normal_pos() + (frame_time * t->get_vel());
		SpaceObject::calculate();
        }
	disableframe += frame_time;
	if (disableframe >= disableframe_count) state = 0;
	return;
	}

Ship *XchaggerDisable::return_disable()
{
  return (ship);
}

void XchaggerDisable::reset_time()
{
  disableframe = 0;
}

XchaggerSpecial::XchaggerSpecial(Vector2 opos, double oangle,
  double ov, int oframes, double orange, int oarmour, Ship *oship,
  SpaceSprite *osprite) : Missile (oship, opos,  oangle, ov, 0, orange, oarmour, oship, osprite),
  disableFrames(oframes),
  disableSprite(oship->data->spriteExtra),
  disableSound(oship->data->sampleSpecial[1])
{
	isblockingweapons = false;
}

void XchaggerSpecial::animateExplosion() {}

void XchaggerSpecial::inflict_damage(SpaceObject *other)
{
	if (other->isShip()) {
		play_sound(disableSound);
		SpaceObject *o = NULL;
		Query a;
		int found = FALSE;
		for (a.begin(this, ALL_LAYERS ,distance(other)+ 10);
		a.current; a.next()) {
			o = a.currento;
			if ((o->getID()) == XCHAGGERDISABLE_SPEC)
				if ((((XchaggerDisable *)o)->return_disable()) == other) {
					((XchaggerDisable *)o)->reset_time();
					found = TRUE;
				}
				if (!found) {
					add(new XchaggerDisable( this, 
						(Ship *)(other), disableSprite, 32, 40, disableFrames, 0));
					//game->addItem(new XchaggerDisable(
					//  (Ship *)(other), disableSprite, 32, 40, disableFrames, 32));
	}  }   }
	state = 0;
	return;
}



REGISTER_SHIP(XchaggerExclave)
