

#include "../ship.h"
#include "../melee/mview.h"
REGISTER_FILE

#include <stdio.h>

#include "../frame.h"


static void removefromtargetlist(SpaceObject *o);
static bool isintargetlist(SpaceObject *o);


class AyronBS;
class AutoGun;


// this is read separately from the layout file - to keep things organized ?
struct BigShipPartOrigInfo
{
	double	crewmax, crew, batt, dynamo, turning, thrusting;
	SpaceSprite	*spr_crewed, *spr_uncrewed;
};

class BigShipPartOrig : public SpaceObject
{

protected:


	Vector2 oldpos;
	Vector2 oldvel;

	AyronBS *owner;
	Vector2	relpos;

	SpaceSprite *sprite_uncrewed;

public:

	double	crewmax, crew, batt, dynamo, turning, thrusting;

	Vector2 relposrot;
	SpaceObject *collider;
	Vector2 Pcoll, Vcoll;

//	BigShipPartOrig(AyronBS *aowner, Vector2 orelpos,
//				double acrew, double abatt, double adynamo,
//				double aturning, double athrusting,
//				SpaceSprite *spr, SpaceSprite *spr_uncrewed);

	BigShipPartOrig(AyronBS *aowner, Vector2 orelpos, int otype, BigShipPartOrigInfo **info);
	

	virtual void animate(Frame *space);
	virtual void calculate();
	virtual int handle_damage(SpaceLocation *source, double normal, double direct=0);
	virtual void inflict_damage(SpaceObject *other);
	//virtual void collide(SpaceObject *other);
	virtual bool hascrew();
	virtual void recrew(int howmany);
	void syncpos();
};


class BigShipPartOrigDevice : public SpaceObject
{
protected:
	BigShipPartOrig *ownerpart;

public:
	BigShipPartOrigDevice(BigShipPartOrig *aownerpart, SpaceSprite *ospr);
	virtual void calculate();
	virtual void animate(Frame *space);
};


class AutoGun : public BigShipPartOrigDevice
{

	double shotrange, shotvel, shotdamage, shotarmour, shotdrain;
	double a_center, a_view, a_track;
	double shotbusy, shotperiod;
	SpaceSprite	*sprshot;

public:

	AutoGun(BigShipPartOrig *aowner, Vector2 orelpos, double centerangle, double moveangle,
		SpaceSprite *spr, SpaceSprite *osprshot);
	//virtual void animate(Frame *space);
	virtual void calculate();
//	virtual int handle_damage(SpaceLocation *source, double normal, double direct=0);
};

class AyronBS : public Ship
{
public:
	int		N, Nx, Ny, Ntypes;
	double	w;
	BigShipPartOrig **parts;
//	int **parts;

	int		nBSinfo;
	BigShipPartOrigInfo **BSinfo;

	double	recharge_rate_ref, accel_rate_ref, turn_rate_ref;

	double	weaponRange, weaponDamage;
	int		weaponColor;
	
	AyronBS(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);
	
	virtual void animate(Frame *space);
	virtual void calculate();
	virtual int handle_damage(SpaceLocation *source, double normal, double direct=0);

	// you can overload this to load a different type of ship-part into the AyronBS.
	virtual BigShipPartOrig *init_part(Vector2 orelpos, int otype, BigShipPartOrigInfo **info);

	virtual int activate_weapon();
	virtual int activate_special();

};


BigShipPartOrig *AyronBS::init_part(Vector2 orelpos, int otype, BigShipPartOrigInfo **info)
{

	if (otype > 0)
		return new BigShipPartOrig(this, orelpos, otype, info);
	else
		return 0;
}



AyronBS::AyronBS(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code)
:
Ship(opos, shipAngle, shipData, code | SpaceSprite::NO_AA)
{

	int i;
	int ix, iy;

	// read the layout
	FILE *inp;

	inp = fopen("ships/shpayrbs_layout.txt", "rt");
	if (!inp) {tw_error("couldn't find the AyronBS layout");}


	
	fscanf(inp, "%i\n", &nBSinfo);

	Ntypes = nBSinfo;

	BSinfo = new BigShipPartOrigInfo* [nBSinfo];

	for ( i = 0; i < nBSinfo; ++i )
	{
		BSinfo[i] = new BigShipPartOrigInfo();

		fscanf(inp, "%*10c %lf %lf %lf %lf %lf",
			 &BSinfo[i]->crew, &BSinfo[i]->batt, &BSinfo[i]->dynamo,
			 &BSinfo[i]->turning, &BSinfo[i]->thrusting);

		BSinfo[i]->spr_crewed = data->more_sprites[i];
		BSinfo[i]->spr_uncrewed = data->more_sprites[i + Ntypes];
	}

	fscanf(inp, "%i %i\n", &Nx, &Ny);
	N = Nx * Ny;

//	parts = new (BigShipPartOrig*) [N];
	parts = new BigShipPartOrig* [N];


	int *itype, *iweapon;
	double *anglecenter, *moveangle;

	itype = new int [N];
	iweapon = new int [N];
	anglecenter = new double [N];
	moveangle = new double [N];

	// the ship hull sprites
	for ( i = 0; i < N; ++i )
	{
		fscanf(inp, "%i",
			&itype[i]);
	}

	// the ship weapons
	for ( i = 0; i < N; ++i )
	{
		fscanf(inp, "%i %lf %lf", &iweapon[i], &anglecenter[i], &moveangle[i]);
		anglecenter[i] *= PI / 180.0;
		moveangle[i] *= PI / 180.0;
	}

	fclose(inp);


	// the dimension of a single block of the ship (on a regular grid).
	// note that each block should be square, and of each grid-block is
	// of the same size.
	w = data->more_sprites[0]->size(0).x;

	// the orig picture has 0.5 extra room to allow for rotation; this extra room
	// is empty.
	w /= 1.5;

	// initialize the ship parts.
	for ( iy = 0; iy < Ny; ++iy )
	{
		for ( ix = 0; ix < Nx; ++ix )
		{
			int k;
			k = iy*Nx + ix;

			Vector2 relpos;

			relpos = w * Vector2(0.5*(Ny-1) - iy, ix - 0.5*(Nx-1));
			
			parts[k] = init_part(relpos, itype[k], BSinfo);
			if (parts[k])
				physics->add(parts[k]);


			//	AutoGun(AyronBS *aowner, Vector2 orelpos, int otype, SpaceSprite *spr);
			if (iweapon[k] == 1)
				game->add(new AutoGun(parts[k], relpos,
				anglecenter[k], moveangle[k],
				data->spriteWeapon, data->spriteWeaponExplosion));
		}
	}

	delete itype;
	delete iweapon;
	delete anglecenter;
	delete moveangle;

	for ( i = 0; i < nBSinfo; ++i )
		delete BSinfo[i];
	delete BSinfo;


	weaponColor  = get_config_int("Weapon", "Color", 0);
	weaponRange  = scale_range(get_config_float("Weapon", "Range", 0));
	weaponDamage = get_config_int("Weapon", "Damage", 0);


	recharge_rate_ref = recharge_rate;
	turn_rate_ref = turn_rate;
	accel_rate_ref = accel_rate;


	// remove this from the physics interaction
	//mass = 0; this value is needed by the ship parts
	collide_flag_anyone = 0;
	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;
	attributes |= ATTRIB_UNDETECTABLE;

	// also, add the ship's parts to the physics target list
//	for (i = 0; i < N; ++i )
//		if (parts[i])
//			game->add_target(parts[i]);
	// update: no need to this here anymore, it's done in the part-constructor.
}


void AyronBS::calculate()
{
	int i;

	// make sure the "ship" is not a real target
	if (isintargetlist(this))
		removefromtargetlist(this);

	// check if any of the ship parts has collided.
	// if one has collided, adjust position and velocity to keep away from the
	// collided object.

	bool updateneeded;

	updateneeded = false;

	for ( i = 0; i < N; ++i )
	{
		if (!parts[i])
			continue;

		SpaceObject *o;
		o = parts[i]->collider;

		if (o && !sameShip(o))
		{
			
			if (parts[i]->pos != parts[i]->Pcoll || parts[i]->vel != parts[i]->Vcoll)
			{
				pos = parts[i]->pos - parts[i]->relposrot;
				vel = parts[i]->vel;
			}

			updateneeded = true;
		}

		if (o)
			parts[i]->collider = 0;

	}

	if (updateneeded)
	{
		for ( i = 0; i < N; ++i )
		{
			if (!parts[i])
				continue;

			parts[i]->syncpos();
		}
	}




	// re-calculate the total ship stats from all of its little parts

	crew = 0;
	batt_max = 0;
	recharge_rate = 0;
	turn_rate = 0;
	accel_rate = 0;

	double Nrecharge = 0;

	for ( i = 0; i < N; ++i )
	{
		if (!parts[i])
			continue;

		if (!parts[i]->exists())
		{
			parts[i] = 0;
			continue;
		}

		// NOTE:
		// this should not occur.
		// the ship parts should always be "there",
		// except that they'll go inert (and grey) if they're "dead"
		// (unless they were never added in the first place of course)

		// add the stats of this part to the total stats
		if (parts[i]->hascrew())
		{
			crew += parts[i]->crew;
			batt_max += parts[i]->batt;
			//recharge_rate += parts[i]->dynamo * recharge_rate_ref;
			Nrecharge += parts[i]->dynamo;
			turn_rate += parts[i]->turning * turn_rate_ref;
			accel_rate += parts[i]->thrusting * accel_rate_ref;
		}
	}

	// this is the delay time between recharges.
	if (Nrecharge > 0)
		recharge_rate = recharge_rate_ref / Nrecharge;

	// detect your death ... weeeeeh :()
	if (!crew)
	{
		state = 0;
		return;
	}

	Ship::calculate();

}

void AyronBS::animate(Frame *space)
{
	return;
}


int AyronBS::handle_damage(SpaceLocation *source, double normal, double direct)
{
	// nothing: all is handled by the ship parts ?
	return Ship::handle_damage(source, normal, direct);
}





//BigShipPartOrig::BigShipPartOrig(AyronBS *aowner, Vector2 orelpos,
//				double acrew, double abatt, double adynamo,
//				double aturning, double athrusting,
//				SpaceSprite *spr, SpaceSprite *spr_uncrewed)

BigShipPartOrig::BigShipPartOrig(AyronBS *aowner, Vector2 orelpos, int otype, BigShipPartOrigInfo **info)
:
SpaceObject(aowner, 0, 0, info[otype-1]->spr_crewed)
{
	owner = aowner;
	relpos = orelpos;

	layer = LAYER_SHIPS;
	set_depth(DEPTH_SHIPS-0.1);		// the ship should come first in calculations.
	mass = owner->mass;

	oldpos = pos;
	oldvel = vel;

	crew      = info[otype-1]->crew;
	crewmax   = info[otype-1]->crew;
	batt      = info[otype-1]->batt;
	dynamo    = info[otype-1]->dynamo;
	turning   = info[otype-1]->turning;
	thrusting = info[otype-1]->thrusting;

	sprite_uncrewed = info[otype-1]->spr_uncrewed;

	relposrot = rotate(relpos, angle + owner->turn_step);

	collide_flag_sameship = 0;
	collide_flag_sameteam = ALL_LAYERS;
	collide_flag_anyone = ALL_LAYERS;

	// check if you've collided with something.
	collider = 0;

	game->add_target(this);
}



// this is in a separate routine, so that it can be called (again) by
// the owner if necessary (collision)

void BigShipPartOrig::syncpos()
{
	// maintain (relative) position wrt the ship
	angle = owner->angle;	// this is the discrete angle (64 values).
	sprite_index = get_index(angle);

	// for positioning, use the accurate angle.
	oldpos = pos;

	relposrot = rotate(relpos, angle + owner->turn_step);
	pos = owner->pos + relposrot;

	// the overall ship velocity
	oldvel = vel;
	vel = owner->vel;
}

void BigShipPartOrig::calculate()
{
	if ( !(owner && owner->exists()) )
	{
		owner = 0;
		state = 0;
		return;
	}

	// reset this here.
	// NO: this must only be reset by the owner, so that you're sure he's seen it.
	//hascollided = false;

	if (!collider)
		syncpos();
	// otherwise, you shouldn't touch the current ship-part settings,
	// instead let the owner handle this case. The vel/ pos are needed
	// to update the ship and all the other parts velocities ...

	/*
	// rotational velocity
	double angrate;

	angrate = 0;
	if (owner->turn_left)
		angrate = -owner->turn_rate;
	if (owner->turn_right)
		angrate = owner->turn_rate;

	double v;
	v = relpos.length() * angrate;

	Vector2 norm;
	norm = Vector2(-relposrot.y, relposrot.x);
	
	if (norm.length() > 0)
		vel += v * unit_vector(norm);
	*/


	SpaceObject::calculate();

}


int BigShipPartOrig::handle_damage(SpaceLocation *source, double normal, double direct)
{
	// transmit damage to the ship owner ...
	//return owner->handle_damage(source, normal, direct);
	// no ...
	// handle damage locally. The big ship can test all parts to calculate its global
	// parameters (like total crew left, total thrust, etc)...

	if (!hascrew())
		return 0;

	double t;
	t = normal + direct;
	crew -= t;

	if (crew <= 0)
	{
		t = -crew;

		// set crew to 0 (false)
		crew = 0;

		// make this ship part inert.
		// (thus exposing other, perhaps more vulnerable, parts of the ship)
		collide_flag_anyone = 0;
		collide_flag_sameship = 0;
		collide_flag_sameteam = 0;
		mass = 0;
		attributes |= ATTRIB_UNDETECTABLE;

		// as soon as this is done, it won't have to handle damage anymore ?

		// remove this thing from the game target list.
		removefromtargetlist(this);

	} else
		t = 0;	// all damage absorbed.

	//return normal+direct - t;	// returns true or false I guess...
	return 1;
}



void BigShipPartOrig::inflict_damage(SpaceObject *other)
{
	SpaceObject::inflict_damage(other);

	// this routine is called because there's a collision going on. Store
	// the current position and velocity, which are affected by the collision.

	Pcoll = normal_pos();
	Vcoll = vel;
	collider = other;
}




AutoGun::AutoGun(BigShipPartOrig *aowner, Vector2 orelpos, double centerangle, double moveangle,
				 SpaceSprite *spr, SpaceSprite *osprshot)
:
BigShipPartOrigDevice(aowner, spr)
{
	sprshot = osprshot;

	a_center = centerangle;
	a_view = moveangle;

	shotvel = 1.0;
	shotdamage = 1;
	shotrange = 1000;
	shotarmour = 1;
	shotdrain = 1;
	shotperiod = 0.5;	// reload time in seconds

	a_track = 0;
	shotbusy = 0;

}

//Shot::Shot(SpaceLocation *creator, Vector2 rpos, double oangle, double ov, 
//	double odamage, double orange, double oarmour, SpaceLocation *opos, 
//	SpaceSprite *osprite, double relativity) 

void AutoGun::calculate()
{
	BigShipPartOrigDevice::calculate();

	if (!(ownerpart && ownerpart->hascrew()))
		return;

	// find the closest target
	Query a;
	SpaceObject *oclosest;
	double R;

	oclosest = 0;
	R = 1E99;

	double da_track;
	da_track = -a_track;	// forces to go back into default position

	for (a.begin(this, OBJECT_LAYERS, 0.75*shotrange); a.current != 0; a.next())
	{
		SpaceObject *o;
		o = a.currento;

		if (!o->isObject())
			continue;

		if (o->sameShip(this) || o->sameTeam(this))
			continue;

		if (o->isPlanet())
			continue;

		if (o->isInvisible())
			continue;


		double L = 0;
		L = distance(o);

		if (!oclosest || L < R)
		{
			// this is a possible candidate but, is it also within the view angle ?

			double a, b, da;
			a = trajectory_angle(o);
			// check relative to the gun's neutral position
			b = ownerpart->get_angle() + a_center;

			da = a - b;
			while (da < -PI)	da += PI2;
			while (da >  PI)	da -= PI2;

			if (da > -a_view && da < a_view)
			{
				// okay! it's close, within dist-range, and it's within angle-range
				oclosest = o;
				R = L;

				// required movement is relative to the gun's current position
				// note that at this moment, you cannot use the value of "angle"
				// which is overridden by the call to BigShipPartOrig::calculate; so,
				// use "b" instead.
				b += a_track;
				da_track = a - b;
				while (da_track < -PI)	da_track += PI2;
				while (da_track >  PI)	da_track -= PI2;
			}
				
		}

	}


	// track the target (or go back to neutral position)
	// max tracking speed per second (0.1 circles / second).
	// this tracking angle is relative to the neutral angle of the gun.

	double da;
	da = 100 * PI2 * frame_time * 1E-3;
	if (da_track > da)
		da_track = da;
	if (da_track < -da)
		da_track = -da;

	a_track += da_track;

	if (a_track < -a_view)
		a_track = -a_view;
	if (a_track > a_view)
		a_track = a_view;

	// absolute angle of the gun
	angle = ownerpart->get_angle() + a_center + a_track;
	sprite_index = get_index(angle);

	// fire the weapon ?
	if (oclosest && ownerpart->ship->batt > shotdrain && !shotbusy)
	{
		
		Shot *s;
		s = new Shot(this, Vector2(0,0), angle, shotvel,
		shotdamage, shotrange, shotarmour, this,
		sprshot, 0);

		s->set_depth(DEPTH_SHIPS + 0.1);
		game->add( s );

		shotbusy = shotperiod;

		ownerpart->ship->handle_fuel_sap(this, shotdrain);
	}


	if (shotbusy > 0)
		shotbusy -= frame_time * 1E-3;
	else
		shotbusy = 0;

}






void BigShipPartOrig::animate(Frame *space)
{
	SpaceSprite *spr;

	if (!hascrew())
		spr = sprite_uncrewed;
	else
		spr = sprite;

	// the aa_stretch_blit scales down too much (dunno why).
	// use the usual stretch_blit instead ?

	Vector2 P, S, W;
	
	S = spr->size(sprite_index);
	P = corner(pos, S );
	W = S * space_zoom;

	BITMAP *b;
	b = spr->get_bitmap(sprite_index);

	masked_stretch_blit(b, space->surface, 0, 0, b->w, b->h, P.x, P.y, W.x, W.y);
	space->add_box(P.x, P.y, W.x, W.y);
}




int AyronBS::activate_weapon()
{
	double dx, dy;

	dx = 75;
	dy = Ny/2 * w - 25;

	// a big laser ?
	if (parts[0] && parts[0]->hascrew())
		add(new Laser(this, angle,
			pallete_color[weaponColor], weaponRange, weaponDamage, weapon_rate,
			this, Vector2(-dx, dy), true));

	if (parts[Nx-1] && parts[Nx-1]->hascrew())
		add(new Laser(this, angle,
			pallete_color[weaponColor], weaponRange, weaponDamage, weapon_rate,
			this, Vector2(dx, dy), true));

	return true;
}


int AyronBS::activate_special()
{
	// add to the crew of one of the damaged compartments.

	// first, check how many are "damaged"
	int i, Ndead;

	Ndead = 0;
	for ( i = 0; i < N; ++i )
		if (parts[i] && parts[i]->crew < parts[i]->crewmax)
			++Ndead;

	if (!Ndead)
		return false;

	// pick a random one from the dead ones.
	int k;
	
	k = random(Ndead) + 1;

	for ( i = 0; i < N; ++i )
	{
		if (parts[i] && parts[i]->crew < parts[i]->crewmax)
		{
			--k;
			if (k <= 0)
				break;
		}
	}

	if (i < N)
	{
		parts[i]->recrew(1);
	}




	return true;
}


bool BigShipPartOrig::hascrew()
{
	return crew != 0;
}

void BigShipPartOrig::recrew(int howmany)
{
	if (crew == 0 && howmany > 0)
	{
		// restore physical interaction
		collide_flag_anyone = ALL_LAYERS;
		collide_flag_sameteam = ALL_LAYERS;

		mass = owner->mass;

		attributes &= ~ATTRIB_UNDETECTABLE;

	
		// add it back to the target list of the game
		if (!isintargetlist(this))
			game->add_target(this);
	}

	crew += howmany;
	if (crew > crewmax)
		crew = crewmax;
}



BigShipPartOrigDevice::BigShipPartOrigDevice(BigShipPartOrig *aownerpart, SpaceSprite *ospr)
:
SpaceObject(aownerpart, aownerpart->pos, aownerpart->angle, ospr)
{
	ownerpart = aownerpart;

	layer = LAYER_SHIPS;
	set_depth(DEPTH_SHIPS+0.01);	// for plotting?

	// such a device shouldn't block things - impacts and damage should be handled
	// by the ship-part that owns this device.
	
	collide_flag_anyone = 0;
	collide_flag_sameship = 0;
	collide_flag_sameteam = 0;
	mass = 0;

}


void BigShipPartOrigDevice::calculate()
{
	if ( !(ownerpart && ownerpart->exists()) )
	{
		ownerpart = 0;
		state = 0;
		return;
	}

	if (!ownerpart->hascrew())
		return;

	angle = ownerpart->angle;
	pos = ownerpart->pos;
	vel = ownerpart->vel;

	SpaceObject::calculate();
}

void BigShipPartOrigDevice::animate(Frame *space)
{
	if (!ownerpart->hascrew())
		return;

	SpaceObject::animate(space);
}


/*
// PROBLEM:
// this works only in half the cases, where YOU collide with something else;
// alternatively, something else can collide with you, in which case the
// result is not influenced by any of this below.
// in that case ...
// you'll need to hack into inflict_damage(other),
// since at least that routine is always called then ....

void BigShipPartOrig::collide(SpaceObject *other)
{
	if (sameShip(other))
		return;

	if (other->ship == owner)
		return;

	Vector2 P, V;

	P = normal_pos();
	V = vel;

	SpaceObject::collide(other);
	
	// check for a positional or velocity change

	if (pos != P || vel != V)
	{
		owner->pos = pos - relposrot;
		owner->vel = vel;
	}

}
*/



// perhaps this should be a Game routine...

bool isintargetlist(SpaceObject *o)
{
	int i;

	for (i = 0; i < game->num_targets; i += 1)
	{
		if (game->target[i] == o)
		{
			return true;
		}
	}

	return false;
}

void removefromtargetlist(SpaceObject *o)
{
	int i;

	for (i = 0; i < game->num_targets; i += 1)
	{
		if (game->target[i] == o)
		{
			game->num_targets -= 1;
			game->target[i] = game->target[game->num_targets];
			break;
		}
	}
}


REGISTER_SHIP(AyronBS)

