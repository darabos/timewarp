/* Modified from Vivisector */

#include "../ship.h"
REGISTER_FILE

#include "../melee/mview.h"

class XXXManglerMine;

class XXXMangler : public Ship {

	int          weaponFrames;
  int          tentacleFrames;
  int          weaponDamage;
  int          damageFrameAmount;
  int          tentacleDamagePerDamageFrame;
  int          damageFrameLeft;
  int          tentacleDamageLeft;
  int          latched;
  Ship         *grabbed;
  double       grabangle;
  double       grabdistance;
  double       grabshipangle;
  int          weaponDartThrust;
  
  double       specialLaunch;
  double       specialRange;
  double       specialVelocity;
  int          specialDamage;
  int          specialArmour;
  int          specialArming;
  XXXManglerMine **weaponObject;

  int          BodyFrames;
  int          CurrBodyFrame;
  int          ToggleDirection;

	int					 normal_recharge;
	int          normal_rate;
	bool				 SeparateSpit;
	

  
  public:
  XXXMangler(Vector2 opos, double shipAngle,
		ShipData *shipData, unsigned int code);

  RGB crewPanelColor();

  virtual int activate_weapon();
  virtual int activate_special();
  virtual void calculate();
  virtual int canCollide(SpaceObject *other);
  virtual void animate(Frame *space);
  virtual void inflict_damage(SpaceObject *other);
  
  int numSeeds;
  int maxSeeds;
};

class XXXManglerMine : public AnimatedShot {

  double       missileVelocity;
  double       missileRange;
  int          missileDamage;
  int          missileArmour;
  int          mineArming;
  int          mineactive;
  double       grabangle;
  double       grabdistance;

  public:
  XXXManglerMine(Vector2 opos, double ov, double oangle, int odamage, int oarmour,
    XXXMangler *oship, SpaceSprite *osprite, int ofcount, int ofsize, double misv, double misr,
    int misd, int misa,int misaf);

  virtual void calculate();
};

class XXXWebMissile : public AnimatedShot {

  int webFrames;

  public:
  XXXWebMissile(Vector2 opos, double oangle, double ov,
  int odamage, double orange, int oarmour, SpaceObject *creator, SpaceLocation *oposloc,
  SpaceSprite *osprite, int ofcount, int ofsize);

  virtual void inflict_damage(SpaceObject *other);
};

class XXXWebEffect : public SpaceObject {
  Ship *ship;
  int   webframe;
  int   webframe_count;
  int   frame_step;
  int   frame_size;
  int   frame_count;

  public:
  XXXWebEffect(Ship *oship, SpaceSprite *osprite, int ofcount,
    int ofsize, int disableFrames);

  virtual void calculate();
};


XXXMangler::XXXMangler(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code) 
	:
  Ship(opos, shipAngle, shipData, code)

{
  weaponFrames     = get_config_int("Weapon", "Frames", 0);
  tentacleFrames   = 0;
  weaponDamage     = get_config_int("Weapon", "Damage", 0);
  weaponDartThrust = get_config_int("Weapon", "DartThrust",0);

  specialRange    = scale_range(get_config_float("Special", "Range", 0));
  specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
  specialDamage   = get_config_int("Special", "Damage", 0);

  specialArmour   = get_config_int("Special", "Armour", 0);
  specialArming   = get_config_int("Special","Arming",0);
  specialLaunch   = scale_velocity(get_config_int("Special","Launch",0));
  latched         = FALSE;
  grabbed         = NULL;

  BodyFrames				= weaponFrames;
  CurrBodyFrame			= 0;
  ToggleDirection		= FALSE;

	normal_recharge		= recharge_amount;
	normal_rate				= recharge_rate;
	SeparateSpit			= FALSE;

  numSeeds=0;
  maxSeeds=8;
  weaponObject = new XXXManglerMine*[maxSeeds];
  for (int i = 0; i < maxSeeds; i += 1) {
    weaponObject[i] = NULL;
		}

}

RGB XXXMangler::crewPanelColor()
{
	RGB c = {64,64,64};
	return c;
}

int XXXMangler::activate_weapon()
{ 
    if (!latched) accelerate(this, angle, weaponDartThrust, MAX_SPEED);
    if (tentacleFrames <= 0) tentacleFrames = weaponFrames;
		else return false;
		return true;
}

int XXXMangler::activate_special()
{
	if (numSeeds == maxSeeds) {
		weaponObject[0]->state = 0;
		numSeeds -= 1;
		for (int i = 0; i < numSeeds; i += 1) {
			weaponObject[i] = weaponObject[i + 1];
		}
		weaponObject[numSeeds] = NULL;
	}
	weaponObject[numSeeds] = new XXXManglerMine(Vector2(0.0, -0.5*get_size().y), specialLaunch, (angle + PI),
		specialDamage, specialArmour, this, data->spriteSpecial, 8, 60, specialVelocity,specialRange,
		specialDamage, specialArmour, specialArming);
	add(weaponObject[numSeeds]);
	numSeeds += 1;
	return(TRUE);
}

void XXXMangler::calculate()
{

   if(tentacleFrames > 0) {
		 
     tentacleFrames -= frame_time;
     if ((tentacleFrames <= 0) && (!latched)) {
       play_sound2(data->sampleWeapon[0]);
      }
    }
   else latched = FALSE;


   if (grabbed != NULL ) {

			// additional criteria here: as suggested by Tau

			 if (!grabbed ->exists() || !grabbed->canCollide(this)) {
				 latched = FALSE;
				 grabbed = NULL;
			 }

		 }


   if (latched) {
		  recharge_step=0;
			recharge_rate=999;
			recharge_amount=0;

      damageFrameLeft-=frame_time;
      if (damageFrameLeft <=0) {
        damageFrameLeft += damageFrameAmount;
        if (tentacleDamageLeft < tentacleDamagePerDamageFrame)
          damage (grabbed, tentacleDamageLeft);
        else {
          damage (grabbed, tentacleDamagePerDamageFrame);
          tentacleDamageLeft -= tentacleDamagePerDamageFrame; }
        }
      grabangle = grabbed->get_angle() - grabshipangle + grabangle;
			grabangle = normalize(grabangle,PI2);
      angle=grabangle;
      grabshipangle = normalize(grabbed->get_angle(),PI2);

      nextkeys &= ~(KEYFLAG_LEFT | KEYFLAG_RIGHT | KEYFLAG_THRUST | KEYFLAG_SPECIAL | KEYFLAG_FIRE);

//      x = grabbed->normal_x()- (cos(grabangle ) * grabdistance);
//      y = grabbed->normal_y()- (sin(grabangle ) * grabdistance);

	  pos = grabbed->normal_pos() - grabdistance * unit_vector(grabangle);
      
		 } else {
				recharge_rate=normal_rate;
				recharge_amount=normal_recharge;

				SeparateSpit=TRUE;

			 }

   int j = 0;

   for (int i = 0; i < numSeeds; i += 1) {
    weaponObject[i-j] = weaponObject[i];
    if (!weaponObject[i]->exists()) j += 1;
      if (j) weaponObject[i] = NULL;
    }
    numSeeds -= j;

   Ship::calculate();


}
int XXXMangler::canCollide(SpaceObject *other)
{
  if ((latched) && (grabbed!=NULL) && (grabbed->exists())) {
    if (grabbed == other)
      return (FALSE);
		}
  return (Ship::canCollide(other));
}
 
void XXXMangler::animate(Frame *space)
{
  BodyFrames -= frame_time * 5;

  if ((tentacleFrames > 0) && (latched)) {
    int RandFrame = rand() % 4;
    angle = normalize(ship->get_angle(),PI2);
//    double TentacleX = ship->normal_x() + (cos(angle ) * height() * .75);
//    double TentacleY = ship->normal_y() + (sin(angle ) * width()  * .75);

	Vector2 TentaclePos;
	TentaclePos = ship->normal_pos() + 0.75 * product(unit_vector(angle), get_size()); //(cos(angle ) * height() * .75);
    
    data->spriteWeapon->animate(TentaclePos , sprite_index + (64 * RandFrame), space);
    CurrBodyFrame = RandFrame;
    if (CurrBodyFrame == 0) ToggleDirection = FALSE;
    else ToggleDirection = TRUE;
    }
  else {
          if (BodyFrames <= 0)
          {
                  if (!( (CurrBodyFrame > 0) && (CurrBodyFrame < 3) ))
                     ToggleDirection = !ToggleDirection;
                
                  if (ToggleDirection) ++CurrBodyFrame;
                  else --CurrBodyFrame;

                  BodyFrames = weaponFrames;
          }
  }
  sprite->animate(pos, sprite_index  + (64 * CurrBodyFrame) , space);

}


void XXXMangler::inflict_damage(SpaceObject *other)
{
  if (tentacleFrames > 0)
    if (!latched)
      if ((!(sameTeam(other))) &&
        (other->isShip())) {
          latched=TRUE;

          grabbed= (Ship *) other;
          grabangle= (trajectory_angle(other) );
          grabdistance = (distance(other) * 1.075);
          grabshipangle = (other->get_angle());
          tentacleDamageLeft = weaponDamage;
          play_sound2(data->sampleExtra[1]);
          if ((tentacleFrames / frame_time)< weaponDamage) {
            tentacleDamagePerDamageFrame = (weaponDamage/tentacleFrames)
              + ((weaponDamage % tentacleFrames) > 0.00001);
            damageFrameLeft = 1;
            damageFrameAmount = 1;
          } else {
            damageFrameAmount = (tentacleFrames/weaponDamage);
            damageFrameLeft = damageFrameAmount;
            tentacleDamagePerDamageFrame = 1;
            }
    }
		if (SeparateSpit) {
				if (other != NULL)
					if (!sameTeam(other) && other->isShip())
						other->accelerate (other, angle, weaponDartThrust  / (other->mass / 20), MAX_SPEED);
				accelerate(this,angle+PI,weaponDartThrust  / mass,MAX_SPEED);
				SeparateSpit = false;
			}
  Ship::inflict_damage(other);
}

XXXManglerMine::XXXManglerMine(Vector2 opos, double ov, double oangle, int odamage,
  int oarmour,XXXMangler *oship, SpaceSprite *osprite, int ofcount, int ofsize, double misv,
  double misr, int misd, int misa,int misaf) :
  AnimatedShot(oship, opos, oangle, ov, odamage, -1.0, oarmour, oship,
    osprite, ofcount, ofsize)

{
  missileVelocity = misv;
  missileRange = misr;
  missileDamage =misd;
  missileArmour = misa;
  mineArming = misaf;
  mineactive = FALSE;
}

void XXXManglerMine::calculate()

{
  AnimatedShot::calculate();
  if (!mineactive) {
    mineArming -= frame_time;
    if (mineArming <= 0) {
      mineactive = TRUE;
      v = 0;
//      vx = 0;
//      vy = 0;
	  vel = 0;
      }
    }
  else {
    SpaceObject *o, *t = NULL;
    double oldrange = 999999;
    Query a;
    for (a.begin(this, bit(LAYER_SHIPS),(missileRange *.9));
           a.current; a.next()) {
		o = a.currento;
		if (!o->sameTeam(this) && (distance(o) < oldrange) && !(o->isAsteroid() || o->isPlanet())) {
			t = o;
			oldrange = distance(o);
			}
		}
    if (t) {
      add(new XXXWebMissile(Vector2(0, 0), trajectory_angle(t),
        missileVelocity,missileDamage,missileRange,missileArmour,this,
        this,data->spriteExtra,2,50));
      play_sound2(data->sampleExtra[0]);
      destroy();
    }
  }
}


XXXWebMissile::XXXWebMissile(Vector2 opos, double oangle, double ov,
  int odamage, double orange, int oarmour, SpaceObject *creator, SpaceLocation *oposloc,
  SpaceSprite *osprite, int ofcount, int ofsize) :
  AnimatedShot(creator, opos, oangle, ov, odamage, orange, oarmour,
  oposloc, osprite, ofcount, ofsize),
  webFrames(3500)
{
}

void XXXWebMissile::inflict_damage(SpaceObject *other) {
	if (other->isShip()) add(new XXXWebEffect(
			(Ship *)(other), data->spriteExtraExplosion, 4, 50, webFrames) );
        Shot::inflict_damage(other);
	state = 0;
	return;
	}

XXXWebEffect::XXXWebEffect(Ship *oship,
  SpaceSprite *osprite,
  int ofcount,
  int ofsize,
  int webFrames) 
	:
  SpaceObject(oship, oship->normal_pos(), 0.0, osprite),
  ship(oship),
  webframe(0),
  webframe_count(webFrames),
  frame_step(0),
  frame_size(ofsize),
  frame_count(ofcount)
{
	collide_flag_anyone = 0;
        layer = LAYER_EXPLOSIONS;
}

void XXXWebEffect::calculate() {

	frame_step+= frame_time;
	while (frame_step >= frame_size) {
		frame_step -= frame_size;
		sprite_index++;
		if(sprite_index == frame_count)
			sprite_index = 0;
		}
	if(!ship->exists()) {
		state = 0;
		return;
		}


//	x = ship->normal_x();
//	y = ship->normal_y();
	pos = ship->normal_pos();

//        vx = ship->get_vx();
//        vy = ship->get_vy();
		vel = ship->get_vel();

        ship->accelerate(this, atan3(vel.y,vel.x) + PI, magnitude(vel), MAX_SPEED);


        ship->nextkeys &= ~(keyflag::left | keyflag::right | keyflag::thrust | keyflag::special);

        if (ship->batt > 0) ship->batt -= 1;

        webframe += frame_time;
        if (webframe >= webframe_count) state = 0;
		SpaceObject::calculate();
	}





REGISTER_SHIP(XXXMangler)