#include "../ship.h"
REGISTER_FILE
#include "../melee/mview.h"

#include "../sc1ships.h"
#include "../frame.h"


/*

  DESCRIPTION:
  Shape is based on quasars, which contain a black hole in the center of a gas disk.
  The quasars shoot jets of hot gas in two directions whenever enough mass enters into
  the disk.

  PRIMARY:
  Matter burners. Shoots forward and backward, it takes all battery power to activate.

  SPECIAL:
  The ship possesses a permanent anti-gravity well. Pushing special, disables the anti-
  gravity shield. This increases battery recharge rate, but leads to crew loss.

  Idea for the main: GeomanNL
  Idea for the special and the race: IamTheSongMan

  Ok ... the ships sucks right now... needs to be improved !!

  BETTER MAYBE :

  Burners, take all power. No recharge. Anti-Grav field disappears completely.

  Permanent anti-gravity property (anti-matter). "High" recharge rate.

  Special, disable anti-gravity; "Slow" recharge rate. Also, a short "bump" in
  the field: temporarily, a strong attraction? Makes the ship extremely maneouverable.
  If anti-grav is re-enabled, then there's a strong anti-bump.

  Special, weakens space, slows down anything in the neighbourhood. The closer to
  the ship, the greater the slow-down. Especially useful as defense against weapons.




  -----------------

Activating the main does 2 things: flames are turned on, and the 
anti-gravity field is turned off. The change in gravity is accompanied 
by a bump in the field power. Recharge is very slow as long as the 
flames are on; manoeuverability is increased. This should be the 
offensive mode.

Activating the special does 1 thing: this weakens space/ changes 
physics slightly, around the ship, slowing everything down (like 
in hyperspace). At the expense of manoeuverability. This should 
be an additional defensive mode.

*/


class	Flamer;
class	GravityShockWave;

class BathaDeviant : public Ship
{
	double weaponRange;
	double weaponVelocity;
	int    weaponDamage;
	int    weaponArmour;
	double weaponAccel;

	double	crewdietime, crewdietimeproceed;
	double	gravforce, grav_force_default;
	double	accel_rate_default;
	double	recharge_rate_default;
	double	speed_max_default;
	double	batt_max_default, batt_max_new;

	double well_size;
	double whipfactor;
	double whipacc_max;

	double slowdownfactor;
	
	GravityShockWave	*ShockWave;

	public:
	Flamer	*flamer1, *flamer2;
	int		weapon1, weapon2, flame_active;

	int		CrewAsteroidHit, CrewShipHit;

	BathaDeviant(Vector2 opos, double angle, ShipData *data, unsigned int code);

	int activate_weapon();
	int activate_special();

	virtual void calculate();

	virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);

	virtual void inflict_damage(SpaceObject *other);

};



class Flamer : public Missile {
	BathaDeviant	*commandship;
	int		iSide;
	double	animate_time, nextpictime;
	int		sprite_offset, Nanimatedflame;
	double	FlameAccel;

	public:
	Flamer(Vector2 opos, double oangle, double ov, int odamage,
			double orange, int oarmour, double oaccel, Ship *oship, SpaceSprite *osprite, int thisiSide,
			BathaDeviant *commandshipref);
	virtual void calculate();
	virtual void inflict_damage(SpaceObject *other);
	virtual int handle_damage(SpaceLocation *source, double normal, double direct);
	};


class GravityShockWave : public SpaceObject	// cannot hit anything .. it simply exists for a short period
{
public:

	Vector2		P, lastP;
	double		R, lastR;

	int			gravsign;

	double	waveradius, wavemaxradius, wavevel;

	GravityShockWave(SpaceLocation *creator, Vector2 opos, 
	double oangle, SpaceSprite *osprite);

	virtual void calculate();
	virtual void animate (Frame *frame);
};

/*
// return 0.0 to 1.0
double tw_drandom()
{
	return	double(tw_random()) / double(0x080000000);
}

// return -1.0 to 1.0
double tw_sdrandom()
{
	return	1.0 - 2.0 * double(tw_random()) / double(0x080000000);
}
*/




BathaDeviant::BathaDeviant(Vector2 opos, double angle, ShipData *data, unsigned int code) 
	:
	Ship(opos, angle, data, code) 
	{
	weaponRange         = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity      = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage        = get_config_int("Weapon", "Damage", 0);
	weaponArmour        = get_config_int("Weapon", "Armour", 0);

	weaponAccel = scale_velocity(get_config_float("Weapon", "Accelerate", 0));

	// properties of this ship are :

	accel_rate_default = accel_rate;
	speed_max_default = speed_max;

	recharge_amount = 1;
	recharge_rate_default = recharge_rate;

	crewdietime = get_config_float("Special", "CrewDieTime", 0);
	crewdietimeproceed = 0.0;

	grav_force_default = scale_acceleration(get_config_float("Special", "GravityForce", 0), 0);
	whipfactor = get_config_float("Special", "WhipFactor", 0);
	whipacc_max = get_config_float("Special", "WhipAccmax", 0);
	well_size = scale_range(get_config_float("Special", "WellSize", 0));

	batt_max_default = batt_max;
	batt_max_new = batt_max;

	flamer1 = 0;
	flamer2 = 0;
	weapon1 = 0;
	weapon2 = 0;

	// how much crew do you lose when you hit an enemy ship or and asteroid?

	CrewAsteroidHit = 1;	//  5
	CrewShipHit = 4;		// 10

	ShockWave = 0;
	flame_active = weapon1 | weapon2;
	}


int BathaDeviant::activate_weapon()
{
	weaponVelocity = 0.0;

	if ( weapon1 && weapon2 )
		return false;	// couldn't be used.

	if ( weapon1 == 0 )
	{
		weapon1 = 1;
		flamer1 = new Flamer(
				Vector2(0.0, 0.5*get_size().y), angle, weaponVelocity, weaponDamage, weaponRange,
				weaponArmour, weaponAccel, this, data->spriteWeapon, 1, this);
		game->add( flamer1 );
		
	}
	if ( weapon2 == 0 )
	{
		weapon2 = 1;
		flamer2 = new Flamer(
				Vector2(0.0, -0.5*get_size().y), angle+PI, weaponVelocity, weaponDamage, weaponRange,
				weaponArmour, weaponAccel, this, data->spriteWeapon, 2, this);
		game->add( flamer2 );
	}

	return(TRUE);
}


int BathaDeviant::activate_special() {

	return(true);
	}


void BathaDeviant :: calculate ()
{

	// check if the weapons exist
	weapon1 = flamer1 && flamer1->exists();
	weapon2 = flamer2 && flamer2->exists();

	int flame_prev_active = flame_active;
	flame_active = weapon1 || weapon2;

	Ship::calculate();

	if ( batt > batt_max_new )
		batt = batt_max_new;

	
	// a shockwave is generated if the gravity field is changed.
	if ( flame_active && !flame_prev_active )		// flames were activated !!
	{
		ShockWave = new GravityShockWave(this, pos, 0.0, this->data->spriteShip);
		ShockWave->gravsign = -1;
		game->add(ShockWave);
	}
	
	if ( !flame_active && flame_prev_active )		// both flames were de-activated !!
	{
		ShockWave = new GravityShockWave(this, pos, 0.0, this->data->spriteShip);
		ShockWave->gravsign =  1;
		game->add(ShockWave);
	}
	

	// apply the gravity force:
	if (!this->fire_special && !flame_active)	// DEFAULT MODE
	{
		gravforce = -grav_force_default;
		recharge_rate = recharge_rate_default;

		accel_rate = accel_rate_default;
		speed_max = speed_max_default;

	}
	
	if ( flame_active )		// OFFENSIVE MODE
	{
		gravforce = 0.0;
		recharge_rate = recharge_rate_default;

		// you also have increased acceleration power
		accel_rate = 5*accel_rate_default;
		speed_max = 2*speed_max_default;

		/* Well ... the ship is already pretty weak, and very quirky, so if you've crew dying as well is... too freaky.
		crewdietimeproceed += frame_time;
		if (crewdietimeproceed > crewdietime)
		{
			
			crewdietimeproceed -= crewdietime;
			crew -= 1;

			if ( crew <= 0 )
				state = 0;
		}
		*/

	}
	
	// regardless of whether the flames are activated or not:
	if ( this->fire_special )	// DEFENSIVE MODE
	{
		accel_rate = 0.5 * accel_rate_default;	// less maneouverable.
		speed_max = speed_max_default;

		double HalfTime = 1000.0 * 0.5;	// 0.5 second to half the speed.
		slowdownfactor = exp(-frame_time / HalfTime);	// is nearly 1

		recharge_rate = 0.5 * recharge_rate_default;
	} else {
		slowdownfactor = 1.0;
	}

	if ( flame_active )
	{
		accel_rate = accel_rate / 2;		// you lose half your (previous) thrust power !
		
		if ( weapon1 + weapon2 == 2 )
			batt_max_new = 2;	// you cannot recharge more than this as long as two flames are on
		else
			batt_max_new = batt_max_default/2;	// this is max battery if one of the flames are on.

		if ( batt > batt_max_new )
			this->handle_fuel_sap(this, batt - batt_max_new);

		//batt_max = batt_max_new;

	}
	else
	{
		accel_rate = accel_rate_default;
		batt_max_new = batt_max_default;
	}


	int layers = bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL) +
		bit(LAYER_CBODIES);
	double passiveRange = 1000.0;	// outside this area, gravity doesn't do anything

	Query a;
	for (a.begin(this, layers, passiveRange); a.current; a.next())
	{
		SpaceObject *o = a.currento;
		if (!(o->isPlanet()) && (o->mass != 0 || o->isShot()) && o != ship )
		{
			
			double Rv = magnitude(o->get_vel());	//sqrt(o->vx * o->vx  +  o->vy * o->vy);
			if (Rv == 0.0) Rv = 0.1;

			Vector2 Vd = min_delta(o->normal_pos(), normal_pos(), map_size);
			double R = magnitude(Vd);

			double bb;
			double Rscaled = R / well_size;

			if ( Rscaled > 1.0 )
				bb = 1.0 / (Rscaled * sqrt(Rscaled));	// outside the gravity well
			// not R*R gravity, but slightly less for better feel (R*root(R))
			// also adds an extra velocity increase.
			else
				bb = sin(Rscaled*0.5*PI);				// inside the gravity well
			// this sine function is stable, probably because it's smooth
			// at R=0 (has a derivative). It works good, much better than bb=const
			// or bb=Rscaled.
			
			
			bb *= frame_time;
			
			// accelerate towards (or away from) the source

			Vector2 Vacc = -(Vd/R) * gravforce*bb;
			
			o->vel += Vacc;
			this->vel -= Vacc;


			// the special introduces drag around the ship, depending on distance

			if ( R < 1000.0 )
			{
				o->vel *= slowdownfactor;
			}


			/*
			// as extra, when the gravity fields changes, the enemy experiences
			// a short overshoot .. a powerful gravity wave hits it ;)

			
			if ( ShockWave && ShockWave->exists() )
			{
				Vector2 Vd = min_delta(o->normal_pos(), ShockWave->normal_pos(), map_size);
				double R = magnitude(Vd);

				if ( fabs(R - ShockWave->waveradius) < 50.0 )	// the wavefront has a short action range
				{
					
					double vel_boost = 0.0025 * frame_time;
					o->vel += ShockWave->gravsign * (Vd/R) * vel_boost;
					
					
				}
			}
			*/
			
	


		}
	}


}

int BathaDeviant::handle_damage(SpaceLocation *source, double normal, double direct)
{

	// hitting an asteroid deals damage
	if (source->isAsteroid())
		crew -= CrewAsteroidHit;

	// hitting a planet with its enormous mass is fatal
	if (source->isPlanet())
		crew = 0;

	// hitting a ship causes major trauma as well
	if (source->isShip())
		crew -= CrewShipHit;

	Ship::handle_damage(source, normal, direct);
	return true;
}

void BathaDeviant::inflict_damage(SpaceObject *other)
{
	// the main body of the ship hits the enemy and does damage ....
	if (other->isShip())
	{
		damage_factor = CrewShipHit;
		SpaceObject::inflict_damage(other);
	}

	if (other->isAsteroid())
	{
		damage_factor = CrewAsteroidHit;
		SpaceObject::inflict_damage(other);
	}

}


// This is an intense flame with a short range.

Flamer::Flamer(Vector2 opos, double oangle, double ov,
						   int odamage, double orange, int oarmour, double oaccel, Ship *oship, SpaceSprite *osprite, int thisiSide,
							BathaDeviant *commandshipref) 
						   :
Missile(oship, opos, oangle, ov, odamage, orange, oarmour, oship,osprite) ,
commandship(commandshipref),
iSide(thisiSide),
FlameAccel(oaccel)
{
	explosionSprite     = data->spriteWeaponExplosion;
	explosionFrameCount = 20;
	explosionFrameSize  = 50;

	sprite_offset = 0;
	animate_time = 0;
	nextpictime = 250.0;
	Nanimatedflame = 4;

}

	


void Flamer::calculate()
{
	if ( !commandship || !ship || !commandship->exists() || !ship->exists() )
	{
		state = 0;
		return;
	}
	

	Missile::calculate();
	
	// fix the location and direction of the flame with respect to the ship.
	
	double D = ( commandship->get_size().y + get_size().y ) / 2.0;
	if ( iSide == 1 )
	{
		pos = commandship->pos + D * unit_vector(commandship->angle);
		angle = commandship->angle;
	} else {
		pos = commandship->pos - D * unit_vector(commandship->angle);
		angle = commandship->angle + PI;
	}
	
	sprite_index = get_index(angle, -0.5*PI);// - 16;
	sprite_index += sprite_offset * 64;

	animate_time += frame_time;
	if ( animate_time > nextpictime )
	{
		animate_time -= nextpictime;
		sprite_offset += 1;
		if ( sprite_offset > Nanimatedflame-1 )		// hard coded ?!?!?!
			sprite_offset = 0;
	}




	// the active weapon also has influence on the commandship by providing thrust:

	commandship->vel += FlameAccel*unit_vector(angle+PI);

	double v = magnitude(commandship->vel);

	if ( v > commandship->speed_max )
		commandship->vel *= commandship->speed_max / v;


}



int Flamer::handle_damage(SpaceLocation *source, double normal, double direct)
{
	
	int totdamage = normal + direct;

	if (source->isShip())
		totdamage = armour;

	armour -= totdamage;

	if ( armour <= 0 )
	{
		state = 0;
		return totdamage + armour;
	}

	return totdamage;
}



void Flamer::inflict_damage(SpaceObject *other)
{
	SpaceObject::inflict_damage(other);

	return;
}


GravityShockWave::GravityShockWave(SpaceLocation *creator, Vector2 opos, 
	double oangle, SpaceSprite *osprite)
: SpaceObject(creator, opos, angle, osprite)
{
	waveradius = 100.0;
	wavevel = 0.3;
	wavemaxradius = 500.0;

	layer = LAYER_SHIPS;
	set_depth(DEPTH_SHIPS);

	collide_flag_anyone = 0;
	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;

	P = corner(pos);
	R = waveradius * space_zoom;
}


void GravityShockWave::calculate()
{

	// increase radius of the circle.
	waveradius += wavevel * frame_time;

	if ( waveradius > wavemaxradius )	// the shockwave doesn't reach everywhere
	{
		state = 0;
		return;
	}

	SpaceLocation::calculate();

	if ( ship && ship->exists() )
		pos = ship->pos;

	lastP = P;
	P = corner(pos);

	lastR = R;
	R = waveradius * space_zoom;



	int layers = bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL) +
		bit(LAYER_CBODIES);
	double passiveRange = 1000.0;	// outside this area, gravity doesn't do anything

	Query a;
	for (a.begin(this, layers, passiveRange); a.current; a.next())
	{
		SpaceObject *o = a.currento;
		if (!(o->isPlanet()) && (o->mass != 0 || o->isShot()) && o != ship )
		{
			
			// as extra, when the gravity fields changes, the enemy experiences
			// a short overshoot .. a powerful gravity wave hits it ;)

			
			Vector2 Vd = min_delta(o->normal_pos(), normal_pos(), map_size);
			double R = magnitude(Vd);
			
			if ( fabs(R - waveradius) < 50.0 )	// the wavefront has a short action range
			{
				
				double vel_boost = 0.0025 * frame_time;
				o->vel += gravsign * (Vd/R) * vel_boost;
				
				
			}
			
	
		}
	}

}





void GravityShockWave::animate(Frame *frame)
{

	if (!state)
		return;



	int i = 255 - 200 * waveradius / wavemaxradius;
	int wavecol = makecol(i, i, i);

	// draw a circle

	circle(frame->surface, (int)P.x, (int)P.y, (int)R, wavecol);

	frame->full_redraw = TRUE;	// otherwise you've nasty drawing artifacts !
	// this tweak isn't neat, but well, doing it another way is difficult :(

}




REGISTER_SHIP ( BathaDeviant )
