#include "../ship.h"
REGISTER_FILE
#include "../other/objanim.h"

class YushSpear;

/*
 * created by: cyhawk@sch.bme.hu and forevian@freemail.hu
 */
class YushRanger : public Ship {
// the ship

  double        weaponRange;
  double        weaponVelocity;
  int           weaponDamage;
  int           weaponArmour;
  double        weaponControl;
  YushSpear*  weaponObject;

  public:
  YushRanger( Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code );

  virtual int activate_weapon();
  virtual int activate_special();
  virtual void calculate_turn_left();
  virtual void calculate_turn_right();
  virtual void calculate();
};

class YushSpear : public Missile {
  SpaceLocation* latched;
  double         rel_angle;
  double         old_angle;
  double         rel_dist;
  double         control;

  public:
  bool           released;

  YushSpear( SpaceLocation *creator, Vector2 opos, double oangle,
    double ov, int odamage, double orange, int oarmour, double ocontrol,
    SpaceLocation *oposloc, SpaceSprite *osprite, double relativity = game->shot_relativity );

  virtual void release();
  virtual void calculate();
  virtual void inflict_damage( SpaceObject* other );
  virtual int handle_damage( SpaceLocation* other, double normal, double direct );
};

YushRanger::YushRanger( Vector2 opos, double shipAngle,
  ShipData *shipData, unsigned int code ):
  Ship( opos, shipAngle, shipData, code )
{
  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  weaponControl  = scale_velocity(get_config_float("Weapon", "Control", 0));
  weaponObject   = NULL;
}

int YushRanger::activate_weapon(){
  if( weaponObject ) return FALSE;
  game->add( weaponObject = new YushSpear( this, Vector2(0, 0.5*get_size().y), angle, weaponVelocity,
    weaponDamage, weaponRange, weaponArmour, weaponControl, this, data->spriteWeapon ));
  return TRUE;
}

int YushRanger::activate_special(){
  double alpha = atan3(vel.y, vel.x);
  alpha = normalize(alpha, PI2);
  double v = vel.length();		//sqrt( vx*vx + vy*vy );
  if( v*2 > speed_max ) v = v>speed_max?v:speed_max;
  else v *= 2;
// collide to left-right directed wall
//  vx = v * cos( PI + 2 * angle - alpha );
//  vy = v * sin( PI + 2 * angle - alpha );
// collide to front-back directed wall

//  vx = v * cos( 2 * angle - alpha );
//  vy = v * sin( 2 * angle - alpha );
  vel = v * unit_vector( 2 * angle - alpha );
  game->add( new ObjectAnimation( this, pos, Vector2(0, 0), angle, data->spriteSpecial,
    0, 4, time_ratio, LAYER_HOTSPOTS ));
  special_sample = random() % 2;
  return TRUE;
}

void YushRanger::calculate_turn_left(){
  if( !fire_weapon ) Ship::calculate_turn_left();
}

void YushRanger::calculate_turn_right(){
  if( !fire_weapon ) Ship::calculate_turn_right();
}

void YushRanger::calculate(){
  Ship::calculate();
  if( weaponObject ){
    if( !weaponObject->exists() || weaponObject->released ){
      weaponObject = NULL;
    }else if( !fire_weapon ){
      weaponObject->release();
      weaponObject = NULL;
    }
  }
}

YushSpear::YushSpear( SpaceLocation *creator, Vector2 opos, double oangle, 
	double ov, int odamage, double orange, int oarmour, double ocontrol,
	SpaceLocation *oposloc, SpaceSprite *osprite, double relativity):
Missile( creator, opos, oangle, ov, odamage, orange, oarmour, oposloc, osprite, relativity ),
released( false ), control( ocontrol ), latched( NULL ){
}

void YushSpear::calculate(){
  if( latched ){
    if( !latched->exists() ){
      latched = NULL;
      state = 0;
      return;
    }
//    double tx = cos( (latched->get_angle() + rel_angle)  );
//    double ty = sin( (latched->get_angle() + rel_angle)  );
	Vector2 t = unit_vector( latched->get_angle() + rel_angle );
//    x = latched->normal_x() + rel_dist * tx;
//    y = latched->normal_y() + rel_dist * ty;
	pos = latched->normal_pos() + rel_dist * t;
//    vx = latched->vx;
//    vy = latched->vy;
	vel = latched->vel;
    angle = old_angle + latched->get_angle();
    sprite_index = get_index(angle);
    return;
  }
  Missile::calculate();
  if( released ) return;
  if( !ship ){ release(); return; }
  if( !ship->exists() ){ ship = NULL; release(); return; }

//  double tx = cos( angle  );
//  double ty = sin( angle  );
  Vector2 t = unit_vector( -angle );
  if( ship->turn_left ){
//    x += control * frame_time * ty;
//    y -= control * frame_time * tx;
	pos -= control * frame_time * t;

  }
  if( ship->turn_right ){
//    x -= control * frame_time * ty;
//    y += control * frame_time * tx;
	pos += control * frame_time * t;

  }
}

void YushSpear::release(){
  released = true;
}

void YushSpear::inflict_damage( SpaceObject* other ){
  if( latched || other->isShot() ) return;
  if( !other->isAsteroid() ) Shot::inflict_damage( other );
  state = 1;                   // don't want to die on contact
  attributes &= ~ATTRIB_SHOT;  // not a shot anymore
  released = true;
  latched = other;
  rel_angle = other->trajectory_angle( this ) - other->get_angle();
  old_angle = angle - other->get_angle();
  rel_dist = distance( other );
  change_owner( other );       // are now part of the other ship
  play_sound2( data->sampleWeapon[1 + random() % 2] );
}

int YushSpear::handle_damage( SpaceLocation* other, double normal, double direct ){
  if( latched ){
    latched->handle_damage( other, normal, direct );
// commenting out the following two lines makes the spears vulnerable to damage
    normal = direct = 0;
  }
  return Shot::handle_damage( other, normal, direct );
}

REGISTER_SHIP(YushRanger)
