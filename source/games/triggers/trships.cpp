


#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "../../melee.h"
//#include "../../melee/mframe.h"
#include "../../melee/mgame.h"
REGISTER_FILE


#include "../../melee/mship.h"
#include "trships.h"
//#include "trhelper.h"






void action_playership::define()
{
	//description = "playership: shipname S x I y I team I";
	//varptr = (void**) (&shipname, &ix, &iy, &team);
	interpret("playership: shipname S xpos I ypos I team I",
					&shipname, &ix, &iy, &team);
}


void action_playership::calculate()
{
	Ship *s;
	s = game->create_ship(game->channel_none, *shipname, "Human", Vector2(*ix, *iy), random(PI2), *team);
	
	// the ship enters the game physics.
	game->add(s->get_ship_phaser());
	
	// add focus to the player
	game->add_focus(s);
}



void action_AIship::define()
{
	//description = "AIship: shipname S x I y I team I";
	//varptr = (void**) (&shipname, &ix, &iy, &team);
	interpret("AIship: shipname S x I y I team I",
					&shipname, &ix, &iy, &team);
}


void action_AIship::calculate()
{
	Ship *s;
	s = game->create_ship(game->channel_none, *shipname, "WussieBot", Vector2(*ix, *iy), random(PI2), *team);
	game->add(s->get_ship_phaser());
}





void action_newteam::define()
{
	//description = "newteam: newteam I";
	//varptr = (void**) (&team);
	interpret("newteam: newteam I",
					&team);
}


void action_newteam::calculate()
{
	*team = game->new_team();
}




