/* $Id$ */ 
#ifndef __SCP_H__
#define __SCP_H__

#include "melee/mlog.h"

const char *tw_version();

void change_teams();
void edit_fleet(int player) ;
//void change_options() ;
int connect_menu(VideoWindow *window, char **address, int *port) ;
int is_escape_pressed() ;
void ship_view_dialog(int si = 0, class Fleet *fleet = NULL);

extern Log *glog; //logging system for networking, demo recording/playback, etc.
void set_global(Log *somelog);
void log_char (char &data, int channel = channel_current);             //helper for using the logging system
void log_char (unsigned char &data, int channel = channel_current);
void log_short(short &data, int channel = channel_current);            //helper for using the logging system
void log_short(unsigned short &data, int channel = channel_current);
void log_int  (int &data, int channel = channel_current);              //helper for using the logging system
void log_int  (unsigned int &data, int channel = channel_current);
void log_data (void *data, int length, int channel = channel_current); //helper for using the logging system
bool is_local (int channel);
void log_file (const char *fname);

void test_net();


extern SAMPLE * menuAccept;
extern SAMPLE * menuFocus;
extern SAMPLE * menuDisabled;
extern SAMPLE * menuSpecial;

#endif // __SCP_H__
