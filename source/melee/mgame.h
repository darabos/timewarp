/* $Id$ */ 
#ifndef __MGAME_H__
#define __MGAME_H__

#include "mframe.h"
#include "mtarget.h"

extern int random_seed[2];
extern int interpolate_frames;

extern bool detailed_network_check;

void idle_extra(int time = 5);	// adding extra network-share to avoid total standstill which may happen, very occasionally...

class GameType {
	private:
	Game *(*_new_game)();
	public:
	double order;
	char *name;
	Game *new_game();
	GameType ( const char *name, Game *(*new_game)(), double order = 0);
	};
extern int num_games;
extern char **game_names;
extern GameType **games;
#define REGISTER_GAME(b, a) static Game *new_ ## b () {return new b();} static GameType ignoreme_ ## b( a, new_ ## b, 0);
GameType *gametype(const char *name);


// these are default data used in ships and game objects, which in principle, should be
// independent of the (normal) game type.
class MeleeData
{
public:
	MeleeData();

	void init();
	void deinit();
	
	SpaceSprite *planetSprite;
	SpaceSprite *asteroidSprite;
	SpaceSprite *asteroidExplosionSprite;
	SpaceSprite *sparkSprite;
	SpaceSprite *hotspotSprite;
	SpaceSprite *kaboomSprite;
	SpaceSprite *panelSprite;
	SpaceSprite *xpl1Sprite;

	Music *planet_victory;
};


const int max_network = 100;

extern const int channel_none;    //not a valid channel
extern const int _channel_buffered;
extern const int channel_init;     // game type, version, length, etc.. things that need to read by a reader independant of a particular game type
extern const int channel_playback; // used for demo playbacks only
// THE SERVER IS THE ONE WHO DEFINES THE DATA (eg initial game conditions; log_files)
// the server is always player 0.
extern const int channel_server;  //data originating on the server
// THE LOCAL CHANNEL IS THE ONE BELONGING TO THE COMPUTER YOU'RE SITTING AT; all other channels are remote
// the local channel is always read/write, while remote channels are always read-only.
// each guy who logs in to a game (defined by the server) receives a local-channel number from the server.
// (eg. player 1, player 2 .. player 7).
extern int channel_network[max_network];  //data originating on the client
extern int p_local, num_network, num_hotseats[max_network], num_bots, num_players;

extern int channel_conn_recv[100];
// The connection id, where a channel last received data from
// This can be used to check which channel belongs to which physical connection.

class PlayerInformation
{
public:
	int channel;
// channels used by players in the game.
// note, that, while channel_network has unique values, the player_channel can have
// duplicate values, because different (hotseat) players share the same
// network channel.

	char name[64];
	Control *control;
	TeamCode team;
	int color;
	bool status;	// true=alive, false=disconnected/dead

	bool haschannel(int ch);	// checks for channel and the buffered channel
	void die();
};
extern PlayerInformation *player[100];


int channel_local();
void init_channels();



class GameEvent2
{
public:
	virtual void calculate(){};
};


template <class G>
class GE : public GameEvent2
{
public:
	typedef void (G::* reffunc) ();	// store the class-function pointer
	G *game;						// store pointer to the class in memory

	reffunc ref;

	GE(G *agame, reffunc aref) : GameEvent2() { game = agame; ref = aref; };
	
	virtual void calculate() {(game->*ref)();};
};

const int max_events = 100;
const int max_requests = 1024;
class EventClass
{
public:
	int N;

	EventClass();

	struct ref
	{
		GameEvent2 *call;
		char name[64];
	} event[max_events];

	void reg(GameEvent2 *g, char *id);

	void request(char *id);	// adds a requested call to the stack
	void issue(int i);		// calls one function of the stack

	int Nreq;
	int req[max_requests];

	void handle();

	bool sendmode;
};

extern EventClass events;
#define EVENT(classname, functionname) {events.reg(new GE<classname>(this, &classname::functionname), #functionname);}
#define CALL(functionname) {events.request( #functionname );}


class Game : public Physics
{
public:
	virtual void register_events();
	virtual void do_game_events2();

	GameType *type;

	virtual void log_file (const char *fname);		// overload to allow networking
	void log_fleet(int channel, class Fleet *fleet);
//	int is_local ( int channel ) ;

protected:

	char events_waiting;
	class GameEvent **waiting_events;
	virtual void do_game_events();
public:
	void send_game_event ( class GameEvent *event );
	virtual void handle_game_event ( int source, class GameEvent *event );
	enum {
		maximum_events_waiting = 100,
		event_invalid = 0,
		event_change_lag,
		event_message,
		event_last
	};


	virtual void preinit();	//pre-initialization (enough so that the destructor won't crash)
	virtual void init(Log *log);    //initialization (loading data, creating a few game objects, whatever)
	virtual ~Game();        //deallocating memory etc.

	virtual void init_lag();

	virtual void _event ( Event *e );

	virtual void change_view (View *new_view);
	virtual void change_view (const char * name );

	View *view;
	VideoWindow *window;

	virtual void prepare();   //called before doing anything with game objects contained within you.  
	//This is particularly necessary if you are working with multiple Games simultaneously

	virtual void calculate();
	virtual void play();
	virtual void animate(Frame *frame); // stupid C++
	virtual void animate();
//	using Physics::animate;

	virtual void object_died(SpaceObject *who, SpaceLocation *source);
	virtual void ship_died(Ship *who, SpaceLocation *source);

	virtual void compare_checksums() ;
	virtual void handle_desynch( int local_checksum, int server_checksum, int client_checksum ) ;

	virtual bool game_ready();

	virtual void set_resolution (int screen_x, int screen_y);
	virtual void redraw();

	bool game_done;
	virtual void quit(const char *message) {game_done = true;}

	// Rob: moved to the physics class
//	bool friendly_fire;
//	double shot_relativity;

	int hiccup_margin;

	int lag_frames;
	int prediction;
	virtual void increase_latency(int n);
	virtual void decrease_latency(int n);
	
	bool view_locked;
	bool physics_locked;

	int num_focuses;
	int focus_index;
	Presence **focus;
	virtual void add_focus (Presence *focus, int channel = -1);

	Targets gametargets;



	virtual Control *create_control (int channel, const char *type, char *config = "Config0", char *file = "scp.ini") ;
	virtual Ship    *create_ship (const char *id, Control *c, Vector2 pos, double angle, int team=0);
	virtual Ship    *create_ship (int channel, const char *id, const char *control, Vector2 pos, double angle, int team=0);

	virtual void pause();
	virtual void unpause();
	virtual bool is_paused();
	virtual void use_idle(int time);

	virtual void play_music();

	Music *music;
	//Music *planet_victory;
	/*
	SpaceSprite *planetSprite;
	SpaceSprite *asteroidSprite;
	SpaceSprite *asteroidExplosionSprite;
	SpaceSprite *sparkSprite;
	SpaceSprite *hotspotSprite;
	SpaceSprite *kaboomSprite;
	SpaceSprite *panelSprite;
	SpaceSprite *xpl1Sprite; //added by Tau
	*/

	public:
	virtual double get_turbo();
	virtual int set_turbo( double t);
	virtual int set_frame_time (int t);
//	protected:
	int time_paused;
	int paused_time;
	double normal_turbo;
	double f4_turbo;
	double next_tic_time;
	double msecs_per_render;
	double next_render_time;
	Histograph *tic_history;
	Histograph *render_history;
	double msecs_per_fps;
	double next_fps_time;
	virtual void fps();
	virtual bool handle_key(int k);
	virtual void save_screenshot();
	int show_fps;
	unsigned char local_checksum, client_checksum, server_checksum;

	// just for debugging the buffered channel
	void net_expect(int val);

	// put all buffered-data generating stuff in this subroutine, for (1) timelag control,
	// and (2) for safety ;) Cause each has to be called in the correct order so that
	// different data won't interfere with each other !!
	void gen_buffered_data();


	// events
	int lagchange;
	void change_lag();
	void chat();
	void test_event1();

	void disconnect();


	// "extreme" desynch testing.
	void heavy_compare();


	void item_sum(char *comment);

	void remove_player(int i);


	virtual PlayerInformation *new_player();	// should return a pointer to a new player-class
};


#endif // __MGAME_H__

