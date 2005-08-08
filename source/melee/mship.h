/* $Id$ */ 
#ifndef __MSHIP_H__
#define __MSHIP_H__

#include "mframe.h"

typedef short KeyCode;

struct keyflag {
	enum { 
		left            =	(1<<0),
		right           =	(1<<1),
		thrust          =	(1<<2),
		back            =	(1<<3),
		fire            =	(1<<4),
		special         =	(1<<5),
		altfire         =	(1<<6),
		next            =	(1<<7),
		prev            =	(1<<8),
		closest         =	(1<<9),
		extra1          =   (1<<10),
		extra2	        =   (1<<11),
		communicate     =   (1<<12),
		dec_lag         =   (1<<13),
		inc_lag         =   (1<<14),
		suicide         =   (1<<15),
		};
	};

class Phaser : public SpaceObject {
	protected:
	Vector2 rel_pos;
	Ship *ship;
	int sprite_index;
	int *colors;
	int num_colors;
	int color_index;
	int frame_size;
	int frame_step;
	int phaser_step_position;
	int phaser_steps;
	int phaser_step_size;

	public:
	Phaser::Phaser(SpaceLocation *creator, Vector2 pos, Vector2 rel_pos, 
		Ship *ship, SpaceSprite *sprite, int osprite_index, int *ocolors, 
		int onum_colors, int ofsize, int steps, int step_time) ;

	virtual void animate(Frame *space);
	virtual void calculate();
	};

/** the classification of where a given ship comes from */
enum ShipOrigin { 
    SHIP_ORIGIN_NONE = 0, /**< None, no classification given*/
    SHIP_ORIGIN_SC1,/**< from Star Control 1*/
    SHIP_ORIGIN_SC2,/**< from Star Control 2*/
    SHIP_ORIGIN_SC3,/**< from Star Control 3*/
    SHIP_ORIGIN_TW_ALPHA,/**< Timewarp "Alpha"*/
    SHIP_ORIGIN_TW_BETA,/**< Timewarp "Beta"*/
    SHIP_ORIGIN_TW_SPECIAL/**< Timewarp "Special"*/
};

// allow other ships to affect control over your ship ?!
class OverrideControl
{
public:
	static void add(Ship *s, OverrideControl *newcontrol);
	static void del(Ship *s, OverrideControl *oldcontrol);

	OverrideControl *next;

	virtual void calculate(short *key) = 0;
};

/** ship class */
class Ship : public SpaceObject
{

public:
	OverrideControl *first_override_control, *last_override_control;

protected:

	int hotspot_frame;
	int recharge_step;
	int weapon_recharge;
	int weapon_low;
	int special_recharge;
	int special_low;

	virtual void calculate_thrust();
	virtual void calculate_turn_left();
	virtual void calculate_turn_right();
	virtual void calculate_fire_weapon();
	virtual void calculate_fire_special();
	virtual void calculate_hotspots();

	virtual int activate_weapon();
	virtual int activate_special();

	virtual double get_angle_ex() const; // stupid helper for camera

	public:
	ShipType *type;
	virtual ShipType *get_shiptype();

	ShipClass *code;

	int death_counter;
	int death_explosion_counter;

	bool hashotspots;
	virtual void assigntarget(SpaceObject *otarget);

	virtual void materialize();
	double crew;
	double crew_max;
	double batt;
	double batt_max;
	double turn_rate;
	double turn_step;
	double speed_max;
	double accel_rate;
	int    recharge_amount;
	int    recharge_rate;
	int    weapon_drain;
	int    weapon_rate;
	int    weapon_sample;
	int    special_drain;
	int    special_rate;
	int    special_sample;
	int    hotspot_rate;
	char   captain_name[16];

	SpaceSprite *spritePanel;


	int update_panel;

	KeyCode nextkeys;

	char thrust;
	char thrust_backwards;
	char turn_left;
	char turn_right;
	char fire_weapon;
	char fire_special;
	char fire_altweapon;
	char target_next;
	char target_prev;
	char target_closest;
	char communicate;

	char target_pressed;
	Control *control;

	friend class ShipPanel;

	Ship(SpaceLocation *creator, Vector2 opos, double shipAngle, 
		SpaceSprite *osprite) ;
	Ship(Vector2 opos, double shipAngle, ShipData *shipData, 
			unsigned int code);
	virtual SpaceLocation *get_ship_phaser() ;
	virtual ~Ship();
	virtual void death();


	virtual double getCrew();
	virtual double getBatt();

	virtual RGB crewPanelColor(int k = 0);
	virtual RGB battPanelColor(int k = 0);
	virtual bool custom_panel_update(BITMAP *panel, int display_type) {return false;};


	void locate();

	void calculate();
	virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);
	virtual int handle_fuel_sap(SpaceLocation *source, double normal);
	virtual double handle_speed_loss(SpaceLocation *source, double normal);

	virtual void animate(Frame *frame);
	virtual void animate_predict(Frame *frame, int time);

	virtual void set_target(SpaceObject *o);

	/** set new external control option */
	virtual void set_override_control(OverrideControl *newcontrol);
	/** remove external control option */
	virtual void del_override_control(OverrideControl *delthiscontrol);
};


#endif // __MSHIP_H__

