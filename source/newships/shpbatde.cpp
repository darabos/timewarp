#include "../ship.h"
REGISTER_FILE
#include "../melee/mview.h"

#include "../sc1ships.h"
#include "../frame.h"
#include "../util/aastr.h"


/*

  quirk: anti-grav field. Its strength depends on the battery.
  battery drains when the ship moves.

  weapon: plasma missiles

  special: gas cloud, each confuses one of the existing homing missiles, which
			have the Deviant as their target.

*/

class SpriteDrawListItem : public Presence
{
public:
	SpriteDrawListItem	*prev, *next;

	Vector2				pos;
	SpaceSprite			**sprites;
	int					sprite_index, sprite_array_index;

	SpriteDrawListItem(SpriteDrawListItem *s, SpaceSprite **osprites);

	void animate(Frame *frame);
	void init(int oindex, Vector2 opos);
};

class SpriteDrawList : public Presence
{
public:
	SpriteDrawListItem	*firstitem, *lastitem;
	SpaceObject			*mother;
	int					Nsprites;
	double				existtime, delaytime;

	SpriteDrawList(SpaceObject *creator, int N, SpaceSprite **osprites, double odelaytime);
	~SpriteDrawList();

	void calculate();

	void animate(Frame *frame);
};



class	BathaMissile;
class	BathaCloud;



class BathaDeviant : public Ship
{
	double weaponRange, weaponVelocity, weaponTurnRate, weaponTailDelay, weaponMass;
	int    weaponDamage, weaponArmour;

	double cloudLifeTime;

	double	gravforce, gravforce_default;

	double well_size;
	double whipfactor;
	double whipacc_max;

	double drain_travel_distance, drain_distance;

	int Nleakingsprites, Ntailsprites;
	SpaceSprite **leakingsprites, **tailsprites;

	public:

	int		CrewAsteroidHit, CrewShipHit, CrewPlanetHit;

	BathaDeviant(Vector2 opos, double angle, ShipData *data, unsigned int code);

	int activate_weapon();
	int activate_special();

	virtual void calculate();

	virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);

	virtual void inflict_damage(SpaceObject *other);

	void animate(Frame *frame);
};



class BathaMissile : public HomingMissile
{

	SpaceSprite *tailsprite;
	public:
	BathaMissile(SpaceLocation *creator, Vector2 rpos, 
		double oangle, double ov, double odamage, double orange, double oarmour, 
		double otrate, double omass, SpaceLocation *opos, SpaceSprite *osprite,
		SpaceObject *otarget, SpaceSprite **tailsprites, int Ntailsprites, double odelaytime);

	void animate(Frame *frame);
};


class BathaCloud : public SpaceObject
{
public:

	double existtime, lifetime, spr_changetime, spr_time;

	BathaCloud(SpaceLocation *creator, Vector2 opos, double oangle,
					SpaceSprite *osprite, double olifetime);

	virtual void calculate();
};




BathaDeviant::BathaDeviant(Vector2 opos, double angle, ShipData *data, unsigned int code) 
:
Ship(opos, angle, data, code) 
{
	weaponRange         = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity      = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage        = get_config_int("Weapon", "Damage", 0);
	weaponArmour        = get_config_int("Weapon", "Armour", 0);
	weaponTurnRate      = scale_turning(get_config_float("Weapon", "TurnRate", 0));
	weaponMass          = get_config_float("Weapon", "Mass", 0);
	weaponTailDelay     = get_config_float("Weapon", "TailDelay", 0);
	
	cloudLifeTime = get_config_float("Special", "LifeTime", 0);
	
	// properties of this ship are :
	
	gravforce_default = scale_acceleration(get_config_float("Quirk", "GravityForce", 0), 0);
	whipfactor = get_config_float("Special", "Quirk", 0);
	whipacc_max = get_config_float("Special", "Quirk", 0);
	well_size = scale_range(get_config_float("Quirk", "WellSize", 0));
	drain_distance = get_config_float("Quirk", "DrainDistance", 0);
	drain_travel_distance = 0;
	
	// how much crew do you lose when you hit an enemy ship or an asteroid?
	
	CrewAsteroidHit = get_config_int("Quirk", "DieAsteroid", 0);
	CrewShipHit = get_config_int("Quirk", "DieShip", 0);		// 10
	CrewPlanetHit = get_config_int("Quirk", "DiePlanet", 0);

	Ntailsprites = 8;
	tailsprites = &(data->more_sprites[0]);

	Nleakingsprites = 4;
	leakingsprites = &(data->more_sprites[Ntailsprites]);	
}




int BathaDeviant::activate_weapon()
{
	STACKTRACE
	BathaMissile *bm;
	bm = new BathaMissile(
		this, Vector2(0.0, 0.5*get_size().y), angle, weaponVelocity, weaponDamage,
		weaponRange, weaponArmour, weaponTurnRate, weaponMass, this, data->spriteWeapon,
		target, tailsprites, Ntailsprites, weaponTailDelay);
	game->add( bm );

	return TRUE;
}


int BathaDeviant::activate_special()
{
	STACKTRACE

	// come to an immediate halt.
//	vel = 0;


	// and also put up a single extra smoke cloud
	BathaCloud *bc;
	Vector2 D;

	double R = tw_random(50, 100);
	D = R * unit_vector(tw_random(PI2));

	bc = new BathaCloud(this, pos+D, angle, data->spriteSpecial, cloudLifeTime);
	game->add(bc);

	return(true);
}


void BathaDeviant :: calculate ()
{
	STACKTRACE


	Ship::calculate();
	
	// if the Batha flies happily around, its battery slowly drains
	drain_travel_distance += vel.magnitude() * frame_time;
	if ( drain_travel_distance > drain_distance )
	{
		if (batt >= 1)
		{
			batt -= 1;
			update_panel = 1;
		} else
			batt = 0;

		drain_travel_distance -= drain_distance;
	}

	// the anti-grav force depends on the battery,
	// but there is some minimum default level (to aid in escaping)

	gravforce = gravforce_default * (0.5 + 0.5 * batt / batt_max);

	
	int layers = bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL) +
		bit(LAYER_CBODIES);
	double passiveRange = 1000.0;	// outside this area, gravity doesn't do anything

	Query a;
	for (a.begin(this, layers, passiveRange); a.current; a.next())
	{
		SpaceObject *o = a.currento;
		if (!(o->isPlanet()) && o->mass != 0 && o != ship )
		{

			Vector2 Vd = min_delta(pos - o->pos, map_size);
			double R = magnitude(Vd);
			if (R < 1)
				continue;

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


		}
	}
	

	if (((game->game_time) & 128) != ((game->game_time+frame_time) & 128))
	{
		int i;
		i = tw_random(Nleakingsprites);

		Vector2 P;

		P.x = tw_random(-20,20);
		P.y = -20 - tw_random(20);

		int duration = 1000*fabs(1 - fabs(P.x/100));
		P = rotate(P, angle+PI + PI/2);

		game->add(new Animation(this, pos+P, leakingsprites[i], sprite_index, 1,
			duration, DEPTH_SHIPS, 1.0));
	}

}

int BathaDeviant::handle_damage(SpaceLocation *source, double normal, double direct)
{
	STACKTRACE

	// hitting a planet with its enormous mass is fatal
	// also creates an extra asteroid ;)
	if (source->isPlanet())
		normal += CrewPlanetHit;

	// hitting an asteroid deals damage
	if (source->isAsteroid())
		normal += CrewAsteroidHit;

	// hitting a ship causes major trauma as well
	if (source->isShip())
		normal += CrewShipHit;

	Ship::handle_damage(source, normal, direct);
	return normal+direct;
}

void BathaDeviant::inflict_damage(SpaceObject *other)
{
	STACKTRACE
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

//Animation::Animation(SpaceLocation *creator, Vector2 opos, 
//	SpaceSprite *osprite, int first_frame, int num_frames, int frame_length, 
//	double depth, double _scale) 

void BathaDeviant::animate(Frame *frame)
{
	STACKTRACE
	Ship::animate(frame);

}



BathaMissile::BathaMissile(SpaceLocation *creator, Vector2 rpos, 
	double oangle, double ov, double odamage, double orange, double oarmour, 
	double otrate, double omass, SpaceLocation *opos, SpaceSprite *osprite,
	SpaceObject *otarget, SpaceSprite **tailsprites, int Ntailsprites,
	double otaildelay) 
:
HomingMissile(creator, rpos, oangle, ov, odamage, orange, oarmour, otrate, opos, osprite, otarget)
{
	game->add(new SpriteDrawList(this, Ntailsprites, tailsprites, otaildelay) );
	mass = omass;
}

	


void BathaMissile::animate(Frame *frame)
{
	STACKTRACE
	// animate the sphere.
	HomingMissile::animate(frame);
}






SpriteDrawListItem::SpriteDrawListItem(SpriteDrawListItem *s, SpaceSprite **osprites)
{
	prev = s;
	next = 0;

	sprites = osprites;

	sprite_index = 0;
	sprite_array_index = 0;
}


void SpriteDrawListItem::init(int oindex, Vector2 opos)
{
	STACKTRACE
	sprite_index = oindex;
	pos = opos;

	sprite_array_index = 0;
}


void SpriteDrawListItem::animate(Frame *frame)
{
	STACKTRACE
	SpaceSprite *spr;
	spr = sprites[sprite_array_index];

	Vector2 C, S;
	S = spr->size();
	C = corner(pos, S);

	spr->draw(C, S*space_zoom, sprite_index, frame);
}


SpriteDrawList::SpriteDrawList(SpaceObject *creator, int N, SpaceSprite **osprites, double odelaytime)
{
	STACKTRACE
	mother = creator;

	SpriteDrawListItem *s;

	Nsprites = N;
	
	s = 0;

	int i;
	for ( i = 0; i < N; ++i )
	{
		s = new SpriteDrawListItem(s, osprites);
		s->init(mother->get_sprite_index(), mother->pos);

		if (i == 0)
			firstitem = s;
	}

	lastitem = s;

	delaytime = odelaytime;
	existtime = 0;
}

SpriteDrawList::~SpriteDrawList()
{
	SpriteDrawListItem *s, *t;

	t = 0;

	s = firstitem;
	while ( s != 0 )
	{
		t = s->next;
		delete s;

		s = t;
	}
}

void SpriteDrawList::calculate()
{
	STACKTRACE
	if ( !(mother && mother->exists()) )
	{
		mother = 0;
		state = 0;
		return;
	}


	existtime += frame_time * 1E-3;
	if ( existtime < delaytime )
		return;
	else
		existtime -= delaytime;

	SpriteDrawListItem *s;
//	Vector2 *P;

	for ( s = firstitem; s != 0; s = s->next)
		if (s->sprite_array_index < Nsprites-1)
			++ s->sprite_array_index;

	// reset the last in the list, bring it to the front and init its values:
	s = lastitem;
	lastitem = s->prev;
	lastitem->next = 0;

	s->next = firstitem;
	s->prev = 0;
	firstitem->prev = s;

	firstitem = s;

	s->init(mother->get_sprite_index(), mother->pos);
}


void SpriteDrawList::animate(Frame *frame)
{
	STACKTRACE
	SpriteDrawListItem *s;
	for ( s = lastitem; s != 0; s = s->prev)
		s->animate(frame);
}




BathaCloud::BathaCloud(SpaceLocation *creator, Vector2 opos, double oangle,
				SpaceSprite *osprite, double olifetime)
:
SpaceObject(creator, opos, oangle, osprite)
{
	layer = LAYER_SHOTS;
	vel = 0;

	lifetime = olifetime;
	existtime = 0;

	// check if one of the homing missiles has the creator as target - change
	// its target:

	int layers = bit(LAYER_SHOTS);
	double passiveRange = 1000.0;	// outside this area, don't do anything

	Query a;
	for (a.begin(this, layers, passiveRange); a.current; a.next())
	{
		SpaceObject *o = a.currento;
		if ( o->isShot() && ((Shot*)o)->isHomingMissile() && o->target == ship )
		{
			o->target = this;
			break;
		}
	}

	spr_changetime = 0.3;
	spr_time = 0;
	sprite_index = 0;
}

void BathaCloud::calculate()
{
	STACKTRACE
	SpaceObject::calculate();

	existtime += frame_time * 1E-3;
	if (existtime > lifetime)
	{
		state = 0;
		return;
	}

	spr_time += frame_time * 1E-3;
	if ( spr_time > spr_changetime )
	{
		spr_time -= spr_changetime;
		++sprite_index;
		if (sprite_index >= sprite->frames())
			sprite_index = 0;
	}
}



REGISTER_SHIP ( BathaDeviant )
