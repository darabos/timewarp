
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <allegro.h>

#include "../melee.h"
REGISTER_FILE
#include "../scp.h"
#include "../frame.h"

#include "mgame.h"

#include "mmain.h"
#include "mview.h"
#include "mcontrol.h"
#include "mcbodies.h"
#include "mshppan.h"
#include "mship.h"
#include "mlog.h"
//#include "mnet1.h"
#include "mitems.h"
#include "mfleet.h"

#include "../games/gflmelee.h"
//#include "../other/radar.h"



PlayerInformation *NormalGame::new_player()
{
	return new NPI();
}

/*void Game::player_said(int who, const char *what) {
	if ((who < 0) || (who >= num_players)) tw_error ("Who said that?!?");
	message.print(8000, 15, "%s: %s", name[who], what);
	return;
	}*/
int NormalGame::add_player (int num, Control *c, int team_index, const char *name, const char *fleet_section,
							const char *fleet_file)
{
	STACKTRACE;

	NPI *p = (NPI*) new_player();
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
	
	p->fleet = new Fleet();
	p->fleet->reset();
	p->color = num + 1;
	strncpy(p->name, name, sizeof(p->name));
	strdup(p->name);

	if (team_index >= team_table_size) {
		int i = team_table_size;
		team_table_size = team_index + 1;
		team_table = (TeamCode*) realloc(team_table, sizeof(TeamCode) * team_table_size);
		for (; i < team_table_size; i += 1) {
			if (i) team_table[i] = new_team();
			else team_table[i] = 0;
		}
	}
	
	if (team_index) p->team = team_table[team_index];
	else p->team = new_team();

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

	set_config_file("tmp.ini");
	set_config_string(sect, "Name", name);
	set_config_string(sect, "Type", c->getTypeName());
	set_config_int(sect, "Team", team_index);
	set_config_int(sect, "Channel", c->channel);
	set_config_int(sect, "StartingFleetCost", p->fleet->getCost());
	set_config_int(sect, "StartingFleetSize", p->fleet->getSize());
	p->fleet->save(NULL, sect);
	p->fleet->save("fleets.tmp", sect);
	return num;
	}

void NormalGame::init_objects()
{
	STACKTRACE;

	int i;
	//add(new Stars2());
	add(new Stars());
	Planet *planet = create_planet();
	//Planet *planet = new Planet (size/2, planetSprite, random(planetSprite->frames()));
	//add (planet);
	if (view) view->camera.pos = size/2;
	add(new WedgeIndicator(planet, 75, 4));
	for (i = 0; i < num_asteroids; i += 1)
		add(new Asteroid());

}

void NormalGame::init_players()
{
	STACKTRACE;


	switch (glog->type)
	{
		case Log::log_normal:
			{
			int i;
			int count = 0;

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
				add_player(count, create_control(channel_server, type, config), ti, name, buffy);

				// you have to override the default settings, so that all multi-player aimed stuff
				// is channeled into a single player channel, which is handled locally.
				++count;
			}

			num_players = count;
			num_network = 1;		// --> you should have just one human CHANNEL (note that there can be more humans, sharing this channel)
			num_bots = 0;
		}
		break;
		case Log::log_net1client:
		case Log::log_net1server: {

			//log_file("server.ini");
			// isn't needed yet ?!

			log_test("mmain(a)");

			int count = 0;

			int j;
			for (j = 0; j < num_network; j += 1)
			{
				int ch;
				ch = channel_network[j];

				int team;
				char buffy[64];
				char config[64];
				char name[512];

				int iseat;
				for ( iseat = 0; iseat < num_hotseats[j]; ++iseat )
				{
					if (count >= num_players)
						break;

					if (iseat > 3)
						break;		// there are at most 4 hotseat-players allowed.
					
					int name_length = -1;

					if (is_local(ch))
					{	// put no networking stuff in this block
						// if you're local, you've to define stuff, of course...
						
						set_config_file("scp.ini");//each side determines whether they are using manually specified teams
						// these are local settings.
						
						sprintf(name, "player__%i", count+1);
						name_length = strlen(name)+1;
						
						sprintf(buffy, "Player%i", iseat+1);
						sprintf(config, "Config%d", iseat);
						
						team = get_config_int(buffy, "Team", 0);
					}

					//int tmp = 1;
					//log_int(ch, tmp);	// this is only a signal for a variable # of bots.
					// which we shouldn't use, cause channel_none players should be treated in
					// another loop, imo ...
					
					channel_current = ch;
					
					log_int(team);						// this uses channel_current
					
					log_int(name_length);				// this uses channel_current
					log_data(name, name_length);		// this uses channel_current

					message.print(1500, 15, "[%i] [%i] [%s]",
						team, name_length, name);
					message.animate(0);

					if (is_local(ch))
					{
						add_player(count, create_control(ch, "Human", config), team, name, buffy);
						
					} else {

						add_player(count, create_control(ch, "Whatever"), team, name, NULL);	
					}

					++count;
				}
				
			}


			/*
			// give overview about the players?
			message.print(1500, 14, "local channel = [%i]", channel_local());
			for (j = 0; j < num_network; j += 1)
			{
				message.print(1500, 14, "[%s] [%s] [%i]", player_name[j], player_control[j]->getTypeName(), player_control[j]->channel);
				message.animate(0);
			}
			*/

			// adding bots
			int team = new_team();
			for (j = 0; j < num_bots; j += 1)
			{
				if (count >= num_players)
					break;

				char name[512], fleetid[512];
				sprintf(name, "Bot_%i", j+1);
				sprintf(fleetid, "Player%i", j+1);

				int ch;
				ch = channel_none;
				add_player(count, create_control(ch, "WussieBot"), team, name, fleetid);

				++count;
			}

			// so, the total number of controls in use by human or bot players is ...
			if (num_players != count)
				tw_error("count mismatch, should not occur");
			


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
	//player_control = NULL;
	//player_name = NULL;
//	player_panel = NULL;
	//player_fleet = NULL;
	//player_color = NULL;

	//player_team = NULL;
	team_table = NULL;
	num_kills = 0;
	kills = NULL;
	//num_players = 0;
	}



void NormalGame::init(Log *_log)
{
	STACKTRACE;
	
//	display_channel_info();


	log_debug("normalgame::init calling game::init\n");
	Game::init(_log);

	team_table_size = 0;

	view->window->locate(0,0,0,0,0,0.9,0,1);

	delete_file("tmp.ini");
	delete_file("fleets.tmp");
	set_config_file ("tmp.ini");
	set_config_string (NULL, "Ignorethis", "");

	log_debug("normalgame::init init players\n");
	if (!glog->playback)
		init_players();

	log_debug("normalgame::init sharing tmp.ini\n");
	log_file("tmp.ini");


	if (glog->playback)
	{
		for (int i = 0; true; i += 1)
		{
			char buffy[64];
			sprintf(buffy, "Player%d", i + 1);
			log_file("tmp.ini");

			const char *type = get_config_string(buffy, "Type", NULL);
			if (!type) break;

			const char *name = get_config_string(buffy, "Name", buffy);
			int channel = get_config_int(buffy, "Channel", -2);
			int ti = get_config_int(buffy, "Team", 0);

			add_player(i, create_control(channel, type), ti, name, buffy);

			NPI* p = (NPI*)player[i];
			p->fleet->load(NULL, buffy);
			p->fleet->save("fleets.tmp", buffy);
		}
	}

	log_debug("normalgame::init prepare\n");
	prepare();
	init_objects();


	next_choose_new_ships_time = game_time + 200;

	// team and health indicators.
	indteamtoggle = 0;
	indhealthtoggle = 0;

	log_debug("normalgame::init done\n");

	return;
	}

NormalGame::~NormalGame()
{
	STACKTRACE;

	//if (player_control) free (player_control);
//	int i;
	//if (player_name) {
	//	for (i = 0; i < num_players; i += 1) {
	//		free(player_name[i]);
	//		}
	//	free(player_name);
	//	}
//	if (player_panel) free (player_panel);

	//if (player_team) free(player_team);
	if (kills) free(kills);
	}

static int kill_all_delay_counter = 0;
void NormalGame::calculate()
{
	STACKTRACE;

	Game::calculate();

//	test_net();

	if (next_choose_new_ships_time <= game_time)
	{
		if (check_end())
		{
			handle_end();
			//return;
		}
		else
		{
			choose_new_ships();
			next_choose_new_ships_time = game_time + 24*60*60*1000;
		}
		
	}

	// specially for play-testers:
	// kill all ships and ship-objects in the melee-game
	if (kill_all_delay_counter > 0)
	{
		kill_all_delay_counter -= frame_time;
	} else {

		if (key[KEY_LCONTROL] && key[KEY_ALT] && key[KEY_K])
		{
			kill_all_delay_counter += 1000;	// 1 second delay

			int i;
			for ( i = 0; i < physics->num_items; ++i )
			{
				SpaceLocation *o;
				o = physics->item[i];
				if (!(o && o->exists()))
					continue;
				if (o->isPlanet() || o->isAsteroid())
					continue;
				o->die();
			}
		}
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
void NormalGame::display_stats()
{
#ifdef _DEBUG
	STACKTRACE;

	pause();
	int i;
	for (i = 0; i < num_players; i += 1)
	{
		NPI* p = (NPI*)player[i];

		Fleet *fleet = p->fleet;
		switch (glog->type) {
			case Log::log_net1client:
			case Log::log_net1server: {
//				if (log->get_direction(player_control[i]->channel) & Log::direction_write) 
					message.print(6000, 15, "%s status: : %d / ?? Ships, %d / ??? points", p->name, fleet->getSize(), fleet->getCost());
//				else
//					message.print(6000, 15, "%s status: : %d / %d points", buffy, fleet->cost, player_total_fleet[i]);
			}
			break;
			default: {
				message.print(6000, 15, "%s status: : %d / ?? Ships, %d / ??? points", p->name, fleet->getSize(), fleet->getCost());
			}
			break;
		}
	}
	unpause();
#endif
}
bool NormalGame::handle_key(int k)
{
	STACKTRACE;
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
			if (glog->type == Log::log_normal) Game::handle_key(k);
			return true;
			}
		break;
		case KEY_F9: {
			if (glog->type != Log::log_normal) return false;
			message.out("MUHAHAHAHAHA!!!!", 5000, 12);
			add(new Planet(random(size), meleedata.planetSprite, random(meleedata.planetSprite->frames())));
			return true;
			}
		break;
		//don't use hardwired normal keys
		case KEY_H:
			if ((k & 255) == 'H'-'A'+1) indhealthtoggle = ~indhealthtoggle;
			break;
		case KEY_T:
			if ((k & 255) == 'T'-'A'+1) indteamtoggle = ~indteamtoggle;
			break;
		}
	return false;
	}



// added ROB
class TeamIndicator : public Presence
{
public:
	int		*indtoggle;
	Ship	*mother;
	TeamIndicator(Ship *creator, int *toggle);

	virtual void calculate();
	virtual void animate(Frame *space);
	virtual void animate_predict(Frame *frame, int time);
};



// NOTENOTENOTE
// if there are >1 clients, then the server will send data to all clients...
// so...
// the clients who don't need it, should discard the data,
// or..
// should you use a specific channel perhaps ?? No - just a specific socket !!
// so, you should connect to a specific socket - and you've to make sure, that
// the remote socket you connect to, is synched for these data (ie. isn't sending
// other crap ;) )

void NormalGame::download_file(char *filename)
{
	FILE *f;
	int L;
	const int N = 1024;
	char fdata[N];

	int count = 0;

	// first you've to check if you're synched
	int k = 0;
	log_int(k, channel_server);
	// this is probably enough --> the server has sent all data to you.
	// there's nothing you've to send to the server, so it's ok.

	// also, check which socket-connection you should use...

	if (p_local == 0)	// the host
	{
		// how does the server determine, who needs the data ?
		// each should send a value to the server, telling whether
		// the data are ok, or not ...

		// duh, this is getting complicated 
		
		f = fopen(filename, "rb");

		for (;;)
		{
			L = fread(fdata, N, 1, f);

			log_int(L);

			if (L == 0)
				break;
			else
				// this is wrong
				log_data(fdata, L, channel_file_data);
	//		glog->net.conn[..]->send(); <-- should be something like this

			count += L;
			message.print(1500, 15, "sent %i bytes", count);
			message.animate(0);
		}
	}
	else 
	{
		f = fopen(filename, "wb");

		for (;;)
		{
			log_int(L);

			if (L == 0)
				break;
			else
				// this is wrong
				log_data(fdata, L, channel_file_data);

			fwrite(fdata, L, 1, f);

			count += L;
			message.print(1500, 15, "received %i bytes", count);
			message.animate(0);
		}
	}

	fclose(f);

}


void NormalGame::check_file(const char *id)
{
	ShipType *type = shiptype(id);

	if (type->data->islocked())
		return;		// it is already loaded, doesn't make sense to compare anymore

	
	int myfsize, otherfsize;
	myfsize = file_size(type->data->file);
	otherfsize = myfsize;

	// compare to the value on the host computer
	// (assumption here is, that the host has the most recent version)

	log_int(otherfsize, channel_file_data);
	
	// so ... if your local version differes from the host version, then ...
	if (otherfsize != myfsize)
	{
	//	int i = tw_alert("File mismatch; download file from server?", "&Ok", "&Abort");
	//	if ( i == 2)
	//	{
			tw_error("DAT files have different size! This may cause a desynch");
	//	} else {
	//		download_file(type->data->file);
	//	}
		
	}
}


void NormalGame::choose_new_ships()
{
	STACKTRACE;

	char tmp[40];
	int i;
	pause();

	
	message.out("Selecting ships...", 1000);
	int *slot = new int [num_players];
	//choose ships and send them across network
	
//	log_test();

	for (i = 0; i < num_players; i += 1)
	{
		NPI* p = (NPI*)player[i];

		slot[i] = -2;
		if (p->control->ship) {
			}
		else {
//			if (player_panel[i]) player_panel[i]->window->hide();
//			player_panel[i] = NULL;
			sprintf (tmp, "Player%d", i+1);
			Fleet *fleet = p->fleet;
			if (fleet->getSize() == 0) continue;
			char buffy[512];

            if (strlen(fleet->getTitle()) != 0) 
                sprintf(buffy, "%s\n%s\n", p->name, fleet->getTitle());
            else
                sprintf(buffy, "%s\n", p->name);

			// humans can choose ships
			if (p->channel != channel_none)
				slot[i] = p->control->choose_ship(window, buffy, fleet);
			else
				// bots choose "randomly" (but synched randomly so don't use rand() !!).
				// this is needed, otherwise a bot would need its own channel, while this way,
				// they can use channel_none.
				slot[i] = random(fleet->getSize());

			share(i, &slot[i]);
			}
		}

	//recieve the ships that were chosen

	share_update();

//	log_test();

	
	/*
	// check which ships are chosen
	for ( i = 0; i < num_players; ++i )
	{
		message.print(1500, 15,
			"P [%i] slot [%i] size [%i]", i, slot[i], player_fleet[i]->getSize());
		message.animate(0);
	}
	readkey();
	*/

	//create the ships that were chosen
	for (i = 0; i < num_players; i += 1)
	{
		NPI* p = (NPI*)player[i];

		if (slot[i] == -2) continue;
		sprintf (tmp, "Player%d", i+1);
		//fleet->load("./fleets.tmp", tmp);
		Fleet *fleet = p->fleet;

		if (slot[i] == -1)
			slot[i] = random() % fleet->getSize();

		if (slot[i] < 0 || slot[i] >= fleet->getSize())
		{
			tw_error("trying to load invalid ship");
		}


		check_file(fleet->getShipType(slot[i])->id);
		

		Ship *s = create_ship(fleet->getShipType(slot[i])->id, p->control, random(size), random(PI2), p->team);
		if (!s) {tw_error("unable to create ship");}
		fleet->clear_slot(slot[i]);
		fleet->Sort();
		//fleet->save("./fleets.tmp", tmp);
		s->locate();
		add ( new WedgeIndicator ( s, 30, p->color ) );
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

		// add a healthbar for the ship, and also a team indicator.
		add(new HealthBar(s, &indhealthtoggle));
		add(new TeamIndicator(s, &indteamtoggle));

		
	}

//	log_test();

	//delete[] slot;
	message.out("Finished selecting ships...", 1500);
	unpause();

	// check if the file-share-channel is still "ök"
	if (log_size_ch(channel_file_data) != 0)
		tw_error("The file channel should be empty. Contains %i chars", log_size_ch(channel_current));

	delete[] slot;

	return;
}



// this should be places elsewhere I think ...
TeamIndicator::TeamIndicator(Ship *s, int *atoggle)
{
	indtoggle = atoggle;
	mother = s;
}

void TeamIndicator::calculate()
{
	if ( !(mother && mother->exists()) )
	{
		mother = 0;
		state = 0;
		return;
	}
}

void TeamIndicator::animate(Frame *space) {
	animate_predict(space, 0);
}
void TeamIndicator::animate_predict(Frame *space, int time)
{
	if (!*indtoggle)
		return;

	if (mother->isInvisible())
		return;

	Vector2i co1, co2;

	co1 = corner(mother->pos - 0.5 * mother->size).round();
	co2 = corner(mother->pos + 0.5 * mother->size).round();
	//co2 = corner(mother->pos + Vector2(d2, -mother->size.y * 0.6));

	if (co2.x < 0) return;
	if (co2.y < 0) return;
	if (co1.x >= space->surface->w) return;
	if (co1.y >= space->surface->h) return;

	int col;
	col = palette_color[mother->get_team() + 1];	// team 0 is black ...

	rect(space->surface, co1.x, co1.y, co2.x, co2.y, col);
	space->add_box(co1.x, co1.y, co2.x, co2.y);
}








bool NormalGame::player_isalive(int i)
{
	NPI* p = (NPI*)player[i];
	return (p->control->ship || p->fleet->getSize() > 0);
}

bool NormalGame::check_end()
{
	int i;
	int numalive = 0;

	for ( i = 0; i < num_players; ++i )
	{
		if (player_isalive(i))
			++ numalive;
	}

	if (numalive <= 1)
		return true;	// game should end, if at most 1 player is left (be it human or AI).
	else
		return false;
}


int NormalGame::local_player()
{
	int i;
	for ( i = 0; i < num_players; ++i )
		if (is_local(player[i]->channel))
			return i;
	return -1;
}




void NormalGame::handle_end()
{
	// pause all players, so that networking sends are only within this routine !!
	pause();

	int i;
	// I guess, you've to make sure, the log-buffer is empty, before writing/reading
	// values to/from it ..
	
//	log_test();

	// I suppose, each player is making/ has made such a choice ...
	int choices[32];
	int k;
	int local_choice = 0;	// only 1 local player needs to make a choice; the rest can be copied for local players
	for ( k = 0; k < num_players; ++k )
	{
		// zero indicates an invalid choice (note that valid choices as 1 and 2 --> see the tw_alert menu below
		int ichoice = 0;

		if (is_local(player[k]->channel))
		{
			// note, that a "local" player can also be an AI ship, but that doesn't matter,
			// what matters is that all players on one computer (AI or human) submit the
			// same choice.
			if (!local_choice)
			{
				char *endmessage;
				endmessage = "The End";
				
				char tmp[512];
				sprintf(tmp, "%s  num = %i  channel = %i framenum = %i", endmessage, k, player[k]->channel, frame_number);
				
				ichoice = tw_alert(tmp, "&QUIT", "&RESTART");
				
				local_choice = ichoice;

			} else {
				// copy the local choice
				ichoice = local_choice;
			}
		}
		
		//log_int(player_control[k]->channel, ichoice);
		
		choices[k] = ichoice;

		// networking
		share(k, &choices[k]);
	}


	// networking
	share_update();


//	log_test();

	// extra test, check if all choices are "valid" (ie non-zero)
	for ( i = 0; i < num_players; ++i )
	{
		if (!choices[i])
			tw_error("an invalid choice was encountered at end of the game");
	}


	// check if all the players want to restart, or quit, if not, give a warning and quit
	k = 0;
	for ( i = 0; i < num_players; ++i )
		k += choices[i];

	// disagreement
	if ( k != 1*num_players && k != 2*num_players )
	{
		// give a warning to the local player
		if (choices[ local_player() ] == 2)
			tw_alert("Someone aborted", "&Continue");

		quit("none");		// THIS IS DESYNCHED ?!
	}
	// agreement on quitting
	else if ( k == 1*num_players )
	{
		quit("none");		// THIS IS DESYNCHED ?!
	}
	// agreement on restarting
	else
	{
		char buffy[512];
		for ( i = 0; i < num_players; ++i )
		{
			NPI* p = (NPI*)player[i];

			// first, kill remaining ship(s)
			if (p->control->ship)
				p->control->ship->die();

			// then, load the fleet
			log_file("tmp.ini");
			sprintf(buffy, "Player%d", i + 1);
			
			p->fleet->reset();
			p->fleet->load(NULL, buffy);
			p->fleet->save("fleets.tmp", buffy);
		}
	}

	// perhaps something goes wrong here, when re-loading the ships ??
//	log_test();
	

	unpause();
}


/** \brief Checks where the stacked data in the log buffer is cleared in-between
different network actions. This only applies to the "unbuffered channels", which
have routines that synchronize the two games explicitly in the code (ie synchronized
halts). This buffer should be empty most of the time, except in special situations.
*/
void log_test(char *comment)
{
#ifdef _DEBUG
	// synch the 2 games, which should assure that all data are read/written in the meantime
	int k, m;
	k = random();
	m = k;
	
	int i;
	for ( i = 0; i < num_network; ++i )
	{
		log_int(k, channel_network[i]);
		if ( k != m )
			tw_error("[%s] log stack is probably mixed up on computer [%i] channel [%i]",
						comment, i, channel_network[i]);
	}

	// check if the "log stack" of data is empty
//	if (log_totalsize() > 0)
//		tw_error("Log isn't empty  %i %i", log_size_pl(0), log_size_pl(1));
// this is not necessarily an error, is it ? Namely, remote computer can still add
// data while this one is waiting?
#endif
// otherwise, it's just an empty routine.
}


int log_size_pl(int netnum)
{
	int k = channel_network[netnum];
	return log_size_ch(k);
}

int log_totalsize()
{
	int test = 0;
	int i;
	for ( i = 0; i < num_players; ++i )
		test += log_size_pl(i);

	return test;
}
REGISTER_GAME(NormalGame, "Melee")
