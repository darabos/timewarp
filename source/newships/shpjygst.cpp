#include "../ship.h"
REGISTER_FILE
#include "../melee/mview.h"

/*
 * created by: cyhawk@sch.bme.hu and forevian@freemail.hu
 */
class JyglarStarfarer : public Ship {
// the ship

  double        weaponRange;          // maximum range
  double        weaponVelocity;
  int           weaponDamage;         // does damage between zero and this value inclusive
  int           weaponArmour;
  int           weaponStray;          // percentage of shots going astray
  double        weaponPull;           // pulling force strength
  int           melody;

  double        specialMass;
  SpaceObject** bubbles;
  int           numBubbles;
  int           maxBubbles;

  public:
  JyglarStarfarer( Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code );

  virtual int activate_weapon();      // shoots a shot
  virtual int activate_special();     // creates a bubble
  virtual void calculate();           // takes note of bursted bubbles
  virtual void calculate_hotspots();  // blue hotspots
  ~JyglarStarfarer();
};

class JyglarShot : public Shot {
// shot that pulls the object hit towards a point before the ship

  double pull;
  SpaceLocation* beacon;   // desired position of target

  public:
  JyglarShot( SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
    int odamage, double orange, int oarmour, SpaceLocation *opos, SpaceSprite *osprite,
    double opull, SpaceLocation* obeacon, double relativity = game->shot_relativity );

  virtual void inflict_damage( SpaceObject* other );

  ~JyglarShot();  // frees up beacon memory
};

class JyglarStrayShot : public JyglarShot {
// shot going astray

  int minturn;
  int maxturn;

  public:
  JyglarStrayShot( SpaceLocation *creator, Vector2 rpos, double oangle, double ov,
    int odamage, double orange, int oarmour, SpaceLocation *opos, SpaceSprite *osprite,
    double opull, SpaceLocation* obeacon, double relativity = game->shot_relativity );

  virtual void calculate();           // changes direction in a random way
};

class JyglarBubble : public SpaceObject {
// bubbles surrounding the ship

  double dangle;
  double dist;
  int countdown;

  public:
  JyglarBubble( SpaceLocation *creator, double odist, double odangle,
    SpaceSprite *osprite, double omass );

  virtual void calculate();           // follows ship movement
  virtual int handle_damage( SpaceLocation* other, double normal, double direct );  // bursts on damage with random sound
};

JyglarStarfarer::JyglarStarfarer( Vector2 opos, double shipAngle,
  ShipData *shipData, unsigned int code ):
  Ship( opos, shipAngle, shipData, code )
{
  weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
  weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
  weaponDamage   = get_config_int("Weapon", "Damage", 0);
  weaponArmour   = get_config_int("Weapon", "Armour", 0);
  weaponStray    = (int)(get_config_float("Weapon", "Stray", 0) * 100);
  weaponPull     = scale_velocity(get_config_int("Weapon", "Pull", 0));

  specialMass = get_config_float("Special", "Mass", 1);
  numBubbles  = 0;
  maxBubbles  = get_config_int("Special", "Number", 10);
  bubbles     = new SpaceObject*[maxBubbles];

  melody = 0;
}

int JyglarStarfarer::activate_weapon(){
  Shot *shot;
  SpaceLocation* beacon = new SpaceLocation( this,
	  pos + unit_vector(angle) * weaponRange / 3, angle );
  if( random() % 100 < weaponStray ){
    game->add( shot = new JyglarStrayShot( this, 
      Vector2(-2.0 + 0.5 * (double)(random() % 5), size.y * 0.6), angle +(- 2.0 + (double)(random() % 5))*ANGLE_RATIO,
      weaponVelocity, random() % (weaponDamage + 1), random() % ((int)weaponRange + 1),
      weaponArmour, this, data->spriteSpecial, weaponPull, beacon ));
    weapon_sample = 7 + random() % 3;
  }else{
    int shot_damage = random() % (weaponDamage + 1);
    game->add( shot = new JyglarShot( this, 
      Vector2(-2.0 + 0.5 * (double)(random() % 5), size.y * 0.5), angle +(- 2.0 + (double)(random() % 5))*ANGLE_RATIO,
      weaponVelocity, shot_damage, random() % ((int)weaponRange + 1),
      weaponArmour, this, data->spriteSpecial, weaponPull, beacon ));
    if( shot_damage == 0 ){
      weapon_sample = 10;
    }else{
      melody += -1 + random() % 3;
      weapon_sample = abs( melody ) % 7;
    }
  }
  shot->explosionSprite = data->spriteSpecialExplosion;
  shot->explosionFrameCount = 6;
  shot->explosionSample = data->sampleSpecial[1 + random() % (data->num_special_samples - 1)];
  return TRUE;
}

int JyglarStarfarer::activate_special(){
  if( numBubbles >= maxBubbles ) return FALSE;
  bubbles[numBubbles] = new JyglarBubble( this, size.x / 5 + random() % (int)(size.x / 3),
    random(PI2), data->spriteWeapon, specialMass);
  game->add( bubbles[numBubbles] );
  numBubbles++;
  return TRUE;
}

void JyglarStarfarer::calculate(){
  int j = 0;
  for( int i = 0; i < numBubbles; i++ ){
    bubbles[i - j] = bubbles[i];
    if( !bubbles[i]->exists() ) j++;
  }
  numBubbles -= j;

  Ship::calculate();
}

void JyglarStarfarer::calculate_hotspots(){
  if( thrust && hotspot_frame <= 0 ){
    game->add( new Animation( this, 
		normal_pos() - unit_vector(angle) * size.y / 4,
      data->spriteExtra, 0, 12, time_ratio, LAYER_HOTSPOTS));
      hotspot_frame += hotspot_rate;
  }
  if( hotspot_frame > 0 ) hotspot_frame -= frame_time;
}

JyglarStarfarer::~JyglarStarfarer(){
  delete bubbles;
}

JyglarShot::JyglarShot( SpaceLocation *creator, Vector2 rpos,
  double oangle, double ov, int odamage, double orange, int oarmour,
  SpaceLocation *opos, SpaceSprite *osprite, double opull, SpaceLocation* obeacon,
  double relativity ):
Shot( creator, rpos, oangle, ov, odamage, orange, oarmour, opos, osprite,
  relativity ), pull( opull ), beacon( obeacon )
{
}

void JyglarShot::inflict_damage( SpaceObject* other ){
  if( other->mass && !other->isPlanet() ){
    other->accelerate( this, other->trajectory_angle( beacon ), pull / other->mass, MAX_SPEED );
  }
  Shot::inflict_damage( other );
}

JyglarShot::~JyglarShot(){
  if( beacon ) delete beacon;
}

JyglarStrayShot::JyglarStrayShot( SpaceLocation *creator, Vector2 rpos,
  double oangle, double ov, int odamage, double orange, int oarmour,
  SpaceLocation *opos, SpaceSprite *osprite, double opull, SpaceLocation* obeacon,
  double relativity ):
JyglarShot( creator, rpos, oangle, ov, odamage, orange, oarmour, opos, osprite,
  opull, obeacon, relativity )
{
//  collide_flag_sameteam = collide_flag_sameship = ALL_LAYERS;
  minturn = -(random() % 2);
  maxturn = random() % 2;
}

void JyglarStrayShot::calculate(){
  Shot::calculate();
  changeDirection( angle + (minturn + random() % (maxturn - minturn + 1)) * frame_time );
}

JyglarBubble::JyglarBubble( SpaceLocation *creator, double odist, double odangle,
    SpaceSprite *osprite, double omass ):
SpaceObject( creator,
  creator->normal_pos() + odist * unit_vector( (creator->get_angle() + odangle) ),
  creator->get_angle() + odangle, osprite )
{
  mass = omass;
  dist = odist;
  dangle = odangle;
  vel = ship->get_vel();
  sprite_index = get_index(angle);
  countdown = 1000 + random() % 2000;
}

void JyglarBubble::calculate(){
  SpaceObject::calculate();
  if( !ship ){
    countdown -= frame_time;
    if( countdown <= 0 ){
      state = 0;
      play_sound( data->sampleExtra[random() % data->num_extra_samples] );
    }
    return;
  }
  if( !(ship && ship->exists()) )
  {
    ship = 0;
    return;
  }
  angle = ship->get_angle() + dangle;
  pos = ship->normal_pos() + dist * unit_vector(angle);
  vel = ship->get_vel();
  sprite_index = get_index(angle);
}

int JyglarBubble::handle_damage( SpaceLocation* source, double normal, double direct ){
  if( normal + direct ){
    state = 0;
    play_sound( data->sampleExtra[random() % data->num_extra_samples], 1000 );
  }
  return normal + direct;
}



REGISTER_SHIP(JyglarStarfarer)
