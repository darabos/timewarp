#ifndef __GAME_MAIN__
#define __GAME_MAIN__

#include "gameproject.h"



// this manipulates the *scp pointer (loads/ unloads data ... real nasty).
extern void play_fg(DATAFILE **scp, int scpguimusic);



class ProjectX : public GameProject
{
public:

	// you can use this to add the first gametype, or initialize some data as well.
	// should be overloaded for each project.
	virtual void init();
	// similarly, you can eg. save stuff to disk if needed.
	virtual void quit();
};




#endif // __GAME_MAIN__


