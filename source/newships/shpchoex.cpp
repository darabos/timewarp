/* $Id$ */ 
#include "../ship.h"
#include "../melee/mcbodies.h"

REGISTER_FILE

// allows other ships to affect control over a ship.
class OverrideControlChorali : public OverrideControl
{
public:
	virtual void calculate(short *key);
};

#define ASTEROID_CENTER 0x03a8c9f5a


/*
Since this is my [Culture20] first ship, I obviously nabbed a lot of this code from other ship files.
The sources of my inspiration:
  Chmmr Avatar for the planet-charge
  Mycon Podship for the Animated Homing Missile (Asteroid; if you haven't seen this yet, grab an asteroid and press fire again)
  Dataklakpak Vivisector for the grabbing, of course (was the main skeleton of my ship)
  Pkunk Fury for the Planet Location info (why it's related to their regeneration I'll never know)
  
 */

/*****************************************************************************/
//debris class defs

#define CH_TWIST_ANGLE 0.03*PI	//added by Tau, as a simple hack for consistent tractor-induced rotation
//enemy "twisting" can be improved further, but I'm too lazy to do that at the moment

class AsteroidDebris : public Asteroid
{
public:
IDENTITY(AsteroidDebris);
  Ship *creator;
  int frame_born;
  bool collide_flag;
  int tractorForce;
  double multiple;
public:
  AsteroidDebris(Ship *creator1, Vector2 new_pos, int tforce );
  virtual void death();
  virtual int canCollide(SpaceLocation *other);
  virtual void calculate();
};

AsteroidDebris::AsteroidDebris(Ship *creator1, Vector2 new_pos, int tforce)
: Asteroid()
{
	creator=creator1;
	pos = new_pos;
	frame_born=(int)(game->frame_number);
	collide_flag=FALSE;
	sprite_index = tw_random(sprite->frames());
	tractorForce=tforce;
	
	if (mass > 0 && creator && tractorForce)
		this->accelerate(creator, this->trajectory_angle(creator), tractorForce / (this->mass * 2), 6);

	attributes &= ~ATTRIB_STANDARD_INDEX;
}

void AsteroidDebris::death()
{
	SpaceObject::death();
}

void AsteroidDebris::calculate()
{

	if (creator && !creator->exists())
		creator = 0;

	SpaceObject::calculate();

	 
	step-= frame_time;
	while(step <= 0) {
		step += speed * time_ratio;
		sprite_index++;
		if(sprite_index == sprite->frames())//ASTEROID_FRAMES)
			sprite_index = 0;
	}
	
	
	
	if(!collide_flag)
    {
		if (game->frame_number - frame_born >= 40)
		{
			collide_flag=TRUE;
		}
    }
}

int AsteroidDebris::canCollide(SpaceLocation *other) 
{
	 

	if (collide_flag)
    {
		return SpaceObject::canCollide(other);
    }
	else if(other != creator)
    {
		return SpaceObject::canCollide(other);
    }

	return 0;
}

class AsteroidCenter : public AsteroidDebris
{
public:
IDENTITY(AsteroidCenter);
  //Ship *creator; already defined in the asteroidcenter class?!
public:
  AsteroidCenter(Ship *creator, Vector2 new_pos);
  virtual void calculate();
  virtual int canCollide(SpaceLocation *other);
  virtual int handle_damage(SpaceLocation *source, double normal, double direct);
  //virtual bool isAsteroid();	//this is not a virtual routine...
  virtual bool die();
};


AsteroidCenter::AsteroidCenter(Ship *creator1, Vector2 new_pos) 
: AsteroidDebris(creator1, new_pos, 0)
{
  layer = LAYER_SHOTS;
  collide_flag_anyone = ALL_LAYERS &~ bit(LAYER_CBODIES);
//  creator=creator1;

  // so that it is not recognized as an asteroid anymore...
  id = ASTEROID_CENTER;
}

bool AsteroidCenter::die()
{
	// do nothing...
	return false;
}

void AsteroidCenter::calculate()
{

	 
	AsteroidDebris::calculate();
	

	if (creator == NULL)
    {
		//this->~AsteroidCenter();	//highly illegal because it's also destroyed in a physics list!!
		state = 0;
    }
	else
    {
		// translate(creator->normal_x() - normal_x(), creator->normal_y() - normal_y());
		pos = creator->normal_pos();
    }
}

int AsteroidCenter::canCollide(SpaceLocation *other) 
{
	 

	if (!other)
		return 0;

  if (!other->damage_factor)
	  return false;

  return
	  SpaceObject::canCollide(other);
}

int AsteroidCenter::handle_damage(SpaceLocation *source, double normal, double direct) 
{
	return iround(normal + direct);
}

//bool AsteroidCenter::isAsteroid()
//{
//  return false;
//}

/*****************************************************************************/
//tractorBeam class defs

class ChoraliTractorBeam : public Laser 
{
public:
IDENTITY(ChoraliTractorBeam);
  int tractorForce;
  int tractorPushForce;
  SpaceLocation *creator;
  public:
  ChoraliTractorBeam(SpaceLocation *creator1, double langle, int lcolor, double lrange, int ldamage, int lfcount,
    SpaceLocation *opos, double rel_x, double rel_y, bool osinc_angle, int tForce, int tPushForce);

  virtual void inflict_damage(SpaceObject *other);
};


ChoraliTractorBeam::ChoraliTractorBeam(SpaceLocation *creator1, double langle, int lcolor, double lrange,
               int ldamage, int lfcount, SpaceLocation *opos, double rel_x, double rel_y, bool osinc_angle, int tForce, int tPushForce) :
Laser(creator1, langle, lcolor, lrange, 
       ldamage, lfcount, opos, Vector2(rel_x,rel_y), osinc_angle)
{
  tractorForce = tForce;
  tractorPushForce = tPushForce;
  creator=creator1;
}

void ChoraliTractorBeam::inflict_damage(SpaceObject *other)
{
	 
	// SpaceObject::inflict_damage(other);
	if(other != NULL)
	{
		if ((other->mass > 0) && ( other->isShip() || other->isAsteroid() ))
		{
			
			other->accelerate(this, other->trajectory_angle(this), tractorForce / (other->mass * 4), 2);
			
			if(other->isShip())
			{
				//twist the enemy
				if (other->trajectory_angle(this) <= other->get_angle() )
				{
					if ((other->get_angle() - other->trajectory_angle(this)) <= PI)
					{
						((Ship*)other)->turn_step+=CH_TWIST_ANGLE;
					}
					else
					{
						((Ship*)other)->turn_step-=CH_TWIST_ANGLE;
					}
				}
				else
				{
					if ((other->trajectory_angle(this) - other->get_angle()) <= PI)
					{
						((Ship*)other)->turn_step-=CH_TWIST_ANGLE;
					}
					else
					{
						((Ship*)other)->turn_step+=CH_TWIST_ANGLE;
					}	    
				}
			}
			
		}
		else if (other->isPlanet() && random(100) <= 20)
		{ //if it's a planet, make some non-regenerating asteroids
			Vector2 dd = creator->normal_pos() - ( (creator->nearest_planet()) )->normal_pos();
			
			dd /= 2;
			add( new AsteroidDebris((Ship *)creator, creator->normal_pos() - dd, tractorForce) );
		}
		else if (other->isShot()) 
		{//if It's a weapon Shot
			if (other->mass > 0)
				other->accelerate(this, (other->trajectory_angle(this) + PI), (tractorPushForce / ((other->mass * 3)+20)), MAX_SPEED);
			
		}   
		state = 0;
	}
}
/*****************************************************************************/

//asteroid missle class defs




class AsteroidMissile : public HomingMissile
{
public:
IDENTITY(AsteroidMissile);
protected:
  Ship *creator;
  SpaceSprite *explosion;
  int sprite_index_count;
  int frame_count;
  double range;

  int    tractorR;
  int    tractorB;
  int    tractorG;
  int    tractorRmin;
  int    tractorBmin;
  int    tractorGmin;


public:
  AsteroidMissile(double ox, double oy, double oangle, double ov, int odamage,
    double orange, int oarmour, double otrate, Ship *oship, SpaceSprite *osprite, int oframe_count, 
    int R, int Rm, int G, int Gm, int B, int Bm);

  virtual void calculate();
  virtual void inflict_damage(SpaceObject *other);
  virtual int handle_damage(SpaceLocation *source, double normal, double direct);
  virtual void death();
};

//SpaceSprite *AsteroidMissile::spriteAsteroidExplosion = NULL;
AsteroidMissile::AsteroidMissile(double ox, double oy, double oangle,
  double ov, int odamage, double orange, int oarmour, double otrate,
  Ship *oship, SpaceSprite *osprite, int oframe_count,
    int R, int Rm, int G, int Gm, int B, int Bm) :
  HomingMissile(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, otrate, 
		oship, osprite, oship->target), 
  explosion(meleedata.asteroidExplosionSprite), frame_count(oframe_count) 
{
	collide_flag_sameship = bit(LAYER_SHIPS) | bit(LAYER_SHOTS);
	//	explosionSprite     = game->asteroidExplosionSprite;
	
	//  explosionSprite     = data->spriteWeaponExplosion;
	//  explosionFrameCount = 10;
	//  explosionFrameSize  = 50;
  
  sprite_index=0;
  sprite_index_count=0;
  creator = oship;
  range = orange;

  tractorR = R;
  tractorB = G;
  tractorG = B;
  tractorRmin = Rm;
  tractorBmin = Gm;
  tractorGmin = Bm;


}

void AsteroidMissile::calculate()
{
	 

  HomingMissile::calculate();
  if(sprite_index_count<63)
    { 
      sprite_index_count++;
    }
  else
    {
      sprite_index_count=0;
    }
  sprite_index=sprite_index_count;
  //  if(random(100) <=20)
  //  {
      add(new ChoraliTractorBeam(this, this->trajectory_angle(creator),
	makecol((random(tractorR-tractorRmin+1))+tractorRmin, (random(tractorG-tractorGmin+1))+tractorGmin, (random(tractorB-tractorBmin+1))+tractorBmin), 
	range, 0, 0, this, 0.0, 0.0, true, 0, 0));
      //  }

}

void AsteroidMissile::inflict_damage(SpaceObject *other)
{
	 
  SpaceObject::inflict_damage(other);
  add(new Animation(this, pos, explosion, 0, explosion->frames(), time_ratio, LAYER_CBODIES));
  state=0;
}

int AsteroidMissile::handle_damage(SpaceLocation *source, double normal, double direct) 
{
	 
  if (!exists()) return 0;
  if (!normal && !direct) return 0;
  
  add(new Animation(this, pos, explosion, 0, explosion->frames(), time_ratio, LAYER_CBODIES));


  state = 0;
  return iround(direct + normal);  
}
void AsteroidMissile::death()
{
	 
  HomingMissile::death();
  add(new Animation(this, pos, explosion, 0, explosion->frames(), time_ratio, LAYER_CBODIES));
}
/*****************************************************************************/


class ChoraliExtractor : public Ship {
public:
IDENTITY(ChoraliExtractor);

  int          weaponFrames;
  int 	       drillFrames;
  int          weaponDamage;
  int          damageFrameAmount;
  int          drillDamagePerDamageFrame;
  int          damageFrameLeft;
  int          drillDamageLeft;
  int          latched;
  int          count;
  SpaceObject *grabbed;
  double       grabangle;
  double       grabdistance;
  double       grabshipangle;
  
  double       specialLaunch;
  double       specialRange;
  double       specialVelocity;
  double       specialForce;
  int          specialDamage;
  int          specialArmour;
  int          specialArming;

  double       AsteroidMissileRange;
  double       AsteroidMissileVelocity;
  int          AsteroidMissileDamage;
  int          AsteroidMissileArmour;
  double       AsteroidMissileTurnRate;

  int    extraColor;
  double extraRange;
  int    extraFrames;
  int    extraDamage;

  int    tractorColor;
  int    tractorR;
  int    tractorB;
  int    tractorG;
  int    tractorRmin;
  int    tractorBmin;
  int    tractorGmin;
  char   color_case;
  double tractorRange;
  int    tractorRate;
  int    tractorDamage;
  int    tractorForce;
  int    tractorPushForce;
  int    tractorMaxBeams;
  int    tractorSpread;
  int    amt_beams;

  double grabbed_old_ship_angle, grabbed_old_enemy_angle;
  double old_angle;

  SpaceLocation *spacePlanet;

  public:
  ChoraliExtractor(Vector2 opos, double shipAngle,
    ShipData *shipData, unsigned int code);

  virtual int activate_weapon();
  virtual int activate_special();
  virtual void calculate();
  virtual int canCollide(SpaceObject *other);
  virtual void animate(Frame *space);
  virtual void inflict_damage(SpaceObject *other);

  virtual void calculate_turn_left();
  virtual void calculate_turn_right();
  virtual void materialize();
  virtual void death();
  Asteroid *asteroid_center;

};


ChoraliExtractor::ChoraliExtractor(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
	:
	Ship(opos, shipAngle, shipData, code)

{
  weaponFrames = get_config_int("Weapon", "Frames", 0);
  drillFrames  = 0;
  weaponDamage   = get_config_int("Weapon", "Damage", 0);

  AsteroidMissileRange    = scale_range(get_config_float("AsteroidMissile", "Range", 0));
  AsteroidMissileVelocity = scale_velocity(get_config_float("AsteroidMissile", "Velocity", 0));
  AsteroidMissileDamage   = get_config_int("AsteroidMissile", "Damage", 0);
  AsteroidMissileArmour   = get_config_int("AsteroidMissile", "Armour", 0);
  AsteroidMissileTurnRate = scale_turning(get_config_float("AsteroidMissile", "TurnRate", 0));

  specialForce = scale_velocity(get_config_float("Special", "Force", 0));

  //  tractorColor    = get_config_int("Tractor", "Color", 0);
  tractorR        = get_config_int("Tractor", "MaxColorR", 0);
  tractorG        = get_config_int("Tractor", "MaxColorG", 0);
  tractorB        = get_config_int("Tractor", "MaxColorB", 0);
  tractorRmin     = get_config_int("Tractor", "MinColorR", 0);
  tractorGmin     = get_config_int("Tractor", "MinColorG", 0);
  tractorBmin     = get_config_int("Tractor", "MinColorB", 0);

  tractorRange    = scale_range(get_config_float("Tractor", "Range", 0));
  tractorRate     = get_config_int("Tractor", "Rate", 0);
  tractorDamage   = get_config_int("Tractor", "Damage", 0);
  tractorForce    = get_config_int("Tractor", "PullForce", 0);
  tractorPushForce    = get_config_int("Tractor", "PushForce", 0);
  tractorMaxBeams = get_config_int("Tractor", "MaxBeams", 0);
  tractorSpread = get_config_int("Tractor", "Spread", 0);
  color_case='0';


  count           = 0;
  latched         = FALSE;
  grabbed         = NULL;
  spacePlanet     = NULL;

  asteroid_center = 0;

}

void ChoraliExtractor::death()
{
	 
  Ship::death();

  // dangerous, this is a memory leak:
  //game->remove(asteroid_center);
	if (asteroid_center && asteroid_center->exists())
		asteroid_center->die();
}
void ChoraliExtractor::materialize()
{
	 
	Ship::materialize();

	
	if (asteroid_center)
	{
		throw("asteroid center already present.");
	}
	asteroid_center = new AsteroidCenter(this, this->normal_pos());
	add(asteroid_center);
}

void ChoraliExtractor::calculate_turn_left()
{
	 
  Ship::calculate_turn_left();
}

void ChoraliExtractor::calculate_turn_right()
{
	 
  Ship::calculate_turn_right();
}

int ChoraliExtractor::activate_weapon()
{
	 

	if (drillFrames > 0)
    {
		count=0;
		
		if (grabbed == NULL)
		{
			drillFrames=0;	  
			return(TRUE);
		}
		else if( !(grabbed->isAsteroid()) )
		{
			drillFrames=0;
			return(TRUE);
		}
		else
		{
			grabbed->die();
			add(new AsteroidMissile(0.0, (size.y * 1.0), angle, AsteroidMissileVelocity,
				AsteroidMissileDamage, AsteroidMissileRange, AsteroidMissileArmour,
				AsteroidMissileTurnRate, this, meleedata.asteroidSprite, 1,
				tractorR, tractorRmin, tractorG, tractorGmin, tractorB, tractorBmin ));
			return(TRUE);
		}
    }
	else
    {
		drillFrames = weaponFrames;
		return(TRUE);
    }
	
}

int ChoraliExtractor::activate_special()
{
	 
	
	if(this->nearest_planet() != NULL)
    {
		spacePlanet = (SpaceLocation *) this->nearest_planet();
    }
	
	if (spacePlanet != NULL)
    {
		if ((grabbed != NULL) && (grabbed->mass > 0))
		{
			if (grabbed->mass + mass > 0)
			{
				// a bit lame to accelerate towards the planet. It's nicer if you move to the planet yourself.
				//grabbed->accelerate(this, grabbed->trajectory_angle(spacePlanet), specialForce / (grabbed->mass + mass), MAX_SPEED);
				//accelerate(this, trajectory_angle(spacePlanet), specialForce / (grabbed->mass + mass), MAX_SPEED);

				grabbed->set_vel ( vel );

				double a = angle;//trajectory_angle(grabbed);
				double dv = specialForce / (grabbed->mass + mass);

				grabbed->accelerate(this, a, dv * grabbed->mass, MAX_SPEED);
				accelerate(this, a+PI, dv * mass, MAX_SPEED);

				latched = FALSE;
				grabbed = NULL;
				drillFrames = 0;
			}
			
		}
		else
		{
			if (mass > 0)
				this->accelerate(this, trajectory_angle(spacePlanet), specialForce / mass, MAX_SPEED);
		}
    }
	
	return (true);
}

void ChoraliExtractor::calculate()
{
	 

	if(drillFrames > 0) 
	{
		if(grabbed == NULL)
		{
			amt_beams = random(tractorMaxBeams) + 1;
			
			for(int i=0;i<amt_beams;i++)
			{
				int r = random(tractorSpread);
				add(new ChoraliTractorBeam(this, (double)(angle+((r - (tractorSpread/2))*ANGLE_RATIO) ),
					makecol((random(tractorR-tractorRmin+1))+tractorRmin, (random(tractorG-tractorGmin+1))+tractorGmin, (random(tractorB-tractorBmin+1))+tractorBmin), 
					tractorRange, tractorDamage, tractorRate, this, 0.0, 0.0, true, tractorForce, tractorPushForce));
				
			}
			if(count>=PI)
			{ count=0;
			}
			if(count==0)
			{
				play_sound2(data->sampleExtra[0]);
			}
			count++;
			
		}

		drillFrames-= frame_time;
		if ((drillFrames <= 0) && (!latched)) 
		{
			play_sound2(data->sampleWeapon[0]);
		}
	}
	else 
	{
		latched = FALSE;
		grabbed = NULL;
	}
	
	if (grabbed != NULL)
	{
		if (!(grabbed->exists()) || grabbed->damage_factor > 0)
		{
			// if it does not exist, or the target deals damage, then, release it again.
			// The latter is because of the following report:
			// "When a Chorhli grabs a guardian, it still hangs on if it goes into blazer form,
			// killing it almost instantly. A bit unfair, no. "
			
			latched = FALSE;
			grabbed = NULL;
		}
	}
	
	if (latched) 
	{
		damageFrameLeft-=frame_time;
		if (damageFrameLeft <=0) 
		{
			damageFrameLeft += damageFrameAmount;
			if (drillDamageLeft < drillDamagePerDamageFrame)
				damage(grabbed, drillDamageLeft);
			else 
			{
				damage(grabbed,drillDamagePerDamageFrame);
				drillDamageLeft -= drillDamagePerDamageFrame; 
			}
			
			
		}
		grabangle = (grabbed->get_angle() - grabshipangle) + grabangle;
		
		grabshipangle = grabbed->get_angle();
		if(grabbed->isShip())
		{
			//turn enemy when ChoraliExtractor turns
			//	   ((Ship*)grabbed)->turn_step -= (old_angle - angle);
			
			//limit enemy movement
			//((Ship*)grabbed)->nextkeys &= ~(keyflag::left | keyflag::right | keyflag::thrust);
			// easier: override enemy position (uhm... but that's done below) and enemy angle (here).
			grabbed->angle = grabbed_old_enemy_angle + (angle - grabbed_old_ship_angle);
			((Ship*)grabbed)->turn_step = 0;
			
			
			//twist the enemy
			if (grabbed->trajectory_angle(this) <= grabbed->get_angle() )
			{
				if ((grabbed->get_angle() - grabbed->trajectory_angle(this)) <= PI)
				{((Ship*)grabbed)->angle+=CH_TWIST_ANGLE*frame_time/25.0;
				}
				else
				{((Ship*)grabbed)->angle-=CH_TWIST_ANGLE*frame_time/25.0;
				}
			}
			else
			{
				if ((grabbed->trajectory_angle(this) - grabbed->get_angle()) <= PI)
				{((Ship*)grabbed)->angle-=CH_TWIST_ANGLE*frame_time/25.0;
				}
				else
				{((Ship*)grabbed)->angle+=CH_TWIST_ANGLE*frame_time/25.0;
				}	    
			}
			
		}
		
		grabbed->pos = this->normal_pos() - unit_vector(angle+PI) * grabdistance;
		grabbed->set_vel ( this->vel );
	}
	old_angle = angle;

	if (asteroid_center && !asteroid_center->exists())
	{
		asteroid_center = 0;
	}

	Ship::calculate();
}

int ChoraliExtractor::canCollide(SpaceObject *other)
{
	 
  if ((latched) && (grabbed!=NULL) && (grabbed->exists()))
    {
      if (grabbed == other)
	return (FALSE);
    }
  return (Ship::canCollide(other));
}
 
void ChoraliExtractor::animate(Frame *space)
{
	 
    sprite->animate( pos, sprite_index, space);
}


void ChoraliExtractor::inflict_damage(SpaceObject *other)
{

	 
	if (drillFrames > 0)
    {
		if (!latched)
		{
			if ((!(sameTeam(other))) && (other->isShip() || other->isAsteroid())) 
			{
				if((this->trajectory_angle(other) <= angle+45) && (this->trajectory_angle(other) >= angle-45))
				{
					latched=TRUE;
					grabbed= (SpaceObject *) other;
					grabbed->set_vel ( this->vel );
					grabangle= (trajectory_angle(other) );
					grabdistance = (distance(other) * 1.1);
					grabshipangle = (other->get_angle());
					grabbed_old_enemy_angle = other->get_angle();
					grabbed_old_ship_angle = angle;
					drillDamageLeft = weaponDamage;
					play_sound2(data->sampleExtra[1]);
					if ((drillFrames / frame_time)< weaponDamage) 
					{
						drillDamagePerDamageFrame = (weaponDamage/drillFrames)
							+ ((weaponDamage % drillFrames) > 0.00001);
						damageFrameLeft = 1;
						damageFrameAmount = 1;
					} 
					else 
					{
						damageFrameAmount = (drillFrames/weaponDamage);
						damageFrameLeft = damageFrameAmount;
						drillDamagePerDamageFrame = 0;
					}
				}
			}
		}
		else //if latched
		{
			
		}
    }
	Ship::inflict_damage(other);
}


REGISTER_SHIP(ChoraliExtractor)













