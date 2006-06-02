/* $Id$ */ 
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

#include "other/ttf.h"
//#include "other/fontmorph.h"

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

#include "menu/mainmenu.h"
#include "menu/menugeneral.h"

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





Game *old_game = NULL;

DATAFILE *scp = NULL;

FILE *debug_file;


int max_networkS = 1;
int MAX_CONFIGURATIONS = 1;
int MAX_TEAMS = 1;


int getKey();

bool game_networked;

bool optimize4latency = true;







Log *new_log (int logtype) { STACKTRACE
	union { Log *log; NetLog *netlog; };
	log = NULL;

	switch (logtype) {
		case Log::log_normal: {
			log = new Log();
			log->init();
			return log;
		}
		case Log::log_net: {
			netlog = new NetLog();
			netlog->init();
			netlog->type = Log::log_net;
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

// the buffer must already exist!!
void share_array(int netnum, char *buffer, int *N)
{
	share(netnum, N);
	share_update();
	share(netnum, buffer, *N);
	share_update();

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


void share_string(int netnum, char *str)
{
	STACKTRACE;

	int Lstr;
	Lstr = strlen(str) + 1;
	share(netnum, &Lstr);
	share_update();
	share(netnum, str, Lstr);	
	share_update();

	message.print(1500, 14, "SHARED [%i] [%s]", Lstr, str);
	message.animate(0);
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

bool log_empty(int channel)
{
	if (glog->log_pos[channel] == glog->log_len[channel])
		return true;
	else
		return false;
}

bool is_local (int channel)
{
	if (is_bot(channel))
		return true;
	else
		return (glog->get_direction (channel) & Log::direction_write);
}

bool is_bot( int channel )
{
	return channel == channel_none;		// bots are placed on "none" channel
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
	for ( i = 0; i < num_network; ++i )
	{
		test1 = 100*rand();
		test2 = 100*rand();
		log_int(test1, channel_network[i]);

		log_int(test2, channel_network[i] + _channel_buffered);
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

/*
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
*/



// CCstatus = Cancel or Continue.
void game_host_menu(int &Nhumans, int &Nbots, char *gname, int &CCstatus)
{
	// init

	BITMAP *temp_screen = 0;
	temp_screen = create_video_bitmap(screen->w, screen->h);
	if (!temp_screen)
		temp_screen = create_bitmap(screen->w, screen->h);


	TWindow *T;
	T = new TWindow("interfaces/multiplayer", 50,50, temp_screen);

	int psize;
	psize = iround(40 * T->scale);
	FONT *usefont1 = load_ttf_font ("fonts/Jobbernole.ttf", psize, 0, 1.5);
	//morph_font(usefont1);
	psize = iround(24 * T->scale);
	FONT *usefont2 = load_ttf_font ("fonts/Jobbernole.ttf", psize, 0, 1.5);
	//morph_font(usefont2);
	psize = iround(20 * T->scale);
	FONT *usefont3 = load_ttf_font ("fonts/Jobbernole.ttf", psize, 0, 1.2);
	//morph_font(usefont3);

	// test:
//	FONT *usefont = load_font_test (40);
//	destroy_font(usefont);
	
	Button *b_accept, *b_cancel;
	b_accept = new Button(T, "accept_");
	b_cancel = new Button(T, "cancel_");

	ButtonValue *b_human, *b_bot;

	b_human = new ButtonValue(T, "humans_", usefont1);
	b_human->set_value(1, Nhumans, max_network);
	b_bot   = new ButtonValue(T, "bots_", usefont1);
	b_bot->set_value(0, Nbots, 99);

	TextButton *game_choice;
	game_choice = new TextButton(T, "type_", usefont1);
	game_choice->set_text(gname, makecol(200,200,200));
	game_choice->passive = false;	// normally it just show info, but now you need it for interaction.

	PopupList *rpopup;
	rpopup = new PopupList(game_choice, "interfaces/multiplayer/gamelist", "list_", 10, 10, usefont3, 0);
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
//		poll_input();
//		poll_mouse();
//		show_mouse(screen);

		T->tree_calculate();
		clear_to_color(temp_screen, 0);
		T->tree_animate();
		blit(temp_screen, screen, 0, 0,  0, 0, temp_screen->w, temp_screen->h);


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
//			clear_to_color(screen, 0);	// erase the screen
		}

		idle(5);
	}

	Nhumans = b_human->value;
	Nbots = b_bot->value;
	strcpy(gname, game_choice->text);

	// exit
	delete T;

	show_mouse(0);

	destroy_bitmap(temp_screen);
	
	show_mouse(screen);

	// for some reason, this crashes if I don't take special precaution (override the kill subroutine
	// with an identical copy as that in allegro... 
	// NEED TO CHECK, WHY !!
	destroy_font(usefont1);
	destroy_font(usefont2);
	destroy_font(usefont3);
//	color_destroy_font(usefont1);
//	color_destroy_font(usefont2);
}




//void play_net ( const char *_address, int _port )
void play_net( bool ishost )
{
	STACKTRACE;

	game_networked = true;

	reset_share();

	reset_color_props();

	// STEP ONE, SETTING / RETRIEVING DATA

	// you've to delete a single-player thingy I suppose... if it exists at all.
	if (glog)
		delete glog;

	NetLog *log = new NetLog();
	log->init();
	set_global(log);	// this sets glog

	log_set_nofake();	// perhaps some (global) settings were still present
	log_resetmode();	// after leaving the last game ... (duh!)

	// resets the channels to their default values ; that's needed, cause if there are
	// bots in the game, channel values can be set to non-human values (-1)
	init_channels();

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
		log->type = Log::log_net;

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
		for ( p = 0; p < max_network; ++p)
		{
			if (p != p_local)
			{
				//log->set_direction(channel_network[p] , Log::direction_read);
				//log->set_direction(channel_network[p] + _channel_buffered, Log::direction_read);
				log->set_r(channel_network[p]);
			}
		}
	

		// user menu: enter port number
//		port = listen_menu( &videosystem.window, port );
		
		if (port == -1) return;
				
		// try to establish a connection to all the required players
		
		int n;
		int val;
		char *addr = 0;
//		int Lstr;

		int w = 99;
		log_int(w, channel_server);
		if (w != 99)
		{
			tw_error("unexpected value");
		}

		net_conn[0] = -1;	// local

		for ( n = 1; n < Nplayers; ++n )	// exclude the local player from this.
		{
	
			char tmp[512];
			sprintf(tmp, "Listening for client # %i", n);
			message.out(tmp);
			message.animate(0);
			
			// listening for another client.
			if (!log->add_listen(port))
				tw_error("listener failed");

			// the index of the connection that was just added :
			net_conn[n] = log->num_connections - 1;
			
			if (net_conn[n] != n-1)
			{
				tw_error("Adding connection with wrong index");
			}

			// you should share data about the new player with all existing players ; include
			// also the player number, so that this new player also gets something useful
			// in addition...

			val = 99;

//			message.print(1500, 14, "SHARING(sending) [%i]", val);
//			message.animate(0);

			addr = log->get_address(log->num_connections-1);

			share(-1, &val);
			share(-1, &n);
			
			/*
			Lstr = strlen(addr) + 1;
			share(-1, &Lstr);
			share_update();
			share(-1, addr, Lstr);
			*/
			share_string(addr);
			
			share_update();

//			message.print(1500, 14, "SHARED [%i] [%i] [%s]", val, n, addr);
//			message.animate(0);

		}

		addr = "signalstart";
		int kstart = 0;		// this signals the end for the client.

		share(-1, &val);
		share(-1, &kstart);
		
		/*
		Lstr = strlen(addr) + 1;
		share(-1, &Lstr);
		share_update();
		share(-1, addr, Lstr);	
		share_update();
		*/


		share_string(addr);

//		message.print(1500, 14, "SHARED [%i] [%i] [%s]", val, kstart, addr);
//		message.animate(0);


		// you're the host, so you determine the gametype !
		// for now, default to a normal melee game
		strcpy(gname, temp_gamename);

	} else {
		log->type = Log::log_net;

		// channel_init and such, are all read-only for the clients
		// uhm, well, actually this is somewhat dangerous, as they're created on-the-fly
		//log->set_all_directions(Log::direction_read);

		int p;
		
		int i;
		for ( i = 0; i <= channel_playback; ++i )
			log->set_direction(i, Log::direction_read);

		//log->set_r(channel_none);
		for ( p = 0; p < max_network; ++p)
		{
			log->set_r(channel_network[p]);
		}

		char *tmp = address;
		// note, that tmp can be changed by the menu, so that it points to a different string?!

		// user menu: enter adress and port number
		if (connect_menu(&videosystem.window, &tmp, &port) == -1) 
			return;

		// saving address
		set_config_string("Network", "Address", tmp);

		message.out("Connecting to server...");
		message.animate(0);

		// connect to the server (whom is player 0 by default, cause nobody else is connected yet.)
		log->add_connect(tmp, port);

		// the index of the connection that was just added :
		// this is to the host, who is player 0 by default, hence net_conn[0]
		// you MUST set this here.
		net_conn[0] = log->num_connections - 1;
		if (net_conn[0] != 0)
		{
			tw_error("First connection should have index 0");
		}

//		free(tmp);
		
		// receive some data on channel_init
		
//		int Lstr;
		int val = -1;
//		message.print(1500, 14, "SHARING(receiving)");
//		message.animate(0);

		char addr[512];
		

		p_local = 0;	// uninitialized value ; you know, cause client can't be player 0
		for (;;)
		{
			share(-1, &val);
			share(-1, &p);	// you receive your own (local) player number (in the first iteration)
			// or, you send it, in later iterations.
			
			/*
			share(-1, &Lstr);
			share_update();
			share(-1, addr, Lstr);
			share_update();
			*/
			share_string(addr);

//			message.print(1500, 14, "SHARED [%i] [%i] [%s]", val, p, addr);
//			message.animate(0);


			if (!p_local)
			{
				p_local = p;	// IMPORTANT TO SET THIS !!

				// also, the new client should listen to all existing clients (if any)
				int idone;
				for ( idone = 1; idone < p_local; ++ idone )	// exclude one, because that's yourself...
				{
					if (!log->add_listen(port))
						tw_error("listener failed");

					net_conn[idone] = log->num_connections - 1;
				}

			} else {
				
				if ( p == 0 )
					break;		// this signals the start of the game

				// otherwise, connect to any newly connected client
				log->add_connect(addr, port);

				// the index of the connection that was just added :
				// temporary assignment...
				net_conn[p] = log->num_connections - 1;
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


	// note that the server has "local" settings which overwrite other players' setting
	num_network = Nplayers;
	num_bots = Nbots;

	message.print(1500, 13, "numnet [%i]", num_network);

	share(-1, &num_network);
	share(-1, &num_bots);
	share_update();
	// you've to share num_network this as early as possible, cause it's also
	// important for the debugging part (log_test).


	// now, each player should share its assigned player number with all other players...
	// so that the player indices are synchronized between the different games.

	int remap_player_index[max_network];
	int i;

	for ( i = 0; i < num_network; ++i )
	{
		
		remap_player_index[i] = p_local;
		log_int(remap_player_index[i], channel_network[i]);
		// note that channel_network are fixed channel indexes.
		//xx or not??? and that net_conn[k] is used as channel_network[net_conn[k]]...

	}

	/*
	for ( i = 0; i < num_network; ++i )
	{
		if (i != p_local)
		{
			message.print(1500, 15, "player [%i]: net_conn = [%i], addr = [%s], remap = [%i]",
				i, net_conn[i], log->net[net_conn[i]].addr, remap_player_index[i]);
		}
	}
	*/

	// next, apply the remapping

	int new_net_conn[max_network];
	for ( i = 0; i < num_network; ++i )
	{
		int k;
		k = remap_player_index[i];
		new_net_conn[k] = net_conn[i];	// the synched index k, maps from local index i

	}

	for ( i = 0; i < num_network; ++i )
	{
		net_conn[i] = new_net_conn[i];
	}
	// the remapping is complete.

	/*
	// now, the server should let everyone know which player number is associated with which connection
	// (it's easiest to initialize it this way, I think)
	// this is only used to initialize the clients, the host just needs to inform them.
	int i;
	for ( i = 1; i < num_network; ++i )
	{
		char addr[512];
		strncpy(addr, log->net[i].addr, 500);
		share_string(-1, addr);

		if (p_local > 0)	// if you're not host, cause the host already knows
		{
			if (i == p_local)
			{
				net_conn[i] = -1;	// default value, if it's your own address
			}
			else
			{

				// check all connections, and see which one fits...
				int k;
				for ( k = 0; k < log->num_connections; ++k )
				{
					message.print(1500, 13, "comparing [%s] to [%s]", log->net[k].addr, addr);

					if ((strcmp(log->net[k].addr, addr) == 0) && (strlen(log->net[k].addr) == strlen(addr)) )
					{
						break;
					}
				}
				message.animate(0);


[22:43] Megagun: Can't remember.. Been a while ago.
[22:44] Megagun: Hmm.. Damn "The address of one of the players wasn't found" bug...
[22:44] Megagun: When networking in TW.
[22:44] Megagun: regular TW, 3player.
[22:45] GeomanNL: you play the game, and suddenly there's that error ?
[22:45] Megagun: When we try to connect
[22:45] Megagun: First player can connect allright, second player cant
[22:45] Megagun: ah
[22:45] Megagun: clicking "ignore" on that bug fixes it
[22:46] GeomanNL: ??
[22:46] Megagun: Odd.
[22:46] Megagun: That bug shows up, both players click "ignore", and it seems to fix itself.
[22:46] Halleck: maybe the bug is the dialogue window itself
[22:46] GeomanNL: might be a mistake I made one day
[22:46] Megagun: heh
[22:47] Megagun: It works on 1v1 though

				if (k == 0)
					tw_error("Finding the host another time... should not happen");

				if (k < 0 || k >= log->num_connections)
					tw_error("The address of one of the players was not found");
				
				net_conn[i] = k;	// player i uses connection k.
				
			}
		}
	}
	*/
	
	

	for ( i = 0; i < num_network; ++i )
	{
		if (i != p_local)
		{
			message.print(1500, 15, "player [%i]: net_conn = [%i], addr = [%s]",
				i, net_conn[i], log->net[net_conn[i]].addr);
		}
	}
	message.animate(0);
	//readkey();

	// for log_test, also (re-)initialize the rng
	rand_resync();


	log_test("a1");

	//log_resetmode();

	log->reset();

	log_test("a2");

	// find out, how many hotseat players there are on this computer
	// this is done by checking the local player-settings.
	// these are stored in scp.ini
	// (that's the way it used to be done - ideally you've an interface option for this).
	num_hotseats[p_local] = 1;

	//xxx disable the hotseats stuff...
	/*
	set_config_file("scp.ini");
	int i;
	for ( i = 0; true; ++i)
	{
		char buffy[64];
		sprintf(buffy, "Player%d", i + 1);
		
		const char *type = get_config_string(buffy, "Type", NULL);
		if (!type)
			break;

		if (strcmp(type, "Human") == 0)		// a human player found.
			++num_hotseats[p_local];
	}
		*/
	if (num_hotseats[p_local] == 0)		// there should be at least 1 local player.
		++num_hotseats[p_local];



	int k1 = 2, k2 = 3, k3 = 5;
	share(-1, &k1);
	share(-1, &k2);
	share(-1, &k3);
	share_update();
	log_test("a10");


	message.print(1500, 13, "numnet [%i]", num_network);
	message.animate(0);

	// NOTE NOTE NOTE
	// that log_test depends on num_network,
	// also, it depends on random()
	log_test("a3");

	message.print(1500, 13, "numnet [%i]  numhotseat [%i][%i]", num_network, num_hotseats[0], num_hotseats[1]);

	// next, share how many hotseat-players there are, on each computer.
	
	int p;
	for ( p = 0; p < num_network; ++p )
	{
		log_int(num_hotseats[p], channel_network[p]);
	}
	//share_update();

	log_test("a4");


	for ( p = 0; p < num_network; ++p )
		message.print(1500,15, "Nhotseats [%i]", num_hotseats[p]);
	
	// no controls (ships) added yet -> should be handled by the game
	// but, based on the information provided here, we can already calculate the
	// number of players that will enter the game ..
	num_players = 0;
	for ( p = 0; p < num_network; ++p )
	{

		int i;
		for ( i = 0; i < num_hotseats[p]; ++i )
		{
//			player_connection_num[num_players] = p; THIS is not guaranteed to work ..
			++num_players;
		}
	}
	num_players += num_bots;
	// this value can be used for game initialization already...

	

	
	share_string(gname);

	log_test("b1");

	message.print(1500,15, "gname [%s]", gname);
	message.animate(0);
	//log_test();
	//readkey();

	set_config_file("client.ini");
	optimize4latency = (get_config_int("Network", "OptimizeLatency", 1) != 0);
	if (optimize4latency)
		log->optimize4latency();
	else
		log->optimize4bandwidth();

	//message.out("connection established");
	
	log_test("b2");

	// NOTE
	// that it's possible that, just before a reset, a remote computer is already
	// transmitting data, which've been received by coincidence while you're resetting
	// the data. You shouldn't reset those data of course, cause they're needed for
	// subsequent actions ... so,
	// first you've to log-int a dummy value, to make sure that the two games
	// are in synch. After that it's ok to clear them, cause there are NO lingering
	// data then ...
	// hmm
	// however, in >2 player environment, simple sharing of a variable doesn't
	// work. Utter chaos I think ...
	log->reset();

	log_test("c");

	if (get_share_num() != 0)
	{
		tw_error("Share error??");
	}

	play_game(gname, log);

	return;
}

void play_net1client ( const char *_address, int _port )
{
	STACKTRACE;

	NetLog *log = new NetLog();
	log->init();
	log->type = Log::log_net;

	log->set_all_directions(Log::direction_read);
	int p;
	for ( p = 0; p < max_network; ++p )	// note, 0==server.
	{
		log->set_direction(channel_network[p] , Log::direction_write | Log::direction_read | NetLog::direction_immediate);
		log->set_direction(channel_network[p] + _channel_buffered, Log::direction_write | Log::direction_read);
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

	log->type = Log::log_net;
	log->set_all_directions(Log::direction_write | Log::direction_read | NetLog::direction_immediate);
	int p;
	for ( p = 1; p < max_network; ++p )	// note, 0==server.
	{
		log->set_direction(channel_network[p] , Log::direction_read);
		log->set_direction(channel_network[p] + _channel_buffered, Log::direction_read);
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

	game_networked = false;


	reset_share();

	p_local = 0;		// IMPORTANT TO SET THIS !!
	// in a single-player game, your computer always is "host", to the local game...
	
	// channel_init and such, are all write for the one who hosts.
	glog->type = Log::log_normal;
	
	int i;
	for ( i = 0; i <= channel_playback; ++i )
		glog->set_direction(i, Log::direction_read | Log::direction_write | Log::direction_immediate);

	glog->set_rw(channel_playback);	// perhaps this is a better setting for the playback channel
	glog->set_rw(channel_local());

	
	// the remote players are always read-only (local player is always write, of course)
	// well.. actually, all the channels are used locally in single-player, there are no remote players,
	// therefore:
	int p;
	for ( p = 0; p < max_network; ++p)
	{
		if (p != p_local)
			glog->set_rw(channel_network[p]);
	}

	// initial values:.
	num_network = 1;
	num_bots = 0;
	num_players = 1;
	
	
	play_game(_gametype_name, _log);
}


void remove_players()
{
	int i;
	for ( i = 0; i < num_players; ++i )
	{
		if (player[i])
		{
			delete player[i];
			player[i] = 0;
		}
	}
}

void reset_network(Log *log)
{
	// this is perhaps a bit useless, here...
	log->clear();

	events.clear();

	int i;
	for ( i = 0; i < max_network; ++i  )
	{
		if (player[i])
		{
			player[i]->desynch_Nitems = 0;
		}
	}
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
		_log->init();		// initialization is done further below.
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

		reset_network(_log);

		new_game->preinit();
		new_game->window = new VideoWindow;
		new_game->window->preinit();
		new_game->window->init(&videosystem.window);
		new_game->window->locate(0,0,0,0,0,1,0,1);
		new_game->init(_log);
		new_game->play();

		reset_network(_log);

		if (glog) delete glog;
		glog = 0;

		game = NULL;
		new_game->game_done = true;
		old_game = new_game;

		remove_players();
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


FONT *TW_font = NULL;

// dialog results
/*int mainRet = 0;
int shipRet = 0;
int keyRet = 0;
int fleetRet = 0;
int optionsRet= 0;*/







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

		if (video_screen)
		{
			destroy_bitmap(video_screen);
			video_screen = 0;
		}
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

				if (!type)
					tw_error("Unrecognized ship item[%i]", si);

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

				if (!sprite)
					tw_error("Failed to initialize sprite");
                

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



