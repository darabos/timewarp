#include "../ship.h"
REGISTER_FILE

/*
 * created by: cyhawk@sch.bme.hu
 */
class KatPoly : public Ship {
// the ship

  double weaponRange;
  double weaponVelocity;
  int    weaponDamage;
  int    weaponArmour;

  int    specialDrain;                    // morphing into alien form costs
                                          // shipSpecialDrain + specialDrain
                                          // while morphing back only costs
                                          // shipSpecialDrain

  Ship* morph;                            // the ship we are currently morphed into

  public:
  KatPoly( Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code );

  virtual double  isInvisible() const;    // not visible when morphed
  virtual int handle_damage( SpaceLocation* other, double normal, double direct );  // not vulnerable when morphed
  virtual void animate( Frame* space );   // not animating when morphed
  virtual void calculate();
  virtual void calculate_hotspots();      // not leaving hotspots when morphed
  virtual void calculate_turn_left();     // not turning when morphed
  virtual void calculate_turn_right();    // not turning when morphed
  virtual void calculate_fire_weapon();   // not firing when morphed
  virtual void calculate_fire_special();  // set proper cost of morphing
  virtual RGB crewPanelColor();           // display stats in the current form's color
  virtual RGB battPanelColor();           // display stats in the current form's color
  virtual int  activate_weapon();         // shoot
  virtual int  activate_special();        // morph
};

KatPoly::KatPoly( Vector2 opos, double shipAngle,
  ShipData *shipData, unsigned int code ):
  Ship( opos, shipAngle, shipData, code )
{
  weaponRange    = scale_range( get_config_float("Weapon", "Range", 0 ));
  weaponVelocity = scale_velocity( get_config_float("Weapon", "Velocity", 0 ));
  weaponDamage   = get_config_int( "Weapon", "Damage", 0 );
  weaponArmour   = get_config_int( "Weapon", "Armour", 0 );

  specialDrain   = get_config_int( "Special", "Drain", 0 );

  morph = NULL;  // start unmorphed
}

double  KatPoly::isInvisible() const{
	return morph ? 1 : 0;
}
int KatPoly::handle_damage( SpaceLocation* other, double normal, double direct ){
  if( !morph ) return Ship::handle_damage( other, normal, direct );
  return 0;
}
void KatPoly::animate( Frame* space ){
  if( !morph ) Ship::animate( space );
}

void KatPoly::calculate(){
	if( morph ){
		morph->nextkeys &= ~keyflag::special;  // we will handle special so disable the morph's
		if( !morph->exists() ) {
			morph = 0;
			state = 0;      // if the morph died we died as well
			return;
		}
		crew = morph->crew;                    // let the crew stat show on the panel
		batt = morph->batt;                    // let the batt stat show on the panel
	}
	Ship::calculate();
}

void KatPoly::calculate_hotspots(){    if( !morph ) Ship::calculate_hotspots(); }
void KatPoly::calculate_turn_left(){   if( !morph ) Ship::calculate_turn_left(); }
void KatPoly::calculate_turn_right(){  if( !morph ) Ship::calculate_turn_right(); }
void KatPoly::calculate_fire_weapon(){ if( !morph ) Ship::calculate_fire_weapon(); }

int KatPoly::activate_weapon(){
  game->add( new AnimatedShot( this, Vector2(0, -size.y/2), angle, weaponVelocity, weaponDamage,
    weaponRange, weaponArmour, this, data->spriteWeapon, 64, time_ratio ));
  return true;
}

void KatPoly::calculate_fire_special(){
  if( morph ){
    if( morph->type != type ){
      // if in alien form use lower cost
      Ship::calculate_fire_special();
      return;
    }
  }
  special_drain += specialDrain;
  Ship::calculate_fire_special();
  special_drain -= specialDrain;
}

RGB KatPoly::crewPanelColor(){
  if( morph ) return morph->crewPanelColor();
  return Ship::crewPanelColor();
}

RGB KatPoly::battPanelColor(){
  if( morph ) return morph->battPanelColor();
  return Ship::battPanelColor();
}

int KatPoly::activate_special(){
  ShipType* morph_target;

  if( morph ){                                // if we have already morphed
    if( morph->type == type ){    // if we are in natural form
      if( !target ) return false;             // we need a target
	  if( !target->isShip()) return false;
      morph_target = ((Ship*)target)->type;      // we will morph to it
    }else{                                    // if we are in alien form
      morph_target = type;              // morph to natural form
    }
    pos = morph->normal_pos();                    // get it's stats
    vel = morph->get_vel();
    angle = morph->get_angle();
    crew = morph->crew;
    batt = morph->batt;
    morph->state = 0;                         // kill last morph
  }else{                                      // if we have never yet morphed
    if( !target ) return false;               // we need a target
    if( !target->isShip()) return false;
    morph_target = ((Ship*)target)->type;      // we will morph to it

    int i;                                    // remove ourselves from the game as a target
    for( i = 0; game->target[i] != this; i++ );
    game->num_targets--;
    game->target[i] = game->target[game->num_targets];
    collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = 0;
    id = 0;                                   // get immaterial
  }
  // create the new ship
  morph = game->create_ship( morph_target->id, control, pos, angle, get_team() );
  game->add( morph );              // add the ship
  morph->materialize();                // materialize it
  morph->crew = crew;                  // set it's attributes
  morph->batt = batt - special_drain;  // [battery has to be decreased now]
  morph->vel = vel;
  update_panel = true;                 // maybe the colors changed
  return true;                         // we did it
}

REGISTER_SHIP(KatPoly)
