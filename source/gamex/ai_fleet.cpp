#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "melee.h"
REGISTER_FILE


#include "gamedata.h"

#include "ai_fleet.h"

// later on, this is a "detail"

// first, make sure all gametypes interconnect well, and
// add dialogue (so that others can experiment sooner rather than later).








ControlWussieFG::ControlWussieFG (const char *name)
:
ControlWussie(name, 0)
{
}

void ControlWussieFG::calculate()
{

	if (!exists()) return;

	target_stuff();

	if (ship) {
		if (!ship->exists() || (ship->death_counter != -1)) {
			//message.print(5000, 12, "Ship died in frame %d", game->frame_number);
			select_ship( NULL, NULL);
			}
		else keys = think();
		}
	
	if (!ship) {
		keys = 0;
		if (temporary) state = 0;
		}

}






ControlHumanFG::ControlHumanFG (const char *name)
:
ControlHuman(name, 0)
{
}

void ControlHumanFG::calculate()
{

	if (!exists()) return;

	target_stuff();

	if (ship) {
		if (!ship->exists() || (ship->death_counter != -1)) {
			//message.print(5000, 12, "Ship died in frame %d", game->frame_number);
			select_ship( NULL, NULL);
			}
		else keys = think();
		}
	
	if (!ship) {
		keys = 0;
		if (temporary) state = 0;
		}

}


