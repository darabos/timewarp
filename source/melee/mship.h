#ifndef __MSHIP_H__
#define __MSHIP_H__

#include "mframe.h"


typedef short KeyCode;

enum {
	KEYFLAG_LEFT    = (1),
	KEYFLAG_RIGHT   = (1<<1),
	KEYFLAG_THRUST  = (1<<2),
	KEYFLAG_BACK    = (1<<3),
	KEYFLAG_FIRE    = (1<<4),
	KEYFLAG_SPECIAL = (1<<5),
	KEYFLAG_ALTFIRE = (1<<6),
	KEYFLAG_NEXT    = (1<<7),
	KEYFLAG_PREV    = (1<<8),
	KEYFLAG_CLOSEST = (1<<9),
	KEYFLAG_EXTRA1  = (1<<10),
	KEYFLAG_EXTRA2  = (1<<11),
	KEYFLAG_EXTRA3  = (1<<12),
	KEYFLAG_EXTRA4  = (1<<13),
	KEYFLAG_EXTRA5  = (1<<14),
	KEYFLAG_SUICIDE = (1<<15),
};

struct keyflag {
	enum { 
		left =		KEYFLAG_LEFT,
		right =		KEYFLAG_RIGHT,
		thrust =	KEYFLAG_THRUST,
		back =		KEYFLAG_BACK,
		fire =		KEYFLAG_FIRE,
		special =	KEYFLAG_SPECIAL,
		altfire =	KEYFLAG_ALTFIRE,
		next =		KEYFLAG_NEXT,
		prev =		KEYFLAG_PREV,
		closest =	KEYFLAG_CLOSEST,
		extra1 =	KEYFLAG_EXTRA1,
		extra2 =	KEYFLAG_EXTRA2,
		extra3 =	KEYFLAG_EXTRA3,
		extra4 =	KEYFLAG_EXTRA4,
		extra5 =	KEYFLAG_EXTRA5,
		suicide =	KEYFLAG_SUICIDE
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


class Ship : public SpaceObject {
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
	float    crew;
	float    crew_max;
	float    batt;
	float    batt_max;
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
};


#endif // __MSHIP_H__

