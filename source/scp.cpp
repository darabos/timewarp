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

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <allegro.h>
#include <assert.h>

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

#define SCPGUI_MUSIC   0
#define SCPGUI_TITLE   1


Game *old_game = NULL;

DATAFILE *scp = NULL;

FILE *debug_file;

/*
 * void showTitle(VideoWindow *window)
 *
 * Blits GUI background bitmap on to a video window.
 */
void showTitle(VideoWindow *window = &videosystem.window);

/*
 * extended_menu(int i);
 *
 * MELEE_EX dialog - selects alternate games as opposed to standard melee. From here you
 * can access diagnostics (DIAGNOSTICS dialog) and ship info. (SHIPVIEW dialog) You can
 * also test key jamming from this dialog.
 */
void extended_menu(int i = -1);

/*
 * void change_teams();
 *
 * TEAMS dialog - from here you can select controllers for each player, and access
 * their respective fleets. (FLEET dialog)
 */
void change_teams();

/*
 * edit_fleet(int player);
 *
 * FLEET dialog - manages fleet compositions for an individual player.
 *
 *
 * PARAMETERS
 *
 * int player
 *   Player index indicating which player's fleet to edit.
 */
void edit_fleet(int player);

/*
 * ship_view_dialog(int si, Fleet *Fleet);
 *
 * SHIPVIEW dialog - displays statistics and text information (if available) about
 * the currently installed ships.
 *
 *
 * PARAMETERS
 *
 * int si
 *   Ship index. By default 0, the first ship in the fleet.
 *
 * Fleet *fleet
 *   Pointer to a fleet upon which the ship list is built. When this parameter
 *   is set to the default value NULL, the reference fleet is used to build
 *   the ship list.
 */
//void ship_view_dialog(int si = 0, Fleet *fleet = NULL);

/*
 * show_diagnostics();
 *
 * DIAGNOSTICS dialog - displays version number and platform data.
 */
void show_diagnostics();

/*
 * void keyjamming_tester();
 *
 * Opens a screen showing which keys are currently pressed. Here the user may test
 * various key combinations for conflicts.
 */
void keyjamming_tester();


void play_demo(const char *file_name = "demo.dmo") ;
void play_game(const char *_gametype_name, Log *_log = NULL) ;
void play_net1client ( const char * address = NULL, int port = -1 ) ;
void play_net1server ( const char *_gametype_name, int port = -1 ) ;


int getKey();

enum {
	MAIN_DIALOG_BOX = 0,
	MAIN_DIALOG_MELEE,
	MAIN_DIALOG_MELEE_EXTENDED,
	MAIN_DIALOG_TEAMS,
	MAIN_DIALOG_OPTIONS,
	MAIN_DIALOG_HELP,
	MAIN_DIALOG_EXIT
};

DIALOG mainDialog[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_shadow_box_proc, 40,   40,   180,  215,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_button_proc,     45,   45,   170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Melee" , NULL, NULL },
  { d_button_proc,     45,   80,   170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Extended Menu" , NULL, NULL },
  { d_button_proc,     45,   115,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Teams" , NULL, NULL },
  { d_button_proc,     45,   150,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Options", NULL, NULL },
  { d_button_proc,     45,   185,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Help", NULL, NULL },
  { d_button_proc,     45,   220,  170,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Exit", NULL, NULL },
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
  { d_button_proc,     255,  240,  60,   18,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"OK", NULL, NULL },
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
		}
		case Log::log_net1server: {
			log = new NetLog();
			log->init();
			log->type = Log::log_net1server;
		}
		default: {
			tw_error("that is not a valid log type");
		}
	}
	return log;
}

char *detect_gametype( Log *_log ) { STACKTRACE
	int ltype;
	_log->unbuffer(Game::channel_init, &ltype, sizeof(int));
	ltype = intel_ordering(ltype);
	int gnamelength;
	_log->unbuffer(Game::channel_init, &gnamelength, sizeof(int));
	gnamelength = intel_ordering(gnamelength);
	if (gnamelength > 1000) {
		tw_error("Game name too long");
		gnamelength = 1000;
	}
	char buffy[1024];
	_log->unbuffer(Game::channel_init, &buffy, gnamelength);
	buffy[gnamelength] = 0;
	_log->reset();
	return strdup(buffy);
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
  { d_button_proc,     50,   320,  160,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Connect", NULL, NULL },
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
  { d_button_proc,     140,  200,  120,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void*) "OK", NULL, NULL },
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

void play_net1client ( const char *_address, int _port ) {STACKTRACE
	NetLog *log = new NetLog();
	log->init();
	log->type = Log::log_net1client;

	log->set_all_directions(Log::direction_read);
	log->set_direction(Game::channel_client , Log::direction_write | Log::direction_read | NetLog::direction_immediate);
	log->set_direction(Game::channel_client + Game::_channel_buffered, Log::direction_write | Log::direction_read);
	
	set_config_file("client.ini");
	char address[128];
	int port, i;
	while (!log->net.isConnected()) {
		if (!_address) strncpy(address, get_config_string("Network", "Address", ""), 127);
		else strncpy(address, _address, 127);
		if (_port == -1) port = get_config_int("Network", "Port", 15515);
		else port = _port;
		char *addressaddress = address;
		if (connect_menu(&videosystem.window, &addressaddress, &port) == -1) 
			return;
		set_config_string("Network", "Address", addressaddress);
		message.out("...");
		i = ((NetLog*)log)->net.connect(addressaddress, port, is_escape_pressed);
		free(addressaddress);
		if (i) {
//						while (is_escape_pressed());
			while (keypressed()) readkey();
			tw_error("connection failed");
		}
	}

	log->net.optimize4latency();
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
	log->set_direction(Game::channel_client , Log::direction_read);
	log->set_direction(Game::channel_client + Game::_channel_buffered, Log::direction_read);
	log->set_direction(Game::channel_server + Game::_channel_buffered, Log::direction_write | Log::direction_read);
	
	set_config_file("client.ini");
	int port;
	while (!log->net.isConnected()) {
		if (_port == -1) port = get_config_int("Network", "Port", 15515);
		else port = _port;

		port = listen_menu( &videosystem.window, port );
		if (port == -1) return;

		message.out("...");
		log->net.listen(port, is_escape_pressed);
		
	}

	log->net.optimize4latency();
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

void play_game(const char *_gametype_name, Log *_log) {STACKTRACE
	bool gui_stuff = false;
	char gametype_name[1024];
	char *c;
	Game *new_game = NULL;

	strncpy(gametype_name, _gametype_name, 1000);
	for (c = gametype_name; c = strchr(c, '_'); *c = ' ');

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

		if (!_log) {
			_log = new Log();
			_log->init();
		}

		GameType *type = gametype(gametype_name);
		if (type)
			new_game = type->new_game();
		else
			throw "wait a sec... I can't find that game type";

		videosystem.window.lock();
		clear_to_color(videosystem.window.surface, palette_color[8]);
		videosystem.window.unlock();
		new_game->preinit();
		new_game->window = new VideoWindow;
		new_game->window->preinit();
		new_game->window->init(&videosystem.window);
		new_game->window->locate(0,0,0,0,0,1,0,1);
		new_game->init(_log);
		new_game->play();
		new_game->log->deinit();
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
		scp = load_datafile("scpgui.dat");
		sound.play_music((Music *)(scp[SCPGUI_MUSIC].dat), TRUE);
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
	r = getController(player_type[i], tmp, Game::channel_none);
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

	scp = load_datafile("scpgui.dat");
	if (!scp)
		tw_error("Couldn't load scpgui.dat");
	sound.play_music((Music *)(scp[SCPGUI_MUSIC].dat), TRUE);

	showTitle();
	enable();
	int mainRet;
	do {
		//mainRet = popup_dialog(mainDialog, MAIN_DIALOG_MELEE);
		mainRet = tw_do_dialog(window, mainDialog, MAIN_DIALOG_MELEE);
		switch (mainRet) {
			case MAIN_DIALOG_MELEE:
				disable();
				play_game("Melee");
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
		}
	} while((mainRet != MAIN_DIALOG_EXIT) && (mainRet != -1));
}

int tw_main(int argc, char *argv[]);

int main(int argc, char *argv[]) { STACKTRACE
	int r;
	r = tw_main(argc, argv);
	return r;
}
END_OF_MAIN();

int tw_main(int argc, char *argv[]) { STACKTRACE
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
	log_debug("Log started(%d)\n", 1);
	if (allegro_init() < 0)
		tw_error_exit("Allegro initialization failed");
	videosystem.preinit();
	init_error();

	try {
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

		int inputs = 7;

		// parse command-line arguments
		for (i = 0; i < argc; i += 1) {
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

		enable_input(inputs);
		sound.init();
		sound.load();
		videosystem.set_resolution(screen_width, screen_height, screen_bpp, fullscreen);

		View *v = NULL;
		v = get_view ( get_config_string("View", "View", NULL) , NULL );
		if (!v) v = get_view ( "Hero", NULL );
		set_view(v);

		init_ships();
		init_fleet();
		init_time();

		if (auto_play) {// FIX ME
			if (!strcmp(auto_play, "game")) play_game(auto_param, NULL);
			if (!strcmp(auto_play, "demo")) play_demo(auto_param);
			if (!strcmp(auto_play, "net1client")) play_net1client(auto_param, auto_port);
			if (!strcmp(auto_play, "net1server")) play_net1server(auto_param, auto_port);
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
	int i = tw_popup_dialog(NULL, select_game_dialog, 2);
	if (i == -1) return NULL;
	else return game_names[select_game_dialog[2].d1];
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
  { d_button_proc,     50,   50,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Play Game" , NULL, NULL },
  { d_button_proc,     50,   75,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Play Game / Net1 Server" , NULL, NULL },
  { d_button_proc,     50,  100,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Play Game / Net1 Client" , NULL, NULL },
  { d_button_proc,     50,  125,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Playback Demo Recording" , NULL, NULL },
  { d_button_proc,     50,  150,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Save Demo Recording" , NULL, NULL },
  { d_button_proc,     50,  175,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Clear State", NULL, NULL },
  { d_button_proc,     50,  200,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Key Tester", NULL, NULL },
  { d_button_proc,     50,  225,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Ship Info", NULL, NULL },
  { d_button_proc,     50,  250,  190,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Diagnostics", NULL, NULL },
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
			if (gname) play_game(gname);
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
			if (!old_game || !old_game->log) {
				tw_alert ( "Aaaaarggh! No recording to save!", ":(");
				return;
			}
			old_game->log->save("demo.dmo");
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
  { d_button_proc,     295,  240,  150,  20,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Select Controller", NULL, NULL },
  { d_button_proc,     50,   255,  220,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Change Team #", NULL, NULL },
  { d_button_proc,     50,   285,  220,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Change Config #", NULL, NULL },
  { d_button_proc,     50,   315,  220,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Edit Config", NULL, NULL },
  { d_button_proc,     50,   345,  220,  25,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Edit Fleet", NULL, NULL },
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
	FLEET_DIALOG_FLEETTITLE = 0,
	FLEET_DIALOG_FLEETLIST,
	FLEET_DIALOG_SHIPLIST,
	FLEET_DIALOG_SELECT,
	FLEET_DIALOG_CLEAR,
	FLEET_DIALOG_INFO,
	FLEET_DIALOG_PLAYERTEXT,
	FLEET_DIALOG_SAVE,
	FLEET_DIALOG_LOAD,
	FLEET_DIALOG_SORTBYNAME,
	FLEET_DIALOG_SORTBYCOST,
	FLEET_DIALOG_TITLE,
	FLEET_DIALOG_MAIN,
	FLEET_DIALOG_FLEETBITMAP,
	FLEET_DIALOG_CLEARALL,
	FLEET_DIALOG_SORTBYORIGIN,
	FLEET_DIALOG_SORTBYCODERS,
	FLEET_DIALOG_NULL
};

char *numeric_string[] = {"Zero", "One", "Two", "Three", "Four", 
		"Five", "Six", "Seven", "Eight", "Nine", "Ten", "Eleven", 
		"Twelve"};

char fleetPlayer[18];
char fleetTitleString[100];

int scp_fleet_dialog_text_list_proc(int msg, DIALOG* d, int c);

int scp_fleet_dialog_bitmap_proc(int msg, DIALOG* d, int c);


// FLEET - dialog structure
DIALOG fleetDialog[] = {
  // (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)
  { d_textbox_proc,    390,  40,   240,  60,   255,  0,    0,    0,       0,    0,    (void *)fleetTitleString, NULL, NULL },
  { d_list_proc2,      390,  110,  240,  280,  255,  0,    0,    D_EXIT,  0,    0,    (void *)fleetpointsListboxGetter, NULL, NULL },
  { scp_fleet_dialog_text_list_proc,
                       10,   90,   240,  380,  255,  0,    0,    D_EXIT,  0,    0,    (void *)shippointsListboxGetter, NULL, NULL },
  { d_button_proc,     256,  220,  128,  40,   255,  0,    0,    D_EXIT,  0,    0,    (void *)">>", NULL, NULL },
  { d_button_proc,     256,  270,  128,  40,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"<<", NULL, NULL },
  { d_button_proc,     256,  330,  128,  40,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Info", NULL, NULL },
  { d_textbox_proc,    390,  10,   240,  30,   255,  0,    0,    0,       0,    0,    (void *)fleetPlayer, NULL, NULL },
  { d_button_proc,     400,  400,  100,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Save", NULL, NULL },
  { d_button_proc,     520,  400,  100,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Load", NULL, NULL },
  { d_button_proc,     25,   10,   100,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"by name", NULL, NULL },
  { d_button_proc,     135,  10,   100,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"by cost", NULL, NULL },
  { d_button_proc,     400,  440,  100,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Title", NULL, NULL },
  { d_button_proc,     256,  430,  128,  40,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Main", NULL, NULL },
  { scp_fleet_dialog_bitmap_proc,
                       256,  10,   128,  200,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { d_button_proc,     520,  440,  100,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Clear", NULL, NULL },
  { d_button_proc,     25,   50,   100,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"by origin", NULL, NULL },
  { d_button_proc,     135,  50,   100,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"by coders", NULL, NULL },
  { d_tw_yield_proc,   0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
  { NULL,              0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    NULL, NULL, NULL }
};


// FLEET - dialog function
void edit_fleet(int player) {STACKTRACE
	char tmp[40];
	char path[80];

	sprintf (tmp, "Player%d", player+1);
	Fleet* fleet = new Fleet;
	fleet->load("fleets.ini", tmp);
	if (player + 1 <= 12)
		sprintf(fleetPlayer, "Player %s Fleet", numeric_string[player+1]);
	else sprintf(fleetPlayer, "Player%d Fleet", player+1);
	showTitle();

	int fleetRet;
	do {
		sprintf(title_str, fleet->title);
		sprintf(fleetTitleString, "%s\n%d points", fleet->title, fleet->cost);

		fleetDialog[FLEET_DIALOG_FLEETLIST].dp3 = fleet;
		fleetRet = tw_do_dialog(NULL, fleetDialog, FLEET_DIALOG_SHIPLIST);

		if ((fleetRet == FLEET_DIALOG_SELECT) ||
				(fleetRet == FLEET_DIALOG_SHIPLIST)) {
			fleet->select_slot(
				fleetDialog[FLEET_DIALOG_FLEETLIST].d1,
				reference_fleet->ship[fleetDialog[FLEET_DIALOG_SHIPLIST].d1]);
			fleetDialog[FLEET_DIALOG_FLEETLIST].d1++;
			SEND_MESSAGE(&fleetDialog[FLEET_DIALOG_FLEETLIST], MSG_DRAW, 0);
		}

		if ((fleetRet == FLEET_DIALOG_CLEAR) ||
				(fleetRet == FLEET_DIALOG_FLEETLIST)) {
			fleet->clear_slot(fleetDialog[FLEET_DIALOG_FLEETLIST].d1);
			fleetDialog[FLEET_DIALOG_FLEETLIST].d1++;
			SEND_MESSAGE(&fleetDialog[FLEET_DIALOG_FLEETLIST], MSG_DRAW, 0);
		}

		if(fleetRet == FLEET_DIALOG_SAVE) {
			sprintf(path, "fleets/");
			if(file_select("Save Fleet", path, "scf")) fleet->save(path, "Fleet");
			showTitle();
		}

		if(fleetRet == FLEET_DIALOG_LOAD) {
			sprintf(path, "fleets/");
			if(file_select("Load Fleet", path, "scf")) fleet->load(path, "Fleet");
			sprintf(title_str, fleet->title);
			sprintf(fleetTitleString, "%s\n%d points", fleet->title, fleet->cost);
			showTitle();
		}

		if (fleetRet == FLEET_DIALOG_INFO) {
			ship_view_dialog(fleetDialog[FLEET_DIALOG_SHIPLIST].d1, reference_fleet);
			showTitle();
//			dialog_message ( fleetDialog, MSG_DRAW, 0, NULL);
		}

		if(fleetRet == FLEET_DIALOG_SORTBYNAME) {
			reference_fleet->sort_alphabetical("Name2");
			reference_fleet->sort_alphabetical("Name1");
			//reference_fleet->sort(fleetsort_by_name);
			SEND_MESSAGE(&fleetDialog[FLEET_DIALOG_SHIPLIST], MSG_DRAW, 0);
		}

		if(fleetRet == FLEET_DIALOG_SORTBYCOST) {
			reference_fleet->sort_numerical("TWCost");
			//reference_fleet->sort(fleetsort_by_cost);
			SEND_MESSAGE(&fleetDialog[FLEET_DIALOG_SHIPLIST], MSG_DRAW, 0);
		}

		if(fleetRet == FLEET_DIALOG_SORTBYORIGIN) {
			reference_fleet->sort_alphabetical("Origin");
			SEND_MESSAGE(&fleetDialog[FLEET_DIALOG_SHIPLIST], MSG_DRAW, 0);
		}

		if(fleetRet == FLEET_DIALOG_SORTBYCODERS) {
			reference_fleet->sort_alphabetical("Coders");
			SEND_MESSAGE(&fleetDialog[FLEET_DIALOG_SHIPLIST], MSG_DRAW, 0);
		}

		if(fleetRet == FLEET_DIALOG_TITLE) {
			if(do_dialog(fleet_titleDialog, FLEET_TITLE_DIALOG_BOX) == FLEET_TITLE_DIALOG_OK)
				sprintf(fleet->title, title_str);
			showTitle();
		}

		if(fleetRet == FLEET_DIALOG_CLEARALL) {
			fleet->reset();
			fleetDialog[FLEET_DIALOG_FLEETLIST].d1 = 0;
			SEND_MESSAGE(&fleetDialog[FLEET_DIALOG_SHIPLIST], MSG_DRAW, 0);
		}
	} while((fleetRet != FLEET_DIALOG_MAIN) && (fleetRet != -1));

	fleet->save("fleets.ini", tmp);
	delete fleet;
	showTitle();
}

int scp_fleet_dialog_text_list_proc(int msg, DIALOG* d, int c) {
	static int next_anim_time = get_time();
	int old_d1 = d->d1;
	int ret = d_text_list_proc( msg, d, c );

	if ( d->d1 != old_d1 || msg == MSG_START || (msg == MSG_IDLE && next_anim_time < get_time()) ) {
		next_anim_time = get_time() + 500 + rand() % 3500;
		ShipType* type = reference_fleet->ship[d->d1];

		BITMAP* panel = NULL;
		DATAFILE* data = NULL;
		if (type && type->data) 
			data = load_datafile_object(type->data->file, "SHIP_P00_PCX");

		if(data) {
			BITMAP* bmp = (BITMAP*)data->dat;
			BITMAP* tmp = create_bitmap(bmp->w, bmp->h);
//			BITMAP* tmp = create_bitmap_ex( color_depth, bmp->w, bmp->h );
			blit(bmp, tmp, 0, 0, 0, 0, bmp->w, bmp->h);
			unload_datafile_object(data);

			char obj[32];
			sprintf(obj, "SHIP_P0%d_PCX", 1 + rand() % 4);
			data = load_datafile_object(type->data->file, obj);
			bmp = (BITMAP*)data->dat;
			blit(bmp, tmp, 0, 0, 4, 65, bmp->w, bmp->h);
			unload_datafile_object(data);
//			gamma_correct_bitmap( tmp, gamma_correction, 0 );

			panel = create_bitmap(128, 200);
//			panel = create_bitmap_ex( color_depth, 128, 200 );
			stretch_blit(tmp, panel, 0, 0, tmp->w, tmp->h, 0, 0, panel->w, panel->h);
			destroy_bitmap(tmp);
		}

		if(fleetDialog[FLEET_DIALOG_FLEETBITMAP].dp)
			destroy_bitmap( (BITMAP*)fleetDialog[FLEET_DIALOG_FLEETBITMAP].dp );
		fleetDialog[FLEET_DIALOG_FLEETBITMAP].dp = panel;
		scare_mouse();
		SEND_MESSAGE(&fleetDialog[FLEET_DIALOG_FLEETBITMAP], MSG_DRAW, 0);
		unscare_mouse();
	}

	return ret;
}

int scp_fleet_dialog_bitmap_proc(int msg, DIALOG* d, int c) {
	if (msg == MSG_END && d->dp) {
		destroy_bitmap( (BITMAP*)d->dp );
		d->dp = NULL;
	}

	if (msg != MSG_DRAW || d->dp)
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
  { d_button_proc,     300,  5,    120,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Done", NULL, NULL },
  { d_button_proc,     300,  45,   120,  30,   255,  0,    0,    D_EXIT,  0,    0,    (void *)"Font Size", NULL, NULL },
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
				ShipType *type = reference_fleet->ship[si];
				PACKFILE *f;
				DATAFILE *d;

				if (sprite)
					destroy_bitmap(sprite);
				sprite = NULL;
				type->data->lock();
				if (type->data->spriteShip) {
					sprite = create_bitmap(180, 180);
					BITMAP *tmp = type->data->spriteShip->get_bitmap_readonly(0);
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
		if(r == SHIPVIEW_DIALOG_SORT) {
			i = shipviewDialog[SHIPVIEW_DIALOG_SORT].d1;

			switch (i) {
			case SHIPVIEW_DIALOG_SORT_BYNAME:
				reference_fleet->sort_alphabetical("Name2");
				reference_fleet->sort_alphabetical("Name1");
				break;
			case SHIPVIEW_DIALOG_SORT_BYCOST:
				reference_fleet->sort_numerical("TWCost");
				break;
			case SHIPVIEW_DIALOG_SORT_BYORIGIN:
				reference_fleet->sort_alphabetical("Origin");
				break;
			case SHIPVIEW_DIALOG_SORT_BYCODERS:
				reference_fleet->sort_alphabetical("Coders");
				break;
			}

			SEND_MESSAGE(&shipviewDialog[SHIPVIEW_DIALOG_LIST], MSG_DRAW, 0);
    }
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


