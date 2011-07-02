
#include <stdio.h>
#include <string.h>

#include "menugeneral.h"
#include "../scp.h"
#include "../util/aastr.h"

#include "../melee/mcontrol.h"

char **player_type = NULL;
int *player_config = NULL;
int *player_team = NULL;


void showTitle(VideoWindow *window) {
	if (!scp) {
		throw ("showTitle - gui stuff not loaded");
		return;
	}

	BITMAP *src = (BITMAP *) scp[SCPGUI_TITLE].dat;

	if (!window->surface)
		return;
	window->lock();
	//aa_stretch_blit(src, window->surface, 0, 0, src->w, src->h, 0, 0, screen->w, screen->h);
	aa_set_mode(AA_DITHER);
	aa_stretch_blit(src, window->surface, 
		0,0,src->w,src->h,
		window->x, window->y, window->w, window->h);
	//blit(src, window->surface, 
	//	0,0,0,0,src->w,src->h);
	window->unlock();
	return;
}




char *playerListboxGetter(int index, int *list_size) {
	static char buf[160];
	char *tmp = buf;

	tmp[0] = 0;
	if(index < 0) {
		*list_size = max_networkS;
		return NULL;
	} else {
		tmp += sprintf(tmp, "Player%d", index + 1);
		if (index + 1 < 10) tmp += sprintf(tmp, " ");
		tmp += sprintf(tmp, "   %d   %d   %s", player_team[index], player_config[index], player_type[index]);
		if ((strlen(buf) >= 80)) throw("playerListboxGetter string too long");
		return buf;
	}
}

char *controlListboxGetter(int index, int *list_size) {
	static char tmp[40];

	tmp[0] = 0;
  if(index < 0) {
    *list_size = num_controls;
    return NULL;
  } else {
    return(control_name[index]);
  }
}

