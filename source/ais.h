/* $Id$ */ 
#ifndef __AIS_H__
#define __AIS_H__

#include "melee/mcontrol.h"
#include "melee/mship.h"
#include "melee/mgame.h"

#define MAX_STATES 3

const int MAX_OPTIONS  = 1024;


//ais/c_input.cpp
/*! \brief Human Ship Control */
class ControlHuman : public Control
{	
	private:
	bool toggle_cruise_thrust_press;

	int thrust;
	bool cruise_control_thrust;

	int left;
	int right;
	int back;
	int fire;
	int special;
	int altfire;
	int next;
	int prev;
	int closest;
	int extra1, extra2, communicate, dec_lag, inc_lag;
	int suicide;
	public:
	virtual void load(const char* inifile, const char* inisection);
	virtual void save(const char* inifile, const char* inisection);
	virtual const char *getTypeName();
	virtual int think();
	ControlHuman (const char *name, int channel) ;
	virtual void setup();
	};

//ais/c_other.cpp
int stupid_bot(Ship *ship) ;

/*! \brief AI with no brain */
class ControlVegetable : public Control
{	
	public:

	// sets auto-update to false, cause this is not supposed to do anything really..
	ControlVegetable(const char *name, int channel);

	virtual const char *getTypeName();
	virtual int think();
	virtual int choose_ship(VideoWindow *window, char *prompt, class Fleet *fleet);
	};

/*! \brief Simple AI */
class ControlMoron : public Control {	
	public:
	virtual const char *getTypeName();
	virtual int think();
	ControlMoron (const char *name, int channel) ;
	};



//ais/c_wussie.cpp
/*! \brief Complex AI */
class ControlWussie : public Control {
  public:
  virtual const char *getTypeName();
  virtual int think();
	ControlWussie (const char *name, int channel) ;
	//virtual void set_target(int i) ;
  virtual void select_ship(Ship* ship_pointer, const char* ship_name);
	virtual double check_danger();
	virtual double evasion(Ship* ship) ;

	double option_range[MAX_STATES][2];
	double option_velocity[MAX_STATES][2];
	int last_seen_time;

 bool option_type[MAX_STATES][2][MAX_OPTIONS];

 int fireoption[2];
 int dontfireoption[2];
 double sweep[2];
 int option_held[2];
 double option_freq[MAX_STATES][2];
 int batt_level[MAX_STATES][2];
 int option_timer[MAX_STATES][2];
 int option_time[2];
 SpaceLocation *bomb;
 double bombx[2],bomby[2];
 double bombvx[2],bombvy[2];
 double bombdistance[2];
 double rel[MAX_STATES][2];
 int tactic[MAX_STATES];
 int planet_safe[MAX_STATES];
 double min_range[MAX_STATES];
 double max_range[MAX_STATES];
 int tactic_state;
 int state;

 // some random settings to give some individuality to a ship.
	double evasion_base_size, evasion_ship_multiplier, evasion_angle_change,
		intercept_abort_speed_factor, scare_crew_factor, scare_owncrew_minimum, scare_close_distance,
		scared_enemyship_speed_factor, out_of_range_multiplier;
 };

/*! \brief ??? */
class KeyState {
  public:
  bool thrust;
  bool turn_left;
  bool turn_right;
  bool fire_weapon;
  bool fire_special;
  KeyCode convert();
};

#endif  // __AIS_H__

