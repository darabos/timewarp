/* $Id$ */ 
#include "../ship.h"
REGISTER_FILE

/*
 * created by: cyhawk@sch.bme.hu
*/


// allows other ships to affect control over a ship.
class OverrideControlMorph : public OverrideControl
{
public:
	virtual void calculate(short *key);
};


class KatPoly : public Ship {
public:
IDENTITY(KatPoly);
// the ship

	OverrideControlMorph *ocm;

  double weaponRange;
  double weaponVelocity;
  int    weaponDamage;
  int    weaponArmour;

  int    specialDrain;                    // morphing into alien form costs
                                          // shipSpecialDrain + specialDrain
                                          // while morphing back only costs
                                          // shipSpecialDrain


  public:
  
  Ship* morph;                            // the ship we are currently morphed into

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
  virtual RGB crewPanelColor(int k = 0);           // display stats in the current form's color
  virtual RGB battPanelColor(int k = 0);           // display stats in the current form's color
  virtual int  activate_weapon();         // shoot
  virtual int  activate_special();        // morph
};

class KatAnimatedShot : public AnimatedShot
{
public:
IDENTITY(KatAnimatedShot);
public:
	KatPoly *mother;

	KatAnimatedShot(KatPoly *creator, Vector2 rpos, 
	double oangle, double ov, double odamage, double orange, double oarmour, SpaceLocation *opos,
	SpaceSprite *osprite, int ofcount, int ofsize, double relativity = 0);

	virtual int handle_damage( SpaceLocation* other, double normal, double direct );
	virtual void inflict_damage(SpaceObject *other);
	virtual void calculate();
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

  ocm = 0;
}




double  KatPoly::isInvisible() const{
	return morph ? 1 : 0;
}

int KatPoly::handle_damage( SpaceLocation* other, double normal, double direct )
{
	 
  if( !morph )
  {
	  Ship::handle_damage( other, normal, direct );
  }

  if (!exists())
  {
	  // you have to include this here, cause a ship can die by external causes (through a handle-damage)
	  // in which case, the control override should be removed... well, not that it really matters, cause
	  // then the morph also dies, but well..

	  if (morph)
		  morph->del_override_control(ocm);
  }

  return 0;
}
void KatPoly::animate( Frame* space ){
	 
  if( !morph ) Ship::animate( space );
}


void OverrideControlMorph::calculate(short *key)
{
	*key &= ~keyflag::special;  // we will handle special so disable the morph's
}


void KatPoly::calculate()
{
	 

	if( morph )
	{
		//morph->nextkeys &= ~keyflag::special;  // we will handle special so disable the morph's
		//xxx hmmm ??
		

		if( !morph->exists() )
		{
			// first, remove the control override:
			morph->del_override_control(ocm);

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

int KatPoly::activate_weapon()
{
	 

	game->add( new KatAnimatedShot( this, Vector2(0, -size.y/2), angle+PI, weaponVelocity, weaponDamage,
		weaponRange, weaponArmour, this, data->spriteWeapon, data->spriteWeapon->frames(), time_ratio ));

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

RGB KatPoly::crewPanelColor(int k){
	 
  if( morph ) return morph->crewPanelColor(k);
  return Ship::crewPanelColor(k);
}

RGB KatPoly::battPanelColor(int k){
	 
  if( morph ) return morph->battPanelColor(k);
  return Ship::battPanelColor(k);
}

int KatPoly::activate_special()
{
	 
	if( !(target && target->exists()) ) return false;               // we need a target
	if( !target->isShip()) return false;

	ShipType* morph_target = 0;
	
	if( morph )
	{                                // if we have already morphed
		if( morph->type == type ){    // if we are in natural form
			morph_target = ((Ship*)target)->get_shiptype();      // we will morph to it
		}else{                                    // if we are in alien form
			morph_target = get_shiptype();              // morph to natural form
		}
		pos = morph->normal_pos();                    // get it's stats
		vel = morph->get_vel();
		angle = morph->get_angle();
		crew = morph->crew;
		batt = morph->batt;
		morph->state = 0;                         // kill last morph
		morph->crew = 0;

		// dangerous, this is a memory leak.
		//game->remove(morph);

		// the following prevents that a new ship will be "selected" based on this "empty" ship
		morph->attributes &= ~ATTRIB_NOTIFY_ON_DEATH;
		morph->control = 0;

		// remove the special-override for the morph
		morph->del_override_control(ocm);
		ocm = 0;

	} else {                                      // if we have never yet morphed

		morph_target = ((Ship*)target)->get_shiptype();      // we will morph to it
		
		//    int i;                                    // remove ourselves from the game as a target
		//    for( i = 0; game->target[i] != this; i++ );
		//    game->num_targets--;
		//    game->target[i] = game->target[game->num_targets];
		targets->rem(this);
		collide_flag_anyone = collide_flag_sameteam = collide_flag_sameship = 0;
		id = 0;                                   // get immaterial
	}
	// create the new ship
	if (!morph_target)
	{
		throw("Kat Poly: morph target == 0.");
	}
	morph = game->create_ship( morph_target->id, control, pos, angle, get_team() );
	game->add( morph );              // add the ship
	morph->materialize();                // materialize it
	morph->crew = crew;                  // set it's attributes
	morph->batt = batt - special_drain;  // [battery has to be decreased now]
	morph->set_vel ( vel );
	update_panel = true;                 // maybe the colors changed

	// add control-override for the morph (disable its special key)
	ocm = new OverrideControlMorph();
	morph->set_override_control(ocm);

	return true;                         // we did it
}



KatAnimatedShot::KatAnimatedShot(KatPoly *creator, Vector2 rpos, 
	double oangle, double ov, double odamage, double orange, double oarmour, SpaceLocation *opos,
	SpaceSprite *osprite, int ofcount, int ofsize, double relativity) 
:
AnimatedShot(creator, rpos, oangle, ov, odamage, orange, oarmour, opos, osprite,
			 ofcount, ofsize, relativity)
{
	mother = creator;
}

void KatAnimatedShot::calculate()
{
	 
	if (!(mother && mother->exists()))
	{
		mother = 0;
		state = 0;
		return;
	}

	AnimatedShot::calculate();
}

int KatAnimatedShot::handle_damage( SpaceLocation* other, double normal, double direct )
{
	 
	if (other != mother->morph)
		return AnimatedShot::handle_damage(other, normal, direct);
	else
	{
		state = 0;
		return 0;
	}
}

void KatAnimatedShot::inflict_damage(SpaceObject *other)
{
	 
	if (other != mother->morph)
		AnimatedShot::inflict_damage(other);
}

REGISTER_SHIP(KatPoly)
