/** 
  AIBusterBot.h
  $Id$

  Class definition of an AI called BusterBot.  Should be able to create several goals, and pick
  appropriate actions based on those goals.

  Revision history:
    2004.06.12 yb started

This file is part of "Star Control: TimeWarp" 
                    http://timewarp.sourceforge.net/
Copyright (C) 2001-2004  TimeWarp development team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
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
#include "AIAction.h"
using namespace AIBuster;

#include <list>
using namespace std;

namespace AIBuster {

	/** tracks plans.  */
	typedef list<AIAction*> PlanType;

	/** iterates through a list of plans. */
	typedef list<AIAction*>::iterator PlanTypeIterator;

	/** the number of game-milliseconds which must pass befor a quick-refresh of plans is made.  */
	const int DefaultTimeToQuickRefresh = 1000;

	/** the number of game-milliseconds which must pass befor a complete-refresh of plans is made.  */
	const int DefaultTimeToCompleteRefresh = 4000;
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

	/** the number of game-milliseconds which must pass befor a quick-refresh of plans is made.  */
	int TimeToQuickRefresh;

	/** the number of game-milliseconds which must pass befor a complete-refresh of plans is made.  */
	int TimeToCompleteRefresh;


};

#endif