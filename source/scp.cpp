/*
 * Star Control - TimeWarp
 *
 * scp.cpp - Main project module
 *
 * 25-Jun-2002
 *
 * - key jamming tester revised to accomodate joystick.
 * - re-added d_tw_yeildslice stuff
 * - improved "debug" button
 * - SHIPVIEW dialog object positions and size modified.
 * - SHIPVIEW dialog function modified to load text and ini files according to shp*.*
 *   file naming convention.
 * - SHIPVIEW dialog ship list sort order selection implemented.
 * - Cosmetic code changes.
 * - Comments added.
 */

#define INCLUDE_GAMEX

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <allegro.h>

#include "jpgalleg/jpgalleg.h"

#ifdef ALLEGRO_MSVC
	#pragma warning (disable:4786)
#endif

#ifdef WIN32
	#include <allegro/platform/aintwin.h>
#endif

#ifdef ALLEGRO_MSVC
	#include <winalleg.h>
	#include <crtdbg.h>
#endif 


DATAFILE *scppal = NULL;

const char *tw_version() {
	static char tw_version_string[1024];
	sprintf(tw_version_string, "%s %s", __TIME__, __DATE__);
	return tw_version_string;
}

#include "melee.h"
REGISTER_FILE
#include "libs.h"
#include "scp.h"
#include "gui.h"
#include "frame.h"

#include "util/get_time.h"
#include "util/profile2.h"

#include "melee/mview.h"
#include "melee/mcontrol.h"
#include "melee/mcbodies.h"
#include "melee/mgame.h"


#include "melee/mmain.h"
#include "melee/mnet1.h"
#include "games/ggob.h"
#include "util/net_tcp.h"
#include "util/aastr.h"
#include "melee/mship.h" //remove
#include "melee/mfleet.h"

#include "util/sounds.h"


//deprecated.  This mode of using dat files is terrible, I can't believe
//this technique was ever created.
#define SCPGUI_MUSIC   0
#define SCPGUI_TITLE   1

#ifdef INCLUDE_GAMEX
	#include "gamex/projectx.h"
// for future use (Rob)
#endif

Game *old_game = NULL;

DATAFILE *scp = NULL;

FILE *debug_file;

/*! \brief Blits GUI background bitmap on to a video window */
void showTitle(VideoWindow *window = &videosystem.window);

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


void play_game(const char *_gametype_name, Log *_log = NULL) ;

void play_single(const char *_gametype_name, Log *_log = NULL);
void play_net (bool ishost);
void play_demo(const char *file_name = "demo.dmo") ;

// probably outdated:
void play_net1client ( const char * address = NULL, int port = -1 ) ;
void play_net1server ( const char *_gametype_name, int port = -1 ) ;

int getKey();

enum {
	MAIN_DIALOG_BOX = 0,
	MAIN_DIALOG_MELEE,
	MAIN_DIALOG_NET_JOIN,
	MAIN_DIALOG_NET_HOST,
	MAIN_DIALOG_MELEE_EXTENDED,
	MAIN_DIALOG_TEAMS,
	MAIN_DIALOG_OPTIONS,
	MAIN_DIALOG_HELP,
	MAIN_DIALOG_EXIT,
#ifdef INCLUDE_GAMEX
	MAIN_DIALOG_FG,
#endif
};

DIALOG mainDialog[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_shadow_box_proc, 40,   40,   180,  215,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { my_d_button_proc,  45,   45,   170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Melee" , NULL, NULL },
  { my_d_button_proc,  250,  45,   170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Join" , NULL, NULL }, //MAIN_DIALOG_NET_JOIN
  { my_d_button_proc,  250,  80,   170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Host" , NULL, NULL }, //MAIN_DIALOG_NET_HOST
  { my_d_button_proc,  45,   80,   170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Extended Menu" , NULL, NULL },
  { my_d_button_proc,  45,   115,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Teams" , NULL, NULL },
  { my_d_button_proc,  45,   150,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Options", NULL, NULL },
  { my_d_button_proc,  45,   185,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Help", NULL, NULL },
  { my_d_button_proc,  45,   220,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Exit", NULL, NULL },
#ifdef INCLUDE_GAMEX
  { my_d_button_proc, 550,   440,   50,  30,   255,  0,    0,    D_EXIT | D_SPECIAL_BUTTON,
                                                                             0,    0,    (void *)"FG" , NULL, NULL },
#endif
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,       1,    0,    NULL, NULL, NULL }
};


#define FLEET_TITLE_DIALOG_BOX    0
#define FLEET_TITLE_DIALOG_EDIT   1
#define FLEET_TITLE_DIALOG_OK     2
#define FLEET_TITLE_DIALOG_CANCEL 3

char title_str[80];

DIALOG fleet_titleDialog[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_box_proc,        180,  210,  280,  60,   255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_edit_proc,       190,  220,  260,  10,   255,  0,    0,    0,       80,   0,    (void *) title_str, NULL, NULL },
  { my_d_button_proc,  255,  240,  60,   18,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"OK", NULL, NULL },
  { d_button_proc,     325,  240,  60,   18,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Cancel", NULL, NULL },
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};



Log *new_log (int logtype) { STACKTRACE
	union { Log *log; NetLog *netlog; };
	log = NULL;

	switch (logtype) {
		case Log::log_normal: {
			log = new Log();
			log->init();
			return log;
		}
		case Log::log_net1server: {
			netlog = new NetLog();
			netlog->init();
			netlog->type = Log::log_net1server;
			return netlog;
		}
		default: {
			tw_error("that is not a valid log type");
		}
	}
	return NULL;
}


// it's probably dangerous if this is declared inside this subroutine...
static char buffy[1024];
char *detect_gametype( Log *_log ) { STACKTRACE
	int ltype;
	_log->unbuffer(channel_init, &ltype, sizeof(int));
	ltype = intel_ordering(ltype);
	int gnamelength;
	_log->unbuffer(channel_init, &gnamelength, sizeof(int));
	gnamelength = intel_ordering(gnamelength);
	if (gnamelength > 1000) {
		tw_error("Game name too long");
		gnamelength = 1000;
	}
	//_log->unbuffer(channel_init, &buffy, gnamelength);
	_log->unbuffer(channel_init, buffy, gnamelength);
	buffy[gnamelength] = 0;
	_log->reset();
	return strdup(buffy);
}

void share_string(char *str)
{
	STACKTRACE;

	int Lstr;
	Lstr = strlen(str) + 1;
	share(-1, &Lstr);
	share_update();
	share(-1, str, Lstr);	
	share_update();

	message.print(1500, 14, "SHARED [%i] [%s]", Lstr, str);
	message.animate(0);
}



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
      tw_error("Couldnt load title music");
  
  Music * mymusic = sound.load_music("TitleScreen.dat#TITLEMUSIC");

  //Music * mymusic = load_mod("scpgui.dat#SCPMUSIC");

  if (!mymusic && sound.is_music_supported())
     tw_error("Couldnt load title music");

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

/** clears the screen, and displays a loading message to the user.
*/
void showLoadingScreen() {
    static BITMAP * logo = NULL;
    static int depth = bitmap_color_depth(screen);

    acquire_screen();
    clear_to_color(screen, palette_color[0]);

    if (NULL == logo || bitmap_color_depth(screen) != depth) 
    {
        depth = bitmap_color_depth(screen);
        
        DATAFILE * data = load_datafile_object("TitleScreen.dat","LOGO");
        if (data != NULL && data->type==DAT_BITMAP) {            
            BITMAP * temp = (BITMAP*) data->dat;
            logo = create_bitmap(temp->w, temp->h);
            blit(temp, logo, 0,0, 0,0, temp->w, temp->h);
            unload_datafile_object(data);
        }
    }

    if (logo != NULL ) {
        if (screen->w/2 >= logo->w) {
            draw_sprite(screen, logo, screen->w/2 - logo->w/2, screen->h/2 - logo->h/2);
        }
        else {
            float ratio = logo->w / logo->h;
            
            int h = screen->h/4;
            int w = iround(ratio * h);                

            stretch_blit(logo, screen, 
                0,0, 
                logo->w, logo->h,
                screen->w/2 - w/2, screen->h/2 - h/2,
                w, h);
        }
    }

    const char * loadString = "Loading...";
    textout_right(screen, font, loadString, 
        screen->w - 1*text_length(font, loadString), screen->h - 4*text_height(font), 
        palette_color[15]);
    release_screen();
}


enum {
	DIALOG_CONNECT_BOX = 0,
	DIALOG_CONNECT_TITLE,
	DIALOG_CONNECT_ADDRESS,
	DIALOG_CONNECT_ADDRESS_BOX,
	DIALOG_CONNECT_ADDRESS_EDIT,
	DIALOG_CONNECT_PORT,
	DIALOG_CONNECT_PORT_BOX,
	DIALOG_CONNECT_PORT_EDIT,
//	DIALOG_CONNECT_TWOLOCALS,
	DIALOG_CONNECT_OK,
	DIALOG_CONNECT_CANCEL,
	DIALOG_CONNECT_BLAH
};

static DIALOG connect_dialog[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_box_proc,        40,   100,  480,  260,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_text_proc,       50,   110,  460,  30,   255,  0,    0,    0,       0,    0,    (void *)"Connect to server:" , NULL, NULL },
  { d_text_proc,       50,   140,  460,  30,   255,  0,    0,    0,       0,    0,    (void *)"IP Address" , NULL, NULL },
  { d_box_proc,        48,   168,  464,  34,   255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_edit_proc,       50,   170,  460,  30,   255,  0,    0,    0,       75,   0,    dialog_string[0], NULL, NULL },
  { d_text_proc,       50,   220,  460,  30,   255,  0,    0,    0,       0,    0,    (void *)"Port #" , NULL, NULL },
  { d_box_proc,        48,   248,  464,  34,   255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_edit_proc,       50,   250,  460,  30,   255,  0,    0,    0,       75,   0,    dialog_string[1], NULL, NULL },
//{ d_check_proc,      50,   290,  160,  20,   255,  0,    0,    0,       0,    0,    (void *)"Two Local Players", NULL, NULL },
  { my_d_button_proc,  50,   320,  160,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Connect", NULL, NULL },
  { d_button_proc,     350,  320,  160,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Abort", NULL, NULL },
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

int connect_menu(VideoWindow *window, char **address, int *port) { STACKTRACE
	int i = 0;

	if (*address)
		strncpy(dialog_string[0], *address, 70);
	if (port)
		sprintf(dialog_string[1], "%d", *port);

	i = tw_popup_dialog(window, connect_dialog, DIALOG_CONNECT_ADDRESS_EDIT);
	if (i != DIALOG_CONNECT_OK)
		return -1;
	*port = atoi(dialog_string[1]);
	*address = strdup(dialog_string[0]);
	return 0;
}


//#define DIALOG_LISTEN_TWOLOCALS 4
#define DIALOG_LISTEN_OK        4
#define DIALOG_LISTEN_CANCEL    5

static DIALOG listen_dialog[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)  (d2)  (dp)
  { d_box_proc,        120,  100,  300,  140,  255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
  { d_text_proc,       140,  110,  240,  25,   255,  0,    0,    0,          0,    0,    (void*)"Listen on which port?", NULL, NULL },
  { d_box_proc,        178,  138,  124,  29,   255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
  { d_edit_proc,       180,  140,  120,  25,   255,  0,    0,    0,          5,    0,    dialog_string[1], NULL, NULL },
//{ d_check_proc,      140,  170,  160,  20,   255,  0,    0,    0,          0,    0,    (void *)"Two Local Players", NULL, NULL },
  { my_d_button_proc,  140,  200,  120,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void*) "OK", NULL, NULL },
  { d_button_proc,     280,  200,  120,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void*) "Cancel", NULL, NULL },
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,          0,    0,    NULL, NULL, NULL }
};

static int listen_menu(VideoWindow *window, int port) { STACKTRACE
	dialog_string[1][0] = '\0';
	int p = -1;
	sprintf(dialog_string[1], "%d", port);
	while ((p <= 0) || (p > 65535)) {
		if (tw_popup_dialog(window, listen_dialog, 2) != DIALOG_LISTEN_OK) {
			//game->quit("Quit - game aborted from network \"listen\" menu");
			return -1;
		}
		p = atoi(dialog_string[1]);
	}
	return p;
}

int is_escape_pressed() {
	poll_keyboard();
	return key[KEY_ESC];
}

/*
static DIALOG clientWaiting[] = 
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)  (d2)  (dp)
  { d_box_proc,        120,  100,  300,  140,  255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
  { d_text_proc,       140,  110,  240,  25,   255,  0,    0,    0,          0,    0,    (void*)"Connecting to                                                    ", NULL, NULL },
  { d_button_proc,     280,  200,  120,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void*) "Cancel", NULL, NULL },
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,          0,    0,    NULL, NULL, NULL }
};
*/


Log *glog = 0; //logging system for networking, demo recording/playback, etc.

void log_char (unsigned char &data, int channel)
{
	log_char (*(char*)&data, channel);
}

void log_short(unsigned short &data, int channel)
{
	log_short (*(short*)&data, channel);
}

void log_int  (unsigned int &data, int channel)
{
	log_int (*(int*)&data, channel);
}

void set_global(Log *somelog)
{
	glog = somelog;
}

void log_char(char &data, int channel)
{
	if (!glog) return;
	glog->log  (channel, &data, 1);
	return;
}

void log_short(short &data, int channel)
{
	if (!glog) return;
	data = intel_ordering_short(data);	
	glog->log  (channel, &data, sizeof(short));
	data = intel_ordering_short(data);
	return;
}

void log_int(int &data, int channel)
{
	if (!glog) return;
	data = intel_ordering(data);
	glog->log  (channel, &data, sizeof(int));
	data = intel_ordering(data);
	return;
}

void log_data(void *data, int size, int channel)
{
	if (!glog) return;
	glog->log  (channel, data, size);
	return;
}

bool is_local (int channel) {
	return (glog->get_direction (channel) & Log::direction_write);
}

void log_file (const char *fname)
{
	glog->log_file(fname);
}


// Note that, because this also tests the unbuffered channel, it has to be inserted into the
// game in a predictable fashion, i.e., every game iteration.
void test_net()
{
	int test, test1, test2;
	
	test = 100*rand();
	log_int(test, channel_init);

	message.print(1500, 14, "SHARED [%i]", test);
	message.animate(0);

	int i;
	for ( i = 0; i < num_humans; ++i )
	{
		test1 = 100*rand();
		test2 = 100*rand();
		log_int(test1, channel_player[i]);

		log_int(test2, channel_player[i] + _channel_buffered);
		// note, that the unbuffered data are not physics, i.e., they can appear later in time.

		//glog->flush();
		//glog->listen();

		message.print(1500, 14, "SHARED P:[%i] unbuf[%i] buf[%i]", i, test1, test2);
		message.animate(0);
	}

//	if (game->frame_number % 25 == 0 && p_local == 0)
//		readkey();
}




/*


// just something which receives connections, and keeps a list of them at least, if
// they're game-hosts.
void play_address_server()
{
	NetLog *l = new NetLog();
	log->init();
	set_global(l);	// this sets glog

	// note that you can manage at most max_connections connections at once.

	if (!log->add_listen(port))
		tw_error("listener failed");

}


*/


#include "twgui/twgui.h"

// a textbutton displaying a integer value, which also takes 2 other buttons to tweak it's value
class ButtonValue : public TextButton
{
public:
	
	ButtonValue(TWindow *menu, char *identbranch, FONT *usefont);

	int value, vmin, vmax;

	Button *left, *right;

	virtual void calculate();

	void set_value(int v1, int v, int v2);
};


ButtonValue::ButtonValue(TWindow *menu, char *identbranch, FONT *usefont)
:
TextButton(menu, identbranch, usefont)
{

	char tmp[512];
	
	strcpy(tmp, identbranch);
	strcat(tmp, "dec_");
	left = new Button(menu, tmp);

	strcpy(tmp, identbranch);
	strcat(tmp, "inc_");
	right = new Button(menu, tmp);

	value = 0;
	vmin = 0;
	vmax = 0;

	passive = false;	// left/right click can also change value
}


void ButtonValue::set_value(int v1, int v, int v2)
{
	vmin = v1;
	vmax = v2;
	value = v;

	char tmp[512];
	sprintf(tmp, "%i", value);
	set_text(tmp, makecol(200,100,100));
}

void ButtonValue::calculate()
{
	TextButton::calculate();

	//if (bdec->flag.left_mouse_press || binc->flag.left_mouse_press)
	if (flag.left_mouse_press || left->flag.left_mouse_press)
	{
		--value;

		if (value < vmin )
			value = vmax;
		
		char tmp[512];
		sprintf(tmp, "%i", value);
		set_text(tmp, makecol(200,100,100));
	}

	if (flag.right_mouse_press || right->flag.left_mouse_press)
	{
		++value;

		if (value >= vmax)
			value = vmin;

		char tmp[512];
		sprintf(tmp, "%i", value);
		set_text(tmp, makecol(200,100,100));
	}
}



#include "other/ttf.h"

// CCstatus = Cancel or Continue.
void game_host_menu(int &Nhumans, int &Nbots, char *gname, int &CCstatus)
{
	// init


	TWindow *T;
	T = new TWindow("interfaces/multiplayer", 50,50, screen);

	int psize;
	psize = 40 * T->scale;
	FONT *usefont1 = load_ttf_font ("fonts/Jobbernole.ttf", psize, 0);
	psize = 20 * T->scale;
	FONT *usefont2 = load_ttf_font ("fonts/Jobbernole.ttf", psize, 0);
	
	Button *b_accept, *b_cancel;
	b_accept = new Button(T, "accept_");
	b_cancel = new Button(T, "cancel_");

	ButtonValue *b_human, *b_bot;

	b_human = new ButtonValue(T, "humans_", usefont1);
	b_human->set_value(1, Nhumans, 8);
	b_bot   = new ButtonValue(T, "bots_", usefont1);
	b_bot->set_value(0, Nbots, 7);

	TextButton *game_choice;
	game_choice = new TextButton(T, "type_", usefont1);
	game_choice->set_text(gname, makecol(200,200,200));
	game_choice->passive = false;	// normally it just show info, but now you need it for interaction.

	PopupList *rpopup;
	rpopup = new PopupList(game_choice, "interfaces/multiplayer/gamelist", "list_", 10, 10, usefont2, 0);
	rpopup->tbl->set_optionlist(game_names, num_games, makecol(200,200,200));
	rpopup->hide();

	T->add(rpopup);

	T->doneinit();
	rpopup->doneinit();

	rpopup->layer = 1;
	T->layer = 2;		// always shown later

	CCstatus = 0;

	// I/O
	for(;;)
	{
		poll_input();
		poll_mouse();
		show_mouse(screen);

		T->tree_calculate();


		// extra check:
		if (b_human->value + b_bot->value > max_player)
		{
			int k = max_player - b_human->value;
			b_bot->set_value(0, k, 7);
		}

		if (b_cancel->flag.left_mouse_press)
			CCstatus = -1;
		if (b_accept->flag.left_mouse_press)
			CCstatus = 1;
		if (CCstatus != 0)
			break;

		if (rpopup->ready())
		{
			int k = rpopup->getvalue();
			if (k >= 0 && k < num_games)
				game_choice->set_text(game_names[k], makecol(200,200,200));
			clear_to_color(screen, 0);	// erase the screen
		}

		T->tree_animate();

		idle(5);
	}

	Nhumans = b_human->value;
	Nbots = b_bot->value;
	strcpy(gname, game_choice->text);

	// exit
	delete T;

	// for some reason, this crashes ?!
	// NEED TO CHECK, WHY !!
//	destroy_font(usefont1);
//	destroy_font(usefont2);
}




//void play_net ( const char *_address, int _port )
void play_net (bool ishost)
{
	STACKTRACE;

	// resets the channels to their default values ; that's needed, cause if there are
	// bots in the game, channel values can be set to non-human values (-1)
	init_channels();


	// STEP ONE, SETTING / RETRIEVING DATA

	NetLog *log = new NetLog();
	log->init();
	set_global(log);	// this sets glog

	set_config_file("client.ini");

	//int ishost;
	//ishost = get_config_int("Network", "Host", 0);

	int port;
	port = get_config_int("Network", "Port", 15515);
	
	char address[128];
	strncpy(address, get_config_string("Network", "Address", ""), 127);

	char gname[512];

	int Nplayers = get_config_int("Network", "Nhumans", 2);
	int Nbots = get_config_int("Network", "Nbots", 0);
	char temp_gamename[512];
	strcpy(temp_gamename, get_config_string("Network", "GameName", "Melee"));

	if (ishost)
	{
		log->type = Log::log_net1server;

		int status = -1;
		game_host_menu(Nplayers, Nbots, temp_gamename, status);

//		status = 1;
//		Nplayers = 2;
//		Nbots = 0;

		if (status == -1)
			return;		// cancel
		// otherwise, continue as planned.
		set_config_file("client.ini");
		set_config_int("Network", "Nhumans", Nplayers);
		set_config_int("Network", "Nbots", Nbots);
		set_config_string("Network", "GameName", temp_gamename);
		
		p_local = 0;		// IMPORTANT TO SET THIS !!

		// channel_init and such, are all write for the one who hosts.

		int i;
		for ( i = 0; i <= channel_playback; ++i )
		{
			// things like channel_init, channel_file_data, channel_playback ...
			// (dunno for sure if channel_playback should be rw, though).
			log->set_direction(i, Log::direction_read | Log::direction_write | Log::direction_immediate);
		}
		log->set_r(channel_playback);	// perhaps this is a better setting for the playback channel
		log->set_rw(channel_local());
		//log->set_rw(channel_none);	// should this be rw? No, cause channel_none == -1, doesn't exist

		// the remote players are always read-only (local player is always write, of course)
		int p;
		for ( p = 0; p < max_player; ++p)
		{
			if (p != p_local)
			{
				//log->set_direction(channel_player[p] , Log::direction_read);
				//log->set_direction(channel_player[p] + _channel_buffered, Log::direction_read);
				log->set_r(channel_player[p]);
			}
		}
	

		// user menu: enter port number
//		port = listen_menu( &videosystem.window, port );
		
		if (port == -1) return;
				
		// try to establish a connection to all the required players
		
		int n;
		int val;
		char *addr = 0;
		int Lstr;

		for ( n = 1; n < Nplayers; ++n )	// exclude the local player from this.
		{
	
			char tmp[512];
			sprintf(tmp, "Listening for client # %i", n);
			message.out(tmp);
			message.animate(0);
			
			if (!log->add_listen(port))
				tw_error("listener failed");

			// you should share data about the new player with all existing players ; include
			// also the player number, so that this new player also gets something useful
			// in addition...

			val = 99;

			message.print(1500, 14, "SHARING(sending) [%i]", val);
			message.animate(0);

			addr = log->get_address(log->num_connections-1);

			share(-1, &val);
			share(-1, &n);
			
			Lstr = strlen(addr) + 1;
			share(-1, &Lstr);
			share_update();
			share(-1, addr, Lstr);
			
			share_update();

			message.print(1500, 14, "SHARED [%i] [%i] [%s]", val, n, addr);
			message.animate(0);

		}

		if (addr)
		{
		int kstart = 0;
		share(-1, &val);
		share(-1, &kstart);
		
		Lstr = strlen(addr) + 1;
		share(-1, &Lstr);
		share_update();
		share(-1, addr, Lstr);	
		share_update();

		message.print(1500, 14, "SHARED [%i] [%i] [%s]", val, kstart, addr);
		message.animate(0);
		}


		// you're the host, so you determine the gametype !
		// for now, default to a normal melee game
		strcpy(gname, temp_gamename);

	} else {
		log->type = Log::log_net1client;

		// channel_init and such, are all read-only for the clients
		// uhm, well, actually this is somewhat dangerous, as they're created on-the-fly
		//log->set_all_directions(Log::direction_read);

		int p;
		
		int i;
		for ( i = 0; i <= channel_playback; ++i )
			log->set_direction(i, Log::direction_read);
		//	log->set_r(i);

		//log->set_r(channel_none);
		for ( p = 0; p < max_player; ++p)
		{
			log->set_r(channel_player[p]);
		}

		char *tmp = address;
		// note, that tmp can be changed by the menu, so that it points to a different string?!

		// user menu: enter adress and port number
//		if (connect_menu(&videosystem.window, &tmp, &port) == -1) 
//			return;

		// saving address
		set_config_string("Network", "Address", tmp);

		message.out("Connecting to server...");
		message.animate(0);

		// connect to the server (whom is player 0 by default, cause nobody else is connected yet.)
		log->add_connect(tmp, port);
//		free(tmp);
		
		// receive some data on channel_init
		
		int Lstr;
		int val = -1;
		message.print(1500, 14, "SHARING(receiving)");
		message.animate(0);

		char addr[512];
		

		p_local = 0;	// uninitialized value ; you know, cause client can't be player 0
		for (;;)
		{
			share(-1, &val);
			share(-1, &p);	// you receive your own (local) player number
			
			share(-1, &Lstr);
			share_update();
			share(-1, addr, Lstr);
			share_update();

			message.print(1500, 14, "SHARED [%i] [%i] [%s]", val, p, addr);
			message.animate(0);


			if (!p_local)
			{
				p_local = p;	// IMPORTANT TO SET THIS !!

				// also, the new client should listen to all existing clients (if any)
				int idone;
				for ( idone = 1; idone < p_local; ++ idone )
				{
					if (!log->add_listen(port))
						tw_error("listener failed");
				}

			} else {
				
				if ( p == 0 )
					break;		// this signals the start of the game

				// otherwise, listen to the other newly connected client
				log->add_connect(addr, port);
			}
			
		}


		if (!p_local)
			tw_error("Failed to initialize player number on startup");

		// the local player is always read/write
		// no need to do this earlier, cause we've only used channel_init so-far.

		log->set_rw(channel_local());



		// this is received from the channel_init
		// and, it's done from within the game::init, that the host is starting.
		// (a bit weird, but it doesn't really matter where you put it)

		//gname = detect_gametype(log);
	}

	// quick hack for testing.
	// note that the server has "local" settings which overwrite other players' setting
	num_humans = Nplayers;
	num_bots = Nbots;

	share(-1, &num_humans);
	share(-1, &num_bots);
	share_update();
	
	set_numplayers(num_humans + num_bots);

	share_string(gname);


	log->optimize4latency();
	//message.out("connection established");
	
	log->reset();

	play_game(gname, log);

	return;
}

void play_net1client ( const char *_address, int _port )
{
	STACKTRACE;

	NetLog *log = new NetLog();
	log->init();
	log->type = Log::log_net1client;

	log->set_all_directions(Log::direction_read);
	int p;
	for ( p = 0; p < max_player; ++p )	// note, 0==server.
	{
		log->set_direction(channel_player[p] , Log::direction_write | Log::direction_read | NetLog::direction_immediate);
		log->set_direction(channel_player[p] + _channel_buffered, Log::direction_write | Log::direction_read);
	}
	
	set_config_file("client.ini");
	char address[128];
	int port, i;
	while (!log->net[0].isConnected()) {
		if (!_address) strncpy(address, get_config_string("Network", "Address", ""), 127);
		else strncpy(address, _address, 127);
		if (_port == -1) port = get_config_int("Network", "Port", 15515);
		else port = _port;
		char *addressaddress = address;
		if (connect_menu(&videosystem.window, &addressaddress, &port) == -1) 
			return;
		set_config_string("Network", "Address", addressaddress);
		message.out("Connecting to server...");
		message.animate(0);
		i = ((NetLog*)log)->net[0].connect(addressaddress, port, is_escape_pressed);
		free(addressaddress);
		if (i) {
//						while (is_escape_pressed());
			while (keypressed()) readkey();
			tw_error("connection failed");
		}
	}

	log->optimize4latency();
	message.out("connection established");
	
	char *gname = detect_gametype(log);
	log->reset();

	play_game(gname, log);

	free(gname);
	return;
}

void play_net1server(const char *_gametype_name, int _port) {STACKTRACE
	NetLog *log = new NetLog();
	log->init();

	log->type = Log::log_net1server;
	log->set_all_directions(Log::direction_write | Log::direction_read | NetLog::direction_immediate);
	int p;
	for ( p = 1; p < max_player; ++p )	// note, 0==server.
	{
		log->set_direction(channel_player[p] , Log::direction_read);
		log->set_direction(channel_player[p] + _channel_buffered, Log::direction_read);
	}
	log->set_direction(channel_server + _channel_buffered, Log::direction_write | Log::direction_read);
	
	set_config_file("client.ini");
	int port;
	while (!log->net[0].isConnected()) {
		if (_port == -1) port = get_config_int("Network", "Port", 15515);
		else port = _port;

		port = listen_menu( &videosystem.window, port );
		if (port == -1) return;

		message.out("Listening for client...");
		message.animate(0);
		log->net[0].listen(port, is_escape_pressed);
		
	}

	log->optimize4latency();
	message.out("connection established");
	
	play_game(_gametype_name, log);

	return;
}

void play_demo ( const char * file_name ) {STACKTRACE
	Log *log = new PlaybackLog();
	log->init();
	log->load(file_name);
	char *gname = detect_gametype(log);
	play_game(gname, log);
	free(gname);
	return;
}


void play_single(const char *_gametype_name, Log *_log)
{
	if (!_log) {
		_log = new Log();
		_log->init();
		set_global(_log);
	}


	p_local = 0;		// IMPORTANT TO SET THIS !!
	
	// channel_init and such, are all write for the one who hosts.
	glog->type = Log::log_normal;
	
	int i;
	for ( i = 0; i <= channel_playback; ++i )
		glog->set_direction(i, Log::direction_read | Log::direction_write | Log::direction_immediate);

	glog->set_rw(channel_playback);	// perhaps this is a better setting for the playback channel
	glog->set_rw(channel_local());
	
	// the remote players are always read-only (local player is always write, of course)
	int p;
	for ( p = 0; p < max_player; ++p)
	{
		if (p != p_local)
			glog->set_r(channel_player[p]);
	}

	// well, you're in single-player mode.
	num_humans = 1;
	num_bots = 0;
	num_players = num_humans + num_bots;
	
	
	play_game(_gametype_name, _log);
}


void play_game(const char *_gametype_name, Log *_log)
{
	STACKTRACE
	bool gui_stuff = false;
	char gametype_name[1024];
	char *c;
	Game *new_game = NULL;

    showLoadingScreen();

	strncpy(gametype_name, _gametype_name, 1000);
	for (c = strchr(gametype_name, '_'); c; c = strchr(c, '_'))
		*c = ' ';


	if (!_log) {
		_log = new Log();
		_log->init();
		set_global(_log);
	}




	if (scp) {
		gui_stuff = true;
		sound.stop_music();
		if (scp) unload_datafile(scp);
		scp = NULL;
	}

	try {
		if (old_game) {
			delete old_game;
			old_game = NULL;
		}

		GameType *type = gametype(gametype_name);
		if (type)
			new_game = type->new_game();
		else
			tw_error("wait a sec... I can't find that game type");

		new_game->preinit();
		new_game->window = new VideoWindow;
		new_game->window->preinit();
		new_game->window->init(&videosystem.window);
		new_game->window->locate(0,0,0,0,0,1,0,1);
		new_game->init(_log);
		new_game->play();
		glog->deinit();
		game = NULL;
		new_game->game_done = true;
		old_game = new_game;
	}

	catch (int i) {
		if (i == -1) throw;
		if (__error_flag & 1) throw;
		if (i != 0) caught_error ("%s %s caught int %d", __FILE__, __LINE__, i);
		if (__error_flag & 1) throw;
	}
	catch (const char *str) {
		if (__error_flag & 1) throw;
		caught_error("message: \"%s\"", str);
		if (__error_flag & 1) throw;
	}
	catch (...) {
		if (__error_flag & 1) throw;
		caught_error("Ack(2)!!!\nAn error occured!\nBut I don't know what error!");
		if (__error_flag & 1) throw;
	}

	if (gui_stuff) {
        prepareTitleScreenAssets();
		showTitle();
	}
	return;
}


char dialog_string[20][128];

int MAX_PLAYERS = 1;
int MAX_CONFIGURATIONS = 1;
int MAX_TEAMS = 1;

// list box getter functions
char *playerListboxGetter(int index, int *list_size) ;
char *controlListboxGetter(int index, int *list_size) ;
char *viewListboxGetter(int index, int *list_size) ;

FONT *TW_font = NULL;

// dialog results
/*int mainRet = 0;
int shipRet = 0;
int keyRet = 0;
int fleetRet = 0;
int optionsRet= 0;*/


char **player_type = NULL;
int *player_config = NULL;
int *player_team = NULL;

Control *load_player(int i) {STACKTRACE
	char tmp[32];
	Control *r = NULL;

	sprintf (tmp, "Config%d", player_config[i]);
	r = getController(player_type[i], tmp, channel_none);
	if (r)
		r->load("scp.ini", tmp);
	return r;
}


class MainMenu : public BaseClass {
	public:
	virtual void _event(Event * e);
	virtual void preinit();
	virtual void deinit();
	virtual void init(VideoWindow *parent);
	virtual void doit();
	virtual void enable();
	virtual void disable();
	int state;
	VideoWindow *window;
} mainmenu;

void MainMenu::_event(Event *e) {
	if (e->type == Event::VIDEO) {
		if (e->subtype == VideoEvent::REDRAW) if (state & 1) showTitle();
	}
}

void MainMenu::enable() {STACKTRACE
	if (!(state & 2)) window->add_callback(this);
	state |= 3;
}

void MainMenu::disable() {STACKTRACE
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
	STACKTRACE
	if (state & 2) {
		window->remove_callback(this);
		window->deinit();
		delete window;
		window = NULL;
	}
}

void MainMenu::doit() {STACKTRACE
	int i;
	char tmp[32];

	set_config_file("scp.ini");
	if (!player_type) {		
		MAX_PLAYERS        = get_config_int("Limits", "MaxPlayers", 12);
		MAX_CONFIGURATIONS = get_config_int("Limits", "MaxConfigurations", 4);
		MAX_TEAMS          = get_config_int("Limits", "MaxTeams", 6);
		player_type = new char*[MAX_PLAYERS];
		player_config = new int[MAX_PLAYERS];
		player_team   = new int[MAX_PLAYERS];
	}
	for (i = 0; i < MAX_PLAYERS; i += 1) {
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
		switch (mainRet) {
			case MAIN_DIALOG_MELEE:
				disable();
				play_single("Melee");
				enable();
				break;
			case MAIN_DIALOG_NET_HOST:
			case MAIN_DIALOG_NET_JOIN:
				disable();
				play_net(mainRet == MAIN_DIALOG_NET_HOST);
				enable();
				break;
			case MAIN_DIALOG_MELEE_EXTENDED:
				disable();
				extended_menu();
				enable();
				break;
			case MAIN_DIALOG_OPTIONS:
				options_menu(NULL);
				break;
			case MAIN_DIALOG_HELP:
				show_file("ingame.txt");
				break;
			case MAIN_DIALOG_TEAMS:
				change_teams();
				showTitle();
				break;
#ifdef INCLUDE_GAMEX
			case MAIN_DIALOG_FG:
				disable();
				play_fg(&scp, SCPGUI_MUSIC);
				enable();
				showTitle();
				break;
#endif
		}
	} while((mainRet != MAIN_DIALOG_EXIT) && (mainRet != -1));

}


/** \brief This routine check if a zip filename is supplied to the program; if so, it
calls pkzipc to extract files to the game root. Under windows this means, you can
drag a update-zip-file onto the program icon, after which the program makes a call to
the pkunzip program to install it automatically in the correct location (namely, the
game-root). pkzipc is part of the pkzip (pkware) package, I think.
*/
static void update_check(int argc, char *argv[])
{
#ifdef _WINDOWS
	// first of all, check if someone drags a zip file on top of your icon --> you
	// may want to unpack it, in the timewarp directory.
	if (argc == 2)
	{
		char *ext;
		ext = strrchr(argv[1], '.');
		
		if (strcmp(ext, ".zip") == 0)
		{
			
			// POSSIBLE:
			// you could make a system call here, to a pack program (winzip, pkzip, winrar)
			// which can unpack this file in the game root.
			
			char command[512];
			char targetdir[512];
			
			strcpy(targetdir, argv[0]);
			char *tmp = strrchr(targetdir, '\\');
			*tmp = 0;
			
			char *zipfile = argv[1];
			sprintf(command, "pkzipc -extract \"%s\" \"%s\"  & pause", zipfile, targetdir);
			/*
			clear_to_color(screen, makecol(0,0,255));
			textout(screen, font, command, 10,10, makecol(255,255,0));
			readkey();
			*/
			system(command);
			
		}
	}
#endif
}
		


int tw_main(int argc, char *argv[]);

int main(int argc, char *argv[]) { STACKTRACE
	int r;
	r = tw_main(argc, argv);
	return r;
}
END_OF_MAIN();

int tw_main(int argc, char *argv[]) { STACKTRACE
#ifdef WIN32
	char szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, sizeof(szPath));
	if (strrchr(szPath, '\\')) *strrchr(szPath, '\\') = '\0';
	SetCurrentDirectory(szPath);
#endif

	update_check(argc, argv);

	int i;
	int auto_port = -1;
	const char *auto_play = NULL, *auto_param = NULL;

	#ifdef __BEOS__
		// set cwd to path of exe 
		// to allow running from icon 
		char datapath[256];
		for (i=strlen(argv[0]) ; argv[0][i]!='/' ; i--);
		strncpy(datapath, argv[0], i);
		chdir(datapath);
	#endif

	#if (defined _MSC_VER) && (defined _DEBUG)
		_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG|_CRTDBG_LEAK_CHECK_DF);
	#endif

	log_debug(NULL);
	time_t start_time = time(NULL);
	log_debug("Log started at %s\n", asctime(localtime(&start_time)));
	if (allegro_init() < 0)
		tw_error_exit("Allegro initialization failed");
	videosystem.preinit();

	try {
		init_time();
		init_error();
		init_profiling();

		set_window_title("Star Control : TimeWarp");
		set_config_file("client.ini");


		int screen_width = 640, screen_height = 480, screen_bpp = 32;
		int fullscreen = 0;

		auto_unload = get_config_int("System", "AutoUnload", 0);


		screen_bpp       = get_config_int("Video", "BitsPerPixel", 16);
		screen_width     = get_config_int("Video", "ScreenWidth", 640);
		screen_height    = get_config_int("Video", "ScreenHeight", 480);
		fullscreen       = get_config_int("Video", "FullScreen", false);

		SpaceSprite::mip_bias = get_config_int ("View", "Mip_bias", 0);
		SpaceSprite::mip_min = get_config_int ("View", "Mip_min", 0);
		SpaceSprite::mip_max = get_config_int ("View", "Mip_max", 0);

		interpolate_frames = get_config_int("View", "InterpolateFrames", 0);
		set_tw_aa_mode(get_config_int("Rendering", "AA_Mode", 0));
		int gamma   = get_config_int("Video", "Gamma", 128);
		set_gamma( gamma );

		// initialize log-channel settings.
		init_channels();

		int inputs = 7;

		// parse command-line arguments
		for (i = 1; i < argc; i += 1) {
			if (false) ;
			else if (!strcmp(argv[i], "-res") && (argc > i + 2)) {
				log_debug("command-line argument -res\n");
				screen_width = atoi(argv[i+1]);
				screen_height = atoi(argv[i+2]);
				i += 2;
			}
			else if (!strcmp(argv[i], "-bpp") && (argc > i + 1)) {
				log_debug("command-line argument -bpp\n");
				screen_bpp = atoi(argv[i+1]);
				i += 1;
			}
			else if (!strcmp(argv[i], "-fullscreen") && (argc > i + 0)) {
				log_debug("command-line argument -fullscreen\n");
				fullscreen = true;
			}
			else if (!strcmp(argv[i], "-window") && (argc > i + 0)) {
				log_debug("command-line argument -window\n");
				fullscreen = false;
			}
			else if (!strcmp(argv[i], "-nosound") && (argc > i + 0)) {
				log_debug("command-line argument -nosound\n");
				sound.disable();
			}
			else if (!strcmp(argv[i], "-nokeyboard") && (argc > i + 0)) {
				log_debug("command-line argument -nokeyboard\n");
				inputs &= ~1;
			}
			else if (!strcmp(argv[i], "-nomouse") && (argc > i + 0)) {
				log_debug("command-line argument -nomouse\n");
				inputs &= ~2;
			}
			else if (!strcmp(argv[i], "-nojoystick") && (argc > i + 0)) {
				log_debug("command-line argument -nojoystick\n");
				inputs &= ~4;
			}
			else if (!strcmp(argv[i], "-noidle") && (argc > i + 0)) {
				log_debug("command-line argument -noidle\n");
				_no_idle = 1;
			}
			else if (!strcmp(argv[i], "-play") && (argc > i + 2)) {
				log_debug("command-line argument -play\n");
				auto_play = argv[i+1];
				auto_param = argv[i+2];
				i += 2;
				if ((argc > i + 0) && (argv[i][0] != '-')) {
					auto_port = atoi(argv[i]);
					i += 1;
				}
			}
			else {
				log_debug("unrecognized command-line argument\n");
			}
		}

		log_debug("command-line arguments parsed\n");

		srand(time(NULL));
		set_color_conversion(COLORCONV_NONE);

		videosystem.set_resolution(screen_width, screen_height, screen_bpp, fullscreen);
		register_bitmap_file_type("jpg", load_jpg, NULL);

		enable_input(inputs);
		sound.init();
		sound.load();
        


        showLoadingScreen();

		View *v = NULL;
		v = get_view ( get_config_string("View", "View", NULL) , NULL );
		if (!v) v = get_view ( "Hero", NULL );
		set_view(v);
        

		twgui_init(get_time, tw_error);


		init_ships();
		init_fleet();
		meleedata.init();//mainmain

		if (auto_play) {// FIX ME
			if (!strcmp(auto_play, "game")) play_single(auto_param, NULL);
			if (!strcmp(auto_play, "demo")) play_demo(auto_param);
			if (!strcmp(auto_play, "net1client")) play_net1client(auto_param, auto_port);
			if (!strcmp(auto_play, "net1server")) play_net1server(auto_param, auto_port);
			if (!strcmp(auto_play, "net")) play_net(atoi(auto_param));
#ifdef INCLUDE_GAMEX
			if (!strcmp(auto_play, "fg")) play_fg(&scp, SCPGUI_MUSIC, auto_param);
#endif
		}
		else {
			mainmenu.preinit();
			mainmenu.init(&videosystem.window);
			mainmenu.doit();
			mainmenu.deinit();
		}
		if (old_game) {
			delete old_game;
			old_game = NULL;
		}

		meleedata.deinit();
		sound.disable();
		disable_input();
		unload_datafile(scppal);
	}

	catch (int i) {
		if (i == -1) throw;
		if (__error_flag & 1) throw;
		if (i != 0) caught_error("%s %s caught int %d", __FILE__, __LINE__, i);
		if (__error_flag & 1) throw;
	}
	catch (const char *str) {
		if (__error_flag & 1) throw;
		caught_error("message: \"%s\"", str);
		if (__error_flag & 1) throw;
	}
	catch (...) {
		if (__error_flag & 1) throw;
		caught_error("Ack!!!\nAn error occured on startup!\nBut I don't know what error!");
		if (__error_flag & 1) throw;
	}

	tw_exit(0);
	return 0;
}


void showTitle(VideoWindow *window) {
	if (!scp) {
		tw_error ("showTitle - gui stuff not loaded");
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




/*
 *** MELEE_EX dialog section - begin
 */


//  - dialog structure
DIALOG select_game_dialog[] = {
  // (dialog proc)     (x)  (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_shadow_box_proc, 160, 120,  320,  240,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_text_proc,       180, 135,  280,  190,  255,  0,    0,    0,       0,    0,    (void *)"Select a game", NULL, NULL},
  { d_list_proc2,      180, 155,  280,  190,  255,  0,    0,    D_EXIT,  0,    0,    (void *)genericListboxGetter, NULL, game_names },//doesn't hold the right value until main() begins
  { d_tw_yield_proc,   0,   0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,   0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};
const char *select_game_menu () {STACKTRACE
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



// MELEE_EX - dialog objects
enum {
	MELEE_EX_DIALOG_BOX = 0,
	MELEE_EX_DIALOG_PLAY_GAME,
	MELEE_EX_DIALOG_PLAY_NET1SERVER,
	MELEE_EX_DIALOG_PLAY_NET1CLIENT,
	MELEE_EX_DIALOG_PLAY_DEMO,
	MELEE_EX_DIALOG_SAVE_DEMO,
	MELEE_EX_DIALOG_CLEAR_STATE,
	MELEE_EX_DIALOG_KEYTESTER,
	MELEE_EX_DIALOG_SHIPINFO,
	MELEE_EX_DIALOG_DIAGNOSTICS,
	MELEE_EX_DIALOG_EXIT
};


// MELEE_EX - dialog structure
DIALOG melee_ex_dialog[] = {
  // (dialog proc)     (x)  (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_shadow_box_proc, 40,  40,   240,  300,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { my_d_button_proc,  50,   50,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Play Game" , NULL, NULL },
  { my_d_button_proc,  50,   75,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Play Game / Net1 Server" , NULL, NULL },
  { my_d_button_proc,  50,  100,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Play Game / Net1 Client" , NULL, NULL },
  { my_d_button_proc,  50,  125,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Playback Demo Recording" , NULL, NULL },
  { my_d_button_proc,  50,  150,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Save Demo Recording" , NULL, NULL },
  { my_d_button_proc,  50,  175,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Clear State", NULL, NULL },
  { my_d_button_proc,  50,  200,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Key Tester", NULL, NULL },
  { my_d_button_proc,  50,  225,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Ship Info", NULL, NULL },
  { my_d_button_proc,  50,  250,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Diagnostics", NULL, NULL },
  { d_button_proc,     80,  280,  190,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Main Menu", NULL, NULL },
  { d_tw_yield_proc,   0,   0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,   0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};


// MELEE_EX - dialog function
void extended_menu(int i) {STACKTRACE
//	melee_ex_dialog[MELEE_EX_DIALOG_GAMELIST].dp3 = game_names;
	if (i == -1)
		i = tw_popup_dialog(NULL, melee_ex_dialog, MELEE_EX_DIALOG_PLAY_GAME);
	switch (i) {
		case -1:
		case MELEE_EX_DIALOG_EXIT: {
			return;
		}
		break;
		case MELEE_EX_DIALOG_PLAY_GAME: {
			const char *gname = select_game_menu();
			if (gname) play_single(gname);
//			play_game(game_names[melee_ex_dialog[MELEE_EX_DIALOG_GAMELIST].d1], log_types[melee_ex_dialog[MELEE_EX_DIALOG_LOGLIST].d1]);
		}
		break;
		case MELEE_EX_DIALOG_PLAY_NET1SERVER: {
			const char *gname = select_game_menu();
			if (gname) play_net1server(gname);
		}
		break;
		case MELEE_EX_DIALOG_PLAY_NET1CLIENT: {
			play_net1client();
		}
		break;
		case MELEE_EX_DIALOG_PLAY_DEMO: {
			play_demo();
		}
		break;
		case MELEE_EX_DIALOG_SAVE_DEMO: {
			if (!old_game || !glog) {
				tw_alert ( "Aaaaarggh! No recording to save!", ":(");
				return;
			}
			glog->save("demo.dmo");
		}
		break;
		case MELEE_EX_DIALOG_CLEAR_STATE: {
			delete old_game;
			old_game = NULL;
			unload_all_ship_data();
		}
		break;
		case MELEE_EX_DIALOG_DIAGNOSTICS: {
			show_diagnostics();
		}
		break;
		case MELEE_EX_DIALOG_SHIPINFO: {
			ship_view_dialog(0, NULL);
		}
		break;
		case MELEE_EX_DIALOG_KEYTESTER: {
			keyjamming_tester();
		}
		break;
	}
	showTitle();
	return;
}


/*
 *** MELEE_EX dialog section - end
 */




/*
 *** TEAMS dialog section - begin
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
void change_teams() {STACKTRACE
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
		else if ((a == TEAMS_DIALOG_SETUP) || (a == TEAMS_DIALOG_PLAYERLIST)) {
			Control *tmpc = load_player(teamsDialog[TEAMS_DIALOG_PLAYERLIST].d1);
			if (tmpc) {
				showTitle();
				tmpc->setup();
				delete tmpc;
				showTitle();
			}
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
	for (i = 0; i < MAX_PLAYERS; i += 1) {
		sprintf(dialog_string[0], "Player%d", i+1);
		set_config_string (dialog_string[0], "Type", player_type[i]);
		set_config_int (dialog_string[0], "Config", player_config[i]);
		set_config_int (dialog_string[0], "Team", player_team[i]);
	}
	twconfig_set_int("/cfg/client.ini/network/NetworkMeleeUseTeams", 
		(teamsDialog[TEAMS_DIALOG_NETTEAMS].flags & D_SELECTED) ? 1 : 0);

	return;
}

/*
 *** TEAMS dialog section - end
 */




/*
 *** FLEET dialog section - begin
 */


// FLEET - dialog objects
enum {
    //FLEET_DIALOG_CLEAR_SCREEN=0,
    FLEET_DIALOG_AVAILABLE_SHIPS_TEXT=0,
    FLEET_DIALOG_SHIP_CATAGORIES_TEXT,
    //TEMP,
    FLEET_DIALOG_SC1_TOGGLE,
    FLEET_DIALOG_SC2_TOGGLE,
    FLEET_DIALOG_SC3_TOGGLE,
    FLEET_DIALOG_TW_OFFICIAL_TOGGLE,
    FLEET_DIALOG_TW_EXP_TOGGLE,
    FLEET_DIALOG_TW_SPECIAL_TOGGLE,
    FLEET_DIALOG_SORTBY_TEXT1,
    FLEET_DIALOG_SORTBY_BUTTON1,
    FLEET_DIALOG_SORTBY_ASCENDING1,
    FLEET_DIALOG_AVAILABLE_SHIPS_LIST,
    FLEET_DIALOG_FLEET_SHIPS_LIST,
    FLEET_DIALOG_PLAYER_FLEET_BUTTON,
    FLEET_DIALOG_PLAYER_FLEET_TITLE,
    FLEET_DIALOG_SAVE_BUTTON,
    FLEET_DIALOG_LOAD_BUTTON,
    FLEET_DIALOG_POINT_LIMIT_TEXT,
    FLEET_DIALOG_POINT_LIMIT_BUTTON,
    FLEET_DIALOG_CURRENT_POINTS_TEXT,
    FLEET_DIALOG_CURRENT_POINTS_VALUE,
    FLEET_DIALOG_SORTBY_TEXT2,
    FLEET_DIALOG_SORTBY_BUTTON2,
    FLEET_DIALOG_SORTBY_ASCENDING2,
	FLEET_DIALOG_ADD_BUTTON,
    FLEET_DIALOG_ADD_ALL_BUTTON,
	FLEET_DIALOG_CLEAR,
    FLEET_DIALOG_CLEARALL,
    FLEET_DIALOG_SHIP_PICTURE_BITMAP,
    //FLEET_DIALOG_SHIP_STATS_BITMAP,
    FLEET_DIALOG_SHIP_SUMMARY_TEXT,
    FLEET_DIALOG_BACK_BUTTON,
    FLEET_DIALOG_HELP_TEXT/**/
};


char *numeric_string[] = {"Zero", "One", "Two", "Three", "Four", 
		"Five", "Six", "Seven", "Eight", "Nine", "Ten", "Eleven", 
		"Twelve"};

char fleetPlayer[18];
char fleetTitleString[100];

int scp_fleet_dialog_text_list_proc(int msg, DIALOG* d, int c);

int scp_fleet_dialog_bitmap_proc(int msg, DIALOG* d, int c);



int d_check_proc_fleeteditor(int msg, DIALOG *d, int c)
{
	if (msg == MSG_CLICK)
	{
		
		/* track the mouse until it is released */
		while (gui_mouse_b()) {
//			state2 = ((gui_mouse_x() >= d->x) && (gui_mouse_y() >= d->y) &&
//				(gui_mouse_x() < d->x + d->w) && (gui_mouse_y() < d->y + d->h));
			
			/* let other objects continue to animate */
			broadcast_dialog_message(MSG_IDLE, 0);
		}
		
		/* should we close the dialog? */
		// imo the following mucho better/ simplere than that messy stuff in the allegro routine
		// ... check d_button_proc in guiproc.c in the allegro sources...

		if (d->flags & D_SELECTED)
			d->flags &= ~D_SELECTED;
		else
			d->flags |= D_SELECTED;

		if ( d->flags & D_EXIT)
			return D_CLOSE;

		return D_O_K; 
	}
	
	return d_check_proc(msg, d, 0);
}


// FLEET - dialog structure
DIALOG fleetDialog[] = {
  // (dialog proc)     (x)  (y)   (w)   (h)   (fg)(bg)(key) (flags)    (d1)   (d2)        (dp)

  //{ d_clear_proc,       0,    0,    0,    0,   255,  0,    0,    0,       0,    0,    NULL },//FLEET_DIALOG_CLEAR_SCREEN
  { d_textbox_proc,     10,  10,   240,  20,   255,  0,    0,     0,       0,    0,    (void *)"Available Ships", NULL, NULL },//FLEET_DIALOG_AVAILABLE_SHIPS_TEXT TODO specify font here in d2 I think
  { d_textbox_proc,     10,  35,   128,  17,   255,  0,    0,     0,       0,    0,    (void *)"Ship Catagories:", NULL, NULL },//FLEET_DIALOG_SHIP_CATAGORIES_TEXT
  
  { d_check_proc_fleeteditor,	// x=30-->x=10
                        10,  52,   128,  14,   255,  0,    0,D_EXIT | D_SELECTED,       0,    0,    (void *)"SC1", NULL, NULL },//FLEET_DIALOG_SC1_TOGGLE
  { d_check_proc_fleeteditor,		
                        10,  66,   128,  14,   255,  0,    0,D_EXIT | D_SELECTED,       0,    0,    (void *)"SC2", NULL, NULL },//FLEET_DIALOG_SC2_TOGGLE
  { d_check_proc_fleeteditor,		
                        10,  79,   128,  14,   255,  0,    0,D_EXIT | D_SELECTED,       0,    0,    (void *)"SC3", NULL, NULL },//FLEET_DIALOG_SC3_TOGGLE
  { d_check_proc_fleeteditor,		
//                      30,  93,   128,  14,   255,  0,    0,D_EXIT | D_SELECTED,       0,    0,    (void *)"TW (Official)", NULL, NULL },//FLEET_DIALOG_TW_OFFICIAL_TOGGLE
                        80,  52,   128,  14,   255,  0,    0,D_EXIT | D_SELECTED,       0,    0,    (void *)"TW (Official)", NULL, NULL },//FLEET_DIALOG_TW_OFFICIAL_TOGGLE
  { d_check_proc_fleeteditor,       
//                      30, 107,   128,  14,   255,  0,    0,D_EXIT,       0,    0,    (void *)"TW (Experimental)", NULL, NULL },//FLEET_DIALOG_TW_EXP_TOGGLE
                        80,  66,   128,  14,   255,  0,    0,D_EXIT,       0,    0,    (void *)"TW (Experimental)", NULL, NULL },//FLEET_DIALOG_TW_EXP_TOGGLE
  { d_check_proc_fleeteditor,       
//                      30, 107,   128,  14,   255,  0,    0,D_EXIT,       0,    0,    (void *)"TW (Special)", NULL, NULL },//FLEET_DIALOG_TW_SPECIAL_TOGGLE
                        80,  79,   128,  14,   255,  0,    0,D_EXIT,       0,    0,    (void *)"TW (Special)", NULL, NULL },//FLEET_DIALOG_TW_SPECIAL_TOGGLE

  { d_textbox_proc,     10, 121,    64,  17,   255,  0,    0,     0,       0,    0,    (void *)"Sort By:", NULL, NULL },//FLEET_DIALOG_SORTBY_TEXT1
  { d_button_proc,      69, 121,   128,  17,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Cost", NULL, NULL },//FLEET_DIALOG_SORTBY_BUTTON1
  { d_button_proc,     197, 121,    16,  17,   255,  0,    0,D_EXIT,       0,    0,    (void *)"^", NULL, NULL },//FLEET_DIALOG_SORTBY_ASCENDING1

  { scp_fleet_dialog_text_list_proc,
                       10,  141,   240, 227,   255,  0,    0,D_EXIT,       0,    0,    (void *)shippointsListboxGetter, NULL, NULL },//FLEET_DIALOG_AVAILABLE_SHIPS_LIST

  //{ d_text_list_proc,
    //                   10,  141,   240, 227,   255,  0,    0,D_EXIT,       0,    0,    (void *)shippointsListboxGetter, NULL, NULL },//FLEET_DIALOG_AVAILABLE_SHIPS_LIST
  { d_list_proc2,      390, 141,   240, 227,   255,  0,    0,D_EXIT,       0,    0,    (void *)fleetpointsListboxGetter, NULL, NULL },//FLEET_DIALOG_FLEET_SHIPS_LIST

  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { my_d_button_proc,  390,  10,   240,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Player 1 Fleet", NULL, NULL },//FLEET_DIALOG_PLAYER_FLEET_BUTTON
  { my_d_button_proc,  390,  40,   128,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Fleet Title", NULL, NULL },//FLEET_DIALOG_PLAYER_FLEET_TITLE
  { my_d_button_proc,  518,  40,    56,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Save", NULL, NULL },//FLEET_DIALOG_SAVE_BUTTON
  { my_d_button_proc,  574,  40,    56,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Load", NULL, NULL },//FLEET_DIALOG_LOAD_BUTTON
  { d_textbox_proc,    390,  60,   128,  20,   255,  0,    0,     0,       0,    0,    (void *)"Point Limit", NULL, NULL },//FLEET_DIALOG_POINT_LIMIT_TEXT
  { my_d_button_proc,  518,  60,   112,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"300\0              ", NULL, NULL },//FLEET_DIALOG_POINT_LIMIT_BUTTON
  { d_textbox_proc,    390,  80,   128,  20,   255,  0,    0,     0,       0,    0,    (void *)"Current Points", NULL, NULL },//FLEET_DIALOG_CURRENT_POINTS_TEXT
  { d_textbox_proc,    518,  80,   112,  20,   255,  0,    0,     0,       0,    0,    (void *)"100\0              ", NULL, NULL },//FLEET_DIALOG_CURRENT_POINTS_VALUE
  { d_textbox_proc,    390, 120,    64,  20,   255,  0,    0,     0,       0,    0,    (void *)"Sort By:", NULL, NULL },//FLEET_DIALOG_SORTBY_TEXT2
  { d_button_proc,     454, 120,   128,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Cost\0             ", NULL, NULL },//FLEET_DIALOG_SORTBY_BUTTON2
  { d_button_proc,     582, 120,    16,  20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"^", NULL, NULL },//FLEET_DIALOG_SORTBY_ASCENDING2

  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)  
  { my_d_button_proc,  270, 210,  100,   25,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Add", NULL, NULL },//FLEET_DIALOG_ADD_BUTTON
  { my_d_button_proc,  270, 235,  100,   25,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Add All", NULL, NULL },//FLEET_DIALOG_ADD_ALL_BUTTON
  { my_d_button_proc,  270, 265,  100,   25,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Remove", NULL, NULL },//FLEET_DIALOG_CLEAR
  { my_d_button_proc,  270, 290,  100,   25,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Remove All", NULL, NULL },//FLEET_DIALOG_CLEARALL
  { scp_fleet_dialog_bitmap_proc,
                        10, 372,   85,   85,   255,  0,    0,    0,       0,    0,    (void *)NULL, NULL, NULL },//FLEET_DIALOG_SHIP_PICTURE_BITMAP

  //{ d_textbox_proc,     10, 372,  310,   85,   255,  0,    0,     0,       0,    0,    (void *)"SHIP PICTURE TODO", NULL, NULL },//FLEET_DIALOG_SHIP_STATS_BITMAP


  { d_textbox_proc,    325, 372,  305,   85,   255,  0,    0,     0,       0,    0,    (void *)"Summary Text\0                                                                  ", NULL, NULL },//FLEET_DIALOG_SHIP_SUMMARY_TEXT
  { d_button_proc,      10, 460,   64,   20,   255,  0,    0,D_EXIT,       0,    0,    (void *)"Back", NULL, NULL },//FLEET_DIALOG_BACK_BUTTON
  { d_textbox_proc,     74, 460,  556,   20,   255,  0,    0,     0,       0,    0,    (void *)"Help Text\0                                                                     ", NULL, NULL },//FLEET_DIALOG_HELP_TEXT

  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,     0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,     0,       0,    0,    NULL, NULL, NULL }/**/
};

bool safeToDrawPreview = false;

// FLEET - dialog function
void edit_fleet(int player) {STACKTRACE
	char tmp[40];
	char path[80];
    char fleetCostString[80] = "";
    char maxFleetCostString[80] = "";
    bool availableFleetDirty = true;

    static Fleet::SortingMethod sortMethod1 = (Fleet::SortingMethod) Fleet::SORTING_METHOD_DEFAULT,
        sortMethod2 = (Fleet::SortingMethod) Fleet::SORTING_METHOD_DEFAULT;
    static bool sortAscending1 = false,
        sortAscending2 = false;


	sprintf (tmp, "Player%d", player+1);
	Fleet* fleet = new Fleet();
	fleet->load("fleets.ini", tmp);

	if (player + 1 <= 12)
		sprintf(fleetPlayer, "Player %s Fleet", numeric_string[player+1]);
	else sprintf(fleetPlayer, "Player%d Fleet", player+1);
	showTitle();

    int fleetRet;
    int selectedSlot;

    fleetDialog[FLEET_DIALOG_CURRENT_POINTS_VALUE].dp = fleetCostString;
    fleetDialog[FLEET_DIALOG_POINT_LIMIT_BUTTON].dp = maxFleetCostString;
    
//	// the reference_fleet is used in the list in a hardcoded way, so over"load" it
//    Fleet *old_reference_fleet = reference_fleet;

	do {
		sprintf(title_str, fleet->getTitle());
		sprintf(fleetTitleString, "%s\n%d points", fleet->getTitle(), fleet->getCost());
        
        fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].dp3 = fleet;
        fleetDialog[FLEET_DIALOG_SORTBY_BUTTON1].dp = Fleet::getSortingMethodName(sortMethod1);
        fleetDialog[FLEET_DIALOG_SORTBY_BUTTON2].dp = Fleet::getSortingMethodName(sortMethod2);

        sprintf(fleetCostString,"%d", fleet->getCost());
        if (fleet->getCost() > fleet->getMaxCost())
            fleetDialog[FLEET_DIALOG_CURRENT_POINTS_VALUE].bg = makecol8(255,0,0);
        else
            fleetDialog[FLEET_DIALOG_CURRENT_POINTS_VALUE].bg = 0;

        sprintf(maxFleetCostString,"%d %s", fleet->getMaxCost(), 
                                            Fleet::getFleetCostName(fleet->getMaxCost()));

        if (sortAscending1)
            fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING1].dp = (void *)"^";
        else
            fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING1].dp = (void *)"v";
        
        if (sortAscending2)
            fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING2].dp = (void *)"^";
        else
            fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING2].dp = (void *)"v";

        //if the user has selected a different choice of available ships, regenerate the
        //list of available ships
        if (availableFleetDirty) {
            availableFleetDirty = false;
            
            //clear out the fleet
            reference_fleet->reset();

            for (int c=0; c<num_shiptypes; c++) {
                switch (shiptypes[c].origin) {
                case SHIP_ORIGIN_SC1:
                    if (fleetDialog[FLEET_DIALOG_SC1_TOGGLE].flags & D_SELECTED)
                        reference_fleet->addShipType(&shiptypes[c]);
                    break;
                
                case SHIP_ORIGIN_SC2:
                    if (fleetDialog[FLEET_DIALOG_SC2_TOGGLE].flags & D_SELECTED)
                        reference_fleet->addShipType(&shiptypes[c]);
                    break;
                
                case SHIP_ORIGIN_SC3:
                    if (fleetDialog[FLEET_DIALOG_SC3_TOGGLE].flags & D_SELECTED)
                        reference_fleet->addShipType(&shiptypes[c]);
                    break;
                
                case SHIP_ORIGIN_TW_ALPHA:
                    if (fleetDialog[FLEET_DIALOG_TW_OFFICIAL_TOGGLE].flags & D_SELECTED)
                        reference_fleet->addShipType(&shiptypes[c]);
                    break;
                
                case SHIP_ORIGIN_TW_BETA:
                    if (fleetDialog[FLEET_DIALOG_TW_EXP_TOGGLE].flags & D_SELECTED)
                        reference_fleet->addShipType(&shiptypes[c]);
                    break;

                case SHIP_ORIGIN_TW_SPECIAL:
                    if (fleetDialog[FLEET_DIALOG_TW_SPECIAL_TOGGLE].flags & D_SELECTED)
                        reference_fleet->addShipType(&shiptypes[c]);
                    break;
                }
            }
            reference_fleet->Sort( sortMethod1, sortAscending1 );
            fleetDialog[FLEET_DIALOG_AVAILABLE_SHIPS_LIST].flags |= D_DIRTY;
        }/**/

		fleetRet = tw_do_dialog(NULL, fleetDialog, -1);

        switch( fleetRet ) {
           case FLEET_DIALOG_AVAILABLE_SHIPS_TEXT: break;
           case FLEET_DIALOG_SHIP_CATAGORIES_TEXT: break;

           case FLEET_DIALOG_SC1_TOGGLE:
           case FLEET_DIALOG_SC2_TOGGLE:
           case FLEET_DIALOG_SC3_TOGGLE:
           case FLEET_DIALOG_TW_OFFICIAL_TOGGLE:
           case FLEET_DIALOG_TW_EXP_TOGGLE:
           case FLEET_DIALOG_TW_SPECIAL_TOGGLE:
               availableFleetDirty = true;
			   break;

           case FLEET_DIALOG_SORTBY_TEXT1: break;
           case FLEET_DIALOG_SORTBY_BUTTON1: 
               sortMethod1 = Fleet::cycleSortingMethod(sortMethod1);
               reference_fleet->Sort( sortMethod1, sortAscending1 );
               fleetDialog[FLEET_DIALOG_SORTBY_BUTTON1].dp = Fleet::getSortingMethodName(sortMethod1);
               break;

           case FLEET_DIALOG_SORTBY_ASCENDING1: 
               sortAscending1 = 1 - sortAscending1;
               reference_fleet->Sort( sortMethod1, sortAscending1 );
               if (sortAscending1)
                   fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING1].dp = (void *)"^";
               else
                   fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING1].dp = (void *)"v";
               break;

           
           

           case FLEET_DIALOG_AVAILABLE_SHIPS_LIST:
           case FLEET_DIALOG_ADD_BUTTON: 
			   int k;
			   k = fleetDialog[FLEET_DIALOG_AVAILABLE_SHIPS_LIST].d1;
			   if (k < 0 || k >= reference_fleet->getSize()) {tw_error("invalid ship choice - bug");}

               selectedSlot = fleet->addShipType(reference_fleet->getShipType(k));
               if (selectedSlot != -1)
                   fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].d1 = selectedSlot;
               
               break;

           case FLEET_DIALOG_PLAYER_FLEET_BUTTON: break;

           case FLEET_DIALOG_PLAYER_FLEET_TITLE:               
               if(do_dialog(fleet_titleDialog, FLEET_TITLE_DIALOG_BOX) == FLEET_TITLE_DIALOG_OK)
                   sprintf(fleet->getTitle(), title_str);
               showTitle();
               break;

           case FLEET_DIALOG_SAVE_BUTTON: 
               sprintf(path, "fleets/");
               if(file_select("Save Fleet", path, "scf")) fleet->save(path, "Fleet");
               showTitle();
               break;

           case FLEET_DIALOG_LOAD_BUTTON: 
               sprintf(path, "fleets/");
               if(file_select("Load Fleet", path, "scf")) fleet->load(path, "Fleet");
               sprintf(title_str, fleet->getTitle());
               sprintf(fleetTitleString, "%s\n%d points", fleet->getTitle(), fleet->getCost());
               showTitle();
               break;

           case FLEET_DIALOG_POINT_LIMIT_TEXT: break;

           case FLEET_DIALOG_POINT_LIMIT_BUTTON:
               fleet->cycleMaxFleetCost();
               break;

           case FLEET_DIALOG_CURRENT_POINTS_TEXT: break;
           case FLEET_DIALOG_CURRENT_POINTS_VALUE: break;
           case FLEET_DIALOG_SORTBY_TEXT2: break;

           case FLEET_DIALOG_SORTBY_BUTTON2: 
               sortMethod2 = Fleet::cycleSortingMethod(sortMethod2);
               fleet->Sort( sortMethod2, sortAscending2 );
               fleetDialog[FLEET_DIALOG_SORTBY_BUTTON2].dp = Fleet::getSortingMethodName(sortMethod2);
               break;

           case FLEET_DIALOG_SORTBY_ASCENDING2: 
               sortAscending2 = 1 - sortAscending2;
               fleet->Sort( sortMethod2, sortAscending2 );
               if (sortAscending2)
                   fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING2].dp = (void *)"^";
               else
                   fleetDialog[FLEET_DIALOG_SORTBY_ASCENDING2].dp = (void *)"v";
               break;

           case FLEET_DIALOG_ADD_ALL_BUTTON: 
               fleet->addFleet(reference_fleet);
               break;

	       case FLEET_DIALOG_CLEAR: 
           case FLEET_DIALOG_FLEET_SHIPS_LIST: 
			fleet->clear_slot(fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].d1);
            if (fleet->getSize() <= 0)
                fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].d1 = 0;
               break;

           case FLEET_DIALOG_CLEARALL: 
               fleet->reset();
               fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].d1 = 0;
               break;

           case FLEET_DIALOG_SHIP_PICTURE_BITMAP: break;
           
           case FLEET_DIALOG_SHIP_SUMMARY_TEXT: break;
           case FLEET_DIALOG_BACK_BUTTON: break;
           case FLEET_DIALOG_HELP_TEXT:/**/
           default:
               ;
        }
		/*if (fleetRet == FLEET_DIALOG_INFO) {
			ship_view_dialog(fleetDialog[FLEET_DIALOG_FLEET_SHIPS_LIST].d1, reference_fleet);
			showTitle();
		}*/

	} while((fleetRet != FLEET_DIALOG_BACK_BUTTON) && (fleetRet != -1));

//	reference_fleet = old_reference_fleet;

	fleet->save("fleets.ini", tmp);
	delete fleet;
	showTitle();
}


int scp_fleet_dialog_text_list_proc(int msg, DIALOG* d, int c) {

	static int next_anim_time = get_time();
	int old_d1 = d->d1;
    int ret = 0;
   

    // allow user to select the ships based on keystrokes:
    // select based on the ship's name
    bool shouldConsumeChar = false;
    if (msg == MSG_CHAR) {
        char typed = (char)(0xff & c);
        if (isalnum (typed)) {
            d->d1 = reference_fleet->getNextFleetEntryByCharacter( d->d1, typed);
            shouldConsumeChar = true;
            if (d->d1 != old_d1) {

                int size = reference_fleet->getSize();
                int height = (d->h-4) / text_height(font);

                ret = D_USED_CHAR; 
                d->flags |= D_DIRTY;

                //scroll such that the selection is shown.
                //only change the scroll if the selection is not already shown,
                //and the number of ships in the list is greater than the number
                //of slots that can be shown simultaneously.
                if ( (size > height) &&
                     ( (d->d1 < d->d2) ||
                       (d->d1 >= d->d2 + height))) 
                {
                    if (d->d1 <= (height/2))
                        d->d2 = 0;
                    else {
                        
                        if (d->d1 >= (size - height))
                            d->d2 = (size - height);
                        else {
                            d->d2 = d->d1 - height/2;
                        }
                    }
                }
            }
        }
    }
    ret = d_text_list_proc( msg, d, c );
    
    if (shouldConsumeChar)
        ret = D_USED_CHAR;

    static BITMAP* panel = create_bitmap(fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].w,
                                         fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].h);
    fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].dp = panel;

    static BITMAP * sprite = create_bitmap(fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].w,
                                           fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].h);
    static int rotationFrame = 0;

    //selection has changed
    if (d->d1 != old_d1) {
        safeToDrawPreview = false;
        float fractionRotated = 0;

        {ShipType* type = reference_fleet->getShipType(old_d1);
        
        if (type && type->data) {
            if (type->data->spriteShip) {
                fractionRotated = (float)((float)rotationFrame / (float)(type->data->spriteShip->frames()));
            }
            type->data->unlock();
        }}

        rotationFrame = 0;

        {ShipType* type = reference_fleet->getShipType(d->d1);
        if (type && type->data) {
            type->data->lock();
            if (type->data->spriteShip)
               rotationFrame = (int)(fractionRotated * type->data->spriteShip->frames());
        }}
    }

	if ( ( d->d1 != old_d1 || msg == MSG_START) || 
         (msg == MSG_IDLE && next_anim_time < get_time()) ) {
        safeToDrawPreview = false;

        //next_anim_time = get_time() + 50 + rand() % 200;
        next_anim_time = get_time() + 20;
        
		ShipType* type = reference_fleet->getShipType(d->d1);
        
        clear_to_color(sprite, 0);

        if (type && type->data && type->data->spriteShip) {
            
            type->data->spriteShip->draw(
                Vector2(fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].w/2,
                        fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].h/2) - type->data->spriteShip->size()/2, 
                type->data->spriteShip->size(), 
                rotationFrame, sprite 
                );

            rotationFrame++;
            if (rotationFrame >= type->data->spriteShip->frames())
                rotationFrame = 0;
        }
        stretch_blit(sprite, panel, 0, 0, sprite->w, sprite->h, 0, 0, panel->w, panel->h);
        safeToDrawPreview = true;
        
        


		//if(data) {


		//static DATAFILE* data = NULL;
		//if (type && type->data) 
		//	data = load_datafile_object(type->data->file, "SHIP_P00_PCX");

/*
			BITMAP* bmp = (BITMAP*)data->dat;

			BITMAP* tmp = create_bitmap(bmp->w, bmp->h);
            //BITMAP* tmp = create_bitmap(fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].w,
              //                          fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].h);

			//blit(bmp, tmp, 0, 0, 0, 0, bmp->w, bmp->h);

            stretch_blit(bmp, tmp, 
                         0, 0, bmp->w, bmp->h,
                         0, 0, fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].w,
			 fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].h);*//*
			//unload_datafile_object(data);

			char obj[32];
			sprintf(obj, "SHIP_P0%d_PCX", 1 + rand() % 4);
			data = load_datafile_object(type->data->file, obj);
			bmp = (BITMAP*)data->dat;
			blit(bmp, tmp, 0, 0, 4, 65, bmp->w, bmp->h);
            //blit(bmp, tmp, 0, 0, 4, 65, bmp->w, bmp->h);

			unload_datafile_object(data);*/
//			gamma_correct_bitmap( tmp, gamma_correction, 0 );

			//panel = create_bitmap(128, 200);
            

            
			//destroy_bitmap(sprite);

			//stretch_blit(tmp, panel, 0, 0, tmp->w, tmp->h, 0, 0, panel->w, panel->h);
			//destroy_bitmap(tmp);
		//}

		//if(fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].dp)
		//	destroy_bitmap( (BITMAP*)fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].dp );

		//fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP].dp = panel;


        //TODO decide if these next 3 lines should be here
		scare_mouse();
        SEND_MESSAGE(&fleetDialog[FLEET_DIALOG_SHIP_PICTURE_BITMAP], MSG_DRAW, 0);
		unscare_mouse();
	}

	return ret;
}

int scp_fleet_dialog_bitmap_proc(int msg, DIALOG* d, int c) {
	//TODO address this: bitmap has to be deleted, but MSG_END does not mean the dialog isn't coming back
    /*if (msg == MSG_END && d->dp) {
		destroy_bitmap( (BITMAP*)d->dp );
		d->dp = NULL;
	}*/

	if ((msg != MSG_DRAW || d->dp) && (safeToDrawPreview) )
		return d_bitmap_proc(msg, d, c);
	return D_O_K;
}

/*
 *** FLEET dialog section - end
 */




/*
 *** SHIPVIEW dialog section - begin
 */


// SHIPVIEW - dialog objects
#define SHIPVIEW_DIALOG_DONE               1
#define SHIPVIEW_DIALOG_FONT               2
#define SHIPVIEW_DIALOG_SORT               3
#define SHIPVIEW_DIALOG_LIST               4
#define SHIPVIEW_DIALOG_DESCRIPTION        5
#define SHIPVIEW_DIALOG_TXTFILE            6
#define SHIPVIEW_DIALOG_PICTURES           7
#define SHIPVIEW_DIALOG_TWYIELD            8


// list of sort algorithm names
char *sorttypes[] = { "Name", "Cost", "Origin", "Coders", NULL };

#define SHIPVIEW_DIALOG_SORT_BYNAME   0
#define SHIPVIEW_DIALOG_SORT_BYCOST   1
#define SHIPVIEW_DIALOG_SORT_BYORIGIN 2
#define SHIPVIEW_DIALOG_SORT_BYCODERS 3


// SHIPVIEW - dialog structure
DIALOG shipviewDialog[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_textbox_proc,    5,    5,    220,  40,   255,  0,    0,    0,       0,    0,    (void *)"Select a ship to examine", NULL, NULL },
  { my_d_button_proc,  300,  5,    120,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Done", NULL, NULL },
  { my_d_button_proc,  300,  45,   120,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Font Size", NULL, NULL },
  { d_list_proc2,      430,  5,    120,  60,   255,  0,    0,    D_EXIT,  0,    0,    (void *) genericListboxGetter, NULL, sorttypes },
  { d_list_proc2,      5,    50,   220,  420,  255,  0,    0,    D_EXIT,  0,    0,    (void *) fleetListboxGetter, NULL, NULL },
  { d_textbox_proc,    230,  110,  400,  160,  255,  0,    0,    0,       0,    0,    (void *) NULL, NULL, NULL },
  { d_textbox_proc,    230,  280,  400,  190,  255,  0,    0,    0,       0,    0,    (void *) NULL, NULL, NULL },
  { d_tw_bitmap_proc,  230,  5,    64,   100,  255,  0,    0,    0,       0,    0,    (void *) NULL, NULL, NULL },
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};

// SHIPVIEW - dialog function
void ship_view_dialog(int si, Fleet *fleet) {
	int i;
	int r = 0;               // result of tw_do_dialog
	const char *sname;
//	char blah[512];
	char txtfile[50000];     // ship text file contents
	char inifile[50000];     // ship ini file contents
	char description[55000]; // ship description contents
	BITMAP *sprite = NULL;

	clear_bitmap(videosystem.surface);

	/*Fleet f, *old;
	old = fleet;
	fleet = f;*/
	//reference_fleet->sort_by("Origin");

	if (!fleet)
		fleet = reference_fleet;

	shipviewDialog[SHIPVIEW_DIALOG_LIST].d1 = si;
	shipviewDialog[SHIPVIEW_DIALOG_LIST].dp3 = fleet;
	shipviewDialog[SHIPVIEW_DIALOG_TXTFILE].dp = txtfile;
	shipviewDialog[SHIPVIEW_DIALOG_DESCRIPTION].dp = description;

	// main dialog loop - begin
	while ((r >= 0) && (r != SHIPVIEW_DIALOG_DONE)) {

		// update ship selection - begin
		if ((r == 0) || (r == SHIPVIEW_DIALOG_LIST)) {
			si = shipviewDialog[SHIPVIEW_DIALOG_LIST].d1;
			sname = shipListboxGetter(si, NULL);
			if (!sname) {
				sprintf(dialog_string[0], "(Null)");
				sprintf(dialog_string[1], "(Null)");
			}
			else {
                
				ShipType *type = reference_fleet->getShipType(si);
				PACKFILE *f;
				//DATAFILE *d;

                if (sprite)
					destroy_bitmap(sprite);
				sprite = NULL;
				type->data->lock();
				if (type->data->spriteShip) {
					sprite = create_bitmap(180, 180);
					clear_to_color(sprite, 0);
					type->data->spriteShip->draw( 
						Vector2(90,90) - type->data->spriteShip->size()/2, 
						type->data->spriteShip->size(), 
						0, sprite 
					);
				}
				type->data->unlock();
                

//				d = load_datafile_object(type->data->file, "SHIP_P00_PCX");

				/*
				if (!d) d = load_datafile_object(blah, "SHIP_S00_BMP");
				if (!d) d = load_datafile_object(blah, "SHIP_S01_PCX");
				if (!d) d = load_datafile_object(blah, "SHIP_S01_BMP");
				*/
				//if (!d) error (" viewships failed to load ship picture");
/*
				switch (d->type) {
					case DAT_RLE_SPRITE: {
						RLE_SPRITE *rle = (RLE_SPRITE*)(d->dat);
						sprite = create_bitmap_ex(rle->color_depth, 180, 180);
						clear_to_color(sprite, 0);
						draw_rle_sprite(sprite, rle, 90-rle->w/2, 90-rle->h/2);
						if (rle->color_depth != bitmap_color_depth(screen)) {
							BITMAP *tmp = create_bitmap(180, 180);
							convert_bitmap(sprite, tmp, 0);
							destroy_bitmap(sprite);
							sprite = tmp;
						}
					}
					break;
					case DAT_BITMAP: {
						sprite = create_bitmap_ex(bitmap_color_depth(screen), 64, 100);
						blit((BITMAP*)(d->dat), sprite, 0, 0, 0, 0, 64, 100);
					}
					break;
				}
				if (d)
					unload_datafile_object(d);
*/
				// read ship text file contents
				f = pack_fopen(type->text, F_READ);
				if (!f)
					sprintf(txtfile, "Failed to load file \"%s\"", type->text);
				else {
					i = pack_fread(txtfile, 49999, f);
					pack_fclose(f);
					txtfile[i] = 0;
				}

				// read ship ini file contents
				f = pack_fopen(type->file, F_READ);
				if (!f)
					sprintf(inifile, "Failed to load file \"%s\"", type->file);
				else {
					i = pack_fread(inifile, 49999, f);
					pack_fclose(f);
					inifile[i] = 0;
				}

				// write ship description contents
				char *c = description;
				c += sprintf(c, "Name: %s\n", type->name);
				c += sprintf(c, "ID: %s\n", type->id);
				c += sprintf(c, "Cost: %d\n", type->cost);
				c += sprintf(c, "\n\n\n\nINI file: (%s)\n", type->file);
				c += sprintf(c, "-------------------------\n%s", inifile);
			}

			shipviewDialog[SHIPVIEW_DIALOG_PICTURES+0].dp = sprite;
		}
		// update ship selection - end

		// change the ship list sorting order - begin
		/*if(r == SHIPVIEW_DIALOG_SORT) {
			i = shipviewDialog[SHIPVIEW_DIALOG_SORT].d1;

			switch (i) {
			case SHIPVIEW_DIALOG_SORT_BYNAME:
                reference_fleet->Sort(Fleet::SORTING_METHOD_NAME1_DESCENDING);
				//reference_fleet->sort_alphabetical("Name2");
				//reference_fleet->sort_alphabetical("Name1");
				break;
			case SHIPVIEW_DIALOG_SORT_BYCOST:
                reference_fleet->Sort(Fleet::SORTING_METHOD_COST_DESCENDING);
				//reference_fleet->sort_numerical("TWCost");
				break;
			case SHIPVIEW_DIALOG_SORT_BYORIGIN:
                reference_fleet->Sort();
				//reference_fleet->sort_alphabetical("Origin");
				break;
			case SHIPVIEW_DIALOG_SORT_BYCODERS:
                reference_fleet->Sort();
				//reference_fleet->sort_alphabetical("Coders");
				break;
			}

			SEND_MESSAGE(&shipviewDialog[SHIPVIEW_DIALOG_LIST], MSG_DRAW, 0);
    }*/
		// change the ship list sorting order - end

		// change font size - begin
		if (r == SHIPVIEW_DIALOG_FONT) {
			i = shipviewDialog[SHIPVIEW_DIALOG_TWYIELD+1].d1;
			i = (((i/2) + 2) % 3) - 1;
			shipviewDialog[SHIPVIEW_DIALOG_TWYIELD+1].d1 = i*2;
		}
		// change font size - end

		r = tw_do_dialog(NULL, shipviewDialog, SHIPVIEW_DIALOG_LIST);
	}
	// main dialog loop - end

	if (sprite)
		destroy_bitmap(sprite);
	return;
}

/*
 *** SHIPVIEW dialog section - end
 */




/*
 * DIAGNOSTICS dialog section - begin
 */


// DIAGNOSTICS - dialog objects
#define DIAGNOSTICS_DIALOG_EXIT           1
#define DIAGNOSTICS_DIALOG_MAIN           3
#define DIAGNOSTICS_DIALOG_FILES          5
#define DIAGNOSTICS_DIALOG_VERSION_TXT    7
//#define DIAGNOSTICS_DIALOG_SHIPS          9


// DIAGNOSTICS - dialog structure
static DIALOG diagnostics_dialog[] =
{
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)  (d2)  (dp)
  { d_box_proc,        0,    0,    640,  480,  255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
  { d_button_proc,     10,   10,   300,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Exit Diagnostics Screen" , NULL, NULL },
  { d_text_proc,       15,   55,   620,  15,   255,  0,    0,    0,          0,    0,    (void *)"Compile-time Options", NULL, NULL },
  { d_textbox_proc,    10,   70,   620,  90,   255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
  { d_text_proc,       15,   165,  620,  15,   255,  0,    0,    0,          0,    0,    (void *)"Compile Times", NULL, NULL },
  { d_textbox_proc,    10,   180,  620,  90,   255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
  { d_text_proc,       15,   275,  620,  15,   255,  0,    0,    0,          0,    0,    (void *)"version.txt", NULL, NULL },
  { d_textbox_proc,    10,   290,  620,  90,   255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
//  { d_text_proc,       15,   275,  620,  15,   255,  0,    0,    0,          0,    0,    (void *)"Ship Abnormalities", NULL, NULL },
//  { d_textbox_proc,    10,   290,  620,  90,   255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,          0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,          0,    0,    NULL, NULL, NULL }
};



int get_diagnostics_string ( char *dest ) {//returns length of string
	char * tmp = dest;

#	if defined _DEBUG
		tmp += sprintf(tmp, "DEBUGGING BUILD!\n");
#	endif
	tmp += sprintf(tmp, "ALLEGRO (.h) version   = Allegro %s, %s\n", 
		ALLEGRO_VERSION_STR, ALLEGRO_PLATFORM_STR);
	tmp += sprintf(tmp, "ALLEGRO (.dll) version = %s\n", allegro_id);
	tmp += sprintf(tmp, "Network type = %s\n", NetTCP::network_type());
	tmp += sprintf(tmp, "Compiler = ");
#	if defined __MINGW32__
		tmp += sprintf(tmp, "MINGW (gcc)\n");
#	elif defined __BORLANDC__
		tmp += sprintf(tmp, "Borland\n");
#	elif defined _MSC_VER
		tmp += sprintf(tmp, "Microsoft Visual C++\n");
#	elif defined DJGPP
		tmp += sprintf(tmp, "DJGPP (gcc)\n");
#	elif defined __GNUC__
		tmp += sprintf(tmp, "gcc\n");
#	else
		tmp += sprintf(tmp, "???\n");
#	endif
	tmp += sprintf(tmp, "Blah = %s\n", tw_version());
	if (sound.is_music_supported()) {
		tmp += sprintf(tmp, "JGMOD (music) Enabled\n");
	} else {
		tmp += sprintf(tmp, "JGMOD (music) Disabled\n");
	}
#	if defined DO_STACKTRACE
		tmp += sprintf(tmp, "DO_STACKTRACE enabled\n");
#	else
		tmp += sprintf(tmp, "DO_STACKTRACE disabled\n");
#	endif
	return tmp - dest;
}

// DIAGNOSTICS - dialog function
void show_diagnostics() {STACKTRACE
	int i;
	char buffy [16000];//fix sometime
	char buffy2[100000];//fix sometime
	char buffy3[16000];//yeah right
	char *tmp;
	PACKFILE *f;

	f = pack_fopen ("./version.txt", F_READ);
	if (!f)
		strcpy(buffy, "Failed to load version.txt");
	else {
		i = pack_fread (buffy, 99999, f);
		pack_fclose(f);
		buffy[i] = 0;
	}

	diagnostics_dialog[DIAGNOSTICS_DIALOG_VERSION_TXT].dp = (void *) buffy;
	diagnostics_dialog[DIAGNOSTICS_DIALOG_FILES].dp = (void *) buffy2;
	tmp = buffy2;


	for (i = 0; i < num_registered_files; i += 1) {
		tmp += sprintf(tmp, "%s %s %s\n", registered_files[i].ftime, registered_files[i].fdate, registered_files[i].fname);
	}
	diagnostics_dialog[DIAGNOSTICS_DIALOG_MAIN].dp = (void *) buffy3;
	tmp = buffy3;
	tmp += get_diagnostics_string( tmp );

/*	diagnostics_dialog[DIAGNOSTICS_DIALOG_SHIPS].dp = (void *) buffy4;
	tmp = buffy4;
	sprintf(tmp, "Ships datafiles missing: ");
	int j = 0;
	for (i = 0; i < num_shiptypes; i += 1) {
		if (
	}*/

	tw_popup_dialog(NULL, diagnostics_dialog, 1);
	return;
}


/*
 * DIAGNOSTICS dialog section - end
 */


void keyjamming_tester() {STACKTRACE
	int i, j = 0;
	char blah[256];

	scare_mouse();
	videosystem.window.lock();
	clear_to_color(videosystem.window.surface, 0);
	textprintf(screen, font, 40, 20, palette_color[15], "Press the keys combinations you wish to test");
	textprintf(screen, font, 40, 40, palette_color[15], "When you're finished, press ESCAPE or F10");
	videosystem.window.unlock();
	unscare_mouse();

	while (!key[KEY_F10] && !key[KEY_ESC]) {
		if (videosystem.poll_redraw()) {
			scare_mouse();
			videosystem.window.lock();
			clear_to_color(videosystem.window.surface, 0);
			textprintf(screen, font, 40, 20, palette_color[15], "Press the keys combinations you wish to test");
			textprintf(screen, font, 40, 40, palette_color[15], "When you're finished, press ESCAPE or F10");
			videosystem.window.unlock();
			unscare_mouse();
		}
		rectfill(screen, 50, 60, 500, 60 + 20 * j, palette_color[0]);
		j = 0;
		poll_input();
		for (i = 0; (i < 32767) && (j < 16); i += 1) {
			if (key_pressed(i)) {
				key_to_description(i, blah);
				scare_mouse();
				acquire_screen();
				textprintf(screen, font, 50, 60+j*20, palette_color[15], "%s", blah);
				release_screen();
				unscare_mouse();
				j += 1;
			}
		}
		idle(20);
	}
	showTitle();
	while (key[KEY_F10])
		poll_keyboard();
	clear_keybuf();
	return;
}


char *playerListboxGetter(int index, int *list_size) {
	static char buf[160];
	char *tmp = buf;

	tmp[0] = 0;
	if(index < 0) {
		*list_size = MAX_PLAYERS;
		return NULL;
	} else {
		tmp += sprintf(tmp, "Player%d", index + 1);
		if (index + 1 < 10) tmp += sprintf(tmp, " ");
		tmp += sprintf(tmp, "   %d   %d   %s", player_team[index], player_config[index], player_type[index]);
		if ((strlen(buf) >= 80)) tw_error("playerListboxGetter string too long");
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



