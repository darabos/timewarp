

#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../../melee.h"
#include "../../melee/mframe.h"
REGISTER_FILE


#include "trmath.h"
//#include "trhelper.h"






void action_addI::define()
{
	//description = "addI: add I += I";
	//varptr = (void**) (&idest, &isrc);
	interpret("addI: add I += I",
				&idest, &isrc);
}


void action_addI::calculate()
{
	*idest += *isrc;
}




void action_subI::define()
{
	//description = "subI: sub I -= I";
	//varptr = (void**) (&idest, &isrc);
	interpret("subI: sub I -= I",
					&idest, &isrc);

}


void action_subI::calculate()
{
	*idest -= *isrc;
}




void action_mulI::define()
{
	//description = "mulI: mul I *= I";
	//varptr = (void**) (&idest, &isrc);
	interpret("mulI: mul I *= I",
					&idest, &isrc);
}


void action_mulI::calculate()
{
	*idest *= *isrc;
}




void action_divI::define()
{
	//description = "divI: div I /= I";
	//varptr = (void**) (&idest, &isrc);
	interpret("divI: div I /= I",
					&idest, &isrc);
}


void action_divI::calculate()
{
	if (*isrc != 0)
		*idest /= *isrc;
	else
		*idest = 0;
}





