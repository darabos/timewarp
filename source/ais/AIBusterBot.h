/** 
  AIBusterBot.h
  $Id$

  Class definition of an AI called BusterBot.  Should be able to create several goals, and pick
  appropriate actions based on those goals.

  Revision history:
    2004.06.12 yb started
*/

#ifndef BUSTERBOT_HEADER_DEFINED
#define BUSTERBOT_HEADER_DEFINED 1

#include <string.h>
#include <stdio.h>
#include <allegro.h>

#include "../melee.h"
#include "../ais.h"
#include "../melee/mcontrol.h"
#include "../melee/mframe.h"
#include "../melee/mship.h"
#include "../melee/mmain.h"

class AIBusterBot : public Control {
public:
	AIBusterBot(const char *name, int channel);
	virtual ~AIBusterBot();
	virtual bool die();
	virtual void calculate();
	virtual int think();
	virtual const char *getTypeName() { return "BusterBot"; }	
};


//class Control : public Presence {	
// public:
//  
//  /*! \brief  controls CANNOT arbitrarily be killed off, because the deal with networking directly */
//  virtual bool die(); 
//  
//  /*! \brief true if this control will delete itself when it's ship dies */
//  bool temporary;                   
//  unsigned char target_sign_color;
//  /*! \brief this pertains to network traffic - see comment above calculate() in mcontrol.cpp */
//  int already;
//  int channel;
//  
//  
//  /*! \brief points at the current ship being controlled */
//  Ship *ship;
//  /*! \brief keys currently pressed */
//  KeyCode keys;
//  /*! \brief name of instance */
//  char *iname;
//  SpaceObject *target;
//  int index;
//  bool valid_target(SpaceObject *t);
//  /*! \brief always-random selection, for the always-random button on the default ship selector */
//  int always_random;
// public:
//  /*! \brief handles camera focusing on a controls ship */
//  virtual SpaceLocation *get_focus();
//  int rand();
//  /*! \brief loads configuration data from a ini file.
//	\example  player1.load("scp.ini", "Keyboard1"); */
//  virtual void load(const char* inifile, const char* inisection);
//  /*! \brief saves configuration data to a ini file. */
//  virtual void save(const char* inifile, const char* inisection);
//  /*! \brief presents the ship selection dialog */
//  virtual int choose_ship(VideoWindow *window, char *prompt, class Fleet *fleet);
//  /*! \brief called whenever the ship being controlled changes 
//	WARNING: select_ship() is not thread-safe */
//  virtual void select_ship(Ship* ship_pointer, const char* ship_name);
//  /*! \brief called every frame of physics */
//  virtual void calculate();
//  /*! \brief called by calculate... this is where the important stuff goes */
//  virtual int think();
//  /*! \brief returns the name of the control type, like "Joystick" */
//  virtual const char *getTypeName() = 0;
//  /*! \brief returns the description of the control, like "NetworkGame(local)_Keyboard0" */
//  virtual char *getDescription();
//  /*! \brief the constructor initializes ship to NULL */
//  Control (const char *name, int channel) ;
//  /*! \brief the destructor, which is rarely used */
//  virtual ~Control () ;
//  /*! called by the GUI stuff to setup the controls (calibrate Joystick, set keys, etc.) */
//  virtual void setup();
//  virtual void set_target(int i);
//  virtual void target_stuff();
//  virtual void animate(Frame *space);
//  
//  
//  enum {_prediction_keys_size = 128};//must be a power of 2
//  //half-assed network bypass for prediction
//  KeyCode *_prediction_keys;
//  unsigned char _prediction_keys_index;
//  
//  /*! \brief for future mid-game lag changes */
//  virtual void _event ( Event *e );
//  
//  virtual void gen_buffered_data();
//};

#endif