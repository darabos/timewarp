/*
 * Star Control - TimeWarp
 *
 * melee/mgame.cpp - Melee base game module
 *
 * 21-Jun-2002
 *
 * - Game::create_ship() function modified to load ship ini files according to shp*.*
 *   file naming convention.
 * - Cosmetic code changes.
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


static char chat_buf[256];
static int chat_len = 0;
static int chat_on = 0;

int random_seed[2];


int interpolate_frames = false;


#define HIST_POWER 4.0

#define CHECKSUM_CHANNEL Game::_channel_buffered
//#define CHECKSUM_CHANNEL 0

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

Game *GameType::new_game() {
	Game *tmp = _new_game();
	tmp->preinit();
	tmp->type = this;
	return tmp;
}

GameType *gametype (const char *name) {
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
#	ifdef LOTS_OF_CHECKSUMS
	game->compare_checksums();
#	endif
}



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
	if ((channel != -1) && !log->playback && !(log->get_direction(channel) & Log::direction_write))
		return;
	num_focuses += 1;
	focus = (Presence **) realloc(focus, sizeof(Presence *) * num_focuses);
	focus[num_focuses - 1] = new_focus;
	new_focus->attributes |= ATTRIB_FOCUS;
	if (num_focuses == 1) new_focus->attributes |= ATTRIB_ACTIVE_FOCUS;
	if (focus_index == -1) focus_index = 0;
}

void Game::add_target(SpaceObject *new_target) {
	num_targets += 1;
	target = (SpaceObject **) realloc(target, sizeof(SpaceObject *) * num_targets);
	target[num_targets - 1] = new_target;
	new_target->attributes |= ATTRIB_TARGET;
}

void Game::prepare() {
	Physics::prepare();
	::game = this;
	return;
}

void Game::set_resolution(int screen_x, int screen_y) {
	int view_x, view_y;
	view_x = screen_x;
	view_y = screen_y;
	redraw();
	return;
}

void Game::redraw() {
	if (!window->surface) return;
	scare_mouse();
	window->lock();
	rectfill(window->surface, window->x, window->y, window->x+window->w-1, window->y+window->h-1, pallete_color[8]);
	FULL_REDRAW += 1;
	view->refresh();
	view->animate(this);
	FULL_REDRAW -= 1;
	window->unlock();
	unscare_mouse();
	return;
}

Ship *Game::create_ship(const char *id, Control *c, Vector2 pos, double angle, int team) {
	ShipType *type = shiptype(id);
	if (!type)
		tw_error("Game::create_ship - bad ship id (%s)", id);
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
	add_target(s);
	s->attributes |= ATTRIB_NOTIFY_ON_DEATH;
	return s;
}

Ship *Game::create_ship(int channel, const char *id, const char *control, Vector2 pos, double angle, int team) {
	Control *c = create_control(channel, control);
	if (!c)
		tw_error("bad Control type!");
	c->temporary = true;
	Ship *s = create_ship(id, c, pos, angle, team);
	return s;
}

void Game::increase_latency() {
	if (CHECKSUM_CHANNEL) {
		log->buffer(channel_server + Game::_channel_buffered, NULL, 2);
		log->buffer(channel_client + Game::_channel_buffered, NULL, 2);
		if (log->playback) 
			log->buffer(channel_playback + Game::_channel_buffered, NULL, 1);
		log->flush();
	}
	lag_frames += 1;
}

void Game::decrease_latency() {
	if (lag_frames <= 1) tw_error("latency decreased too far");
	if (CHECKSUM_CHANNEL) {
		log->unbuffer(channel_server + Game::_channel_buffered, NULL, 2);
		log->unbuffer(channel_client + Game::_channel_buffered, NULL, 2);
		if (log->playback) 
			log->unbuffer(channel_playback + Game::_channel_buffered, NULL, 1);
	}
	lag_frames -= 1;
}

int Game::is_local (int channel) {
	return (log->get_direction (channel) & Log::direction_write);
}
void Game::log_file (const char *fname) {
	log->log_file(fname);
}

void Game::log_fleet(int channel, Fleet *fleet) {
	int fl;
	void *tmpdata = fleet->serialize(&fl);
	char buffer[16384];

	if (fl > 16000)
		tw_error("blah");
	memcpy(buffer, tmpdata, fl);
	free(tmpdata);
	log_int(channel, fl);
	if (fl > 16000)
		tw_error("blah");
	log_data(channel, buffer, fl);
	fleet->deserialize(buffer, fl);
}

Control *Game::create_control (int channel, const char *type, char *config, char *file) {
	if ((channel != channel_none) && !(log->get_direction(channel) & Log::direction_write)) {
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

void Game::log_char(int channel, char &data) {
	if (!log) return;
	log->log  (channel, &data, 1);
	return;
}

void Game::log_short(int channel, short &data) {
	if (!log) return;
	data = intel_ordering_short(data);	
	log->log  (channel, &data, sizeof(short));
	data = intel_ordering_short(data);
	return;
}

void Game::log_int(int channel, int &data) {
	if (!log) return;
	data = intel_ordering(data);
	log->log  (channel, &data, sizeof(int));
	data = intel_ordering(data);
	return;
}

void Game::log_data(int channel, void *data, int size) {
	if (!log) return;
	log->log  (channel, data, size);
	return;
}

void Game::idle(int time) {
	if (log->listen()) return;
	::idle(time);
	return;
}

void Game::animate(Frame *frame) {
	Physics::animate(frame);
}

void Game::animate() {
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
	else view->animate(this);
	t = get_time2() - t - paused_time;
	render_history->add_element(pow(t, HIST_POWER));
	return;
}

bool Game::game_ready() { 
	if (CHECKSUM_CHANNEL == 0) return 1;
	if (log->playback) {
		return (log->ready(channel_server + Game::_channel_buffered) != 0);
	}
	else switch (log->type) {
		case Log::log_normal: {
			return true;
		}
		break;
		case Log::log_net1server:
		case Log::log_net1client: {
			if (!log->ready(channel_client + Game::_channel_buffered)) return false;
			if (!log->ready(channel_server + Game::_channel_buffered)) return false;
			return true;
		}
	}
	return true;
}

void Game::handle_desynch(int local_checksum, int server_checksum, int client_checksum) {
	char buf[100000];
	char *f = buf;
	error("Game Desynchronized\nTime=%d Frame=%d\nClient=%d Server=%d Local=%d", game_time, frame_number, (int)client_checksum, (int)server_checksum, (int)local_checksum);
}

//static int old_num_items;
//static int old_rng;
//static int old_frame;
//static char old_checksum_buf[200][200];

void Game::compare_checksums() {
	unsigned char local_checksum = checksum() & 255;
	unsigned char client_checksum = local_checksum;
	unsigned char server_checksum = local_checksum;
	bool desync = false;

	log_char(channel_server + CHECKSUM_CHANNEL, server_checksum);
	if (lag_frames)
		log_char(channel_client + CHECKSUM_CHANNEL, client_checksum);

	if (server_checksum != client_checksum)
		desync = true;
	if (log->playback) {
		if (lag_frames) 
			log_char (channel_playback + CHECKSUM_CHANNEL, local_checksum);
		if (local_checksum != server_checksum) desync = true;
	}

	this->local_checksum = local_checksum;
	this->client_checksum = client_checksum;
	this->server_checksum = server_checksum;

	if (desync) handle_desynch(local_checksum, server_checksum, client_checksum);
}

void Game::do_game_events() {
	int i;

	//transmit from server
	if (log->get_direction(channel_server) & Log::direction_write) {
		COMPILE_TIME_ASSERT(sizeof(events_waiting) == sizeof(char));
		log->buffer( channel_server + _channel_buffered, &events_waiting, sizeof(events_waiting) );
		for (i = 0; i < events_waiting; i += 1) {
			log->buffer ( channel_server + _channel_buffered, waiting_events[i], waiting_events[i]->size );
		}
		//deallocate transmitted events
		for (i = 0; i < events_waiting; i += 1) free(waiting_events[i]);
		events_waiting = 0;
	}

	//transmit from client
	if (log->get_direction(channel_client) & Log::direction_write) {
		COMPILE_TIME_ASSERT(sizeof(events_waiting) == sizeof(char));
		log->buffer( channel_client + _channel_buffered, &events_waiting, sizeof(events_waiting) );
		for (i = 0; i < events_waiting; i += 1) {
			log->buffer ( channel_client + _channel_buffered, waiting_events[i], waiting_events[i]->size );
		}
		//deallocate transmitted events
		for (i = 0; i < events_waiting; i += 1) free(waiting_events[i]);
		events_waiting = 0;
	}

	//double-check transmission
	if (events_waiting) {
		tw_error("Game::do_game_events - events weren't sent properly");
		for (i = 0; i < events_waiting; i += 1) free(waiting_events[i]);
		events_waiting = 0;
	}

	//recieve
	char ne;
	COMPILE_TIME_ASSERT(sizeof(events_waiting) == sizeof(ne));
	char buffy[1024];

	//recieve from server
	log->unbuffer(channel_server + _channel_buffered, &ne, sizeof(ne));
	for (i = 0; i < ne; i += 1) {
		char *tmp = buffy;
		log->unbuffer(channel_server + _channel_buffered, &buffy, sizeof(GameEvent));
		int s = ((GameEvent*)tmp)->size;
		if (s > 1024) {
			tmp = (char *)malloc(s);
			memcpy(tmp, buffy, sizeof(GameEvent));
		}
		log->unbuffer(channel_server + _channel_buffered, tmp + sizeof(GameEvent), s - sizeof(GameEvent));
		handle_game_event ( channel_server, ((GameEvent*)tmp));
		if (tmp != buffy) free(tmp);
	}


	//recieve from client
	log->unbuffer(channel_client + _channel_buffered, &ne, sizeof(ne));
	for (i = 0; i < ne; i += 1) {
		char *tmp = buffy;
		log->unbuffer(channel_client + _channel_buffered, &buffy, sizeof(GameEvent));
		int s = ((GameEvent*)tmp)->size;
		if (s > 1024) {
			tmp = (char *)malloc(s);
			memcpy(tmp, buffy, sizeof(GameEvent));
		}
		log->unbuffer(channel_client + _channel_buffered, tmp + sizeof(GameEvent), s - sizeof(GameEvent));
		handle_game_event ( channel_client, ((GameEvent*)tmp));
		if (tmp != buffy) free(tmp);
	}
}

void Game::handle_game_event ( int source, class GameEvent *event ) {
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

void Game::send_game_event ( class GameEvent *event ) {
	if (events_waiting == maximum_events_waiting) {
		tw_error("too many GameEvents");
		return;
	}
	if (!waiting_events) waiting_events = new GameEvent*[maximum_events_waiting];
	waiting_events[events_waiting] = event;
	events_waiting += 1;
}


void Game::calculate() {
	int i;
	double t = get_time2();
	int active_focus_destroyed = false;


	paused_time = 0;
	compare_checksums();
	do_game_events();

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
	for (i = 0; i < num_targets; i += 1) {
		if (!target[i]->exists()) {
			num_targets -= 1;
			target[i] = target[num_targets];
			i -= 1;
		}
	}


	Physics::calculate();


	view->calculate(this);


	t = get_time2() - t - paused_time;
	tic_history->add_element(pow(t, HIST_POWER));
	return;
}

void Game::play() {
	set_resolution(window->w, window->h);
	if (is_paused()) unpause();
	try {
		while(!game_done) {
			unsigned int time = get_time();
			poll_input();
			if(!sound.is_music_playing()) {
				play_music();
			}
			if ((next_tic_time <= time) && (next_render_time > game_time) && game_ready()) {
				calculate();
				if (auto_unload) unload_unused_ship_data();//expiremental
				log->flush();
				log->listen();
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
			}
			else {
				animate();
				next_render_time = game_time + msecs_per_render;
			}
			while (keypressed())
				handle_key(readkey());
		}
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
//ArrayIndexOutOfBounds NullPointerException
/*	catch (exception &e) {
		if (__error_flag & 1) throw;
		caught_error ("A predefined exception occured\n%s", e.what());
		if (__error_flag & 1) throw;
		}*/
	catch (...) {
		if (__error_flag & 1) throw;
		caught_error("Ack(1)!!!\nAn error occured in the game!\nBut I don't know what error!");
		if (__error_flag & 1) throw;
	}
	return;
}

void Game::ship_died(Ship *who, SpaceLocation *source) {
	if (source && source->data) {
		Music *tmp = NULL;
		if (source && source->ship && source->ship->data) tmp = source->ship->data->moduleVictory;
		if (tmp) sound.play_music(tmp);
	}
	return;
}

//#include "mcbodies.h"
void Game::fps() {
	if ((!log->playback) && ((log->type == Log::log_net1server) || (log->type == Log::log_net1client))) {
		int ping = ((NetLog*)log)->ping;
		char *tt = "good";
		if (ping > 100) tt = "okay";
		if (ping > 200) tt = "bad";
		if (ping > 400) tt = "BAD!";
		if (ping > 800) tt = "VERY BAD!";
		message.print(msecs_per_fps, 12, "ping: %dms (that's %s)", ping, tt);
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
		message.print(msecs_per_fps, 12, "tic time: %.3fms (that's %s)", tt, tmp);

		if (rt < 2)
			tmp = "good";
		else if (rt < 20)
			tmp = "ok";
		else if (rt < 50)
			tmp = "bad";
		else
			tmp = "BAD!";
		message.print(msecs_per_fps, 12, "render time: %.3fms (that's %s)", rt, tmp);
		message.print(msecs_per_fps, 12, "debug: %d", debug_value);
		message.print(msecs_per_fps, 12, "shipdatas loaded: %d", shipdatas_loaded);
	}

	if (chat_on)
		message.print(msecs_per_fps, 15, "say: %s", chat_buf);
}

void Game::preinit() {
	Physics::preinit();
	planetSprite = asteroidSprite = asteroidExplosionSprite = hotspotSprite = kaboomSprite = panelSprite = sparkSprite = xpl1Sprite = NULL;
	planet_victory = NULL;
	log = NULL;
	tic_history = render_history = NULL;

	events_waiting = 0;
	waiting_events = NULL;
	num_focuses = 0;
	focus_index = 0;
	focus = NULL;
	num_targets = 0;
	target = NULL;
	view = NULL;
	window = NULL;
	music = NULL;
}

void Game::init(Log *_log) {
	int i;

	game_done = false;
	log = _log;
	if (!log) {
		log = new Log();
		log->init();
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

	if (!log->playback) {
		switch (log->type) {
			case Log::log_normal: {
			}
			break;
			case Log::log_net1server: {

/*				log->set_all_directions(Log::direction_write | Log::direction_read | NetLog::direction_immediate);
				log->set_direction(channel_client , Log::direction_read);
				log->set_direction(channel_client + Game::_channel_buffered, Log::direction_read);
				log->set_direction(channel_server + Game::_channel_buffered, Log::direction_write | Log::direction_read);
*/
			#ifdef REVERSE_CONNECT
/*				set_config_file("client.ini");
				char address[128];
				while (!((NetLog*)log)->net.isConnected()) {
					strncpy(address, get_config_string("Network", "Address", ""), 127);
					int port = get_config_int("Network", "Port", 15515);
					char *addressaddress = address;
					if (connect_menu(&addressaddress, &port)) {
						game->quit("Quit - aborted connection");
						break;
					}
					set_config_string("Network", "Address", addressaddress);
					message.out("...");
					i = ((NetLog*)log)->net.connect(addressaddress, port, is_escape_pressed);
					free(addressaddress);
					if (i) {
						while (keypressed()) readkey();
						tw_error("connection failed");
					}
				}*/
			#else
/*				set_config_file("client.ini");
				int port = get_config_int("Network", "Port", 15515);
				while (!((NetLog*)log)->net.isConnected()) {
					port = listen_menu(window, port);
					message.out("...");
					((NetLog*)log)->net.listen(port, is_escape_pressed);
				}*/
			#endif
/*				((NetLog*)log)->net.optimize4latency();
				message.out("connection established");*/
			}
			break;
			case Log::log_net1client: {
/*				log->set_all_directions(Log::direction_read);
				log->set_direction(Game::channel_client , Log::direction_write | Log::direction_read | NetLog::direction_immediate);
				log->set_direction(Game::channel_client + Game::_channel_buffered, Log::direction_write | Log::direction_read);
*/
			#ifndef REVERSE_CONNECT
/*				set_config_file("client.ini");
				char address[128];
				while (!((NetLog*)log)->net.isConnected()) {
					strncpy(address, get_config_string("Network", "Address", ""), 127);
					int port = get_config_int("Network", "Port", 15515);
					char *addressaddress = address;
					if (connect_menu(window, &addressaddress, &port)) {
						game->quit("Quit - aborted connection(2)");
						break;
					}
					set_config_string("Network", "Address", addressaddress);
					message.out("...");
					i = ((NetLog*)log)->net.connect(addressaddress, port, is_escape_pressed);
					free(addressaddress);
					if (i) {
//						while (is_escape_pressed());
						while (keypressed()) readkey();
						tw_error("connection failed");
					}
				}*/
			#else
/*				set_config_file("client.ini");
				int port = get_config_int("Network", "Port", 15515);
				while (!((NetLog*)log)->net.isConnected()) {
					port = listen_menu(port);
					message.out("...");
					((NetLog*)log)->net.listen(port, is_escape_pressed);
				}*/
			#endif
/*				((NetLog*)log)->net.optimize4latency();
				message.out("connection established");*/
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
	int tmp = log->type;
	log_int(channel_init, tmp);
	if (log->playback) log->type = tmp;

	char buffy[128];
	i = strlen(type->name);
	memcpy(buffy, type->name, i);
	if (i > 127) tw_error("long gamename1");
	log_int (channel_init, i);
	if (i > 127) tw_error("long gamename2");
	log_data(channel_init, buffy, i);
	buffy[i] = 0;
	if (strcmp(buffy, type->name)) tw_error("wrong game type");

	i = rand();
//	i = 9223;
	log_int(channel_server, i);
	random_seed[0] = i;
	tw_random_seed(i);
	i = rand();
//	i = 7386;
	log_int(channel_server, i);
	random_seed[1] = i;
	tw_random_seed_more(i);

	if (!is_paused()) pause();

	text_mode(-1);
	if (!melee) melee = load_datafile("melee.dat");
	if (!melee) tw_error("Error loading melee data\n");

	panelSprite             = new SpaceSprite(&melee[MELEE_PANEL], PANEL_FRAMES, SpaceSprite::IRREGULAR);
	kaboomSprite            = new SpaceSprite(&melee[MELEE_KABOOM], KABOOM_FRAMES);
	hotspotSprite           = new SpaceSprite(&melee[MELEE_HOTSPOT], HOTSPOT_FRAMES);
	sparkSprite             = new SpaceSprite(&melee[MELEE_SPARK], SPARK_FRAMES);
	asteroidExplosionSprite = new SpaceSprite(&melee[MELEE_ASTEROIDEXPLOSION], ASTEROIDEXPLOSION_FRAMES);
	asteroidSprite          = new SpaceSprite(&melee[MELEE_ASTEROID], ASTEROID_FRAMES);
	planetSprite            = new SpaceSprite(&melee[MELEE_PLANET], PLANET_FRAMES);
	xpl1Sprite              = new SpaceSprite(&melee[MELEE_XPL1], XPL1_FRAMES);
	planet_victory = (Music*) (melee[MELEE_PLANET+PLANET_FRAMES].dat);

	set_config_file("client.ini");
	msecs_per_fps = get_config_int("View", "FPS_Time", 200);
	msecs_per_render = (int)(1000. / get_config_float("View", "MinimumFrameRate", 10) + 0.5);
	prediction = get_config_int("Network", "Prediction", 50);
	if ((prediction < 0) || (prediction > 100)) tw_error ("Prediction out of bounds (0 < %d < 100)", prediction);

	log_file("server.ini");
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

	init_lag();
	log_int(channel_server, lag_frames);
	log_int(channel_init, lag_frames);

	tic_history = new Histograph(16);
	render_history = new Histograph(16);

	prepare();

	return;
}

void Game::init_lag() {
	if ((log->type == Log::log_net1server) || (log->type == Log::log_net1client)) {
		int lag_time = 0;//get_config_int("Network", "Lag", 200);
		char blah = 0;
		log_char(channel_server, blah);
		log->flush();
		log_char(channel_client, blah);
		log->flush();
		log_char(channel_server, blah);
		log->flush();
		log_char(channel_client, blah);
		log->flush();
		log_char(channel_server, blah);
		log->flush();
		lag_time = ((NetLog*)log)->ping;
		log_int(channel_server, lag_time);
		int lag_frames = (int) (1.5 + lag_time * normal_turbo / (double) frame_time );
#		ifdef _DEBUG
//			lag_frames += 15;
#		endif
//			lag_frames += 4;
		message.print(15000, 15, "target ping set to: %d ms (pessimistically: %d ms)", lag_time, iround(lag_frames * frame_time / normal_turbo));
		for (int i = 0; i < lag_frames; i += 1)
			increase_latency();
	}
	else {
/*		int lag_frames = 10;//0;
		for (int i = 0; i < lag_frames; i += 1)
			increase_latency();//*/
	}
}

void Game::change_view(View *new_view) {//this function looks wrong to me
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

void Game::change_view(const char * name) {
	View *v = get_view(name, view);
	if (!v)
		tw_error("Game::change_view - invalid view name");
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

Game::~Game() {
	message.out("deleteing GameEvents");
	int i;
	for (i = 0; i < events_waiting; i += 1) free(waiting_events[i]);
	delete waiting_events;

	message.out("deleteing histographs");
	delete tic_history; tic_history = NULL;
	delete render_history; render_history = NULL;
	
	message.out("deleteing misc. sprites");

	delete xpl1Sprite;
	delete panelSprite;
	delete kaboomSprite;
	delete hotspotSprite;
	delete sparkSprite;
	delete asteroidExplosionSprite;
	delete asteroidSprite;
	delete planetSprite;

	xpl1Sprite = NULL;
	panelSprite = NULL;
	kaboomSprite = NULL;
	hotspotSprite = NULL;
	sparkSprite = NULL;
	asteroidExplosionSprite = NULL;
	asteroidSprite = NULL;
	planetSprite = NULL;

	//planet_victory = (Music*) (melee[MELEE_PLANET+PLANET_FRAMES].dat);

	if (music && (music != (Music*)-1)) {
		sound.stop_music();
		sound.unload_music(music);
	}

	message.out("deleteing data file");
	unload_datafile(melee); melee = NULL;

	message.out("other shit");
	message.flush();
	delete view;
	delete window;
}

bool Game::is_paused() {
	if (time_paused != -1) return true;
	return false;
}

void Game::pause() {
	if (time_paused != -1) tw_error ("can't pause -- already paused");
	time_paused = get_time();
}

void Game::unpause() {
	if (time_paused == -1) tw_error ("can't unpause -- not paused");
	redraw();
	paused_time += get_time() - time_paused;
	time_paused = -1;
	return;
}

void Game::save_screenshot() {
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

bool Game::handle_key(int k) {
	switch (k >> 8) {

		case KEY_1: {
			interpolate_frames = !interpolate_frames;
			return true;
		}
		break;
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
			if (tw_alert("Game is paused", "&Abort game", "&Resume playing") == 1) {
				game->quit("quit - Game aborted from keyboard");
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
					send_game_event(new GameEventMessage(chat_buf));
//					player_said(0, chat_buf);
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

int Game::set_frame_time(int t) {
	this->frame_time = t;
	prepare();
	return 1;
}

int Game::set_turbo(double t) {
	this->normal_turbo = t;
	prepare();
	return 1;
}

double Game::get_turbo() {
	return this->normal_turbo;
}

void Game::play_music() {
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

