/** 
GameSessionConfiguration.h
$Id$

  Definition of a class GameSessionConfiguration, which can be used to
  configure a particular game session.
  
  Revision history:
    2004.06.21 yb started
	
  This file is part of "Star Control: TimeWarp" 
  http://timewarp.sourceforge.net/
  Copyright (C) 2001-2004  TimeWarp development team
	  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
		
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/


#ifndef GAMESESSIONCONFIGURATION_HEADER
#define GAMESESSIONCONFIGURATION_HEADER 1

#include <map>
using namespace std;


/** defines when players will choose their fleet for a given game session.  
    Some gametypes will force this value.  The definitions are:
	NO_FLEET, ROUND_START, SPAWN_TIME */
typedef enum { 
	
	/** the chosen gametype doesn't have a fleet selection step, or it'll happen
	    on a schedule the gametype manages itself (ie no automation here). */
	NO_FLEET=0, 
	
	/** players must select their fleet before each round.*/
	ROUND_START,

	/** players choose their fleet when they spawn. */
	SPAWN_TIME

} FleetSelectionPolicy;


enum { MAX_GAMETYPE_NAME = 100 };

class GameSessionConfiguration {

public:
    /** constructor.  */
	GameSessionConfiguration(const char * gametype) {
		strcpy(gametypeName, gametype);
	}
	
	char * getGameType() { return gametypeName; }
	

/*	void insertParameter(const char * key, char * value) { 
		_parameters.insert( pair<const char *, char*> (key,value) );
	}

	void removeParameter(const char * key) { 
		_parameters.erase(key);
	}

	void alterParameter(const char * key, char * value) {
        delete _parameters[key]; // ... is this a good idea, or not? (yb)
		_parameters[key] = value;
	}/**/

protected:
	
    char gametypeName[MAX_GAMETYPE_NAME];
	/*struct LessThanStr
	{
		bool operator()(const char* s1, const char* s2) const
		{ return strcmp(s1, s2) < 0; }
	};
	
	typedef map <const char *, char *, LessThanStr> Parameters;
	typedef map <const char *, char *, LessThanStr>::iterator ParametersIterator;

	Parameters _parameters;*/
};


class MeleeSessionConfiguration : public GameSessionConfiguration {

public:
	MeleeSessionConfiguration(const char * gametype,
                              unsigned short numberOfHumans,
		                      unsigned short maxPlayers,
							  bool padPlayerSlotsWithBots,
							  FleetSelectionPolicy fleetSelectionPolicy) :
	    GameSessionConfiguration(gametype),
        _numberOfHumans(numberOfHumans),
        _maxPlayers(maxPlayers),
        _padPlayerSlotsWithBots(padPlayerSlotsWithBots),
        _fleetSelectionPolicy(fleetSelectionPolicy)
	{
	}

	MeleeSessionConfiguration(const char * gametype) :
	    GameSessionConfiguration(gametype),

		_numberOfHumans(DEFAULT_NUMBER_OF_HUMANS),
		_maxPlayers(DEFAULT_MAX_PLAYERS),
		_padPlayerSlotsWithBots(DEFAULT_PAD_PLAYER_SLOTS_WITH_BOTS),
		_fleetSelectionPolicy((FleetSelectionPolicy)DEFAULT_FLEET_SELECTION_POLICY)
	{
	}

	enum { DEFAULT_NUMBER_OF_HUMANS = 1 };
	enum { DEFAULT_MAX_PLAYERS = 2 };
	enum { DEFAULT_PAD_PLAYER_SLOTS_WITH_BOTS = 1 };
	enum { DEFAULT_FLEET_SELECTION_POLICY = ROUND_START };



protected:
	unsigned short _numberOfHumans;
	unsigned short _maxPlayers;
	bool _padPlayerSlotsWithBots;
	FleetSelectionPolicy _fleetSelectionPolicy;

};





#endif