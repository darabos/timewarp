
#ifndef _MENU_GENERAL
#define _MENU_GENERAL

#include "../melee.h"

extern char **player_type;
extern int *player_config;
extern int *player_team;

/*! \brief Blits GUI background bitmap on to a video window */
void showTitle(VideoWindow *window = &videosystem.window);


// list box getter functions
char *playerListboxGetter(int index, int *list_size) ;
char *controlListboxGetter(int index, int *list_size) ;
char *viewListboxGetter(int index, int *list_size) ;


#endif
