#include "../ship.h"
REGISTER_FILE



static const int maxshots = 8;

class FopVob : public Ship
{
	double	weaponRange;
	int		weaponDamage;
	int		weaponArmour;
	
	double	specialPeriod;
	double	specialR;

	int Nshots;
	double T0[maxshots];

	
public:
	FopVob(Vector2 opos, double shipAngle,
		ShipData *shipData, unsigned int code);

	virtual void calculate();
	virtual void animate(Frame *f);

protected:
	virtual int activate_weapon();

	Vector2 getP(int i);
	double  getV(int i);
	double  getY(int i);
};



FopVob::FopVob(Vector2 opos, double shipAngle,
				 ShipData *shipData, unsigned int code) 
				 :
Ship(opos, shipAngle, shipData, code) {
	
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	
	specialPeriod  = get_config_float("Special", "Periode", 1);
	specialR		= 50.0;	// oscillation wavelength
	
	Nshots = 0;
}

int FopVob::activate_weapon()
{
	// add shots to the queue
	if (Nshots < maxshots )
	{
		T0[Nshots] = physics->game_time * 1E-3;
		++Nshots;
		return TRUE;

	} else
		return FALSE;

}


// relative position:
double FopVob::getY(int i)
{
	double y, t;

	t = physics->game_time * 1E-3;

	y = specialR * sin(PI2 * (t - T0[i]) / specialPeriod);

	return y;
}

Vector2 FopVob::getP(int i)
{
	Vector2 P;
	//double y, t;

	//t = physics->game_time * 1E-3;

	//y = specialR * sin(PI2 * (t - T0[i]) / specialPeriod);

	P = getY(i) * unit_vector(angle);

	return P;
}

// relative velocity:
double FopVob::getV(int i)
{
	double V, t;
	t = physics->game_time * 1E-3;

	// the derivative of the getP equation; with a correction of 1E-3 cause
	// frame-time is in mseconds, while these equations measure in seconds.
	V = specialR * PI2*cos(PI2 * (t - T0[i]) / specialPeriod) * 1E-3;

	return V;
}

//Shot::Shot(SpaceLocation *creator, Vector2 rpos, double oangle, double ov, 
//	double odamage, double orange, double oarmour, SpaceLocation *opos, 
//	SpaceSprite *osprite, double relativity) 

void FopVob::calculate()
{
	Ship::calculate();

	if (!fire_special || special_low)
	{
		// if special isn't held, release shots that are in the queue
		while (Nshots > 0)
		{
			--Nshots;

			Vector2	P;
			double	V;

			V = getV(Nshots);
			//P = rotate(getP(Nshots), -PI/2);
			P = Vector2(0, getY(Nshots));

			// make real shots
			add(new Missile(this,
				P, angle, V, weaponDamage, weaponRange,
				weaponArmour, this, data->spriteWeapon, 1.0));
		}
	}
}


void FopVob::animate(Frame *f)
{
	// animate the ship, as usual:
	Ship::animate(f);

	// animate the queued shots:
	int i;
	for ( i = 0; i < Nshots; ++i )
	{
		Vector2 P;
		P = pos + getP(i);
		data->spriteWeapon->animate(P, sprite_index, f);
	}
}


REGISTER_SHIP(FopVob)
