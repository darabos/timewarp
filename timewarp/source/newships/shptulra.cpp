#include "../ship.h"
REGISTER_FILE
#include "../melee/mview.h"

#define TULKON_DEVICE_MIN_DIST 26
#define TULKON_DEVICE_MAX_DIST 50
#define TULKON_DEVICE_PULL 2
#define TULKON_BOMB_DROP_DIST 16

class TulkonDevice;

/*
 * created by: cyhawk@sch.bme.hu and forevian@freemail.hu
 */
class TulkonRam : public Ship {
// the body of the ship

  double        weaponForce;
  int           weaponDamage;

  int           specialDamage;
  int           specialArmour;
  double        specialMass;
  double        specialDRange;
  double        specialSRange;

  TulkonDevice *ram;

  SpaceObject** bombs;
  int           numBombs;
  int           maxBombs;

  public:
  TulkonRam( Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code );

  virtual int activate_weapon();         // furthers the ramming device
  virtual int activate_special();        // ejects a mine
  virtual void calculate_fire_weapon();  // releases the ramming device when needed
  virtual void calculate_fire_special(); // sound tweak
  virtual void calculate();              // keeps bomb registry up to date
  virtual void calculate_hotspots();     // places two streams of blue hotspots
  virtual int handle_fuel_sap( SpaceLocation* source, double normal );
  virtual void materialize();            // places the ramming device into game space
};

class TulkonDevice : public SpaceObject {
// the ramming device

//  double px, py;
//  double pvx, pvy;
  Vector2 P, PV;

  double force;

  void ram( bool mode );

  public:
  TulkonDevice( Ship* creator, SpaceSprite* osprite, double odist, int odamage,
    double oforce );

  double dist;                                        // distance of ship and device
  bool   ramming;                                     // if we are ramming this frame

  virtual void calculate();                           // follows ship movement
  virtual int canCollide( SpaceLocation* other );     // prevents collision with ship
  virtual void collide( SpaceObject* other );         // forwards kinetic energy to ship
  virtual void inflict_damage( SpaceObject* other );  // rams on contact and damages
  virtual int handle_damage( SpaceLocation* other, double normal, double direct ); // forwards directDamage and play sfx
};

class TulkonBomb : public AnimatedShot {
// bombs

  friend TulkonDevice;

  double drange;
  double srange;
  bool   exploding;

  protected:
  bool   rammed;

  public:
  TulkonBomb( SpaceLocation* creator, Vector2 opos, int odamage, double odrange,
    double osrange, double oarmour, double omass, SpaceSprite* osprite,
    double orelativity = game->shot_relativity );

  virtual void calculate();                            // checks sensory range for heat
  virtual void inflict_damage( SpaceObject* other );   // does not inflict damage
  virtual void collide( SpaceObject* other );          // if rammed flies straight
  virtual int handle_damage( SpaceLocation* source, double normal, double direct );  // if destroyed explodes
  virtual void soundExplosion();                       // Forevian's sound needs to be laud
};

TulkonRam::TulkonRam( Vector2 opos, double shipAngle,
  ShipData *shipData, unsigned int code ):
  Ship( opos, shipAngle, shipData, code )
{
  weaponForce   = get_config_float( "Weapon", "Force", 0 );
  weaponDamage  = get_config_int( "Weapon", "Damage", 0 );

  specialDamage = get_config_int( "Special", "Damage", 0 );
  specialArmour = get_config_int( "Special", "Armour", 0 );
  specialMass   = get_config_float( "Special", "Mass", 0 );
  specialDRange = scale_range( get_config_float( "Special", "DamageRange", 0 ));
  specialSRange = scale_range( get_config_float( "Special", "SensorRange", 0 ));

  ram = new TulkonDevice( this, data->spriteWeapon, TULKON_DEVICE_MAX_DIST, weaponDamage,
    weaponForce );

  numBombs = 0;
  maxBombs = get_config_int("Special", "Number", 10);
  bombs = new SpaceObject*[maxBombs];
}

int TulkonRam::activate_weapon(){
  if( ram->dist <= TULKON_DEVICE_MIN_DIST ) return FALSE;
  ram->dist -= TULKON_DEVICE_PULL;
  return TRUE;
}

int TulkonRam::activate_special(){
  if( numBombs == maxBombs ){
    bombs[0]->state = 0;
    numBombs--;
    for( int i = 0; i < numBombs; i++ ){
      bombs[i] = bombs[i + 1];
    }
  }

  bombs[numBombs] = new TulkonBomb( this, Vector2(-get_size().x/2-TULKON_BOMB_DROP_DIST, get_size().y/5), specialDamage,
    specialDRange, specialSRange, specialArmour, specialMass, data->spriteSpecial, 1.0 );
  game->add( bombs[numBombs] );
  numBombs++;

  special_sample = random() % 3;
  return TRUE;
}

void TulkonRam::calculate_fire_special(){
  special_low = FALSE;

  if (fire_special) {
    if (batt < special_drain) {
      special_low = true;
      return;
    }

    if (special_recharge > 0)
      return;

    if (!activate_special())
      return;

    batt -= special_drain;
    if (recharge_amount > 1) recharge_step = recharge_rate;
    special_recharge += special_rate;

    play_sound2(data->sampleSpecial[special_sample], 1000);
  }
  return;
}

void TulkonRam::calculate_fire_weapon(){
  Ship::calculate_fire_weapon();
  if( !fire_weapon && ram->dist != TULKON_DEVICE_MAX_DIST ){
    ram->ramming = true;
  }else{
    ram->ramming = false;
  }
}

void TulkonRam::calculate()
{
  int j = 0;
  for( int i = 0; i < numBombs; i++ ){
    bombs[i - j] = bombs[i];
    if( !bombs[i]->exists() ) j++;
  }
  numBombs -= j;

  recharge_rate = scale_frames( 11 - crew / 2 );

  Ship::calculate();
}

void TulkonRam::calculate_hotspots(){
  if( thrust && hotspot_frame <= 0 ){
//    double tx = cos( angle );
//    double ty = sin( angle );
	Vector2 t = unit_vector(angle);
    game->add( new Animation( this, 
//      normal_x() - tx * w / 2.5 + ty * h / 5,
//      normal_y() - ty * w / 2.5 - tx * h / 5,
	  normal_pos() - product(complex_multiply(unit_vector(angle), get_size()), Vector2(1/2.5, 1/5.0)),
      data->spriteExtra, 0, 12, time_ratio, LAYER_HOTSPOTS));
    game->add( new Animation( this, 
//      normal_x() - tx * w / 2.5 - ty * h / 5,
//      normal_y() - ty * w / 2.5 + tx * h / 5,
	  normal_pos() - product(complex_multiply(unit_vector(-angle), get_size()), Vector2(1/2.5, 1/5.0)),
      data->spriteExtra, 0, 12, time_ratio, LAYER_HOTSPOTS));
      hotspot_frame += hotspot_rate;
  }
  if( hotspot_frame > 0 ) hotspot_frame -= frame_time;
}

int TulkonRam::handle_fuel_sap( SpaceLocation* other, double normal ){
	return 0;
}

void TulkonRam::materialize(){
  Ship::materialize();
  game->add( ram );
}

TulkonDevice::TulkonDevice( Ship* creator, SpaceSprite* osprite, double odist,
  int odamage, double oforce ):
SpaceObject( creator, creator->normal_pos(), creator->get_angle(),
  osprite ), dist( odist ), ramming( false ), force( oforce ){
  collide_flag_sameship = collide_flag_sameteam = ALL_LAYERS;

//  vx = ship->get_vx();
//  vy = ship->get_vy();
  vel = ship->get_vel();
  angle = ship->get_angle();
  sprite_index = get_index(angle);

//  x = ship->normal_x() + cos( angle ) * dist;
//  y = ship->normal_y() + sin( angle ) * dist;
  pos = ship->normal_pos() + dist * unit_vector(angle);

  mass = ship->mass;
  damage_factor = odamage;
}

void TulkonDevice::calculate(){
  SpaceObject::calculate();
  if( !ship ){ state = 0; return; }
  if( !ship->exists() ){ ship = NULL; state = 0; return; }

  ram( ramming );

  angle = ship->get_angle();
  sprite_index = get_index(angle);

  if( dist > TULKON_DEVICE_MAX_DIST ) dist = TULKON_DEVICE_MAX_DIST;
  if( dist < TULKON_DEVICE_MIN_DIST ) dist = TULKON_DEVICE_MIN_DIST;

//  x = ship->normal_x() + cos( angle  ) * dist;
//  y = ship->normal_y() + sin( angle  ) * dist;
  pos = ship->normal_pos() + dist * unit_vector(angle);

//  px = x; py = y; pvx = vx; pvy = vy;

  P = pos;
  PV = vel;
}

int TulkonDevice::canCollide( SpaceLocation* other ){
  if( other == ship ) return FALSE;
  return SpaceObject::canCollide( other );
}

void TulkonDevice::collide( SpaceObject* other ){
  if( ramming ){
    // these lines of code execute
    // once a [ramming] frame 
    // after all calculate()s
    // and before any inflict_damage()s
    play_sound2( data->sampleWeapon[1] );             // play ram_nohit.wav
  }
  SpaceObject::collide( other );
  // by now all collision calculations must have occured either on our side or on the other
  if( ramming ){
    // we return to the speed of the ship to avoid jumping away a bit
    ram( false );
    return;
  }
  if( pos != P ) ship->translate( pos - P );
  if( vel != PV ){
    ship->vel = PV;
  }
}

void TulkonDevice::inflict_damage( SpaceObject* other ){
  if( dist < TULKON_DEVICE_MAX_DIST - TULKON_DEVICE_PULL &&
    ( sameShip( other ) || !other->isShot() )) ram( true );
  if( !ramming ){
    int odf = damage_factor;
    damage_factor = 0;
    SpaceObject::inflict_damage( other );
    damage_factor = odf;
  }else{
    SpaceObject::inflict_damage( other );
  }
  if( ramming ){
    if( sameShip( other ) && !other->isShip() ){       // this must be a TulkonBomb
      sound.stop( data->sampleWeapon[1] );             // stop ram_nohit.wav
      play_sound( data->sampleWeapon[2], 1000 );       // play ram_mine.wav
      TulkonBomb *tb = (TulkonBomb*)other;
      // we tell her to fly in the way we're facing
      tb->rammed = true;
      tb->angle = angle;
    }else{
      sound.stop( data->sampleWeapon[1] );             // stop ram_nohit.wav
      play_sound( data->sampleWeapon[3], 1000 );       // play ram_ship.wav
    }
  }
}

int TulkonDevice::handle_damage( SpaceLocation* other, double normal, double direct ){
  if( direct ) {
	  ship->damage(other, 0, direct);
  }
  normal += direct;
  if( normal > 0 && normal <= 2 ){
     play_sound( data->sampleWeapon[4], 500 );       // play ram_smallhit.wav
  }else if( normal > 2 ){
     play_sound( data->sampleWeapon[5], 500 );       // play ram_largehit.wav
  }
  return normal;
}

void TulkonDevice::ram( bool mode ){
  if( ramming = mode ){
    mass = 1000 * (TULKON_DEVICE_MAX_DIST - dist);
//    vx = ship->get_vx() + cos( angle  ) * (TULKON_DEVICE_MAX_DIST - dist) *
//      0.001 * force;
//    vy = ship->get_vy() + sin( angle  ) * (TULKON_DEVICE_MAX_DIST - dist) *
//      0.001 * force;
	vel = ship->get_vel() + unit_vector(angle) * (TULKON_DEVICE_MAX_DIST - dist) *
      0.001 * force;
    dist = TULKON_DEVICE_MAX_DIST;
    sound.stop( data->sampleWeapon[0] );
  }else{
//    vx = ship->get_vx();
//    vy = ship->get_vy();
	vel = ship->get_vel();
    mass = ship->mass;
  }
}

TulkonBomb::TulkonBomb( SpaceLocation* creator, Vector2 opos, int odamage,
  double odrange, double osrange, double oarmour, double omass, SpaceSprite* osprite,
  double orelativity ):
AnimatedShot( creator, opos, 0, 0, odamage, -1, oarmour, creator, osprite, 64, time_ratio, orelativity ),
  rammed( false ){

  collide_flag_sameship = collide_flag_sameteam = ALL_LAYERS;
  mass = omass;
  srange = osrange;
  drange = odrange;
  exploding = false;
  explosionSample = data->sampleSpecial[3];
  explosionSprite = data->spriteSpecialExplosion;
  explosionFrameCount = 12;

  sprite_index = random() % 64;
}

void TulkonBomb::calculate(){
  AnimatedShot::calculate();

  Query q;
  for( q.begin( this, bit(LAYER_HOTSPOTS), srange ); q.currento; q.next() ){
    if( q.currento->get_sprite() == game->hotspotSprite ){
		damage(this, armour);
    }
  }
  q.end();
}

void TulkonBomb::collide( SpaceObject* other ){
  AnimatedShot::collide( other );
  // by now all collision calculations must have occured either on our side or on the other
  if( rammed ){
//    double v = sqrt( vx*vx + vy*vy );
	v = magnitude(vel);
//    vx = cos( angle ) * v;
//    vy = sin( angle ) * v;
	vel = v * unit_vector(angle);
    rammed = false;
  }
}

int TulkonBomb::handle_damage( SpaceLocation* other, double normal, double direct ){
	int s = exists();
  int d= AnimatedShot::handle_damage( other, normal, direct );
  if( s && !exists() ){
    Query q;
    for( q.begin( this, OBJECT_LAYERS, drange ); q.currento; q.next() ){
// we could use a distance dependant damage factor
//      int dmg = (int)ceil((drange - distance(q.currento)) / drange * damage_factor);
	  q.currento->damage(this, damage_factor);
    }
    animateExplosion();
    soundExplosion();
  }
  return d;
}

void TulkonBomb::soundExplosion(){
  play_sound2( explosionSample, 1000 );
}

void TulkonBomb::inflict_damage( SpaceObject* other ){}

REGISTER_SHIP(TulkonRam)