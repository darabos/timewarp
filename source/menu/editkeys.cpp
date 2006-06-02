
#include <allegro.h>

#include <stdio.h>
#include <string.h>

#include "melee.h"

#include "ais.h"
#include "melee/mship.h"
#include "gui.h"



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
	{ my_d_button_proc,  50,   100,  180,  30,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[0], NULL, NULL },
  { my_d_button_proc,  50,   140,  100,  30,   255,  0,    0,    D_EXIT,  0,    0,    (char*)"Next Joystick", NULL, NULL },
  { my_d_button_proc,  160,  140,  70,   30,   255,  0,    0,    D_EXIT,  0,    0,    (char*)"Done", NULL, NULL },
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



#define KEY_DIALOG_MODIFY    0
#define KEY_DIALOG_OK        17
#define KEY_DIALOG_CANCEL    18
#define KEY_DIALOG_CALIBRATE 19
#define KEY_DIALOG_PREV_PLAYER 20
#define KEY_DIALOG_NEXT_PLAYER 21

DIALOG keyDialog[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_textbox_proc,    0,    0,    160,  80,   255,  0,    0,    0,       0,    0,    dialog_string[0], NULL, NULL },
  { my_d_button_proc,  60,   90,   500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[1], NULL, NULL },
  { my_d_button_proc,  60,   120,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[2], NULL, NULL },
  { my_d_button_proc,  60,   150,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[3], NULL, NULL },
  { my_d_button_proc,  60,   180,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[4], NULL, NULL },
  { my_d_button_proc,  60,   210,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[5], NULL, NULL },
  { my_d_button_proc,  60,   240,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[6], NULL, NULL },
  { my_d_button_proc,  60,   270,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[7], NULL, NULL },
  { my_d_button_proc,  60,   300,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[8], NULL, NULL },
  { my_d_button_proc,  60,   330,  500,  25,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[9], NULL, NULL },
  { my_d_button_proc,  60,   360,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[10], NULL, NULL },
  { my_d_button_proc,  60,   375,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[11], NULL, NULL },
  { my_d_button_proc,  60,   390,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[12], NULL, NULL },
  { my_d_button_proc,  60,   405,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[13], NULL, NULL },
  { my_d_button_proc,  60,   420,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[14], NULL, NULL },
  { my_d_button_proc,  60,   435,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[15], NULL, NULL },
  { my_d_button_proc,  60,   450,  500,  15,   255,  0,    0,    D_EXIT,  0,    0,    dialog_string[16], NULL, NULL },

  { my_d_button_proc,  260,  20,   120,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void*)"Apply Changes", NULL, NULL },
  { d_button_proc,     260,  50,   120,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void*)"Reload Keys", NULL, NULL },

  { my_d_button_proc,  400,  35,   160,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void*)"Calibrate Joysticks", NULL, NULL },

  { my_d_button_proc,  180,  20,   60,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void*)"prev", NULL, NULL },
  { my_d_button_proc,  180,  50,   60,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void*)"next", NULL, NULL },
  
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};


static player_keys userkeys;
static char player_id[512];


void load_keys(int config_num)
{
	sprintf (player_id, "Config%d", config_num);
	player_keys_load_write(&userkeys, "scp.ini", player_id, key_load);
}

void write_keys(int config_num)
{
	sprintf (player_id, "Config%d", config_num);
	player_keys_load_write(&userkeys, "scp.ini", player_id, key_write);
}
	
/*! \brief setap players keys */
void ControlHuman_setup(int config_num)
{

	if (config_num < 0)
		config_num = 0;
	if (config_num > 3)
		config_num = 3;

	load_keys(config_num);


	int i, t = 0;
	int last = 0;
	while (true) {
		char *s;
		int index = 0;

		s = dialog_string[index]; index += 1;

		s += sprintf(s, "Set key configuration %i", config_num+1);
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "Left:           ");
		key_to_description(userkeys.left, s);
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "Right:          ");
		key_to_description(userkeys.right, s);
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "Thrust:         ");
		key_to_description(userkeys.thrust, s);
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "Backwards:      ");
		key_to_description(userkeys.back, s);
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "Fire:           ");
		key_to_description(userkeys.fire, s);
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "Special:        ");
		key_to_description(userkeys.special, s);
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "AltFire:        ");
		key_to_description(userkeys.altfire, s);
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "Next Target:    ");
		key_to_description(userkeys.next, s);
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "Prev Target:    ");
		key_to_description(userkeys.prev, s);
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "Closest Target: ");
		key_to_description ( userkeys.closest, s );
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "Extra1: ");
		key_to_description ( userkeys.extra1, s );
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "Extra2: ");
		key_to_description ( userkeys.extra2, s );
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "Communicate: ");
		key_to_description ( userkeys.communicate, s );
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "Decrease Lag: ");
		key_to_description ( userkeys.dec_lag, s );
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "Increase Lag: ");
		key_to_description ( userkeys.inc_lag, s );
		s = dialog_string[index]; index += 1;

		s += sprintf ( s, "Suicide: ");
		key_to_description ( userkeys.suicide, s );

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
		if (t == KEY_ESC)
			return;

		switch (i) {
			case 1:  userkeys.left        = t; break;
			case 2:  userkeys.right       = t; break;
			case 3:  userkeys.thrust      = t; break;
			case 4:  userkeys.back	     = t; break;
			case 5:  userkeys.fire        = t; break;
			case 6:  userkeys.special     = t; break;
			case 7:  userkeys.altfire     = t; break;
			case 8:  userkeys.next        = t; break;
			case 9:  userkeys.prev        = t; break;
			case 10: userkeys.closest     = t; break;
			case 11: userkeys.extra1      = t; break;
			case 12: userkeys.extra2      = t; break;
			case 13: userkeys.communicate = t; break;
			case 14: userkeys.dec_lag     = t; break;
			case 15: userkeys.inc_lag     = t; break;
			case 16: userkeys.suicide     = t; break;
			case KEY_DIALOG_OK:  write_keys(config_num); break;
			case KEY_DIALOG_CANCEL: load_keys(config_num); break;
			case KEY_DIALOG_CALIBRATE: calibrate_joysticks(); break;

			case KEY_DIALOG_PREV_PLAYER:
				--config_num;
				if (config_num < 0) config_num = 3;
				load_keys(config_num);
				break;

			case KEY_DIALOG_NEXT_PLAYER:
				++config_num;
				if (config_num > 3) config_num = 0;
				load_keys(config_num);
				break;
		}
	}
}


