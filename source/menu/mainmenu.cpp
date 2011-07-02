
#include <allegro.h>
#include <stdio.h>
#include <string.h>

#include "mainmenu.h"
#include "menugeneral.h"
#include "../gui.h"
#include "../scp.h"

#include "menuextended.h"




MainMenu mainmenu;

void play_single(const char *_gametype_name, Log *_log);
void play_net( bool ishost );



SAMPLE * menuAccept = NULL;
SAMPLE * menuFocus = NULL;
SAMPLE * menuDisabled = NULL;
SAMPLE * menuSpecial = NULL;
/** 
  loads up the title screen and music, and starts playing the background menu music. 
*/
void prepareTitleScreenAssets() {
  scp = load_datafile("scpgui.dat");
  if (!scp)
      throw("Couldnt load title music");
  
  Music * mymusic = sound.load_music("TitleScreen.dat#TITLEMUSIC");

  //Music * mymusic = load_mod("scpgui.dat#SCPMUSIC");

  if (!mymusic && sound.is_music_supported())
     throw("Couldnt load title music");

  if (mymusic) sound.play_music( mymusic, TRUE);

  {DATAFILE * data = load_datafile_object("TitleScreen.dat", "MENUACCEPT");
  if (data != NULL && data->type==DAT_SAMPLE) {
      menuAccept = (SAMPLE*) data->dat;
  }}

  {DATAFILE * data = load_datafile_object("TitleScreen.dat", "MENUFOCUS");
  if (data != NULL && data->type==DAT_SAMPLE) {
      menuFocus = (SAMPLE*) data->dat;
  }}

  {DATAFILE * data = load_datafile_object("TitleScreen.dat", "MENUDISABLED");
  if (data != NULL && data->type==DAT_SAMPLE) {
      menuDisabled = (SAMPLE*) data->dat;
  }}

  {DATAFILE * data = load_datafile_object("TitleScreen.dat", "MENUSPECIAL");
  if (data != NULL && data->type==DAT_SAMPLE) {
      menuSpecial = (SAMPLE*) data->dat;
  }}
  
}


enum {
//	MAIN_DIALOG_BOX = 0,
	MAIN_DIALOG_NET_JOIN = 0,
	MAIN_DIALOG_NET_HOST,
	MAIN_DIALOG_MELEE,
	MAIN_DIALOG_SC1ARENA,
	MAIN_DIALOG_OTHER_GAME,
	MAIN_DIALOG_EDITFLEET,
	MAIN_DIALOG_TEAMS,
	MAIN_DIALOG_OPTIONS,
	MELEE_EX_DIALOG_SHIPINFO,
	MAIN_DIALOG_HELP,
	MAIN_DIALOG_EXIT

};

DIALOG mainDialog[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
//  { d_shadow_box_proc, 40,   40,   180,  285,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { my_d_button_proc,  45,   45,   170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Join" , NULL, NULL }, //MAIN_DIALOG_NET_JOIN
  { my_d_button_proc,  45,   80,   170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Host" , NULL, NULL }, //MAIN_DIALOG_NET_HOST
  { my_d_button_proc,  45,   115,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Melee" , NULL, NULL }, //MAIN_DIALOG_MELEE
  { my_d_button_proc,  45,   150,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"SC1 Arena" , NULL, NULL }, //MAIN_DIALOG_SC1ARENA
  { my_d_button_proc,  45,   185,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Other game" , NULL, NULL },
  { my_d_button_proc,  250,  45,   170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Edit Fleet", NULL, NULL },
  { my_d_button_proc,  250,  80,   170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Edit Teams" , NULL, NULL },
  { my_d_button_proc,  250,  115,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Options", NULL, NULL },
  { my_d_button_proc,  250,  150,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Ship Info", NULL, NULL },//MELEE_EX_DIALOG_SHIPINFO
  { my_d_button_proc,  250,  185,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Help", NULL, NULL },
  { my_d_button_proc,  150,  220,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Exit", NULL, NULL },
 { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,       1,    0,    NULL, NULL, NULL }
};


void MainMenu::_event(Event *e) {
	if (e->type == Event::VIDEO) {
		if (e->subtype == VideoEvent::REDRAW) if (state & 1) showTitle();
	}
}

void MainMenu::enable() { 
	if (!(state & 2)) window->add_callback(this);
	state |= 3;
}

void MainMenu::disable() { 
	state &=~ 1;
}

void MainMenu::preinit() {
	window = NULL;
	state = 0;
}

void MainMenu::init(VideoWindow *parent) {
	if (window) window->init(parent);
	else {
		window = new VideoWindow();
		window->preinit();
		window->init(parent);
	}
}

void MainMenu::deinit() {
	 
	if (state & 2) {
		window->remove_callback(this);
		window->deinit();
		delete window;
		window = NULL;
	}
}

void MainMenu::doit() { 
	int i;
	char tmp[32];

	set_config_file("scp.ini");
	if (!player_type) {		
		max_networkS        = get_config_int("Limits", "MaxPlayers", 12);
		MAX_CONFIGURATIONS = get_config_int("Limits", "MaxConfigurations", 4);
		MAX_TEAMS          = get_config_int("Limits", "MaxTeams", 6);
		player_type = new char*[max_networkS];
		player_config = new int[max_networkS];
		player_team   = new int[max_networkS];
	}
	for (i = 0; i < max_networkS; i += 1) {
		sprintf(tmp, "Player%d", i+1);
		player_type[i] = strdup(get_config_string(tmp, "Type", "Human"));
		player_config[i] = get_config_int (tmp, "Config", i % MAX_CONFIGURATIONS);
		player_team[i] = get_config_int (tmp, "Team", 0);
	}

    prepareTitleScreenAssets();
	showTitle();
	enable();

	int mainRet;
	do {

		//mainRet = popup_dialog(mainDialog, MAIN_DIALOG_MELEE);
		mainRet = tw_do_dialog(window, mainDialog, MAIN_DIALOG_MELEE);
		switch (mainRet)
		{
			case MAIN_DIALOG_MELEE:
				disable();
				play_single("Melee");
				enable();
				showTitle();
				break;

			case MAIN_DIALOG_SC1ARENA:
				disable();
				play_single("SC1 Arena");
				enable();
				showTitle();
				break;
			
			case MAIN_DIALOG_NET_HOST:
			case MAIN_DIALOG_NET_JOIN:
				disable();
				play_net(mainRet == MAIN_DIALOG_NET_HOST);
				enable();
				showTitle();
				break;

			case MAIN_DIALOG_OTHER_GAME:
				{
				const char *gname = select_game_menu();
				disable();
				if (gname) play_single(gname);
				enable();
				showTitle();
				break;
				}

			case MAIN_DIALOG_OPTIONS:
				options_menu(NULL);
				showTitle();
				break;

			case MAIN_DIALOG_HELP:
				show_file("ingame.txt");
				showTitle();
				break;

			case MAIN_DIALOG_TEAMS:
				change_teams();
				showTitle();
				break;

			case MAIN_DIALOG_EDITFLEET:
				edit_fleet(0);
				showTitle();
				break;

			case MELEE_EX_DIALOG_SHIPINFO:
				ship_view_dialog(0, NULL);
				showTitle();
				break;

		}
	} while((mainRet != MAIN_DIALOG_EXIT) && (mainRet != -1));

}

