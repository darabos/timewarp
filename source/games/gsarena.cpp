#include <allegro.h> //allegro library header
#include <stdio.h>

#include "../melee.h"          //used by all TW source files.  well, mostly.
REGISTER_FILE                  //done immediately after #including melee.h, just because I said so
#include "../melee/mgame.h"    //Game stuff
#include "../melee/mcbodies.h" //asteroids & planets
#include "../melee/mship.h"    //ships
#include "../melee/mlog.h"     //networking / demo recording / demo playback
#include "../melee/mcontrol.h" //controllers & AIs
#include "../melee/mview.h"    //Views & messages
#include "../melee/mshppan.h"  //ship panels...
#include "../melee/mitems.h"  //ship panels...
#include "../melee/mfleet.h"
#include "../other/orbit.h"

class SuperArena : public Game { //declaring our game type
	virtual void init(Log *_log) ; //happens when our game is first started
	ShipPanel *human_panel[2];  //we need to keep track of the ship panels, in case we have to move them around because the player changes screen resolutions
	Control *human_control[2];

	virtual void calculate() ;
	//to do the screen layouts, we need to know if those panels still exist, so we need to check every frame for them going away
	//calculate is good for things that need to be done every frame
	TeamCode team[9]; //the teams (array)
	TeamCode station_team;
	int respawn_time; //the time left until the game is restarted
	void pick_new_ships(); //restarts the game

//	void new_ship(int team, int human);


	Ship *s[9]; //the starbases
	int started,KTS;
	int died[9];
	int num_teams; //added by orz, attempted bugfix
	};

void SuperArena::calculate() {
	Game::calculate();
	if (human_panel[0] && !human_panel[0]->exists()) human_panel[0] = NULL;
	if (human_panel[1] && !human_panel[1]->exists()) human_panel[1] = NULL;
	if (respawn_time == -1) {
	  //	int i, humans = 0, enemies = 0;
		int i, t1=0, t2=0, t3=0, t4=0, t5=0, t6=0, t7=0, t8=0;
		for (i = 0; i < num_targets; i += 1) {
			if (target[i]->get_team() == team[1])  t1 += 1;
			if (target[i]->get_team() == team[2])  t2 += 1;
			if (target[i]->get_team() == team[3])  t3 += 1;
			if (target[i]->get_team() == team[4])  t4 += 1;
			if (target[i]->get_team() == team[5])  t5 += 1;
			if (target[i]->get_team() == team[6])  t6 += 1;
			if (target[i]->get_team() == team[7])  t7 += 1;
			if (target[i]->get_team() == team[8])  t8 += 1;
		}
		//if either team has no targetable items remaining (generally ships), pick new ships
	  //		if (humans = 0) respawn_time = game_time + 5000; //5000 milliseconds is 10 seconds
	  //	if (!humans || !enemies) respawn_time = game_time + 5000; //5000 milliseconds is 10 seconds
 //		iMessage("Team1 have %d ships",t1);
	//	iMessage("Team2 have %d ships",t2);
		if ((t2==0) && (t3==0) && (t4==0) && (t5==0) && (t6==0) && (t7==0) && (t8==0))  respawn_time = game_time + 5000;
		if ((t1==0) && (t3==0) && (t4==0) && (t5==0) && (t6==0) && (t7==0) && (t8==0))  respawn_time = game_time + 5000;
		if ((t1==0) && (t2==0) && (t4==0) && (t5==0) && (t6==0) && (t7==0) && (t8==0))  respawn_time = game_time + 5000;
		if ((t1==0) && (t2==0) && (t3==0) && (t5==0) && (t6==0) && (t7==0) && (t8==0))  respawn_time = game_time + 5000;
		if ((t1==0) && (t2==0) && (t3==0) && (t4==0) && (t6==0) && (t7==0) && (t8==0))  respawn_time = game_time + 5000;
		if ((t1==0) && (t2==0) && (t3==0) && (t4==0) && (t5==0) && (t7==0) && (t8==0))  respawn_time = game_time + 5000;
		if ((t1==0) && (t2==0) && (t3==0) && (t4==0) && (t5==0) && (t6==0) && (t8==0))  respawn_time = game_time + 5000;
		if ((t1==0) && (t2==0) && (t3==0) && (t4==0) && (t5==0) && (t6==0) && (t7==0))  respawn_time = game_time + 5000;
		if ((t1==0) && (t2==0) && (t3==0) && (t4==0) && (t5==0) && (t6==0) && (t7==0) && (t8==0))  respawn_time = game_time + 5000;


		if ((started == 1) && (KTS == 1)) {
			for (int num=1; num<=num_teams; num+=1) {
				if ( (died[num]!=1) && (!s[num]->exists()) ) {
					message.print( 4000, 4000, "Team %d starbase was destroyed", num+1 ); //("Player's 1 Starbase was destroyed", 3000);
					for (i = 0; i < num_targets; i += 1) {
						if (target[i]->get_team() == team[num]) {
							target[i]->handle_damage(NULL, 0, 999);
							//target[i]->handle_damage(target[i]);
						}
					}
					died[num]=1;
				}
			}
		}
   }
	else if (respawn_time <= game_time) pick_new_ships();

	return;
	}

void SuperArena::pick_new_ships() {
	int i,t,p[9],j;
	log_file("sarena.ini");
	num_teams = t = get_config_int(NULL, "Teams", 2);
	p[1] = get_config_int(NULL, "Team1", 2);
	p[2] = get_config_int(NULL, "Team2", 2);
	p[3] = get_config_int(NULL, "Team3", 2);
	p[4] = get_config_int(NULL, "Team4", 2);
	p[5] = get_config_int(NULL, "Team5", 2);
	p[6] = get_config_int(NULL, "Team6", 2);
	p[7] = get_config_int(NULL, "Team7", 2);
	p[8] = get_config_int(NULL, "Team8", 2);

	// is KTS on?
	KTS = get_config_int(NULL, "KTS", 0);

	acquire_screen();
	clear_to_color(screen, pallete_color[4]);
	release_screen();
	for (i = 0; i < num_items; i += 1) {
		if (item[i]->exists()) item[i]->die();
		}
	if (human_panel[0]) human_panel[0]->window->hide();
	if (human_panel[1]) human_panel[1]->window->hide();


	Fleet _fleet;
	Fleet *fleet = &_fleet;

	//player 1 selects a new ship
	log_file("fleets.ini");
	fleet->load(NULL, "Player1");
	if (fleet->size>0) {
	i = human_control[0]->choose_ship(window, "player 1 \nship 1", fleet);
	log_int(channel_server, i);
	if (i == -1) i = random() % fleet->size;
	Ship *s = create_ship(fleet->ship[i]->id, human_control[0], Vector2(map_size.x/4 , map_size.y*3.0/4), 0, team[1]);
	add(s->get_ship_phaser());
	human_panel[0] = new ShipPanel(s);
	human_panel[0]->window->init(window);
	human_panel[0]->window->locate(0,0.9, 0,0, 0,0.1, 0,.25);

	add(human_panel[0]);
	add_focus(s, channel_server);
	}

	if ((log->type == Log::log_net1server) || (log->type == Log::log_net1client)) {
		fleet->load("fleets.ini", "Player2");
		log_fleet(channel_client, fleet);
		if (fleet->size>0) {
		i = human_control[1]->choose_ship(window, "player 2 \nship 1", fleet);
		log_int(channel_client, i);
		if (i == -1) i = random() % fleet->size;
		Ship *s = create_ship(fleet->ship[i]->id, human_control[1], Vector2(size.x/4 , size.y/4), PI, team[2]);
		add(s->get_ship_phaser());
		add_focus(s, channel_client);
		human_panel[1] = new ShipPanel(s);
		human_panel[0]->window->init(window);
		human_panel[0]->window->locate(0,0.9, 0,0.25, 0,0.1, 0,.25);
		add(human_panel[1]);
		}
		}
	else human_panel[1] = NULL;


	Ship *e;
	Planet *Pl;
	OrbitHandler *handler;
	char buf[256];

	for ( j = 1; j < p[1]; j += 1) {
		log_file("fleets.ini");
		fleet->load(NULL, "Player1");
		if (fleet->size>0) {
			sprintf( buf, "Player 1 \nShip %d", j+1 );
			i = human_control[0]->choose_ship(window, buf, fleet);
			log_int(channel_server, i);
			if (i == -1) i = random() % fleet->size;
			e = create_ship(channel_none, fleet->ship[i]->id, "WussieBot", Vector2(size.x/4, size.y*3/4), random(PI2), team[1]);
			add(e->get_ship_phaser());
		}
	}
	if (KTS==1) {
		Pl = new Planet(Vector2(size.x/4+500, size.y*3/4+500), planetSprite, random()%planetSprite->frames());
		add(Pl);
		s[1] = create_ship(channel_none, "staba", "WussieBot", Vector2(size.x/4, size.y*3/4), 1, team[1]);
		add ( new BlinkyIndicator(s[1], 2) );
		handler = new OrbitHandler(Pl,Vector2(size.x/2,size.y/2),random(PI2), (SpaceLocation *)Pl,
		(SpaceLocation *)s[1], 400, 10.0*ANGLE_RATIO,0);
		add(s[1]);
		game->add(handler);
	}

	if (t>1) {
		for ( j = 0; j < p[2]; j += 1) {
			log_file("fleets.ini");
			fleet->load(NULL, "Player2");
			if (fleet->size>0) {
				sprintf( buf, "Player 2 \nShip %d", j+1 );
				i = human_control[0]->choose_ship(window, buf, fleet);
				log_int(channel_server, i);
				if (i == -1) i = random() % fleet->size;
				e = create_ship(channel_none, fleet->ship[i]->id, "WussieBot", Vector2(size.x/4, size.y/4), random(PI2), team[2]);
				add(e->get_ship_phaser());
			}
		}
		if (KTS==1) {
			Pl = new Planet(Vector2(size.x/4+500, size.y/4+500), planetSprite, random()%planetSprite->frames());
			add(Pl);
			s[2] = create_ship(channel_none, "staba", "WussieBot", Vector2(size.x/4, size.y*3/4), 1, team[2]);
			add ( new BlinkyIndicator(s[2], 4) );
			handler = new OrbitHandler(Pl,Vector2(size.x/2,size.y/2),random(PI2), (SpaceLocation *)Pl,
			(SpaceLocation *)s[2], 400, 10.0*ANGLE_RATIO,0);
			add(s[2]);
			game->add(handler);
		}
	}


	if (t>2) {
		for ( j = 0; j < p[3]; j += 1) {
			log_file("fleets.ini");
			fleet->load(NULL, "Player3");
			if (fleet->size>0) {
				sprintf( buf, "Player 3 \nShip %d", j+1 );
				i = human_control[0]->choose_ship(window, buf, fleet);
				log_int(channel_server, i);
				if (i == -1) i = random() % fleet->size;
				e = create_ship(channel_none, fleet->ship[i]->id, "WussieBot", Vector2(size.x*3/4, size.y/4), random(PI2), team[3]);
				add(e->get_ship_phaser());
			}
		}
		if (KTS==1) {
			Pl = new Planet(Vector2(size.x*3/4+500, size.y/4+500), planetSprite, random()%planetSprite->frames());
			add(Pl);
			s[3] = create_ship(channel_none, "staba", "WussieBot", Vector2(size.x/4, size.y*3/4), 1, team[3]);
			add ( new BlinkyIndicator(s[3], 4) );
			handler = new OrbitHandler(Pl,Vector2(size.x/2,size.y/2),random(PI2), (SpaceLocation *)Pl,
			(SpaceLocation *)s[3], 400, 10.0*ANGLE_RATIO,0);
			add(s[3]);
			game->add(handler);
		}
	}

	if (t>3) {
		for ( j = 0; j < p[4]; j += 1) {
			log_file("fleets.ini");
			fleet->load(NULL, "Player4");
			if (fleet->size>0) {
				sprintf( buf, "Player 4 \nShip %d", j+1 );
				i = human_control[0]->choose_ship(window, buf, fleet);
				log_int(channel_server, i);
				if (i == -1) i = random() % fleet->size;
				e = create_ship(channel_none, fleet->ship[i]->id, "WussieBot", Vector2(size.x*3/4, size.y*3/4), random(PI2), team[4]);
				add(e->get_ship_phaser());
			}
		}
		if (KTS==1) {
			Pl = new Planet(Vector2(size.x*3/4+500, size.y*3/4+500), planetSprite, random()%planetSprite->frames());
			add(Pl);
			s[4] = create_ship(channel_none, "staba", "WussieBot", Vector2(size.x/4, size.y*3/4), 1, team[4]);
			add ( new BlinkyIndicator(s[4], 4) );
			handler = new OrbitHandler(Pl,Vector2(size.x/2,size.y/2),random(PI2), (SpaceLocation *)Pl,
			(SpaceLocation *)s[4], 400, 10.0*ANGLE_RATIO,0);
			add(s[4]);
			game->add(handler);
		}
	}


	if (t>4) {
		for ( j = 0; j < p[5]; j += 1) {
			log_file("fleets.ini");
			fleet->load(NULL, "Player5");
			if (fleet->size>0) {
				sprintf( buf, "Player 5 \nShip %d", j+1 );
				i = human_control[0]->choose_ship(window, buf, fleet);
				log_int(channel_server, i);
				if (i == -1) i = random() % fleet->size;
				e = create_ship(channel_none, fleet->ship[i]->id, "WussieBot", Vector2(size.x/2, size.y*3/4), random(PI2), team[5]);
				add(e->get_ship_phaser());
			}
		}
		if (KTS==1) {
			Pl = new Planet(Vector2(size.x/2+500, size.y*3/4+500), planetSprite, random()%planetSprite->frames());
			add(Pl);
			s[5] = create_ship(channel_none, "staba", "WussieBot", Vector2(size.x/4, size.y*3/4), 1, team[5]);
			add ( new BlinkyIndicator(s[5], 4) );
			handler = new OrbitHandler(Pl,Vector2(size.x/2,size.y/2),random(PI2), (SpaceLocation *)Pl,
			(SpaceLocation *)s[5], 400, 10.0*ANGLE_RATIO,0);
			add(s[5]);
			game->add(handler);
		}
	}

	if (t>5) {
		for ( j = 0; j < p[6]; j += 1) {
			log_file("fleets.ini");
			fleet->load(NULL, "Player6");
			if (fleet->size>0) {
				sprintf( buf, "Player 6 \nShip %d", j+1 );
				i = human_control[0]->choose_ship(window, buf, fleet);
				log_int(channel_server, i);
				if (i == -1) i = random() % fleet->size;
				e = create_ship(channel_none, fleet->ship[i]->id, "WussieBot", Vector2(size.x/4, size.y/2), random(PI2), team[6]);
				add(e->get_ship_phaser());
			}
		}
		if (KTS==1) {
			Pl = new Planet(Vector2(size.x/4+500, size.y/2+500), planetSprite, random()%planetSprite->frames());
			add(Pl);
			s[6] = create_ship(channel_none, "staba", "WussieBot", Vector2(size.x/4, size.y*3/4), 1, team[6]);
			add ( new BlinkyIndicator(s[6], 4) );
			handler = new OrbitHandler(Pl,Vector2(size.x/2,size.y/2),random(PI2), (SpaceLocation *)Pl,
			(SpaceLocation *)s[6], 400, 10.0*ANGLE_RATIO,0);
			add(s[6]);
			game->add(handler);
		}
	}

	if (t>6) {
		for ( j = 0; j < p[7]; j += 1) {
			log_file("fleets.ini");
			fleet->load(NULL, "Player7");
			if (fleet->size>0) {
				sprintf( buf, "Player 7 \nShip %d", j+1 );
				i = human_control[0]->choose_ship(window, buf, fleet);
				log_int(channel_server, i);
				if (i == -1) i = random() % fleet->size;
				e = create_ship(channel_none, fleet->ship[i]->id, "WussieBot", Vector2(size.x/2, size.y/4), random(PI2), team[7]);
				add(e->get_ship_phaser());
			}
		}
		if (KTS==1) {
			Pl = new Planet(Vector2(size.x/2+500, size.y/4+500), planetSprite, random()%planetSprite->frames());
			add(Pl);
			s[7] = create_ship(channel_none, "staba", "WussieBot", Vector2(size.x/4, size.y*3/4), 1, team[7]);
			add ( new BlinkyIndicator(s[7], 4) );
			handler = new OrbitHandler(Pl,Vector2(size.x/2,size.y/2),random(PI2), (SpaceLocation *)Pl,
			(SpaceLocation *)s[7], 400, 10.0*ANGLE_RATIO,0);
			add(s[7]);
			game->add(handler);
		}
	}

	if (t>7) {
		for ( j = 0; j < p[8]; j += 1) {
			log_file("fleets.ini");
			fleet->load(NULL, "Player8");
			if (fleet->size>0) {
				sprintf( buf, "Player 8 \nShip %d", j+1 );
				i = human_control[0]->choose_ship(window, buf, fleet);
				log_int(channel_server, i);
				if (i == -1) i = random() % fleet->size;
				e = create_ship(channel_none, fleet->ship[i]->id, "WussieBot", Vector2(size.x*3/4, size.y/2), random(PI2), team[8]);
				add(e->get_ship_phaser());
			}
		}
		if (KTS==1) {
			Pl = new Planet(Vector2(size.x*3/4+500, size.y/2+500), planetSprite, random()%planetSprite->frames());
			add(Pl);
			s[8] = create_ship(channel_none, "staba", "WussieBot", Vector2(size.x/4, size.y*3/4), 1, team[8]);
			add ( new BlinkyIndicator(s[8], 4) );
			handler = new OrbitHandler(Pl,Vector2(size.x/2,size.y/2),random(PI2), (SpaceLocation *)Pl,
			(SpaceLocation *)s[8], 400, 10.0*ANGLE_RATIO,0);
			add(s[8]);
			game->add(handler);
		}
	}

	// planets & stations code
	int g,m,s;
	log_file("sarena.ini");
	g = get_config_int(NULL, "Planet", 0);
	if (g > 0) {
		Pl = new Planet(Vector2(size.x/2, size.y/2), planetSprite, random()%planetSprite->frames());
		add(Pl);
		s = get_config_int(NULL, "Station", 0);
		if (s > 0) {
			Ship *Station1 = create_ship(channel_none, "staba", "WussieBot", Vector2(size.x/2, size.y/2), 1, station_team);
			handler = new OrbitHandler(Pl,Vector2(size.x/2,size.y/2),random(PI2), (SpaceLocation *)Pl,
			(SpaceLocation *)Station1, 400, 10.0*ANGLE_RATIO,0);
			add(Station1);
			game->add(handler);
		}
	}

	log_file("sarena.ini");
	g = get_config_int(NULL, "Oplanets", 0);
	s = get_config_int(NULL, "Ostations", 0);
	for (m = 0; m < g; m += 1) {
		Pl = new Planet(Vector2(random() % (int)size.x, random() % (int)size.y), planetSprite, random()%planetSprite->frames());
		add(Pl);
		if (s>0) {
			Ship *Station1 = create_ship(channel_none, "staba", "WussieBot", Vector2(size.x/2, size.y/2), 1, station_team);
			handler = new OrbitHandler(Pl,Vector2(size.x/2,size.y/2),random(PI2), (SpaceLocation *)Pl,
			(SpaceLocation *)Station1, 400, 10.0*ANGLE_RATIO,0);
			add(Station1);
			game->add(handler);
		}
		s = s-1;
	}

	Planet *c;
//	load_datafile("melee.dat");
	log_file("sarena.ini");
	m = get_config_int(NULL, "Comets", 2);
	for (g = 1; g < m; g += 1) {
		c = new Planet (Vector2(random()%int(size.x), random()%int(size.y)), kaboomSprite, 1);
		c->mass = get_config_int(NULL, "Comet_mass", 2);
		c->gravity_force *= 0;
		c->gravity_whip = 0;
		c->accelerate(NULL, random(PI2), get_config_int(NULL, "Comet_acc", 2), get_config_int(NULL, "Comet_max", 2));
		add (c);
		}

	//if (human_panel[0]) human_panel[0]->locate(view->view_x + view->view_w, 0);   //if the panel still exists, position on the right-hand side
	//if (human_panel[1]) human_panel[1]->locate(view->view_x + view->view_w, 100); //same for the other panel, if there is another panel

	redraw();
	respawn_time = -1;
	started = 1;
   return;
	}

//void SuperArena::new_ship(int team, int human) {



void SuperArena::init(Log *_log) {
	Game::init(_log);
	int j,i;
	log_file("sarena.ini");
	j = get_config_int(NULL, "Size", 3840);
	view->window->locate(0,0,0,0,0,0.9,0,1);
	size.x = j;
	size.y = j;
	//width & height are the dimensions of the game map
	//they should be modified before prepare() is called
	//If for some reason you modify them later, call prepare() again
	//but you probably shouldn't be modifying them later.
	//The normal size is 3840x3840
	prepare();

	log_file("sarena.ini");
	i = get_config_int(NULL, "Teams", 2);
	add(new Stars());
	for (j = 1; j < i+1; j += 1) {
	team[j] = new_team();
   iMessage("Team%d created",j);
	}
	station_team = new_team();

	human_control[0] = create_control(channel_server, "Human");

	if ((log->type == Log::log_net1server) || (log->type == Log::log_net1client)) {
		human_control[1] = create_control(channel_client, "Human");
		}
	else human_control[1] = NULL;

	// asteroids code
	log_file("sarena.ini");
	j = get_config_int(NULL, "Asteroids", 2);
	if (j > 0) for (i = 1; i < j; i += 1) add(new Asteroid());
	else {
	j = get_config_int(NULL, "Size", 2);
	for (i = 0; i < j/900; i += 1) add(new Asteroid());
	}
	human_panel[0] = NULL;
	human_panel[1] = NULL;

	change_view("Hero");
	//sets it to Hero view mode
	view_locked = false;
	//prevents the view mode from being changed through the menu

	respawn_time = 500;

	message.out("Welcome to Teams Arena!", 3000, 15);
	}

REGISTER_GAME(SuperArena, "Super Arena (.INI)")
