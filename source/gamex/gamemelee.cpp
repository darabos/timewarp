

// basic melee type, less complicated than "normal" melee is needed I think...
// initing ships is complicated ... should use the normal game stuff perhaps ?



#include <allegro.h>
#include <stdio.h>

#include "../melee.h"
REGISTER_FILE

#include "../melee/mlog.h"
#include "../melee/mcontrol.h"
#include "../melee/mframe.h"
#include "../melee/mview.h"
#include "../melee/mitems.h"

#include "../scp.h"
#include "../util/history.h"


#include "gamesolarview.h"
#include "gamemelee.h"
#include "gameplanetscan.h"
#include "gamemelee.h"

#include <string>

#include "../sc2ships.h"

#include "../melee/mcbodies.h"



class InitShipGeneral
{
public:
	//InitShipGeneral(char *id, char *base);
	void set(char *id, char *base);
	char idname[128];
	std::string filebase;
	virtual Ship *spawnship(Vector2 opos, double oangle, int allyflag) {return 0;};
	ShipData *getdata();
};

void InitShipGeneral::set(char *id, char *base)
{
	strcpy(idname, id);
	filebase = base;
}

ShipData *InitShipGeneral::getdata()
{
	ShipData *sd;
	sd = shipdata( (filebase + ".dat").c_str() );	// check the list
	sd->load();		// load the sprites into the data (is only done if that's still needed).
	return sd;
}


class InitShipOrzne : public InitShipGeneral
{
public:
	InitShipOrzne() {set("orzne", "ships/sc2/shporzne");};
	virtual Ship *spawnship(Vector2 opos, double oangle, int allyflag)
		{return new OrzNemesis(opos, oangle, getdata(), allyflag);};
};



int Nships = 0;
InitShipGeneral *shiplist[64];

void add2shipslist(InitShipGeneral *s)
{
	shiplist[Nships] = s;
	++Nships;
}

void initshipslist()
{
	add2shipslist( new InitShipOrzne() );
}


Ship *createship(char *id, Vector2 opos, double oangle, int allyflag)
{
	int i;
	for ( i = 0; i < Nships; ++i )
	{
		if (strcmp(id, shiplist[i]->idname) == 0)
			return shiplist[i]->spawnship(opos, oangle, allyflag);
	}
	return 0;
}

#include "../melee.h"
#include "../melee/mframe.h"

Ship *GameMelee::create_ship(const char *id, Vector2 pos, double angle, int team)
{
	Control *c = 0;
	int channel = 0;

	c = getController("WussieBot", "whatever", channel);
	if (!c) {
		tw_error("Game::create_control - bad control type");
	}
//	c->load(file, config);
	add(c);


	ShipType *type = shiptype(id);
	if (!type)
	{tw_error("Game::create_ship - bad ship id (%s)", id);}
	/*if(!ini) {
		sprintf(buffer, "ships/shp%s.ini", id);
		ini = buffer;
	}	
	log_file(buffer);*/
//	log_file(type->file);
	set_config_file(type->file);
//	if (team == 0) team = new_team();
	Ship *s = type->get_ship(pos, angle, get_code(new_ship(), team));
	if (c)
		c->select_ship(s, id);
//	add_target(s);
	s->attributes |= ATTRIB_NOTIFY_ON_DEATH;
	return s;
}





void GameMelee::init_menu()
{
	// place the menu into video-memory, cause we're using this as basis for
	// drawing; the game draws onto part of the menu.
	T = new TWindow("gamex/interface/melee", 0, 0, game_screen, true);

	maparea = new Area(T, "map_");
}




void GameMelee::init()
{

	GameBare::init();

	double H = 2000;
	size = Vector2(H, H);
	
	prepare();


	// for stand-alone testing
	if (!enemyfleet)
	{
		enemyfleet = new XFleet();
		enemyfleet->add("orzne", 2);
		// this gives a memory leak, but only in stand-alone testing so I don't care.
	}


//	wininfo.init( Vector2(200,200), 800.0, tempframe );
	wininfo.zoomlimit(size.x);
	wininfo.scaletowidth(size.x);	// zoom out to this width.

	initshipslist();


	// Stars requires game->log, which I can't, nor need (imo), to support for a (single player) full-game 
	//physics = 0;
	//add(new Stars());
	//physics = this;

	int i;
	for ( i = 0; i < 10; ++i )
		add(new Asteroid());

	team_player = new_team();


	Ship *s;

	for ( i = 0; i < enemyfleet->Nships; ++i )
	{
		//s = createship(enemyfleet->info->name, Vector2(0,0), 0, team_player);
		s = create_ship(enemyfleet->info->name, Vector2(0,0), 0, team_player);
		s->layer = LAYER_SHIPS;
		s->collide_flag_anyone = ALL_LAYERS;
		add(s);
//		s->hashotspots = false;
	}


	// take control of one of those ships...
	player = s;

}



void GameMelee::handle_edge(SpaceLocation *s)
{
	// override the default deletion, and let the thingy re-appear as in normal
	// melee, on the other side.
	normalize(s->pos);
}


void GameMelee::quit()
{
	// mapwrap is only true in a melee game; disable it once you leave the melee game.
//	mapwrap = false;

	GameBare::quit();
}






void GameMelee::calculate()
{
	if (next)
		return;


	wininfo.center(player->pos);

	
	GameBare::calculate();


	// do some AI stuff: if control==0, this is the only thing that is setting the controls ...
//	int i;
//	for ( i = 0; i < num_items; ++i )
//	{
//		if (item[i]->isShip())
//			((Ship*)item[i])->AI();
//	}

	// the human controlled ship: override any AI orders

	player->thrust = false;
	player->turn_left = false;
	player->turn_right = false;
	player->fire_weapon = false;
	player->fire_special = false;

	if (key[KEY_UP])
		player->thrust = true;
	
	if (key[KEY_LEFT])
		player->turn_left = true;

	if (key[KEY_RIGHT])
		player->turn_right = true;

	if (key[KEY_ENTER])
		player->fire_weapon = true;

	if (key[KEY_RSHIFT])
		player->fire_special = true;



	// zoom in/out of the map.

	double dt;
	dt = frame_time * 1E-3;

	if (key[KEY_EQUALS])
		wininfo.zoom(1 + 1*dt);

	if (key[KEY_MINUS])
		wininfo.zoom(1 / (1 + 1*dt));

}

void GameMelee::animate(Frame *frame)
{
	if (next)
		return;

	//message.print(1500, 15, "%i %i", int(cg.space_center.x), int(cg.space_center_nowrap.x));

	GameBare::animate(frame);

}


void GameMelee::set_xfleet(XFleet *f)
{
	enemyfleet = f;
}



GameMelee::GameMelee()
{
	enemyfleet = 0;
}


GameMelee::~GameMelee()
{
}



