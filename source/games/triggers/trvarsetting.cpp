


#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../../melee.h"
#include "../../melee/mframe.h"
REGISTER_FILE


#include "trvarsetting.h"
//#include "trhelper.h"






void action_setI::define()
{
	//description = "setI: set I = I";
	//varptr = (void**) (&idest, &isrc);
	interpret("setI: set I = I",
					&idest, &isrc);
}


void action_setI::calculate()
{
	*idest = *isrc;
}







void action_setS::define()
{
	//description = "setI: set I = I";
	//varptr = (void**) (&idest, &isrc);
	interpret("setS: set S = S",
					&idest, &isrc);
}


void action_setS::calculate()
{
	*idest = *isrc;
}







void action_convID::define()
{
	//description = "convID: set I = D";
	//varptr = (void**) (&idest, &dsrc);
	interpret("convID: set I = D",
					&idest, &dsrc);
}


void action_convID::calculate()
{
	*idest = iround(*dsrc);
}




