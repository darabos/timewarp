#include "../ship.h"
REGISTER_FILE
#include "../melee/mview.h"
#include "../other/nullphas.h"

#define BIPOLE_STABILITY 0.4

class BipoleShip;

/*
 * created by: cyhawk@sch.bme.hu and forevian@freemail.hu
 */
class BipoleKatamaran : public Ship {
// abstract class that is instanciated at start and is coordinating movement afterwards

  double       specialIncreaseRate;
  double       specialDecreaseRate;
  double       specialDamage;
  int          specialColor;
  double       specialMinRange;
  double       specialMaxRange;

  double       extraBackThrust;        // the ships accelerate backwards with this ratio
  double       extraMaxSpin;           // a value attempting to regulate spinning

  double       range;
  double       residualDamage;

  double       od, oangle;

  BipoleShip*  left;
  BipoleShip*  right;

  public:
  BipoleKatamaran(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  virtual double  isInvisible() const; // this ship should not be percievable in any way
  virtual int  activate_weapon();      // fires the weapons of the ships
  virtual int  activate_special();     // increases range
  virtual void animate(Frame* space);  // does not appear
  virtual void calculate();            // coordinates ship movement and places laser beam
  virtual void calculate_thrust();     // accelerates both ships
  virtual void calculate_turn_left();  // accelerates right ship and deccelerates left
  virtual void calculate_turn_right(); // accelerates left ship and deccelerates right
  virtual void calculate_hotspots();   // does not leave hotspots
  virtual void materialize();          // remove itself as a target
  //virtual int  exists();               // returns if at least one ship exists
//note from orz : exists() cannot be overriden
  virtual void death();                // kill ships too

  virtual SpaceLocation *get_ship_phaser();  // returns a BipolePhaser
};

class BipoleShip : public Ship {
// one of the two sub-ships making up the Katamaran

  friend BipoleKatamaran;

  double       weaponRange;
  double       weaponVelocity;
  int          weaponDamage;
  int          weaponArmour;

  double       specialBrakes;

  protected:
  bool         shooting;
  Ship*        parent;

  public:
  BipoleShip(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code, Ship* oparent, SpaceSprite* osprite);

  virtual int activate_weapon();          // shoots with every second call
  virtual int activate_special();         // deccelerates
  virtual void calculate_fire_weapon();   // works if on our own
  virtual void calculate_fire_special();  // works if on our own
  virtual void calculate_thrust();        // works if on our own
  virtual void calculate_turn_left();     // works if on our own
  virtual void calculate_turn_right();    // works if on our own
//  virtual void calculate_hotspots();
  virtual int handle_damage( SpaceLocation* source, double normal, double direct );
  virtual void destroyed( SpaceLocation* source );
};

SpaceLocation* BipoleKatamaran::get_ship_phaser() {
  return new NullPhaser( this );
}

BipoleKatamaran::BipoleKatamaran(Vector2 opos, double shipAngle,
  ShipData *shipData, unsigned int code) :
  Ship(opos, shipAngle, shipData, code)
{
  collide_flag_anyone = 0;
  collide_flag_sameteam = 0;
  collide_flag_sameship = 0;

  specialIncreaseRate = get_config_float("Special", "IncreaseRate", 0);
  specialDecreaseRate = get_config_float("Special", "DecreaseRate", 0);
  specialMinRange = get_config_int("Special", "MinRange", 0);
  specialMaxRange = get_config_int("Special", "MaxRange", 0);
  specialDamage = get_config_float("Special", "Damage", 0);
  specialColor = get_config_int("Special", "Color", 0);

  extraBackThrust = get_config_float("Extra", "BackThrust", 0);
  extraMaxSpin = scale_turning(get_config_float("Extra", "MaxSpin", 0)) * ANGLE_RATIO;

  crew_max *= 2;
  batt_max *= 2;
//  weapon_drain *= 2;
  special_drain *= 2;

  range = specialMinRange;
  residualDamage = 0;
  od = range;
  oangle = angle;

  left = new BipoleShip( pos + (range/2)*unit_vector(angle-90),/*pos.x+(range/2)*sin( angle ),
    pos.y-(range/2)*cos( angle ),*/ angle, data, code, this, sprite );
  right = new BipoleShip(  pos + (range/2)*unit_vector(angle+90),/*pos.x-(range/2)*sin( angle ),
    pos.y+(range/2)*cos( angle ),*/ angle, data, code, this, data->spriteSpecial );
  left->change_owner( this );
  right->change_owner( this );
  game->add_target( left );
  game->add_target( right );
  left->shooting = true;
  id = 0; /* this is nothing */
}

double BipoleKatamaran::isInvisible() const{
  return 2;
}

int BipoleKatamaran::activate_weapon(){
  if( !left || !right ) return FALSE;
  weapon_sample = random(11);
  int left_w = left->activate_weapon();
  int right_w = right->activate_weapon();
  return left_w || right_w;
}

int BipoleKatamaran::activate_special(){
  if( !left || !right ) return FALSE;
  if( range >= specialMaxRange ){
    range = specialMaxRange;
    return FALSE;
  }
  range += specialIncreaseRate;
  return TRUE;
}

void BipoleKatamaran::animate(Frame* space){
  /* we do not show */
  return;
}

void BipoleKatamaran::calculate(){
  if( left ) if( !left->exists() ){
    left = NULL;
    if( right ){
      right->parent = NULL;
      BITMAP *bmp = spritePanel->get_bitmap(0);
      blit( spritePanel->get_bitmap(8), bmp, 0, 0, 0, 0, 63, 99);
      update_panel = TRUE;
    }
  }
  if( right ) if( !right->exists() ){
    right = NULL;
    if( left ){
      left->parent = NULL;
      BITMAP *bmp = spritePanel->get_bitmap(0);
      blit( spritePanel->get_bitmap(7), bmp, 0, 0, 0, 0, 63, 99);
      update_panel = TRUE;
    }
  }
  if( !left && !right ){
    state = 0;
    game->ship_died( this, control->target );
//    Ship::calculate();
    return;
  }
  if( !right ){
    batt = left->batt;
    crew = left->crew;
	pos = left->normal_pos();
//    x = left->normal_x();
//    y = left->normal_y();
    angle = left->angle;
    Ship::calculate();
    return;
  }
  if( !left ){
    batt = right->batt;
    crew = right->crew;
//    x = right->normal_x();
//    y = right->normal_y();
	pos = right->normal_pos();
    angle = right->angle;
    Ship::calculate();
    return;
  }
  crew = left->crew + right->crew;
  batt = left->batt + right->batt;

  oangle = angle;
  angle = normalize( left->trajectory_angle( right ) - PI/2, PI2 );
  left->angle = angle;
  right->angle = angle;
  double d = left->distance( right );
//  x = left->normal_x() + 0.5*d*cos( angle+PI/2 );
//  y = left->normal_y() + 0.5*d*sin( angle+PI/2 );
  pos = left->normal_pos() + 0.5 * d * unit_vector(angle+PI/2);

  int o_recharge_step = recharge_step;
  recharge_step = frame_time;
  Ship::calculate();
  recharge_step = o_recharge_step;

//  left->nextkeys &= keyflag::thrust | keyflag::turn_left | keyflag::turn_right;
//  right->nextkeys &= keyflag::thrust | keyflag::turn_left | keyflag::turn_right;

  left->batt = batt / 2;
  right->batt = batt - left->batt;

  if( range > specialMinRange ) range -= specialDecreaseRate * frame_time;
  if( range < specialMinRange ) range = specialMinRange;
  if( range > specialMaxRange ) range = specialMaxRange;

  /* calculate frame_time indepentent damage */
  int dmg = (int)(specialDamage * frame_time);
  residualDamage += specialDamage * (double)frame_time - (int)(specialDamage * frame_time);
  if( residualDamage >= 1.0 ){
    dmg += 1;
    residualDamage -= 1.0;
  }
  /* place the laser */
  game->add(new Laser( this, angle + PI/2,
    pallete_color[specialColor], d, dmg, 1,
    left, Vector2(0,0), true ));
  /* it goes the other way too -- without the damage */
  game->add(new Laser( this, angle - PI/2,
    pallete_color[specialColor], d, 0, 1,
    right, Vector2(0,0), true ));

  /* place the laser */
  game->add(new Laser( this, angle + PI/2,
    pallete_color[specialColor], d, dmg, 1,
    left, Vector2(0,10), true ));
  /* it goes the other way too -- without the damage */
  game->add(new Laser( this, angle - PI/2,
    pallete_color[specialColor], d, 0, 1,
    right, Vector2(0,10), true ));

  /* we try to do something against spinning wildly */
  double dangle = angle - oangle;
  if( dangle < -PI ) dangle += PI2;
  if( dangle > PI ) dangle -= PI2;

  if(( !turn_left && !turn_right )||( dangle > extraMaxSpin*frame_time || dangle < -extraMaxSpin*frame_time )){
    left->accelerate(this, left->get_angle(), -dangle*(180/PI)*0.1*accel_rate * frame_time, 0.5*speed_max);
    right->accelerate(this, right->get_angle(), dangle*(180/PI)*0.1*accel_rate * frame_time, 0.5*speed_max);
  }
  
  /*double decay = 0.001;
  if (game->frame_number & 1) {
		left->vx = (decay * -frame_time + 1) * ( left->vx - right->vx ) + right->vx;
		left->vy = (decay * -frame_time + 1) * ( left->vy - right->vy ) + right->vy;
		}
  else {
		right->vx = (decay * -frame_time + 1) * ( right->vx - left->vx ) + left->vx;
		right->vy = (decay * -frame_time + 1) * ( right->vy - left->vy ) + left->vy;
  }*/
	
  // we try to stabilize the distance of the two ships around _range_ 
  d = left->distance( right ) - range;
  double dd = d - od;
  od = d;
  if( d > 0.0 ){
    if( dd < d ){
      left->accelerate( this, left->trajectory_angle( right ), BIPOLE_STABILITY*d*accel_rate*frame_time, speed_max );
      right->accelerate( this, right->trajectory_angle( left ), BIPOLE_STABILITY*d*accel_rate*frame_time, speed_max );
    }else if( dd > d ){
      left->accelerate( this, right->trajectory_angle( left ), BIPOLE_STABILITY*dd*accel_rate*frame_time, speed_max );
      right->accelerate( this, left->trajectory_angle( right ), BIPOLE_STABILITY*dd*accel_rate*frame_time, speed_max );
    }
  }else if( d < 0.0 ){
    if( dd < d ){
      left->accelerate( this, right->trajectory_angle( left ), BIPOLE_STABILITY*d*accel_rate*frame_time, MAX_SPEED );
      right->accelerate( this, left->trajectory_angle( right ), BIPOLE_STABILITY*d*accel_rate*frame_time, MAX_SPEED );
    }else if( dd > d ){
      left->accelerate( this, left->trajectory_angle( right ), BIPOLE_STABILITY*dd*accel_rate*frame_time, MAX_SPEED );
      right->accelerate( this, right->trajectory_angle( left ), BIPOLE_STABILITY*dd*accel_rate*frame_time, MAX_SPEED );
    }
  }
  
}

void BipoleKatamaran::calculate_thrust(){
  if( !left || !right ) return;
  if( left->thrust && !left->turn_left && !left->turn_right ){
    left->accelerate(this, angle, accel_rate * frame_time, speed_max);
  }
  if( right->thrust && !right->turn_left && !right->turn_right ){
    right->accelerate(this, angle, accel_rate * frame_time, speed_max);
  }
}
void BipoleKatamaran::calculate_turn_left(){
  if( !left || !right ) return;
  if( left->turn_left ){
    left->nextkeys &= ~keyflag::thrust;
    left->accelerate(this, left->get_angle(), -extraBackThrust * accel_rate * frame_time, speed_max);
  }
  if( right->turn_left ){
    right->nextkeys |= keyflag::thrust;
    right->accelerate(this, right->get_angle(), accel_rate * frame_time, speed_max);
  }
}
void BipoleKatamaran::calculate_turn_right(){
  if( !left || !right ) return;
  if( left->turn_right ){
    left->nextkeys |= keyflag::thrust;
    left->accelerate(this, left->get_angle(), accel_rate * frame_time, speed_max);
  }
  if( right->turn_right ){
    right->nextkeys &= ~keyflag::thrust;
    right->accelerate(this, right->get_angle(), -extraBackThrust * accel_rate * frame_time, speed_max);
  }
}

void BipoleKatamaran::calculate_hotspots(){
  return;
}

void BipoleKatamaran::materialize(){
  int i;
  for( i = 0; game->target[i] != this; i++ );
  game->num_targets--;
  game->target[i] = game->target[game->num_targets];
  left->control = control;
  right->control = control;
  game->add( left->get_ship_phaser() );
  game->add( right->get_ship_phaser() );
}

void BipoleKatamaran::death(){
  if( left ) left->die();
  if( right ) right->die();
}

BipoleShip::BipoleShip(Vector2 opos, double shipAngle,
  ShipData *shipData, unsigned int code, Ship* oparent, SpaceSprite* osprite) :
  Ship(opos, shipAngle, shipData, code)
{
  parent = oparent;
  sprite = osprite;

  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);

  double extraBackThrust = get_config_float("Extra", "BackThrust", 0);
  specialBrakes = accel_rate * extraBackThrust;

  shooting = false;
}

int BipoleShip::activate_weapon(){
  shooting = !shooting;
  if( !shooting ) return FALSE;  // if we would return TRUE single ship would fire slower

  weapon_sample = random(11);
  game->add( new Missile( this, Vector2(0.0,size.y/2.0), angle, weaponVelocity, weaponDamage,
    weaponRange, weaponArmour, this, data->spriteWeapon ));

  return TRUE;
}

int BipoleShip::activate_special(){
  accelerate_gravwhip( this, angle, -specialBrakes * frame_time, speed_max );

  return TRUE;
}

void BipoleShip::calculate_fire_weapon(){
  if( !parent ) Ship::calculate_fire_weapon();
}
void BipoleShip::calculate_fire_special(){
  if( !parent ) Ship::calculate_fire_special();
}
void BipoleShip::calculate_thrust(){
  if( !parent ) Ship::calculate_thrust();
}
void BipoleShip::calculate_turn_left(){
  if( !parent ) Ship::calculate_turn_left();
}
void BipoleShip::calculate_turn_right(){
  if( !parent ) Ship::calculate_turn_right();
}

/*void BipoleShip::calculate_hotspots() {
  if( thrust &&( hotspot_frame <= 0 )){
    game->addItem(new Animation( this, 
      normal_x() - (cos(angle ) * w / 2.5),
      normal_y() - (sin(angle ) * h / 2.5),
      game->hotspotSprite, 0, HOTSPOT_FRAMES, time_ratio, LAYER_HOTSPOTS));
    hotspot_frame += hotspot_rate;
  }
  if (hotspot_frame > 0) hotspot_frame -= frame_time;
}*/

int BipoleShip::handle_damage(SpaceLocation *source, double normal, double direct) {
	int s = exists();
	int i = Ship::handle_damage(source, normal, direct);
	if (s && !exists()) destroyed( source );
	return i;
	}

void BipoleShip::destroyed( SpaceLocation* source ){
	play_sound((SAMPLE *)(melee[MELEE_BOOMSHIP].dat));
	game->add(new Animation(this, pos,
    game->kaboomSprite, 0, KABOOM_FRAMES, time_ratio, LAYER_EXPLOSIONS));

// we do not report ship_died events -- the parent does
//  game->ship_died(this, source);
}

REGISTER_SHIP(BipoleKatamaran)
