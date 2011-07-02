/* $Id$ */ 
/*
 * Star Control - TimeWarp
 *

 */

#include <stdio.h>
#include <string.h>
//#include <allegro.h>
#include "../melee.h"
REGISTER_FILE
#include "../ais.h"
#include "../melee/mship.h"
#include "../gui.h"

bool cruise_control = false;


void player_key_edit(const char *inisection, const char *id, int *value, KEY_EDIT_OPTION option)
{
	if (option == key_load)
		*value = get_config_int(inisection, id, 0);
	else
		set_config_int(inisection, id, *value);
}

/*! \brief load players keys 
  \param inifile with players keys
  \param inisection with players keys 
*/
void player_keys_load_write (player_keys *userkeys, const char *inifile, const char *inisection, KEY_EDIT_OPTION localoption) {
	
	set_config_file (inifile);

	player_key_edit(inisection, "Thrust",		&userkeys->thrust, localoption);
	player_key_edit(inisection, "Backwards",	&userkeys->back, localoption);
	player_key_edit(inisection, "Left",			&userkeys->left, localoption);
	player_key_edit(inisection, "Right",		&userkeys->right, localoption);
	player_key_edit(inisection, "Fire",			&userkeys->fire, localoption);
	player_key_edit(inisection, "AltFire",		&userkeys->altfire, localoption);
	player_key_edit(inisection, "Special",		&userkeys->special, localoption);
	player_key_edit(inisection, "Next_Target",	&userkeys->next, localoption);
	player_key_edit(inisection, "Prev_Target",	&userkeys->prev, localoption);
	player_key_edit(inisection, "Closest_Target",	&userkeys->closest, localoption);
	player_key_edit(inisection, "Extra1",		&userkeys->extra1, localoption);
	player_key_edit(inisection, "Extra2",		&userkeys->extra2, localoption);
	player_key_edit(inisection, "Communicate",	&userkeys->communicate, localoption);
	player_key_edit(inisection, "DecLag",	&userkeys->dec_lag, localoption);
	player_key_edit(inisection, "IncLag",	&userkeys->inc_lag, localoption);
	player_key_edit(inisection, "Extra6",	&userkeys->suicide, localoption);

	return;
}


/*! \brief Get control name */
const char *ControlHuman::getTypeName() {
	return "Keyboard/Joystick";
}

void ControlHuman::load(const char* inifile, const char* inisection)
{
	player_keys_load_write(&userkeys, inifile, inisection, key_load);
}

void ControlHuman::save(const char* inifile, const char* inisection)
{
	throw("should not be used?");
	player_keys_load_write(&userkeys, inifile, inisection, key_write);
}


#include "../melee/mview.h"
/*! \brief Process get input from player */
int ControlHuman::think()
{
	int r = 0;

	if (ship && ship->target && !ship->target->exists ())
	{
		ship->target = 0;
	}

	// if you want a computer AI to do the work for you ;)
	if (cyborg_control)
	{
		// BUT you have to be careful to preserve the (synchronized) randomness, because this is a local
		// operation and not synched between computers.
		// you must also be careful at the moment of loading, because "random" is used there, too, to
		// initialize some settings.
		tw_random_push_state();


		if (!cyborg)
		{
			// load and initialize the cyborg AI
			cyborg = getController("WussieBot", "cyborg-human-interface", channel_none);
			cyborg->auto_update = false;	// networked players need think() to be called externally, so that it's synched...

			cyborg->load("scp.ini", "Config0");

			// needs to check pointers, through the game interface
//			physics->add(cyborg);	//better, exclude this...

		}

		// synchronize the AIs
		if (ship && !cyborg->ship)
		{
			char shipname[6];
			int n = strlen(ship->type->file);
			strncpy(shipname, &ship->type->file[n-9], 5);
			shipname[5] = 0;
			
			push_config_state();
			set_config_file(ship->type->file);
			
			// this assigns the AI to that ship, but the ship->control should still points to the human AI...
			// so that needs to be corrected, then
			cyborg->select_ship(ship, "cyborg-human-interface");
			ship->control = this;	// restore the proper "control" hierarchy

			pop_config_state();
			//select_ship(0, 0);
		}

		// determine the key sequence.

		r = cyborg->think();

		tw_random_pop_state();


		return r;
	} else {
		// switch the ship control back to this human control...
		if (cyborg && cyborg->ship)
		{
			tw_random_push_state();
			select_ship(cyborg->ship, cyborg->ship->data->file);
			cyborg->select_ship(0, 0);
			tw_random_pop_state();
		}
	}

	// manual thrust or cruise change
	if (key_pressed(userkeys.thrust))
	{
		if (!cruise_control)
		{
			// direct control
			r |= keyflag::thrust;
		} else {
			// toggle thrust on/off.
			if (toggle_cruise_thrust_press)
			{
				cruise_control_thrust = !cruise_control_thrust;
				toggle_cruise_thrust_press = false;
			}
		}
	} else {
		toggle_cruise_thrust_press = true;
	}

	// automatic thrust
	if (cruise_control_thrust)
	{
		r |= keyflag::thrust;
	}

	if (key_pressed(userkeys.back)) r |= keyflag::back;
	
	if (key_pressed(userkeys.left))
		r |= keyflag::left;
	
	if (key_pressed(userkeys.right))
		r |= keyflag::right;

	if (key_pressed(userkeys.fire))
		r |= keyflag::fire;

	if (key_pressed(userkeys.altfire))
		r |= keyflag::altfire;

	if (key_pressed(userkeys.special))
		r |= keyflag::special;

	if (key_pressed(userkeys.next))
		r |= keyflag::next;

	if (key_pressed(userkeys.prev))
		r |= keyflag::prev;

	if (key_pressed(userkeys.closest)) r |= keyflag::closest;
	if (key_pressed(userkeys.extra1)) r |= keyflag::extra1;
	if (key_pressed(userkeys.extra2))
		r |= keyflag::extra2;
	if (key_pressed(userkeys.communicate)) r |= keyflag::communicate;
	if (key_pressed(userkeys.dec_lag)) r |= keyflag::dec_lag;
	if (key_pressed(userkeys.inc_lag)) r |= keyflag::inc_lag;
	if (key_pressed(userkeys.suicide)) r |= keyflag::suicide;
	return r;
}

ControlHuman::ControlHuman(const char *name, int channel)
:
Control(name, channel)
{
	auto_update = false;	// networked player need think() to be called externally, so that it's synched...

	toggle_cruise_thrust_press = true;
	cruise_control_thrust = false;
}

