#ifndef _MELEE_H
#include "../melee.h"
#endif
#ifndef _MGAME_H
#include "mgame.h"
#endif

#ifndef _MMAIN_H
#define _MMAIN_H

class NormalGame : public Game {
	public:
	enum { 
		num_asteroids = 4 
		};
		
	~NormalGame();

	virtual void calculate();
	virtual void preinit();	
	virtual void init (Log *_log = NULL);
	virtual void set_resolution(int screen_x, int screen_y);

	virtual void init_players();
	virtual void init_objects();

	virtual void ship_died(Ship *who, SpaceLocation *source);
	//kill history stuff
	int num_kills;
	struct ShipKill {
		int time;
		struct Party {
			unsigned int ally_flag;
			ShipData *data;
			ShipType *type;
		};
		Party victim, killer;
		} *kills;
	void display_stats();

	virtual int add_player (Control *c, int team_index, const char *name, const char *fleet, const char *fleet_file = "fleets.ini") ;
	int num_players;
	Control **player_control;
//	ShipPanel **player_panel;
	Fleet **player_fleet;
	char **player_name;
	char *player_attributes;
	TeamCode *player_team;
	TeamCode *team_table;
	int team_table_size;
//	virtual void player_said(int who, const char *what);

	protected:
	virtual bool handle_key(int k);

	int next_choose_new_ships_time;
	void choose_new_ships() ;

	};


#endif
