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

#ifdef ALLEGRO_MSVC
	#pragma warning (disable:4786)
#endif

#include <string.h>
#include <stdio.h>
#include <allegro.h>

#include "../melee.h"
#include "../ais.h"
#include "../melee/mcontrol.h"
#include "../melee/mframe.h"
#include "../melee/mship.h"
#include "../melee/mmain.h"

#include <list>
using namespace std;


/** a namespace to avoid cluttering up the default namespace.  */
namespace AIBuster {

	typedef unsigned short PriorityType;

	enum { PRIORITY_NONE=0, PRIORITY_MAX=255 } ;

	/** An AIAction is a potential action that an AI can take.  */
	class AIAction {
	public:
		
		/** Constructor.  Creates the action, tracking its priority and target.
		    @priority The action's priority, rated from 0 to 255.  Higher means that the AI
			          will favour this action over lower priority ones.
		    @target the target of this action.
		*/
		AIAction(PriorityType priority, Presence * target) 
			: _priority(priority), _target(target) 
		{ }

		/** Figures out what this action's priority, if it has changed from its previous value.  Returns its new priority.  
		    If the action is no longer valid, the returned value will be zero (and should no longer be considered in the 
			action list).  
		    @return the new priority for this action.  It'll be a positive number for actions that can still be performed,
		            and zero for actions that can't be performed.  */
		PriorityType virtual updatePriority() { return _priority; }

		bool const operator < (const AIAction & rhs) { return rhs._priority < this->_priority; }

		virtual void performAction(int & state) { }

	//protected:
		
		/** a priority of the action.  Higher means that it's more likely to take place. */
		PriorityType _priority;
		
		/** the target of the action.  */
		Presence * _target;
	};

	typedef list<AIAction*> PlanType;
	typedef list<AIAction*>::iterator PlanTypeIterator;


	//typedef priority_queue< AIBuster::AIAction, 
	//	                    vector<AIBuster::AIAction>
	//					   > PlanType;
	//typedef priority_queue<AIBuster::AIAction>::iterator PlanTypeIterator;

	const int TimeToQuickRefresh = 1000;
	const int TimeToCompleteRefresh = 4000;
}

/** This class defines an AI which can make several plans, sort the plans by priority, and combine the top
    plans into a concrete action.  
    @see ::Control */
class AIBusterBot : public Control {

public:
	/** Constructor. 
	    @param name 
	    @param channel	*/
	AIBusterBot(const char *name, int channel);
	
	/** Destructor. */
	virtual ~AIBusterBot();

	/** called when the AI is destroyed, but not when the ship it controls is destroyed. */
	virtual bool die();

	/** Called periodically. */
	virtual void calculate();

	/** Does all the AI in here. 
	    @return a keyflag, which has each action the AI wishes to perform at a particular moment OR'ed into it.  
		@see ::keyflag
	*/
	virtual int think();

	/** @return the name of the bot*/
	virtual const char *getTypeName() { return "BusterBot (Not working yet)"; }

	/** @return the name of the bot*/
	virtual char * getDescription() { return "BusterBot (Not working yet)"; }

	/** Chooses a ship.  
	    @param ship_pointer 
	    @param ship_name */
	virtual void select_ship(Ship* ship_pointer, const char* ship_name);

	/** returns a description of what's going on inside this class for debug purposes.  */
	virtual char * toString(); 

protected:

	AIBuster::PlanType plans;

	/** gets rid of all plans. */ 
	virtual void clearPlans();

	/** Quickly goes through each plan, and updates its priority.  Invalid plans are removed. If a plan's priority has 
	    changed, move its place in the queue accordingly.  */
	virtual void quickRefreshPlans();

	/** Completely dump the plans list and make a new one from scratch.  */ 
	virtual void completeRefreshPlans();

	/** Creates an action which can deal with the given space object.  
	    @param object the SpaceObject to deal with. 
	    @return a new AIAction which can try to deal with the object.  The priority and target are set appropriately.*/
	AIBuster::AIAction * AIBusterBot::createAction(SpaceObject * object);

};

#endif