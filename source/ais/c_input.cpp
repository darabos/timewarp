/*
 * Star Control - TimeWarp
 *
 * c_input.cpp - controller input module
 *
 * 25-Jun-2002, Revision 1 by The Fly
 *
 *
 * - ControlHuman::setup() function modified. setup_key_description() 
 * is now redundant due to modifications in key_to_description(). There
 * was also a duplication of closest target setting.
 * - JOY dialog structure and function modified. Should work O.K. now.
 * - Cosmetic code changes.
 */

#include <stdio.h>
#include <string.h>
#include <allegro.h>
#include "../melee.h"
REGISTER_FILE
#include "../ais.h"
#include "../melee/mship.h"


#define JOY_DIALOG_BOX          0
#define JOY_DIALOG_TITLE        1
#define JOY_DIALOG_CALIBRATE    2
#define JOY_DIALOG_SWITCH       3
#define JOY_DIALOG_DONE         4
#define JOY_DIALOG_DESCRIPTION  5

DIALOG joyDialog[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_box_proc,        40,   40,   200,  250,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_textbox_proc,    50,   50,   180,  30,   255,  0,    0,    0,       0,    0,    (char*)"Joystick Calibration", NULL, NULL },
	{ d_button_proc,     50,   100,  180,  30,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[0], NULL, NULL },
  { d_button_proc,     50,   140,  100,  30,   255,  0,    0,    D_EXIT,  0,    0,    (char*)"Next Joystick", NULL, NULL },
  { d_button_proc,     160,  140,  70,   30,   255,  0,    0,    D_EXIT,  0,    0,    (char*)"Done", NULL, NULL },
  { d_textbox_proc,    50,   180,  180,  100,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

void calibrate_joysticks() {
	int i;
	const char *cal;

	if (!num_joysticks) {
		tw_alert("No joysticks detected", "Done");
		return;
	}

	int which_joystick = 0;
	while (true) {
		if(poll_joystick() != 0)
			return;
		if (::joy[which_joystick].flags & JOYFLAG_CALIBRATE) {
			cal = calibrate_joystick_name(which_joystick);
			if (cal)
				sprintf(dialog_string[0], "Calibrate : %s", cal);
			else
				sprintf(dialog_string[0], "Calibrate : ?");
		}	else
			sprintf (dialog_string[0], "De-Calibrate");

		char buffy[1024];
		char *d = buffy;

		d += sprintf(d, "Joystick %d\n\n", which_joystick);

		for (i = 0; i < ::joy[which_joystick].num_sticks; i += 1) {
			d += sprintf(d, "%s %d\n", ::joy[which_joystick].stick[i].name, i);
		}

		d += sprintf(d, "\nBUTTONS\n");
		for (i = 0; i < ::joy[which_joystick].num_buttons; i += 1) {
			d += sprintf(d, " %s", ::joy[which_joystick].button[i].name);
		}
		
		joyDialog[JOY_DIALOG_DESCRIPTION].dp = buffy;
		i = tw_popup_dialog(NULL,joyDialog, 1);
		if (i == -1)
			i = JOY_DIALOG_DONE;

		switch (i) {
			case JOY_DIALOG_SWITCH:
				if (num_joysticks)
					which_joystick = (which_joystick + 1) % num_joysticks; 
			  break;
			case JOY_DIALOG_DONE:
				return;
			case JOY_DIALOG_CALIBRATE:
				if (::joy[which_joystick].flags & JOYFLAG_CALIBRATE) {
					calibrate_joystick(which_joystick);
					save_joystick_data("joys.ini");
				} else {
					delete_file("joys.ini");
					remove_joystick();
					install_joystick(JOY_TYPE_AUTODETECT);
				}
			  break;
		}
	}
}

/*! \brief load players keys 
  \param inifile with players keys
  \param inisection with players keys 
*/
void ControlHuman::load (const char *inifile, const char *inisection) {
	set_config_file (inifile);
	thrust  = get_config_int(inisection, "Thrust",      0);
	back    = get_config_int(inisection, "Backwards",   0);
	left    = get_config_int(inisection, "Left",        0);
	right   = get_config_int(inisection, "Right",       0);
	fire    = get_config_int(inisection, "Fire",        0);
	altfire = get_config_int(inisection, "AltFire",     0);
	special = get_config_int(inisection, "Special",     0);
	next    = get_config_int(inisection, "Next_Target", 0);
	prev    = get_config_int(inisection, "Prev_Target", 0);
	closest = get_config_int(inisection, "Closest_Target", 0);
	extra1  = get_config_int(inisection, "Extra1", 0);
	extra2  = get_config_int(inisection, "Extra2", 0);
	extra3  = get_config_int(inisection, "Extra3", 0);
	extra4  = get_config_int(inisection, "Extra4", 0);
	extra5  = get_config_int(inisection, "Extra5", 0);
	extra6  = get_config_int(inisection, "Extra6", 0);
	return;
}
/*! \brief Save players key
  \param inifile with players keys
  \param inisection with players keys
 */
void ControlHuman::save (const char *inifile, const char *inisection) {
	set_config_file (inifile);
	set_config_int(inisection, "Thrust",         thrust);
	set_config_int(inisection, "Backwards",      back);
	set_config_int(inisection, "Left",           left);
	set_config_int(inisection, "Right",          right);
	set_config_int(inisection, "Fire",           fire);
	set_config_int(inisection, "AltFire",        altfire);
	set_config_int(inisection, "Special",        special);
	set_config_int(inisection, "Next_Target",    next);
	set_config_int(inisection, "Prev_Target",    prev);
	set_config_int(inisection, "Closest_Target", closest);
	set_config_int(inisection, "Extra1", extra1);
	set_config_int(inisection, "Extra2", extra2);
	set_config_int(inisection, "Extra3", extra3);
	set_config_int(inisection, "Extra4", extra4);
	set_config_int(inisection, "Extra5", extra5);
	set_config_int(inisection, "Extra6", extra6);
	return;
}

/*! \brief Get control name */
const char *ControlHuman::getTypeName() {
	return "Keyboard/Joystick";
}

/*! \brief Process get input from player */
int ControlHuman::think() {
	int r = 0;
	if (key_pressed(thrust)) r |= keyflag::thrust;
	if (key_pressed(back)) r |= keyflag::back;
	if (key_pressed(left)) r |= keyflag::left;
	if (key_pressed(right)) r |= keyflag::right;
	if (key_pressed(fire)) r |= keyflag::fire;
	if (key_pressed(altfire)) r |= keyflag::altfire;
	if (key_pressed(special)) r |= keyflag::special;
	if (key_pressed(next)) r |= keyflag::next;
	if (key_pressed(prev)) r |= keyflag::prev;
	if (key_pressed(closest)) r |= keyflag::closest;
	if (key_pressed(extra1)) r |= keyflag::extra1;
	if (key_pressed(extra2))
		r |= keyflag::extra2;
	if (key_pressed(extra3)) r |= keyflag::extra3;
	if (key_pressed(extra4)) r |= keyflag::extra4;
	if (key_pressed(extra5)) r |= keyflag::extra5;
	if (key_pressed(extra6)) r |= keyflag::extra6;
	return r;
}

ControlHuman::ControlHuman(const char *name, int channel) : Control(name, channel) {}

#define KEY_DIALOG_MODIFY    0
#define KEY_DIALOG_OK        17
#define KEY_DIALOG_CANCEL    18
#define KEY_DIALOG_CALIBRATE 19

DIALOG keyDialog[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_textbox_proc,    0,    0,    160,  80,   255,  0,    0,    0,       0,    0,    dialog_string[0], NULL, NULL },
  { d_button_proc,     60,   90,   500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[1], NULL, NULL },
  { d_button_proc,     60,   120,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[2], NULL, NULL },
  { d_button_proc,     60,   150,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[3], NULL, NULL },
  { d_button_proc,     60,   180,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[4], NULL, NULL },
  { d_button_proc,     60,   210,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[5], NULL, NULL },
  { d_button_proc,     60,   240,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[6], NULL, NULL },
  { d_button_proc,     60,   270,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[7], NULL, NULL },
  { d_button_proc,     60,   300,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[8], NULL, NULL },
  { d_button_proc,     60,   330,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[9], NULL, NULL },
  { d_button_proc,     60,   360,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[10], NULL, NULL },
  { d_button_proc,     60,   375,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[11], NULL, NULL },
  { d_button_proc,     60,   390,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[12], NULL, NULL },
  { d_button_proc,     60,   405,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[13], NULL, NULL },
  { d_button_proc,     60,   420,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[14], NULL, NULL },
  { d_button_proc,     60,   435,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[15], NULL, NULL },
  { d_button_proc,     60,   450,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[16], NULL, NULL },

  { d_button_proc,     180,  20,   180,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void*)"Accept Changes", NULL, NULL },
  { d_button_proc,     180,  50,   180,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void*)"Cancel", NULL, NULL },
  { d_button_proc,     360,  35,   200,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void*)"Calibrate Joysticks", NULL, NULL },
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

/*! \brief setap players keys */
void ControlHuman::setup() {
	int i, t = 0;
	int last = 0;
	while (true) {
		char *s;
		int index = 0;

		s = dialog_string[index]; index += 1;

		s += sprintf(s, "Set Your Keys\nController %s", getDescription());
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "Left:           ");
		key_to_description(left, s);
		s = dialog_string[index]; index += 1;
		s += sprintf ( s, "Right:          ");
		key_to_description(right, s);
		s = dialog_string[index]; index += 1;
		s += sprintf ( s, "Thrust:         ");
		key_to_description(thrust, s);
		s = dialog_string[index]; index += 1;
		s += sprintf ( s, "Backwards:      ");
		key_to_description(back, s);
		s = dialog_string[index]; index += 1;
		s += sprintf ( s, "Fire:           ");
		key_to_description(fire, s);
		s = dialog_string[index]; index += 1;
		s += sprintf ( s, "Special:        ");
		key_to_description(special, s);
		s = dialog_string[index]; index += 1;
		s += sprintf ( s, "AltFire:        ");
		key_to_description(altfire, s);
		s = dialog_string[index]; index += 1;
		s += sprintf ( s, "Next Target:    ");
		key_to_description(next, s);
		s = dialog_string[index]; index += 1;
		s += sprintf ( s, "Prev Target:    ");
		key_to_description(prev, s);
		s = dialog_string[index]; index += 1;
		s += sprintf ( s, "Closest Target: ");
		key_to_description ( closest, s );

		s = dialog_string[index]; index += 1;
		s += sprintf ( s, "Extra1: ");
		key_to_description ( extra1, s );
		s = dialog_string[index]; index += 1;
		s += sprintf ( s, "Extra2: ");
		key_to_description ( extra2, s );
		s = dialog_string[index]; index += 1;
		s += sprintf ( s, "Extra3: ");
		key_to_description ( extra3, s );
		s = dialog_string[index]; index += 1;
		s += sprintf ( s, "Extra4: ");
		key_to_description ( extra4, s );
		s = dialog_string[index]; index += 1;
		s += sprintf ( s, "Extra5: ");
		key_to_description ( extra5, s );
		s = dialog_string[index]; index += 1;
		s += sprintf ( s, "Extra6: ");
		key_to_description ( extra6, s );

		s = dialog_string[index]; index += 1;

		int maxlen = 0;
		for (i = 1; i < KEY_DIALOG_OK; i += 1) {
			int t = strlen(dialog_string[i]);
			if (maxlen < t) maxlen = t;
		}
		for (i = 1; i < KEY_DIALOG_OK; i += 1) {
			int t = strlen(dialog_string[i]);
			dialog_string[i][maxlen] = 0; 
			memset(dialog_string[i]+t, ' ', maxlen-t);
		}
		clear_keybuf();
		if (last < KEY_DIALOG_OK) last += 1;
		i = tw_do_dialog(NULL, keyDialog, last);
		if (i == -1) return;
		if (i < KEY_DIALOG_OK) {
			t = get_key();
			clear_keybuf();
		}
		if (t == KEY_ESC) i = KEY_DIALOG_CANCEL;
		switch (i) {
			case 1:  left    = t; break;
			case 2:  right   = t; break;
			case 3:  thrust  = t; break;
			case 4:  back    = t; break;
			case 5:  fire    = t; break;
			case 6:  special = t; break;
			case 7:  altfire = t; break;
			case 8:  next    = t; break;
			case 9:  prev    = t; break;
			case 10: closest = t; break;
			case 11: extra1 = t; break;
			case 12: extra2 = t; break;
			case 13: extra3 = t; break;
			case 14: extra4 = t; break;
			case 15: extra5 = t; break;
			case 16: extra6 = t; break;
			case KEY_DIALOG_OK:  save("scp.ini", getDescription()); return;
			case KEY_DIALOG_CANCEL: load("scp.ini", getDescription()); return;
			case KEY_DIALOG_CALIBRATE: calibrate_joysticks(); break;
		}
	}
	return;
}
