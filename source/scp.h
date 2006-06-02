/* $Id$ */ 
#ifndef __SCP_H__
#define __SCP_H__

#include "melee/mlog.h"
#include "melee/mview.h"

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
bool is_bot (int channel);
void log_file (const char *fname);
bool log_empty(int channel = channel_current);

void test_net();


extern bool optimize4latency;


extern SAMPLE * menuAccept;
extern SAMPLE * menuFocus;
extern SAMPLE * menuDisabled;
extern SAMPLE * menuSpecial;

extern BITMAP *video_screen;

extern bool game_networked;


void play_game(const char *_gametype_name, Log *_log = NULL) ;

void play_single(const char *_gametype_name, Log *_log = NULL);
void play_net (bool ishost);
void play_demo(const char *file_name = "demo.dmo") ;

// probably outdated:
void play_net1client ( const char * address = NULL, int port = -1 ) ;
void play_net1server ( const char *_gametype_name, int port = -1 ) ;


/*! \brief MELEE_EX dialog - selects alternate games as opposed to standard melee. 
  From here you can access diagnostics (DIAGNOSTICS dialog) and ship info. 
  (SHIPVIEW dialog) You can also test key jamming from this dialog.
 */
void extended_menu(int i = -1);

/*! \brief TEAMS dialog - from here you can select controllers for each player, and access their respective fleets. (FLEET dialog)
 */
void change_teams();

/*! \brief FLEET dialog - manages fleet compositions for an individual player.
  \param player Player index indicating which player's fleet to edit.
 */
void edit_fleet(int player);

/*! \brief SHIPVIEW dialog - displays statistics and text information (if available) about the currently installed ships.
  \param si Ship index. By default 0, the first ship in the fleet.
  \param fleet Pointer to a fleet upon which the ship list is built. When 
  this parameter is set to the default value NULL, the reference fleet is 
  used to build the ship list.
*/
//void ship_view_dialog(int si = 0, Fleet *fleet = NULL);

/*! \brief DIAGNOSTICS dialog - displays version number and platform data */
void show_diagnostics();

/*! \brief Opens a screen showing which keys are currently pressed. Here the user may test various key combinations for conflicts. */
void keyjamming_tester();

extern DATAFILE *scp;


//deprecated - should be replaced with something that doesn't depend on data file-content-ordering
#define SCPGUI_MUSIC   0
#define SCPGUI_TITLE   1

extern int max_networkS;
extern int MAX_CONFIGURATIONS;
extern int MAX_TEAMS;

extern Game *old_game;

#endif // __SCP_H__
