#ifndef _SCP_H
#define _SCP_H

const char *tw_version();

void change_teams();
void edit_fleet(int player) ;
void change_options() ;
int connect_menu(VideoWindow *window, char **address, int *port) ;
int is_escape_pressed() ;
void ship_view_dialog(int si = 0, class Fleet *fleet = NULL);

#endif
