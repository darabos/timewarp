/** 
  AIBusterBot.cpp
  $Id$

  Class implementation of an AI called BusterBot.  Should be able to create several goals, and pick
  appropriate actions based on those goals.

  Revision history:
    2004.06.12 yb started
*/

#ifdef ALLEGRO_MSVC
	#pragma warning (disable:4786)
#endif

#include <string.h>
#include <stdio.h>
#include <allegro.h>

#include "../melee.h"
REGISTER_FILE
#include "../ais.h"
#include "../melee/mcontrol.h"
#include "../melee/mframe.h"
#include "../melee/mship.h"
#include "../melee/mmain.h"

#include "../melee/mview.h" //for message.out()

#include "AIBusterBot.h"
#include "AIActions.h"
using namespace AIBuster;

AIBusterBot::AIBusterBot(const char *name, int channel) : 
Control(name, channel)
{ 
	clearPlans();
}

AIBusterBot::~AIBusterBot() 
{
	clearPlans();	
}

bool AIBusterBot::die()
{ 
	clearPlans();
	return Control::die();
}

void AIBusterBot::select_ship(Ship* ship_pointer, const char* ship_name) {
	Control::select_ship(ship_pointer, ship_name);
	completeRefreshPlans();
}

void AIBusterBot::calculate() 
{
	Control::calculate();

}

int AIBusterBot::think() 
{
	static int nextQuickRefresh = game->game_time;
	static int nextCompleteRefresh = game->game_time;

	if (game->game_time >= nextCompleteRefresh) { 

		nextQuickRefresh = game->game_time + AIBuster::TimeToQuickRefresh;
		nextCompleteRefresh = game->game_time + AIBuster::TimeToCompleteRefresh ;
		
		message.print(1000,15,"AI: Complete refresh. %i plans", plans.size());//TODO remove

		this->completeRefreshPlans();
	}
	else if (game->game_time >= nextQuickRefresh) {
		nextQuickRefresh = game->game_time + AIBuster::TimeToQuickRefresh;

		message.print(1000,15,"AI: Quick refresh. %i plans", plans.size());//TODO remove

		this->quickRefreshPlans();
	}

	//set initial action to do nothing
	int action = 0;

	//pick an action to perform
	if (plans.size() == 0) 
		return action;

	
	for (PlanTypeIterator i=plans.begin(); 
	     i!=plans.end(); 
		 i++) 
	{
	    ASSERT(*i != NULL);

 		(*i)->performAction(action);
	}

	return action;
}



void AIBusterBot::clearPlans() 
{ 
	for (PlanTypeIterator i=plans.begin(); i!=plans.end(); i++) {
		delete *i;
	}

	plans.clear(); 
}

void AIBusterBot::quickRefreshPlans()
{
	for (PlanTypeIterator i=plans.begin(); i!=plans.end(); i++) {

		PriorityType oldPri = (*i)->_priority;
		PriorityType updatedPri = (*i)->updatePriority();

        //if this action now has zero priority, then it is no longer relevent: remove it
		if (updatedPri == PRIORITY_NONE) {

			plans.remove(*i);
			delete *i;
			continue;
		}

		if (oldPri != updatedPri) {
			// do somethin' here... perhaps pull this action out of the list, and re-insert it?  to reduce the sort time later
		}
	}
	plans.sort();
}

void AIBusterBot::completeRefreshPlans()
{STACKTRACE
	clearPlans();

    if (this->ship == NULL)
		return;

	Query q;
	SpaceObject *p = NULL;

	for (q.begin(this->ship, bit(LAYER_SHIPS), 50000); q.current; q.next())
	{
		p = q.currento;
		AIAction * action = createAction(p);
		if (action != NULL) {
			plans.insert(plans.begin(), action);
		}
	}
}

AIAction * AIBusterBot::createAction(SpaceObject * object) 
{STACKTRACE

    AIAction * action = NULL;

	if (object->isShip() && 
		object->exists() && 
		this->ship != NULL &&
		object->get_team() != this->ship->get_team())  // asuming that all teams are pair-wise enemies
	{
		//action = new AIAction(100, object);
		action = new AIActionAttack(100, object);

	}
	return action;
}




	
// copied and pasted from WussieBot 
/*Query ap;
SpaceObject *p;
if (ship->target && !ship->target->isInvisible ())
	last_seen_time = game->game_time;
else if ((rand () & 32767) < frame_time)
	last_seen_time = game->game_time - 1000;
for (ap.begin (ship, bit (LAYER_CBODIES), planet_safe[state]);
	ap.current; ap.next ())
{
	p = ap.currento;
	if (p->isPlanet ())
	{
		pangle = ship->trajectory_angle (p);
		va = atan (ship->get_vel ());
		if (fabs (pangle - va) < PI/3)
		{
			avoid_planet = TRUE;
			if (normalize (pangle - va, PI2) <
				normalize (va - pangle, PI2))
				angle =
					-PI/2 + ship->get_angle () -
					ship->trajectory_angle (p);
			else
				angle =
					PI/2 + ship->get_angle () -
					ship->trajectory_angle (p);
		}
	}
}/**/

	





char * AIBusterBot::toString()
{
	return "It's broken...";
}




