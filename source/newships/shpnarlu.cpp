/* $Id$ */ 
#include "../ship.h"
#include "../scp.h"
REGISTER_FILE

#define NAROOL_POISON_ID     0x1122

#include "../util/aastr.h"
#include "../melee/mview.h"
#include "../frame.h"

#include <stdlib.h>

#include "../other/lightning.h"




class NaroolPoison;

class NaroolLurker : public Ship {
public:
IDENTITY(NaroolLurker);
	double weaponRange;
	double weaponVelocity;
	double poison;
	int    weaponDamage;
	int    weaponArmour;
	int    weaponDuration;
	int    normalRechargeAmount;
	int	   cloak;
	int    cloak_frame;

	Lightning lightning;

	public:

	static int cloak_color[3];

	NaroolLurker(Vector2 opos, double shipAngle,
		ShipData *shipData, unsigned int code);

	virtual ~NaroolLurker();


	virtual double isInvisible() const;
	virtual int activate_weapon();
	virtual void calculate_fire_special();
	virtual void calculate_hotspots();
	virtual void calculate();
	virtual void animate(Frame *space);

	virtual void inflict_damage(SpaceObject *other);

};
class NaroolGas: public AnimatedShot {
public:
IDENTITY(NaroolGas);

	int hitShip;
	int duration;
	float poison;

	public:

	NaroolGas(double ox,double oy,double oangle,double ov, int odamage, int duration, 
		double orange, int oarmour, float poison, NaroolLurker *oship, SpaceSprite *osprite, double relativity = 0);

	virtual void inflict_damage(SpaceObject *other);
	virtual void soundExplosion();
	virtual void animateExplosion();
};

class NaroolPoison : public SpaceObject {
public:
IDENTITY(NaroolPoison);
public:
  Ship *oship;
  double poison;
  int duration;
  int start;

	NaroolPoison(NaroolGas *gas, int duration, float poison, Ship *kill_it, SpaceSprite *osprite);
	virtual void calculate();
	virtual void animate(Frame *space);
	};

int NaroolLurker::cloak_color[3] = { 15, 10, 2 };

NaroolLurker::NaroolLurker(Vector2 opos, double shipAngle,
	ShipData *shipData, unsigned int code) 
	:
  Ship(opos, shipAngle, shipData, code)
{
	weaponRange    = scale_range(get_config_float("Weapon", "Range", 0));
	weaponVelocity = scale_velocity(get_config_float("Weapon", "Velocity", 0));
	weaponDamage   = get_config_int("Weapon", "Damage", 0);
	weaponArmour   = get_config_int("Weapon", "Armour", 0);
	weaponDuration = get_config_int("Weapon", "Duration", 0);
	poison         = get_config_float("Weapon", "poison", 0);

	normalRechargeAmount = recharge_amount;

	cloak = FALSE;
	cloak_frame = 0;

	BITMAP *shpbmp = sprite->get_bitmap(0);
	double maxsparktime = get_config_float("Quirk", "maxsparktime", 2000);
	double Rmax = get_config_float("Quirk", "Rmax", 1);
	lightning.init(shpbmp, 0, maxsparktime, Rmax);


}

NaroolLurker::~NaroolLurker()
{

}

double NaroolLurker::isInvisible() const
{
	if ( (cloak_frame >= 300) && (!lightning.visible()) )
		return 1;
	else
		return 0;
	}

int NaroolLurker::activate_weapon()
{
	STACKTRACE
	double a, relativity = game->shot_relativity;
	if (target) {
		a = intercept_angle2(pos, vel * relativity, weaponVelocity, 
			target->normal_pos(), target->get_vel());
		}
	else a = angle;
	add(new NaroolGas(0.0, 0.0,
		a , weaponVelocity, weaponDamage, weaponDuration, weaponRange, weaponArmour, 
		poison, this, data->spriteWeapon, relativity));
  return(TRUE);
}

void NaroolLurker::calculate_fire_special()
{
	STACKTRACE
	special_low = FALSE;

	if(fire_special) {
		if((batt < special_drain) && (!cloak)) {
		special_low = TRUE;
		return;
		}

		if(special_recharge > 0)
			return;

		if(cloak) {
			cloak = FALSE;

			play_sound2(data->sampleSpecial[1]);
				recharge_amount = normalRechargeAmount;
		} else {
			cloak = TRUE;
			play_sound2(data->sampleSpecial[0]);
			//batt -= special_drain;
			//recharge_amount = 1;
			recharge_amount = 0;
		}

		special_recharge = special_rate;
	}
}

void NaroolLurker::calculate_hotspots()
{
	STACKTRACE
	if(!cloak)
		Ship::calculate_hotspots();
}

void NaroolLurker::calculate()
{
	STACKTRACE
	if((cloak) && (cloak_frame < 300))
		cloak_frame+= frame_time;
	if((!cloak) && (cloak_frame > 0))
		cloak_frame-= frame_time;

	Ship::calculate();


	lightning.update(frame_time);
}

void NaroolLurker::animate(Frame *space)
{
	STACKTRACE;
	if((cloak_frame > 0) && (cloak_frame < 300))
	{
		sprite->animate_character( pos, sprite_index, pallete_color[cloak_color[(int)(cloak_frame / 100)]], space);
	} else if((cloak_frame >= 300))
	{
		if (!show_red_cloaker || (control && is_bot(control->channel)) || (control && !is_local(control->channel)) || (!game_networked && num_network>1))	// bots and remote players are "hidden"
		{
			sprite->animate_character( pos, sprite_index, pallete_color[0], space);

			
			//	aa_set_mode(find_aa_mode(general_attributes));
			
			Vector2 P, S;
			S = sprite->size(0) * ::space_zoom;
			P = corner(pos, sprite->size(0));
			
			lightning.draw(space, P, S);

		
		} else
			sprite->animate_character( pos, sprite_index, pallete_color[4], space);
	} else
		Ship::animate(space);



}

NaroolGas::NaroolGas(double ox, double oy,double oangle,double ov,int odamage, int oduration, 
  double orange,int oarmour, float poison, NaroolLurker *oship, SpaceSprite *osprite, double relativity) :
  AnimatedShot(oship, Vector2(ox,oy), oangle, ov, odamage, orange, oarmour, oship,
    osprite, 50, 21, relativity)

{
	duration = oduration;
	hitShip = FALSE;
	this->poison = poison;
	collide_flag_anyone = bit(LAYER_SHIPS) | bit(LAYER_CBODIES);
}

void NaroolGas::inflict_damage(SpaceObject *other) {
	STACKTRACE
	play_sound2(data->sampleExtra[1]);

	if (other->isShip()) {
		add( new NaroolPoison(this, duration, poison, (Ship*)other, data->spriteWeapon));
		add(new FixedAnimation(this, other, data->spriteWeaponExplosion, 0, 8, 150, LAYER_EXPLOSIONS));
		}
	else { 
		add(new Animation(this, pos, data->spriteWeaponExplosion, 0, 8, 150, LAYER_EXPLOSIONS));
		}
	AnimatedShot::inflict_damage(other);
	return;
	}

void NaroolGas::animateExplosion() {
	return;
	}

void NaroolGas::soundExplosion() {
	STACKTRACE
	if (!hitShip) {
		play_sound2(data->sampleExtra[1]);
		}
	return;
	}

NaroolPoison::NaroolPoison(NaroolGas *gas, int nduration, float poison, Ship *nship, SpaceSprite *osprite) :
  SpaceObject (gas, nship->normal_pos(), 0.0, osprite),
  oship(nship),
  poison(poison),
  duration(nduration)
{
	target = oship;
	id |= NAROOL_POISON_ID;
	layer = LAYER_HOTSPOTS;
	start = TRUE;
	collide_flag_anyone = 0;
	Query q;
	for (q.begin(oship, bit(LAYER_HOTSPOTS), 10); q.current; q.next()) {
		if ((q.current->getID() == getID()) && (((NaroolPoison*)q.current)->oship == oship)) {
			((NaroolPoison*)q.current)->duration = duration;
			state = 0;
			}
		}
	q.end();
	}

void NaroolPoison::calculate() {
	STACKTRACE
	int chance;
	SpaceObject::calculate();

	pos = oship->normal_pos();
	vel = oship->get_vel();
	duration -= frame_time;
	if (duration < 0){
		if (oship->spritePanel)
		{
		blit(oship->data->spritePanel->get_bitmap(0), oship->spritePanel->get_bitmap(0),
				16 , 18, 16, 18, 32, 30);
		oship->update_panel = TRUE;
		}
		state = 0;
		return;
		}
	if (!(oship && oship->exists()))
	{
		oship = 0;
		state = 0;
		}
	else {
		if (start) {
			start = FALSE;
			if (oship->spritePanel)
			{
			data->spriteExtra->draw(16, 18, 0, oship->spritePanel->get_bitmap(0));
//			draw_sprite(oship->spritePanel->get_bitmap(0), data->spriteExtra->get_bitmap(0),16,18);
			oship->update_panel = TRUE;
			}
			}
		chance = iround(tw_random(1000.0));
		if (chance < frame_time * poison) {
			play_sound(data->sampleExtra[0]);
			damage(oship, 0, 1);
			}
		}
	return;
	}

void NaroolPoison::animate(Frame *space)
{ 
}




void NaroolLurker::inflict_damage(SpaceObject *other)
{
	STACKTRACE
	Ship::inflict_damage(other);

	//you've hit something; activate sparks.
	lightning.reset();

	// but, where did you hit it ?
	// place the source somewhere... at the edge .. how ?
	double a, R;
	a = trajectory_angle(other);
	R = 40;
	
	lightning.locate(R * unit_vector(a));
}



REGISTER_SHIP(NaroolLurker)
