


#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../../melee.h"
#include "../../melee/mframe.h"
REGISTER_FILE

#include "../../melee/mcbodies.h"

#include "trgamesetting.h"
//#include "trhelper.h"






void action_mapsize::define()
{
	//description = "mapsize: mapsize I , I";
	//varptr = (void**) (&sizex, &sizey);
	interpret("mapsize: mapsize I , I",
					&sizex, &sizey);
}


void action_mapsize::calculate()
{
	physics->size.x = *sizex;
	physics->size.y = *sizey;
	physics->prepare();
}






void action_planet::define()
{
	//description = "mapsize: mapsize I , I";
	//varptr = (void**) (&sizex, &sizey);
	interpret("planet: planet_xpos I ypos I",
					&x, &y);
}


void action_planet::calculate()
{
	::create_planet(Vector2(*x, *y));
}



