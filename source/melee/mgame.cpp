/* $Id$ */ 
/*
 * Star Control - TimeWarp
 *
 * melee/mgame.cpp - Melee base game module
 *
 */

//#define REVERSE_CONNECT
//for debuging

#include <exception>
using std::exception;
#include <typeinfo>//

#include <stdio.h>
#include <string.h>
//#include <stdlib.h>//
#include <string.h>
#include <allegro.h>

#include "../melee.h"
REGISTER_FILE
#include "../scp.h"
#include "../util/net_tcp.h"
#include "../util/history.h"
#include "../util/sounds.h"
#include "../util/get_time.h"
#include "../frame.h"
#include "../libs.h"

#include "mframe.h"
#include "mgame.h"
#include "mcontrol.h"
#include "mship.h"
#include "mshppan.h"
#include "mview.h"
#include "mlog.h"
#include "mnet1.h"
#include "mfleet.h"

#include <typeinfo>

#include <stdarg.h>

/** this enables a (costly) network desynch test, by checking all the physics items on position. */
int do_desynch_test = 0;

/** this forces each player to experience the same lag */
int global_lag_synch = 0;

/** \brief this causes lots of extra checks to be performed on object-by-object
synch comparison. This is ok for testing on lan, but is best set to false
for internet play, and non-testing compilations.
*/
bool detailed_network_check = false;


bool PlayerInformation::haschannel(int ch)
{
	return (channel == ch  ||  channel + _channel_buffered == ch );
}

bool PlayerInformation::islocal()
{
	if (channel < 0)
		return false;// it's a bot, not a human player!
	return is_local(channel);
}

bool PlayerInformation::ishost()
{
	// player[0] is the host by default.
	return player[0] == this;
}

bool hostcomputer()
{
	// if this computer has the host, then p_local=0
	return p_local == 0;
}

// destroy stuff that belongs to the player.
void PlayerInformation::die()
{
	control->die();
}

PlayerInformation::PlayerInformation()
{
	status = true;
	control = 0;
	color = 1;
	strcpy(name, "default");
	team = 0;
	desynch_Nitems = 0;
	desynch_received = false;
	crc = 0;
}

NPI::NPI()
{
	fleet = 0;
	fleet = new Fleet();
}
NPI::~NPI()
{
	if (fleet)
		delete fleet;
}


void display_channel_info(char *txt = "")
{
	int p;
	for ( p = 0; p < num_network; ++p )
	{
		if (!player[p])
			continue;

		int ch = player[p]->channel;
		message.print(1500, 15, "[%s] player[%i]  (direct) [%i]  (buffered) [%i]", txt, p,
			glog->log_len[ch] - glog->log_pos[ch],
			glog->log_len[ch+1] - glog->log_pos[ch+1]);
	}
	message.animate(0);
	//if (hostcomputer()) readkey();
}





void idle_extra(int time)
{
	if (game) game->use_idle(time);
	if (glog) glog->use_idle(time);

	idle(time);	
}



static int next_frame = 0;


void Game::increase_latency(int n)
{
	// make sure both have identical lag !!
	// to keep games synchronized (and to aid in pushing synched fake data ???)
	share(-1, &n);

	lag_frames += n;
	next_frame += n;

	//message.print(1500, 13, "Increasing latency [%i]", n);
	//message.animate(0);
	//if (hostcomputer()) readkey();
}

void Game::decrease_latency(int n)
{
	// make sure both have identical lag !!
	share(-1, &n);

	lag_frames -= n;
	next_frame -= n;

//	message.print(1500, 13, "Decreasing latency [%i]", n);
//	message.animate(0);
}



void Game::event_sharecontrols(int iplayer)
{
	if (!log_synched)
	{
		// write mode...

		// you should never share the array-index, since that can change... when a player is removed
		log_int( iplayer );
		log_short( player[iplayer]->control->keys );
	} else {
		
		// in read mode ...
		// you can receive such calls from any player.
		int k;
		log_int(k);
		if (k != iplayer)
			tw_error("Unexpected player index in share-controls");
		if (!player[k])
			tw_error("Receiving data from a disconnected player? Impossible!");
		log_short( player[k]->control->keys );

		//message.print(1500, 14, "player[%i] key[%i]", iplayer, player[iplayer]->control->keys);
	}

}



static char chat_buf[256];
static int chat_len = 0;
static int chat_on = 0;

MeleeData meleedata;

MeleeData::MeleeData()
{
	panelSprite             = NULL;
	kaboomSprite            = NULL;
	hotspotSprite           = NULL;
	sparkSprite             = NULL;
	asteroidExplosionSprite = NULL;
	asteroidSprite          = NULL;
	planetSprite            = NULL;
	xpl1Sprite              = NULL;

	planet_victory			= NULL;	

	melee = NULL;
}



void MeleeData::init()
{
	melee = load_datafile("melee.dat");
	if (!melee) tw_error("Error loading melee data\n");

	meleedata.panelSprite             = new SpaceSprite(&melee[MELEE_PANEL], PANEL_FRAMES, SpaceSprite::IRREGULAR);
	meleedata.kaboomSprite            = new SpaceSprite(&melee[MELEE_KABOOM], KABOOM_FRAMES,
		SpaceSprite::ALPHA | SpaceSprite::MASKED);// | SpaceSprite::MIPMAPED);
	meleedata.hotspotSprite           = new SpaceSprite(&melee[MELEE_HOTSPOT], HOTSPOT_FRAMES,
		SpaceSprite::ALPHA | SpaceSprite::MASKED);// | SpaceSprite::MIPMAPED);
	meleedata.sparkSprite             = new SpaceSprite(&melee[MELEE_SPARK], SPARK_FRAMES,
		SpaceSprite::ALPHA | SpaceSprite::MASKED /*| SpaceSprite::MIPMAPED*/ | SpaceSprite::MATCH_SCREEN_FORMAT);
	meleedata.asteroidExplosionSprite = new SpaceSprite(&melee[MELEE_ASTEROIDEXPLOSION], ASTEROIDEXPLOSION_FRAMES);
	meleedata.asteroidSprite          = new SpaceSprite(&melee[MELEE_ASTEROID], ASTEROID_FRAMES);
	meleedata.planetSprite            = new SpaceSprite(&melee[MELEE_PLANET], PLANET_FRAMES);
	meleedata.xpl1Sprite              = new SpaceSprite(&melee[MELEE_XPL1], XPL1_FRAMES,
		SpaceSprite::ALPHA | SpaceSprite::MASKED);// | SpaceSprite::MIPMAPED);

	planet_victory = (Music*) (melee[MELEE_PLANET+PLANET_FRAMES].dat);
}

void MeleeData::deinit()
{
	panelSprite             = NULL;
	kaboomSprite            = NULL;
	hotspotSprite           = NULL;
	sparkSprite             = NULL;
	asteroidExplosionSprite = NULL;
	asteroidSprite          = NULL;
	planetSprite            = NULL;
	xpl1Sprite              = NULL;

	planet_victory			= NULL;		

	unload_datafile(melee);
	
	melee = NULL;
}




int interpolate_frames = false;


#define HIST_POWER 4.0

#define CHECKSUM_CHANNEL _channel_buffered
//#define CHECKSUM_CHANNEL 0   THIS IS HIGHLY ILLEGAL IF YOU'd USE THIS ! Cause it's not also included in the buffer-filling!!

#ifdef _MSC_VER
#	pragma warning( disable : 4800 ) //int -> bool, performance warning
#endif


int num_games = 0;
char **game_names = NULL;
GameType **games = NULL;

GameType::GameType(const char *name, Game *(*new_game)(), double order) {
	this->name = strdup(name);
	this->_new_game = new_game;
	this->order = order;
	::num_games += 1;
	::game_names = (char **) realloc(::game_names, sizeof(char*) * (num_games+1));
	::games = (GameType**) realloc(::games, sizeof(GameType*) * (num_games+1));
	games[num_games-1] = this;
	games[num_games] = NULL;
	game_names[num_games-1] = this->name;
	game_names[num_games] = NULL;
}

Game *GameType::new_game() {STACKTRACE
	Game *tmp = _new_game();
	tmp->preinit();
	tmp->type = this;
	return tmp;
}

GameType *gametype (const char *name) {STACKTRACE
	GameType ** g;
	for (g = games; *g; g ++)
		if (!strcmp((*g)->name, name))
			return *g;
	return NULL;
}

void __checksync( const char *fname, int line) {
	if (!game) {
		error("request to compare checksums without a valid game\nfrom file %f, line %d", fname, line);
		return;
	}
//#	ifdef LOTS_OF_CHECKSUMS
//	game->compare_checksums();
//#	endif
}


const int channel_none = -1;
const int _channel_buffered = 1;
const int channel_init = 4;     // game type, version, length, etc.. things that need to read by a reader independant of a particular game type
const int channel_playback = 8; // used for demo playbacks only
const int channel_server = 12;  //data originating on the server

int channel_network[max_network];  //data originating on the client
// Note that you MUST take care, that player i maps to channel_network i on initialization!!
// (is needed by the channel_local() function)

int net_conn[max_network];		// the network connection index...
// determines that player [i] maps to connection with number net_conn[i] on this computer.

int num_hotseats[max_network];		// number of hotseat-players per connected computer
int channel_conn_recv[max_network];

int p_local;				// this defines the local player.
//int num_players = 0;		// the number of players in the game.
int num_network = 0;
int num_bots = 0, num_players = 0;


NPI *player[max_network];


//void set_numplayers(int n)
//{
//	num_players = n;
//}

int channel_local()
{
	// IMPORTANT:
	// you've to make sure that later on, that channel_network and player->channel are indeed the same.
	if (player[p_local])
		return player[p_local]->channel;
	else
		return channel_network[p_local];
}

void init_channels()
{
	p_local = -1;

	// default values for the available channels
	channel_network[0] = channel_server; //data originating on the server (=player 0)
	int i;
	for ( i = 1; i < max_network; ++i )
	{
		channel_network[i] = channel_network[i-1] + 4; //data originating on the client

		channel_conn_recv[i] = -1;
	}

	// actual number of connected computers that use these channels...
	num_network = 0;

	// initialize the log also, if possible
	if (glog)
	{
		glog->expand_logs(channel_network[max_network-1] + 4);
	} /* else
		tw_error("Warning:it's best to initialize log channels here !!"); */
};


void Game::_event(Event *e) {
	switch (e->type) {
		case Event::VIDEO: {
			if (game_done)
				return;
			if (e->subtype == VideoEvent::REDRAW)
				this->redraw();
		} break;
		case Event::TW_CONFIG: {
			ConfigEvent *c = (ConfigEvent*)e;
			switch (c->subtype) {
			case ConfigEvent::SET: {
					if (0) ;
					else if (!strcmp(c->name, "server.ini/game/shotrelativity")) {
						double v = atof(c->value);
						this->shot_relativity = v;
					}
					else if (!strcmp(c->name, "server.ini/game/friendlyfire")) {
						int v = atoi(c->value);
						this->friendly_fire = (bool) v;
					}
				} break;
			}
			issue_event(num_presences, (BaseClass**)presence, e);
			issue_event(num_items, (BaseClass**)item, e);
			//WTF?  Why do I have to cast these pointers?
		} break;
	}
}

void Game::add_focus(Presence *new_focus, int channel) {
	if ((channel != -1) && !glog->playback && !(glog->get_direction(channel) & Log::direction_write))
		return;
	num_focuses += 1;
	focus = (Presence **) realloc(focus, sizeof(Presence *) * num_focuses);
	focus[num_focuses - 1] = new_focus;
	new_focus->attributes |= ATTRIB_FOCUS;
	if (num_focuses == 1) new_focus->attributes |= ATTRIB_ACTIVE_FOCUS;
	if (focus_index == -1) focus_index = 0;
}

void Game::set_focus(Control *c)
{
	focus_index = -1;

	int i;
	for ( i = 0; i < num_focuses; ++i )
	{
		if (focus[i] == c)
			focus_index = i;
	}
}


void Game::prepare() {
#ifdef _MSC_VER
	_asm { finit }
#elif defined(__GCC__) && defined(__i386__)
	asm("finit");
#endif
	Physics::prepare();
	::game = this;
	::targets = &gametargets;
	return;
}

void Game::set_resolution(int screen_x, int screen_y) {
	int view_x, view_y;
	view_x = screen_x;
	view_y = screen_y;
	redraw();
	return;
}

void Game::redraw() {STACKTRACE
	if (!window->surface) return;
	scare_mouse();
	window->lock();
	rectfill(window->surface, window->x, window->y, window->x+window->w-1, window->y+window->h-1, pallete_color[8]);
	FULL_REDRAW += 1;
	view->refresh();
	view->animate_predict(this, 0);
	FULL_REDRAW -= 1;
	window->unlock();
	unscare_mouse();
	return;
}

Ship *Game::create_ship(const char *id, Control *c, Vector2 pos, double angle, int team) {STACKTRACE
	ShipType *type = shiptype(id);
	if (!type)
	{tw_error("Game::create_ship - bad ship id (%s)", id);}
	/*if(!ini) {
		sprintf(buffer, "ships/shp%s.ini", id);
		ini = buffer;
	}	
	log_file(buffer);*/
	set_config_file(type->file);
	if (team == 0) team = new_team();
	Ship *s = type->get_ship(pos, angle, get_code(new_ship(), team));
	if (c)
		c->select_ship(s, id);
	gametargets.add(s);
	s->attributes |= ATTRIB_NOTIFY_ON_DEATH;
	return s;
}

Ship *Game::create_ship(int channel, const char *id, const char *control, Vector2 pos, double angle, int team) {STACKTRACE
	Control *c = create_control(channel, control);
	if (!c)
	{tw_error("bad Control type!");}
	c->temporary = true;
	Ship *s = create_ship(id, c, pos, angle, team);
	return s;
}



/*
void Game::log_fleet(int channel, Fleet *fleet)
{
	STACKTRACE;

	int fl;
	char buffer[16384];

	channel_current = channel;

	if (log_writable())	// if channel_current is open for writing.
	{
		void *tmpdata = fleet->serialize(&fl);
		
		if (fl > 16000)	{tw_error("blah");}
		memcpy(buffer, tmpdata, fl);
		free(tmpdata);
	}

	log_int(fl);
	if (fl > 16000)	{tw_error("blah");}
	log_data(buffer, fl);

	fleet->deserialize(buffer, fl);
}
*/

Control *Game::create_control (int channel, const char *type, char *config, char *file) {STACKTRACE
	if ((channel != channel_none) && !is_local(channel)) {
		type = "VegetableBot";
		config = "Config0";
		file = "scp.ini";
	}

	Control *c = getController(type, "whatever", channel);
	if (!c) {
		tw_error("Game::create_control - bad control type (%s)", type);
		return c;
	}
	c->load(file, config);
	add(c);
	return c;
}

void Game::use_idle(int time)
{
	// you can put stuff here, which *has* to be done, even if the game is idle() (which
	// usually means, it's in some closed loop).
	return;
}


void Game::animate(Frame *frame) {_STACKTRACE("Game::animate(Frame*)")
	Physics::animate(frame);
}

void Game::animate() {_STACKTRACE("Game::animate(void)")

	double t = get_time2();
	paused_time = 0;
	int prediction_time = 0;
	if (interpolate_frames) {
		prediction_time += iround(frame_time + (get_time() - next_tic_time) * turbo);
		if (prediction_time > hiccup_margin) prediction_time = hiccup_margin;
	}
	if (lag_frames) {
		if (prediction_time > frame_time) prediction_time = frame_time;
		prediction_time += (prediction * lag_frames * frame_time) / 100;
	}
	if (prediction_time) view->animate_predict(this, prediction_time);
	else view->animate_predict(this, 0);
	t = get_time2() - t - paused_time;
	render_history->add_element(pow(t, HIST_POWER));

	return;
}

bool Game::game_ready() {STACKTRACE
	if (CHECKSUM_CHANNEL == 0) return 1;
	if (glog->playback) {
		return (glog->ready(channel_server + _channel_buffered) != 0);
	}
	else switch (glog->type) {
		case Log::log_normal: {
			return true;
		}
		break;
		case Log::log_net: {
			int i;
			bool result = true;
			for ( i = 0; i < num_network; ++i )
			{
				if (player[i])
				{
					if (!glog->ready(player[i]->channel + _channel_buffered))
						result = false;
				}
			}
			//if (!log->ready(channel_server + Game::_channel_buffered)) return false;
			return result;	// if one or more channels aren't ready yet, it returns false.
		}
	}
	return true;
}

/*
void Game::handle_desynch(int local_checksum, int server_checksum, int client_checksum) {STACKTRACE
	error("Game Desynchronized\nTime=%d Frame=%d\nClient=%d Server=%d Local=%d", game_time, frame_number, (int)client_checksum, (int)server_checksum, (int)local_checksum);
}
*/

//static int old_num_items;
//static int old_rng;
//static int old_frame;
//static char old_checksum_buf[200][200];


//if a game is killed due to an error, this may be executed
void handle_game_error ( Game *game )
{
	log_debug("handle_game_error() executed\n");
	if (glog) {
		glog->save("error.dmo");
		log_debug("Demo recording saved to error.dmo\n");
	}
/*
	FILE *f;

	f = fopen("error.log", "a");
	if (!f) return;

	if (exitmessage)
		fprintf(f, "\n\n-------- error report, with message [%s]\n", exitmessage);
	else
		fprintf(f, "\n\n-------- error report, unknown error\n");
	//fprintf(f, "-------- showing in-game objects --------\n");
	fprintf(f, "timewarp version = %s\n", tw_version());

	if (game)
		fprintf(f, "lag_frames = %i", game->lag_frames);
	else
		fprintf(f, "no game defined");

	time_t t;
	tm *td;
	t = ::time(0);
	td = ::localtime(&t);
	// month: 0=januari
	fprintf(f, "local time = %i-%02i-%02i %02i:%02i\n\n", td->tm_mday, td->tm_mon+1, 1900+td->tm_year,
		td->tm_hour, td->tm_min);

	if (physics)
	{
		fprintf(f, "name, pos(x,y), vel(x,y), state, obj-pointer(this), ship-pointer(ship), target pointer(target)\n\n");
		
		int i;
		for (i = 0; i < physics->num_items; i += 1)
		{
			SpaceLocation *s;
			s = physics->item[i];
			
			if (!(s && s->exists() && s->detectable()))
				continue;
			
			int is = s->state;
			Vector2 p = s->pos;
			Vector2 v = s->vel;
			
			// set "enable run-type information" for this feature
			// (rebuild all after changing that option)
			fprintf(f, "%30s %11.3e %011.3e %11.3e %11.3e %3i 0x%08X 0x%08X 0x%08X\n",
				typeid(*s).name(), p.x, p.y, v.x, v.y, is, (unsigned int)s, (unsigned int)s->ship, (unsigned int)s->target );
		}
	} else {
		fprintf(f, "No physics defined\n");
	}

	#ifdef DO_STACKTRACE
	//char *s = tw_error_str;
//	fprintf(f, "%s\n", tw_error_str);
	//free(s);	// cause s was allocated with malloc().

	fprintf(f, "\nPROCLIST: level of call, line number in file, file name (top=most recent call)\n\n");

	const char *fname = 0;
	int *linenum = 0, *level = 0;

	int i;
	i = 0;	// start with the most recent one.
	while (	get_stacklist_info(i, &fname, &linenum, &level) )
	{
		++i;	// go on until it's a full circle
		if (fname && linenum && level)
			fprintf(f, "%2i   %4i  %s\n", *level, *linenum, fname);
	}

	fprintf(f, "\nPROCSTACK: level of call, line number in file, file name (top=most recent call)\n\n");

	// also, read the "other" stack info ...
	i = 0;
	while ( get_stacktrace_info( i, &fname, &linenum, &level) )
	{
		++i;
		if (fname && linenum && level)
			fprintf(f, "%2i   %4i  %s\n", *level, *linenum, fname);
	}

	#endif

	fprintf(f, "----------------- end error log ---------------------\n");

	fclose(f);
	*/
}



void Game::event_share_desynch(int iplayer)
{
	STACKTRACE;

	if (!log_synched)
	{
		if (!is_local(channel_current))
		{
			tw_error("Sending desynch check data from a non-local channel.");
		}

		// sending data

		// initialize your local data
		int Nlocal;
		double xlocal[max_synch_check], ylocal[max_synch_check];

		int i;
		Nlocal = num_items;
		if (Nlocal > max_synch_check)
			Nlocal = max_synch_check;
		
		for ( i = 0; i < Nlocal; ++i )
		{
			xlocal[i] = item[i]->pos.x;
			ylocal[i] = item[i]->pos.y;
		}

		// send (also to yourself)
		log_int(Nlocal);
		log_data(xlocal, Nlocal*sizeof(double));
		log_data(ylocal, Nlocal*sizeof(double));
	}

	if (log_synched)
	{
		// receiving data

		if (!player[iplayer])
			tw_error("Trying to share synched data with non-existing player");
		// receive.
		int N;

		// receive the number of elements
		log_int(N);

		if (N < 0 || N > max_synch_check)
			tw_error("Number of received elements for desynch check exceeds bounds");

		// receive that positional data
		log_data(player[iplayer]->desynch_xpos, N*sizeof(double));
		log_data(player[iplayer]->desynch_ypos, N*sizeof(double));

		player[iplayer]->desynch_Nitems = N;

		player[iplayer]->desynch_received = true;
	}
}



void Game::check_desynch()
{
	STACKTRACE;

	// all the players have shared their synch-data by CALLs.
	// now, it's time to compare them to the local player.

	// we're using events, so it's not guaranteed that you receive this each iteration.
	if (!player[p_local]->desynch_received)
		return;

	
	bool error = false;

	int p;
	for ( p = 0; p < num_network; ++p )	// note, 0==server.
	{
		if (!player[p])
			continue;

		// we're using events, so it's not guaranteed that you receive this each iteration.
		if (!player[p]->desynch_received)
			continue;

		//message.print(1500, 15, "Desynch test: game_time = %i player %i", game_time, p);
	
		// reset the receive detector.
		player[p]->desynch_received = false;

		//message.print(1500, 15, "Desynch test of player[%i], %i elements", p, player[p]->desynch_Nitems);
		//message.animate(view->frame);


		// check if the data differ from the local data:
		if (player[p]->desynch_Nitems != player[p_local]->desynch_Nitems)
		{
			message.print(1500, 15, "Desynch test: different number of items!");
			message.animate(view->frame);
			error = true;
		} else {
			
			// compare all the positions
			int i;
			for ( i = 0; i < player[p_local]->desynch_Nitems; ++i )
			{
				if (player[p]->desynch_xpos[i] != player[p_local]->desynch_xpos[i] ||
					player[p]->desynch_ypos[i] != player[p_local]->desynch_ypos[i])
				{
					error = true;
					if (i < num_items)
					{
						message.print(1500, 15, "Desynch (probably in item of type %s)", item[i]->get_identity() );
						message.animate(view->frame);
					}
					// note that, because of lag, this estimate could be wrong ... but it's 
					// a bit excessive if you also pass the item-id's over the network.
					
					//tw_error("Desynch in item %s", item[i]->get_identity() );
				}
			}
		}
	}

	/*
	if (error)
	{
		//message.print(1500, 15, "game_time = %i", game_time);
		message.animate(0);
//		readkey();
	}
	*/

}


/*
void Game::do_game_events() {_STACKTRACE("Game::do_game_events()")

	int i, p;
	for ( p = 0; p < num_network; ++p )	// note, 0==server.
	{
		if ((glog->get_direction(channel_network[p] + _channel_buffered) & Log::direction_write) != 0)
		{
			COMPILE_TIME_ASSERT(sizeof(events_waiting) == sizeof(char));

			// you _always_ transmit the # of events (can be null)
			glog->buffer( channel_network[p] + _channel_buffered, &events_waiting, sizeof(events_waiting) );

			for (i = 0; i < events_waiting; i += 1)
			{
				glog->buffer ( channel_network[p] + _channel_buffered, waiting_events[i], waiting_events[i]->size );
			}
			//deallocate transmitted events
			for (i = 0; i < events_waiting; i += 1)
				free(waiting_events[i]);

			events_waiting = 0;
		}
	}

	//double-check transmission
	if (events_waiting) {
		tw_error("Game::do_game_events - events weren't sent properly");
		for (i = 0; i < events_waiting; i += 1) free(waiting_events[i]);
		events_waiting = 0;
	}

	//receive
	char ne;
	COMPILE_TIME_ASSERT(sizeof(events_waiting) == sizeof(ne));
	char buffy[1024];

	for ( p = 0; p < num_network; ++p )	// note, 0==server.
	{
		// you _always_ read the # of events.
		glog->unbuffer(channel_network[p] + _channel_buffered, &ne, sizeof(ne));

		for (i = 0; i < ne; i += 1) {
			char *tmp = buffy;
			//glog->unbuffer(channel_network[p] + _channel_buffered, &buffy, sizeof(GameEvent));
			glog->unbuffer(channel_network[p] + _channel_buffered, buffy, sizeof(GameEvent));
			int s = ((GameEvent*)tmp)->size;
			if (s > 1024) {
				tmp = (char *)malloc(s);
				memcpy(tmp, buffy, sizeof(GameEvent));
			}
			glog->unbuffer(channel_network[p] + _channel_buffered, tmp + sizeof(GameEvent), s - sizeof(GameEvent));
			handle_game_event ( channel_network[p], ((GameEvent*)tmp));
			if (tmp != buffy) free(tmp);
		}
	}
}

void Game::handle_game_event ( int source, class GameEvent *event ) {STACKTRACE
	if ((event->type <= event_invalid) || (event->type >= event_last)) {
		tw_error("Game::handle_game_event - Bad event type: %d", event->type);
	}
	switch (event->type) {
		case event_change_lag: ((GameEventChangeLag*)event)->execute(source);
		break;
		case event_message: ((GameEventMessage*)event)->execute(source);
		break;
	}
}

void Game::send_game_event ( class GameEvent *event ) {STACKTRACE
	if (events_waiting == maximum_events_waiting) {
		tw_error("too many GameEvents");
		return;
	}
	if (!waiting_events) waiting_events = new GameEvent*[maximum_events_waiting];
	waiting_events[events_waiting] = event;
	events_waiting += 1;
}
*/

void Game::log_file (const char *fname)
{
	glog->log_file(fname);
}


void Game::net_expect(int val)
{
	if (!detailed_network_check)
		return;

	int k;
	int p;
	bool result = true;
	for ( p = 0; p < num_network; ++p )
	{
		if (!player[p])
			continue;

		k = val;

		log_int(k, player[p]->channel + _channel_buffered);

		if (k != val)
		{
			result = false;
			message.print(1500, 14, "frame[%i] time[%i]", frame_number, game_time);
			message.animate(0);
			tw_error("error in buffered data");
		}
	}

	if (!result)
	{
		message.print(1500, 14, "frame[%i] time[%i]", frame_number, game_time);
		message.animate(0);
		tw_error("error in buffered data");
	}
}

void Game::calculate() {_STACKTRACE("Game::calculate")
	int i;
	double t = get_time2();
	int active_focus_destroyed = false;


	paused_time = 0;


	for (i = 0; i < num_focuses; i += 1) {
		if (!focus[i]->exists()) {
			num_focuses -= 1;
			if (focus_index == i) {
				focus[i]->attributes &= ~ATTRIB_ACTIVE_FOCUS;
				active_focus_destroyed = 1;
				focus_index -= 1;
				if (num_focuses && (focus_index < 0))
					focus_index += 1;
			}
			focus[i] = focus[num_focuses];
			i -= 1;
		}
	}


	if (active_focus_destroyed && (focus_index >= 0))
		focus[focus_index]->attributes |= ATTRIB_ACTIVE_FOCUS;



	Physics::calculate();

	gametargets.calculate();

	view->calculate(this);


	t = get_time2() - t - paused_time;
	tic_history->add_element(pow(t, HIST_POWER));
	return;
}

static int tot_idle_time = 50;


void Game::network_share_keys()
{


	int i;
	for ( i = 0; i < num_network; ++i )
	{
		if (!player[i])
			continue;

		if (player[i]->islocal())
		{
			// share your key presses with other people.
			player[i]->control->keys = player[i]->control->think();

			if (player[i]->control->keys != 0)
			{	// this will transmit the key
				// but you only need to do that if there's anything to send (cause 0 is the default value,
				// see below, and you don't have to send a default value).
				// and you only have to send you own (local) keys. You receive other peoples keys.
				//use_player_index = i;
				CALL(event_sharecontrols, i);
			}
		}

		// default value (=predictable) for all players (!) until it's re-read from the network channel by events.handle ->
		// -> that's why you've to call this between game.calculate and events.handle()
		player[i]->control->keys = 0;
	}

	/*
	// and the AI ? the AI is always predictable.
	// therefore, no need to share the AI ...
	for ( i = num_network; i < num_players; ++i )
	{
		player[i]->control->keys = player[i]->control->think();
	}
	// end of handling I/O
	*/
}



void Game::network_crc_check()
{
	// share (and test) desynch information
	int p;
	for ( p = 0 ; p < num_network; ++p )
	{
		if (!player[p])
			continue;

		// sharing
		if (player[p] && player[p]->islocal())
		{
			CALL(share_crc, p);
		}
		
		// checking the results of a previous share.
		if (p > 0)
		{
			if (player[p] && player[p-1] && player[p]->crc != player[p-1]->crc)
			{
				tw_error("The game is desynched.");
			}
		}
	}
}

void Game::init_lag()
{
	// synch all players here, to skip delays due to loading files and such.
	// do this 2 times?
	int syncpoint[max_network];
	int i;
	for ( i = 0; i < num_network; ++i )
	{
		syncpoint[i] = i;
		share(i, &syncpoint[i]);
	}
	share_update();
	// ok, they're all synched now, now start timing
	int t1 = get_time();
	for ( i = 0; i < num_network; ++i )
	{
		syncpoint[i] = i;
		share(i, &syncpoint[i]);
	}
	share_update();
	int t2 = get_time();

	// I suppose this means, that the player with slowest connection, will have the best play,
	// while others simulate that slow connection by adding extra lag. This extra lag is simulated
	// locally hence it's bothersome for him; for the slow player it's externally so he'll have reasonable
	// play locally (profiting from fast incoming data).

	// it's probably also possible to distribute lag otherwise: eg, by setting lag=1 for the host,
	// and letting everyone else compensate for that. But, well, I don't do that.

	int maxlag = t2 - t1;

	// copy the host-setting about the lag...
	share(-1, &global_lag_synch);
	// and for doing a desynch test...
	share(-1, &do_desynch_test);
	
	share_update();

	if (global_lag_synch)
	{
		// in this case, the lag should be synchronized on each computer. This is for "fair play"
		for ( i = 0; i < num_network; ++i )
		{
			if ( i == p_local )
				syncpoint[i] = t2 - t1;

			share(i, &syncpoint[i]);
		}
		share_update();

		// now, syncpoint[i] is the "lag" of that player. Find the biggest lag.
		for ( i = 0; i < num_network; ++i )
		{
			if (syncpoint[i] > maxlag)
				maxlag = syncpoint[i];
		}

		// distribute the lag evenly between sender and receiver:
		// uhm... no, because actually, this is the one-way lag: because the local player has zero-lag.
		//maxlag /= 2;	so, disable this here.

		share(-1, &maxlag);
		share_update();

		// the maxlag value is overwritten by the host.
	}


	// we need to set lag ... the worst there is, perhaps ?
	lag_buffer = (maxlag / frame_time) + 1;
	// +1, because it is (usually) rounded down, while we would like a small margin of error.
	
	if (lag_buffer <= 1)
		lag_buffer = 1;	// well, you do need to start...

	if (lag_buffer > 10)
		lag_buffer = 10;

	message.print(1500, 15, "lag set to [%i] frames", lag_buffer);
}

void Game::event_lag_increase(int iplayer)
{
	if (log_synched && (is_local(channel_current) || global_lag_synch) )
	{
		// you only do this locally... you ignore the request from other players,
		// unless global-synch of lag is enabled.
		lag_increase += 1;
		message.print(1500, 15, "lag increase to [%i] frames or [%i] ms", lag_buffer + lag_increase, 
			(lag_buffer + lag_increase) * frame_time);
	}
}

void Game::event_lag_decrease(int iplayer)
{
	if (log_synched && (is_local(channel_current) || global_lag_synch))
	{
		if (lag_buffer-lag_decrease > 1)
		{
			// you only do this locally... you ignore the request from other players,
			// unless global-synch of lag is enabled.
			lag_decrease += 1;
			message.print(1500, 15, "lag decrease to [%i] frames or [%i] ms", lag_buffer - lag_decrease,
				(lag_buffer - lag_decrease) * frame_time);
		}
	}
}


void Game::test_startit()
{
	// first, check if the buffer is empty:
	int i;
	for ( i = 0 ; i < num_network; ++i )
	{
		int buf;
		for ( buf = 0; buf < 2; ++buf )
		{
			int ch = player[i]->channel + buf;
			int N = glog->log_len[ch];
			int k = glog->log_pos[ch];
			if (k != N)
				tw_error("Buffer has unread data");
		}
	}

	lag_buffer = 1;
	for (lag_buffer = 1; lag_buffer < 5; ++ lag_buffer)
	{
		for ( i = 0; i < lag_buffer; ++i )
		{
			// signal that you're ready to start the first game iteration.
			int p;
			for ( p = 0 ; p < num_network; ++p )
			{
				if (player[p]->islocal())
				{
					CALL(start_iteration, p);
				}
			}
		}
		
		int p;
		for ( p = 0 ; p < num_network; ++p )
		{
			int buf;
			for ( buf = 0; buf < 2; ++buf )
			{
				int ch = player[p]->channel + buf;
				int N = glog->log_len[ch];
				int k = glog->log_pos[ch];
				if ( (buf == 1 && N - k != 8*lag_buffer) ||	// buffered channel should've 1 int call-id and 1 int content
					(buf == 0 && k != N) )				// direct channel shouldn't change
					tw_error("Buffer has inadequate amount of data");
			}
		}

		
		
		for ( i = 0; i < lag_buffer; ++i )
		{
			events.set_wait();
	
			int t = get_time();
			for (;;)
			{
				NetLog *l = (NetLog*) glog;
				if (l->type == Log::log_net)
				{
					l->flush_noblock();
					//glog->listen();
					l->recv_noblock();		// receive stuff, if you can
				}
				events.handle();	// this will read events. You send events during the game.
				
				if (events.all_ready())
					break;
				
			}
		}
	}
}



static const int max_time_checks = 20;
static int dt = frame_time;
static int N_time_span[max_time_checks];
static int last_time = 0;

void Game::iteration_histogram(int time)
{

	int i = (time - last_time) / dt;
	last_time = time;

	if (i < 0)
		i = 0;
	if (i > max_time_checks-1)
		i = max_time_checks-1;

	N_time_span[i] += 1;
}

void Game::iteration_histogram_init(int time)
{
	last_time = time;

	int i;
	for ( i = 0; i < max_time_checks; ++i )
	{
		N_time_span[i] = 0;
	}

	dt = frame_time;
}

void Game::iteration_histogram_writelog()
{
	int i;
	for ( i = 0; i < max_time_checks; ++i )
	{
		log_debug("delay = %5i to %5ims  N = %6i\n",
			i * dt, (i+1)* dt, N_time_span[i]);
	}
}


static int base_delay = 0;	// for debug (lag-test) purpose only

// toggle control
static bool toggle_key_inc_lag = true;
static bool toggle_key_dec_lag = true;

void Game::play()
{
	_STACKTRACE("Game::play");

	set_resolution(window->w, window->h);
	prepare();
	if (is_paused()) unpause();

	//test_startit();	//only test this for single-player game

	// this sets the lag_buffer.
	init_lag();


	int i;
	for ( i = 0; i < lag_buffer; ++i )
	{
		int p;
		for ( p = 0 ; p < num_network; ++p )
		{
			if (player[p]->islocal())
			{
				// signal that you're ready to start the first game iteration.
				CALL(start_iteration, p);
			}
		}

	}


	// can be overridden by events.
	lag_increase = 0;
	lag_decrease = 0;

	// tracker about home many extra physics iterations are inserted; this shouldn't get too high
	int num_catchups = 0;
	const int max_catchups = 3;

	int time_start = get_time();

	iteration_histogram_init(time_start);


	// if you increase lag, iterations can be skipped...
	skip_iteration = false;

	try {
		while(!game_done)
		{

			unsigned int time;
			

			poll_input();
			videosystem.poll_redraw();
			if(!sound.is_music_playing()) {
				play_music();
			}


			// physics can  catch up with graphics.
			bool catching_up;
			catching_up = false;

			// check if something interesting happens in this iteration (false), otherwise (true)
			// issue an idle time.
			bool idle_iteration = true;

			time = get_time();
			// this is used to time animation and physics


#ifdef _DEBUG
			// if you press space, this will block a "send" on this machine; thus, you can simulate
			// the effect of lag, even on a LAN network.
			debug_net_block = false;
			if (key[KEY_SPACE] != 0)
			{
				++base_delay;
				if (base_delay >= 50)
					base_delay = 0;

				if (base_delay != 0)//(rand()%50) != 0)
					debug_net_block = true;
			}
#endif

			// note, you go to the next calculation either because some required time has
			// passed, or because you need to catch up with some kind of lag...

			if (time >= next_tic_time)//geo:physics should be independent of animation && (next_render_time > game_time)) &&
				/*(game_ready() || game_time == 0)*/ {		// note that game_time==0 is also needed, cause otherwise it'll wait for data, while no data've been generated yet.
				_STACKTRACE("Game::play - Game physics");



				// wait till you receive signal that you can proceed to the next iteration.
				// ALL COMMUNICATION EXCEPT FOR DESYNCH TEST SHOULD GO THROUGH HERE.
				// also see the CALL and EVENT macros
					
				if (events.all_ready())
				{
					// only reset the call-state, if previously, all calls have been handled.
					events.set_wait();
				}
			
				
				int t = get_time();

				NetLog *l = (NetLog*) glog;
				if (l->type == Log::log_net)
				{
					l->flush_noblock();
					
					l->recv_noblock();		// receive stuff, if you can
					
				}
				events.handle();	// this will read events. You send events during the game.
				// if all "start-iterations" are set, then they're all_ready.
				
				// Note, that the IF statement ensures, that you do not block the game by waiting
				// for (networked) input. This means, that graphics won't be blocked: otherwise,
				// graphics of this user can be delayed by graphics of another user, resulting in
				// a pretty slow framerate.

				if (events.all_ready())
				{
					idle_iteration = false;
					
					
					iteration_histogram(time);	//ok, another iteration; update the histogram!

					// check if a player was asked to be removed here..
					int i;
					for ( i = 0; i < num_network; ++i )
					{
						if (!player[i])
							continue;
						
						if (!player[i]->status)
						{
							if ( i == p_local)
							{
								// send remaining data before disconnecting.
								NetLog *l = (NetLog*) glog;
								if (l->type == Log::log_net)
								{
									l->flush_block();
								}
							}
							
							player[i]->control->keys = 0;	// leave with clean default keys.
							remove_player(i);
							// this will set player[i]=0
						}
					}
					
					// test if there was an event that causes the game to quit.
					if (game_done)
					{
						iteration_histogram_writelog();
						return;
					}
					//	}
					// note that, to reduce lag, you keep reading data from the buffer without
					// adding extra stuff (through CALL), so that it becomes smaller.
					// BUT: that skips keys, and this leads to a desynch SO THIS IS NOT A GOOD WAY TO DO IT !!
					
					
					int t_execute = get_time();

					// calculate game stuff
					// this increases game_time.
					calculate();

					// this measures how much time it takes to execute one physics iteration
					t_execute = get_time() - t_execute;
					
					
					
					// check if the network is in synch?
					
					// enable this if you want to check for desynches
					// but, it only makes sense (in current implementation), if the two games are synched.
					if (global_lag_synch && do_desynch_test)
					{
						// send desynch data for local player(s)
						int p;
						for ( p = 0 ; p < num_network; ++p )
						{
							if (player[p] && player[p]->islocal())
							{
								CALL(event_share_desynch, p);
							}
						}
						
						// check desynch data of the players in the current (lagged) frame.
						check_desynch();
					}
					
					// it's best to do this only in debug mode...
					

					// handle some important buttons...
					// must be done before the keys are re-set to 0 by the share_key
					if (player[p_local] && !skip_iteration )
					{

						if ( (player[p_local]->control->keys & keyflag::inc_lag) != 0)
						{

							if (toggle_key_inc_lag)
							{
								CALL(event_lag_increase, p_local);
								toggle_key_inc_lag = false;
							}
						} else {
							toggle_key_inc_lag = true;
						}
						
						if ( (player[p_local]->control->keys & keyflag::dec_lag) != 0)
						{

							if (toggle_key_dec_lag && lag_buffer > 1)
							{
								CALL(event_lag_decrease, p_local);
								toggle_key_dec_lag = false;
							}
						} else {
							toggle_key_dec_lag = true;
						}
					}
					
					// this can also do some game stuff ... (namely, Esc = quit)
					while (keypressed())
						handle_key(readkey());
					
					// actually ... duh ... it's best to call this AFTER the events-handle, cause then the data
					// have some time to be sent across the network, while the game is doing its calculations.
					// namely, then it's using the idle() time that's required or so...
					
					
					network_share_keys();
					
					// after it's used, it can be re-set.
					skip_iteration = false;

					// crc checking is now only available, if all the games involved run synchronized,
					// this is also a (much simpler) desynch test.
					if (global_lag_synch)
						network_crc_check();


					// approve of the next iteration, after ALL possible event thingies are done.
					if (lag_decrease == 0)
					{
						int p;
						for ( p = 0 ; p < num_network; ++p )
						{
							if (player[p] && player[p]->islocal())
							{
								CALL(start_iteration, p);
							}
						}
					} else {
						--lag_decrease;	// skip new-iteration calls.
						--lag_buffer;
					}
					
					// and increasing lag means, adding extra, empty, iterations...
					int ilag;
					for ( ilag = 0; ilag < lag_increase; ++ilag )
					{
						int p;
						for ( p = 0 ; p < num_network; ++p )
						{
							if (player[p] && player[p]->islocal())
							{
								// notify within this iteration
								// that this iteration is "empty", i.e., no control-info and such
								CALL(notify_skip_iteration, p);

								CALL(start_iteration, p);

							}
						}
						++lag_buffer;
						--lag_increase;
					}
					
					if (auto_unload)
						unload_unused_ship_data();
				
					
					
					
					//if (key[KEY_F4])
					//	turbo = f4_turbo;
					//else
					//	turbo = normal_turbo;

					// ------------ GAME TIME MANAGEMENT ------------

					// this hiccup-margin allows physics to "catch up" a few iterations that are
					// skipped by graphics.
					int time_current = get_time();



					double factor = 0.25;
					int frame_time_length = frame_time / turbo;
					if (t_execute < factor*frame_time_length)
					{
						// in this case, you can try to have physics in a linear fashion

						if ((hiccup_margin >= 0)
							&& (num_catchups < max_catchups)		// prevent too many catchups (that will delay graphics update too much)
							&& (next_tic_time + hiccup_margin > time_current + frame_time_length))
						{

							
							if (next_tic_time + frame_time_length < time_current)
							{
								catching_up = true;
								next_tic_time += frame_time_length;
								// perhaps this is better... cause you're below "time" anyway...
								// so you will be catching up anyway... in the loop that follows
								// An immediate catchup will happen, because next_tic_time < time, which
								// is the only constraint to enter this physics calculation immediately
								// again, thus "accelerating" the physics updates, thus making up for
								// time lost in animation or during network delays.

							} else {
								catching_up = false;
								
								// trying to execute frames in a linear fashion.
								next_tic_time += frame_time_length;	// but if there are >1 catchups, then hurry up!!
							}
							
						} else {
							catching_up = false;
							next_tic_time = time_current + frame_time_length;	// execute as often as possible.
						}
					} else {
						// otherwise, the game calculations take so long, that trying to catch up
						// is nigh impossible.
						catching_up = false;
						next_tic_time += frame_time_length;

						if (next_tic_time < time_current)
							next_tic_time = time_current + frame_time_length;	// execute as often as possible.
					}

					if (catching_up)
						++num_catchups;
					else
						num_catchups = 0;

					
					if (next_fps_time <= game_time)
					{
						next_fps_time += msecs_per_fps;
						fps();
					}					
					
					
				}

				
			}
				
				
			
			// ALWAYS send/recv at end.
			// this is the best place for this, in FRONT of the animation, cause
			// it's best to send data BEFORE an animation starts...
			
			// if you don't have to be careful about your connection speed.
			if (glog->type == Log::log_net)
			{
				NetLog *l = (NetLog*) glog;
				l->recv_noblock();		// receive stuff, if you can
				
				// this helps to reduce idle-time, cause it doesn't have to wait till
				// data are received first (namely that's what game_ready() tests).
				l->flush_noblock();			// this sends, if there's something to send at least
			}
			

				
			// geo: instead of game_time, I use time, so that physics and animation are competely
			// independent; then, if physics has to wait for networked data, animation can continue
			// regardless of that.
			// well... except if physics tries to catch up a few frames of time. But in that case,
			// in networked games, it can take a while to synch back again;
			// so there must be an upper limit to the animation frame rate !!
			time = get_time();
			if (!catching_up || time >= next_render_time - msecs_per_render + msecs_per_render_max)
			{
				if (/*interpolate_frames ||*/ (time >= next_render_time))
				{
					_STACKTRACE("Game::play - Game rendering")
						
					idle_iteration = false;

					animate();

					#ifdef _DEBUG
					//idle(debug_idle_time_animate);
					#endif


					next_render_time += msecs_per_render;

					time = get_time();
					// actually, just don't bother about linear timing animations... because there is
					// a rather bad effect on game lag if too much time is allocated for animation.
					next_render_time = time + msecs_per_render;
				}
			}


			if (idle_iteration)
			{
				// if nothing interesting happened in this iteration, then insert some idle time
				int n = 1;
				idle(n);
				tot_idle_time += n;
			}

			

				

		}

	}
	catch (int i) {
		if (i == -1) throw;
		if (__error_flag & 1) throw;
		handle_game_error(this);
		if (i != 0) {
			caught_error ("%s %s caught int %d", __FILE__, __LINE__, i);
		}
		if (__error_flag & 1) throw;
	}
	catch (const char *str) {
		if (__error_flag & 1) throw;
		handle_game_error(this);
		caught_error("message: \"%s\"", str);
		if (__error_flag & 1) throw;
	}
//ArrayIndexOutOfBounds NullPointerException
	catch (exception &e) {
		if (__error_flag & 1) throw;
		handle_game_error(this);
		caught_error ("A standard exception occured\n%s", e.what());
		if (__error_flag & 1) throw;
		}
	catch (...) {
		if (__error_flag & 1) throw;
		handle_game_error(this);
		caught_error("Ack(1)!!!\nAn error occured in the game!\nBut I don't know what error (check error log)!");
		if (__error_flag & 1) throw;
	}
	return;
}


void Game::ship_died(Ship *who, SpaceLocation *source)
{
	STACKTRACE;

	if (source && source->data) {
		Music *tmp = NULL;
		//if (source && source->ship && source->ship->data) tmp = source->ship->data->moduleVictory;
		// note: it's not guaranteed that a ship exists longer than its weapon, while data keep existing, right ?
		//if (source && source->exists())
		// better NOT check for "exists", cause usually weapons die on impact. The fact that they're
		// used by physics is enough guarantee that they are alive...
		if (source)
			tmp = source->data->moduleVictory;
		if (tmp) sound.play_music(tmp);
	}
	return;
}


void Game::object_died(SpaceObject *who, SpaceLocation *source)
{
	if (who && who->isShip())
	{
		ship_died((Ship*)who, source);
	}
}

#include "../util/profile2.h"
void Game::fps()
{
	STACKTRACE;

	if (this->show_fps) {
		if ((!glog->playback) && (glog->type == Log::log_net))
		{
			char tmp[512];
			int conn;
			strcpy(tmp, "ping: ");
			for ( conn = 0; conn < ((NetLog*)glog)->num_connections; ++conn )
			{
				int ping = ((NetLog*)glog)->ping[conn];
				int k =strlen(tmp);
				sprintf(&tmp[k], " [%i] ", ping);
			}
			//		char *tt = "good";
			//		if (ping > 100) tt = "okay";
			//		if (ping > 200) tt = "bad";
			//		if (ping > 400) tt = "BAD!";
			//		if (ping > 800) tt = "VERY BAD!";
			message.print((int)msecs_per_fps, 12, tmp);
		}
     
/*			double a = 1.0;
			double b = 1.0;
//			double *c = (double *)(((int)&a - (int)&b) & 0x80000000);
//			message.print(1000, 15, "inf = %f", a + *c);
				{
				SpaceLocation *frog = getFirstItem(LAYER_CBODIES);
//				const type_info *d = &typeid(*frog);
				message.print(msecs_per_fps, 15, "%s %d %d %d", 
						d->name(), sizeof(*frog), sizeof(Asteroid),
						0);
				}*/
		double tt = pow(tic_history->get_average(0, 1000/frame_time), 1/HIST_POWER);
		double rt = pow(render_history->get_average(0, 1000/frame_time), 1/HIST_POWER);
		char *tmp;

		if (tt*8 < frame_time)
			tmp = "good";
		else if (tt*2 < frame_time)
			tmp = "ok";
		else if (tt < frame_time)
			tmp = "bad";
		else
			tmp = "BAD!";
		message.print((int)msecs_per_fps, 12, "tic time: %.3fms (that's %s)", tt, tmp);

		if (rt < 2)
			tmp = "good";
		else if (rt < 20)
			tmp = "ok";
		else if (rt < 50)
			tmp = "bad";
		else
			tmp = "BAD!";
		message.print((int)msecs_per_fps, 12, "render time: %.3fms (that's %s)", rt, tmp);
		message.print((int)msecs_per_fps, 12, "debug: %d", debug_value);
		message.print((int)msecs_per_fps, 12, "shipdatas: [%i] items: [%i] deadites: [%i]  collisions: [%i]  quadrants: [%i]",
			shipdatas_loaded, num_items, num_dead_presences, num_collisions, num_quadrant);
	}

	if (chat_on)
		message.print((int)msecs_per_fps, 15, "say: %s", chat_buf);
}

void Game::preinit()
{
	STACKTRACE;

	Physics::preinit();
//	meleedata.planetSprite = meleedata.asteroidSprite = meleedata.asteroidExplosionSprite = meleedata.hotspotSprite = meleedata.kaboomSprite = meleedata.panelSprite = meleedata.sparkSprite = meleedata.xpl1Sprite = NULL;
// you should reset it here (again), cause there can be subgames of this type.
//	planet_victory = NULL;
	tic_history = render_history = NULL;

	events_waiting = 0;
	waiting_events = NULL;
	num_focuses = 0;
	focus_index = 0;
	focus = NULL;
//	num_targets = 0;
//	target = NULL;
	gametargets.reset();
	view = NULL;
	window = NULL;
	music = NULL;
}


void Game::init(Log *_log)
{
	_STACKTRACE("Game::init");

	//display_channel_info("game::init");

	register_events();

	game_done = false;

	if (!glog) {
		Log *log = new Log();
		log->init();
		set_global(log);
	}

	// go to default mode...
	log_resetmode();

	lag_frames = 0;
	show_fps = 0;
	game_time = 0;
	frame_time = 1;
	frame_number = 0;
	hiccup_margin = 100;
	next_tic_time = get_time();
	next_render_time = game_time;
	next_fps_time = game_time;
	view_locked = false;
	physics_locked = false;
	if (glog->type != Log::log_normal || glog->playback) physics_locked = true;
	local_checksum = client_checksum = server_checksum = 0;

	Physics::init();
	prepare();

	if (!window) {
		window = new VideoWindow();
		window->preinit();
	}

	set_config_file("client.ini");
	change_view(get_config_string("View", "View", "Hero"));

//	focus_index = p_local;


	window->add_callback(this);

	if (!glog->playback) {
		switch (glog->type) {
			case Log::log_normal: {
			}
			break;
			case Log::log_net: {
			}
			break;
			default: {
				tw_error("Knee!");
			}
			break;
		}
	}

/* CONTENTS OF CHANNEL channel_init :

offset	size	format		data
0		4		int			log type number
4		4		int			size of game type name
8		?		char[]		game type name
?		4		int			lag frames

*/



	//display_channel_info("game - log_int");

//	int tmp = glog->type;
//	log_int(tmp, channel_init);	//xxx crash
//	if (glog->playback) glog->type = tmp;

	log_debug("game::init rand\n");

	//display_channel_info("game - log_int");

	rand_resync();

	channel_current = channel_server;


	if (!is_paused()) pause();

	text_mode(-1);


	set_config_file("client.ini");
	msecs_per_fps = get_config_int("View", "FPS_Time", 200);

	//msecs_per_render = (int)(1000. / get_config_float("View", "MinimumFrameRate", 10) + 0.5);
	// instead such a weird equation, it's simply defined in terms of milliseconds.
	msecs_per_render = int( get_config_float("View", "MinimumFrameRate", 10) + 0.5 );
	
	msecs_per_render_max = 100;
	if (msecs_per_render > msecs_per_render_max)
		msecs_per_render = msecs_per_render_max;

	prediction = get_config_int("Network", "Prediction", 50);
	if ((prediction < 0) || (prediction > 100)) {tw_error ("Prediction out of bounds (0 < %d < 100)", prediction);}

	log_debug("game::init sharing server.ini\n");

	//display_channel_info("game - log_file server.ini");

	set_config_file("server.ini");
	
	//display_channel_info("game - log_file server.ini was received");

	camera_hides_cloakers = get_config_int("View", "CameraHidesCloakers", 1);
	share(-1, &camera_hides_cloakers);

	show_red_cloaker = get_config_int("View", "ShowRedCloaker", 1);
	share(-1, &show_red_cloaker);

	time_ratio = (int)(1000. / get_config_float ("Game", "SC2FrameRate", 20));
	share(-1, &time_ratio);

	distance_ratio = (3840. / get_config_float ("Game", "SC2TotalDistance", 8000));
	share(-1, &distance_ratio);

	frame_time = (int)(1000. / get_config_float ("Game", "TicRate", 40) + 0.5);
	share(-1, &frame_time);

	normal_turbo = get_config_float("Game", "Turbo", 1.0);
	share(-1, &normal_turbo);

	f4_turbo = get_config_float("Game", "F4Turbo", 10.0);	
	share(-1, &f4_turbo);

	friendly_fire = get_config_int("Game", "FriendlyFire", 0) == 0 ? 0 : 1;
	share(-1, &friendly_fire);

	shot_relativity = get_config_float("Game", "ShotRelativity", 0);
	share(-1, &shot_relativity);

	size = Vector2 (
		get_config_float("Game", "MapWidth", 0),
		get_config_float("Game", "MapHeight", 0)
	);
	share(-1, &size.x);
	share(-1, &size.y);

	global_lag_synch = get_config_int("Network", "SyncLag", 0);
	share(-1, &global_lag_synch);
	do_desynch_test = get_config_int("Network", "DesyncTest", 0);
	share(-1, &do_desynch_test);

	// the host shares its information with the rest of the field...
	share_update();

	turbo = normal_turbo;

//	log_debug("game::init init_lag\n");


	//display_channel_info("game - init_lag");

//	init_lag();
	
//	log_int(lag_frames, channel_server);
//	log_int(channel_init, lag_frames);

	tic_history = new Histograph(128);
	render_history = new Histograph(128);

	prepare();

	log_debug("game::init done\n");

	init_players();


	return;
}



void Game::change_view(View *new_view) {STACKTRACE//this function looks wrong to me
	View *v = new_view;
	v->preinit();
	v->init(view);
	if (view)
		v->replace(view);
	else {
		v->window->init(window);
		v->window->locate(
			0, 0,
			0, 0,
			0, 1,
			0, 1
			);
	}
	view = v;
	return;
}

void Game::change_view(const char * name) {STACKTRACE
	View *v = get_view(name, view);
	if (!v)	{tw_error("Game::change_view - invalid view name");}
	if (view)
		v->replace(view);
	else 
	{
		v->window->init(window);
		v->window->locate(
			0, 0,
			0, 0,
			0, 1,
			0, 1
			);
	}
	view = v;
	return;
}

Game::~Game() {STACKTRACE
	message.out("deleteing GameEvents");
	int i;
	for (i = 0; i < events_waiting; i += 1) free(waiting_events[i]);
	delete[] waiting_events;

	message.out("deleteing histographs");
	delete tic_history; tic_history = NULL;
	delete render_history; render_history = NULL;

	/*
	message.out("deleteing misc. sprites");

	delete meleedata.xpl1Sprite;
	delete meleedata.panelSprite;
	delete meleedata.kaboomSprite;
	delete meleedata.hotspotSprite;
	delete meleedata.sparkSprite;
	delete meleedata.asteroidExplosionSprite;
	delete meleedata.asteroidSprite;
	delete meleedata.planetSprite;

	meleedata.xpl1Sprite = NULL;
	meleedata.panelSprite = NULL;
	meleedata.kaboomSprite = NULL;
	meleedata.hotspotSprite = NULL;
	meleedata.sparkSprite = NULL;
	meleedata.asteroidExplosionSprite = NULL;
	meleedata.asteroidSprite = NULL;
	meleedata.planetSprite = NULL;

	planet_victory = (Music*) (melee[MELEE_PLANET+PLANET_FRAMES].dat);
	*/

	if (music && (music != (Music*)-1)) {
		sound.stop_music();
		sound.unload_music(music);
	}

//	message.out("deleteing data file");
//	unload_datafile(melee); melee = NULL;

	message.out("deleteing game objects");
	destroy_all();

	message.out("other shit");
	message.flush();

	delete view;
	window->remove_callback(this);
	delete window;
}

bool Game::is_paused() {STACKTRACE
	if (time_paused != -1) return true;
	return false;
}

void Game::pause() {STACKTRACE
	if (time_paused != -1) tw_error ("can't pause -- already paused");
	time_paused = get_time();
}

void Game::unpause() {STACKTRACE
	if (time_paused == -1) tw_error ("can't unpause -- not paused");
	redraw();
	paused_time += get_time() - time_paused;
	time_paused = -1;
	return;
}

void Game::save_screenshot() {STACKTRACE
	static int shot_index = 0;
	char path[80];

	while (true) {
		sprintf(path, "./scrshots/shot%04d.pcx", shot_index);
		if (!exists(path)) break;
		shot_index += 1;
		if (shot_index > 10000) {
			sprintf(path, "./scrshots/");
			break;
		}
	}

	if(file_select("Save screen shot", path, "BMP;PCX;TGA")) {
		BITMAP *bmp;
		PALETTE pal;
		get_palette(pal);
		bmp = create_sub_bitmap(screen, 0, 0, SCREEN_W, SCREEN_H);
		save_bitmap(path, bmp, pal);
		destroy_bitmap(bmp);
	}

//	font = (FONT *)(scppal[SCPPAL_FONT].dat);
	return;
}

bool Game::handle_key(int k)
{
	STACKTRACE;

	switch (k >> 8) {
		#if !defined _DEBUG
		case KEY_F11: {
			pause();
			save_screenshot();
			unpause();
			return true;
		}
		break;
		case KEY_F10: 
		#endif

		case KEY_ESC: {
//(*((int*)NULL)) = 0;
 			pause();
			if (tw_alert("Game is paused", "&Abort game", "&Resume playing") == 1)
			{
				//game->quit("quit - Game aborted from keyboard");
				// all local players should disconnect... or so...
				int i;
				for ( i = 0; i < num_network; ++i )
				{
					if (player[i] && player[i]->islocal() )
					{
						CALL(disconnect, p_local);
					}
				}
			}
			unpause();
			return true;
		}
		break;


		case KEY_F1: {// help
			pause();
			show_file("ingame.txt");
			unpause();
			return true;
		}
		break;
		case KEY_F3: {// switch hero
			if (num_focuses) focus_index = (focus_index + 1) % num_focuses;
			message.print(1000, 15, "Camera focus %d (of %d)", focus_index+1, num_focuses);
			return true;
		}
		break;
		case KEY_F4: {// toggle cruise-control on/off
			extern bool cruise_control;
			cruise_control = !cruise_control;

			if (cruise_control)
			{
				message.print(1500, 12, "cruise control enabled");
			} else {
				message.print(1500, 12, "cruise control disabled");
			}
			return true;
		}
		break;

		case KEY_F9: {// toggle cyborg control on/off for the local player.
			if (player[p_local] && player[p_local]->control)
			{
				Control *c = player[p_local]->control;

				c->cyborg_control = !c->cyborg_control;
				
				if (c->cyborg_control)
				{
					message.print(1500, 12, "cyborg control enabled");
				} else {
					message.print(1500, 12, "cyborg control disabled");
				}
			}
			return true;
		}
		break;

		case KEY_F6: {// send message
			chat_len = 0;
			chat_buf[0] = '\0';
			chat_on = 1;
			return true;
		}
		break;
		case KEY_F7: {
			if (physics_locked) return false;
			if (frame_time < 7) {
				frame_time = 50;
			}
			else if (frame_time < 15) {
				frame_time = 5;
			}
			else if (frame_time < 30) {
				frame_time = 10;
			}
			else {
				frame_time = 25;
			}
			message.print(1000, 15, "Game Tic rate set to %f / second", 1000./frame_time);
			return true;
		}
		break;
		case KEY_F8: {
			if (view_locked) return false;
			if (!view || !view->type) return false;
			if (!strcmp(view->type->name, "Hero")) {
				game->change_view("Enemy");
				message.print(2500, 15, "View mode changed to 'Enemy'");
				return true;
			}
			if (!strcmp(view->type->name, "Enemy")) {
				game->change_view("Hero");
				message.print(2500, 15, "View mode changed to 'Hero'");
				return true;
			}
			//redraw();
			return true;
		}
		break;
		default: {
			if (chat_on) {
				if ((k >> 8) == KEY_ENTER) {
					//send_game_event(new GameEventMessage(chat_buf));
//					player_said(0, chat_buf);
					CALL(chat, p_local);
					chat_on = 0;
				}
				else if (k & 255) {
					if ((k & 255) == 8) {
						if (chat_len > 0) {
							chat_len -= 1;
							chat_buf[chat_len] = 0;
						}
					}
					else {
						chat_buf[chat_len] = k & 255;
						if (chat_len < 255) chat_len += 1;
						chat_buf[chat_len] = 0;
					}
				}
				return true;
			}
		}
		break;
		case KEY_F2: {
			pause();
//			font = (FONT *)(scppal[SCPPAL_FONT].dat);
			//view->set_window(NULL, 0, 0, 0, 0);
			//change_options();
			options_menu(this);
			//set_resolution(video.width, video.height);
//			font = (FONT *)(scppal[SCPPAL_FONT].dat);
			unpause();
			return true;
		}
		break;
#		ifdef _DEBUG
			case KEY_B://supposed to be F12, but the debugger isn't fond of that
#		endif
		case KEY_F12: {
			show_fps = !show_fps;
			return true;
		}
		break;
	}
	return false;
}

int Game::set_frame_time(int t) {STACKTRACE
	this->frame_time = t;
	prepare();
	return 1;
}

int Game::set_turbo(double t) {STACKTRACE
	this->normal_turbo = t;
	prepare();
	return 1;
}

double Game::get_turbo() {
	return this->normal_turbo;
}

void Game::play_music() {STACKTRACE
	if (-1 == (int)music) return;
	if (!music) {
		set_config_file("client.ini");
		const char *f = get_config_string("Sound", "Music", "melee.dat#MELEEMUS_MOD");

		music = sound.load_music(f);
		if (!music) music = (Music*) -1;
	}
	if (music) sound.play_music(music, TRUE);
	return;
}
















EventClass::EventClass()
{
	N = 0;


	// initially, all are ready.
	int i;
	for ( i = 0; i < max_network; ++i )
		need_more[i] = false;
}


void EventClass::clear()
{
	// clear the registry: is important, cause this gets rid of references with obsolete game pointers

	int i;
	for ( i = 0; i < N; ++i )
	{
		if (event[i].call)
		{
			delete event[i].call;
			event[i].call = 0;
		}
	}

	N = 0;

	// you've to reset this, too... otherwise, decisions made in one game, are still present in a
	// next game.
	for ( i = 0; i < max_network; ++i )
		need_more[i] = false;
}


void EventClass::reg(GameEvent2 *g, char *id)
{	
	event[N].call = g;
	strncpy(event[N].name, id, 64);

	// check if there's a ( or ) character in there (indicates a mistake in the naming)
	if (strchr(id, '(') || strchr(id, ')') )
		tw_error("An event name with ( or ) is probably a typo");

	++N;

	if (N > max_events)
		tw_error("pushing too many events");

}


// use this, to add a request to the stack ; it's searched by its id-string

void EventClass::request(char *id, int iplayer)
{
	// find the index of the function
	int i;
	for ( i = 0; i < N; ++i )
	{
		if (strcmp(event[i].name, id) == 0)
		{

			break;
		}
	}

	if (i < N)
	{
		// you're (local) in this channel, currently. Because, only the local player(s) can request something.
		// hmm... actually, that's not correct. There can be >1 local players

		if (!player[iplayer]->islocal())
			tw_error("Requesting network calls only apply to local players.");

		channel_current = player[iplayer]->channel + _channel_buffered;;

		// execute the event in write-mode.
		log_resetmode();	// go to default mode
		log_nextmode();	// jumps from default to write mode

		log_int(i);			// function id
		// player index is not stored: it's assumed that each player has its own channel
		issue(i, iplayer);			// data
		//event[i].call->calculate();
	}

	// go to default mode.
	log_resetmode();
}

void EventClass::issue(int i, int iplayer)
{
	event[i].call->calculate(iplayer);
}


void EventClass::set_wait()
{
	int i;
	for ( i = 0; i < num_network; ++i )
	{
		need_more[i] = true;
	}
}

void EventClass::unset_wait(int i)
{
	need_more[i] = false;
}

bool EventClass::all_ready()
{
	int i;
	for ( i = 0; i < num_network; ++i )
	{
		if (!player[i])
			continue;

		if (need_more[i])		// you've to wait for this player.
			return false;
	}

	return true;
}

// Handle (read) one event at a time...
void EventClass::handle()
{

	// note, if you're in write-mode only, then you're faking buffered data ...

	int p;

	bool data_left = true;

	while ( data_left )
	{

		// keeps track if all of the available data for this iteration are read
		data_left = false;	// assume all is read

		for ( p = 0; p < num_network; ++p )
		{
			if (!player[p])
				continue;
			
			if (!need_more[p])
				continue;
			
			// note, you can send+receive on your own channel, cause it's already buffered; the
			// receive reads from the start, and the send adds to the end of the log buffer.
			
			channel_current = player[p]->channel + _channel_buffered;
			event_player_current = p;	// can also be useful.
			
			if (log_empty())	// only try to read the buffer, if there are data in the buffer...
				continue;
			// that will avoid getting caught inside this loop, while data are only being read outside
			// of this loop.
			// note that it uses channel_current.

			log_resetmode();	// go to default mode.
			
			log_nextmode();	// go to write mode
			log_nextmode();	// and skip, go to read mode.
			
			// executes a call to the function in the list, with that id-number
			int req;
			log_int(req);

			if (log_empty())	// only try to read the buffer, if there are data in the buffer...
			{
				message.print(1500, 12, "waiting for network data (should not happen)!");
				// if this indeed occurs, then I'll have to add a length-measure for each "event" ...
				// so I hope it doesn't happen.
			}

			// player index is not stored: it's assumed that each player has its own channel
			issue(req, p);

			if (!log_empty() && need_more[p])
				data_left = true;
			
		}

	}
	
	// leave the logs in default mode for subsequent operations !!
	log_resetmode();
}


EventClass events;




//static bool has_registered = false;
void Game::register_events()
{
	/*
	this is wrong; the combination between Game and the function is handled
	in a template, which is hidden behind the EVENT macro. It's impossible
	to pass class-routines directly by pointer ...
	*/
	EVENT(Game, chat);
	EVENT(Game, change_lag);
	EVENT(Game, test_event1);
	EVENT(Game, disconnect);
	EVENT(Game, start_iteration);
	EVENT(Game, notify_skip_iteration);
	EVENT(Game, event_sharecontrols);
	EVENT(Game, event_lag_increase);
	EVENT(Game, event_lag_decrease);
	EVENT(Game, event_share_desynch);
	EVENT(Game, share_crc);
}





/*
void Game::do_game_events2()
{
	events.handle();
}
*/






void Game::share_crc(int iplayer)
{
	int crcvalue;

	if (log_synched)
	{
		// in receiving mode.
		log_int(crcvalue);

		// store it for later use (for when all the players are updated)
		player[iplayer]->crc = crcvalue;

	} else {
		// in sending mode
		// you've to define the data before sharing it with others

		int i, sumpos, sumvel;
		
		sumpos = 0;
		sumvel = 0;
		
		for ( i = 0; i < num_items; ++i )
		{
			if (item[i]->exists())
			{
				SpaceLocation *o = item[i];
				sumpos += iround(o->pos.x + o->pos.y);
				sumvel += iround(o->vel.x + o->vel.y);
			}
		}

		crcvalue = sumpos + sumvel;

		log_int(crcvalue);

	}

}




void Game::change_lag(int iplayer)
{
	STACKTRACE;

	// always use channel_current
	log_int(lagchange);

	// note that write-mode acts directly - but you don't want that, you want to wait
	// till the buffer catches up, and you're in read-mode to check it.

	if (log_synched)
	{
		if (lagchange > 0)
			increase_latency(lagchange);
		else
			decrease_latency(-lagchange);
	}
}




void Game::chat(int iplayer)
{
	STACKTRACE;

	// always use channel_current
	int n = strlen(chat_buf+1) + 1;
	log_int(n);
	log_data(chat_buf, n);

	if (log_synched)
	{
		message.out(chat_buf, 6000, 15);
	}
}



#include "mcbodies.h"

void Game::test_event1(int iplayer)
{
	// it's allowed to manipulate game physics directly, here, cause it's automatically
	// synched.

	if (log_synched)
		add(new Asteroid());
}




void Game::item_sum(char *comment)
{
	int i, sumpos, sumvel;
	
	sumpos = 0;
	sumvel = 0;

	for ( i = 0; i < num_items; ++i )
	{
		SpaceLocation *o = item[i];
		sumpos += iround(o->pos.x + o->pos.y);
		sumvel += iround(o->vel.x + o->vel.y);
	}

	message.print(1500, 15, "[%s] num[%i] pos[%i] vel[%i]", comment, num_items, sumpos, sumvel);
	message.animate(0);
}

/*
void Game::heavy_compare()
{
	if (!detailed_network_check)
		return;

	//message.print(1500, 13, "num_items = %i", num_items);
	//message.animate(0);

	int p;
	
	const int max_comp = 512;
	int val[max_comp];

	int N, Nprev;
	for ( p = 0; p < num_network; ++p )
	{
		if (!player[p])
			continue;

		channel_current = player[p]->channel + _channel_buffered;

		log_resetmode();

		while (log_nextmode())
		{
			// mode 0 : first, perform all write-actions.
			// mode 1 : then, perform all read-actions.
			//  needs to be done, since the first read (N) determines all following read/writes.
			//  and read/writes cannot be mixed!!

			N = num_items;
			
			if (N > max_comp)		// don't compare absurd much
				N = max_comp;
			log_int(N);
			
			if (log_synched)
			{
				// only compare for player > 0, otherwise you've nothing to compare to
				if (p > 0 && N != Nprev)
					tw_error("different number of items between games");

				// only update, in synched mode - in true-time, comparisons like this
				// are impossible in a buffered channel.
				Nprev = N;
			}
			
			
			int i;
			for ( i = 0; i < N; ++i )
			{
				
				int test, id;
				
				if (!log_synched)
				{
					// only generate in true-time
					// otherwise, N may differ from the true number of items in the game.
					test = iround(item[i]->pos.x + item[i]->pos.y +
								item[i]->vel.x + item[i]->vel.y);
					id = item[i]->debug_id;
				}


				// exchange
				log_int(test);
				log_int(id);

				// use the data in game-time
				if (log_synched)
				{
					if (p > 0)
					{
						// compare previous player to current player
						if (test != val[i])
							tw_error("Differing position/velocity values between game items, debug_id = %i", id);
					}

					// also needs to be done for player 0
					// only update/compare while you're in synched mode.
					val[i] = test;
					
				}

			}
		}

	}

	// leave the logs in default mode for subsequent operations !!
	log_resetmode();
}
*/


void Game::disconnect(int iplayer)
{
//	message.print(1500, 12, "Disconnecting channel[%i] log_synched[%i] (press key!)", channel_current, (int)log_synched);
//	message.animate(0);
//	readkey();
	// you are in channel_current, so the channel_current player disconnects...

	if (log_synched)
	{
		// in receiving mode.

		// check which players are to be removed
		if (!player[iplayer])
			tw_error("Can't disconnect a non-existing player");

		if (player[iplayer]->haschannel(channel_current))
		{
			player[iplayer]->status = false;
			//remove_player(i);	// do this later, because you're still reading data, in a loop
			//xxx check
			//--i;	// repeat player i		BUT, this should also apply to channel_current and such !!
		}
	}
}


void Game::notify_skip_iteration(int iplayer)
{
	if (log_synched)
	{
		skip_iteration = true;
	}
}

void Game::start_iteration(int iplayer)
{
	if (!log_synched)
	{
		// in write mode:
		// local players give the "go" signal (each is treated)

		if (player[iplayer] && player[iplayer]->islocal())
			log_int(iplayer, player[iplayer]->channel + _channel_buffered);

	} else {
		// in receiving mode.

		// receive the player number
		int k;
		log_int(k);

		if (k != iplayer)
			tw_error("Expected player number does not match");

		events.unset_wait(k);

		//message.print(1500, 14, "event: start_iteration player %i  game_time %i", iplayer, game_time);
	}
}


NPI *Game::new_player()
{
	NPI *p;
	p = new NPI();
	


	return p;
}



void Game::remove_player(int i)
{
	message.print(1500, 15, "removing player[%i]", i);
	message.animate(0);

	if (!player[i])
		tw_error("Error: player does not exist, cannot remove player.");
	

	if (player[i]->status)
	{
		tw_error("Player status is still active!");
		// you've to use the status, to signal that this player is dead.
		// you've to actually remove him by calling this routine, in a separate loop (looping over all players, checking for their status)
		// when it's safe.
	}


	// if you're the local player, simply quit the game
	if (player[i]->islocal())
	{
		game->quit("none");
		//tw_alert("Stopping", "&Ok");

		// remove the connections to all other players
		int k;
		for ( k = 0; k < num_network; ++k )
		{
			if (glog->type == Log::log_net)
			{
				if (net_conn[k] != -1)	// if not local
					((NetLog*)glog)->rem_conn(net_conn[k]);
			}
		}

		// in principle you should also set the read/write direction on those
		// channels to 0... but that's not so important.
	} else {
		// Disable that players buffered channel
		// note that this references a local buffer, which has to be disabled like this, independently
		// from the actual network connection.
		// (it's independent, because in principle, this supports >1 player per computer)

		int ch;
		//	int k;
		
		ch = player[i]->channel;
		glog->set_direction(ch, 0);
		glog->set_direction(ch + _channel_buffered, 0);
		// the read and write directions are disabled because of the value 0.

		
		// remove the players network-connection from the network... which network conn.
		// goes to that player... well, that's analyzed on-the-fly by the packet
		// receiving algorithm (most channels are only owned by 1 player, on 1 network
		// connection)
		//	k = ((NetLog*)glog)->conn(ch);//channel_conn_recv[ ch ];
		
		//	if (k >= 0)
		if (net_conn[i] != -1)
		{
			message.print(1500, 14, "REMOVE player[%i] conn[%i]", i, net_conn[i] );
			message.animate(0);
			
			((NetLog*)glog)->rem_conn(net_conn[i]);
		}
	}


	// Let's keep using the array indices to
	// reference each player. So, just delete the player and set the pointer to zero; you've to
	// take this into account in iterations over "all" players, by checking for null-pointers.

	delete player[i];
	player[i] = 0;


	message.print(1500, 14, "frame[%i] time[%i] (press key!)", frame_number, game_time);
	message.animate(0);
//	readkey();
}



int Game::add_player (int num, Control *c, int team_index, const char *name, const char *fleet_section,
							const char *fleet_file)
{
	STACKTRACE;

	NPI *p = new_player();
	player[num] = p;

	p->channel = c->channel;

					

	//player_control =   (Control**) realloc(player_control,   sizeof(Control*)   * (num+1));
	//player_name    =      (char**) realloc(player_name,      sizeof(char*)      * (num+1));
//	player_panel   = (ShipPanel**) realloc(player_panel,     sizeof(ShipPanel*) * (num+1));
	//player_fleet   =     (Fleet**) realloc(player_fleet,     sizeof(Fleet *)    * (num+1));
	//player_team    =   (TeamCode*) realloc(player_team,      sizeof(TeamCode)   * (num+1));
	//player_color   =        (int*) realloc(player_color,      sizeof(char*)      * (num+1));
	
	p->control = c;
	
	
	add_focus(c, c->channel);
//	player_panel[num] = NULL;
	
	p->fleet->reset();
	p->color = num + 1;
	strncpy(p->name, name, sizeof(p->name));
	strdup(p->name);

	p->team = new_team();

	/* IGNORE FLEET FOR NOW
	char sect[40];
	sprintf(sect, "Player%d", num+1);
	if (c->channel == channel_none)
	{
	//	tw_error("channel_none not allowed here");
		//log_file(fleet_file);
		//::fleet->load(NULL, fleet_section);

		// change GEO:
		// bot ships are defined by the server only (!)
		if (glog->writeable(channel_server))	// the server...
			p->fleet->load(fleet_file, fleet_section);

		log_fleet(channel_server, p->fleet);
		c->target_sign_color = ((3+num) % 7) + 1;

		}
	else if (glog->get_direction(c->channel) & Log::direction_write)
	{
		p->fleet->load(fleet_file, fleet_section);
		log_fleet(c->channel, p->fleet);
		c->target_sign_color = ((3+num) % 7) + 1;
		}
	else {
		log_fleet(c->channel, p->fleet);
		}
		*/

	/*
	set_config_file("tmp.ini");
	set_config_string(sect, "Name", name);
	set_config_string(sect, "Type", c->getTypeName());
	set_config_int(sect, "Team", team_index);
	set_config_int(sect, "Channel", c->channel);
	set_config_int(sect, "StartingFleetCost", p->fleet->getCost());
	set_config_int(sect, "StartingFleetSize", p->fleet->getSize());
	p->fleet->save(NULL, sect);
	p->fleet->save("fleets.tmp", sect);
	*/
	return num;
}


void Game::init_players()
{
	STACKTRACE;


	switch (glog->type)
	{
		case Log::log_normal:
			{
			int i;
			int count = 0;

			num_network = 0;
			num_bots = 0;

			for ( i = 0; true; i += 1)
			{
				char buffy[64];
				sprintf(buffy, "Player%d", i + 1);
				set_config_file("scp.ini");
				const char *type = get_config_string(buffy, "Type", NULL);
				if (!type) break;
				if (strcmp(type, "none") == 0) continue;
				const char *name = get_config_string(buffy, "Name", buffy);
				char config[64];
				sprintf(config, "Config%d", get_config_int(buffy, "Config", 0));
				//int channel = channel_server;
				//if (strcmp(type, "WussieBot") == 0) channel = channel_none;
				//if (strcmp(type, "MoronBot") == 0) channel = channel_none;
				int ti = get_config_int(buffy, "Team", 0);
				

				int ch;
				int k;

				if (strcmp(type, "Human") == 0)
				{
					ch = channel_network[i];

					if (i > num_network)
					{
						// if there's a bot mentioned in-between humans...
						player[i] = player[num_network];	// the bot is moved backwards
						player[num_network] = 0;	// the human
					}

					k = num_network;
					++num_network;

				} else {
					ch = channel_none;
					k = i;
					++num_bots;
				}

				add_player(k, create_control(ch, type, config), ti, name, buffy);

				// you have to override the default settings, so that all multi-player aimed stuff
				// is channeled into a single player channel, which is handled locally.
				++count;
			}

			//num_network = count;		// --> you should have just one human CHANNEL (note that there can be more humans, sharing this channel)
			//num_bots = 0;
			num_players = num_network + num_bots;
		}
		break;
		case Log::log_net: {

			//log_file("server.ini");
			// isn't needed yet ?!

			//log_test("mmain(a)");

			int j;
			for (j = 0; j < num_network; j += 1)
			{
				int ch;
				ch = channel_network[j];
				
				int team;
				char buffy[64];
				char name[512];
				
								
				//int tmp = 1;
				//log_int(ch, tmp);	// this is only a signal for a variable # of bots.
				// which we shouldn't use, cause channel_none players should be treated in
				// another loop, imo ...
				
				channel_current = ch;
				

				// you can't do this here, yet; first, the players must be initialized. But, this isn't important anyway.
				//share(j, &team);						// this uses channel_current
				//share_string(j, name);				// this uses channel_current
				//share_update();
				
			//	message.print(1500, 15, "[%i] [%i] [%s]",
			//		team, name_length, name);
			//	message.animate(0);
				
				team = j+1;

				if (is_local(ch))
				{
					// you also need keyboard info for local players, read from ini file.
					// Config0 are controls of player0
					//char config[64];
					//set_config_file("scp.ini");
					//sprintf(config, "Config%d", get_config_int("Config0", "Config", 0));
					set_config_file("scp.ini");
					add_player(j, create_control(ch, "Human", "Config0"), team, name, buffy);
					
				} else {
					
					add_player(j, create_control(ch, "Whatever"), team, name, NULL);	
					player[j]->control->auto_update = false;
					// disable automatic key calculation (which is enable by default for all non-humans).
				}
			}
				
				


			// adding bots
			int team = new_team();

			for (j = num_network; j < num_players; j += 1)
			{

				char name[512], fleetid[512];
				sprintf(name, "Bot_%i", j+1);
				sprintf(fleetid, "Player%i", j+1);

				int ch;
				ch = channel_none;
				add_player(j, create_control(ch, "WussieBot"), team, name, fleetid);

			}


		}
		break;
	}

	return;
}
