/* $Id$ */ 
#ifndef __GAMEHIERARCHY_H__
#define __GAMEHIERARCHY_H__

#include "../melee/mgame.h"

/*
  This class should allow you to start, play, end a game from within a 
  game (while the  other games are also running in the background). Games
  and their physics are independent.

  The GameStart is special, in that it hosts default melee game data, and
  also manages the games that are spawned.
*/


// this one is a game"type" which is not played exclusively, but only 1 iterations, and
// should be managed by some "play" subroutine (in a main game)
class IterGame : public Game
{
public:
	virtual void play_iteration(unsigned int time);
};




class MainGame;

// an extended game class, which can be embedded in a sequence
// of initiated games ...

class SubGame : public IterGame
{
public:

//	SubGame		*prev, *next;
	MainGame	*maingame;		// the first game (contains the melee data ?)

//	double		refscaletime;		// scale factor for time in the "prev" game

	SubGame();
	~SubGame();

	virtual void init(Log *_log);
};



const int MaxSubGames = 16;	// some arbitrary number



//class GameStart : public SubGame
class MainGame : public IterGame
{
public:

	IterGame	*subgame[MaxSubGames];		// not too many..
	int Nsubgames;

	//virtual void preinit();
	//virtual void init(Log *log);

	//virtual void calculate();
	//virtual void animate(Frame *frame);
	//virtual void animate();

	virtual void addsubgame(SubGame *asubgame);
	virtual void removesubgame(int k);

	virtual void play();
};


#endif // __GAMEHIERARCHY_H__


