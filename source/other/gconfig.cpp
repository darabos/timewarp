/* $Id$ */ 
/*
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

#ifdef ALLEGRO_MSVC
#pragma warning (disable:4786)	// identifier was truncated to '255' characters in the browser information
#endif

extern "C" {
#	include "lua.h"
#	include "lualib.h"
#	include "lauxlib.h"
}


#include <string>

#include "melee.h"
REGISTER_FILE
#include "melee/mship.h"
#include "games/ggob.h"
#include "melee/mview.h" // for message
#include "other/orbit.h"

#define gobgame ((GobGame*)game)

static int l_AddSun(lua_State* ls);
static int l_AddPlanet(lua_State* ls);
static int l_AddStation(lua_State* ls);





//////////////////////////////////////////////////////////////////////////////////////////////
//Helper functions
//////////////////////////////////////////////////////////////////////////////////////////////

/*! \brief Find planet by it name */
Planet * FindPlanet (std::string name)
{
  Planet *p = NULL;
  Query q;
  q.begin(game->item[0], bit(LAYER_CBODIES), 99999999, QUERY_OBJECT);

  // if first object is planet
  if (game->item[0]->isPlanet())
	{
	  p = (Planet*)game->item[0];
	  if (p->GetName() == name)
		return p;
	}

  while (q.current) 
	{
	  if (q.current->isPlanet()) 
		{
		  p = (Planet*)q.current;
		  if (p->GetName() == name)
			return p;
		}
	  q.next();
	}

  return NULL;
}

SpaceSprite * GetSprite(char *fileName, char *spriteName)
{
	STACKTRACE;

	DATAFILE *tmpdata;
	tmpdata= load_datafile_object(fileName,spriteName);
	if(tmpdata==NULL)
	{
	  tw_error("Unable to load space sprite");
	  return NULL;
	}

	SpaceSprite *spr=new SpaceSprite(tmpdata, 1, SpaceSprite::MASKED, 1);
	unload_datafile_object(tmpdata);
	return spr;
}

void InitInitializeModule ( lua_State* L )
{
  ASSERT ( L != NULL );
  
  /////////////////////////////////////////////////////////
  // Register C function for using in lua
  ////////////////////////////////////////////////////////
  lua_register(L, "AddSun",              l_AddSun);
  lua_register(L, "AddPlanet",           l_AddPlanet);
  lua_register(L, "AddStation",          l_AddStation);
}

/*! \brief add sun to gob like game
  \param name (string) name of the star used as unique ID of planet form config script
  \param x (string) sun absciss, can be integer value or '?' for ramdom position
  \param y (string) sun ordinate, can be integer value or '?' for ramdom position
*/
static int l_AddSun(lua_State* ls)
{
  int top = lua_gettop(ls); // number of argument
  if (top != 3)
	{
	  tw_error("Wrong argument count in Add Sun (lua script)");
	}
  if ( !lua_isstring(ls, 1) ) {tw_error("AddSun wrong argument type for 1 arg");}
  if ( !lua_isstring(ls, 2) ) {tw_error("AddSun wrong argument type for 2 arg");}
  if ( !lua_isstring(ls, 3) ) {tw_error("AddSun wrong argument type for 3 arg");}

  const char* name   = lua_tostring(ls, 1);
  const char* x      = lua_tostring(ls, 2);
  const char* y      = lua_tostring(ls, 3);

  // Load cordinates
  int nx, ny;
  if ( !strcmp(x, "?") )
	nx = iround(random (game->size.x));
  else
	nx = atoi(x);

  if ( !strcmp(y, "?") )
	ny = iround(random (game->size.y));
  else
	ny = atoi(y);
  Vector2 pos(nx,ny);
	
  char starName[100];
  sprintf(starName,"Star%03d",random()%1);
  SpaceSprite * StarPic=GetSprite("solar.dat",starName);
  if(StarPic==NULL)	error("File error, star pic.  Bailing out...");
  
  Sun *p = new Sun(pos, StarPic, 0);
  if (p == NULL) {tw_error ("Unable to create sun");};
  //p->SetName(name);

  gobgame->add(p);

  return 0;
}

/*! \brief add planet to gob like game
  \param name (string) name of the planet used as unique ID of planet form config script
  \param x (string) planets absciss, can be integer value or '?' for ramdom position
  \param y (string) planets ordinate, can be integer value or '?' for ramdom position
*/
static int l_AddPlanet(lua_State* ls)
{
  int top = lua_gettop(ls); // number of argument
  if (top != 3)
	{
	  tw_error("Wrong argument count in Add Planet (lua script)");
	}
  if ( !lua_isstring(ls, 1) ) {tw_error("AddPlanet wrong argument type for 1 arg");}
  if ( !lua_isstring(ls, 2) ) {tw_error("AddPlanet wrong argument type for 2 arg");}
  if ( !lua_isstring(ls, 3) ) {tw_error("AddPlanet wrong argument type for 3 arg");}

  const char* name   = lua_tostring(ls, 1);
  const char* x      = lua_tostring(ls, 2);
  const char* y      = lua_tostring(ls, 3);

  // Load cordinates
  int nx, ny;
  if ( !strcmp(x, "?") )
	nx = iround(random (game->size.x));
  else
	nx = atoi(x);

  if ( !strcmp(y, "?") )
	ny = iround(random (game->size.y));
  else
	ny = atoi(y);
  Vector2 pos(nx,ny);	

  Planet *p = create_planet(pos);
  if (p == NULL) {tw_error ("Unable to create planet");};
  p->SetName(name);

  while (true) {
	SpaceLocation *n;
	n = p->nearest_planet();
	if (!n || (p->distance(n) > 1500)) break;
	p->translate(random(game->size));
  }

  return 0;
}

/*! \brief add gob space station
  \param name (string) space station name
  \param x (string) not used, supposed for station what does not orbiting planet
  \param y (string) not used, supposed for station what does not orbiting planet
  \param buildtype (string) Ship type producing by station
  \param commander (string) External AI script for station commander
  \param orbit (string) nearest planet
  \param sprite (string) sprite for station
  \param background (string) background picture
*/
static int l_AddStation(lua_State* ls)
{
  int top = lua_gettop(ls); // number of argument
  if (top != 8)
	{
	  tw_error("Wrong argument count in Add Planet (lua script)");
	}
  if ( !lua_isstring(ls, 1) ) {tw_error("AddStation wrong argument type for 1 arg");}
  if ( !lua_isstring(ls, 2) ) {tw_error("AddStation wrong argument type for 2 arg");}
  if ( !lua_isstring(ls, 3) ) {tw_error("AddStation wrong argument type for 3 arg");}
  if ( !lua_isstring(ls, 4) ) {tw_error("AddStation wrong argument type for 4 arg");}
  if ( !lua_isstring(ls, 5) ) {tw_error("AddStation wrong argument type for 5 arg");}
  if ( !lua_isstring(ls, 6) ) {tw_error("AddStation wrong argument type for 6 arg");}
  if ( !lua_isstring(ls, 7) ) {tw_error("AddStation wrong argument type for 7 arg");}
  if ( !lua_isstring(ls, 8) ) {tw_error("AddStation wrong argument type for 8 arg");}
	
  const char* name        = lua_tostring(ls, 1);
  //const char* x           = lua_tostring(ls, 2);
  //const char* y           = lua_tostring(ls, 3);
  const char* buildtype   = lua_tostring(ls, 4);
  const char* commander   = lua_tostring(ls, 5);
  const char* orbit       = lua_tostring(ls, 6);
  const char* sprite      = lua_tostring(ls, 7);
  const char* background  = lua_tostring(ls, 8);

  Planet *p = FindPlanet( orbit );
  ASSERT (p!= NULL);
	
  DATAFILE *tmpdata; 
  bool c = true;
  std::string file, pic;
  for (unsigned int i = 0; i< strlen(sprite);i++)
	{
	  if (sprite[i] == '#')
		{
		  c = false;
		  continue;
		}
	  if (c)
		file +=sprite[i];
	  else
		pic +=sprite[i];
	}
  tmpdata = load_datafile_object(file.c_str(), pic.c_str());
  if (!tmpdata) error( "couldn't find gob.dat#station0sprite");
  SpaceSprite * stationSprite = new SpaceSprite(tmpdata, 1, SpaceSprite::MASKED | SpaceSprite::MIPMAPED, 64);
  unload_datafile_object(tmpdata);
  stationSprite->permanent_phase_shift(8);


  GobStation *gs = new GobStation(stationSprite, p, buildtype, 
								  /*"gob.dat#station0picture.bmp"*/ background, name );
  //delete station_sprite;
  gs->install_external_ai(commander);
  gs->collide_flag_sameship = ALL_LAYERS;
  gs->collide_flag_sameteam = ALL_LAYERS;
  gs->collide_flag_anyone = ALL_LAYERS;
  gobgame->add ( gs );

  return 0;
}
