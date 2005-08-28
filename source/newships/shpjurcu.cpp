/* $Id$ */ 

#include "../ship.h"

REGISTER_FILE

class JurgathaCutter: public Ship
{
public:
IDENTITY(JurgathaCutter);
	int batt_counter;

	double weaponRange;
	double weaponSpeed;
	double weaponDamage;
	int weaponColor;

	double specialDamage;
	int specialFrames;

public:
	JurgathaCutter(Vector2 opos, double shipAngle, ShipData *shipData, unsigned int code);
protected:
	virtual void calculate();
	virtual int activate_weapon();
	virtual int activate_special();
};

class JurgathaPortal : public Animation
{
public:
IDENTITY(JurgathaPortal);
public:
	JurgathaPortal(SpaceLocation *creator, Vector2 opos, double damage, SpaceSprite *osprite, int ofct, int ofsz);
	virtual void calculate();
};

JurgathaCutter::JurgathaCutter(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code) :
	Ship(opos, shipAngle, shipData, code)
{
	batt_counter = 0;

	weaponRange		= scale_range(get_config_float("Weapon", "Range", 0));
	weaponSpeed		= get_config_float("Weapon", "Speed", 1);
	weaponDamage	= get_config_float("Weapon", "Damage", 0);
	weaponColor		= get_config_int("Weapon", "Color", 2);

	specialDamage   = get_config_float("Special", "Damage", 0);
	specialFrames   = scale_frames(get_config_int("Special", "Frames", 0));
}


void JurgathaCutter::calculate()
{
	if(thrust)
	{
		if(batt_counter==3)
		{
			batt_counter = 0;
			batt>batt_max?batt=batt_max:batt++;
		}
		else batt_counter++;
	}
	else if(!thrust)
	{
		if (batt_counter>=2)
		{
			batt_counter = 0;
			batt<0?batt=0:batt--;
		}
		else batt_counter++;
	}

	Ship::calculate();
}

int JurgathaCutter::activate_weapon()
{
	int random_number = tw_random(10);

    game->add(new Laser(this, angle, pallete_color[weaponColor], weaponRange,
      weaponDamage, weapon_rate, this, Vector2(-(double)(random_number), 2.0), true));

    random_number = random(10);

    game->add(new Laser(this, angle, pallete_color[weaponColor], weaponRange,
      weaponDamage, weapon_rate, this, Vector2((double)(random_number), 2.0), true));

	return(TRUE);
}


int JurgathaCutter::activate_special()
{
	game->add(new JurgathaPortal(this, pos, specialDamage, data->spriteSpecial,
		data->spriteSpecial->frames(), specialFrames));

	return(TRUE);
}

JurgathaPortal::JurgathaPortal(SpaceLocation *creator, Vector2 opos, double fdamage, SpaceSprite *osprite, int ofct, int ofsz) :
	Animation(creator, opos, osprite, 0, ofct, ofsz, LAYER_HOTSPOTS)
{
	damage_factor = fdamage;

	// normally, an animation doesn't take part in collisions, so that this damage by itself won't do much
}

void JurgathaPortal::calculate()
{
	Animation::calculate();

	Query a;

	// the animation needs to check if it inflicts damage on its own.
	for (a.begin(this, bit(LAYER_SHIPS) + bit(LAYER_SHOTS) + bit(LAYER_SPECIAL),
			sprite->width()/2, QUERY_OBJECT); a.current; a.next())
	{
		if (!a.currento->sameTeam(this) && !(a.currento->isAsteroid() || a.currento->isPlanet() ) )
		{
			//a.currento->directDamage++;
			// inflict damage per second...
			a.currento->handle_damage(this, 0, damage_factor * frame_time / 1E3);
		}
	}
}

REGISTER_SHIP(JurgathaCutter)

