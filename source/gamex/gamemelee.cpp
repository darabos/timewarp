

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
#include "GameMelee.h"
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



void GameMelee::init()
{

	GameBare::init();

	double H = 2000;
	size = Vector2(H, H);

	view->window->locate(
		0, 0,
		0, 0,
		0, 0.75,
		0, 1.0
		);
	
	prepare();


	// for stand-alone testing
	if (!enemyfleet)
	{
		enemyfleet = new XFleet();
		enemyfleet->add("orzne", 2);
		// this gives a memory leak, but only in stand-alone testing so I don't care.
	}

	//space_center_nowrap = 0;
	cg.reset();	// resets all global/cached values to 0.

	
//	mapwrap = true;

	wininfo.init( Vector2(200,200), 800.0, view->frame );
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
		s = createship(enemyfleet->info->name, Vector2(0,0), 0, team_player);
		s->layer = LAYER_SHIPS;
		s->collide_flag_anyone = ALL_LAYERS;
		add(s);
//		s->hashotspots = false;
	}


	// take control of one of those ships...
	player = s;



	cg.space_center = wininfo.mapcenter;
	cg.space_zoom = wininfo.zoomlevel;
	cg.space_view_size = Vector2(view->window->w, view->window->h);
	cg.space_center_nowrap = cg.space_center;

	cg.setglobals();
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







void CacheGlobals::setglobals()
{
	::space_center = this->space_center;
	::space_view_size = this->space_view_size;
	::space_zoom = this->space_zoom;
	::space_center_nowrap = this->space_center_nowrap;
}

void CacheGlobals::reset()
{
	this->space_center = 0;
	this->space_view_size = 0;
	this->space_zoom = 0;
	this->space_center_nowrap = 0;
}


void GameMelee::calculate()
{
	if (next)
		return;


	wininfo.center(player->pos);

	cg.space_center = player->pos;
	cg.space_center_nowrap += player->vel * frame_time;
	cg.space_zoom = wininfo.zoomlevel;
	cg.space_view_size = Vector2(view->window->w, view->window->h);

	cg.setglobals();
	
	
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

	cg.setglobals();

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



