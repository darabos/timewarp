#ifndef __MCONTROL_H__
#define __MCONTROL_H__

#include "../melee.h"
#include "mframe.h"
#include "mship.h"

extern const char num_controls;
extern char **control_name;
Control *getController(const char *type, const char *name, int channel);

class Control : public Presence {	
	public:

	virtual bool die(); // controls CANNOT arbitrarily be killed off, because the deal with networking directly

	bool temporary;                   //true if this control will delete itself when it's ship dies
	unsigned char target_sign_color;
	int already;//this pertains to network traffic - see comment above calculate() in mcontrol.cpp
	int channel;



//points at the current ship being controlled
	Ship *ship;
//keys currently pressed
	KeyCode keys;
//name of instance
	char *iname;
//number of targets
//	int num_targets;
//array of targets
//	SpaceObject **target;
//index (into the array, target) of the currently active target)
//	int active_target;
	SpaceObject *target;
	int index;
	bool valid_target(SpaceObject *t);
//always-random selection, for the always-random button on the default ship selector
	int always_random;
	public:
//handles camera focusing on a controls ship
	virtual SpaceLocation *get_focus();
	int rand();
//loads configuration data from a ini file. 
//example:  player1.load("scp.ini", "Keyboard1");
	virtual void load(const char* inifile, const char* inisection);
//saves configuration data to a ini file. 
	virtual void save(const char* inifile, const char* inisection);
//presents the ship selection dialog
	virtual int choose_ship(VideoWindow *window, char *prompt, class Fleet *fleet);
//called whenever the ship being controlled changes
//WARNING: select_ship() is not thread-safe
	virtual void select_ship(Ship* ship_pointer, const char* ship_name);
//called every frame of physics
	virtual void calculate();
//called by calculate... this is where the important stuff goes
	virtual int think();
//returns the name of the control type, like "Joystick"
	virtual const char *getTypeName() = 0;
//returns the description of the control, like "NetworkGame(local)_Keyboard0"
	virtual char *getDescription();
//the constructor initializes ship to NULL
	Control (const char *name, int channel) ;
//the destructor, which is rarely used
	virtual ~Control () ;
//called by the GUI stuff to setup the controls (calibrate Joystick, set keys, etc.)
	virtual void setup();
//	virtual void add_target(SpaceObject *killit) ;
//	virtual void remove_target(SpaceObject *killit) ;
//sets the target
	virtual void set_target(int i);
	virtual void target_stuff();
	virtual void animate(Frame *space);


	enum {_prediction_keys_size = 128};//must be a power of 2
//half-assed network bypass for prediction
	KeyCode *_prediction_keys;
	unsigned char _prediction_keys_index;

//for future mid-game lag changes
	virtual void _event ( Event *e );


	virtual void gen_buffered_data();
	};




#endif // __MCONTROL_H__
