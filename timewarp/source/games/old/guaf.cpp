#include <allegro.h> //allegro library header

#include "../melee.h"          //used by all TW source files.  well, mostly.
REGISTER_FILE                  //done immediately after #including melee.h, just because I said so
#include "../melee/mgame.h"    //Game stuff
#include "../melee/mcbodies.h" //asteroids & planets
#include "../melee/mship.h"    //ships
#include "../melee/mlog.h"     //networking / demo recording / demo playback
#include "../melee/mcontrol.h" //controllers & AIs
#include "../melee/mview.h"    //Views & messages
#include "../melee/mshppan.h"  //ship panels...
#include "../melee/mfleet.h"    //fleets...

class ChooseArena : public Game { //declaring our game type
	virtual void init(Log *_log) ; //happens when our game is first started
	//virtual void set_resolution(int screen_x, int screen_y) ; //we want to change our screen layout, to make room for the ship panels
	ShipPanel *human_panel[2];  //we need to keep track of the ship panels, in case we have to move them around because the player changes screen resolutions
	Control *human_control[2];
	virtual void calculate() ;
	//to do the screen layouts, we need to know if those panels still exist, so we need to check every frame for them going away
	//calculate is good for things that need to be done every frame
	TeamCode human_team, enemy_team; //the two teams
	int respawn_time; //the time left until the game is restarted
	void pick_new_ships(); //restarts the game
	};

void ChooseArena::calculate() {
	Game::calculate();
	if (human_panel[0] && !human_panel[0]->exists()) human_panel[0] = NULL;
	if (human_panel[1] && !human_panel[1]->exists()) human_panel[1] = NULL;
	if (respawn_time == -1) {
		int i, humans = 0, enemies = 0;
		for (i = 0; i < num_targets; i += 1) {
			if (target[i]->get_team() == human_team) humans += 1;
			if (target[i]->get_team() == enemy_team) enemies += 1;
			}
		//if either team has no targetable items remaining (generally ships), pick new ships
		if (!humans || !enemies) respawn_time = game_time + 5000; //5000 milliseconds is 5 seconds
		}
	else if (respawn_time <= game_time) pick_new_ships();
	return;
	}

/*void ChooseArena::set_resolution(int screen_x, int screen_y) {
	view->set_window(screen, //the view window is on the screen
		0, 0, //with the upper left hand corner of the view window on the upper left corner of the screen
		screen_x - 64, screen_y //the height is full-screen, but the width doesn't cover the right-hand side, to make room for the ship panel there
		);
	if (human_panel[0]) human_panel[0]->locate(view->view_x + view->view_w, 0);   //if the panel still exists, position on the right-hand side
	if (human_panel[1]) human_panel[1]->locate(view->view_x + view->view_w, 100); //same for the other panel, if there is another panel
	redraw();
	}*/

void ChooseArena::pick_new_ships() {
	int i;
	acquire_screen();
	clear_to_color(screen, pallete_color[4]);
	release_screen();
	for (i = 0; i < num_items; i += 1) {
		if (item[i]->exists()) item[i]->die();
		}
	if (human_panel[0]) human_panel[0]->window->hide();
	if (human_panel[1]) human_panel[1]->window->hide();

	//player 1 selects a new ship
	Fleet fleet;
	log_file("fleets/all.scf");
	fleet.load(NULL, "Fleet");
	i = human_control[0]->choose_ship(window, "Hey You Player 1!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	Ship *s = create_ship(fleet.ship[i]->id, human_control[0], width/2 + 100, height/2, 0, human_team);
	add(s->get_ship_phaser());
	human_panel[0] = new ShipPanel(s);
	add(human_panel[0]);
	add_focus(s, channel_server);

	if ((log->type == Log::log_net1server) || (log->type == Log::log_net1client)) {
		log_file("fleets/all.scf");
		fleet.load(NULL, "Fleet");
		i = human_control[1]->choose_ship(window, "Hey You NET GUY!\nPick a ship!", &fleet);
		log_int(channel_client, i);
		if (i == -1) i = random() % fleet.size;
		Ship *s = create_ship(fleet.ship[i]->id, human_control[1], width/2 + 100, height/2, PI, human_team);
		add(s->get_ship_phaser());
		add_focus(s, channel_client);
		human_panel[1] = new ShipPanel(s);
		add(human_panel[1]);
		}
	else human_panel[1] = NULL;


	Ship *e;
	log_file("fleets/all.scf");
	fleet.load(NULL, "Fleet");
	i = human_control[0]->choose_ship(window, "Hey You  Player 2!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4, height/4, random(PI2), enemy_team);
	add(e->get_ship_phaser());

	log_file("fleets/all.scf");
	fleet.load(NULL, "Fleet");
	i = human_control[0]->choose_ship(window, "Hey You!  Player 3\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width*3/4, height/4, random(PI2), enemy_team);
	add(e->get_ship_phaser());

	log_file("fleets/all.scf");
	fleet.load(NULL, "Fleet");
	i = human_control[0]->choose_ship(window, "Hey You Player 4!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width*3/4, height*3/4, random(PI2), enemy_team);
	add(e->get_ship_phaser());

	log_file("fleets/all.scf");
	fleet.load(NULL, "Fleet");
	i = human_control[0]->choose_ship(window, "Hey You Player 5!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4, height*3/4, random(PI2), enemy_team);
	add(e->get_ship_phaser());

	add(new Planet(width/4, height/2, planetSprite, 0));

	if (human_panel[0]) human_panel[0]->window->hide();//locate(view->view_x + view->view_w, 0);   //if the panel still exists, position on the right-hand side
	if (human_panel[1]) human_panel[1]->window->hide();//locate(view->view_x + view->view_w, 100); //same for the other panel, if there is another panel

	redraw();
	respawn_time = -1;
	return;
	}

void ChooseArena::init(Log *_log) {
	Game::init(_log);

	add(new Stars());

	width = 4200;
	height = 4200;
	//width & height are the dimensions of the game map
	//they should be modified before prepare() is called
	//If for some reason you modify them later, call prepare() again
	//but you probably shouldn't be modifying them later.
	//The normal size is 3840x3840
	prepare();

	human_team = new_team();
	enemy_team = new_team();

	human_control[0] = create_control(channel_server, "Human");

	if ((log->type == Log::log_net1server) || (log->type == Log::log_net1client)) {
		human_control[1] = create_control(channel_client, "Human");
		}
	else human_control[1] = NULL;

	int i;
	for (i = 0; i < 7; i += 1) add(new Asteroid());

	human_panel[0] = NULL;
	human_panel[1] = NULL;

	change_view("Hero"); 
	//sets it to Hero view mode
	view_locked = true;
	//prevents the view mode from being changed through the menu

	respawn_time = 500;

	message.out("Welcome to Teams Arena!", 3000, 15);
	}

REGISTER_GAME(ChooseArena, "1on4")


//******************************************************************************
//******************************************************************************
//******************************************************************************
class TeamArena : public Game { //declaring our game type
	virtual void init(Log *_log) ; //happens when our game is first started
	virtual void set_resolution(int screen_x, int screen_y) ; //we want to change our screen layout, to make room for the ship panels
	ShipPanel *human_panel[2];  //we need to keep track of the ship panels, in case we have to move them around because the player changes screen resolutions
	Control *human_control[2];
	virtual void calculate() ; 
	//to do the screen layouts, we need to know if those panels still exist, so we need to check every frame for them going away
	//calculate is good for things that need to be done every frame
	TeamCode human_team, enemy_team; //the two teams
	int respawn_time; //the time left until the game is restarted
	void pick_new_ships(); //restarts the game
	};

void TeamArena::calculate() {
	Game::calculate();
	if (human_panel[0] && !human_panel[0]->exists()) human_panel[0] = NULL;
	if (human_panel[1] && !human_panel[1]->exists()) human_panel[1] = NULL;
	if (respawn_time == -1) {
		int i, humans = 0, enemies = 0;
		for (i = 0; i < num_targets; i += 1) {
			if (target[i]->get_team() == human_team) humans += 1;
			if (target[i]->get_team() == enemy_team) enemies += 1;
			}
		//if either team has no targetable items remaining (generally ships), pick new ships
		if (!humans || !enemies) respawn_time = game_time + 5000; //5000 milliseconds is 10 seconds
		}
	else if (respawn_time <= game_time) pick_new_ships();
	return;
	}

void TeamArena::set_resolution(int screen_x, int screen_y) {
	/*view->set_window(screen, //the view window is on the screen
		0, 0, //with the upper left hand corner of the view window on the upper left corner of the screen
		screen_x - 64, screen_y //the height is full-screen, but the width doesn't cover the right-hand side, to make room for the ship panel there
		);
	if (human_panel[0]) human_panel[0]->locate(view->view_x + view->view_w, 0);   //if the panel still exists, position on the right-hand side
	if (human_panel[1]) human_panel[1]->locate(view->view_x + view->view_w, 100); //same for the other panel, if there is another panel*/
	view->window->locate(0,0,0,0, -64,1.0, 0,1.0);
	redraw();
	}

void TeamArena::pick_new_ships() {
	int i;
	acquire_screen();
	clear_to_color(screen, pallete_color[4]);
	release_screen();
	for (i = 0; i < num_items; i += 1) {
		if (item[i]->exists()) item[i]->die();
		}
	if (human_panel[0]) human_panel[0]->window->hide();
	if (human_panel[1]) human_panel[1]->window->hide();

	//player 1 selects a new ship
	Fleet fleet;
	log_file("fleets/all.scf");
	fleet.load(NULL, "Fleet");
	i = human_control[0]->choose_ship(window, "Hey You Player 1!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	Ship *s = create_ship(fleet.ship[i]->id, human_control[0], width/4 + 100, height*3/4, 0, human_team);
	add(s->get_ship_phaser());
	human_panel[0] = new ShipPanel(s);
	add(human_panel[0]);
	add_focus(s, channel_server);

	if ((log->type == Log::log_net1server) || (log->type == Log::log_net1client)) {
		log_file("fleets/all.scf");
		fleet.load(NULL, "Fleet");
		i = human_control[1]->choose_ship(window, "Hey You NET GUY!\nPick a ship!", &fleet);
		log_int(channel_client, i);
		if (i == -1) i = random() % fleet.size;
		Ship *s = create_ship(fleet.ship[i]->id, human_control[1], width/4 + 100, height*3/4, PI, human_team);
		add(s->get_ship_phaser());
		add_focus(s, channel_client);
		human_panel[1] = new ShipPanel(s);
		add(human_panel[1]);
		}
	else human_panel[1] = NULL;


	Ship *e;
//Human AI fleet
	log_file("fleets/all.scf");
	fleet.load(NULL, "Fleet");
	i = human_control[0]->choose_ship(window, "Hey You Player 2!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4, height*3/4, random(PI2), human_team);
	add(e->get_ship_phaser());

	log_file("fleets/all.scf");
	fleet.load(NULL, "Fleet");
	i = human_control[0]->choose_ship(window, "Hey You Player 3!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4-100, height*3/4, random(PI2), human_team);
	add(e->get_ship_phaser());

   log_file("fleets/all.scf");
	fleet.load(NULL, "Fleet");
	i = human_control[0]->choose_ship(window, "Hey You Player 4!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4+200, height*3/4, random(PI2), human_team);
	add(e->get_ship_phaser());

//Comp fleet
	log_file("fleets/all.scf");
	fleet.load(NULL, "Fleet");
	i = human_control[0]->choose_ship(window, "Hey You Player 5!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4, height/4, random(PI2), enemy_team);
	add(e->get_ship_phaser());

	log_file("fleets/all.scf");
	fleet.load(NULL, "Fleet");
	i = human_control[0]->choose_ship(window, "Hey You Player 6!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4+100, height/4, random(PI2), enemy_team);
	add(e->get_ship_phaser());

	log_file("fleets/all.scf");
	fleet.load(NULL, "Fleet");
	i = human_control[0]->choose_ship(window, "Hey You Player 7!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4-100, height/4, random(PI2), enemy_team);
	add(e->get_ship_phaser());

	log_file("fleets/all.scf");
	fleet.load(NULL, "Fleet");
	i = human_control[0]->choose_ship(window, "Hey You Player 8!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4+200, height/4, random(PI2), enemy_team);
	add(e->get_ship_phaser());

	add(new Planet(width/2, height/2, planetSprite, 0));

	//if (human_panel[0]) human_panel[0]->window->locate(view->view_x + view->view_w, 0);   //if the panel still exists, position on the right-hand side
	//if (human_panel[1]) human_panel[1]->window->locate(view->view_x + view->view_w, 100); //same for the other panel, if there is another panel

	redraw();
	respawn_time = -1;
	return;
	}

void TeamArena::init(Log *_log) {
	Game::init(_log);

	add(new Stars());

	width = 4200;
	height = 4200;
	//width & height are the dimensions of the game map
	//they should be modified before prepare() is called
	//If for some reason you modify them later, call prepare() again
	//but you probably shouldn't be modifying them later. 
	//The normal size is 3840x3840
	prepare();

	human_team = new_team();
	enemy_team = new_team();

	human_control[0] = create_control(channel_server, "Human");
	
	if ((log->type == Log::log_net1server) || (log->type == Log::log_net1client)) { 
		human_control[1] = create_control(channel_client, "Human");
		}
	else human_control[1] = NULL;

	int i;
	for (i = 0; i < 7; i += 1) add(new Asteroid());

	human_panel[0] = NULL;
	human_panel[1] = NULL;

   change_view("Hero"); 
	//sets it to Hero view mode
	view_locked = false;
	//prevents the view mode from being changed through the menu

	respawn_time = 500;

	message.out("Welcome to Teams Arena!", 3000, 15);
	}

REGISTER_GAME(TeamArena, "4on4")
//******************************************************************************
//******************************************************************************
//******************************************************************************
class FleetsMelee : public Game { //declaring our game type
	virtual void init(Log *_log) ; //happens when our game is first started
	virtual void set_resolution(int screen_x, int screen_y) ; //we want to change our screen layout, to make room for the ship panels
	ShipPanel *human_panel[2];  //we need to keep track of the ship panels, in case we have to move them around because the player changes screen resolutions
	Control *human_control[2];
	virtual void calculate() ; 
	//to do the screen layouts, we need to know if those panels still exist, so we need to check every frame for them going away
	//calculate is good for things that need to be done every frame
	TeamCode human_team, enemy_team; //the two teams
	int respawn_time; //the time left until the game is restarted
	void pick_new_ships(); //restarts the game
	};

void FleetsMelee::calculate() {
	Game::calculate();
	if (human_panel[0] && !human_panel[0]->exists()) human_panel[0] = NULL;
	if (human_panel[1] && !human_panel[1]->exists()) human_panel[1] = NULL;
	if (respawn_time == -1) {
		int i, humans = 0, enemies = 0;
		for (i = 0; i < num_targets; i += 1) {
			if (target[i]->get_team() == human_team) humans += 1;
			if (target[i]->get_team() == enemy_team) enemies += 1;
			}
		//if either team has no targetable items remaining (generally ships), pick new ships
		if (!humans || !enemies) respawn_time = game_time + 5000; //5000 milliseconds is 10 seconds
		}
	else if (respawn_time <= game_time) pick_new_ships();
	return;
	}

void FleetsMelee::set_resolution(int screen_x, int screen_y) {
	/*view->set_window(screen, //the view window is on the screen
		0, 0, //with the upper left hand corner of the view window on the upper left corner of the screen
		screen_x - 64, screen_y //the height is full-screen, but the width doesn't cover the right-hand side, to make room for the ship panel there
		);
	if (human_panel[0]) human_panel[0]->locate(view->view_x + view->view_w, 0);   //if the panel still exists, position on the right-hand side
	if (human_panel[1]) human_panel[1]->locate(view->view_x + view->view_w, 100); //same for the other panel, if there is another panel*/
	redraw();
	}

void FleetsMelee::pick_new_ships() {
	int i;
	acquire_screen();
	clear_to_color(screen, pallete_color[4]);
	release_screen();
	for (i = 0; i < num_items; i += 1) {
		if (item[i]->exists()) item[i]->die();
		}
	if (human_panel[0]) human_panel[0]->window->hide();
	if (human_panel[1]) human_panel[1]->window->hide();

	//player 1 selects a new ship
	Fleet fleet;
	log_file("fleets.ini");
	fleet.load(NULL, "Player1");
	if (fleet.size>0) {
	i = human_control[0]->choose_ship(window, "Hey You Player 1!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	Ship *s = create_ship(fleet.ship[i]->id, human_control[0], width/4 + 100, height*3/4, 0, human_team);
	add(s->get_ship_phaser());
	human_panel[0] = new ShipPanel(s);
	add(human_panel[0]);
	add_focus(s, channel_server);
	}
	if ((log->type == Log::log_net1server) || (log->type == Log::log_net1client)) {
		log_file("fleets.ini");
		fleet.load(NULL, "Player2");
		i = human_control[1]->choose_ship(window, "Hey You NET GUY!\nPick a ship!", &fleet);
		log_int(channel_client, i);
		if (i == -1) i = random() % fleet.size;
		Ship *s = create_ship(fleet.ship[i]->id, human_control[1], width/4 + 100, height*3/4, PI, human_team);
		add(s->get_ship_phaser());
		add_focus(s, channel_client);
		human_panel[1] = new ShipPanel(s);
		add(human_panel[1]);
		}
	else human_panel[1] = NULL;


	Ship *e;
//Human AI fleet
	log_file("fleets.ini");
	fleet.load(NULL, "Player2");
	if (fleet.size>0) {
	i = human_control[0]->choose_ship(window, "Hey You Player 2!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4, height*3/4, random(PI2), human_team);
	add(e->get_ship_phaser());
	}

	log_file("fleets.ini");
	fleet.load(NULL, "Player3");
	if (fleet.size>0) {
	i = human_control[0]->choose_ship(window, "Hey You Player 3!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4-100, height*3/4, random(PI2), human_team);
	add(e->get_ship_phaser());
	}

	log_file("fleets.ini");
	fleet.load(NULL, "Player4");
	if (fleet.size>0) {
	i = human_control[0]->choose_ship(window, "Hey You Player 4!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4+200, height*3/4, random(PI2), human_team);
	add(e->get_ship_phaser());
	}
//Comp fleet
	log_file("fleets.ini");
	fleet.load(NULL, "Player5");
	if (fleet.size>0) {
	i = human_control[0]->choose_ship(window, "Hey You Player 5!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4, height/4, random(PI2), enemy_team);
	add(e->get_ship_phaser());
	}

	log_file("fleets.ini");
	fleet.load(NULL, "Player6");
	if (fleet.size>0) {
	i = human_control[0]->choose_ship(window, "Hey You Player 6!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4+100, height/4, random(PI2), enemy_team);
	add(e->get_ship_phaser());
	}

	log_file("fleets.ini");
	fleet.load(NULL, "Player7");
	if (fleet.size>0) {
	i = human_control[0]->choose_ship(window, "Hey You Player 7!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4-100, height/4, random(PI2), enemy_team);
	add(e->get_ship_phaser());
	}

	log_file("fleets.ini");
	fleet.load(NULL, "Player8");
	if (fleet.size>0) {
	i = human_control[0]->choose_ship(window, "Hey You Player 8!\nPick a ship!", &fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet.size;
	e = create_ship(channel_none, fleet.ship[i]->id, "WussieBot", width/4+200, height/4, random(PI2), enemy_team);
	add(e->get_ship_phaser());
	}

	add(new Planet(width/2, height/2, planetSprite, 0));

	//if (human_panel[0]) human_panel[0]->locate(view->view_x + view->view_w, 0);   //if the panel still exists, position on the right-hand side
	//if (human_panel[1]) human_panel[1]->locate(view->view_x + view->view_w, 100); //same for the other panel, if there is another panel

	redraw();
	respawn_time = -1;
	return;
	}

void FleetsMelee::init(Log *_log) {
	Game::init(_log);

	add(new Stars());

	width = 4200;
	height = 4200;
	//width & height are the dimensions of the game map
	//they should be modified before prepare() is called
	//If for some reason you modify them later, call prepare() again
	//but you probably shouldn't be modifying them later. 
	//The normal size is 3840x3840
	prepare();

	human_team = new_team();
	enemy_team = new_team();

	human_control[0] = create_control(channel_server, "Human");
	
	if ((log->type == Log::log_net1server) || (log->type == Log::log_net1client)) { 
		human_control[1] = create_control(channel_client, "Human");
		}
	else human_control[1] = NULL;

	int i;
	for (i = 0; i < 7; i += 1) add(new Asteroid());

	human_panel[0] = NULL;
	human_panel[1] = NULL;

	change_view("Hero");
	//sets it to Hero view mode
	view_locked = false;
	//prevents the view mode from being changed through the menu

	view->window->locate(0,0,0,0, -PANEL_WIDTH,1.0, 0,1.0);

	respawn_time = 500;

	message.out("Welcome to Teams Arena!", 3000, 15);
	}

REGISTER_GAME(FleetsMelee, "Fleets Melee")