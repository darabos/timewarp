#ifndef __MMAIN_H__
#define __MMAIN_H__

#include "../melee.h"
#include "mgame.h"

int log_size_pl(int netnum);
int log_totalsize();
void log_test(char *comment);

// normal player information.
class NPI : public PlayerInformation
{
public:
	Fleet *fleet;
};



class DirectConnection
{
	int conn_remote;

public:

	bool isserver;
	
	bool set(int i);
	
	void get(void *data, int N);
	void send(void *data, int N);

	void exchange(void *data, int N);		// host and client send each other data
	void server(void *data, int N);		// host sends data (client receives) --> host data are copied
	void client(void *data, int N);		// client sends data (host receives) --> client data are copied

	void exchange(int &x);
	void server(int &x);
	void client(int &x);


};

extern DirectConnection direct;


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

	virtual int add_player (int num, Control *c, int team_index, const char *name, const char *fleet, const char *fleet_file = "fleets.ini") ;
	//int num_players;		should be global, for networking (which should also be global)
//	Control **player_control;
//	ShipPanel **player_panel;
//	Fleet **player_fleet;
//	char **player_name;
	char *player_attributes;
//	TeamCode *player_team;
	TeamCode *team_table;
	int team_table_size;
//	int *player_color;
//	virtual void player_said(int who, const char *what);

	protected:
	virtual bool handle_key(int k);

	int next_choose_new_ships_time;
	virtual void choose_new_ships() ;

	int indhealthtoggle, indteamtoggle;

	bool check_end();
	void handle_end();
	bool player_isalive(int i);
	int local_player();


	void check_file(const char *id, int iplayer);
	void download_file(char *filename);

	virtual PlayerInformation *new_player();	// should return a pointer to a new player-class
	};


#endif // __MMAIN_H__
