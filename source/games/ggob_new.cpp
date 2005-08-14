/* $Id$ */ 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include <allegro.h>

#ifdef ALLEGRO_MSVC
#pragma warning (disable:4786)
#endif

#include "../melee.h"
REGISTER_FILE
#include "../scp.h"
#include "../frame.h"

#include "../melee/mgame.h"
#include "../melee/mmain.h"
#include "../melee/mview.h"
#include "../melee/mcontrol.h"
#include "../melee/mcbodies.h"
#include "../melee/mshppan.h"
#include "../melee/mship.h"
#include "../melee/mshot.h"
#include "../melee/mlog.h"
#include "../melee/manim.h"
#include "../melee/mfleet.h"
#include "../gui.h"

#include "../util/aastr.h"

#include "ggob.h"
#include "../sc1ships.h"
#include "../sc2ships.h"

#include "other/gup.h"
#include "other/configrw.h"
#include "other/gconfig.h"
#include "ais/ext_ai.h"


/* 
   Using ini files is absolutly wrong approch for SAVING game, since it cant be used for saving planets, suns, enemy ships.
   It is hard to impruve and hard to maintain

   void GobGame::config(bool option)
   {
   config_read = option;

   GobPlayer *gp = gobplayer[0];
   Ship *s = 0;

   // store ship properties (includes its upgrades)
   section = "ship";

   if (option == CONFIG_READ)
   {
   Vector2 P;
   double a;

   char shipid[64];
   conf("name", shipid, "supbl");
   conf("x", P.x, 0);
   conf("y", P.y, 0);
   conf("a", a, 0);

   // needed, cause the ship initialization overwrites the config file used now
   push_config_state();
		
   //s = create_ship(shipid, gp->control, P, 0.0, 0);
   gp->new_ship(shiptype(shipid));

   pop_config_state();
   s = gp->ship;

   gp->ship = s;
   gp->ship->pos = P;
   gp->ship->angle = a;

   } else {
   s = gp->ship;
   conf("name", (char*)s->type->id, "supbl");
   conf("x", s->pos.x, 0);
   conf("y", s->pos.y, 0);
   conf("a", s->angle, 0);
   }

   conf("crew", s->crew, 1);
   conf("crewm", s->crew_max, 1);
   conf("batt", s->batt, 1);
   conf("battm", s->batt_max, 1);
   conf("accr", s->accel_rate, 1);
   conf("speedmax", s->speed_max, 1);
   conf("turnr", s->turn_rate, 1);
   conf("mass", s->mass, 1);
   conf("rechamount", s->recharge_amount, 1);
   conf("rechrate", s->recharge_rate, 1);
   conf("specrate", s->special_rate, 1);
   conf("specdrain", s->special_drain, 1);
   conf("weaprate", s->weapon_rate, 1);
   conf("weapdrain", s->weapon_drain, 1);
   conf("damfac", s->damage_factor, 1);
   //	conf("", s-, 1);

   section = "props";
   conf("kills", gp->kills, 0);
   conf("bucka", gp->buckazoids, 0);
   conf("starb", gp->starbucks, 0);
   conf("vbucka", gp->value_buckazoids, 0);
   conf("vstarb", gp->value_starbucks, 0);

   section = "special";
   if (strcmp("supbl", s->type->id))
   {		
   conf("damage", ((SupoxBlade*)s)->weaponDamage, 1);
   conf("drain", ((SupoxBlade*)s)->weapon_drain, 1);
   conf("armour", ((SupoxBlade*)s)->weaponArmour, 1);
   }


   // hmm .. well .. there are lots of special ships and devices, so
   // this is not all, by far, yet !!

   //	conf("", s-, 1);
   //	conf("", s-, 1);
   //	conf("", s-, 1);

   s->vel = 0;


   section = "upgrades";

   // each player keeps track of its own set of upgrades.
   // hmm, and what if one of the upgrades gets deleted ?
   int i;
   for (i = 0; gp->upgrade_list[i]; i += 1)
   {
   confnum("num", i, gp->upgrade_list[i]->num);
   }
   }


   void GobGame::save_game()
   {
   // write player settings to some config file ?
   set_config_file("gamedata/gob/player.ini");
   config(CONFIG_WRITE);
   flush_config_file();
   }

   void GobGame::load_game()
   {
   set_config_file("gamedata/gob/player.ini");
   config(CONFIG_READ);
   }


   void GobGame::quit(const char *message)
   {
   //save_game();
   Game::quit(message);
   }

*/

#define gobgame ((GobGame*)game)

////////////////////////////////////////////////////////////////////////
//				Gob stuff
////////////////////////////////////////////////////////////////////////

/*! \brief save game */
/*
  void GobGame::save_game2()
  {
  // this is very hard task and unfortunatly it does not support by program architecture

  }
*/



/*! \brief Player who kill asteroid get his bukazoid.
  \param source ??????
  \param normal normal damage 
  \param direct direct damage
  \return total damage to asteroid
*/
int GobAsteroid::handle_damage (SpaceLocation *source, double normal, double direct) 
{
  STACKTRACE;

  if (!exists()) return 0;
  int i = Asteroid::handle_damage(source, normal, direct);
  if (!exists()) {
	GobPlayer *p = gobgame->get_player(source);
	if (p) p->buckazoids += 1;
  }
  return i;
}

/*! \brief After asteroid death show explosion and generate new asteroid */
void GobAsteroid::death () 
{
  STACKTRACE;

  Animation *a = new Animation(this, pos,
							   explosion, 0, explosion->frames(), time_ratio, get_depth());
  a->match_velocity(this);
  game->add(a);

  game->add ( new GobAsteroid() );
  return;
}

/*! \brief Init various gob variables and stations sprites to NULL */ 
void GobGame::preinit() 
{
  STACKTRACE;

  Game::preinit();

  //  gobplayers = 0;
  //gobplayer = NULL;
  max_enemies = 0;
  //gobenemy = NULL;

  defenderSprite = NULL;
}

/*! \brief Add player to game
  \param control Player control
*/
void GobGame::add_gobplayer(Control *control) 
{
  STACKTRACE;

  GobPlayer * p = new GobPlayer();
  p->init(control, new_team(), this);
  gobplayer.push_back(p);

  add_focus(control, control->channel);
  return;
}

/*! \brief Handle player death
  At this point divinefavor upgrade can save player from such fate
  \param killer ???
*/
void GobPlayer::died(SpaceLocation *killer) 
{
  STACKTRACE;

  if (upgrade_list[UpgradeIndex::divinefavor]->num && (random()&1)) { //divine favor
	ship->crew = ship->crew_max;
	ship->batt = ship->batt_max;
	ship->translate(random(Vector2(-2048,-2048), Vector2(2048,2048)));
	ship->state = 1;
  }
  else ship = NULL;
  return;
}

/*! \brief ???
  \param sample ???
  \param source ???
  \param vol ???
  \param freq ???
*/
void GobGame::play_sound (SAMPLE *sample, SpaceLocation *source, int vol, int freq) 
{
  STACKTRACE;

  double v;
  Vector2 d = source->normal_pos() - space_center;
  d = normalize(d + size/2, size) - size/2;
  v = 1000;
  if (space_zoom > 0.01) v = 500 + space_view_size.x / space_zoom / 4;
  v = 1 + magnitude_sqr(d) / (v*v);
  Game::play_sound(sample, source, iround(vol/v), freq);
}

/*! \brief Init Game Stuff
  Alloc memory for enemies, load game settings, load various game
  sprites, set Hero view, include some hard understandable code for network 
  play
  \param _log Game log I think
*/
void GobGame::init(Log *_log) 
{
  STACKTRACE;

  unsigned int i;
  Game::init(_log);

  log_file("server.ini");
  max_enemies = get_config_int("Gob", "MaxEnemies", 32);


  size = Vector2(24000, 24000);

  enemy_team = new_team();

  // load objects from config file

  lua_State * ls = lua_open();
  InitInitializeModule(ls);
  luaopen_base(ls);
  luaopen_table(ls);
  luaopen_io(ls);
  luaopen_string(ls);
  luaopen_math(ls);
  luaopen_debug(ls);

  lua_dofile (ls, "gamedata/gob/config.lua");

  lua_close(ls);

  prepare();

  add(new Stars());

  for (i = 0; i < 19; i += 1) add(new GobAsteroid());


  //  int ichoice = 2;	// default, "no" 'don't load a game

  /*
	if (!lag_frames)
	{
	// check a menu to see what the player wants ...
	// (but only if it's not a networked game)
	ichoice = tw_alert("Continue saved game?", "&YES", "&NO");
	}

	if (ichoice == 2)
	{
  */
  set_config_file("client.ini");

  int p;
  for ( p = 0; p < num_network; ++p )
	{

	  
	  char buffy[256];
	  sprintf(buffy, "Config%d", p);
	  
	  //add_gobplayer(create_control(channel_network[p], "Human", buffy));
	  add_gobplayer(player[p]->control);	// this was initialized in the Game
	}

  num_players = num_network;	// this gets rid of the bots.
	  
  for (std::list<GobPlayer*>::iterator ip = gobplayer.begin(); 
	   ip != gobplayer.end(); ip++)
	{
	  (*ip)->new_ship(shiptype("supbl"));
	  Ship *s = (*ip)->ship;
	  s->translate( size/2 - s->normal_pos() );

	  s->locate();	// locates you somewhere at random.

	  double angle = (PI2 * i) / num_network;
	  s->translate(rotate(Vector2(260, 120), angle));
	  s->accelerate(s, PI2/3 + angle, 0.17, MAX_SPEED);	
	}

  //	} 

  /*
	else if (ichoice == 1) {

	// just supports ONE player.

	set_config_file("client.ini");

	for (i = 0; i < 1; i += 1) {
	char buffy[256];
	sprintf(buffy, "Config%d", i);
	add_gobplayer(create_control(channel_server, "Human", buffy));
	}
	num_players = 1;
		
	load_game();
	} else {
	tw_error("Strange, this option is not supported");
	}
  */

  for (i = 0; i < gobplayer.size(); i += 1) add ( new RainbowRift() );

  next_add_new_enemy_time = 1000;
  add_new_enemy();
  this->change_view("Hero");
  view_locked = true;
  view->window->locate(
					   0,0,
					   0,0,
					   0,0.9,
					   0,1
					   );

  quest_source = new QuestSource();
  quest_source->LoadQuestList( "gamedata/TestQuestSource.lua" );
  quest_source->GetQuest("gamedata/SecretPlanet.lua", *(gobplayer.begin()));
  return;
}

/*! \brief Free game resources */
GobGame::~GobGame() 
{
  delete defenderSprite;

  for (std::list<GobPlayer*>::iterator ip = gobplayer.begin();
	   ip != gobplayer.end(); ip++)
	{
	  delete *ip;
	}
	
  for (std::list<GobEnemy*>::iterator ie = gobenemy.begin();
	   ie != gobenemy.end(); ie++)
	{
	  delete *ie;
	}

  delete quest_source;
  return;
}


/*! \brief Print game information: enemies count, time, cordinates, money */
void GobGame::fps() 
{
  STACKTRACE;

  Game::fps();

  message.print((int)msecs_per_fps, 15, "enemies: %d", gobenemy.size());
  message.print((int)msecs_per_fps, 15, "time: %d", game_time / 1000);

  int i = 0;

  for (std::list<GobPlayer*>::iterator ip = gobplayer.begin();
	   ip != gobplayer.end(); ip++)
	{
	  if (!is_local((*ip)->channel)) continue;

	  if ((*ip)->ship) {
		message.print((int)msecs_per_fps, 15-i, "coordinates: %d x %d", 
					  iround((*ip)->ship->normal_pos().x), 
					  iround((*ip)->ship->normal_pos().y));
	  }
	  message.print((int)msecs_per_fps, 15-i, "starbucks: %d", (*ip)->starbucks);
	  message.print((int)msecs_per_fps, 15-i, "buckazoids: %d", (*ip)->buckazoids);
	  message.print((int)msecs_per_fps, 15-i, "kills: %d", (*ip)->kills);
	}
  return;
}

/*! \brief Add enemy to game 
  This function called once per frame of physics, used to add new enemys to the game
*/
void GobGame::calculate() 
{
  STACKTRACE;

  if (!lag_frames)	// if it's not networked?
	quest_source->ProcessQuests();

  if (next_add_new_enemy_time <= game_time) {
	next_add_new_enemy_time = game_time;
	int t = 28;
	if ((random(t)) < 4) add_new_enemy();
	int e = gobenemy.size();
	e -= random(1 + game_time / (250 * 1000));
	if (0) ;
	else if (e >=12) next_add_new_enemy_time += 15000;
	else if (e >= 7) next_add_new_enemy_time += 7000;
	else if (e >= 4) next_add_new_enemy_time += 5000;
	else if (e >= 2) next_add_new_enemy_time += 3000;
	else if (e >= 1) next_add_new_enemy_time += 2000;
	else next_add_new_enemy_time += 1000;
  }
  Game::calculate();
  return;
}

/*! \brief Search for requested enemy ship
  \param what space location of enemy ship
  \return GobEnemy, or NULL if enemy not found
*/
GobEnemy* GobGame::get_enemy(SpaceLocation *what)
{STACKTRACE
   assert(what->isShip());
 Ship *s = what->ship;
 if (!s) return NULL;

 for (std::list<GobEnemy*>::iterator ie = gobenemy.begin();
	  ie != gobenemy.end(); ie++)
   {
	 if ((*ie)->ship == s) return *ie;
   }
 return NULL;
}

/*! \brief ship death
  \param who is killed
  \param source loacation that killed?
*/
void GobGame::ship_died(Ship *who, SpaceLocation *source) 
{
  STACKTRACE;

  EventShipDie esd;
  esd.victim        = who;
  esd.player_killer = get_player(source);
  gobgame->GenerateEvent(&esd);


  GobPlayer *p = this->get_player(who);
  if (p && (p->ship == who)) { //Player died
	EventPlayerDied esd;
	esd.player = p;
	gobgame->GenerateEvent(&esd);
	p->died(source);
  }

  if ( get_enemy(who) != NULL)
	{
	  GobEnemy *e = get_enemy(who);
	  e->died(source);
	  gobenemy.remove(e);
	}

  Game::ship_died(who, source);
  return;
}

/*! \brief Get Player from location
  \param what player location
  \return player, NULL if no player
*/
GobPlayer *GobGame::get_player(SpaceLocation *what) 
{
  STACKTRACE;

  if ( what == NULL )
	return NULL;

  for (std::list<GobPlayer*>::iterator ip = gobplayer.begin();
	   ip != gobplayer.end(); ip++)
	{
	  if (what->get_team() == (*ip)->team) return *ip;
	}
  return NULL;
}

/*! \brief Create enemy ship
  Create random enemy ship if enemy limit is not riched. Also it patch some of the ships.
*/
void GobGame::add_new_enemy (std::string type, Vector2* pos ) 
{
  STACKTRACE;

	
  static const char *enemy_types[] = {
	"thrto", "zfpst", "shosc", "dragr", 
	"ktesa", "kahbo", "ilwsp", 
	"syrpe", "kzedr", "mmrxf", 
	"lk_sa", "druma", "earcr", 
	"yehte", "herex", "virli", 
	"chmav", "plopl", "narlu"
  };
  const int num_enemy_types = sizeof(enemy_types)/sizeof(enemy_types[0]);

  std::map <std::string, const char *> dialogMap;
  dialogMap["thrto"] = "gamedata/thraddash.lua";
  dialogMap["zfpst"] = "gamedata/zoqfot.lua";
  dialogMap["shosc"] = "gamedata/shofixty.lua";
  dialogMap["dragr"] = NULL;
  dialogMap["ktesa"] = NULL;
  dialogMap["kahbo"] = NULL;
  dialogMap["ilwsp"] = "gamedata/ilwrath.lua";
  dialogMap["syrpe"] = "gamedata/syreen.lua";
  dialogMap["kzedr"] = "gamedata/urquan.lua";
  dialogMap["mmrxf"] = NULL;
  dialogMap["lk_sa"] = NULL;
  dialogMap["druma"] = "gamedata/druuge.lua";
  dialogMap["earcr"] = "gamedata/human.lua";
  dialogMap["yehte"] = "gamedata/yehat.lua";
  dialogMap["herex"] = NULL;
  dialogMap["virli"] = NULL;
  dialogMap["chmav"] = "gamedata/chmmr.lua"; 
  dialogMap["plopl"] = NULL;
  dialogMap["narlu"] = NULL;

  if (gobenemy.size() == max_enemies) return;
  GobEnemy *ge = new GobEnemy();

  int base = game_time / 30 / 1000;
  if (gobenemy.size() >= 4) 
	base += (gobenemy.size()*gobenemy.size() - 10) / 5;
  gobenemy.push_back(ge);

  base = iround(base / 1.5);
  int e = 99999;
  while (e >= num_enemy_types) {
	e = base;
	e = random(e + 2);
	e = random(e + 3);
	if (e < pow(2.5*base,0.4) - 1) 
	  e = random(num_enemy_types);
	if (e > sqrt( 3.0*base) + 2) 
	  e = random(e + 1);
	e = e;
  }

  if (type != "")
	{
	  int i;
	  for (i=0; i<num_enemy_types; i++)
		{
		  if (type == enemy_types[i])
			{
			  e = i;
			  break;
			}
		}
	}
  Ship *ship;
  if (pos == NULL)
	ship = create_ship(channel_server, enemy_types[e], "WussieBot", random(size), random(PI2), enemy_team);
  else
	ship = create_ship(channel_server, enemy_types[e], "WussieBot", *pos, random(PI2), enemy_team);

  ship->install_external_ai(dialogMap[std::string(enemy_types[e])]);

  if (!strcmp(enemy_types[e], "shosc")) 
	{
	  ((ShofixtiScout*)ship)->specialDamage /= 4;
	}
  if (!strcmp(enemy_types[e], "zfpst")) 
	{
	  ((ZoqFotPikStinger*)ship)->specialDamage /= 2;
	}
  if (!strcmp(enemy_types[e], "syrpe")) 
	{
	  ((SyreenPenetrator*)ship)->specialDamage /= 2;
	}
  if (!strcmp(enemy_types[e], "dragr")) 
	{
	  ship->special_drain *= 2;
	}
  if (!strcmp(enemy_types[e], "chmav")) {
	((ChmmrAvatar*)ship)->weaponDamage += 1;
	((ChmmrAvatar*)ship)->weaponDamage /= 2;
	((ChmmrAvatar*)ship)->specialForce *= 2;
	((ChmmrAvatar*)ship)->specialRange *= 2;
  }
  int sb, bz;
  sb = 1 + e / 4;
  if (sb > 2) sb -= 1;
  bz = (e - 9) / 2;
  if (bz > 1) bz -= 1;
  if (sb < 0) sb = 0;
  if (bz < 0) bz = 0;
  ge->init(ship, sb, bz);
  add(ship->get_ship_phaser());
  //add(ship);
  return;
}
/*! \brief Set amount starbucks and bukazoids for death
  \param ship enemy ship
  \param kill_starbucks amount starbucks
  \param kill_buckazoids amount buckazoids
*/
void GobEnemy::init(Ship *ship, int kill_starbucks, int kill_buckazoids) 
{
  STACKTRACE;

  this->ship = ship;
  this->starbucks = kill_starbucks;
  this->buckazoids = kill_buckazoids;
  return;
}
/*! \brief Give player his bounty
  \param what location of player
*/
void GobEnemy::died(SpaceLocation *what) {
  STACKTRACE;

  GobPlayer *p = gobgame->get_player(what);
  if (p) {
	p->starbucks += starbucks;
	p->buckazoids += buckazoids;
	p->kills += 1;
  }
  return;
}

/*! \brief Free player resources */
GobPlayer::~GobPlayer() 
{
}

/*! brief Init player 
  \param c player control
  \param team player team
*/
void GobPlayer::init(Control *c, TeamCode team, GobGame * g) 
{
  STACKTRACE;

  channel = c->channel;
  starbucks = 0;
  buckazoids = 0;
  kills = 0;
  value_starbucks = 0;
  value_buckazoids = 0;
  ship = NULL;
  panel = NULL;
  control = c;
  total = 0;
  this->team = team;
  int i, j;
  for (i = 0; ::upgrade_list[i]; i += 1) ::upgrade_list[i]->index = i;
  upgrade_list = new Upgrade*[i+1];
  upgrade_list[i] = NULL;
  for (j = 0; j < i; j += 1) {
	upgrade_list[j] = ::upgrade_list[j]->duplicate();
	upgrade_list[j]->clear(NULL, NULL, this);
  }
  return;
}


/*! \brief Conform purch, take charge for purch upgrade
  \param name upgrade name
  \param price_starbucks price
  \param price_buckazoids price
*/
int GobPlayer::charge (char *name, int price_starbucks, int price_buckazoids) 
{
  STACKTRACE;

  char buffy1[512];
  sprintf(buffy1, "Price: %d starbucks plus %d buckazoids", price_starbucks, price_buckazoids);
  if ((starbucks < price_starbucks) || (buckazoids < price_buckazoids)) {
	if (is_local(channel)) 
	  alert("You don't have enough.", name, buffy1, "Cancel", NULL, 0, 0);
	return 0;
  }
  int r = 0;
  if (is_local(channel)) 
	r = alert ("Do you wish to make this purchase?", name, buffy1, "&No", "&Yes", 'n', 'y');
  log_int(r, channel);
  if (r == 2) {
	starbucks -= price_starbucks;
	buckazoids -= price_buckazoids;
	return 1;
  }
  return 0;
}

/*! \brief Create new ship, remove upgrades
  \param type type of new ship
*/
void GobPlayer::new_ship(ShipType *type) 
{
  STACKTRACE;

  Ship *old = ship;
  Vector2 pos = 0;
  double a = 0;
  int i;
  if (old) {
	pos = old->normal_pos();
	a = old->get_angle();
  }

  ship = game->create_ship ( type->id, control, pos, a, team);

  if (panel) panel->die();
  panel = NULL;
  panel = new ShipPanel(ship);
  panel->always_redraw = true;
  panel->window->init(game->window);
  if (is_local(control->channel)) {
	panel->window->locate(
						  0,0.9,
						  0,0,
						  0,0.1,
						  0,0.25
						  );
  }
  else {
	panel->window->locate(
						  0,0.9,
						  0,0.25,
						  0,0.1,
						  0,0.25
						  );
  }
  panel->set_depth(10);
  game->add(panel);

  for (i = 0; upgrade_list[i]; i += 1) {
	upgrade_list[i]->clear(old, ship, this);
  }
  if (old) {
	old->die();
	game->add(ship);
  }
  else game->add(ship);//->get_ship_phaser());

  game->set_focus(ship->control);

  return;
}

/*! \brief Check money, ask questions
  \param s player
*/
void GobStation::buy_new_ship_menu(GobPlayer *s) 
{
  STACKTRACE;

  char buffy1[512], buffy2[512];
  ShipType *otype = s->ship->type;
  ShipType *ntype = shiptype(build_type);
  if (otype == ntype) {
	sprintf (buffy1, "You already have a %s", ntype->name);
	if (is_local(s->channel)) 
	  alert(buffy1, NULL, NULL, "&Cancel", NULL, 'c', 0);
	return;
  }
  int ossb = (s->value_starbucks*3) / 4 + (s->ship->type->cost*1)/1;
  int osbz = (s->value_buckazoids*3) / 4 + (s->ship->type->cost*1)/1;
  int nssb = ntype->cost;
  int nsbz = ntype->cost;
  sprintf (buffy1, "You have a %s worth %d s$ / %d b$", otype->name, ossb, osbz);
  sprintf (buffy2, "A %s costs %d s$ / %d b$", ntype->name, nssb, nsbz);
  if ((nssb <= (ossb + s->starbucks)) && (nsbz <= (osbz + s->buckazoids))) {
	int i = 0;
	if (is_local(s->channel))
	  i = alert(buffy1, buffy2, "Do you wish to buy it?", "Yeah!", "No", 'y', 'n');
	log_int(i, s->channel);
	if (i == 1) {
	  s->starbucks -= nssb - ossb;
	  s->buckazoids -= nsbz - osbz;
	  s->new_ship(ntype);
	}
  }
  else {
	if (is_local(s->channel)) 
	  alert (buffy1, buffy2, "You don't have enough to buy it", "Cancel", NULL, 0, 0);
  }
  return;
}
/*! \brief Create station 
  \param pic ???
  \param orbit_me ???
  \param ship ???
  \param background ???
  \param sname station name
*/
GobStation::GobStation ( SpaceSprite *pic, SpaceLocation *orbit_me, 
						 const char *ship, const char *background, std::string sname ) : 
  Orbiter(pic, orbit_me, random(200) + 500) 
{
  build_type = ship;
  background_pic = background;
  layer = LAYER_CBODIES;
  mass = 99;
  name = sname;
  if (sname=="Kohr-Ah")
	set_team(gobgame->enemy_team);
}

GobStation::~GobStation()
{
}

enum {
  STATION_DIALOG_DEPART = 0,
  STATION_DIALOG_UPGRADE, 
  STATION_DIALOG_NEWSHIP, 
  STATION_DIALOG_REPAIR,  
  STATION_DIALOG_COMMANDER, 
  //  STATION_DIALOG_SAVE,    
};

static DIALOG station_dialog[] =
  {// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)  (d2)  (dp)
	{ d_button_proc,     385,  50,   150,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Depart Station" , NULL, NULL },//STATION_DIALOG_DEPART
	{ my_d_button_proc,  385,  90,   150,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Upgrade Ship" , NULL, NULL },//STATION_DIALOG_UPGRADE
	{ my_d_button_proc,  385,  130,  150,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Buy New Ship" , NULL, NULL },//STATION_DIALOG_NEWSHIP
	{ my_d_button_proc,  385,  170,  150,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Repair Ship" , NULL, NULL },//STATION_DIALOG_REPAIR
	{ my_d_button_proc,  385,  210,  150,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Commander" , NULL, NULL },//STATION_DIALOG_COMMANDER
	//{ my_d_button_proc,  385,  250,  150,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Save Game" , NULL, NULL },//STATION_DIALOG_SAVE
	{ d_text_proc,       185,  420,  270,  30,   255,  0,    0,    0,          0,    0,    dialog_string[0], NULL, NULL },
	{ d_tw_yield_proc,        0,    0,    0,    0,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,    255,  0,    0,    0,          0,    0,    NULL, NULL, NULL }
  };

/*! \brief Process station dialog
  \param s Player at the station
*/
void GobStation::station_screen(GobPlayer *s) 
{
  STACKTRACE;

  BITMAP *background = load_bitmap(background_pic.c_str(), NULL);
  if (!background) {
	message.print(1000, 15, "%s", background_pic.c_str());
	error ("couldn't load station background");
  }
  game->window->lock();
  aa_set_mode(AA_DITHER);
  aa_stretch_blit(background, game->window->surface, 
				  0,0,background->w,background->h, 
				  game->window->x,game->window->y,game->window->w, game->window->h);
  game->window->unlock();

  while (true) {
	sprintf(dialog_string[0], "%03d Starbucks  %03d Buckazoids", s->starbucks, s->buckazoids);
	int r = 0;
	if (is_local(s->channel)) 
	  r = tw_do_dialog(game->window, station_dialog, STATION_DIALOG_DEPART);
	log_int(r, s->channel);
	switch (r) {
	  case STATION_DIALOG_UPGRADE: {
		upgrade_menu(this, s);
		aa_set_mode(AA_DITHER);
		aa_stretch_blit(background, game->window->surface, 
						0,0,background->w,background->h, 
						game->window->x,game->window->y,
						game->window->w, game->window->h);
	  }
		break;
	  case STATION_DIALOG_NEWSHIP: {
		buy_new_ship_menu(s);
	  }
		break;
	  case STATION_DIALOG_REPAIR: { 
		if (s->ship->crew == s->ship->crew_max) {
		  if (is_local(s->channel)) 
			alert("You don't need repairs", "", "", "Oh, okay", "I knew that", 0, 0);

		  break;
		}
		int p = 0;
		if (is_local(s->channel)) 
		  p = alert3("Which would you prefer", "to pay for your repairs", "", "1 &Starbuck", "1 &Buckazoid", "&Nothing!", 's', 'b', 'n');
		log_int(p, s->channel);
		switch (p) 
		  {
			case 1:
			  if (s->starbucks) 
				{
				  s->starbucks -= 1;
				  s->ship->crew = s->ship->crew_max;
				}
			  else 
				{
				  if (is_local(s->channel)) 
					alert("You don't have enough!", NULL, NULL, "&Shit", NULL, 's', 0);
				}
			  break;
			case 2: 
			  if (s->buckazoids) 
				{
				  s->buckazoids -= 1;
				  s->ship->crew = s->ship->crew_max;
				}
			  else 
				{
				  if (is_local(s->channel)) 
					alert("You don't have enough!", NULL, NULL, "&Shit", NULL, 's', 0);
				}
			  break;
			case 3: 
			  r = STATION_DIALOG_DEPART;
			  break;
		  }
	  }
		break;

	  case STATION_DIALOG_COMMANDER:
		if (ext_ai)
		  ext_ai->Dialog(s->ship);
		break;

		/*
		  case STATION_DIALOG_SAVE:
		  {
		  // saves the last game.
		  gobgame->save_game();
		  }
		*/

		break;

	}
	if (r == STATION_DIALOG_DEPART) break;
  }
  return;
}

/*! \brief  Meet with station
  \param other location of object that colide with station
*/
void GobStation::inflict_damage(SpaceObject *other) 
{
  STACKTRACE;

  SpaceObject::inflict_damage(other);
  if (!other->isShip()) return;
  GobPlayer *p = gobgame->get_player(other);
  if (!p) return;

  gobgame->pause();

  EventEnterStation e;
  e.player  = p;
  e.station = this;
  gobgame->GenerateEvent(&e);

  if (get_team()!=gobgame->enemy_team)
	station_screen(p);

  gobgame->unpause();
  return;
}

int num_upgrade_indexes;
int upgrade_index[999];
GobPlayer *upgrade_list_for;

/*! \brief ???
  \param index ???
  \param list_size ???
*/
char *upgradeListboxGetter(int index, int *list_size) 
{
  STACKTRACE;

  static char tmp[150];
  if(index < 0) {
	*list_size = num_upgrade_indexes;
	return NULL;
  }
  int i = upgrade_index[index];
  sprintf(tmp, "%1d %3d s$ / %3d b$  :  %s", upgrade_list_for->upgrade_list[i]->num, upgrade_list_for->upgrade_list[i]->starbucks, upgrade_list_for->upgrade_list[i]->buckazoids, upgrade_list_for->upgrade_list[i]->name);
  return tmp;
}

#define UPGRADE_DIALOG_EXIT 0
#define UPGRADE_DIALOG_LIST 3
static DIALOG upgrade_dialog[] =
  {// (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)  (d2)  (dp)
	{ my_d_button_proc,  10,  415,  170,  30,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Station menu" , NULL, NULL },
	{ d_textbox_proc,    20,  40,   250,  40,   255,  0,    0,    D_EXIT,     0,    0,    (void *)"Upgrade Menu", NULL, NULL },
	{ d_text_proc,       10,  100,  540,  20,   255,  0,    0,    D_EXIT,     0,    0,    (void *)" # Starbucks Buckazoids Description                     ", NULL, NULL },
	{ d_list_proc,       10,  120,  540,  280,  255,  0,    0,    D_EXIT,     0,    0,    (void *) upgradeListboxGetter, NULL, NULL },
	{ d_text_proc,       185, 420,  270,  30,   255,  0,    0,    0,          0,    0,    dialog_string[0], NULL, NULL },
	{ d_tw_yield_proc,        0,    0,    0,    0,  255,  0,    0,    0,       0,    0,    NULL, NULL, NULL },
	{ NULL,              0,    0,    0,    0,   255,  0,    0,    0,          0,    0,    NULL, NULL, NULL }
  };

/*! \brief show upgrade dialog
  \param station station where player upgraded
  \param gs Player
*/
void GobStation::upgrade_menu(GobStation *station, GobPlayer *gs) 
{
  STACKTRACE;

  int i;
  upgrade_list_for = gs;
  clear_to_color(screen, palette_color[8]);
  while (true) {
	sprintf(dialog_string[0], "%03d Starbucks  %03d Buckazoids", gs->starbucks, gs->buckazoids);
	int j = 0;
	for (i = 0; gs->upgrade_list[i]; i += 1) {
	  if (gs->upgrade_list[i]->update(gs->ship, station, gs)) {
		upgrade_index[j] = i;
		j += 1;
	  }
	}
	num_upgrade_indexes = j;
	int m = 0;
	if (is_local(gs->channel))
	  m = tw_do_dialog(game->window, upgrade_dialog, UPGRADE_DIALOG_EXIT);
	log_int(m, gs->channel);
	if (m == UPGRADE_DIALOG_EXIT) return;
	if (m == UPGRADE_DIALOG_LIST) {
	  int i = 0;
	  if (is_local(gs->channel))
		i = upgrade_dialog[UPGRADE_DIALOG_LIST].d1;
	  log_int(i, gs->channel);
	  i = upgrade_index[i];
	  Upgrade *u = gs->upgrade_list[i];
	  if (gs->charge(u->name, u->starbucks, u->buckazoids)) {
		u->execute(gs->ship, station, gs);
		u->charge(gs);
	  }
	}
  }
  return;
}

/*! \brief ???
  \param ship ???
*/
GobDefender::GobDefender ( Ship *ship) 
  : SpaceObject (ship, ship->normal_pos(), 0, gobgame->defenderSprite)
{
  base_phase = 0;
  next_shoot_time = 0;
  collide_flag_anyone = 0;
}
/*! \brief AI for GobDefender 
  Called once per physics frame
*/
void GobDefender::calculate() 
{
  STACKTRACE;

  SpaceObject::calculate();
  if (!ship) {
	die();
	return;
  }
  if (next_shoot_time < gobgame->game_time) {
	SpaceObject *target = NULL;
	Query q;
	q.begin(this, OBJECT_LAYERS &~ bit(LAYER_SHIPS), 300);
	while (q.currento && !target) {
	  if (!q.currento->sameTeam(ship)) {
		SpaceLine *l = new PointLaser ( 
									   this, palette_color[4], 2, 150, 
									   this, q.currento
									   );
		add(l);
		if (l->exists()) target = q.currento;
	  }
	  q.next();
	}
	if (target) {
	  next_shoot_time = gobgame->game_time + 400;
	}
  }
  double a = base_phase + (gobgame->game_time % 120000) * ( PI2 / 1000.0) / 6;
  angle = normalize(a,PI2);
  pos = normalize(ship->normal_pos() + 270 * unit_vector ( angle ));
  return;
}
/*! \brief ??? */
RainbowRift::RainbowRift () 
  //: SpaceLocation ( NULL, 12800, 12800, 0) 
  : SpaceLocation ( NULL, random(map_size), 0) 
{
  int i;
  collide_flag_sameship = 0;
  collide_flag_sameteam = 0;
  collide_flag_anyone = 0;
  for (i = n*6-6; i < n*6+2; i += 1) {
	p[i] = 75 + random(150.0);
  }
  for (i = 0; i < n; i += 1) {
	squiggle();
  }
  next_time = game->game_time;
  next_time2 = game->game_time;
}
/*! \brief ??? 
  \param frame ???
*/
void RainbowRift::animate( Frame *frame ) 
{
  STACKTRACE;

  Vector2 s;
  s = corner(pos, Vector2(300,300));
  if ((s.x < -500) || (s.x > space_view_size.x + 500) || 
	  (s.y < -500) || (s.y > space_view_size.y + 500))
	return;
  int b[n*6+2];
  int i;
  for (i = 0; i < n*6+2; i += 2) { 
	b[i] = iround(s.x + p[i] * space_zoom);
	b[i+1] = iround(s.y + p[i+1] * space_zoom);
  }
  for (i = 0; i < n; i += 1) {
	RGB tc = c[n-i-1];
	int a = tw_color(tc.r, tc.g, tc.b);
	spline ( frame->surface, &b[i*6], a );
  }
  frame->add_box ( 
				  iround(s.x - 2), iround(s.y -2), 
				  iround(300 * space_zoom+5), iround(300 * space_zoom+5)
				  );
  return;
}
/*! \brief ??? */
void RainbowRift::squiggle() 
{
  STACKTRACE;

  int i;
  int m = n*6+2;
  for (i = 0; i < m - 6; i += 1) {
	p[i] = p[i+6];
  }
  p[m-6] = p[m-8] * 2 - p[m-10];
  p[m-5] = p[m-7] * 2 - p[m-9];
  p[m-4] = 75 + random(150.0);
  p[m-3] = 75 + random(150.0);
  p[m-2] = 75 + random(150.0);
  p[m-1] = 75 + random(150.0);
  for (i = 0; i < n-1; i += 1) {
	c[i] = c[i+1];
  }
  int r, g, b;
  r = int(game->game_time * 0.5) % 360;
  hsv_to_rgb( r, 1.0, 1.0, &r, &g, &b );
  c[n-1].r = r;
  c[n-1].g = g;
  c[n-1].b = b;
  return;
}

/*! \brief ??? */
void RainbowRift::calculate() 
{
  STACKTRACE;

  while (game->game_time > next_time) {
	next_time += 25;
	squiggle();
  }
  while (game->game_time > next_time2) {
	next_time2 += random(10000);
	Query q;
	for (q.begin(this, bit(LAYER_SHIPS), 40); q.current; q.next()) {
	  GobPlayer *p = gobgame->get_player(q.currento);
	  if (q.currento == p->ship) {
		int i = 0;
		i = p->control->choose_ship(game->window, "You found the Rainbow Rift!", reference_fleet);
		log_int(i, p->channel);
		if (i == -1) i = random(reference_fleet->getSize());
		game->redraw();
		if (reference_fleet->getShipType(i) == p->ship->type) {
		  p->starbucks += random(80);
		  p->buckazoids += random(80);
		  game->add(new RainbowRift());
		}
		else {
		  p->starbucks += random(1+p->value_starbucks);
		  p->buckazoids += random(1+p->value_buckazoids);
		  p->new_ship(reference_fleet->getShipType(i));
		}
		die();
	  }
	}
  }
  return;
}


REGISTER_GAME(GobGame, "GOB")

  
  
