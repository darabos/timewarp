#include "../ship.h"
REGISTER_FILE
#include "../melee/mcbodies.h"

/*
 * created by: cyhawk@sch.bme.hu and forevian@freemail.hu
 *
 * Yes, this is a joke.
 */
class PlanetShip : public Ship {
  int    planet_index;

  Planet* planet;

  Planet* nearest_other_planet();

  public:

  PlanetShip(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  virtual int activate_special();
  virtual double isInvisible() const;
  virtual void calculate_hotspots();
  virtual void calculate();
  virtual int handle_damage( SpaceLocation* other, double normal, double direct );
  virtual void inflict_damage( SpaceObject* other );
  virtual int accelerate_gravwhip( SpaceLocation *source, double angle,
    double velocity, double max_speed );
  virtual void materialize();
  virtual void death();
};

class InvisiblePlanet : public Planet {
  public:
  InvisiblePlanet( SpaceLocation* creator, Vector2 opos,
    SpaceSprite *sprite, int index );

  virtual void animate( Frame* space );
  virtual void calculate();
};

PlanetShip::PlanetShip( Vector2 opos, double shipAngle,
  ShipData *shipData, unsigned int code ):
  Ship( opos, shipAngle, shipData, code )
{
  sprite_index = planet_index = random() % 3;
  if( sprite_index != 0 ){
    BITMAP *bmp = spritePanel->get_bitmap( 0 );
    blit( spritePanel->get_bitmap( 6 + sprite_index ), bmp, 0, 0, 0, 0, 63, 99 );
    update_panel = TRUE;
  }
  planet = new InvisiblePlanet( this, pos, sprite, sprite_index );
}

double PlanetShip::isInvisible() const{
	return (special_recharge > 0) ? 1 : 0;
}

int PlanetShip::activate_special(){
  return TRUE;
}

void PlanetShip::calculate_hotspots(){
  return;
}

void PlanetShip::calculate(){
  Ship::calculate();
  if( special_recharge > 0 ){
//    vx = vy = 0;
	vel = 0;
  }
  sprite_index = planet_index;
//  planet->x = x;
//  planet->y = y;
  planet->pos = pos;
//  planet->vx = vx;
//  planet->vy = vy;
  planet->vel = vel;
}

void PlanetShip::materialize(){
  planet->translate( pos );
  game->add( planet );
}

void PlanetShip::death(){
  planet->state = 0;
}

int PlanetShip::handle_damage(SpaceLocation *other, double normal, double direct){
//  if( weapon_recharge > 0 ) damage = 0;
  return Ship::handle_damage( other , normal, direct);
}

void PlanetShip::inflict_damage(SpaceObject *other){
	if( weapon_recharge <= 0 ) return;
	int i = 1;
	if (other->isShip()) {
		i = (int) ceil(((Ship*)other)->crew / 3.0);
		}
	if (other->mass == 0) other->state = 0;
	damage(other, 0, i);
	i /= 2;
	if (i >= BOOM_SAMPLES) i = BOOM_SAMPLES - 1;
	if (!other->isShot()) 
		play_sound((SAMPLE *)(melee[MELEE_BOOM + i].dat));
	return;
}

int PlanetShip::accelerate_gravwhip( SpaceLocation *source, double angle,
  double velocity, double max_speed ){

  Planet *p = nearest_other_planet();
  if( !p ) return SpaceLocation::accelerate( source, angle, velocity, max_speed );
  double tmp;
  tmp = distance( p ) / p->gravity_range;
  if( tmp > 1 ) return SpaceLocation::accelerate( source, angle, velocity, max_speed );
  return SpaceLocation::accelerate( source, angle, velocity,
    max_speed * (p->gravity_whip * tmp + 1) );
}

Planet *PlanetShip::nearest_other_planet(){
  Planet *p = NULL;
  double r = 99999999;
  Query q;
  q.begin( this, bit(LAYER_CBODIES), 1600 );
  while( q.current ){
    if( q.current->isPlanet() && q.currento != planet ){
      double t = distance(q.current);
      if( t < r ){
        r = t;
        p = (Planet*)q.current;
      }
    }
    q.next();
  }
  q.end();
  return p;
}

InvisiblePlanet::InvisiblePlanet( SpaceLocation *creator, Vector2 opos,
  SpaceSprite *sprite, int index ):Planet( opos, sprite, index){
  collide_flag_sameship = collide_flag_sameteam = collide_flag_anyone = 0;
  change_owner( creator );
}

void InvisiblePlanet::animate( Frame* space ){}

void InvisiblePlanet::calculate(){
// just exclude the PlanetShip from the gravity effect

  SpaceObject::calculate();
  SpaceObject *o;
  Query a;
  a.begin(this, OBJECT_LAYERS, gravity_range);
  for (;a.currento;a.next()) {
    o = a.currento;
    if (o->mass > 0 && o != ship) {
      double r = distance(o);
      if (r < gravity_mindist) r = gravity_mindist;
      double sr = 1;
//gravity_power truncated here
      if (gravity_power > 0) {
        r /= 40 * 4;
        for (int i = 0; i < gravity_power; i += 1) sr *= r;
        o->accelerate(this, trajectory_angle(o) + PI, frame_time * gravity_force / sr, MAX_SPEED);
      }else{
        r = 1 - r/gravity_range;
        for (int i = 0; i < -gravity_power; i += 1) sr *= r;
        o->accelerate(this, trajectory_angle(o) + PI, frame_time * gravity_force * sr, MAX_SPEED);
      }
    }
  }
}

REGISTER_SHIP(PlanetShip)
