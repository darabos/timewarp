/** 
  AIActions.h
  $Id$

  Creates some default actions available for AIs.

  Revision history:
    2004.06.15 yb started
*/

#ifndef AIACTIONS_HEADER_DEFINED
#define AIACTIONS_HEADER_DEFINED 1

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

#include "AIBusterBot.h"
using namespace AIBuster;

namespace AIBuster { 

	class AIActionAttack : public AIAction {
	public:
		AIActionAttack(PriorityType priority, Presence * target) 
			: AIAction(priority, target) 
		{ }

		virtual void performAction(int & state)
		{ 
			state |= keyflag::right;
		}

		virtual PriorityType updatePriority() {
			if (_target == NULL)
				_priority = PRIORITY_NONE;

			return AIAction::updatePriority();
		}
	};

}


#endif