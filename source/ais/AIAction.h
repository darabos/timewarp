/** 
  AIAction.h
  $Id$

  Class definition of AIAction.  

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

#ifndef AIACTION_HEADER_DEFINED
#define AIACTION_HEADER_DEFINED 1

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
		PriorityType virtual updatePriority()
		{ 
			if (_target == NULL)
				_priority = PRIORITY_NONE;

			return _priority; 
		}

		bool const operator < (const AIAction & rhs) { return rhs._priority < this->_priority; }

		/** Do whatever the action wants to do.  "State" is modified to have this action's "keys" changed.
		    @param state the "keys" which are changed as a result of performing this action.  */
		virtual void performAction(int & state) { }

		/** a priority of the action.  Higher means that it's more likely to take place. */
		PriorityType _priority;
		
		/** the target of the action.  */
		Presence * _target;

	}; //class AIAction

}//namespace AIBuster

#endif