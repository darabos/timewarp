
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <allegro.h>

#include "../melee.h"
REGISTER_FILE
#include "../scp.h"


#include "mmain.h"
#include "mview.h"
#include "mcontrol.h"
#include "mcbodies.h"
#include "mshppan.h"
#include "mship.h"
#include "mlog.h"
#include "mnet1.h"
#include "mitems.h"
#include "mfleet.h"


/*void Game::player_said(int who, const char *what) {
	if ((who < 0) || (who >= num_players)) tw_error ("Who said that?!?");
	message.print(8000, 15, "%s: %s", name[who], what);
	return;
	}*/
int NormalGame::add_player (Control *c, int team_index, const char *name, const char *fleet_section, const char *fleet_file) {STACKTRACE
	int i = num_players;
	num_players += 1;
	player_control = (Control**) realloc(player_control,   sizeof(Control*)   * num_players);
	player_name    =    (char**) realloc(player_name,      sizeof(char*)      * num_players);
//	player_panel = (ShipPanel**) realloc(player_panel,     sizeof(ShipPanel*) * num_players);
	player_fleet =     (Fleet**) realloc(player_fleet,     sizeof(Fleet *)    * num_players);
	player_team =    (TeamCode*) realloc(player_team,      sizeof(TeamCode)   * num_players);
	player_control[i] = c;
	add_focus(c, c->channel);
//	player_panel[i] = NULL;
	player_fleet[i] = new Fleet();
	player_fleet[i]->reset();
	player_name[i] = strdup(name);
	if (team_index >= team_table_size) {
		int i = team_table_size;
		team_table_size = team_index + 1;
		team_table = (TeamCode*) realloc(team_table, sizeof(TeamCode) * team_table_size);
		for (; i < team_table_size; i += 1) {
			if (i) team_table[i] = new_team();
			else team_table[i] = 0;
		}
	}
	if (team_index) player_team[i] = team_table[team_index];
	else player_team[i] = new_team();
	char sect[40];
	sprintf(sect, "Player%d", i+1);
	if (c->channel == channel_none) {
		tw_error("channel_none not allowed here");
		//log_file(fleet_file);
		//::fleet->load(NULL, fleet_section);
		}
	else if (log->get_direction(c->channel) & Log::direction_write) {
		player_fleet[i]->load(fleet_file, fleet_section);
		log_fleet(c->channel, player_fleet[i]);
		c->target_sign_color = ((3+i) % 7) + 1;
		}
	else {
		log_fleet(c->channel, player_fleet[i]);
		}
	set_config_file("tmp.ini");
	set_config_string(sect, "Name", name);
	set_config_string(sect, "Type", c->getTypeName());
	set_config_int(sect, "Team", team_index);
	set_config_int(sect, "Channel", c->channel);
	set_config_int(sect, "StartingFleetCost", player_fleet[i]->cost);
	set_config_int(sect, "StartingFleetSize", player_fleet[i]->size);
	player_fleet[i]->save(NULL, sect);
	player_fleet[i]->save("fleets.tmp", sect);
	return i;
	}

Planet *create_planet( Vector2 position = map_size/2 );//remove me!
void NormalGame::init_objects() {STACKTRACE
	int i;
	//add(new Stars2());
	add(new Stars());
	Planet *planet = create_planet();
	//Planet *planet = new Planet (size/2, planetSprite, random(planetSprite->frames()));
	//add (planet);
	if (view) view->camera.pos = size/2;
	add(new WedgeIndicator(planet, 75, 4));
	for (i = 0; i < num_asteroids; i += 1) add(new Asteroid());
	}

void NormalGame::init_players() {STACKTRACE
	switch (log->type) {
		case Log::log_normal: {
			for (int i = 0; true; i += 1) {
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
				add_player(create_control(channel_server, type, config), ti, name, buffy);
			}
		}
		break;
		case Log::log_net1client:
		case Log::log_net1server: {
			log_file("server.ini");
			int use_teams_menu = get_config_int("Network", "NetworkMeleeUseTeams", 0);
			//if (use_teams_menu) {
			if (1) {
				int j;
				for (j = 0; j < 2; j += 1) {
					int ch;
					if (j == 0) ch = channel_server;
					else ch = channel_client;
					if (is_local(ch)) {
						log_file("client.ini");
						int use_teams_menu = get_config_int("Network", "NetworkMeleeUseTeams", 0);
						for (int i = 0; true; i += 1) {
							char buffy[64];
							const char *simple_config = 
								"[Player1]\nType=Human\nConfig=0\nTeam=0\n";
							sprintf(buffy, "Player%d", i + 1);
							if (use_teams_menu) set_config_file("scp.ini");
							else set_config_data(simple_config, strlen(simple_config));
							const char *type = get_config_string(buffy, "Type", NULL);
							if (!type) {
								int tmp = 0;
								log_int(ch, tmp);
								break;
							}
							if (strcmp(type, "none") == 0) continue;
							const char *name = get_config_string(buffy, "Name", buffy);
							char config[64];
							sprintf(config, "Config%d", get_config_int(buffy, "Config", 0));
							int channel = channel_server;
							//if (strcmp(type, "WussieBot") == 0) channel = channel_none;
							//if (strcmp(type, "MoronBot") == 0) channel = channel_none;
							int ti = get_config_int(buffy, "Team", 0);
							{int tmp = 1; log_int(ch, tmp);}
							log_int(ch, ti);
							int name_length = strlen(name);
							log_int(ch, name_length);
							log_data(ch, (char*)name, name_length);
							add_player(create_control(ch, type, config), ti, name, buffy);
						}
					}
					else {
						for (int i = 0; true; i += 1) {
							int tmp;
							log_int(ch, tmp);
							if (tmp == 0) break;
							int team;
							char *name;
							log_int(ch, team);
							int name_length;
							log_int(ch, name_length);
							name = (char*)malloc((name_length+1)*sizeof(char));
							log_data(ch, name, name_length);
							name[name_length] = 0;
							add_player(create_control(ch, "Whatever"), team, name, NULL);
						}
					}
				}
			}
/*			else {
				set_config_file("client.ini");
				int lp = get_config_int("Network", "LocalPlayers", 1);
				int cp = lp, sp = lp;
				log_int(channel_server, sp);
				log_int(channel_client, cp);
				int i;
				char buffy1[256];
				char buffy2[256];
				char buffy3[256];
				for (i = 0; i < sp; i += 1) {
					sprintf(buffy1, "Config%d", i);
					sprintf(buffy2, "Server Player%d", i+1);
					sprintf(buffy3, "Player%d", i+1);
					add_player(create_control(channel_server, "Human", buffy1), 0, buffy2, buffy3);
				}
				for (i = 0; i < cp; i += 1) {
					sprintf(buffy1, "Config%d", i);
					sprintf(buffy2, "Client Player%d", i+1);
					sprintf(buffy3, "Player%d", sp+i+1);
					add_player(create_control(channel_client, "Human", buffy1), 0, buffy2, buffy3);
				}
			}*/
		}
		break;
	}
	return;
}

void NormalGame::set_resolution(int screen_x, int screen_y) {
	int view_x, view_y;
	view_x = screen_x;
	view_y = screen_y;
	int n, m;
	n = 99;//int(ceil(double(num_players) / int(view_y / PANEL_HEIGHT)));
	if (n) m = int(ceil(num_players / (double)n));
	else m = 0;
	//view->set_window(screen, 0, 0, view_x - PANEL_WIDTH * m, view_y);
	redraw();
	return;
	}

void NormalGame::preinit() {
	Game::preinit();
	player_control = NULL;
	player_name = NULL;
//	player_panel = NULL;
	player_fleet = NULL;

	player_team = NULL;
	team_table = NULL;
	num_kills = 0;
	kills = NULL;
	num_players = 0;
	}
void NormalGame::init(Log *_log) {STACKTRACE
	Game::init(_log);

	team_table_size = 0;

	view->window->locate(0,0,0,0,0,0.9,0,1);

	delete_file("tmp.ini");
	delete_file("fleets.tmp");
	set_config_file ("tmp.ini");
	set_config_string (NULL, "Ignorethis", "");
	if (!log->playback) init_players();
	log_file("tmp.ini");
	if (log->playback) {
		for (int i = 0; true; i += 1) {
			char buffy[64];
			sprintf(buffy, "Player%d", i + 1);
			log_file("tmp.ini");
			const char *type = get_config_string(buffy, "Type", NULL);
			if (!type) break;
			const char *name = get_config_string(buffy, "Name", buffy);
			int channel = get_config_int(buffy, "Channel", -2);
			int ti = get_config_int(buffy, "Team", 0);
			add_player(create_control(channel, type), ti, name, buffy);
			player_fleet[i]->load(NULL, buffy);
			player_fleet[i]->save("fleets.tmp", buffy);
			}
		}
/*
	int i;
	set_config_file("tmp.ini");
	int num_players        = get_config_int("Game", "NumPlayers", 0);
	for (i = 0; i < num_players; i += 1) {
		add_player();
		}

	char tmp[80];
	const char *t;

	for (i = 0; i < num_players; i += 1) {
		set_config_file("tmp.ini");
		sprintf (tmp, "Player%d", i+1);
		name[i] = strdup(get_config_string(tmp, "Name", "nameless"));
		panel[i] = NULL;
		fleet->load(NULL, tmp);
		total_fleet[i] = fleet->cost;
		slot[i] = -2;
		fleet->save("./fleets.tmp", tmp);
		if ((i < first_local_player) || (i >= first_local_player + num_local_players)) {
			control[i] = getController("VegetableBot", name[i]);
			}
		else {
			set_config_file("./tmp.ini");
			t = get_config_string(tmp, "Type", NULL);
			control[i] = getController(t, name[i]);
			if (!control[i]) tw_error("bad Controller type! (%s)", t);
			sprintf(tmp, "Config%d", get_config_int(tmp, "Config", 0));
			player_control[i]->initialize();
			player_control[i]->load("scp.ini", tmp);
			}
		}*/


	prepare();
	init_objects();

	next_choose_new_ships_time = game_time + 200;

	return;
	}

NormalGame::~NormalGame() {STACKTRACE
	if (player_control) free (player_control);
	int i;
	if (player_name) {
		for (i = 0; i < num_players; i += 1) {
			free(player_name[i]);
			}
		free(player_name);
		}
//	if (player_panel) free (player_panel);

	if (player_team) free(player_team);
	if (kills) free(kills);
	}

void NormalGame::calculate() {STACKTRACE
	Game::calculate();
	if (next_choose_new_ships_time <= game_time) {
		choose_new_ships();
		next_choose_new_ships_time = game_time + 24*60*60*1000;
	}
	return;
	}

void NormalGame::ship_died(Ship *who, SpaceLocation *source) {STACKTRACE
	int n = game_time + 4000;
	if (next_choose_new_ships_time > n) next_choose_new_ships_time = n;
	Game::ship_died(who, source);

/*	if (!(num_kills & 31)) kills = (ShipKill*) realloc(kills, (num_kills+32) * sizeof(ShipKill) );
	kills[num_kills].time = this->game_time;
	kills[num_kills].victim.ally_flag = who->ally_flag;
	kills[num_kills].victim.data = who->data;
	kills[num_kills].victim.type = who->type;
//	if (who->control) kills[num_kills].victim_player = who->control->player;
	if (source) {
		kills[num_kills].killer.ally_flag = source->ally_flag;
		kills[num_kills].killer.data = source->data;
		if (source->ship) kills[num_kills].killer.type = source->ship->type;
		else kills[num_kills].killer.type = NULL;
	}
	else {
		kills[num_kills].killer.ally_flag = -1;
		kills[num_kills].killer.data = NULL;
		kills[num_kills].killer.type = NULL;

	}
//	kills[num_kills].killer_player = get_player(source);
	num_kills += 1;

	if (who->type) {
		if (source->ship && source->ship->type) 
			message.print(2500, 7, "%s killed %s", source->ship->type->name, who->type->name);
		else message.print(2500, 7, "%s died", who->type->name);
	}

	int i;
	for (i = 0; i < num_kills; i += 1) {

	}
//*/
	return;
	}
void NormalGame::display_stats() {STACKTRACE
	pause();
	int i;
	for (i = 0; i < num_players; i += 1) {
		Fleet *fleet = player_fleet[i];
		switch (log->type) {
			case Log::log_net1client:
			case Log::log_net1server: {
//				if (log->get_direction(player_control[i]->channel) & Log::direction_write) 
					message.print(6000, 15, "%s status: : %d / ?? Ships, %d / ??? points", player_name[i], fleet->size, fleet->cost);
//				else
//					message.print(6000, 15, "%s status: : %d / %d points", buffy, fleet->cost, player_total_fleet[i]);
			}
			break;
			default: {
				message.print(6000, 15, "%s status: : %d / ?? Ships, %d / ??? points", player_name[i], fleet->size, fleet->cost);
			}
			break;
		}
	}
	unpause();
	return;
}
#include "../other/radar.h"
bool NormalGame::handle_key(int k) {STACKTRACE
	switch (k >> 8) {
		default: {
			return Game::handle_key(k);
			}
		break;
		case KEY_F5: {
			display_stats();
			return true;
			}
		break;
		case KEY_F7: {
			if (log->type == Log::log_normal) Game::handle_key(k);
			return true;
			}
		break;
		case KEY_F9: {
			if (log->type != Log::log_normal) return false;
			message.out("MUHAHAHAHAHA!!!!", 5000, 12);
			add(new Planet(random(size), planetSprite, random(planetSprite->frames())));
			return true;
			}
		break;
		}
	return false;
	}
void NormalGame::choose_new_ships() {STACKTRACE
	char tmp[40];
	int i;
	pause();
	message.out("Selecting ships...", 1000);
	int *slot = new int[num_players];
	//choose ships and send them across network
	for (i = 0; i < num_players; i += 1) {
		slot[i] = -2;
		if (player_control[i]->ship) {
			}
		else {
//			if (player_panel[i]) player_panel[i]->window->hide();
//			player_panel[i] = NULL;
			sprintf (tmp, "Player%d", i+1);
			Fleet *fleet = player_fleet[i];
			if (fleet->size == 0) continue;
			char buffy[512];
			sprintf(buffy, "%s\n%s\n%d of ??? points", player_name[i], fleet->title, fleet->cost);
			slot[i] = player_control[i]->choose_ship(window, buffy, fleet);
			if (player_control[i]->channel != channel_none) {
				slot[i] = intel_ordering(slot[i]);
				log->buffer(player_control[i]->channel, &slot[i], sizeof(int));
				log->flush();
				//slot[i] = intel_ordering(slot[i]);
				}
			}
		}
	//recieve the ships that were chosen
	log->listen();
	for (i = 0; i < num_players; i += 1) {
		if (slot[i] == -2) continue;
		if (player_control[i]->channel != channel_none) {
			log->unbuffer(player_control[i]->channel, &slot[i], sizeof(int));
			slot[i] = intel_ordering(slot[i]);
			}
		}
	//create the ships that were chosen
	for (i = 0; i < num_players; i += 1) {
		if (slot[i] == -2) continue;
		sprintf (tmp, "Player%d", i+1);
		//fleet->load("./fleets.tmp", tmp);
		Fleet *fleet = player_fleet[i];
		if (slot[i] == -1) slot[i] = random() % fleet->size;
		Ship *s = create_ship(fleet->ship[slot[i]]->id, player_control[i], random(size), random(PI2), player_team[i]);
		fleet->clear_slot(slot[i]);
		fleet->sort();
		//fleet->save("./fleets.tmp", tmp);
		s->locate();
		add ( new WedgeIndicator ( s, 30, i+1 ) );
		ShipPanel *panel = new ShipPanel(s);
		panel->window->init(window);
		panel->window->locate(
			0, 0.9,
			0, i * (100.0/480), 
			0, 0.1,
			0, (100.0/480)
			);
		add(panel);
		add(s->get_ship_phaser());
		}
	delete slot;
	message.out("Finished selecting ships...", 1500);
	unpause();
	return;
	}

REGISTER_GAME(NormalGame, "Melee")
