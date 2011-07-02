

#include <allegro.h>
#include <stdio.h>

#include "../scp.h"
#include "../gui.h"
#include "../melee.h"
#include "menugeneral.h"

#include "../melee/mcontrol.h"
#include "../melee/mgame.h"

#include "editkeys.h"

/*
Control *load_player(int i) { 
	char tmp[32];
	Control *r = NULL;

	sprintf (tmp, "Config%d", player_config[i]);
	r = getController(player_type[i], tmp, channel_none);
	if (r)
		r->load("scp.ini", tmp);
	return r;
}
*/

// TEAMS - dialog objects
enum {
	TEAMS_DIALOG_BOX = 0,
	TEAMS_DIALOG_TITLE,
	TEAMS_DIALOG_PLAYERLIST_TEXT,
	TEAMS_DIALOG_PLAYERLIST,
	TEAMS_DIALOG_CONTROLLIST,
	TEAMS_DIALOG_NETTEAMS,
	TEAMS_DIALOG_SELECTCONTROL,
	TEAMS_DIALOG_TEAM_NUM,
	TEAMS_DIALOG_CONFIG_NUM,
	TEAMS_DIALOG_SETUP,
	TEAMS_DIALOG_FLEET,
	TEAMS_DIALOG_MAINMENU
};

// TEAMS - dialog structure
DIALOG teamsDialog[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_box_proc,        35,   35,   420,  385,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_textbox_proc,    150,  40,   200,  25,   255,  0,    0,    0,       0,    0,    (void *)"Teams Dialog", NULL, NULL },
  { d_text_proc,       40,   70,   240,  160,  255,  0,    0,    D_EXIT,  0,    0,    (void *)" Player   Team Config Type", NULL, NULL },
  { d_list_proc,       40,   85,   240,  145,  255,  0,    0,    D_EXIT,  0,    0,    (void *)playerListboxGetter, NULL, NULL },
  { d_list_proc,       290,  70,   160,  160,  255,  0,    0,    D_EXIT,  0,    0,    (void *)controlListboxGetter, NULL, NULL },
  { d_check_proc,      100,  236,  120,  20,   255,  0,    0,    0,       1,    0,    (void *)"Teams in Net Games",    NULL, NULL },
  { my_d_button_proc,  295,  240,  150,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Select Controller", NULL, NULL },
  { my_d_button_proc,  50,   255,  220,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Change Team #", NULL, NULL },
  { my_d_button_proc,  50,   285,  220,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Change Config #", NULL, NULL },
  { my_d_button_proc,  50,   315,  220,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Edit Config", NULL, NULL },
  { my_d_button_proc,  50,   345,  220,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Edit Fleet", NULL, NULL },
  { d_button_proc,     90,   380,  220,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Main Menu", NULL, NULL },
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};


// TEAMS - dialog function
void change_teams() { 
	int a, i;

	set_config_file("scp.ini");

	teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1 = 0;
	teamsDialog[TEAMS_DIALOG_NETTEAMS].flags = 
		twconfig_get_int("/ini/client.ini/network/NetworkMeleeUseTeams")
		? D_SELECTED : 0;

	while (1) {
		dialog_string[0][0] = 0;
		sprintf(dialog_string[0], "Config #");


		a = tw_do_dialog(NULL, teamsDialog, 0);
		if((a == TEAMS_DIALOG_SELECTCONTROL) || (a == TEAMS_DIALOG_CONTROLLIST)) {
			player_type[teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1] =
				control_name[teamsDialog[TEAMS_DIALOG_CONTROLLIST].d1];
			teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1 += 1;
		}
		else if ((a == TEAMS_DIALOG_SETUP) || (a == TEAMS_DIALOG_PLAYERLIST))
		{
			int iplayer = player_config[teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1];
			showTitle();
			ControlHuman_setup(iplayer);
			showTitle();
			/*
			Control *tmpc = load_player(teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1);
			if (tmpc) {
				showTitle();
				tmpc->setup();
				delete tmpc;
				showTitle();
			}
			*/
		}
		else if (a == TEAMS_DIALOG_TEAM_NUM) {
			player_team[teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1] += 1;
			player_team[teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1] %= MAX_TEAMS;
		}
		else if (a == TEAMS_DIALOG_CONFIG_NUM) {
			player_config[teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1] += 1;
			player_config[teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1] %= MAX_CONFIGURATIONS;
		}
		else if (a == TEAMS_DIALOG_FLEET) {
			edit_fleet(teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1);
			showTitle();
		}
		else break;
	}

	set_config_file("scp.ini");
	for (i = 0; i < max_networkS; i += 1) {
		sprintf(dialog_string[0], "Player%d", i+1);
		set_config_string (dialog_string[0], "Type", player_type[i]);
		set_config_int (dialog_string[0], "Config", player_config[i]);
		set_config_int (dialog_string[0], "Team", player_team[i]);
	}
	twconfig_set_int("/cfg/client.ini/network/NetworkMeleeUseTeams", 
		(teamsDialog[TEAMS_DIALOG_NETTEAMS].flags & D_SELECTED) ? 1 : 0);

	return;
}


