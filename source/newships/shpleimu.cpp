

#include "../ship.h"
REGISTER_FILE

#include "../frame.h"


class LeiMule : public Ship
{
	double	weaponRange, weaponVelocity, weaponDamage, weaponArmour;
	double	specialRange, specialVelocity, specialDamage, specialArmour;
	
	bool shipmode;
	
public:
	LeiMule(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);
	
protected:
	
	virtual int activate_weapon();
	virtual int activate_special();
	
};




LeiMule::LeiMule(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code)
:
Ship(opos,  shipAngle, shipData, code)
{
	
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);

	specialRange    = scale_range(get_config_float("Special", "Range", 0));
	specialVelocity = scale_velocity(get_config_float("Special", "Velocity", 0));
	specialDamage   = get_config_int("Special", "Damage", 0);
	specialArmour   = get_config_int("Special", "Armour", 0);

}


int LeiMule::activate_weapon()
{	
	STACKTRACE
	

	Vector2 P0;
	double A0, R0, Aweapon;

	P0 = Vector2(0,0);
	R0 = 50;


	Aweapon = 0.025 * PI;

	A0 = 0.5 * PI + Aweapon;


	int i, N;

	// per wing
	N = 1;

	for ( i = -N; i <= N; ++i )
	{
		if (i == 0)
			continue;

		Vector2 rpos;

		rpos = P0 + (fabs(i*R0) / N) * unit_vector(A0*sign(i) - PI/2);

		double aoffs;
		aoffs = -i * Aweapon;

		add(new Shot(this, rpos, angle + aoffs,
			weaponVelocity, weaponDamage, weaponRange, weaponArmour,
			this, data->spriteWeapon, 0.0));
	}
	
	return TRUE;
}


int LeiMule::activate_special()
{
	STACKTRACE

	
	Vector2 P0;
	double A0, R0, Aweapon;

	P0 = Vector2(0,0);
	R0 = 50;


	Aweapon = 0.025 * PI;

	A0 = 0.5 * PI + Aweapon;


	int i, N;

	// per wing
	N = 2;

	for ( i = -N; i <= N; ++i )
	{
		if (i == 0)
			continue;

		Vector2 rpos;

		rpos = P0 + (fabs(i*R0) / N) * unit_vector(A0*sign(i) - PI/2);

		double aoffs;
		aoffs = -i * Aweapon + PI;

		add(new Shot(this, rpos, angle + aoffs,
			specialVelocity, specialDamage, specialRange, specialArmour,
			this, data->spriteSpecial, 0.0));
	}

	return TRUE;
}





REGISTER_SHIP(LeiMule)

