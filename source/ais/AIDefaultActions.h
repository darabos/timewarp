/** 
  AIDefaultActions.h
  $Id$

  Creates some default actions available for AIs.

  Revision history:
    2004.06.15 yb started

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
			return AIAction::updatePriority();
		}
	};

}


#endif