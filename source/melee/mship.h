#ifndef _MFRAME_H
#include "mframe.h"
#endif

#ifndef _MSHIP_H
#define _MSHIP_H



typedef short KeyCode;

#define KEYFLAG_LEFT       1
#define KEYFLAG_RIGHT      2
#define KEYFLAG_THRUST     4
#define KEYFLAG_BACK       8
#define KEYFLAG_FIRE      16
#define KEYFLAG_SPECIAL   32
#define KEYFLAG_ALTFIRE   64
#define KEYFLAG_NEXT     128
#define KEYFLAG_PREV     256
#define KEYFLAG_CLOSEST  512
struct keyflag {
	enum { 
		left = 1, 
		right = 2, 
		thrust = 4, 
		back = 8,
		fire = 16, 
		special = 32, 
		altfire = 64, 
		next = 128, 
		prev = 256, 
		closest = 512
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
	ShipClass *code;

	int death_counter;
	int death_explosion_counter;

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
	char   captain_name[13];

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
	virtual RGB crewPanelColor();
	virtual RGB battPanelColor();

	void locate();
  
	void calculate();
	virtual int handle_damage(SpaceLocation *source, double normal, double direct = 0);
	virtual int handle_fuel_sap(SpaceLocation *source, double normal);
	virtual double handle_speed_loss(SpaceLocation *source, double normal);

	virtual void animate(Frame *frame);
	virtual void animate_predict(Frame *frame, int time);
};

#endif
