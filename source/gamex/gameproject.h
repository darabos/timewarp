#ifndef __GAME_PROJECT__
#define __GAME_PROJECT__

#include "../melee/mframe.h"
#include "../melee/mview.h"
#include "../frame.h"



class GameProject;

// only a little more than physics ...
// intended to include graphics/screen routines/data and
// data loading routines for each specific gametype ?!,
// 
// for a game definition, there should be no constructor, and no destructor specific code defined,
// cause that's inflexible and thus, "bad" - better use some extra overhead (eg init/destroy) which
// can be tweaked easier.

class GameBare : public Physics
{
private:

	double next_tic_time;
	double msecs_per_render;
	double next_render_time;
	
	virtual void idle(int time = 5);

	double escapetime;

public:

	GameBare *prev, *next, *gamerequest;
	bool state;
	bool hardexit;

	// in principle, it sets state=0, but it can be overloaded to have additional uses,
	// like storing data elsewhere on exit, destroying allocated data (if data have
	// been allocated), and such ...
	virtual void quit();
	virtual void refocus();

	// use this only to set pointers to 0 or other default values, not
	// to initialize data (use init for that)
	GameBare();
	virtual ~GameBare();

	View *view;
	VideoWindow *window;

	virtual void animate();
	virtual void animate(Frame *frame);
	
	virtual void set_resolution (int screen_x, int screen_y);
	virtual void redraw();

//	virtual void change_view (View *new_view);
	virtual void change_view (const char * name );

//	virtual void preinit();	//pre-initialization (enough so that the destructor won't crash)
	// doh - the stuff that's in preinit, should really be inside the constructor !!

	virtual void init();    //initialization (loading data, creating a few game objects, whatever)

	virtual void prepare();   //called before doing anything with game objects contained within you.  
	//This is particularly necessary if you are working with multiple Games simultaneously

	// code required to play 1 iteration of the game.
	virtual void play_iteration(unsigned int time);

//	virtual void ship_died(Ship *who, SpaceLocation *source);


	bool game_done;

//	void quit(const char *message) {game_done = true;}

//	virtual void play_music();
//	Music *music;

//	virtual int set_frame_time (int t);

	virtual bool handle_key(int k);

	//virtual void checksync();


	virtual void calculate();
	virtual void checknewgame();

	virtual void calc_kinetic(SpaceLocation *s);
	virtual void handle_edge(SpaceLocation *s);
};



// pretty straightforward ... this has nothing to do with a real game, but, it's called first.

class GameProject
{
public:
	GameBare *first, *last;

	GameProject();

	// you can use this to add the first gametype, or initialize some data as well.
	// should be overloaded for each project.
	virtual void init();
	// similarly, you can eg. save stuff to disk if needed.
	virtual void quit();

	void add(GameBare *asubgame);
	void destroy(GameBare *asubgame);

	// manages play_iterations (also its own) of games.
	void play();
};



#endif

