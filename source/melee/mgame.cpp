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


/** \brief this causes lots of extra checks to be performed on object-by-object
synch comparison. This is ok for testing on lan, but is best set to false
for internet play, and non-testing compilations.
*/
bool detailed_network_check = false;


bool PlayerInformation::haschannel(int ch)
{
	return (channel == ch  ||  channel + _channel_buffered == ch );
}

// destroy stuff that belongs to the player.
void PlayerInformation::die()
{
	control->die();
}



void display_channel_info(char *txt = "")
{
	int p;
	for ( p = 0; p < num_network; ++p )
	{
		int ch = channel_network[p];
		message.print(1500, 15, "[%s] player[%i]  (direct) [%i]  (buffered) [%i]", txt, p,
			glog->log_len[ch] - glog->log_pos[ch],
			glog->log_len[ch+1] - glog->log_pos[ch+1]);
	}
	message.animate(0);
	//if (p_local == 0) readkey();
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
	//if (p_local == 0) readkey();
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



void Game::gen_buffered_data()
{
	++ next_frame;		// doesn't this lead to additional lag-frames in the buffer ??????

	// this is needed to push (fake) data to create an initial lag-buffer, without
	// having the trouble of synchronized reads which have the nasty habit of just begging
	// for instant data ;)

	if (next_frame > 1)
		log_set_fake();	// buffering data means ... well you know.
	else
		log_set_nofake();
	log_set_default();



	// there must be at least one "next frame" otherwise nothing happens.

	//log_show_data = true;
	while (next_frame > 0)
	{
		-- next_frame;

		// game events
		net_expect(92308549);		
		compare_checksums();
		
		// checksum check
		net_expect(692930);
		do_game_events2();		// testing version only , for now !!
		
		// the control data ... how to do those ? They're one of the presences, so, call them
		// in the following way, I think that's ok ?
		net_expect(2909485);
		int i;
		for ( i = 0; i < num_presences; ++i )
			presence[i]->gen_buffered_data();
		
		for ( i = 0; i < num_items; ++i )
			item[i]->gen_buffered_data();
		net_expect(8938034);
	}
	//log_show_data = false;

	//display_channel_info();
	//if (p_local == 0) readkey();
	//readkey();

	log_set_nofake();
	log_resetmode();
}



static char chat_buf[256];
static int chat_len = 0;
static int chat_on = 0;

int random_seed[2];

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
		SpaceSprite::ALPHA | SpaceSprite::MASKED | SpaceSprite::MIPMAPED);
	meleedata.hotspotSprite           = new SpaceSprite(&melee[MELEE_HOTSPOT], HOTSPOT_FRAMES,
		SpaceSprite::ALPHA | SpaceSprite::MASKED | SpaceSprite::MIPMAPED);
	meleedata.sparkSprite             = new SpaceSprite(&melee[MELEE_SPARK], SPARK_FRAMES,
		SpaceSprite::ALPHA | SpaceSprite::MASKED | SpaceSprite::MIPMAPED | SpaceSprite::MATCH_SCREEN_FORMAT);
	meleedata.asteroidExplosionSprite = new SpaceSprite(&melee[MELEE_ASTEROIDEXPLOSION], ASTEROIDEXPLOSION_FRAMES);
	meleedata.asteroidSprite          = new SpaceSprite(&melee[MELEE_ASTEROID], ASTEROID_FRAMES);
	meleedata.planetSprite            = new SpaceSprite(&melee[MELEE_PLANET], PLANET_FRAMES);
	meleedata.xpl1Sprite              = new SpaceSprite(&melee[MELEE_XPL1], XPL1_FRAMES,
		SpaceSprite::ALPHA | SpaceSprite::MASKED | SpaceSprite::MIPMAPED);

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
int num_hotseats[max_network];		// number of hotseat-players per connected computer
int channel_conn_recv[100];

int p_local;				// this defines the local player.
//int num_players = 0;		// the number of players in the game.
int num_network = 0;
int num_bots = 0, num_players = 0;


PlayerInformation *player[100];


//void set_numplayers(int n)
//{
//	num_players = n;
//}

int channel_local()
{
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
	log_file(type->file);
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
				if (!glog->ready(channel_network[i] + _channel_buffered))
					result = false;
			//if (!log->ready(channel_server + Game::_channel_buffered)) return false;
			return result;	// if one or more channels aren't ready yet, it returns false.
		}
	}
	return true;
}

void Game::handle_desynch(int local_checksum, int server_checksum, int client_checksum) {STACKTRACE
	error("Game Desynchronized\nTime=%d Frame=%d\nClient=%d Server=%d Local=%d", game_time, frame_number, (int)client_checksum, (int)server_checksum, (int)local_checksum);
}

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




void Game::compare_checksums()
{
	STACKTRACE;

	heavy_compare();	// compare all "live" items

	unsigned char local_checksum = checksum() & 255;
	unsigned char client_checksum[max_network];
	memset(client_checksum, 0, sizeof(char)* max_network);
	unsigned char server_checksum = local_checksum;
	bool desync = false;

	log_char(server_checksum, channel_server + CHECKSUM_CHANNEL);
	if (lag_frames)
	{
		int i;
		for ( i = 1; i < num_network; ++i )	// note, 0==server.
		{
			// this checks for "null" channels...
			if (glog->log_dir[channel_network[i] + CHECKSUM_CHANNEL] == 0)
				continue;

			client_checksum[i] = local_checksum;
			log_char(client_checksum[i], channel_network[i] + CHECKSUM_CHANNEL);

			if (server_checksum != client_checksum[i])
				desync = true;
		}


		/*
		if (p_local == 0)	// serverside-check only
		{
			message.print(1500, 14,
				"1: %i  2: %i  3: %i  frame: %i",
				(int)server_checksum,
				(int)client_checksum[1],	// note that "0" in principle equals the server, and it isn't defined.
				(int)client_checksum[2],
				frame_number);
			message.animate(0);
			//readkey();
			idle(10);		// this slows the game down a lot...
		}
		//*/
		
		

	}

	if (glog->playback) {
		if (lag_frames) 
			log_char (local_checksum, channel_playback + CHECKSUM_CHANNEL);
		if (local_checksum != server_checksum) desync = true;
	}

	this->local_checksum = local_checksum;
	this->client_checksum = client_checksum[1];
	this->server_checksum = server_checksum;

	if (desync)
	{
		handle_desynch(local_checksum, server_checksum, this->client_checksum);
	}
}

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
		k= val;

		log_int(k, channel_network[p] + _channel_buffered);

		if (k != val)
			result = false;
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

void Game::play() {_STACKTRACE("Game::play")
	set_resolution(window->w, window->h);
	prepare();
	if (is_paused()) unpause();

	try {
		while(!game_done)
		{

			unsigned int time = get_time();
			poll_input();
			videosystem.poll_redraw();
			if(!sound.is_music_playing()) {
				play_music();
			}
			if ((next_tic_time <= time) && (next_render_time > game_time) &&
				(game_ready() || game_time == 0)) {		// note that game_time==0 is also needed, cause otherwise it'll wait for data, while no data've been generated yet.
				_STACKTRACE("Game::play - Game physics")

				// first some i/o and net-traffic
				gen_buffered_data();
				glog->flush_noblock();
				glog->listen();

				// then calculate game stuff
				calculate();

				if (auto_unload) unload_unused_ship_data();

				if (key[KEY_F4])
					turbo = f4_turbo;
				else
					turbo = normal_turbo;
				next_tic_time += (frame_time / turbo);
				if ((hiccup_margin >= 0) && (next_tic_time + hiccup_margin < get_time()))
					next_tic_time = get_time();
				if (next_fps_time <= game_time) {
					next_fps_time += msecs_per_fps;
					fps();
				}

//				message.print(10, 15, "Idle time = %i", tot_idle_time / frame_number);
//				message.animate(0);
			}
			else if (interpolate_frames || (game_time > next_render_time - msecs_per_render)) {
				_STACKTRACE("Game::play - Game rendering")
				animate();
				next_render_time = game_time + msecs_per_render;
			}
			else
			{
				int n = 1;
				idle(n);
				tot_idle_time += n;
				if (glog->type == Log::log_net)
				{
					NetLog *l = (NetLog*) glog;
					l->recv_noblock();		// receive stuff, if you can

					// this helps to reduce idle-time, cause it doesn't have to wait till
					// data are received first (namely that's what game_ready() tests).
					l->flush_noblock();			// this sends, if there's something to send at least
				}
			}
			while (keypressed())
				handle_key(readkey());
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
		if (source && source->exists()) tmp = source->data->moduleVictory;
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
void Game::fps() {STACKTRACE
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

	if (this->show_fps) {
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
		message.print((int)msecs_per_fps, 12, "shipdatas loaded: %d", shipdatas_loaded);
	}

	if (chat_on)
		message.print((int)msecs_per_fps, 15, "say: %s", chat_buf);
}

void Game::preinit() {STACKTRACE
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

	int tmp = glog->type;
	log_int(tmp, channel_init);
	if (glog->playback) glog->type = tmp;

	log_debug("game::init rand\n");

	//display_channel_info("game - log_int");

	rand_resync();
	/*
	channel_current = channel_server;

	i = rand();
//	i = 9223;
	log_int(i);
	random_seed[0] = i;
	rng.seed(i);
	i = rand();
//	i = 7386;
	log_int(i);
	random_seed[1] = i;
	rng.seed_more(i);
	*/

	if (!is_paused()) pause();

	text_mode(-1);


	set_config_file("client.ini");
	msecs_per_fps = get_config_int("View", "FPS_Time", 200);
	msecs_per_render = (int)(1000. / get_config_float("View", "MinimumFrameRate", 10) + 0.5);
	prediction = get_config_int("Network", "Prediction", 50);
	if ((prediction < 0) || (prediction > 100)) {tw_error ("Prediction out of bounds (0 < %d < 100)", prediction);}

	log_debug("game::init sharing server.ini\n");

	//display_channel_info("game - log_file server.ini");

	log_file("server.ini");
	
	//display_channel_info("game - log_file server.ini was received");

	camera_hides_cloakers = get_config_int("View", "CameraHidesCloakers", 1);
	time_ratio = (int)(1000. / get_config_float ("Game", "SC2FrameRate", 20));
	distance_ratio = (3840. / get_config_float ("Game", "SC2TotalDistance", 8000));
	frame_time = (int)(1000. / get_config_float ("Game", "TicRate", 40) + 0.5);
	normal_turbo = get_config_float("Game", "Turbo", 1.0);
	f4_turbo = get_config_float("Game", "F4Turbo", 10.0);	
	turbo = normal_turbo;
	friendly_fire = get_config_int("Game", "FriendlyFire", 0) == 0 ? 0 : 1; //MSVC sucks also
	shot_relativity = get_config_float("Game", "ShotRelativity", 0);
	size = Vector2 (
		get_config_float("Game", "MapWidth", 0),
		get_config_float("Game", "MapHeight", 0)
	);

	log_debug("game::init init_lag\n");


	//display_channel_info("game - init_lag");

	init_lag();
	
	log_int(lag_frames, channel_server);
//	log_int(channel_init, lag_frames);

	tic_history = new Histograph(128);
	render_history = new Histograph(128);

	prepare();

	log_debug("game::init done\n");


	return;
}

void Game::init_lag()
{
	STACKTRACE;
	//message.out("init_lag");
	//message.animate(0);

	if (glog->type == Log::log_net) {
		int lag_time = 0;//get_config_int("Network", "Lag", 200);
		int blah = 0;

		//display_channel_info();

		lag_time = 0;

		NetLog* l = (NetLog*)glog;

		int itry, Ntry;
		Ntry = 5;
		for ( itry = 0; itry < Ntry; ++itry )
		{
			int p;
			for ( p = 0; p < num_network; ++p )
			{
				// server-side lag-test ?
				log_int(blah, channel_network[p]);
			}
				
			if (itry > 0)
			{
				// get the average lag time over the connections
				int conn;
				for ( conn = 0; conn < l->num_connections; ++conn )
				{
					lag_time += l->ping[conn];
					message.print(1500, 12, "lag[%i][%i] = %i", itry, conn, l->ping[conn]);
				}
			}
		}
		
		lag_time /= ((Ntry-1) * l->num_connections);
		

//		lag_time = 10;


		log_int(lag_time, channel_server);
		int lagf = (int) (1.5 + lag_time * normal_turbo / (double) frame_time );
#		ifdef _DEBUG
//			lagf += 5;
#		endif
//			lagf += 5;
		message.print(15000, 15, "target ping set to: %d ms (pessimistically: %d ms)", lag_time, iround(lagf * frame_time / normal_turbo));

		// I think the computer is somewhat conservative in estimating lag.
		lagf -= 1;

		// a check, just in case.
		if (lagf < 1)	// each accounts for 1 frame, which is 25 ms...
			lagf = 1;

		increase_latency(lagf);
	}
	else {
		int lagf = 0;//10;//0;
		increase_latency(lagf);//*/
	}
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

bool Game::handle_key(int k) {STACKTRACE
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
				CALL(disconnect);
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
					CALL(chat);
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
	Nreq = 0;
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

void EventClass::request(char *id)
{
	int i;
	for ( i = 0; i < N; ++i )
	{
		if (strcmp(event[i].name, id) == 0)
		{
			req[Nreq] = i;
			++Nreq;

			if (Nreq > max_requests)
				tw_error("pushing too many requests");
			return;
		}
	}
}

void EventClass::issue(int i)
{
	event[i].call->calculate();
}


void EventClass::handle()
{

	// note, if you're in write-mode only, then you're faking buffered data ...

	int i;
	int p;

	int Ne[max_network];	// the number of events per computer(-channel)

	for ( p = 0; p < num_network; ++p )
		Ne[p] = 0;

	Ne[p_local] = Nreq;
	Nreq = 0;


	for ( p = 0; p < num_network; ++p )
	{
		// note, you can send+receive on your own channel, cause it's already buffered; the
		// receive reads from the start, and the send adds to the end of the log buffer.
		
		channel_current = channel_network[p] + _channel_buffered;

		log_resetmode();

		while (log_nextmode())
		{

			// this is needed, cause if there's a mix of read/write operations, in a buffered
			// environment, "current" settings get overwritten by buffered settings with each
			// call to the networking. This can mix up sequencing of shared data.

			// in write-mode, this pushes N and N events onto the end of the log.
			// in read-mode, this scans N and then N events from the bottom of the log.

			log_int(Ne[p]);
			
			
			for ( i = 0; i < Ne[p]; ++i )
			{
				log_int(req[i]);
				issue(req[i]);		// executes a call to the function in the list, with that id-number
			}
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
}






void Game::do_game_events2()
{
	events.handle();
}










void Game::change_lag()
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




void Game::chat()
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

void Game::test_event1()
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
		channel_current = channel_network[p] + _channel_buffered;

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



void Game::disconnect()
{
	// you are in channel_current, so the channel_current player disconnects...

	if (log_synched)	// in receiving mode.
	{
		// check which players are to be removed
		int i;
		for ( i = 0; i < num_players; ++i )
		{
			if (player[i]->haschannel(channel_current))
			{
				remove_player(i);
				--i;	// repeat player i
			}
		}
	}
}


PlayerInformation *Game::new_player()
{
	PlayerInformation *p;
	p = new PlayerInformation();
	
	p->status = true;
	p->control = 0;
	p->color = 1;
	strcpy(p->name, "default");
	p->team = 0;

	return p;
}



void Game::remove_player(int i)
{
	message.print(1500, 15, "removing player[%i]", i);
	message.animate(0);

	// if you're the local player, simply quit the game
	if (is_local(player[i]->channel))
	{
		game->quit("none");
		//tw_alert("Stopping", "&Ok");

		// remove the connections to all other players
		int k;
		for ( k = 0; k < num_network; ++k )
			if (glog->type == Log::log_net)
				((NetLog*)glog)->rem_conn(k);

		// in principle you should also set the read/write direction on those
		// channels to 0... but that's not so important.
	}



	player[i]->status = 0;	// tell the game, that this player is gone ?!

	
	// Disable that players buffered channel ... (but keep the direct channel)
	// cause it's still in the networking part ...
	int ch;
	int k;

	ch = player[i]->channel + _channel_buffered;
	glog->log_dir[ch] = 0;

	// remove the players network-connection from the network... which network conn.
	// goes to that player... well, that's analyzed on-the-fly by the packet
	// receiving algorithm (most channels are only owned by 1 player, on 1 network
	// connection)
	k = channel_conn_recv[ ch ];
	message.print(1500, 14, "REMOVE channel[%i] conn[%i]", ch, k );
	message.animate(0);
	//if (p_local == 0) readkey();

	if (k >= 0)
		((NetLog*)glog)->rem_conn(k);

	// remove the player from the player-list, so that its data won't be used anymore...
	-- num_players;
	player[i] = player[num_players];




	message.print(1500, 14, "frame[%i] time[%i]", frame_number, game_time);
}



