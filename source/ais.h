#ifndef _MCONTROL_H
#include "melee/mcontrol.h"
#endif
#ifndef _MSHIP_H
#include "melee/mship.h"
#endif
#ifndef _MGAME_H
#include "melee/mgame.h"
#endif

#ifndef _AIS_H
#define _AIS_H

#define MAX_STATES 3
#define MAX_OPTION 4

//ais/c_input.cpp
class ControlHuman : public Control {	
	private:
	int thrust;
	int left;
	int right;
	int back;
	int fire;
	int special;
	int altfire;
	int next;
	int prev;
	int closest;
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
class ControlVegetable : public Control {	
	public:
	virtual const char *getTypeName();
	virtual int think();
	virtual int choose_ship(VideoWindow *window, char *prompt, class Fleet *fleet);
	ControlVegetable (const char *name, int channel) ;
	};
class ControlMoron : public Control {	
	public:
	virtual const char *getTypeName();
	virtual int think();
	ControlMoron (const char *name, int channel) ;
	};



//ais/c_wussie.cpp
class ControlWussie : public Control {
  public:
  virtual const char *getTypeName();
  virtual int think();
	ControlWussie (const char *name, int channel) ;
	//virtual void set_target(int i) ;
  virtual void select_ship(Ship* ship_pointer, const char* ship_name);
	virtual double check_danger();
	double evasion(Ship* ship) ;

	double option_range[MAX_STATES][2];
	double option_velocity[MAX_STATES][2];
	int last_seen_time;
 int option_type[MAX_STATES][2][MAX_OPTION];
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
 };

class KeyState {
  public:
  bool thrust;
  bool turn_left;
  bool turn_right;
  bool fire_weapon;
  bool fire_special;
  KeyCode convert();
};

#endif