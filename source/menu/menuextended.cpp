

#include <allegro.h>
#include <stdio.h>

#include "../scp.h"
#include "../gui.h"
#include "../melee.h"
#include "menugeneral.h"

#include "../melee/mcontrol.h"
#include "../melee/mgame.h"

//  - dialog structure
DIALOG select_game_dialog[] = {
  // (dialog proc)     (x)  (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_shadow_box_proc, 160, 120,  320,  240,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_text_proc,       180, 135,  280,  190,  255,  0,    0,    0,       0,    0,    (void *)"Select a game", NULL, NULL},
  { d_list_proc2,      180, 155,  280,  190,  255,  0,    0,    D_EXIT,  0,    0,    (void *)genericListboxGetter, NULL, game_names },//doesn't hold the right value until main() begins
  { d_tw_yield_proc,   0,   0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,   0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};


const char *select_game_menu () {
	select_game_dialog[2].dp3 = game_names;
	set_config_file("client.ini");
	select_game_dialog[2].d1 = get_config_int("Menu", "SelectGame", 0);
	int i = tw_popup_dialog(NULL, select_game_dialog, 2);
	if (i == -1) return NULL;
	else {
		set_config_int("Menu", "SelectGame", select_game_dialog[2].d1);
		return game_names[select_game_dialog[2].d1];
	}
}



