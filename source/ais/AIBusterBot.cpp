/** 
  AIBusterBot.cpp
  $Id$

  Class implementation of an AI called BusterBot.  Should be able to create several goals, and pick
  appropriate actions based on those goals.

  Revision history:
    2004.06.12 yb started
*/


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

AIBusterBot::AIBusterBot(const char *name, int channel) : 
Control(name, channel)
{ }

AIBusterBot::~AIBusterBot() 
{
	
}

bool AIBusterBot::die()
{ 
	return Control::die();
}

void AIBusterBot::calculate() 
{

}

int AIBusterBot::think() 
{
	return 0;
}
